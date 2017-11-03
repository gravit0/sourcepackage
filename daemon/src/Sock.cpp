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
#include <exception>
#include <thread>
#include <mutex>
#include <map>
#include "EventManager.hpp"
#include "Logger.hpp"
Sock::Sock(std::string filepath, int max_connect) {
    this->max_connect = max_connect;
    epollsock = epoll_create(max_connect);
    events = new epoll_event[max_connect];
    loopEnable = false;
    sock_ = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_ < 0) {
        throw socket_exception(socket_exception::SocketError);
    }
    srvr_name.sun_family = AF_UNIX;
    filename_c = filepath.c_str();
    strcpy(srvr_name.sun_path, filename_c);
    if (bind(sock_, (sockaddr*) &srvr_name, sizeof(srvr_name)) < 0) {
        throw socket_exception(socket_exception::BindError);
    }
    listen(sock_, max_connect-1);
}

socket_exception::socket_exception(Errors err) {
    thiserr = err;
}

const char* socket_exception::what() const noexcept {
    switch (thiserr) {
        case AcceptError: return "Accept Error";
        case SocketError: return "Socket Error";
        case BindError: return "Bind Error";
        case RecvError: return "Recv Error";
        default: return "Unknown Error";
    }
}

Client::Client(int sock) {
    this->sock = sock;
}
std::map<int, Client*> smap;

void Sock::loop(void (*lpfunc)(std::string, Client*)) {
    loopEnable = true;
    static struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLPRI | EPOLLERR | EPOLLHUP;
    ev.data.fd = sock_;
    epoll_ctl(epollsock, EPOLL_CTL_ADD, sock_, &ev);

    while (loopEnable) {
        int t = wait(cfg.socket_timeout);
        if (t < 0) {
            logger->logg('C',"Error " + t);
            //loopEnable = false;
            continue;
        }
        for (int i = 0; i < t; i++) {
            if (events[i].events & EPOLLIN) {
                if (events[i].data.fd == sock_) {
                    int native_sock = accept(sock_, NULL, NULL);
                    static struct epoll_event ev;
                    ev.events = EPOLLIN | EPOLLPRI | EPOLLERR | EPOLLHUP;
                    ev.data.fd = native_sock;
                    epoll_ctl(epollsock, EPOLL_CTL_ADD, native_sock, &ev);
                    if (native_sock < 0) {
                        throw socket_exception(socket_exception::AcceptError);
                    }
                    Client* rsock = new Client(native_sock);
                    smap[native_sock] = rsock;
                } else {
                    Client* rsock = smap[events[i].data.fd];
                    rsock->read();
                    if (rsock->bytes == 0) {
                        delete rsock;
                        continue;
                    }
                    rsock->buf[rsock->bytes] = 0;
                    printf("Client sent: %s\n", rsock->buf);
                    std::string cmd(rsock->buf, rsock->bytes);
                    lpfunc(cmd, rsock);
                    if (rsock->isAutoClosable) delete rsock;
                }
            }
            if (events[i].events & EPOLLHUP)
            {
                delete smap[events[i].data.fd];
                smap.erase(events[i].data.fd);
            }
        }
    }
}

int Sock::wait(int timeout) {
    return epoll_wait(epollsock, events, max_connect, timeout);
}

int Sock::deattach() {
    //int ret = rsock;
    //rsock = 0;
    return 0;
}

int Client::write(std::string str) {
    const char* cstr = str.c_str();
    std::cout << "Client recv: " << str << std::endl;
    return send(sock, cstr, str.size() + 1, 0);
}

int Client::read() {
    bytes = recv(sock, buf, sizeof (buf), 0);
    return bytes;
}

Client::~Client() {
    close(sock);
    if(isListener) event.removeListener(this);
}

void Sock::stop() {
    loopEnable = false;
}

Sock::~Sock() {
    close(sock_);
    close(epollsock);
    delete[] events;
    unlink(filename_c);
}

