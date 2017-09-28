#ifndef MAIN_H
#define MAIN_H
#include <vector>
#include <string>
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
    std::vector<std::string> dependencies;
    std::list<FileAction> files;
    std::vector<Package*> dependencie;
    bool isInstalled;
    bool isDependence;
    bool isStartInstall;
    void install();
    void remove_();
};
std::vector<std::string> parsecmd(std::string cmd);
extern std::list<Package*> packs;
#endif
