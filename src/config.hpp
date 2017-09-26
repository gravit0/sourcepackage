#ifndef CONFIG_H
#define CONFIG_H
#include <string>
#define COPY_BUF_SIZE 8191 
#define SOCK_BUF_SIZE 1024
struct __Configuration
{
    std::string rootdir = "/";
    bool isSetRootdir = false;
    std::string packsdir = "/";
    bool isSetPackdir = false;
    std::string sockfile = "/run/sp";
    bool isSetSockfile = false;
    bool isDaemon = false;
};
extern __Configuration cfg;
#endif
