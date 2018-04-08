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
#include "call_table.hpp"
Sock::Sock(std::string filepath, int max_connect) : table((unsigned int)cmds::MAX_COMMANDS,nullptr) {
    this->max_connect = max_connect;
    epollsock = epoll_create(max_connect);
    events = new epoll_event[max_connect];
    loopEnable = false;
    sock_ = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_ < 0) {
        throw new socket_exception(socket_exception::SocketError);
    }
    srvr_name.sun_family = AF_UNIX;
    filename_c = filepath.c_str();
    strcpy(srvr_name.sun_path, filename_c);
    if (bind(sock_, (sockaddr*) & srvr_name, sizeof (srvr_name)) < 0) {
        throw new socket_exception(socket_exception::BindError);
    }
    listen(sock_, max_connect - 1);
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
int Client::write(std::pair<void*,size_t> data)
{
    return send(sock, data.first, data.second, 0);
}
std::map<int, Client*> smap;

void Sock::loop() {
    loopEnable = true;
    static struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLPRI | EPOLLERR | EPOLLHUP;
    ev.data.fd = sock_;
    epoll_ctl(epollsock, EPOLL_CTL_ADD, sock_, &ev);
    loop_impl(multithread_loop::MASTER);
}


void Sock::loop_impl([[maybe_unused]] multithread_loop l) {
    while (loopEnable) {
        int t = wait(cfg.epoll_timeout);
        if (t < 0) {
            logger->logg('C', "Wait error " + t);
            //loopEnable = false;
            continue;
        }
        for (int i = 0; i < t; i++) {
            if (events[i].events & EPOLLIN) {
                if (events[i].data.fd == sock_) {
                    int native_sock = accept(sock_, NULL, NULL);
                    if (native_sock < 0) {
                        throw socket_exception(socket_exception::AcceptError);
                    }
                    static struct epoll_event ev;
                    ev.events = EPOLLIN | EPOLLPRI | EPOLLERR | EPOLLHUP;
                    ev.data.fd = native_sock;
                    epoll_ctl(epollsock, EPOLL_CTL_ADD, native_sock, &ev);
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
                    if(exec(rsock->buf,rsock->bytes,rsock) < 0) {
                        delete rsock;
                        continue;
                    }
                    if (rsock->isAutoClosable) {
                        delete rsock;
                    }
                }
            }
            if (events[i].events & EPOLLHUP) {
                delete smap[events[i].data.fd];
                smap.erase(events[i].data.fd);
            }
        }
    }
}
int Sock::exec(char* data, unsigned int size,Client* t)
{
    if(size < sizeof(message_head))
    {
        logger->logg('W', "Command protocol error: 1");
        return -1;
    }
    message_head* head = (message_head*)data;
    if(size != sizeof(message_head) + head->size)
    {
        logger->logg('W', "Command protocol error: 2");
        return -1;
    }
    std::string cmd(data + sizeof(message_head), head->size);
    //printf("Client sent: %s\n", rsock->buf);
    //lpfunc(head,cmd, rsock);
    if(head->cmd >= cmds::MAX_COMMANDS)
    {
        logger->logg('W', "Command protocol error: unsupported command");
        return -1;
    }
    auto result = table.table[head->cmd](head->cmdflags,cmd);
    if(std::holds_alternative<CallTable::pair>(result)) {
        auto pair = std::get<CallTable::pair>(std::move(result));
        t->write(pair);
        if(pair.second == sizeof(message_result))
        {
            delete static_cast<message_head*>(pair.first);
        }
        else
        {
            delete[] static_cast<char*>(pair.first);
        }
    }
    else
    {
        auto results = std::get<message_result::results>(std::move(result));
        message_result result{0,results,0,0};
        t->write({&result,sizeof(result)});
    }
    return 0;
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
    //std::cout << "Client recv: " << str << std::endl;
    return send(sock, cstr, str.size() + 1, 0);
}

int Client::read() {
    bytes = recv(sock, buf, sizeof (buf), 0);
    return bytes;
}
int Client::send_ok()
{
    message_result result{0,0,0,0};
    return send(sock, &result, sizeof(result), 0);
}
int Client::send_error(unsigned int errorcode)
{
    message_error result{1,0,0,sizeof(errorcode),errorcode};
    return send(sock, &result, sizeof(result), 0);
}
Client::~Client() {
    close(sock);
    if (isListener) event.removeListener(this);
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

