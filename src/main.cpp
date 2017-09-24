#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <fcntl.h>           /* Definition of AT_* constants */
#include <unistd.h>
#include "main.hpp"
#include <string.h>
#include <list>
#pragma once
using namespace std;
std::list<Package*> packs;
std::string packsdir;
std::string rootdir;
vector<string> parsecmd(std::string cmd)
{
    int opos = 0;
    std::vector<std::string> list;
    while(true)
    {
        int pos = cmd.find(':',opos);
        if(pos <= 0)
        {
            std::string value = cmd.substr(opos,pos - cmd.size());
            list.push_back(value);
            break;
        }
        std::string value = cmd.substr(opos,pos - opos);
        list.push_back(value);
        opos = pos + 1;
    }
    return list;
}
Package* find_pack(std::string name)
{
    for(auto i = packs.begin();i!=packs.end();++i)
    {
        if((*i)->name == name) return (*i);
    }
    return nullptr;
}
void Package::install()
{
    if(isStartInstall || isInstalled) return;
    else isStartInstall = true;
    if(!dependencies.empty())
    {
        for(auto i = dependencies.begin();i!=dependencies.end();++i)
        {
            Package* dep = find_pack(*i);
            if(dep == nullptr) dep = get_pack(*i);
            dep->install();
            dep->isDependence = true;
            dep->dependencie.push_back(this);
        }
    }
    for(auto i = files.begin();i!=files.end();++i)
    {
        std::string filename = (*i).filename;
        if((*i).action == 2) symlink((rootdir+filename).c_str(),(dir + (*i).filename).c_str());
        else if((*i).action == 1)
        {
            std::string buf;
            std::fstream f(rootdir+filename,std::ios_base::in | std::ios_base::binary);
            std::fstream f2(dir+filename,std::ios_base::out  | std::ios_base::binary);
            while(!f.eof())
            {
                f >> buf;
                f2 << buf;
            }
            f.close();
            f2.close();
        }
    }
    isInstalled = true;
}
void Package::remove_()
{
    for(auto i = files.begin();i!=files.end();++i)
    {
        remove((rootdir+(*i).filename).c_str());
    }
    isInstalled = false;
    if(!dependencies.empty())
    {
        for(auto i = dependencies.begin();i!=dependencies.end();++i)
        {
            Package* dep = find_pack(*i);
            if(dep == nullptr) dep = get_pack(*i);
            if(dep->isDependence)
            {
                //dep->dependencie.erase(std::find<std::list<Package*>::iterator,Package*>(dep->dependencie.begin(),dep->dependencie.end(),this));
            }
        }
    }
}
//Package* get_pack(std::string dir)
//{
//    std::fstream f;
//    f.open(dir + "/config.cfg",std::ios_base::in);
//    if(!f.fail())
//    {
//        Package* pack = new Package();
//        pack->isInstalled = false;
//        pack->isDependence = false;
//        pack->isStartInstall = false;
//        std::string info;
//        std::getline(f,info);
//        int pos = info.find(':');
//        std::string name = info.substr(0,pos);
//        std::string dep = info.substr(pos + 1,info.size());
//        pack->name = name;
//        std::list<FileAction> files;
//        while(std::getline(f,info))
//        {
//            FileAction t;
//            int pos2 = info.find(' ');
//            std::string act = info.substr(0,pos2);
//            std::string name = info.substr(pos2 + 1,info.size());
//            if(act == "cp") t.action = 1;
//            else if(act == "ln") t.action = 2;
//            t.filename = name;
//            files.push_back(t);
//        }
//        pack->files = files;
//        pack->dir = dir;
//        packs.push_back(pack);
//        return pack;
//    }
//    return nullptr;
//}
Package* get_pack(std::string dir)
{
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
                continue;
            }
            if(state == 2) {
                FileAction t;
                if(category == "cp") t.action = 1;
                else if(category == "ln") t.action = 2;
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
    return nullptr;
}
int main(int argc, char** argv)
{
//    if(argc < 2) {
//        std::cerr << "Use: ./init [fifo]";
//        //return 1;

//    }
    //symlink("/tmp/r","/tmp/s");
    Package* z = get_pack("/tmp/testpack");
    std::cout << "Name: " << z->name << std::endl;
    std::cout << "Version: " << z->version << std::endl;
    std::cout << "Author: " << z->author << std::endl;
    for(auto i = z->files.begin();i!=z->files.end();++i)
    {
        std::string filename = (*i).filename;
        std::cout << (*i).action << " " << filename << std::endl;
    }
    return 0;
    std::fstream f("/tmp/f",std::ios_base::in) ;
    std::string cmd;
    while(true) {
        f >> cmd;
        std::vector<std::string> args = parsecmd(cmd);
        std::cerr << args.size();
        for(size_t i=0;i<args.size();i++)
        {
            std::cout << args.at(i) << std::endl;
        }
        std::string basecmd = args[0];
        if(basecmd == "install")
        {
            std::string pckname = args[1];
            Package* pck = find_pack(pckname);
            if(pck == nullptr) pck = get_pack(packsdir+pckname);
            pck->install();
        }
        else if(basecmd == "remove")
        {
            std::string pckname = args[1];
            Package* pck = find_pack(pckname);
            if(pck != nullptr) pck->remove_();
        }
        else if(basecmd == "load")
        {
            std::string pckdir = args[1];
            get_pack(pckdir);
        }
        else if(basecmd == "setroot")
        {
            std::string roottdir = args[1];
            rootdir = roottdir;
        }
    }
}
