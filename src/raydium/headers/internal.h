#ifndef _INTERNAL_H
#define _INTERNAL_H

/*=
"Internal" informations access
2000
**/

__rayapi  void raydium_internal_dump (void);
/**
This function is now systematically called by Raydium at application's exit,
displaying some informations about loaded textures, objects, registered data,
network statistics.
**/

__rayapi  void raydium_internal_dump_matrix (int n);
/**
Dumps matrix to console.
##n## values are:
%%
0 for GL_PROJECTION_MATRIX
1 for GL_MODELVIEW_MATRIX
%%
**/

#endif
