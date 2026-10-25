//!NOBINDINGS
#ifndef _PHP_H
#define _PHP_H

/*=
PHP scripting engine
5000
**/

// Introduction
/**
This is the internal part of the RayPHP API, where Raydium
deals with Zend engine.

All this is for internal use, so no documentation is provided.
**/

#include "../php_wrappers.c"
// use this macro when registering your functions
#define C2PHP ZEND_FN


// Dirty globals... (needed for WIN32 PHP support)
#ifdef ZTS
extern zend_compiler_globals *compiler_globals;
extern zend_executor_globals *executor_globals;
extern php_core_globals *core_globals;
extern sapi_globals_struct *sapi_globals;
extern void ***tsrm_ls;
#endif
__rayapi void raydium_php_error (int type, const char *msg, ...);
__rayapi int raydium_php_uwrite (const char *str, uint str_length TSRMLS_DC);
__rayapi void raydium_php_init_request (char *filename);
__rayapi int raydium_php_exec (char *name);
__rayapi void raydium_php_close (void);
__rayapi void raydium_php_init (void);
#endif
