/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   EventManager.cpp
 * Author: gravit
 * 
 * Created on 31 октября 2017 г., 23:13
 */

#include "EventManager.hpp"
#include <sstream>
EventManager event;
EventManager::EventManager() {
}

EventManager::~EventManager() {
}

void EventManager::addListener(EventListener ev)
{
    ev.client->isListener = true;
    list.push_back(ev);
}
void EventManager::sendEvent(int event, std::string data)
{
    for(auto &i : list)
    {
        if(i.event & event)
        {
            i.client->write("event " + ((std::ostringstream&)(std::ostringstream() << event)).str() + data);
        }
    }
}
void EventManager::removeListener(Client* client)
{
    for(auto i = list.begin();i!=list.end();++i)
    {
        if((*i).client == client)
        {
            list.erase(i);
        }
    }
}