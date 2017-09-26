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
using namespace std;
std::list<Package*> packs;
__Configuration cfg;

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
static const char *optString = "divc:s:r:";

void config_parse(std::string filename) {
    std::fstream f(filename, std::ios_base::in);
    if(!f) {
        std::cerr << "Config " << filename <<" not found" << std::endl;
        return;
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
    }
}

int main(int argc, char** argv) {
    int opt = getopt(argc, argv, optString);
    cfg.isDaemon = false;
    std::string config_file = "/etc/sp.cfg";
    while (opt != -1) {
        switch (opt) {
            case 'd':
                cfg.isDaemon = true;
                break;
            case 's': {
                cfg.sockfile = std::string(optarg);
                cfg.isSetSockfile = true;
                break;
            }
            case 'i': {
                cfg.isAutoinstall = true;
                break;
            }
            case 'r': {
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
        opt = getopt(argc, argv, optString);
    }

    //Package* z = get_pack("/tmp/testpack");
    //std::cout << "Name: " << z->name << std::endl;
    //std::cout << "Version: " << z->version << std::endl;
    //std::cout << "Author: " << z->author << std::endl;
    //for(auto i = z->files.begin();i!=z->files.end();++i)
    //{
    //    std::string filename = (*i).filename;
    //    std::cout << (*i).action << " " << filename << std::endl;
    //}
    config_parse(config_file);
    if(cfg.isAutoinstall)
    {
        auto list = parsecmd(cfg.autoinstall);
        std::cout << list.size();
        for(auto i = list.begin();i!=list.end();++i)
        {
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
    struct sockaddr srvr_name, rcvr_name;
    char buf[SOCK_BUF_SIZE];
    int sock;
    unsigned int namelen, bytes;

    sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket failed");
        return EXIT_FAILURE;
    }
    srvr_name.sa_family = AF_UNIX;
    strcpy(srvr_name.sa_data, cfg.sockfile.c_str());
    if (bind(sock, &srvr_name, strlen(srvr_name.sa_data) +
            sizeof (srvr_name.sa_family)) < 0) {
        perror("bind failed");
        return EXIT_FAILURE;
    }
    while (1) {
        bytes = recvfrom(sock, buf, sizeof (buf), 0, &rcvr_name, &namelen);
        if (bytes < 0) {
            perror("recvfrom failed");
            return EXIT_FAILURE;
        }
        buf[bytes] = 0;
        rcvr_name.sa_data[namelen] = 0;
        printf("Client sent: %s\n", buf);
        std::string cmd(buf, bytes);
        std::vector<std::string> args = parsecmd(cmd);
        std::string basecmd = args[0];
        if (basecmd == "install") {
            std::string pckname = args[1];
            Package* pck = find_pack(pckname);
            if (pck == nullptr) pck = get_pack(cfg.packsdir + pckname);
            if (pck == nullptr) {
                std::cerr << "package " << pckname << " not found";
                goto ifend;
            }
            pck->install();
        } else if (basecmd == "installu") {
            std::string pckname = args[1];
            Package* pck = find_pack(pckname);
            if (pck == nullptr) pck = get_pack(pckname);
            if (pck == nullptr) {
                std::cerr << "package " << pckname << " not found";
                goto ifend;
            }
            pck->install();
        } else if (basecmd == "remove") {
            std::string pckname = args[1];
            Package* pck = find_pack(pckname);
            if (pck != nullptr) pck->remove_();
            else std::cerr << "package " << pckname << " not found";
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
        } else if (basecmd == "stop") {
            break;
        }
ifend:
        ;
    }
    close(sock);
    unlink(cfg.sockfile.c_str());
    return 0;
}
