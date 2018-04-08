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
#include <chrono>
#include <thread>

CallTable::CmdResult cmd_unknown(unsigned int, std::string)
{
    return CallTable::CmdResult(message_result::ERROR_CMDINCORRECT);
}
CallTable::CmdResult cmd_stop(unsigned int, std::string)
{
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
        Package::ptr p = i.second;
        std::cerr << "PKG:" << p->name << std::endl;
        char* c_str = p->name.data();
        int str_size = p->name.size();
        memcpy(it+1,c_str,str_size);
        it[str_size+1] = 0;
        *it = (p->isInstalled << 0) | (p->isStartInstall << 1) | (p->isDependence << 2);
        it+=str_size + 2;
    }
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
        std::optional<Package::ptr> pck = Package::find(pckname);
        unsigned int flags = 0;
        if (!pck)
        {
            if (flag & cmdflags::install::full_path) pck = Package::get(pckname);
            else pck = Package::get(cfg.packsdir + pckname);
            if (!pck)
            {
                return CallTable::CmdResult(message_result::ERROR_PKGNOTFOUND);
            }
        }
        if (flag & cmdflags::install::fakeinstall) flags |= Package::flag_fakeInstall;
        if (flag & cmdflags::install::nodep) flags |= Package::flag_nodep;
        (*pck)->install(flags);
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
    return CallTable::CmdResult(message_result::OK);
}
CallTable::CmdResult cmd_remove(unsigned int, std::string pckname)
{
    auto pck = Package::find(pckname);
    if (pck)
    {
        (*pck)->remove();
    }
    else {
        return CallTable::CmdResult(message_result::ERROR_PKGNOTFOUND);
    }
    return CallTable::CmdResult(message_result::OK);
}
CallTable::CmdResult cmd_fixdir(unsigned int, std::string)
{
    chdir("/");
    return CallTable::CmdResult(message_result::OK);
}
CallTable::CmdResult cmd_packinfo(unsigned int, std::string pckname)
{
    auto pck = Package::find(pckname);
    if (!pck)
    {
        return CallTable::CmdResult(message_result::OK);
    }
    return CallTable::CmdResult(message_result::OK);
}
CallTable::CmdResult cmd_load(unsigned int, std::string pckname)
{
    auto pck = Package::find(pckname);
    if(!pck)
    {
        return CallTable::CmdResult(message_result::ERROR_PKGALREADYLOADED);
    }
    else{
        auto pck = Package::get(pckname);
        if(!pck)
        {
            return CallTable::CmdResult(message_result::ERROR_PKGNOTFOUND);
        }
    }
    return CallTable::CmdResult(message_result::OK);
}
CallTable::CmdResult cmd_unload(unsigned int, std::string pckname)
{
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
    gsock->table.add(&cmd_unknown);  // 17
    gsock->table.add(&cmd_unknown); // 18
    gsock->table.add(&cmd_unknown); // 19
    gsock->table.add(&cmd_setns);  // 20
    gsock->table.add(&cmd_loadmodule);  // 21
}
