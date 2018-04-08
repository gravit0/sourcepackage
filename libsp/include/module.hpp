#include "call_table.hpp"
#include "Sock.hpp"
struct _module_version
{
    unsigned int version;
    unsigned int api;
};
struct _module_api
{
    CallTable::CallCell* calltable;
};
