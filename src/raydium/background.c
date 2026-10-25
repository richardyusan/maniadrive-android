/*
    Raydium - CQFD Corp.
    http://raydium.org/
    License: GPL - GNU General Public License, see "gpl.txt" file.
*/

#ifndef DONT_INCLUDE_HEADERS
#include "index.h"
#else
#include "headers/background.h"
#endif 

void raydium_background_color_change(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
raydium_background_color[0]=r;
raydium_background_color[1]=g;
raydium_background_color[2]=b;
raydium_background_color[3]=a;
//glColor4fv(raydium_background_color);
raydium_clear_color_update();
raydium_fog_color_update();
}
