#ifndef _KEY_H
#define _KEY_H
/*=
Keyboard & keys
1000
**/

// Introduction
/*=
Keyboard API is already described at the top of this guide,
see section "keyboard input".
**/

__rayapi  void raydium_key_normal_callback (GLuint key, int x, int y);
/**
Internal callback.
**/

__rayapi  void raydium_key_special_callback (GLuint key, int x, int y);
/**
Internal callback.
**/

__rayapi  void raydium_key_special_up_callback (GLuint key, int x, int y);
/**
Internal callback.
**/

__rayapi  int raydium_key_pressed (GLuint key);
/**
Will return state of ##key## in the ##raydium_keys[]## array.
This function is usefull to test keyboard from PHP, since RayPHP doest not
support array for now.
**/

#endif
