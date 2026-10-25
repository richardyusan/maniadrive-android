/*
    Raydium - CQFD Corp.
    http://raydium.org/
    License: GPL - GNU General Public License, see "gpl.txt" file.
*/

#ifndef DONT_INCLUDE_HEADERS
#include "index.h"
#else
#include "headers/random.h"
#endif 

void raydium_random_randomize(void)
{
srand(time(NULL));
}

GLfloat raydium_random_pos_1(void)
{
return (GLfloat) rand() / (GLfloat) RAND_MAX;
}

/* de -1.0 a 1.0 */
GLfloat raydium_random_neg_pos_1(void)
{
return (2.f * raydium_random_pos_1()) - 1.f;
}

GLfloat raydium_random_0_x(GLfloat i)
{
return(raydium_random_pos_1() * i);
}

GLfloat raydium_random_f(GLfloat min, GLfloat max)
{
return( (raydium_random_pos_1() * (max-min)) + min);
}

int raydium_random_i(int min, int max)
{
return (rand()%(max-min+1))+min;
}


signed char raydium_random_proba(GLfloat proba)
{
GLfloat min;
GLfloat max;

max=2*proba;
min=max-2;

return raydium_random_f(min,max)>=0;
}
