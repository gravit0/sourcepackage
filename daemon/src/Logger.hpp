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
    enum class level
    {
        info = 1,
        warning = 2,
        error = 3,
        critical = 4
    };
    LOG_TYPE type;
    Logger(LOG_TYPE t, std::string file);
    Logger(LOG_TYPE t);
    void logg(char level, std::string str);
    virtual ~Logger();
private:

};
extern Logger * logger;
#endif /* LOGGER_H */

