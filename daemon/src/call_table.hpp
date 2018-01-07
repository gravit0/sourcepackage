#ifndef CALLTABLE_HPP
#define CALLTABLE_HPP
#include <string>
#include <vector>
#include "Sock.hpp"
class CallTable
{
public:
    typedef void (*CallCell)(unsigned int, std::string, Client*);
    CallCell* table;
    unsigned int size;
    unsigned int autoincrement;
    CallTable(unsigned int size,CallCell _default);
    bool add(CallCell c);
    inline void call(unsigned int index,unsigned int arg1, std::string arg2, Client* arg3)
    {
        return table[index](arg1,arg2,arg3);
    }
    bool realloc(unsigned int newsize);
};
#endif
