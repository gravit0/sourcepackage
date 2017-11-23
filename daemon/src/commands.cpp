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

void cmd_exec(std::string cmd, Client* sock) {
    std::vector<std::string> args = split(cmd, ' ');
    std::string basecmd = args[0];
    if (basecmd == "install") {
        std::string pckname = args[1];
        try {
            Package* pck = Package::find(pckname);
            bool isFakeInstall = false;
            bool isNoDep = false;
            bool isFullPath = false;
            unsigned int flags = 0;
            if (args.size() > 2) {
                for (auto &i : args[2]) {
                    if (i == 'f') isFakeInstall = true;
                    else if (i == 'u') isFullPath = true;
                    else if (i == 'd') isNoDep = true;
                }
            }
            if (pck == nullptr) {
                if (isFullPath) Package::get(pckname);
                else pck = Package::get(cfg.packsdir + pckname);
            }
            if (pck == nullptr) {
                sock->write("error pkgnotfound");
                goto ifend;
            }
            if (isFakeInstall) flags |= Package::flag_fakeInstall;
            if (isNoDep) flags |= Package::flag_nodep;
            pck->install(flags);
            sock->write("0");
            event.sendEvent(EventListener::EVENT_INSTALL, pck->dir + " " + (pck->isDaemon ? "d" : ""));
        } catch (package_exception err) {
            if (err.thiserr == package_exception::DependencieNotFound) sock->write("error depnotfound");
            else if (err.thiserr == package_exception::ErrorParsePackage) sock->write("error pkgincorrect");
            else if (err.thiserr == package_exception::FileNotFound) sock->write("error pkgfilenotfound");
        }
    } else if (basecmd == "findfile") {
        std::string filename = args[1];
        bool isBreak;
        Package* resultpck = nullptr;
        for (auto &i : packs) {
            isBreak = false;
            for (auto&j : i->files) {
                if (j.filename == args[2]) {
                    isBreak = true;
                    break;
                }
            }
            if (isBreak) {
                resultpck = i;
                break;
            }
        }
        sock->write("0 " + resultpck->name);
    } else if (basecmd == "packinfo") {
        std::string pckname = args[1];
        Package* pck = Package::find(pckname);
        if (pck == nullptr) {
            sock->write("error pkgnotfound");
            goto ifend;
        }
        std::string dep;
        bool isStart = false;
        for (auto &i : pck->dependencies) {
            if (isStart) dep += ":";
            dep += i;
        }
        sock->write("0 " + pck->name + " " + ((std::ostringstream&)(std::ostringstream() << pck->version.major)).str()
                + " " + pck->dir + " " + pck->author + " " + dep);
    } else if (basecmd == "remove") {
        std::string pckname = args[1];
        Package* pck = Package::find(pckname);
        if (pck != nullptr) {
            pck->remove();
            event.sendEvent(EventListener::EVENT_REMOVE, pck->dir + " " + (pck->isDaemon ? "d" : ""));
            sock->write("0");
        } else {
            sock->write("error pkgnotfound");
        }
    } else if (basecmd == "load") {
        std::string pckdir = args[1];
        Package::get(pckdir);
        sock->write("0");
    } else if (basecmd == "fixdir") {
        chdir("/");
        sock->write("0");
    } else if (basecmd == "addListen") {
        EventListener ev;
        ev.client = sock;
        ev.event = std::stoi(args[1]);
        event.addListener(ev);
        sock->write("0");
        sock->isAutoClosable = false;
    } else if (basecmd == "freeme") {
        sock->isAutoClosable = false;
        sock->write("0");
    } else if (basecmd == "removeListen") {
        event.removeListener(sock);
        sock->write("0");
        sock->isAutoClosable = true;
    } else if (basecmd == "unload") {
        std::string pckdir = args[1];
        Package* pck = Package::find(pckdir);
        delete pck;
        packs.remove(pck);
        sock->write("0");
    } else if (basecmd == "unloadall") {
        for (auto i = packs.begin(); i != packs.end(); ++i) {
            delete(*i);
        }
        packs.clear();
        sock->write("0");
    } else if (basecmd == "reloadall") {
        for (auto& i : packs) {
            Package::read_pack(i->dir, i);
        }
        sock->write("0");
    } else if (basecmd == "reload") {
        std::string pckname = args[1];
        Package* pck = Package::find(pckname);
        if (pck == nullptr) {
            sock->write("error pkgnotfound");
            goto ifend;
        }
        Package::read_pack(pck->dir, pck);
        sock->write("0");
    } else if (basecmd == "updateall") {
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
    } else if (basecmd == "config") {
        std::string param = args[1];
        std::string value = args[2];
        if (param == "rootdir") cfg.rootdir = value;
        else if (param == "packsdir") cfg.packsdir = value;
        else if (param == "socket_timeout") cfg.epoll_timeout = std::stoi(value);
        sock->write("0");
    } else if (basecmd == "getpacks") {
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
    } else if (basecmd == "setconfig") {
        std::string cfgdir = args[1];
        int result = config_parse(cfgdir);
        if (result == 1) {
            sock->write("error cfgnotfound");
        } else {
            sock->write("0");
        }
    } else if (basecmd == "stop") {
        gsock->stop();
        sock->write("0");
    } else {
        sock->write("error commandnotfound");
    }
ifend:
    ;
}
