/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Sock.hpp
 * Author: gravit
 *
 * Created on 28 сентября 2017 г., 16:45
 */

#ifndef SOCK_HPP
#define SOCK_HPP
#include "config.hpp"
#include <sys/types.h>
#include <sys/socket.h>
class Sock {
private:
    struct sockaddr srvr_name;
    char buf[SOCK_BUF_SIZE];
    unsigned int bytes;
    int sock_,rsock;
    const char* filename_c;
public:
    Sock(std::string filepath);
    Sock(const Sock& orig);
    void loop(void (*lpfunc)(std::string,Sock*));
    int write(std::string str);
    void clientclose();
    virtual ~Sock();
private:

};

#endif /* SOCK_HPP */

