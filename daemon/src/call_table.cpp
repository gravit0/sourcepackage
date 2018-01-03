#include "call_table.hpp"
CallTable::CallTable(int size)
{
    table = new CallCell[size];
    this->size = size;
}
