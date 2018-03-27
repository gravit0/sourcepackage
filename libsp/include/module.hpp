typedef std::pair<void*,size_t> (*CallCell)(unsigned int, std::string);
struct _module_version
{
    unsigned int version;
    unsigned int api;
};
struct _module_api
{
    CallCell* calltable;
};
