/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   commands.cpp
 * Author: gravit
 *
 * Created on 2 ноября 2017 г., 13:21
 */
#include "main.hpp"
#include "EventManager.hpp"
#include <sstream>
#include <iostream>
#include <sys/stat.h>
#include <fstream>

std::pair<void*,size_t> cmd_unknown(unsigned int, std::string)
{
    message_result* result = new message_result{5,0,0,0};
    std::cerr << "UNKNOWN CMD" << std::endl;
    return {result,sizeof(result)};
}
void push_cmds()
{
    gsock->table.add(&cmd_unknown);
    gsock->table.add(&cmd_unknown);
    gsock->table.add(&cmd_unknown);
    gsock->table.add(&cmd_unknown);
    gsock->table.add(&cmd_unknown);
    gsock->table.add(&cmd_unknown);
    gsock->table.add(&cmd_unknown);
    gsock->table.add(&cmd_unknown);
    gsock->table.add(&cmd_unknown);
    gsock->table.add(&cmd_unknown);
    gsock->table.add(&cmd_unknown);
    gsock->table.add(&cmd_unknown);
    gsock->table.add(&cmd_unknown);
    gsock->table.add(&cmd_unknown);
    gsock->table.add(&cmd_unknown);
    gsock->table.add(&cmd_unknown);
    gsock->table.add(&cmd_unknown);
    gsock->table.add(&cmd_unknown);
    gsock->table.add(&cmd_unknown);
}
void cmd_exec(message_head* head, std::string cmd, Client* sock) {
    std::vector<std::string> args = split(cmd, ' ');
    std::string basecmd = args[0];
    if (head->cmd == cmds::install) {
        std::string pckname = args[0];
        try {
            Package* pck = Package::find(pckname);
            unsigned int flags = 0;
            if (pck == nullptr) {
                if (head->flag & flags::fullpath) pck = Package::get(pckname);
                else pck = Package::get(cfg.packsdir + pckname);
            }
            if (pck == nullptr) {
                sock->write("error pkgnotfound");
                goto ifend;
            }
            if (head->cmdflags & cmdflags::install::fakeinstall) flags |= Package::flag_fakeInstall;
            if (head->cmdflags & cmdflags::install::nodep) flags |= Package::flag_nodep;
            pck->install(flags);
            sock->write("0");
            event.sendEvent(EventListener::EVENT_INSTALL, pck->dir + " " + (pck->isDaemon ? "d" : ""));
        } catch (package_exception err) {
            if (err.thiserr == package_exception::DependencieNotFound) sock->write("error depnotfound");
            else if (err.thiserr == package_exception::ErrorParsePackage) sock->write("error pkgincorrect");
            else if (err.thiserr == package_exception::FileNotFound) sock->write("error pkgfilenotfound");
        }
    } else if (head->cmd == cmds::findfile) {
        std::string filename = args[0];
        bool isBreak;
        Package* resultpck = nullptr;
        for (auto &i : packs) {
            isBreak = false;
            for (auto&j : i->files) {
                if (j.filename == filename) {
                    isBreak = true;
                    break;
                }
            }
            if (isBreak) {
                resultpck = i;
                break;
            }
        }
        if(resultpck != nullptr) sock->write("0 " + resultpck->name);
        else sock->write("error notfound");
    } else if (head->cmd == cmds::exportfiles) {
        std::string filename = args[0];
        std::fstream f;
        f.open(filename,std::ios_base::out);
        for (auto &i : packs) {
            for (auto&j : i->files) {
                f << i->name + " " + j.filename << std::endl;
            }
        }
        f.close();
        sock->write("0");
    }else if (head->cmd == cmds::packinfo) {
        std::string pckname = args[0];
        Package* pck = Package::find(pckname);
        if (pck == nullptr) {
            sock->write("error pkgnotfound");
            goto ifend;
        }
        std::string dep;
        bool isStart = false;
        for (auto &i : pck->dependencies) {
            if (isStart) dep += ":";
            dep += i.name;
        }
        sock->write("0 " + pck->name + " " + ((std::ostringstream&)(std::ostringstream() << pck->version.major)).str()
                + " " + pck->dir + " " + pck->author + " " + dep);
    } else if (head->cmd == cmds::remove) {
        std::string pckname = args[0];
        Package* pck = Package::find(pckname);
        if (pck != nullptr) {
            pck->remove();
            event.sendEvent(EventListener::EVENT_REMOVE, pck->dir + " " + (pck->isDaemon ? "d" : ""));
            sock->write("0");
        } else {
            sock->write("error pkgnotfound");
        }
    } else if (head->cmd == cmds::load) {
        std::string pckdir = args[0];
        Package::get(pckdir);
        sock->write("0");
    } else if (head->cmd == cmds::fixdir) {
        chdir("/");
        sock->write("0");
    } else if (head->cmd == cmds::add_listener) {
        EventListener ev;
        ev.client = sock;
        ev.event = std::stoi(args[0]);
        event.addListener(ev);
        sock->write("0");
        sock->isAutoClosable = false;
    } else if (head->cmd == cmds::freeme) {
        sock->isAutoClosable = false;
        sock->write("0");
    } else if (head->cmd == cmds::remove_listener) {
        event.removeListener(sock);
        sock->write("0");
        sock->isAutoClosable = true;
    } else if (head->cmd == cmds::unload) {
        std::string pckdir = args[0];
        Package* pck = Package::find(pckdir);
        delete pck;
        packs.remove(pck);
        sock->write("0");
    } else if (head->cmd == cmds::unloadall) {
        for (auto i = packs.begin(); i != packs.end(); ++i) {
            delete(*i);
        }
        packs.clear();
        sock->write("0");
    } else if (head->cmd == cmds::reloadall) {
        for (auto& i : packs) {
            Package::read_pack(i->dir, i);
        }
        sock->write("0");
    } else if (head->cmd == cmds::reload) {
        std::string pckname = args[0];
        Package* pck = Package::find(pckname);
        if (pck == nullptr) {
            sock->write("error pkgnotfound");
            goto ifend;
        }
        Package::read_pack(pck->dir, pck);
        sock->write("0");
    } else if (head->cmd == cmds::updateall) {
        for (auto& i : packs) {
            Package_Version oldver = i->version;
            i->clear();
            Package::read_pack(i->dir, i);
            if (i->version > oldver) {

                i->isInstalled = false;
                i->install(Package::flag_update);
            }
        }
        sock->write("0");
    } else if (head->cmd == cmds::config) {
        std::string param = args[0];
        std::string value = args[1];
        if (param == "rootdir") cfg.rootdir = value;
        else if (param == "packsdir") cfg.packsdir = value;
        else if (param == "socket_timeout") cfg.epoll_timeout = std::stoi(value);
        sock->write("0");
    } else if (head->cmd == cmds::getpacks) {
        std::string reply = "0 ";
        for (auto& i : packs) {
            reply += i->name;
            reply += ":";
            if (i->isInstalled) reply += "i";
            if (i->isDependence) reply += "d";
            if (i->isDaemon) reply += "D";
            reply += ' ';
        }
        sock->write(reply);
    } else if (head->cmd == cmds::setconfig) {
        std::string cfgdir = args[1];
        int result = config_parse(cfgdir);
        if (result == 1) {
            sock->write("error cfgnotfound");
        } else {
            sock->write("0");
        }
    } else if (head->cmd == cmds::stop) {
        gsock->stop();
        sock->write("0");
    } else {
        sock->write("error commandnotfound");
    }
ifend:
    ;
}
