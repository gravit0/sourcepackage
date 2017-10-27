#ifndef MAIN_H
#define MAIN_H
#include <vector>
#include <string>
#include <mutex>
#include <list>
#include "config.hpp"
struct FileAction
{
    std::string filename;
    int action;
};
class Package
{
public:
    std::string name;
    std::string version;
    std::string author;
    std::string dir;
    std::string daemonfile;
    std::string logfile;
    std::vector<std::string> dependencies;
    std::list<FileAction> files;
    std::vector<Package*> dependencie;
    bool isInstalled;
    bool isDependence;
    bool isStartInstall;
    void install();
    void fakeinstall();
    void remove_();
    static Package* find(std::string name);
    static Package* unload(std::string name);
    static Package* get(std::string dir);
    static std::mutex mutex;
};
std::vector<std::string> split(const std::string cmd,const char splitchar);
extern std::list<Package*> packs;
#endif
