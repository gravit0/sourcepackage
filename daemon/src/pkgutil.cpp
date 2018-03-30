#include "main.hpp"
#include <fstream>
#include "util.hpp"
#include <boost/property_tree/ini_parser.hpp>
#include <iostream>
Package::ptr Package::find(const std::string& name) {
    auto i = packs.find(name);
    if(i == packs.end()) return nullptr;
    return i->second;
}

Package::ptr Package::unload(const std::string& name) {

    auto i = packs.find(name);
    packs.erase(i);
    return nullptr;
}

int Package::read_pack(const std::string dir, Package::ptr pack) {
    int state = 0;
    auto lam = [&,pack](std::string first,std::string last,std::string category,bool isSet) {
        if(isSet) {
            if(category == "package") state = 0;
            else if(category == "data") state = 1;
            else if(category == "advanced") state = 2;
            else state = -1;
        }
        if(state==1)
        {
            FileAction t;
            std::vector<std::string> v = split(last, ':');
            int offset= 0;
            if (v[0] == "l") t.action = FileAction::LINK;
            else if (v[0] == "f") t.action = FileAction::FILE;
            else if (v[0] == "d") t.action = FileAction::DIR;
            else if (v[0] == "L") {
                    t.action = FileAction::TARGETLINK;
                    t.target = v[1];
                    offset++;
            }
            else if (v[0] == "h") {
                    t.action = FileAction::HARDLINK;
                    t.target = v[1];
                    offset++;
            }
            else t.action = FileAction::FILE;
            int size=v.size();
            if (size >= offset+2) {
                t.mode = std::stoi(v[offset+1]);
            } else t.mode = -1;
            if (size >= offset+3) {
                t.group = std::stoi(v[offset+2]);
            } else t.group = -1;
            if (size >= offset+4) {
                t.owner = std::stoi(v[offset+3]);
            } else t.owner = -1;
            t.filename = first;
            pack->files.push_back(t);
        }
        else if(state == 2)
        {
            if(first == "isDaemon") pack->isDaemon = last == "true" ? true : false;
        }
        else if(state == 0)
        {
            if(first == "name") pack->name = last;
            else if(first == "author") pack->author = last;
            else if(first == "license") pack->license = last;
            else if(first == "dependencies") {
                if (!last.empty()) {
                    auto arr = split(last,':');
                    for(auto &i : arr)
                    {
                        Package_dependencie dep;
                        int pos = i.find('=');
                        if(pos<0) dep.name = i;
                        else {
                            dep.name = i.substr(0,pos);
                            std::string ver = i.substr(pos+1);
                            dep.version.parse(ver);
                        }
                        pack->dependencies.push_back(dep);
                    }
                }
            }
        }
    };
    int ret = RecArrUtils::ini_parser_lam(dir + "/package.ini",lam);
    pack->isInstalled = false;
    pack->isDependence = false;
    pack->isStartInstall = false;
    pack->dir = dir;
    return ret;
}

Package::ptr Package::get(const std::string dir) {
    Package::ptr pack = std::shared_ptr<Package>(new Package);
    if (Package::read_pack(dir, pack) != 0) {
            return nullptr;
    }
    packs[pack->name] = pack;
    return pack;
}

void Package::toIni(std::string dir) {
    RecursionArray arr, data, main;
    main.add("name", name);
    main.add("version", version.major);
    main.add("version_min", version.minor);
    main.add("build", version.build);
    main.add("author", author);
    main.add("license", license);
    for (auto &i : files) {
        std::string value;
        if (i.action == FileAction::FILE) value += "f";
        else if (i.action == FileAction::DIR) value += "d";
        else if (i.action == FileAction::LINK) value += "l";
        if (i.mode >= 0) {
            value += ":";
            value += i.mode;
        }
        data.push_back(RecursionArray::value_type(i.filename, RecursionArray(value)));
    }
    std::string dep;
    {
        bool isStart = false;
        for (auto &i : dependencies) {
            if (isStart) dep += ":";
            dep += i.name;
            isStart = true;
        }
    }
    main.add("dependencies", dep);
    arr.add_child("main", main);
    arr.add_child("data", data);
    boost::property_tree::ini_parser::write_ini(dir + "/package.ini", arr);
}
