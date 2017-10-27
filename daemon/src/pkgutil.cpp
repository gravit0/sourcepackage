#include "main.hpp"
#include "pkgutil.hpp"
#include <fstream>
Package* Package::find(std::string name)
{
    for(auto i = packs.begin();i!=packs.end();++i)
    {
        if((*i)->name == name) return (*i);
    }
    return nullptr;
}
Package* Package::unload(std::string name)
{
    Package::mutex.lock();
    for(auto i = packs.begin();i!=packs.end();++i)
    {
        if((*i)->name == name) {
            packs.erase(i);
            delete (*i);
        }
    }
    Package::mutex.unlock();
    return nullptr;
}
Package* Package::get(std::string dir)
{
    Package::mutex.lock();
    std::fstream f;
    f.open(dir + "/config.cfg",std::ios_base::in);
    if(!f.fail())
    {
        Package* pack = new Package();
        pack->isInstalled = false;
        pack->isDependence = false;
        pack->isStartInstall = false;
        std::string info;
        std::list<FileAction> files;
        std::string category;
        std::string name;
        int state = 0;
        while(std::getline(f,info))
        {
            if(info.size()<=1) continue;
            if(info[0] == '[')
            {
                if(state == 0) {
                    name=info.substr(1,info.size() - 2);
                    state = 1; }
                else { category=info.substr(1,info.size() - 2);  state = 2;}
                continue;
            }
            if(state == 1) {
                int pos = info.find('=');
                if(pos < 0) continue;
                std::string frist = info.substr(0,pos);
                std::string last = info.substr(pos + 1,info.size());
                if(frist == "version") pack->version = last;
                else if(frist == "creator") pack->author = last;
                else if(frist == "daemonfile") pack->daemonfile = last;
                else if(frist == "logfile") pack->logfile = last;
                else if(frist == "creator") pack->author = last;
                else if(frist == "dependencies") pack->dependencies = split(last,':');
                continue;
            }
            if(state == 2) {
                FileAction t;
                if(category == "cp") t.action = 1;
                else if(category == "ln") t.action = 2;
                else if(category == "dir") t.action = 3;
                t.filename = info;
                files.push_back(t);
            }
        }
        pack->name = name;
        pack->files = files;
        pack->dir = dir;
        packs.push_back(pack);
        return pack;
    }
    Package::mutex.unlock();
    return nullptr;
}
