/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   util.hpp
 * Author: gravit
 *
 * Created on 23 октября 2017 г., 15:35
 */

#ifndef UTIL_HPP
#define UTIL_HPP

#include <boost/property_tree/ptree.hpp>
#include <string>
typedef boost::property_tree::ptree RecursionArray;
namespace RecArrUtils {
    void printTree(const RecursionArray& tree, const std::string& prefix = "");
    char SlashReplaceEx(std::string* str, const unsigned int frist_pos);
    RecursionArray fromArcan(const std::string& str);
    std::string toArcan(const RecursionArray& tree);
    std::string printTreeEx(const RecursionArray& tree, const std::string& prefix = "");
    std::string IntToByte(int integer);
    int ini_parser(std::string filename, RecursionArray* arr);
    int ini_parser_lam(std::string filename, std::function<void (std::string_view key, std::string_view value, std::string_view category, bool isSetCategory) > lam);
}
#endif /* UTIL_HPP */

