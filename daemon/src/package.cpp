#include "main.hpp"
#include <unistd.h>
#include <fcntl.h>           /* Definition of AT_* constants */
#include <sys/stat.h>
#include <fstream>
#include <iostream>
std::mutex Package::mutex;
void Package::install()
{
    
    if(isStartInstall || isInstalled) return;
    else isStartInstall = true;
    if(!dependencies.empty())
    {
        for(auto i = dependencies.begin();i!=dependencies.end();++i)
        {
            Package* dep = Package::find(*i);
            if(dep == nullptr) dep = Package::get(cfg.packsdir + (*i));
            if(dep == nullptr) 
            {
                std::cerr << "Package " << (*i) << "not found(dep)" << std::endl;
                isStartInstall = false;
                return;
            }
            dep->install();
            dep->isDependence = true;
            dep->dependencie.push_back(this);
        }
    }
    for(auto i = files.begin();i!=files.end();++i)
    {
        std::string filename = (*i).filename;
        std::string pckfile = (dir + filename);
        struct stat statbuff;
        lstat(pckfile.c_str(), &statbuff);
        if((*i).action == FileAction::LINK) symlink(pckfile.c_str(),(cfg.rootdir+filename).c_str());
        else if((*i).action == FileAction::DIR) mkdir((cfg.rootdir+filename).c_str(),0755);
        else if((*i).action == FileAction::FILE)
        {
            if(S_ISLNK(statbuff.st_mode))
            {
                char lbuff[1024];
                int len;
                if ((len = readlink((dir + filename).c_str(), lbuff, sizeof(lbuff)-1)) != -1) {
                    lbuff[len] = '\0';
                    symlink(lbuff,(cfg.rootdir+filename).c_str());
                }
                continue;
            }
            std::string buf;
            std::fstream f(dir+filename,std::ios_base::in | std::ios_base::binary);
            std::fstream f2(cfg.rootdir+filename,std::ios_base::out  | std::ios_base::binary);
            if (f) {
                // get length of file:
                f.seekg (0, f.end);
                int length = f.tellg();
                int buflength = length;
                if(length > COPY_BUF_SIZE) buflength = COPY_BUF_SIZE;
                f.seekg (0, f.beg);

                char * buffer = new char [buflength];
                while(length > 0) {
                // read data as a block:
                if(length < COPY_BUF_SIZE) buflength = length;
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
            auto mode = statbuff.st_mode;
            chmod((cfg.rootdir+filename).c_str(),mode);
        }
    }
    isInstalled = true;
    isStartInstall = false;
    
}
void Package::fakeinstall()
{
    
    if(isStartInstall || isInstalled) return;
    else isStartInstall = true;
    if(!dependencies.empty())
    {
        for(auto i = dependencies.begin();i!=dependencies.end();++i)
        {
            Package* dep = Package::find(*i);
            if(dep == nullptr) dep = Package::get(cfg.packsdir + (*i));
            if(dep == nullptr) 
            {
                std::cerr << "Package " << (*i) << "not found(dep)" << std::endl;
                isStartInstall = false;
                return;
            }
            dep->fakeinstall();
            dep->isDependence = true;
            dep->dependencie.push_back(this);
        }
    }
    isInstalled = true;
    isStartInstall = false;
    
}
void Package::remove_()
{
    
    if(!isInstalled) return;
    for(auto i = files.begin();i!=files.end();++i)
    {
        remove((cfg.rootdir+(*i).filename).c_str());
    }
    isInstalled = false;
    isDependence = false;
    if(!dependencies.empty())
    {
        for(auto i = dependencies.begin();i!=dependencies.end();++i)
        {
            Package* dep = Package::find(*i);
            if(dep == nullptr) dep = Package::get(*i);
            if(dep->isDependence)
            {
                for(auto j = dep->dependencie.begin();j!=dep->dependencie.end();++j)
                {
                    if((*j) == this) {
                        dep->dependencie.erase(j);
                        break;
                    };
                }
                if(dep->dependencie.size() == 0) {
                    dep->remove_();
                }
                //dep->dependencie.erase(std::find<std::list<Package*>::iterator,Package*>(dep->dependencie.begin(),dep->dependencie.end(),this));
            }
        }
    }
    
}
