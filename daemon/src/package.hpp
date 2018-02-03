/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   package.hpp
 * Author: gravit
 *
 * Created on 12 ноября 2017 г., 13:55
 */

#ifndef PACKAGE_HPP
#define PACKAGE_HPP
#include <string>
#include <list>
#include <exception>

struct FileAction {
    std::string filename,target;
    int mode;
    int group;
    int owner;

    enum ActionList {
        DIR,
        FILE,
        LINK,
        HARDLINK,
        TARGETLINK
    };
    ActionList action;
};

struct Package_Version {
    int major;
    int minor;
    int build;
    bool operator>(Package_Version ver);
    int parse(std::string str);
};
struct Package_dependencie
{
    std::string name;
    Package_Version version;
    enum class Type
    {
        EQUAL,
        UP,
        DOWN,
        NONE
    };
    Type version_operator = Type::NONE;
};
class Package : public boost::noncopyable {
private:
    Package() = default;
public:
    std::string name;
    std::string author;
    std::string dir;
    std::string license;
    std::vector<Package_dependencie> dependencies;
    std::list<FileAction> files;
    std::vector<Package*> dependencie;
    Package_Version version;
    bool isInstalled;
    bool isDependence;
    bool isStartInstall;
    bool isDaemon;
    static const unsigned int flag_update = 1 << 2;
    static const unsigned int flag_fakeInstall = 1 << 1;
    static const unsigned int flag_nodep = 1 << 0;
    void install(unsigned int flags = 0);
    void remove();
    void toIni(std::string dir);
    void clear();
    static Package* find(const std::string& name);
    static Package* unload(const std::string& name);
    static int read_pack(const std::string dir, Package* pack);
    static Package* get(const std::string dir);
    static std::mutex mutex;
};

class package_exception : public std::exception {
public:

    enum Errors {
        DependencieNotFound,
        ErrorParsePackage,
        FileNotFound,
        DependencieBadVersion
    };
    Errors thiserr;
    std::string pkg;
    package_exception(Errors err);
    package_exception(Errors err,const std::string& pkg);
    virtual const char* what() const noexcept;
};

#endif /* PACKAGE_HPP */

