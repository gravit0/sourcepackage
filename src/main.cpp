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
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
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
Package* unload_pack(std::string name)
{
    for(auto i = packs.begin();i!=packs.end();++i)
    {
        if((*i)->name == name) {
            packs.erase(i);
            delete (*i);
        }
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
        if((*i).action == 2) symlink((dir + (*i).filename).c_str(),(rootdir+filename).c_str());
        else if((*i).action == 1)
        {
            std::string buf;
            std::fstream f(dir+filename,std::ios_base::in | std::ios_base::binary);
            std::fstream f2(rootdir+filename,std::ios_base::out  | std::ios_base::binary);
            if (f) {
                // get length of file:
                f.seekg (0, f.end);
                int length = f.tellg();
                int buflength = length;
                if(length > 8191) buflength = 8191;
                f.seekg (0, f.beg);

                char * buffer = new char [buflength];
                while(length > 0) {
                // read data as a block:
                if(length < 8191) buflength = length;
                f.read (buffer,buflength);
                if (f)
                  f2.write(buffer,buflength);
                else
                  std::cout << "error: only " << f.gcount() << " could be read";
                // ...buffer contains the entire file...
                length = length - buflength;
                }
                delete[] buffer;
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
                else if(frist == "dependencies") pack->dependencies = parsecmd(last);
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
static const char *optString = "dvs:";
int main(int argc, char** argv)
{
//    if(argc < 2) {
//        std::cerr << "Use: ./init [fifo]";
//        //return 1;

//    }
    //symlink("/tmp/r","/tmp/s");
    int opt = getopt(argc,argv,optString);
    bool daemon = false;
    std::string sock_file = "/run/sp";
    while( opt != -1 ) {
            switch( opt ) {
                case 'd':
                    daemon = true;
                    break;
                case 's':
                    sock_file = std::string(optarg);
                    break;
                case 'v':
                    std::cout << "Source Package v 0.0.1" << std::endl;
                    break;
                default:
                    /* сюда на самом деле попасть невозможно. */
                    break;
            }
            opt = getopt( argc, argv, optString );
        }
    //Package* z = get_pack("/tmp/testpack");
    //std::cout << "Name: " << z->name << std::endl;
    //std::cout << "Version: " << z->version << std::endl;
    //std::cout << "Author: " << z->author << std::endl;
    //for(auto i = z->files.begin();i!=z->files.end();++i)
    //{
    //    std::string filename = (*i).filename;
    //    std::cout << (*i).action << " " << filename << std::endl;
    //}
    if(!daemon) return 0;
    struct sockaddr srvr_name, rcvr_name;
#define BUF_SIZE 1024
      char buf[BUF_SIZE];
      int   sock;
      unsigned int   namelen, bytes;

      sock = socket(AF_UNIX, SOCK_DGRAM, 0);
      if (sock < 0)
      {
        perror("socket failed");
        return EXIT_FAILURE;
      }
      srvr_name.sa_family = AF_UNIX;
      strcpy(srvr_name.sa_data, sock_file.c_str());
      if (bind(sock, &srvr_name, strlen(srvr_name.sa_data) +
            sizeof(srvr_name.sa_family)) < 0)
      {
        perror("bind failed");
        return EXIT_FAILURE;
      }
      while(1)
      {
            bytes = recvfrom(sock, buf, sizeof(buf),  0, &rcvr_name, &namelen);
            if (bytes < 0)
            {
                perror("recvfrom failed");
                return EXIT_FAILURE;
            }
            buf[bytes] = 0;
            rcvr_name.sa_data[namelen] = 0;
            printf("Client sent: %s\n", buf);
            std::string cmd(buf,bytes);
            std::vector<std::string> args = parsecmd(cmd);
            std::string basecmd = args[0];
            if(basecmd == "install")
            {
                std::string pckname = args[1];
                Package* pck = find_pack(pckname);
                if(pck == nullptr) pck = get_pack(packsdir+pckname);
                if(pck == nullptr) {
                    std::cerr << "package " << pckname << " not found";
                    goto ifend;
                }
                std::cerr << pck->files.size();
                pck->install();
            }
            else if(basecmd == "installu")
            {
                std::string pckname = args[1];
                Package* pck = find_pack(pckname);
                if(pck == nullptr) pck = get_pack(pckname);
                if(pck == nullptr) {
                    std::cerr << "package " << pckname << " not found";
                    goto ifend;
                }
                pck->install();
            }
            else if(basecmd == "remove")
            {
                std::string pckname = args[1];
                Package* pck = find_pack(pckname);
                if(pck != nullptr) pck->remove_();
                else std::cerr << "package " << pckname << " not found";
            }
            else if(basecmd == "load")
            {
                std::string pckdir = args[1];
                get_pack(pckdir);
            }
            else if(basecmd == "unload")
            {
                std::string pckdir = args[1];
                get_pack(pckdir);
            }
            else if(basecmd == "setroot")
            {
                std::string roottdir = args[1];
                rootdir = roottdir;
            }
            else if(basecmd == "setpckdir")
            {
                std::string packsdirr = args[1];
                packsdir = packsdirr;
            }
            else if(basecmd == "stop")
            {
                break;
            }
            ifend: ;
            }
        close(sock);
        unlink(sock_file.c_str());
        return 0;
}
