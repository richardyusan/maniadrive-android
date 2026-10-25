#ifndef _RANDOM_H
#define _RANDOM_H
/*=
Random
400
**/

// Introduction
/**
These functions deals with random numbers generation.
**/

__rayapi void raydium_random_randomize (void);
/**
This function initialize the random number generator 
with current time for seed.
**Note: ** You are not supposed to use this function.
**/

__rayapi GLfloat raydium_random_pos_1 (void);
/**
"positive, to one": 0 <= res <= 1
**/

__rayapi GLfloat raydium_random_neg_pos_1 (void);
/**
"negative and positive, one as absolute limit": -1 <= res <= 1
**/

__rayapi GLfloat raydium_random_0_x (GLfloat i);
/**
"zero to x": 0 <= res <= x
**/

__rayapi GLfloat raydium_random_f (GLfloat min, GLfloat max);
/**
min <= res <= max (float)
**/

__rayapi int raydium_random_i (int min, int max);
/**
min <= res <= max (integer)
**/

__rayapi signed char raydium_random_proba (GLfloat proba);
/**
Returns true or false (0 or 1) depending of "proba" factor.
##proba## must be: 0 <= proba <=1
ex: 50% = 0.5
**/

#endif
