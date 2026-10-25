/*
    Raydium - CQFD Corp.
    http://raydium.org/
    License: GPL - GNU General Public License, see "gpl.txt" file.
*/

#ifndef DONT_INCLUDE_HEADERS
#include "index.h"
#else
#include "headers/mouse.h"
#endif 

/*
#define raydium_mouse_hide() glutSetCursor(GLUT_CURSOR_NONE);
#define raydium_mouse_show() glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
#define raydium_mouse_move(x,y) glutWarpPointer(x,y)
*/

signed char raydium_mouse_isvisible(void)
{
int ret;
ret=glutGet(GLUT_WINDOW_CURSOR);
return (ret==GLUT_CURSOR_NONE?0:1);
}

void raydium_mouse_init(void)
{
if(raydium_window_mode==RAYDIUM_RENDERING_NONE)
    return;
memset(raydium_mouse_button,0,3);
raydium_mouse_click=0;
raydium_mouse_x=raydium_mouse_y=raydium_window_ty=0;
raydium_log("mouse: OK");
raydium_mouse_hide();
}


void raydium_mouse_click_callback(int but,int state,int x,int y)
{
unsigned char n=0,s=0;
signed char special=0;


if(but==GLUT_LEFT_BUTTON) n=0;
else if(but==GLUT_RIGHT_BUTTON) n=1;
else if(but==GLUT_MIDDLE_BUTTON) n=2;
else 
    {
    special=1;
    n=but;
    }

if(state==GLUT_DOWN) s=1;
if(state==GLUT_UP) s=0;
if(s) raydium_mouse_click=n+1;
if(!special)
    raydium_mouse_button[n]=s;
}


void raydium_mouse_move_callback(int x, int y)
{
if(x>=0 && x<raydium_window_tx) raydium_mouse_x=x;
if(y>=0 && y<raydium_window_ty) raydium_mouse_y=y;
}

int raydium_mouse_button_pressed(int button)
{
return raydium_mouse_button[button];
}
