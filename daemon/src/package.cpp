#include "main.hpp"
#include <unistd.h>
#include <fcntl.h>           /* Definition of AT_* constants */
#include <sys/stat.h>
#include <fstream>
#include <iostream>
std::mutex Package::mutex;

const char* package_exception::what() const noexcept {
    switch (thiserr) {
        case DependencieNotFound: return "Dependencie Not Found";
        case DependencieBadVersion: return "Dependencie Bad Version";
        case ErrorParsePackage: return "Error parse package";
        case FileNotFound: return "File in the package was not found";
        default: return "Unknown Error";
    }
}

package_exception::package_exception(Errors err) {
    thiserr = err;
}
package_exception::package_exception(Errors err,const std::string& pkg)
{
    thiserr = err;
    this->pkg = pkg;
}
bool Package_Version::operator>(Package_Version ver) {
    if (major > ver.major) return true;
    if (minor > ver.minor) return true;
    if (build > ver.build) return true;
    return false;
}

int Package_Version::parse(std::string str) {
    const char* c_str = str.c_str();
    int pos = str.find('.');
    major = std::stoi(std::string(c_str,pos));
    int pos2 = str.find('.',pos+1);
    minor = std::stoi(std::string(c_str+pos,pos-pos2));
    int pos3 = str.find('-',pos2+1);
    build = std::stoi(std::string(c_str+pos2,pos2-pos3));
    return 0;
}

void Package::install(unsigned int flags) {

    if (isStartInstall || isInstalled) return;
    else isStartInstall = true;
    if (!dependencies.empty()) {
        if (!(flags & flag_nodep))
            for (auto i = dependencies.begin(); i != dependencies.end(); ++i) {
                Package::ptr dep = Package::find((*i).name);
                if (dep == nullptr) dep = Package::get(cfg.packsdir + (*i).name);
                if (dep == nullptr) {
                    isStartInstall = false;
                    throw new package_exception(package_exception::DependencieNotFound,(*i).name);
                    return;
                }
                dep->install();
                dep->isDependence = true;
                dep->dependencie.push_back(dep);
            }
    }
    if (!(flags & Package::flag_fakeInstall))
        for (auto& i : files) {
            std::string filename = i.filename;
            std::string pckfile = (dir + filename);
            const char* pckfile_c = pckfile.c_str();
            std::string targetfile = (cfg.rootdir + filename);
            const char* targetfile_c = targetfile.c_str();
            struct stat statbuff;
            int statresult = 0;
            if(i.action == FileAction::DIR || i.action == FileAction::FILE) statresult = stat(pckfile_c, &statbuff);
            if (!cfg.isIgnoreLowException && statresult < 0) {
                    std::cerr << "FILE " << pckfile << " FSTAT RETURN CODE " << statresult << std::endl;
                    perror("[C]");
                //throw package_exception(package_exception::FileNotFound);
                continue;
            }
            auto filemode = statbuff.st_mode;
            if (i.mode >= 0) filemode = i.mode;
            if (i.action == FileAction::LINK) symlink(pckfile_c, targetfile_c);
            else if (i.action == FileAction::TARGETLINK) symlink(i.target.c_str(), targetfile_c);
            else if (i.action == FileAction::HARDLINK) link(i.target.c_str(), targetfile_c);
            else if (i.action == FileAction::DIR) {
                mkdir(targetfile_c, filemode);
                auto uid = statbuff.st_uid;
                auto gid = statbuff.st_gid;
                if (i.owner >= 0) uid = i.owner;
                if (i.group >= 0) gid = i.group;
                if (i.owner >= 0 || i.group > 0) chown(targetfile_c, uid, gid);
            } else if (i.action == FileAction::FILE) {
                int f1 = open(pckfile_c,O_RDONLY);
                int f2 = open(targetfile_c,O_WRONLY | O_CREAT,filemode);
                if(f2 < 0)
                {
                    std::cerr << "FILE " << pckfile << " OPEN RETURN CODE " << statresult << std::endl;
                    perror("[C]");
                    break;
                }
                char* buf = new char[COPY_BUF_SIZE+1];
                int writable = 0;
                while(true)
                {
                    writable = read(f1,buf,COPY_BUF_SIZE);
                    if(writable > 0) {
                            int writeresult = write(f2,buf,writable);
                            if(writeresult < 0)
                            {
                                std::cerr << "FILE " << pckfile << " WRITE RETURN CODE " << statresult << std::endl;
                                perror("[C]");
                                break;
                            }
                    }
                    else if(writable == 0) break;
                    else {
                        std::cerr << "FILE " << pckfile << " READ RETURN CODE " << writable << std::endl;
                        perror("[C]");
                        break;
                    }
                }
                close(f1);
                close(f2);
                delete[] buf;
                auto uid = statbuff.st_uid;
                auto gid = statbuff.st_gid;
                if (i.owner >= 0) uid = i.owner;
                if (i.group >= 0) gid = i.group;
                chown(targetfile_c, uid, gid);
            }
        }
    else std::cerr << "Fake install" << std::endl;
    isInstalled = true;
    isStartInstall = false;

}

void Package::clear() {
    this->files.clear();
    this->dependencie.clear();
    this->dependencies.clear();
    this->name.clear();
    this->license.clear();
}

void Package::remove() {

    if (!isInstalled) return;
    for (auto i = files.begin(); i != files.end(); ++i) {
        ::remove((cfg.rootdir + (*i).filename).c_str());
    }
    isInstalled = false;
    isDependence = false;
    if (!dependencies.empty()) {
        for (auto i = dependencies.begin(); i != dependencies.end(); ++i) {
            Package::ptr dep = Package::find((*i).name);
            if (dep == nullptr) dep = Package::get((*i).name);
            if (dep->isDependence) {
                for (auto j = dep->dependencie.begin(); j != dep->dependencie.end(); ++j) {
                    if ((*j).get() == this) {
                        dep->dependencie.erase(j);
                        break;
                    };
                }
                if (dep->dependencie.size() == 0) {
                    dep->remove();
                }
                //dep->dependencie.erase(std::find<std::list<Package::ptr>::iterator,Package::ptr>(dep->dependencie.begin(),dep->dependencie.end(),this));
            }
        }
    }

}
