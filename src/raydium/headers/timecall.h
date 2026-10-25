#ifndef _TIMECALL_H
#define _TIMECALL_H
/*=
Timecalls
2700
**/

// Concept
/**
As you may already know, in a real time application (as a game), you need 
to control in-game time evolution.
For example, you cannot increment a car position by 1 at each frame since 
it will generate an irregular scrolling (a frame is never rendered within 
the same time as the previous or the next one).

Raydium supports timecalls, wich are a great solution for this problem. 
Usage is very simple: write a simple function, and ask Raydium to call it 
at the desired rate.
**/

// Constraints
/**
There is an important risk with timecalls: infinite loops.
If a callback is long, it may take more CPU time than he would, as in this
very simple example:

foo() is a function, taking 200 ms for his own execution. If you ask for 
a 6 Hz execution, Raydium will execute foo() six times on the first frame, 
taking 1200 ms. On the next frame, Raydium will need to execute foo() 7 
times (the asked 6 times, and one more for the 200 ms lost during the last 
frame), taking 1400 ms, so 8 times will be needed for the next frame, then 9, ...

So you need to create callbacks as short as possible, since long callbacks 
may cause a game freeze on slower machines than yours. (1 FPS syndrom)
**/

// Hardware devices and methods
/**
Raydium must use a very accurate system timer, and will try many methods:
##/dev/rtc## , ##gettimeofday()## (Linux only) and 
##QueryPerformanceCounter## for win32.

##gettimeofday()## will use a CPU counter and is extremely accurate. 
It's far the best method. (0.001 ms accuracy is possible)

##/dev/rtc## is quite good, and Raydium will try to configure RTC at 
##RAYDIUM_TIMECALL_FREQ_PREFERED## rate (8192 Hz by default), but may 
require a "##/proc/sys/dev/rtc/max-user-freq##" modification:
##echo 8192 > /proc/sys/dev/rtc/max-user-freq##

You may want to look at common.c for interesting defines about timecalls. 
**/

#ifdef WIN32
#define __GETTIMEOFDAY_USEC 1000
#else
#define __GETTIMEOFDAY_USEC 1000000
#endif
__rayapi void raydium_timecall_raydium (GLfloat step);
/**
Internal Raydium callback.
**/

#ifdef WIN32
__rayapi float raydium_timecall_internal_w32_detect_modulo(int div);
/**
Internal, WIN32 only: Returns timer resolution for ##div## divisor.
**/

__rayapi int raydium_timecall_internal_w32_divmodulo_find(void);
/**
Internal, WIN32 only: Detects the best timer divisor for the current CPU.
**/

#endif
__rayapi unsigned long raydium_timecall_devrtc_clock (void);
/**
Internal, Linux only: Reads and return RTC clock.
**/

__rayapi unsigned long raydium_timecall_clock (void);
/**
Returns current "time".
**/

__rayapi signed char raydium_timecall_devrtc_rate_change (unsigned long new_rate);
/**
Internal, Linux only: Modifies RTC clock rate.
**/

__rayapi void raydium_timecall_devrtc_close (void);
/**
Internal, Linux only: Will close RTC clock.
**/

__rayapi unsigned long raydium_timecall_devrtc_init (void);
/**
Internal, Linux only: Will open RTC clock.
**/

__rayapi int raydium_timecall_detect_frequency (void);
/**
Internal: This function will find the best timer available for current
platform, and adjust properties to your hardware (rate, divisor, ...).
**/

__rayapi void raydium_timecall_init (void);
/**
Internal use.
**/


__rayapi int raydium_timecall_add (void *funct, GLint hz);
/**
There is two sort of timecalls with Raydium:

1. Standard ones:
%%(c)
raydium_timecall_add(function,800);
%%
##void function(void)## will be called 800 times per second.

2. Elastic timed ones:
%%(c)
raydium_timecall_add(function,-80);
%%
##void function(float step)## will be called for each frame, with a 
"##step## factor" as argument. In the above example, a 160 Hz game will call 
function with step = 0.5, but step = 2.0 for a 40 Hz game.

A standard timecall will use ##void(void)## function and a positive ##hertz##
argument, as an elasitc one will use ##void(float)## and negative ##hertz## argument.
**/

__rayapi void raydium_timecall_freq_change (int callback, GLint hz);
/**
This function changes the ##callback## frequency. See above for possibles
values of ##hz## (negative and positive values).
**/

__rayapi void raydium_timecall_callback (void);
/**
Internal use (frame fired callback).
**/

#endif
