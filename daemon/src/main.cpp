#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unistd.h>
#include "main.hpp"
#include <string.h>
#include <sys/stat.h>
#include <list>
#include <sys/signal.h>
#include <thread>
#include "basefunctions.h"
#include "Sock.hpp"
#include "getopts.h"
#include <functional>
#include "util.hpp"
#include "EventManager.hpp"
#include <boost/property_tree/ini_parser.hpp>
using namespace std;
std::mutex pack_mutex;
std::map<std::string, Package::ptr> packs;
Configuration cfg;
Logger * logger;
Sock* gsock;
int main_set_signals();
std::vector<std::string_view> split(std::string_view cmd, const char splitchar) {
    int opos = 0;
    std::vector<std::string_view> list;
    while (true) {
        int pos = cmd.find(splitchar, opos);
        bool isKav = false;
        std::string value;
        if (cmd[opos] == '"')
            while (cmd[pos - 1] != '"') {
                isKav = true;
                if (pos < 0) break;
                pos = cmd.find(splitchar, pos + 1);
            }
        if (pos <= 0) {
            if (isKav) value = cmd.substr(opos + 1, cmd.size() - opos - 2);
            else value = cmd.substr(opos, pos - cmd.size());
            list.push_back(value);
            break;
        }
        if (isKav) value = cmd.substr(opos + 1, pos - opos - 2);
        else value = cmd.substr(opos, pos - opos);
        list.push_back(value);
        opos = pos + 1;
    }
    return list;
}

int config_parse(const std::string& filename) {
    auto lam = [](std::string_view frist, std::string_view last, std::string_view, bool) {
        if (frist == "rootdir" && !cfg.isSetRootdir) cfg.rootdir = last;
        else if (frist == "pkgdir" && !cfg.isSetPackdir) cfg.packsdir = last;
        else if (frist == "sockfile" && !cfg.isSetSockfile) cfg.sockfile = last;
        else if (frist == "autoinstall") cfg.autoinstall = last;
        else if (frist == "pidfile") cfg.pidfile = last;
        else if (frist == "daemontype") {
            if (last == "simple") cfg.daemon_type = Configuration::CFG_DAEMON_SIMPLE;
            else if (last == "forking") cfg.daemon_type = Configuration::CFG_DAEMON_FORKING;
        } else if (frist == "setuid_mode") {
            if (last == "suid") cfg.setuid_mode = Configuration::CFG_SETUID_SUID;
            else if (last == "usermode") cfg.setuid_mode = Configuration::CFG_SETUID_USERMODE;
            else if (last == "change") cfg.setuid_mode = Configuration::CFG_SETUID_CHANGE;
        } else if (frist == "reinstall_socket") {
            cfg.reinstall_socket = (last == "true") ? true : false;
        } else if (frist == "ignore_low_exception") {
            cfg.isIgnoreLowException = (last == "true") ? true : false;
        } else if (frist == "max_connect") {
            cfg.max_connect = std::stoi(std::string(last));
        } else if (frist == "epoll_timeout") {
            cfg.epoll_timeout = std::stoi(std::string(last));
        }
    };
    RecArrUtils::ini_parser_lam(filename,lam);
    chdir(cfg.rootdir.c_str());
    return 0;
}

void signal_handler(int sig) {
    if (gsock != nullptr) delete gsock;
    if (sig == SIGTERM) exit(0);
    exit(-sig);
}

