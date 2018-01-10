#ifndef CALLTABLE_HPP
#define CALLTABLE_HPP
#include <string>
#include <vector>
struct message_result
{
    unsigned char version;
    unsigned char code;
    signed short flag; //Зарезервировано
    unsigned int size;
    enum : unsigned char{
        OK = 0,
        ERROR_FILENOTFOUND = 1,
        ERROR_DEPNOTFOUND = 2,
        ERROR_PKGNOTFOUND = 3,
        ERROR_PKGINCORRECT = 4,
        ERROR_CMDINCORRECT = 5
    };
};
struct message_error : public message_result
{
    unsigned int errorcode;
};
class CallTable
{
public:
    typedef std::pair<void*,size_t> (*CallCell)(unsigned int, std::string);
    CallCell* table;
    unsigned int size;
    unsigned int autoincrement;
    CallTable(unsigned int size,CallCell _default);
    bool add(CallCell c);
    inline std::pair<void*,size_t> call(unsigned int index,unsigned int arg1, std::string arg2)
    {
        return table[index](arg1,arg2);
    }
    bool realloc(unsigned int newsize);
};
#endif
