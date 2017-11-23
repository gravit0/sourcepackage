#include "main.hpp"
#include <fstream>
#include "util.hpp"
#include <boost/property_tree/ini_parser.hpp>

Package* Package::find(const std::string& name) {
    for (auto i = packs.begin(); i != packs.end(); ++i) {
        if ((*i)->name == name) return (*i);
    }
    return nullptr;
}

Package* Package::unload(const std::string& name) {

    for (auto i = packs.begin(); i != packs.end(); ++i) {
        if ((*i)->name == name) {
            packs.erase(i);
            delete (*i);
        }
    }

    return nullptr;
}

int Package::read_pack(const std::string& dir, Package* pack) {
    RecursionArray arr;
    try {
        boost::property_tree::ini_parser::read_ini(dir + "/package.ini", arr);
    } catch (boost::property_tree::ini_parser::ini_parser_error err) {
        return 1;
    }
    pack->isInstalled = false;
    pack->isDependence = false;
    pack->isStartInstall = false;
    std::string info;
    std::list<FileAction> files;
    std::string category;
    std::string name;
    const RecursionArray main = arr.get_child("package");
    pack->name = main.get<std::string>("name", "");
    pack->author = main.get<std::string>("author", "");
    pack->license = main.get<std::string>("license", "");
    pack->version.major = main.get<int>("version", 1);
    pack->version.minor = main.get<int>("mversion", 0);
    pack->version.build = main.get<int>("build", 0);
    const RecursionArray filesarr = arr.get_child("data");
    for (auto &i : filesarr) {
        const std::string value = i.second.get<std::string>("");
        FileAction t;
        std::vector<std::string> v = split(value, ':');
        if (v[0] == "l") t.action = FileAction::LINK;
        else if (v[0] == "f") t.action = FileAction::FILE;
        else if (v[0] == "d") t.action = FileAction::DIR;
        else t.action = FileAction::FILE;
        if (v.size() >= 2) {
            t.mode = std::stoi(v[1]);
        } else t.mode = -1;
        if (v.size() >= 3) {
            t.group = std::stoi(v[2]);
        } else t.group = -1;
        if (v.size() >= 4) {
            t.owner = std::stoi(v[3]);
        } else t.owner = -1;
        t.filename = i.first;
        files.push_back(t);
    }
    const RecursionArray advanced = arr.get_child("advanced", RecursionArray());
    if (!advanced.empty()) {
        pack->isDaemon = advanced.get<std::string>("isDaemon", "false") == "true" ? true : false;
    }
    std::string dep = main.get<std::string>("dependencies", "");
    if (!dep.empty()) pack->dependencies = split(dep, ':');
    pack->files = files;
    pack->dir = dir;
    return 0;
}

Package* Package::get(const std::string& dir) {
    Package* pack = new Package();
    if (Package::read_pack(dir, pack) != 0) return nullptr;
    packs.push_back(pack);
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
            dep += i;
            isStart = true;
        }
    }
    main.add("dependencies", dep);
    arr.add_child("main", main);
    arr.add_child("data", data);
    boost::property_tree::ini_parser::write_ini(dir + "/package.ini", arr);
}