int main(int argc, char** argv) {
    //RecArrUtils::ini_parser_lam("/home/gravit/test3.cfg",[](std::string key,std::string value,std::string category,bool isSet){
    //                                std::cout << key << " " << value << " " << category << " " << isSet << std::endl ;
    //                            });
    logger = new Logger(Logger::LOG_STDERR);
    gsock = nullptr;
    //signal(SIGTERM, signal_handler);
    main_set_signals();
    int opt = getopt_long(argc, argv, getopts::optString, getopts::long_options, NULL);
    cfg.isDaemon = false;
    std::string config_file = "/etc/sp.cfg";
    while (opt != -1) {
        switch (opt) {
            case 'd':
                cfg.isDaemon = true;
                std:: cout << "start daemon" << std::endl;
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
            {
                std::cout << "Source Package 2.0.0-1" << std::endl;
                std::cout << "Author: Gravit" << std::endl;
                std::cout << "Github: https://github.com/gravit0/sourcepackage" << std::endl;
                std::cout << "This free software: you can modify and distribute it." << std::endl;
                return 0;
                break;
            }
            default:
                /* сюда на самом деле попасть невозможно. */
                break;
        }
        opt = getopt_long(argc, argv, getopts::optString, getopts::long_options, NULL);
    }
    if (getopts::longopts.isNoWarning == 1) cfg.isAllowWarning = false;
    if (getopts::longopts.isHelp == 1) {
        std::cout << "USAGE: " << std::string(argv[0]) << " [ARGS]" << std::endl;
        std::cout << "ARGS:" << std::endl;
        std::cout << "-d,  --daemon    start daemon, listen socket" << std::endl;
        std::cout << "-c [config]      use config [config]" << std::endl;
        std::cout << "-s [sockfile]    use UNIX socket [sockfile]" << std::endl;
        std::cout << "-r [rootdir]     set root directory" << std::endl;
        std::cout << "-v               program version" << std::endl;
        std::cout << "--no-forking     Deny daemontype=forking" << std::endl;
        std::cout << "--no-warning     Deny print warnings" << std::endl;
        return 0;
    }
    if (getopts::longopts.isDaemon == 1) cfg.isDaemon = true;
    int cfgres = config_parse(config_file);
    if (cfgres == 1) {
        std::cerr << "[CRITICAL] Config " << config_file << " not found" << std::endl;
        return -1;
        //throw std::string("Config not found.");
    }
    if (cfg.isAllowWarning) {
        struct stat statbuff;
        if (stat(cfg.rootdir.c_str(), &statbuff) != 0) logger->logg('W', "rootdir " + cfg.rootdir + " not found");
        if (stat(cfg.packsdir.c_str(), &statbuff) != 0) logger->logg('W', "packsdir " + cfg.packsdir + " not found");
    }
    if (cfg.isAutoinstall) {
        std::cerr << "Autoinstall started..." << std::endl;
        auto list = split(cfg.autoinstall, ':');
        for (auto i = list.begin(); i != list.end(); ++i) {
            std::string pckname = std::string(*i);
            auto pck = Package::find(pckname);
            if (!pck) pck = Package::get(cfg.packsdir + pckname);
            if (!pck) {
                logger->logg('E', "package " + pckname + " not found");
                continue;
            }
            std::cerr << "Autoinstall " << pckname << std::endl;
            (*pck)->install();
        }
    }
    //Check privileges
    {
        cfg.security.ppid = getppid();
        cfg.security.pid = getpid();
        cfg.security.uid = getuid();
        cfg.security.euid = geteuid();
        cfg.security.gid = getgid();
        cfg.security.egid = getegid();
        if (cfg.setuid_mode == Configuration::CFG_SETUID_SUID) {
            if (cfg.security.uid != cfg.security.euid) {
                cfg.security.uid = cfg.security.euid;
                setuid(cfg.security.euid);
            } else {
                logger->logg('E', "SUID mode enabled, SUID bit is not set. Please add suid bit or set config.");
            }
            if (cfg.security.gid != cfg.security.egid) {
                cfg.security.gid = cfg.security.egid;
                setgid(cfg.security.egid);
            } else {
                logger->logg('E', "SUID mode enabled, SGID bit is not set. Please add sgid bit or set config.");
            }
        }
        if (cfg.security.uid != 0 && (cfg.setuid_mode == Configuration::CFG_SETUID_USERMODE || cfg.setuid_mode == Configuration::CFG_SETUID_NONE)) {
            logger->logg('E', "UID != 0 and usermode off");
        }
    }
    if(!cfg.isDaemon && !getopts::longopts.isHelp && !cfg.isPrintVersion) {
            std::cout << "Start daemon" << std::endl;
            cfg.isDaemon = true;
    }
    ////
    if (!cfg.isDaemon) return 0;
    try {
        gsock = new Sock(cfg.sockfile, cfg.max_connect + 1);
        if (cfg.daemon_type == Configuration::CFG_DAEMON_FORKING && !(getopts::longopts.isNoForking == 1)) {
            int pid = fork();
            if (pid > 0) {
                std::fstream pidfile;
                pidfile.open(cfg.pidfile, std::ios_base::out);
                if (pidfile) pidfile << pid;
                else {
                    logger->logg('E', "pid file " + cfg.pidfile + " is not created");
                }
                pidfile.close();
                exit(0);
            } else cfg.security.pid = getpid();
        }
        push_cmds();
        //std::thread th([]{
        //    std::cerr << "THREAD RUN" << std::endl;
        //    gsock->loop_impl(Sock::multithread_loop::SLAVE);
        //    std::cerr << "THREAD END" << std::endl;
        //}
        //);
        gsock->loop();
        //th.join();
    } catch (socket_exception* e) {
        logger->logg('C', "An exception was thrown out. Information: " + std::string(e->what()));
        perror("[C] Failed reason:");
        delete e;
        exit(-1);
    }
    delete gsock;
    //for (auto i = packs.begin(); i != packs.end(); ++i) {
    //    delete (*i).second;
    //}
    packs.clear();
    delete logger;
    return 0;
}
