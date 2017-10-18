/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   getopts.h
 * Author: gravit
 *
 * Created on 1 октября 2017 г., 16:15
 */

#ifndef GETOPTS_H
#define GETOPTS_H
#include "config.hpp"
namespace getopts
{
    #include <getopt.h>
    enum types : int
    {
        no_arg = 0,
        required_arg = 1,
        optional_arg = 2
    };
    struct longopts_st
    {
        int isDaemon = 0;
        int isNoForking = 0;
        int isNoWarning = 1;
        int isHelp = 0;
    };
    static longopts_st longopts;
    static const char *optString = "divc:s:r:";
    const struct option long_options[] = {
        {"daemon",no_arg,&longopts.isDaemon,1},
        {"no-forking",no_arg,&longopts.isNoForking,1},
        {"no-warning",no_arg,&longopts.isNoWarning,1},
        {"help",no_arg,&longopts.isHelp,1},
        //{"optc",no_argument,&flag_c,-121},
        {NULL,0,NULL,0}
    };
};

#endif /* GETOPTS_H */

