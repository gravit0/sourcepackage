#ifndef CONFIG_H
#define CONFIG_H
#include <string>
#define COPY_BUF_SIZE 8191 
#define SOCK_BUF_SIZE 1024

struct Configuration {
    std::string rootdir = "/";
    bool isSetRootdir = false;
    std::string packsdir = "/";
    bool isSetPackdir = false;
    std::string sockfile = "/run/sp";
    std::string autoinstall = "";
    bool isSetSockfile = false;
    bool isDaemon = false;
    bool isAutoinstall = false;
    bool reinstall_socket = true;
    bool isAllowWarning = false;
    
    enum DAEMON_TYPE {
        CFG_DAEMON_SIMPLE,
        CFG_DAEMON_FORKING
    };
    DAEMON_TYPE daemon_type = CFG_DAEMON_SIMPLE;

};
extern Configuration cfg;
#endif
