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
                Package* dep = Package::find((*i).name);
                if (dep == nullptr) dep = Package::get(cfg.packsdir + (*i).name);
                if (dep == nullptr) {
                    throw package_exception(package_exception::DependencieNotFound);
                    isStartInstall = false;
                    return;
                }
                dep->install();
                dep->isDependence = true;
                dep->dependencie.push_back(this);
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
            if(i.action == FileAction::DIR || i.action == FileAction::FILE) statresult = lstat(pckfile_c, &statbuff);
            if (!cfg.isIgnoreLowException && statresult < 0) {
                throw package_exception(package_exception::FileNotFound);
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
                if (S_ISLNK(statbuff.st_mode)) {
                    char lbuff[SYMLINK_BUF_SIZE];
                    int len;
                    if ((len = readlink(pckfile_c, lbuff, sizeof (lbuff) - 1)) != -1) {
                        lbuff[len] = '\0';
                        symlink(lbuff, targetfile_c);
                    }
                    continue;
                }
                std::string buf;
                std::fstream f(pckfile, std::ios_base::in | std::ios_base::binary);
                std::fstream f2(targetfile, std::ios_base::out | std::ios_base::binary);
                if (f) {
                    // get length of file:
                    f.seekg(0, f.end);
                    int length = f.tellg();
                    int buflength = length;
                    if (length > COPY_BUF_SIZE) buflength = COPY_BUF_SIZE;
                    f.seekg(0, f.beg);

                    char * buffer = new char [buflength];
                    while (length > 0) {
                        if (length < COPY_BUF_SIZE) buflength = length;
                        f.read(buffer, buflength);
                        if (f)
                            f2.write(buffer, buflength);
                        else
                            std::cout << "error: only " << f.gcount() << " could be read";
                        length = length - buflength;
                    }
                    delete[] buffer;
                }
                f.close();
                f2.close();
                chmod(targetfile_c, filemode);
                auto uid = statbuff.st_uid;
                auto gid = statbuff.st_gid;
                if (i.owner >= 0) uid = i.owner;
                if (i.group >= 0) gid = i.group;
                chown(targetfile_c, uid, gid);
            }
        }
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
            Package* dep = Package::find((*i).name);
            if (dep == nullptr) dep = Package::get((*i).name);
            if (dep->isDependence) {
                for (auto j = dep->dependencie.begin(); j != dep->dependencie.end(); ++j) {
                    if ((*j) == this) {
                        dep->dependencie.erase(j);
                        break;
                    };
                }
                if (dep->dependencie.size() == 0) {
                    dep->remove();
                }
                //dep->dependencie.erase(std::find<std::list<Package*>::iterator,Package*>(dep->dependencie.begin(),dep->dependencie.end(),this));
            }
        }
    }

}
