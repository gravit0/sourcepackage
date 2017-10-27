#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <fcntl.h>           /* Definition of AT_* constants */
#include "main.hpp"
#include <string.h>
#include <sys/stat.h>
#include <list>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <arpa/inet.h>
#include <thread>
#include "basefunctions.h"
#include "Sock.hpp"
#include "getopts.h"
#include <functional>
#include "util.hpp"
#include <boost/property_tree/ini_parser.hpp>
using namespace std;
std::mutex pack_mutex;
std::list<Package*> packs;
Configuration cfg;
const char* package_exception::what() const noexcept {
    switch (thiserr) {
        case DependencieNotFound: return "Dependencie Not Found";
        case ErrorParsePackage: return "Error parse package";
        default: return "Unknown Error";
    }
}
package_exception::package_exception(Errors err) {
    thiserr = err;
}
std::vector<std::string> split(const std::string cmd, const char splitchar) {
    int opos = 0;
    std::vector<std::string> list;
    while (true) {
        bool tr = false;
        int pos = findNoSlash(cmd, splitchar, opos, &tr);
        if (pos <= 0) {
            std::string value = cmd.substr(opos, pos - cmd.size());
            if (tr) SlashReplace(&value, 0);
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
    RecursionArray acfg;
    boost::property_tree::ini_parser::read_ini(filename, acfg);
    RecursionArray maincfg = acfg.get_child("main");
    for (auto& i : maincfg) {
        std::string frist = i.first;
        std::string last = i.second.get<std::string>("");
        if (frist == "rootdir" && !cfg.isSetRootdir) cfg.rootdir = last;
        else if (frist == "pkgdir" && !cfg.isSetPackdir) cfg.packsdir = last;
        else if (frist == "sockfile" && !cfg.isSetSockfile) cfg.sockfile = last;
        else if (frist == "autoinstall") cfg.autoinstall = last;
        else if (frist == "daemontype") {
            if (last == "simple") cfg.daemon_type = Configuration::CFG_DAEMON_SIMPLE;
            else if (last == "forking") cfg.daemon_type = Configuration::CFG_DAEMON_FORKING;
        } else if (frist == "reinstall_socket") {
            cfg.reinstall_socket = (last == "true") ? true : false;
        }
    }
    return 0;
}
std::list<std::thread*> closed_thread;
std::list<int> closed_socks;
Sock* gsock;

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
        if(args.size() >= 2)
        {
            for( auto &i : args[2])
            {
                if(i == 'f') isFakeInstall = true;
                else if(i == 'u') isFullPath = true;
                else if(i == 'd') isNoDep = true;
            }
        }
        if (pck == nullptr) {
            if(isFullPath) Package::get(pckname);
            else pck = Package::get(cfg.packsdir + pckname);
        }
        if (pck == nullptr) {
            sock->write("error pkgnotfound");
            goto ifend;
        }
        if(isFakeInstall) flags |= Package::flag_fakeInstall;
        if(isNoDep) flags |= Package::flag_nodep;
        pck->install(flags);
        sock->write("0 " + pck->daemonfile + " " + pck->logfile);
        } catch(package_exception err)
        {
            if(err.thiserr == package_exception::DependencieNotFound) sock->write("error depnotfound");
            if(err.thiserr == package_exception::ErrorParsePackage) sock->write("error pkgincorrect");
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
    } else if (basecmd == "remove") {
        std::string pckname = args[1];
        Package* pck = Package::find(pckname);
        if (pck != nullptr) {
            pck->remove_();
            sock->write("0 ");
        } else {
            sock->write("error pkgnotfound");
        }
    } else if (basecmd == "load") {
        std::string pckdir = args[1];
        Package::get(pckdir);
        sock->write("0 ");
    } else if (basecmd == "apistream") {
        sock->isAutoClosable = false;
        auto lambda = [sock]() {
            bool isloop = true;
            sock->write("0 ");
            while (isloop) {
                if (sock->read() < 1) {
                    std::cout << "Apistream closed.";
                    isloop = false;
                    break;
                }
                sock->buf[sock->bytes] = 0;
                std::string command(sock->buf, sock->bytes);
                cmd_exec(command, sock);
            }
            delete sock;
        };
        std::thread th(lambda);
        th.detach();
    } else if (basecmd == "unload") {
        std::string pckdir = args[1];
        for (auto i = packs.begin(); i != packs.end(); ++i) {
            if ((*i)->name == pckdir) {
                delete (*i);
                packs.erase(i);
            }
        }
        sock->write("0 ");
    } else if (basecmd == "setroot") {
        std::string rootdir = args[1];
        cfg.rootdir = rootdir;
        sock->write("0 ");
    } else if (basecmd == "setpckdir") {
        std::string packsdir = args[1];
        cfg.packsdir = packsdir;
        sock->write("0 ");
    } else if (basecmd == "getpacks") {
        std::string reply;
        for (auto& i : packs) {
            reply += i->name;
            reply += ":";
            if (i->isInstalled) reply += "i";
            if (i->isDependence) reply += "d";
            reply += ' ';
        }
        sock->write(reply);
    } else if (basecmd == "setconfig") {
        std::string cfgdir = args[1];
        int result = config_parse(cfgdir);
        if (result == 1) {
            sock->write("error cfgnotfound");
        } else {
            sock->write("0 ");
        }
    } else if (basecmd == "stop") {
        gsock->stop();
        sock->write("0 ");
    }
ifend:
    ;
}

