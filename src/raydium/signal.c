/*
    Raydium - CQFD Corp.
    http://raydium.org/
    License: GPL - GNU General Public License, see "gpl.txt" file.
*/

#ifndef DONT_INCLUDE_HEADERS
#include "index.h"
#else
#include "headers/signal.h"
#endif

void raydium_signal_handler(int sig)
{
raydium_log("signal %i disabled.",sig);
}

void raydium_signal_install_trap(void)
{
signed int err=0;

if(signal(SIGINT,raydium_signal_handler)==SIG_ERR)
    err++;
#ifndef WIN32
if(signal(SIGPIPE,raydium_signal_handler)==SIG_ERR)
    err++;
#endif

if(err)
    raydium_log("Signal Handlers: FAILED !");
else
    raydium_log("Signal Handlers: OK");
}
