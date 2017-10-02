#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <fcntl.h>           /* Definition of AT_* constants */
#include "main.hpp"
#include <string.h>
#include <list>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "pkgutil.hpp"
#include "Sock.hpp"
#include "getopts.h"
using namespace std;
std::list<Package*> packs;
Configuration cfg;

std::vector<std::string> parsecmd(std::string cmd) {
    int opos = 0;
    std::vector<std::string> list;
    while (true) {
        int pos = cmd.find(':', opos);
        if (pos <= 0) {
            std::string value = cmd.substr(opos, pos - cmd.size());
            list.push_back(value);
            break;
        }
        std::string value = cmd.substr(opos, pos - opos);
        list.push_back(value);
        opos = pos + 1;
    }
    return list;
}

int config_parse(std::string filename) {
    std::fstream f(filename, std::ios_base::in);
    if (!f) {
        return 1;
    }
    std::string info;
    std::string category;
    while (std::getline(f, info)) {
        if (info.size() <= 1) continue;
        if (info[0] == '[') {
            category = info.substr(1, info.size() - 2);
            continue;
        }
        int pos = info.find('=');
        if (pos < 0) continue;
        std::string frist = info.substr(0, pos);
        std::string last = info.substr(pos + 1, info.size());
        if (frist == "rootdir" && !cfg.isSetRootdir) cfg.rootdir = last;
        else if (frist == "pkgdir" && !cfg.isSetPackdir) cfg.packsdir = last;
        else if (frist == "sockfile" && !cfg.isSetSockfile) cfg.sockfile = last;
        else if (frist == "autoinstall") cfg.autoinstall = last;
        else if (frist == "daemontype") {
            if (last == "simple") cfg.daemon_type = Configuration::CFG_DAEMON_SIMPLE;
            else if (last == "forking") cfg.daemon_type = Configuration::CFG_DAEMON_FORKING;
        }
    }
    return 0;
}

void cmd_exec(std::string cmd, Sock* sock) {
    std::vector<std::string> args = parsecmd(cmd);
    std::string basecmd = args[0];
    if (basecmd == "install") {
        std::string pckname = args[1];
        Package* pck = find_pack(pckname);
        if (pck == nullptr) pck = get_pack(cfg.packsdir + pckname);
        if (pck == nullptr) {
            std::string errstr = "package " + pckname + " not found\n";
            sock->write(errstr);
            goto ifend;
        }
        pck->install();
    } else if (basecmd == "fakeinstall") {
        std::string pckname = args[1];
        Package* pck = find_pack(pckname);
        if (pck == nullptr) pck = get_pack(cfg.packsdir + pckname);
        if (pck == nullptr) {
            std::string errstr = "package " + pckname + " not found\n";
            sock->write(errstr);
            goto ifend;
        }
        pck->fakeinstall();
    }else if (basecmd == "installu") {
        std::string pckname = args[1];
        Package* pck = find_pack(pckname);
        if (pck == nullptr) pck = get_pack(pckname);
        if (pck == nullptr) {
            std::string errstr = "package " + pckname + " not found\n";
            sock->write(errstr);
            goto ifend;
        }
        pck->install();
    } else if (basecmd == "remove") {
        std::string pckname = args[1];
        Package* pck = find_pack(pckname);
        if (pck != nullptr) pck->remove_();
        else {
            std::string errstr = "package " + pckname + " not found\n";
            sock->write(errstr);
        }
    } else if (basecmd == "load") {
        std::string pckdir = args[1];
        get_pack(pckdir);
    } else if (basecmd == "unload") {
        std::string pckdir = args[1];
        get_pack(pckdir);
    } else if (basecmd == "setroot") {
        std::string rootdir = args[1];
        cfg.rootdir = rootdir;
    } else if (basecmd == "setpckdir") {
        std::string packsdir = args[1];
        cfg.packsdir = packsdir;
    } else if (basecmd == "getpacks") {
        std::string reply;
        for(auto i=packs.begin();i!=packs.end();i++)
        {
            reply+= (*i)->name;
            reply+= ":";
            if((*i)->isInstalled) reply+= "i";
            if((*i)->isDependence) reply+= "d";
            reply+='\n';
        }
        sock->write(reply);
    } else if (basecmd == "setconfig") {
        std::string cfgdir = args[1];
        int result = config_parse(cfgdir);
        if (result == 1) {
            std::string errstr = "Config " + cfgdir + " not found\n";
            sock->write(errstr);
        } else {
            std::string errstr = "Config " + cfgdir + " found. pkgdir = " + cfg.packsdir + " rootdir = " + cfg.rootdir + "\n";
            sock->write(errstr);
        }
    } else if (basecmd == "stop") {
        sock->stop();
    }
    ifend: ;
}

int main(int argc, char** argv) {
    int opt = getopt_long(argc, argv, getopts::optString,getopts::long_options,NULL);
    cfg.isDaemon = false;
    std::string config_file = "/etc/sp.cfg";
    while (opt != -1) {
        switch (opt) {
            case 'd':
                cfg.isDaemon = true;
                break;
            case 's':
            {
                cfg.sockfile = std::string(optarg);
                cfg.isSetSockfile = true;
                break;
            }
            case 'i':
            {
                cfg.isAutoinstall = true;
                break;
            }
            case 'r':
            {
                cfg.rootdir = std::string(optarg);
                cfg.isSetRootdir = true;
                break;
            }
            case 'c':
                config_file = std::string(optarg);
                break;
            case 'v':
                std::cout << "Source Package v 0.0.2" << std::endl;
                break;
            default:
                /* сюда на самом деле попасть невозможно. */
                break;
        }
        opt = getopt_long(argc, argv, getopts::optString,getopts::long_options,NULL);
    }
    if(getopts::longopts.isHelp == 1)
    {
        std::cout << "Использование: " << std::string(argv[0]) << " [КЛЮЧ]" << std::endl;
        std::cout << "Аргументы:" << std::endl;
        std::cout << "-d,  --daemon    Запускает демон" << std::endl;
        std::cout << "-c [config]      Использовать конфиг [config]" << std::endl;
        std::cout << "-s [sockfile]    Использовать UNIX сокет [sockfile]" << std::endl;
        std::cout << "-r [rootdir]     Изменяет root директорию" << std::endl;
        std::cout << "-v               Версия программы" << std::endl;
        std::cout << "--no-forking     Запрет daemontype=forking" << std::endl;
        return 0;
    }
    if(getopts::longopts.isDaemon == 1) cfg.isDaemon = true;
    int cfgres = config_parse(config_file);
    if (cfgres == 1) {
        std::cerr << "Config " << config_file << " not found" << std::endl;
        return -1;
        //throw std::string("Config not found.");
    }
    if (cfg.isAutoinstall) {
        auto list = parsecmd(cfg.autoinstall);
        std::cout << list.size();
        for (auto i = list.begin(); i != list.end(); ++i) {
            std::string pckname = (*i);
            Package* pck = find_pack(pckname);
            if (pck == nullptr) pck = get_pack(cfg.packsdir + pckname);
            if (pck == nullptr) {
                std::cerr << "package " << pckname << " not found";
                continue;
            }
            pck->install();
        }
    }
    if (!cfg.isDaemon) return 0;
    if (cfg.daemon_type == Configuration::CFG_DAEMON_FORKING && !(getopts::longopts.isNoForking == 1)) {
        int pid = fork();
        if (pid > 0) {
            exit(0);
        }
    }
    Sock* sock = new Sock(cfg.sockfile);
    sock->loop(&cmd_exec);
    delete sock;
    return 0;
}
