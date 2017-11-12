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
#include <sys/un.h>
#include <exception>
#include <boost/noncopyable.hpp>
#include <sys/epoll.h>
#include <memory>
class Sock;
class Client : public boost::noncopyable {
protected:
    int sock;
    Client(int sock);
    friend Sock;
    char buf[SOCK_BUF_SIZE];
    unsigned int bytes;
public:
    typedef std::shared_ptr<Client> ptr;
    bool isListener = false;
    bool isAutoClosable = true;
    int write(std::string str);
    int read();
    virtual ~Client();
};

class Sock : public boost::noncopyable {
private:
    struct sockaddr_un srvr_name;
    int sock_;
    const char* filename_c;
    bool loopEnable;
    int epollsock;
    int max_connect;
    epoll_event* events;
public:
    Sock(std::string filepath, int max_connect);
    void loop(void (*lpfunc)(std::string, Client*));
    int deattach();
    void stop();
    int wait(int timeout);
    virtual ~Sock();
private:

};

class socket_exception : public std::exception {
public:

    enum Errors {
        SocketError,
        BindError,
        AcceptError,
        RecvError
    };
    Errors thiserr;
    socket_exception(Errors err);
    virtual const char* what() const noexcept;
};
#endif /* SOCK_HPP */

