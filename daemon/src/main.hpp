#ifndef MAIN_H
#define MAIN_H
#include <vector>
#include <string>
#include <mutex>
#include <list>
#include "config.hpp"

struct FileAction {
    std::string filename;
    int mode;
    int group;
    int owner;

    enum ActionList {
        DIR,
        FILE,
        LINK
    };
    ActionList action;
};

class Package {
public:
    std::string name;
    int version_major;
    int version_minor;
    int version_build;
    std::string author;
    std::string dir;
    std::string daemonfile;
    std::string license;
    std::string logfile;
    std::vector<std::string> dependencies;
    std::list<FileAction> files;
    std::vector<Package*> dependencie;
    bool isInstalled;
    bool isDependence;
    bool isStartInstall;
    static const unsigned int flag_fakeInstall = 1 << 0;
    static const unsigned int flag_nodep = 1 << 0;
    void install(unsigned int flags=0);
    void remove_();
    static Package* find(std::string name);
    static Package* unload(std::string name);
    static Package* get(std::string dir);
    static std::mutex mutex;
};
class package_exception : public std::exception {
public:

    enum Errors {
        DependencieNotFound,
        ErrorParsePackage
        
    };
    Errors thiserr;
    package_exception(Errors err);
    virtual const char* what() const noexcept;
};
std::vector<std::string> split(const std::string cmd, const char splitchar);
extern std::list<Package*> packs;
#endif
