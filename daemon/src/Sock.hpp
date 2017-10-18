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
#include <exception>
#include <boost/noncopyable.hpp>
class Client : public boost::noncopyable
{
private:
    int sock;
public:
    bool isAutoClosable = true;
    char buf[SOCK_BUF_SIZE];
    unsigned int bytes;
    Client(int sock);
    int write(std::string str);
    int read();
    ~Client();
};
class Sock : public boost::noncopyable {
private:
    struct sockaddr srvr_name;
    int sock_;
    const char* filename_c;
    bool loopEnable;
public:
    Sock(std::string filepath);
    Sock(const Sock& orig);
    void loop(void (*lpfunc)(std::string,Client*));
    int deattach();
    void stop();
    virtual ~Sock();
private:

};
class socket_exception : public std::exception
{
public:
    enum Errors
    {
        SocketError,
        BindError,
        AcceptError
    };
    Errors thiserr;
    socket_exception(Errors err);
    virtual const char* what() const noexcept;
};
#endif /* SOCK_HPP */

