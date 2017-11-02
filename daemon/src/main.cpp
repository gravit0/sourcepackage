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
std::list<Package*> packs;
Configuration cfg;
Logger * logger;
Sock* gsock;
const char* package_exception::what() const noexcept {
    switch (thiserr) {
        case DependencieNotFound: return "Dependencie Not Found";
        case ErrorParsePackage: return "Error parse package";
        case FileNotFound: return "File in the package was not found";
        default: return "Unknown Error";
    }
}
package_exception::package_exception(Errors err) {
    thiserr = err;
}
std::vector<std::string> split(const std::string& cmd, const char splitchar) {
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
int config_parse(const std::string& filename) {

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
        } else if (frist == "ignore_low_exception") {
            cfg.isIgnoreLowException = (last == "true") ? true : false;
        }
    }
    return 0;
}
void signal_handler(int sig) {
    if (gsock != nullptr) delete gsock;
    if (sig == SIGTERM) exit(0);
    exit(-sig);
}
int main(int argc, char** argv) {
    //Package* pck = Package::get("/home/gravit/packs/ld/");
    //return 0;
    logger = new Logger(Logger::LOG_STDERR);
    logger->logg('C',"test");
    gsock = nullptr;
    signal(SIGTERM, signal_handler);
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
            {
                std::cout << "Source Package 1.0.0-test" << std::endl;
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
        std::cout << "-d,  --daemon    start daemonn, listen socket" << std::endl;
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
        if (stat(cfg.rootdir.c_str(), &statbuff) != 0) logger->logg('W',"rootdir " + cfg.rootdir + " not found");
        if (stat(cfg.packsdir.c_str(), &statbuff) != 0) logger->logg('W',"packsdir " + cfg.packsdir + " not found");
    }
    if (cfg.isAutoinstall) {
        auto list = split(cfg.autoinstall, ':');
        for (auto i = list.begin(); i != list.end(); ++i) {
            std::string pckname = (*i);
            Package* pck = Package::find(pckname);
            if (pck == nullptr) pck = Package::get(cfg.packsdir + pckname);
            if (pck == nullptr) {
                logger->logg('E',"package " + pckname + " not found");
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
        gsock = new Sock(cfg.sockfile,cfg.max_connect+1);
        gsock->loop(&cmd_exec);
    } catch (socket_exception e) {
        logger->logg('C',"An exception was thrown out. Information: " + std::string(e.what()));
        perror("[C] Failed reason:");
        exit(-1);
    }
    delete gsock;
    for (auto i = packs.begin(); i != packs.end(); ++i) {
        delete(*i);
    }
    delete logger;
    return 0;
}
