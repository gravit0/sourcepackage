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
#include <unistd.h>
#include <fstream>
#include <fcntl.h>
#include <dlfcn.h>

CallTable::CmdResult cmd_unknown(unsigned int, std::string)
{
    std::cerr << "UNKNOWN CMD" << std::endl;
    //return CallTable::CmdResult(CallTable::pair{result,sizeof(result)});
    return CallTable::CmdResult(message_result::ERROR_CMDINCORRECT);
}
CallTable::CmdResult cmd_stop(unsigned int, std::string)
{
    std::cerr << "STOP CMD" << std::endl;
    gsock->stop();
    return CallTable::CmdResult(message_result::OK);
}
CallTable::CmdResult cmd_getpacks(unsigned int, std::string)
{
    char* buf;
    int bufsize = 0;
    for (auto& i : packs)
    {
        bufsize+=i.first.size() + 2;
    }
    buf= new char[bufsize + sizeof(message_result)];
    char* it = buf  + sizeof(message_result);
    for (auto& i : packs)
    {
        Package* p = i.second;
        std::cerr << "PKG " << p->name << std::endl;
        char* c_str = p->name.data();
        int str_size = p->name.size();
        memcpy(it+1,c_str,str_size);
        it[str_size+1] = 0;
        *it = (p->isInstalled << 0) | (p->isStartInstall << 1) | (p->isDependence << 2);
        it+=str_size + 2;
    }
    std::cerr << "PKGLIST " << packs.size() << std::endl;
    message_result* m_result = (message_result*) buf;
    m_result->version = 0;
    m_result->code = message_result::OK;
    m_result->flag = 0;
    m_result->size = bufsize;
    return CallTable::CmdResult(CallTable::pair{buf,bufsize + sizeof(message_result)});
}
CallTable::CmdResult cmd_setns(unsigned int, std::string file)
{
    int fd = open(file.c_str(),O_RDONLY);
    setns(fd,0);
    close(fd);
    chdir("/");
    return CallTable::CmdResult(message_result::OK);
};
struct _module_version
{
    unsigned int version;
    unsigned int api;
};
struct _module_api
{
    CallTable::CallCell* calltable;
};
_module_api module_api;
bool module_api_init = false;
CallTable::CmdResult cmd_loadmodule(unsigned int, std::string file)
{
    if(!module_api_init)
    {
        module_api.calltable = gsock->table.table;
        module_api_init = true;
    }
    void* fd = dlopen(file.c_str(), RTLD_LAZY);
    if(fd == NULL) {
        perror("[MODULE ERROR]");
        return CallTable::CmdResult(message_result::ERROR_FILENOTFOUND);
    }
    _module_version (*sp_module_init)(_module_api);
    void (*sp_module_main)();
    sp_module_init = (_module_version (*)(_module_api))dlsym(fd,"sp_module_init");
    _module_version v = sp_module_init(module_api);
    if(v.api != 1)
    {
        std::cerr << "[MODULE] Unsupported API version: " << v.api << std::endl;
        return CallTable::CmdResult(message_result::ERROR_FILENOTFOUND);
    }
    sp_module_main = (void (*)())dlsym(fd,"sp_module_call_main");
    sp_module_main();
    return CallTable::CmdResult(message_result::OK);
};
CallTable::CmdResult cmd_install(unsigned int flag, std::string pckname)
{
    try {
        Package* pck = Package::find(pckname);
        unsigned int flags = 0;
        if (pck == nullptr)
        {
            std::cerr << cfg.packsdir + pckname << std::endl;
            /*if (flag & cmdflags::install::full_path) pck = Package::get(pckname);
            else*/ pck = Package::get(cfg.packsdir + pckname);
            if (pck == nullptr)
            {
                std::cerr << "INSTALL CMD 2" << std::endl;
                return CallTable::CmdResult(message_result::ERROR_PKGNOTFOUND);
            }
        }
        if (flag & cmdflags::install::fakeinstall) flags |= Package::flag_fakeInstall;
        if (flag & cmdflags::install::nodep) flags |= Package::flag_nodep;
        pck->install(flags);
        event.sendEvent(EventListener::EVENT_INSTALL, pck->dir + " " + (pck->isDaemon ? "d" : ""));
    }
    catch (package_exception* err)
    {
        message_result::results errcode;
        if (err->thiserr == package_exception::DependencieNotFound) errcode = message_result::ERROR_DEPNOTFOUND;
        else if (err->thiserr == package_exception::ErrorParsePackage) errcode = message_result::ERROR_PKGINCORRECT;
        else if (err->thiserr == package_exception::FileNotFound) errcode = message_result::ERROR_FILENOTFOUND;
        delete err;
        return CallTable::CmdResult(errcode);
    }
    std::cerr << "INSTALL CMD" << std::endl;
    return CallTable::CmdResult(message_result::OK);
}
CallTable::CmdResult cmd_remove(unsigned int, std::string pckname)
{
    Package* pck = Package::find(pckname);
    if (pck != nullptr)
    {
        pck->remove();
        event.sendEvent(EventListener::EVENT_REMOVE, pck->dir + " " + (pck->isDaemon ? "d" : ""));
    }
    else {
        return CallTable::CmdResult(message_result::ERROR_PKGNOTFOUND);
    }
    return CallTable::CmdResult(message_result::OK);
}
CallTable::CmdResult cmd_freeme(unsigned int, std::string) // Устаревшее
{
    return CallTable::CmdResult(message_result::ERROR_CMDINCORRECT);
}
CallTable::CmdResult cmd_fixdir(unsigned int, std::string)
{
    chdir("/");
    return CallTable::CmdResult(message_result::OK);
}
CallTable::CmdResult cmd_packinfo(unsigned int, std::string pckname)
{
    Package* pck = Package::find(pckname);
    if (pck == nullptr)
    {
        return CallTable::CmdResult(message_result::OK);
    }
    return CallTable::CmdResult(message_result::OK);
}
CallTable::CmdResult cmd_load(unsigned int, std::string pckname)
{
    Package* pck = Package::find(pckname);
    if(pck != nullptr)
    {
        return CallTable::CmdResult(message_result::ERROR_PKGALREADYLOADED);
    }
    else{
        Package* pck = Package::get(pckname);
        if(pck == nullptr)
        {
            return CallTable::CmdResult(message_result::ERROR_PKGNOTFOUND);
        }
    }
    return CallTable::CmdResult(message_result::OK);
}
CallTable::CmdResult cmd_unload(unsigned int, std::string pckname)
{
    Package* pck = packs[pckname];
    delete pck;
    packs.erase(pckname);
    return CallTable::CmdResult(message_result::OK);
}
CallTable::CmdResult cmd_setconfig(unsigned int, std::string cfgdir)
{
    if (config_parse(cfgdir) == 1)
    {
        return CallTable::CmdResult(message_result::ERROR_FILENOTFOUND);
    }
    return CallTable::CmdResult(message_result::OK);
}
void push_cmds()
{
    gsock->table.add(&cmd_unknown); // 0
    gsock->table.add(&cmd_install); // 1
    gsock->table.add(&cmd_remove);  // 2
    gsock->table.add(&cmd_load);    // 3
    gsock->table.add(&cmd_unload);  // 4
    gsock->table.add(&cmd_stop);    // 5
    gsock->table.add(&cmd_getpacks); // 6
    gsock->table.add(&cmd_setconfig); // 7
    gsock->table.add(&cmd_unknown); // 8
    gsock->table.add(&cmd_unknown); // 9
    gsock->table.add(&cmd_unknown); // 10
    gsock->table.add(&cmd_unknown); // 11
    gsock->table.add(&cmd_unknown); // 12
    gsock->table.add(&cmd_unknown); // 13
    gsock->table.add(&cmd_unknown); // 14
    gsock->table.add(&cmd_unknown); // 15
    gsock->table.add(&cmd_fixdir);  // 16
    gsock->table.add(&cmd_freeme);  // 17
    gsock->table.add(&cmd_unknown); // 18
    gsock->table.add(&cmd_unknown); // 19
    gsock->table.add(&cmd_setns);  // 20
    gsock->table.add(&cmd_loadmodule);  // 20
}
void cmd_exec(message_head* head, std::string cmd, Client* sock) {
    std::vector<std::string> args = split(cmd, ' ');
    std::string basecmd = args[0];
    if (head->cmd == cmds::install) {

    } else if (head->cmd == cmds::findfile) {
        std::string filename = args[0];
        bool isBreak;
        Package* resultpck = nullptr;
        for (auto &i : packs) {
            isBreak = false;
            Package* p = i.second;
            for (auto&j : p->files) {
                if (j.filename == filename) {
                    isBreak = true;
                    break;
                }
            }
            if (isBreak) {
                resultpck = p;
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
                Package* p = i.second;
                for (auto&j : p->files) {
                    f << p->name + " " + j.filename << std::endl;
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

    } else if (head->cmd == cmds::load) {
        std::string pckdir = args[0];
        Package::get(pckdir);
        sock->write("0");
    } else if (head->cmd == cmds::fixdir) {
    } else if (head->cmd == cmds::add_listener) {
        EventListener ev;
        ev.client = sock;
        ev.event = std::stoi(args[0]);
        event.addListener(ev);
        sock->write("0");
        sock->isAutoClosable = false;
    } else if (head->cmd == cmds::remove_listener) {
        event.removeListener(sock);
        sock->write("0");
        sock->isAutoClosable = true;
    }  else if (head->cmd == cmds::reload) {
        std::string pckname = args[0];
        Package* pck = Package::find(pckname);
        if (pck == nullptr) {
            sock->write("error pkgnotfound");
            goto ifend;
        }
        Package::read_pack(pck->dir, pck);
        sock->write("0");
    } else if (head->cmd == cmds::config) {
        std::string param = args[0];
        std::string value = args[1];
        if (param == "rootdir") cfg.rootdir = value;
        else if (param == "packsdir") cfg.packsdir = value;
        else if (param == "socket_timeout") cfg.epoll_timeout = std::stoi(value);
        sock->write("0");
    } else if (head->cmd == cmds::setconfig) {

    } else if (head->cmd == cmds::stop) {
        gsock->stop();
        sock->write("0");
    } else {
        sock->write("error commandnotfound");
    }
ifend:
    ;
}
