#ifndef _REGAPI_H
#define _REGAPI_H

/*=
RegAPI
4100
**/

// Introduction
/**
RegAPI is an internal system that exports some Raydium's API functions to
scripting engine, creating bindings.
See RayPHP chapter for more informations anout scripting.
**/

__rayapi void raydium_register_api(void);
/**
Internal. Will register Raydium API.
**/

#endif
