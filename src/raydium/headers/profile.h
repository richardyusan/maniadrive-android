#ifndef _PROFILE_H
#define _PROFILE_H
#ifdef DEBUG_PROFILE

/*=
Profiling (sort of ...)
3500
**/

// Introduction
/**
You will find here a few functions for a **very simple** profiling.
For anything else than a quick time measure, use real profiling tools.
Note: Use only one "profiler" at a time.
**/

__rayapi unsigned long raydium_profile_timer;
__rayapi void raydium_profile_start(void);
/**
Starts measure.
**/

__rayapi void raydium_profile_end(char *tag);
/**
Stops measure and displays result using ##tag## string.
**/

#endif
#endif
