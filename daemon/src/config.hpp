#ifndef CONFIG_H
#define CONFIG_H
#include <string>

#include "Logger.hpp"
#define COPY_BUF_SIZE 131072
#define SOCK_BUF_SIZE 1024

struct _SecurityInfo {
    int pid;
    int ppid;
    int uid, euid;
    int gid, egid;
};

struct Configuration {
    std::string rootdir = "/";
    std::string packsdir = "/";
    std::string sockfile = "/run/sp";
    std::string pidfile = "/run/sp.pid";
    std::string autoinstall = "";
    _SecurityInfo security;
    int epoll_timeout = 1000;
    int max_connect = 10;
    Logger::level log_level;

    enum DAEMON_TYPE {
        CFG_DAEMON_SIMPLE,
        CFG_DAEMON_FORKING
    };

    enum SETUID_MODES {
        CFG_SETUID_NONE,
        CFG_SETUID_SUID,
        CFG_SETUID_CHANGE,
        CFG_SETUID_USERMODE
    };
    SETUID_MODES setuid_mode = CFG_SETUID_NONE;
    DAEMON_TYPE daemon_type = CFG_DAEMON_SIMPLE;
    bool isSetRootdir = false;
    bool isSetPackdir = false;
    bool isSetSockfile = false;
    bool isDaemon = false;
    bool isAutoinstall = false;
    bool reinstall_socket = true;
    bool isAllowWarning = true;
    bool isIgnoreLowException = false;
};
extern Configuration cfg;
#endif
