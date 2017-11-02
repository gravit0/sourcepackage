/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Logger.cpp
 * Author: gravit
 * 
 * Created on 28 сентября 2017 г., 19:16
 */

#include "Logger.hpp"
#include <iostream>

Logger::Logger(LOG_TYPE t, std::string file) {
    type = t;
    if (t != LOG_TYPE::LOG_FILE) throw "Logic Error: LOG_TYPE != FILE ";
    of.open(file, std::ios_base::out);
}

Logger::Logger(LOG_TYPE t) {
    type = t;
    if (t == LOG_TYPE::LOG_FILE) throw "Logic Error: LOG_TYPE == FILE ";
    else if (t == LOG_TYPE::LOG_KMSG) of.open("/proc/kmsg", std::ios_base::out);
}

void Logger::logg(char level, std::string str) {
    if (type == LOG_TYPE::LOG_FILE || type == LOG_TYPE::LOG_KMSG) of << str;
    else if (type == LOG_TYPE::LOG_STDOUT) std::cout << str;
    else if (type == LOG_TYPE::LOG_STDERR) std::cerr << str;
}

Logger::~Logger() {
}

