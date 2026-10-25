#ifndef _CLI_H
#define _CLI_H
/*=
Command Line Interface
2401
**/

// Introduction
/**
Here, you'll find a few functions to deal with command line
interface of Raydium.
**/

__rayapi int raydium_init_cli_option(char *option, char *value);
/**
This function will search command line ##option##.
If this option is found, the functions stores any argument to ##value## and
returns 1.
The function will return 0 if ##option## is not found.

Example (search for: ##--ground##)
%%(c)
char model[RAYDIUM_MAX_NAME_LEN];
if(raydium_init_cli_option("ground",model))
    {
    setground(model);
    }
%%
**/

__rayapi int raydium_init_cli_option_default(char *option, char *value, char *default_value);
/**
Same as above, but allows you to provide a default value (##default##) if
the ##option## is not found on command line.
**/

__rayapi void raydium_init_internal_homedir_find(char *);
/**
Internal use.
**/

#ifndef RAYDLL
__rayapi void raydium_init_args(int argc, char **argv);
/**
You must use this function, wich send application arguments to Raydium 
and external libs (GLUT, OpenAL, ...).
This must be done **before** any other call to Raydium.
Example:
%%(c)
int main(int argc, char **argv)
{
raydium_init_args(argc,argv);
[...]
%%
**/
#endif

#ifndef RAYDLL
__rayapi void raydium_init_args_name(int argc, char **argv, char *app_name);
/**
Same as above, but with application short name. This string is used to
build things like runtime configuration directory name (~/.raydium/ by default).
Use this wrapper if you don't want to share your configuration with Raydium.
**/
#endif


/*
Ok ... all this is a very ugly part: under win32, a DLL use a different
atexit() queue than the application. We can't use the DLL_PROCESS_DETACH
DLL reason, since it seems that OpenAL DLL is unloaded *before* raydium.dll !

So the idea here is to provide a wrapper to the application for these two
init functions so they use their own atexit() queue, in a transparent way.

See atexit.h header for wrappers.
*/
#ifdef RAYDLL
__rayapi void raydium_init_args_hack(int argc, char **argv);
__rayapi void raydium_init_args_name_hack(int argc, char **argv, char *app_name);
#endif


#endif
