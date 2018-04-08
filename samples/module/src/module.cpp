#include <iostream>
#define MODULE_VERSION 1
#define MODULE_API 1
#include "sp_module_api.hpp"
CallTable::CmdResult cmd_unknown1(unsigned int, std::string)
{
    std::cerr << "[MODULE HOOK] UNKNOWN CMD" << std::endl;
    return message_result::ERROR_CMDINCORRECT;
}
void sp_module_call_main()
{
    std::cerr << "[MODULE] load OK" << std::endl;
    //t[1] = &cmd_unknown;
    api.calltable[cmds::loadmodule] = &cmd_unknown1;
    std::cerr << "[MODULE] hook " << std::hex << (long long int) api.calltable << std::endl;
}

