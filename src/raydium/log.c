/*
    Raydium - CQFD Corp.
    http://raydium.org/
    License: GPL - GNU General Public License, see "gpl.txt" file.
*/

#ifndef DONT_INCLUDE_HEADERS
#include "index.h"
#else
#include "headers/log.h"
#endif 

void raydium_console_line_add(char *format, ...);

void raydium_log(char *format, ...)
{
char str[RAYDIUM_MAX_NAME_LEN];
va_list argptr;
int retlen;


va_start(argptr,format);
retlen = vsnprintf(str,RAYDIUM_MAX_NAME_LEN - 1, format,argptr);
va_end(argptr);

if(retlen < 0) retlen = 0;
str[retlen] = '\0';

printf("Raydium: %s\n",str);
if(raydium_log_file) fprintf(raydium_log_file,"%s\n",str);
raydium_console_line_add("%s", str);
}
