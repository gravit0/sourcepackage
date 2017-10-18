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
Sock::Sock(std::string filepath) {
    loopEnable = false;
    sock_ = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_ < 0) {
        throw socket_exception(socket_exception::SocketError);
    }
    srvr_name.sa_family = AF_UNIX;
    filename_c = filepath.c_str();
    strcpy(srvr_name.sa_data, filename_c);
    if (bind(sock_, &srvr_name, strlen(srvr_name.sa_data) +
            sizeof (srvr_name.sa_family)) < 0) {
        throw socket_exception(socket_exception::BindError);
    }
    listen(sock_, 1);
}
socket_exception::socket_exception(Errors err)
{
    thiserr = err;
}
const char* socket_exception::what() const noexcept
{
    switch(thiserr)
    {
        case AcceptError: return "Accept Error";
        case SocketError: return "Socket Error";
        case BindError: return "Bind Error";
        default: return "Unknown Error";
    }
}
Client::Client(int sock)
{
    this->sock = sock;
}
void Sock::loop(void (*lpfunc)(std::string,Client*)){
    loopEnable = true;
    while (loopEnable) {
        Client* rsock = new Client(accept(sock_,NULL,NULL));
        if(rsock < 0)
        {
            throw socket_exception(socket_exception::AcceptError);
        }
        rsock->read();
        if (rsock->bytes < 0) {
            delete rsock;
            continue;
        }
        rsock->buf[rsock->bytes] = 0;
        printf("Client sent: %s\n", rsock->buf);
        std::string cmd(rsock->buf, rsock->bytes);
        lpfunc(cmd,rsock);
        if(rsock->isAutoClosable) delete rsock;
    }
}
int Sock::deattach()
{
    //int ret = rsock;
    //rsock = 0;
    return 0;
}
Sock::Sock(const Sock& orig) {
}
int Client::write(std::string str)
{
    const char* cstr = str.c_str();
    return send(sock,cstr,str.size()+1,0);
}
int Client::read()
{
    bytes = recv(sock, buf, sizeof(buf),0);
    if (bytes < 0) {
       perror("recv failed");
       close(sock);
    }
    return bytes;
}
Client::~Client()
{
    close(sock);
}
void Sock::stop()
{
    loopEnable = false;
}
Sock::~Sock() {
    close(sock_);
    unlink(filename_c);
}

