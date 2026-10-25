#ifndef _SHADER__H
#define _SHADER__H
#include "../shader.h"

/*=
Shaders
4500
**/

// Introduction
/**
Raydium provides a support for OpenGL Shading Language (GLSL). 
This documentation talks only about Raydium Shader API, and not about
the Shading Language itself. With Raydium, shaders works by two: you must
provide a vertex shader and a fragment shader each time. This is a very
usual way to do. 

You must know that **only one** shader can be active at a time.
Once a shader is loaded, Raydium API allows you to attach this shader to
a texture, so you don't have to deal manually with activation/deactivation.

You can also change all "uniform" variables from shaders 
using ##raydium_shader_var_...()## functions.
Into this set, all functions that does **not** contain the **_name**
suffix **are only able to deal with current shader !**.

You can use the global variable ##raydium_shader_support## to detect if
current hardware supports GLSL or not (1=OK 0=no shader support).
**/


__rayapi void raydium_shader_init(void);
/**
Internal use. Init all shader subsystem.
**/

__rayapi signed char raydium_shader_isvalid(int shader);
/**
Internal use. Returns true (1) if ##shader## slot is in bounds and filled.
**/

__rayapi int raydium_shader_find(char *name);
/**
Returns shader's ID using its ##name##.
**/

__rayapi void raydium_shader_infolog(GLhandleARB shader);
/**
Internal use.
Reports full driver error message when shader compilation or linking fails.
**/

__rayapi int raydium_shader_load(char *name, char *file_vert, char *file_frag);
/**
Loads the vertex shader ##file_vert## and the fragment shader ##file_frag##.
The shader is stored with the provided ##name##. This function returns the
shader ID or -1 in case of failure.
**/

__rayapi int raydium_shader_variable(int shader, char *name);
/**
Returns an ID for the variable "##name## of the provided ##shader##.
**/

__rayapi signed char raydium_shader_var_i(int var_id, int value);
/**
This function will change the ##value## of the variable ##var_id## of
the current shader.
Value is an integer.
**/

__rayapi signed char raydium_shader_var_i_name(char *shader, char *variable, int value);
/**
Same as above, but using shader's name and variable's name. This function is
able to change the ##variable##'s ##value## even is the ##shader## is not
the current one.
**/

__rayapi signed char raydium_shader_var_f(int var_id, float value);
/**
This function will change the ##value## of the variable ##var_id## of
the current shader.
Value is a float.
**/

__rayapi signed char raydium_shader_var_f_name(char *shader, char *variable, float value);
/**
Same as above, but using shader's name and variable's name. This function is
able to change the ##variable##'s ##value## even is the ##shader## is not
the current one.
**/

__rayapi signed char raydium_shader_var_2f(int var_id, float value1, float value2);
/**
This function will change the ##value## of the variable ##var_id## of
the current shader.
Value is an "array" of 2 floats (vec2).
**/

__rayapi signed char raydium_shader_var_2f_name(char *shader, char *variable, float value1, float value2);
/**
Same as above, but using shader's name and variable's name. This function is
able to change the ##variable##'s ##value## even is the ##shader## is not
the current one.
**/

__rayapi signed char raydium_shader_var_3f(int var_id, float value1, float value2, float value3);
/**
This function will change the ##value## of the variable ##var_id## of
the current shader.
Value is an "array" of 3 floats (vec3).
**/

__rayapi signed char raydium_shader_var_3f_name(char *shader, char *variable, float value1, float value2, float value3);
/**
Same as above, but using shader's name and variable's name. This function is
able to change the ##variable##'s ##value## even is the ##shader## is not
the current one.
**/

__rayapi signed char raydium_shader_var_4f(int var_id, float value1, float value2, float value3, float value4);
/**
This function will change the ##value## of the variable ##var_id## of
the current shader.
Value is an "array" of 4 floats (vec4).
**/

__rayapi signed char raydium_shader_var_4f_name(char *shader, char *variable, float value1, float value2, float value3, float value4);
/**
Same as above, but using shader's name and variable's name. This function is
able to change the ##variable##'s ##value## even is the ##shader## is not
the current one.
**/


__rayapi signed char raydium_shader_current(int shader);
/**
This function will change the current active shader with ##shader##.
To disable a shader and get back to regular OpenGL fixed function pipeline,
set ##shader## value to ##-1##.
**/

__rayapi signed char raydium_shader_current_name(char *shader);
/**
Same as above, but using shader's name.
**/

__rayapi signed char raydium_shader_attach_texture(int shader, int texture);
/**
During rendering, each time the ##texture## will be used by any object,
the ##shader## will be activated.
**/

__rayapi signed char raydium_shader_attach_texture_name(char *shader, char *texture);
/**
Same as above, but using shader's name and texture's name.
**/

#endif
