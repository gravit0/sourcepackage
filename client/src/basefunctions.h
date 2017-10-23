/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   basefunctions.h
 * Author: gravit
 *
 * Created on 3 октября 2017 г., 17:03
 */

#ifndef BASEFUNCTIONS_H
#define BASEFUNCTIONS_H

#include <string>
#include <sstream>
long long int byteToLong(std::string buffer);
int byteToInt(std::string buffer);
std::string IntToByte(int integer);
void SlashReplace(std::string* str, const unsigned int frist_pos);
int findNoSlash(const std::string& str,const char ch, const unsigned int frist_pos,bool* isReplace);

#endif /* BASEFUNCTIONS_H */

