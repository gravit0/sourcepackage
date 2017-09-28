#ifndef PKGUTIL_H
#define PKGUTIL_H
#include "main.hpp"
#include <string>
extern Package* find_pack(std::string name);
extern Package* unload_pack(std::string name);
extern Package* get_pack(std::string dir);
#endif
