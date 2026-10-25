#ifndef _FILE_TRI_H
#define _FILE_TRI_H
/*=
Files (TRI format)
2101
**/

// Warning
/**
It's important to use only functions with ##raydium_file_*## prefix.
All other functions may change or disappear. Upper level functions are
available (see ##object.c##).
**/

// Introduction
/**
##file.c## use .tri mesh files (text), available in 4 versions:

1. version 1: providing normals and uv texture mapping informations.
2. version 0: providing uv texture mapping.
3. version -1: only providing vertices. 
4. version 2: mesh animation support
	 
Version 1 example file:
%%
1
5.1 15.75 -3.82 0.0000 0.0000 -1.0000 0.5158 0.5489 rgb(0.5,0.5,0.5)
6.3 11.75 -3.82 0.0000 0.0000 -1.0000 0.5196 0.5365 rgb(0.5,0.5,0.5)
5.0 11.75 -3.82 0.0000 0.0000 -1.0000 0.5158 0.5365 rgb(0.5,0.5,0.5)
...
%%
	      
You can find the file version on first line, and then data.
Next lines: vertex position (x,y,z), normal (x,y,z), texture mapping (u,v)
and texture (string). 

Version 2 files are a bit different, as showed below:
%%
2
3 1743
0 39 stand
40 45 run
46 53 attack
1
5.1 15.75 -3.82 0.0000 0.0000 -1.0000 0.5158 0.5489 rgb(0.5,0.5,0.5)
6.3 11.75 -3.82 0.0000 0.0000 -1.0000 0.5196 0.5365 rgb(0.5,0.5,0.5)
5.0 11.75 -3.82 0.0000 0.0000 -1.0000 0.5158 0.5365 rgb(0.5,0.5,0.5)
...
%%

You may have seen that headers are longer for v2 files. You'll find (just
after the version number) how many "anims" are hosted by this file, and how
many vertices are required for one frame. Then you'll find one line per
"anim", with starting frame, ending frame and anim's name.
Then starts a regular tri file ("sub-file", with its own version number)
with ALL concatened frames.
**/


#define DONT_SAVE_DUMMY_TEXTURE
__rayapi void dump_vertex_to (char *filename);
/**
This function save all scene to filename (.tri file) in version 1.
Vertice may be sorted.
Please, try to do not use this function.
**/

__rayapi void dump_vertex_to_alpha (char *filename);
/**
Now useless and deprecated.
**/

__rayapi int raydium_file_set_textures (char *name);
/**
Internal use.
This function analyze texture filename, and search for extended multitexturing
informations (u,v and another texture).
**/

__rayapi void read_vertex_from (char *filename);
/**
Loads filename. Again, avoid use of this function.
**/

#endif
