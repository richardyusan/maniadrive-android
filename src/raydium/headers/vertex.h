#ifndef _VERTEX_H
#define _VERTEX_H

/*=
vertices
1700
**/

// Introduction
/**
You can create objets at runtime, if needed, using the following functions.
Each of theses functions adds only one vertex so, obviously, you need to 
call three time the same function to add one triangle. 
**/


__rayapi void raydium_vertex_add (GLfloat x, GLfloat y, GLfloat z);
/**
Adds a vertex at (##x,y,z##).
**/

__rayapi void raydium_vertex_uv_add (GLfloat x, GLfloat y, GLfloat z, GLfloat u, GLfloat v);
/**
Same as above, but providing texture mapping informations with ##u## and ##v##.
**/

__rayapi void raydium_vertex_uv_normals_add (GLfloat x, GLfloat y, GLfloat z, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat u, GLfloat v);
/**
Same as above, giving vertex's normal with (##nx,ny,nz##).
**/

#endif
