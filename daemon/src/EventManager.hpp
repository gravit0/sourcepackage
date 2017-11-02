/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   EventManager.hpp
 * Author: gravit
 *
 * Created on 31 октября 2017 г., 23:13
 */

#ifndef EVENTMANAGER_HPP
#define EVENTMANAGER_HPP
#include <string>
#include <list>
#include <map>
#include "Sock.hpp"
struct EventListener {
    static const int EVENT_INSTALL = 1 << 0;
    static const int EVENT_REMOVE = 1 << 1;
    int event;
    Client* client;
};
class EventManager {
private:
    std::list<EventListener> list;
public:
    EventManager();
    virtual ~EventManager();
    void sendEvent(int event, std::string data);
    void addListener(EventListener ev);
    void removeListener(Client* client);
    static std::map<int,char> eventmap;
private:

};
extern EventManager event;
#endif /* EVENTMANAGER_HPP */