void signal_handler(int sig) {
    if (gsock != nullptr) delete gsock;
    if (sig == SIGTERM) exit(0);
    exit(-sig);
}

int main(int argc, char** argv) {
    gsock = nullptr;
    signal(SIGTERM, signal_handler);
    if (getpid() == 1) {
        std::cout << "I am NOT init!" << std::endl;
    }
    int opt = getopt_long(argc, argv, getopts::optString, getopts::long_options, NULL);
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
                std::cout << "Source Package v 0.2" << std::endl;
                break;
            default:
                /* сюда на самом деле попасть невозможно. */
                break;
        }
        opt = getopt_long(argc, argv, getopts::optString, getopts::long_options, NULL);
    }
    if (getopts::longopts.isNoWarning == 1) cfg.isAllowWarning = false;
    if (getopts::longopts.isHelp == 1) {
        std::cout << "Использование: " << std::string(argv[0]) << " [КЛЮЧ]" << std::endl;
        std::cout << "Аргументы:" << std::endl;
        std::cout << "-d,  --daemon    Запускает демон" << std::endl;
        std::cout << "-c [config]      Использовать конфиг [config]" << std::endl;
        std::cout << "-s [sockfile]    Использовать UNIX сокет [sockfile]" << std::endl;
        std::cout << "-r [rootdir]     Изменяет root директорию" << std::endl;
        std::cout << "-v               Версия программы" << std::endl;
        std::cout << "--no-forking     Запрет daemontype=forking" << std::endl;
        std::cout << "--no-warning     Запрет вывода предупреждений" << std::endl;
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
        if (stat(cfg.rootdir.c_str(), &statbuff) != 0) std::cout << "[WARNING] rootdir " << cfg.rootdir << " not found" << std::endl;
        if (stat(cfg.packsdir.c_str(), &statbuff) != 0) std::cout << "[WARNING] packsdir " << cfg.packsdir << " not found" << std::endl;
    }
    if (cfg.isAutoinstall) {
        auto list = split(cfg.autoinstall, ':');
        std::cout << list.size();
        for (auto i = list.begin(); i != list.end(); ++i) {
            std::string pckname = (*i);
            Package* pck = Package::find(pckname);
            if (pck == nullptr) pck = Package::get(cfg.packsdir + pckname);
            if (pck == nullptr) {
                std::cerr << "package " << pckname << " not found" << std::endl;
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
    try {
        gsock = new Sock(cfg.sockfile);
        gsock->loop(&cmd_exec);
    } catch (socket_exception e) {
        std::cout << "[CRITICAL] An exception was thrown out. Information: " << e.what() << std::endl;
        perror("[CRITICAL] Failed reason:");
        exit(-1);
    }
    delete gsock;
    for (auto i = packs.begin(); i != packs.end(); ++i) {
        delete(*i);
    }
    return 0;
}
