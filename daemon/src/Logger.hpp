/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Logger.hpp
 * Author: gravit
 *
 * Created on 28 сентября 2017 г., 19:16
 */

#ifndef LOGGER_HPP
#define LOGGER_HPP
#include <string>
#include <fstream>

class Logger {
private:
    std::fstream of;
public:

    enum LOG_TYPE {
        LOG_KMSG,
        LOG_STDOUT,
        LOG_STDERR,
        LOG_FILE
    };
    LOG_TYPE type;
    Logger(LOG_TYPE t, std::string file);
    Logger(LOG_TYPE t);
    void logg(std::string str);
    virtual ~Logger();
private:

};

#endif /* LOGGER_H */

