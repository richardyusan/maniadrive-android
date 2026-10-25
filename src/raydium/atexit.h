/*
    Raydium - CQFD Corp.
    http://raydium.org/
    License: GPL - GNU General Public License, see "gpl.txt" file.
*/

#ifndef ATEXIT_H
#define ATEXIT_H

#define RAYDIUM_MAX_ATEXIT_FUNCTIONS	32

__global void *raydium_atexit_functions[RAYDIUM_MAX_ATEXIT_FUNCTIONS];
__global int raydium_atexit_index;

// proto
__rayapi void raydium_log (char *format, ...);

#endif
