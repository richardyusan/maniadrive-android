#ifndef _ATEXIT__H
#define _ATEXIT__H
#include "../atexit.h"

/*=
Atexit functions
4400
**/

// Introduction
/**
Raydium provides its own atexit function, since Win32 DLL requires a bit
of magic for such things. This support is mainly here for internal reasons,
you can continue to use regular atexit() in your applications.
**/


__rayapi int raydium_atexit(void (*func)(void));
/**
As the original atexit():
Register a function to be called at norma program termination.
Functions so registered are called in the reverse order of their 
registration; no arguments are passed.
Returns 0 if successful.
**/

__rayapi void raydium_atexit_call(void);
/**
Internal use. Will call all registered functions.
**/

__rayapi void raydium_atexit_init(void);
/**
Internal use.
**/

// Hack. See cli.h header for more information.
#ifdef RAYDLL
#define raydium_init_args(argc,argv)\
{\
atexit(raydium_atexit_call);\
raydium_init_args_hack(argc,argv);\
}

#define raydium_init_args_name(argc,argv,app_name)\
{\
atexit(raydium_atexit_call);\
raydium_init_args_name_hack(argc,argv,app_name);\
}
#endif

#endif
