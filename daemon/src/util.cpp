/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   util.cpp
 * Author: gravit
 *
 * Created on 23 октября 2017 г., 15:28
 */
#include "util.hpp"
#include "basefunctions.h"
#include <iostream>
#include <fstream>
using namespace std;
namespace RecArrUtils {

    void printTree(const RecursionArray& tree, const std::string& prefix) {
        for (auto &v : tree) {
            boost::property_tree::ptree tmp = v.second.get_child("");
            if (tmp.empty())
                std::cout << prefix << v.first << ": \"" << v.second.get<std::string>("") << "\"\n";
            else {
                std::cout << prefix << v.first << "\n" << prefix << "{";
                printTree(tmp, prefix + "\t");
                std::cout << "\n" << prefix << "}\n";
            }
        }
    }

    std::string printTreeEx(const RecursionArray& tree, const std::string& prefix) {
        std::stringstream steam;
        for (auto &v : tree) {
            boost::property_tree::ptree tmp = v.second.get_child("");
            if (tmp.empty())
                steam << "\n" << prefix << v.first << ": \"" << v.second.get<std::string>("") << "\"";
            else {
                steam << "\n" << prefix << v.first << "\n" << prefix << "{" << printTreeEx(tmp, prefix + "\t") << "\n" << prefix << "}";
            }
        }
        return steam.str();
    }

    std::string toArcan(const RecursionArray& tree) {
        std::stringstream steam;
        for (auto &v : tree) {
            boost::property_tree::ptree tmp = v.second.get_child("");
            if (tmp.empty()) {
                std::string second = v.second.get<std::string>("");
                int tPos = 0;
                while (true) {
                    tPos = second.find("\\[");
                    if (tPos < 0) break;
                    second.replace(tPos, 2, "[");
                }
                tPos = 0;
                while (true) {
                    tPos = second.find("\\]");
                    if (tPos < 0) break;
                    second.replace(tPos, 2, "]");
                }
                tPos = 0;
                while (true) {
                    tPos = second.find("\\\\");
                    if (tPos < 0) break;
                    second.replace(tPos, 2, "\\");
                }
                steam << v.first << "[" << v.second.get<std::string>("") << "]";
            } else {
                steam << v.first << "[" << toArcan(tmp) << "]";
            }
        }
        return steam.str();
    }

    char SlashReplaceEx(std::string* str, const unsigned int frist_pos) {
        bool NoAdept = false;
        unsigned int size = str->size();
        char returnchar = 'b';
        for (unsigned int i = frist_pos; i < size; ++i) {
            char thch = (*str)[i];
            if (thch == '\\') {
                NoAdept = !NoAdept;
                if (i + 1 < size) {
                    char nextch = (*str)[i + 1];
                    str->replace(i, 2, 1, nextch);
                }
            } else {
                if (NoAdept) {
                    NoAdept = false;
                } else if (thch == '@') {
                    if (i + 1 < size)
                        returnchar = (*str)[i + 1];
                }
            }
        }
        return returnchar;
    }

    RecursionArray fromArcan(const std::string &str) {
        RecursionArray arr;
        int i = 0, Shet = 0;
        while (i < (int) str.size()) {

            bool isReplaceA = false, isReplaceB = false;
            int first_pos = findNoSlash(str, '[', i, &isReplaceA);
            if (first_pos < 0) break;
            std::string first;
            if (first_pos == i) {
                first = std::to_string(Shet);
                Shet++;
            } else first = str.substr(i, first_pos - i);
            char typed = SlashReplaceEx(&first, 0);
            int second_pos = findNoSlash(str, ']', first_pos, &isReplaceB);
            if (second_pos < 0) break;
            int recursion_pos = findNoSlash(str, '[', first_pos + 1, &isReplaceB);
            if (recursion_pos > 0) {
                if (recursion_pos < second_pos) {

                    int recursions = 0;
                    while (recursion_pos < second_pos) {
                        recursion_pos = findNoSlash(str, '[', recursion_pos + 1, &isReplaceB);

                        recursions++;
                        if (recursion_pos < 0) break;
                    }
                    int last_second = second_pos;
                    while (recursions > 0) {
                        last_second = findNoSlash(str, ']', last_second + 1, &isReplaceB);
                        if (last_second < 0) {
                            cout << "WARNING: Arcan Decode failed: 1" << endl << flush;
                            goto norecursion;
                        }
                        recursions--;
                    }
                    if (last_second < 0) break;
                    std::string second = str.substr(first_pos + 1, last_second - first_pos - 1);
                    arr.add_child(first, fromArcan(second));
                    i = last_second + 1;
                    continue;
                }
            }
norecursion:
            if (typed == '0') {
                arr.push_back(RecursionArray::value_type(first, RecursionArray("false")));
            } else if (typed == '1') {
                arr.push_back(RecursionArray::value_type(first, RecursionArray("true")));
            } else if (typed == 'i') {
                std::string second = str.substr(first_pos + 1, second_pos - first_pos - 1);
                if (isReplaceB)
                    SlashReplace(&second, 0);
                arr.push_back(RecursionArray::value_type(first, RecursionArray(std::to_string(byteToInt(second)))));

            } else if (typed == 'l') {
                std::string second = str.substr(first_pos + 1, second_pos - first_pos - 1);
                if (isReplaceB)
                    SlashReplace(&second, 0);
                arr.push_back(RecursionArray::value_type(first, RecursionArray(std::to_string(byteToLong(second)))));

            } else {
                std::string second = str.substr(first_pos + 1, second_pos - first_pos - 1);
                if (isReplaceB)
                    SlashReplace(&second, 0);
                arr.push_back(RecursionArray::value_type(first, RecursionArray(second)));
            }
            i = second_pos + 1;
        }
        return arr;
    }
    
    int ini_parser(std::string filename,RecursionArray* arr)
    {
        std::string category;
        std::fstream f(filename,std::ios_base::in);
        if(!f)
        {
            return -1;
        }
        std::string info;
        RecursionArray m;
        std::string key,value;
        while(std::getline(f,info))
        {
            if(info[0] == '#') continue;
            int size = info.size();
            if(info[0] == '[')
            {
                if(!category.empty())
                {
                    arr->push_back(RecursionArray::value_type(category,m));
                    m.clear();
                }
                category = info.substr(1,size - 2);
                continue;
            }
            int pos = info.find('=');
            if(pos<0) continue;
            key = info.substr(0,pos);
            value = info.substr(pos+1,size-pos);
            m.push_back(RecursionArray::value_type(key,RecursionArray(value)));
        }
        arr->push_back(RecursionArray::value_type(category,m));
        return 0;
    }
}
