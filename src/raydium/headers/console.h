#ifndef _CONSOLE_H
#define _CONSOLE_H
/*=
In-game console
3000
**/

// Introduction
/**
This chapter introduce Raydium console, allowing applications to take 
user keyboard input (game commands, chat, ...) and to send informations 
to this console.
The end user can call the console using "the key below esc". 

By default, if PHP support is enabled, all user commands will be redirected
to PHP engine. Each command will get his own context, don't expect to create
anything else than "single line PHP scripts" with the console. See PHP chapter
for more informations.
The console allows the user to prefix command with the following characters:

- ##/##: Non PHP command. The command will be sent to application (see
##raydium_console_gets_callback##, below.

- ##>##: Will launch argument as a PHP script (identical to ##include("...")##)

- ##!##: Will launch argument as a sequence script

Command history is saved to ##raydium_history## file when application exits.

You can use a ##void prompt(char *)## callback to get user commands. Your
callback must be registered thru ##raydium_console_gets_callback##:
%%(c) raydium_console_gets_callback=prompt; %%

This console provides auto-completion of register functions and variables.
See the suitable chapter for more information.
**/

__rayapi void raydium_console_init (void);
/**
Internal use.
**/

__rayapi void raydium_console_history_save (void);
/**
Internal use (will flush console history to disk).
You can call it by yourself if needed.
**/

__rayapi int raydium_console_gets (char *where);
/**
**DISABLED**.
Use ##raydium_console_gets_callback## function pointer instead.
**/

__rayapi void raydium_console_history_previous (void);
/**
Internal use.
**/

__rayapi void raydium_console_history_next (void);
/**
Internal use.
**/

__rayapi void raydium_console_history_add (char *str);
/**
Internal use.
**/

__rayapi void raydium_console_exec_script (char *file);
/**
Internal use.
**/

__rayapi void raydium_console_exec_last_command (void);
/**
Internal use.
**/

__rayapi void raydium_console_line_add (char *format, ...);
/**
Mostly reserved for internal use, but unless ##raydium_log##, this function will
add the provided data only to ingame console, and not to "native" console.
**/

__rayapi int raydium_console_history_read(char **hist);
/**
This function will build an history list.
See this example :
%%(c)
char *hist[RAYDIUM_CONSOLE_MAX_LINES];
int i,n;
n=raydium_console_history_read(hist);
for(i=0;i<n;i++)
    printf("> %s\n",hist[i]);
%%
**Warning**: Be sure that there's no new history line between the call and
the end of ##hist## usage (Or copy ##hist## to a safer place).
**/


__rayapi void raydium_console_event (void);
/**
Internal use. Will switch console up and down.
**/

__rayapi void raydium_console_draw (void);
/**
Internal use.
**/

__rayapi int raydium_console_internal_isalphanumuscore (char c);
/**
Internal use.
**/

__rayapi void raydium_console_complete (char *str);
/**
Internal use.
**/

#endif
