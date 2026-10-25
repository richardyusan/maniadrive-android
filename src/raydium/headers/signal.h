#ifndef _HEADERS_SIGNAL_H
#define _HEADERS_SIGNAL_H
/*=
Signals
2500
**/

// Quickview
/**
There almost nothing to said about signals management, except that Raydium 
will try to catch SIGINT signal (sended by CTRL+C sequence, for example).
There's nothing else for now, but we plan a user callback for this signal. 
**/

__rayapi void raydium_signal_handler (int sig);
__rayapi void raydium_signal_install_trap (void);
#endif
