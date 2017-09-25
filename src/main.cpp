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
#include "pkgutil.hpp"
using namespace std;
std::list<Package*> packs;
std::string packsdir;
std::string rootdir;
std::vector<std::string> parsecmd(std::string cmd)
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
static const char *optString = "dvs:";
int main(int argc, char** argv)
{
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
      char buf[SOCK_BUF_SIZE];
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
