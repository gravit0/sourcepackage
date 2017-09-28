/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Sock.cpp
 * Author: gravit
 * 
 * Created on 28 сентября 2017 г., 16:46
 */

#include "Sock.hpp"
#include <iostream>
#include <unistd.h>
#include "config.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
Sock::Sock(std::string filepath) {
    sock_ = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_ < 0) {
        perror("socket failed");
        //throw new std::exception("socket failed");
    }
    srvr_name.sa_family = AF_UNIX;
    filename_c = filepath.c_str();
    strcpy(srvr_name.sa_data, filename_c);
    if (bind(sock_, &srvr_name, strlen(srvr_name.sa_data) +
            sizeof (srvr_name.sa_family)) < 0) {
        perror("bind failed");
    }
    listen(sock_, 1);
}
void Sock::loop(void (*lpfunc)(std::string,Sock*)){
    while (1) {
        rsock = accept(sock_,NULL,NULL);
        if(rsock < 0)
        {
            perror("accept");
            exit(3);
        }
        bytes = recv(rsock, buf, sizeof (buf),0);
        if (bytes < 0) {
            perror("recv failed");
            close(rsock);
            continue;
        }
        buf[bytes] = 0;
        printf("Client sent: %s\n", buf);
        std::string cmd(buf, bytes);
        std::string errstr = "Config ";
        lpfunc(cmd,this);
        close(rsock);
    }
}
Sock::Sock(const Sock& orig) {
}
int Sock::write(std::string str)
{
    const char* cstr = str.c_str();
    return send(rsock,cstr,str.size()+1,0);
}
void Sock::clientclose()
{
    close(rsock);
}
Sock::~Sock() {
    close(sock_);
    unlink(filename_c);
}

