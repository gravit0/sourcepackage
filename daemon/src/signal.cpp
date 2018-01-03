#include "main.hpp"
#include "Sock.hpp"
#include <signal.h>
#include <unistd.h>
void hdl(int sig)
{
    if(sig==SIGPIPE)
    {
        //IGNORED
    }
    else if(sig==SIGTERM)
    {
        delete gsock;
    }
}
int main_set_signals()
{
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_flags = SA_RESTART;
    act.sa_handler = hdl;
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGPIPE);
    sigaddset(&set, SIGTERM);
    act.sa_mask = set;
    sigaction(SIGPIPE, &act, 0);
    sigaction(SIGTERM, &act, 0);
    return 0;
}
