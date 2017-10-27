/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   fswclient.cpp
 * Author: gravit
 *
 * Created on 23 октября 2017 г., 14:29
 */
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <getopt.h>
#include "basefunctions.h"
#include <vector>
#define SOCK_NAME sock_path
#define BUF_SIZE 256
std::vector<std::string> split(const std::string cmd,const char splitchar) {
    int opos = 0;
    std::vector<std::string> list;
    while (true) {
        bool tr = false;
        int pos = findNoSlash(cmd,splitchar,opos,&tr);
        if (pos <= 0) {
            std::string value = cmd.substr(opos, pos - cmd.size());
            if(tr) SlashReplace(&value,0);
            list.push_back(value);
            break;
        }
        std::string value = cmd.substr(opos, pos - opos);
        list.push_back(value);
        opos = pos + 1;
    }
    return list;
}
static const char *optString = "c:ui:r:sl:f:p";
struct Args{
    bool flagInstall = false;
    bool flagRemove = false;
    bool flagLoad = false;
    bool flagGetpack = false;
    bool flagStop = false;
    std::string pkgname;
    bool flagU = false;
};
int main(int argc, char ** argv)
{
    int opt = getopt(argc,argv,optString);
    char sock_path[BUF_SIZE];
    Args args;
    int init_path = 0;
    char buf[BUF_SIZE];
    buf[0] = 0;
    while( opt != -1 ) {
            switch( opt ) {
                case 'i': {
                    args.flagInstall = true;
                    args.pkgname = optarg;
                    break;
                    
                }
                case 'u': {
                    args.flagU = true;
                    break;
                    
                }
                case 'l': {
                    args.flagLoad = true;
                    args.pkgname = optarg;
                    break;
                    
                }
                case 'c': {
                    strcat(buf,"setconfig\t");
                    strcat(buf,optarg);
                    break;
                    
                }
                case 'r':{
                    args.flagRemove = true;
                    args.pkgname = optarg;
                    break;
                    
                }
                case 'p': {
                    args.flagGetpack = true;
                    break; 
                    
                }
                case 'f': {
                    strcpy(sock_path, optarg);
                    init_path = 1;
                    break; 
                    
                }
                case 's': {
                    args.flagStop = true;
                    break;
                }
                default:
                    /* сюда на самом деле попасть невозможно. */
                    break;
            }
            opt = getopt( argc, argv, optString );
        }
    //Проверка противоречий
    if((int) args.flagInstall + (int) args.flagRemove 
            + (int) args.flagLoad + (int) args.flagStop
            + (int) args.flagGetpack > 1) {
        std::cout << "Error request!";
    }
    if(args.flagInstall)
    {
        if(args.flagU) strcat(buf,"installu\t");
        else strcat(buf,"install\t");
        strcat(buf,args.pkgname.c_str());
    }
    else if(args.flagRemove)
    {
        strcat(buf,"remove\t");
        strcat(buf,args.pkgname.c_str());
    }
    else if(args.flagLoad)
    {
        if(args.flagU) strcat(buf,"unload\t");
        else strcat(buf,"load\t");
        strcat(buf,args.pkgname.c_str());
    }
    else if(args.flagGetpack)
    {
        strcat(buf,"getpacks\t");
        strcat(buf,args.pkgname.c_str());
    }
    else if(args.flagStop)
    {
        strcat(buf,"stop\t");
    }
  int   sock;
  sock = socket(AF_UNIX, SOCK_STREAM, 0);
  struct sockaddr srvr_name;
  if (sock < 0) 
  {
    perror("socket failed");
    return EXIT_FAILURE;
  }
  srvr_name.sa_family = AF_UNIX;
  if(init_path) strcpy(srvr_name.sa_data, SOCK_NAME);
  else strcpy(srvr_name.sa_data, "/run/sp");
  if(connect(sock, &srvr_name, sizeof(srvr_name)) < 0)
  {
     perror("connect failed");
     exit(2);
  }
  send(sock, buf, strlen(buf), 0);
  buf[0] = '\0';
  recv(sock,buf,sizeof(buf),0);
  if(buf[0] != '\0'){
      std::string cmd(buf);
      std::vector<std::string> args = split(cmd,'\t');
      if(args[0] == "0") goto sockclose;
      else std::cout << cmd;
  }
  sockclose:
  close(sock);
  return 0;
}
