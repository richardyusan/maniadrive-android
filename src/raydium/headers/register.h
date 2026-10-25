#ifndef _REGISTER_H
#define _REGISTER_H

/*=
Data registration
3400
**/

// Introduction
/**
Raydium supports scripting, for example using PHP in the current implementation.
All ##raydium_register_*## functions are provided as a "bridge" between
your applications and PHP scripts, allowing you to "export" native variables 
and functions to PHP scripts.
For more informations, see PHP chapters.
**/

__rayapi int raydium_register_find_name (char *name);
/**
Lookups a **variable** by ##name##. Search is not possible (yet) for
registered functions.
Mostly used internally.
**/

__rayapi signed char raydium_register_name_isvalid (char *name);
/**
Tests ##name##, and returns his viability as a boolean.
Accepted intervals for variables and functions: [a-z], [A-Z] and '_'
Numerics are not allowed.
**/

__rayapi int raydium_register_variable (void *addr, int type, char *name);
/**
Will register a new variable. You must provide variable's address (##addr##), 
##type## and ##name##.
Current available types are: ##RAYDIUM_REGISTER_INT##, ##RAYDIUM_REGISTER_FLOAT##,
and ##RAYDIUM_REGISTER_STR##.
**/

__rayapi int raydium_register_variable_const_f(float val, char *name);
/**
Will register a new ##float## constant.
**/

__rayapi int raydium_register_variable_const_i(int val, char *name);
/**
Will register a new ##int## constant.
**/

__rayapi void raydium_register_variable_unregister_last (void);
/**
Variable are registered on a stack. As you may want to create "temporary"
variables (usefull for building script's arguments, for example), this function
allows you to unregister last registered variable. Multiple calls are possible.
**/

__rayapi int raydium_register_modifiy (char *var, char *args);
/**
Deprecated.
**/

__rayapi void raydium_register_function (void *addr, char *name);
/**
Will register a function. You only need to provide an address (##addr##)
and a name.
**/

__rayapi void raydium_register_dump (void);
/**
Will dump to console all registered variables and functions.
**/

#endif
