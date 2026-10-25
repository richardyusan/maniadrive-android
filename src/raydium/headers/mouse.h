#ifndef _MOUSE_H
#define _MOUSE_H
/*=
Mouse
1100
**/

// Introduction
/**
Mouse API is almost explainded at the top of this guide, but here it 
is some other usefull functions (macros, in facts)
**/


#define raydium_mouse_hide() glutSetCursor(GLUT_CURSOR_NONE);
/**
Hides mouse cursor.
**/

#define raydium_mouse_show() glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
/**
Shows mouse cursor.
**/

#define raydium_mouse_move(x,y) glutWarpPointer((x),(y))
/**
Moves cursor to (##x##,##y##) position (in pixel).
Example if you want to move cursor at window's center: 
%%(c)raydium_mouse_move(raydium_window_tx/2, raydium_window_ty/2);%%
**/

__rayapi signed char raydium_mouse_isvisible(void);
/**
Returns true or false (0 or 1), if the mouse is visible or not.
See ##raydium_mouse_show()## and ##raydium_mouse_hide()## above.
**/

__rayapi void raydium_mouse_init (void);
/**
Internal use.
**/

__rayapi void raydium_mouse_click_callback (int but, int state, int x, int y);
/**
Internal callback.
**/

__rayapi void raydium_mouse_move_callback (int x, int y);
/**
Internal callback.
**/

__rayapi int raydium_mouse_button_pressed (int button);
/**
returns ##button## state. (See first part of this document)
**/

#endif
