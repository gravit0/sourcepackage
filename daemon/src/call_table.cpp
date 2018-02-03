#include "call_table.hpp"
#include <unistd.h>
#include <string.h>
CallTable::CallTable(unsigned int size,CallCell _default)
{
    table = new CallCell[size];
    this->size = size;
    for(unsigned int i=0;i<size;++i)
    {
        table[i] = _default;
    }
    autoincrement = 0;
}
bool CallTable::add(CallCell c)
{
    if(autoincrement == size) return false;
    table[autoincrement] = c;
    autoincrement++;
    return true;
}
bool CallTable::realloc(unsigned int newsize)
{
    if(size <= newsize) return false;
    CallCell* newtable = new CallCell[newsize];
    memcpy(newtable,table,size*sizeof(CallCell));
    delete[] table;
    table = newtable;
    size = newsize;
    return true;
}
CallTable::~CallTable()
{
    delete[] table;
}
