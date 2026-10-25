#ifndef _NORMAL_H
#define _NORMAL_H

/*=
Normals
1600
**/

// Introduction
/**
This file provides some usefull functions for normal generation and smoothing.
You can find some more informations about normals at the top of this guide. 
**/


__rayapi void raydium_normal_generate_lastest_triangle (int default_visu);
/**
Generate normal for the last created triangle (see ##raydium_vertex_index##)
if ##default_visu## is true ( != 0 ), this function will restore "visu" 
normals too.
**/

__rayapi void raydium_normal_restore_all (void);
/**
This function restore visu normals with standard ones (##raydium_vertex_normal_*##)
**/

__rayapi void raydium_normal_regenerate_all (void);
/**
This function will regenerate standard and visu normals for the whole 
scene (ground, objects, ...).
**/

__rayapi void raydium_normal_smooth_all (void);
/**
This function will smooth the whole scene, using adjacent vertices.
Note this function can take a lot of time.
**/

void raydium_normal_smooth_from_to(GLuint from, GLuint to);
/**
Same as above, but only from ##from## vertex to ##to## vertex (excluded).
In other words: will smooth [from;to[
**/


#endif
