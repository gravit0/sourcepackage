#include "main.hpp"
#include <fstream>
#include "util.hpp"
#include <boost/property_tree/ini_parser.hpp>

Package* Package::find(std::string name) {
    for (auto i = packs.begin(); i != packs.end(); ++i) {
        if ((*i)->name == name) return (*i);
    }
    return nullptr;
}

Package* Package::unload(std::string name) {
    
    for (auto i = packs.begin(); i != packs.end(); ++i) {
        if ((*i)->name == name) {
            packs.erase(i);
            delete (*i);
        }
    }
    
    return nullptr;
}

Package* Package::get(std::string dir) {
    
    RecursionArray arr;
    try {
        boost::property_tree::ini_parser::read_ini(dir + "/package.ini",arr);
    } catch(boost::property_tree::ini_parser_error err)
    {
        return nullptr;
    }
    Package* pack = new Package();
    pack->isInstalled = false;
    pack->isDependence = false;
    pack->isStartInstall = false;
    std::string info;
    std::list<FileAction> files;
    std::string category;
    std::string name;
    const RecursionArray main = arr.get_child("package");
    pack->name = main.get<std::string>("name","");
    pack->version_major = main.get<int>("version",1);
    pack->version_minor = main.get<int>("version_min",0);
    pack->version_build = main.get<int>("build",0);
    const RecursionArray filesarr = arr.get_child("data");
    for(auto &i : filesarr)
    {
        const std::string value= i.second.get<std::string>("");
        FileAction t;
        std::vector<std::string> v = split(value,':');
        if(v[0] == "l") t.action = FileAction::LINK;
        else if(v[0] == "f") t.action = FileAction::FILE;
        else if(v[0] == "d") t.action = FileAction::DIR;
        else t.action = FileAction::FILE;
        t.filename = i.first;
        files.push_back(t);
    }
    std::string dep = main.get<std::string>("dependencies","");
    if(!dep.empty()) pack->dependencies = split(dep,':');
    pack->files = files;
    pack->dir = dir;
    packs.push_back(pack);
    
    return pack;
}
