#include <iostream>
#define MODULE_VERSION 1
#define MODULE_API 1
#include "sp_module_api.hpp"
std::pair<void*,size_t> cmd_unknown1(unsigned int, std::string)
{
    message_result* result = new message_result{0,message_result::OK,0,0};
    std::cerr << "[MODULE HOOK] UNKNOWN CMD" << std::endl;
    return {result,sizeof(result)};
}
void sp_module_call_main()
{
    std::cerr << "[MODULE] load OK" << std::endl;
    //t[1] = &cmd_unknown;
    api.calltable[cmds::loadmodule] = &cmd_unknown1;
    std::cerr << "[MODULE] hook " << std::hex << (long long int) api.calltable << std::endl;
}

