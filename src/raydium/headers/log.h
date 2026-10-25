#ifndef _LOG_H
#define _LOG_H

/*=
Logging
300
**/

// Introduction to log.c
/**
Raydium uses and provides his own logging system, 
hidden behind a single function, as shown below.
**/

#ifndef RAYDIUM_NETWORK_ONLY
__rayapi void raydium_console_line_add (char *format, ...);
#endif
__rayapi void raydium_log (char *format, ...);
/**
This function must be used like "printf", using a format 
("%s, %i, %x, ...") and then, suitable variables, 
but without the end-line char ('\n')

%%(c)
raydium_log("You are player %i, %s",player_number,player_name);
%%
 
For now, this function writes to the parent terminal and the in-game console, with "Raydium: " string prefix.
The user can force logging to a file, using ##--logfile## command line switch.
**/
#endif
