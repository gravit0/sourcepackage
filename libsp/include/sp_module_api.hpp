#ifndef MODULE_VERSION
#error "define MODULE_VERSION not found"
#endif
#ifndef MODULE_API
#error "define MODULE_API not found"
#endif
#include "libsp.hpp"
#include "module.hpp"
_module_api api;
_module_version module_version;
extern "C"
{
    _module_version sp_module_init(_module_api api);
    void sp_module_call_main();
}
_module_version sp_module_init(_module_api _api)
{
    api = _api;
    module_version.version = MODULE_VERSION;
    module_version.api = MODULE_API;
    return module_version;
}
