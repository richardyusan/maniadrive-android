#ifndef _RENDER_H
#define _RENDER_H
/*=
Rendering
1300
**/

// Introduction
/*=
render.c contains Raydium rendering core, so only "public" and 
interesting function will be documented.

It' obvious for me that many parts of this code have to be 
rewritten (tips: slow, buggy, old, ... :) 
**/

__rayapi void raydium_render_lightmap_color(GLfloat *color);
/**
You may force a new lightmap rendering color "filter" anytime with this
function, allowing advanced lighting effects.
HUGE WARNING: You must turn off display lists if you change this value after
first object's render.
See ##raydium_rendering_displaylists_disable()## if needed.
**/

__rayapi void raydium_render_lightmap_color_4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
/**
Same as above, using 4 values.
**/

__rayapi int  raydium_rendering_prepare_texture_unit (GLenum tu, GLuint tex);
/**
This function will "prepare" hardawre texture unit ##tu## to render ##tex## texture.
There almost no reason to call this function by yourself.
**/

__rayapi void raydium_rendering_internal_prepare_texture_render (GLuint tex);
/**
Same as above, but for texture unit #0 only.
**/

__rayapi void raydium_rendering_internal_restore_render_state (void);
/**
Internal. Deprecated.
**/

// DO NOT DOCUMENT THIS ... THING !
__rayapi char infov (GLfloat x, GLfloat y);

__rayapi void raydium_rendering_from_to_simple(GLuint from, GLuint to);
/**
Same as ##raydium_rendering_from_to()##, but only with vertices (no
UV, no normals, no textures, no colors, ...).
Mostly used for internal shadow maps creation.
**/

__rayapi void raydium_rendering_from_to (GLuint from, GLuint to);
/**
Renders vertices from ##from## to ##to##.
Using object management functions is a better idea.
**/

__rayapi void raydium_rendering (void);
/**
Renders all vertices (probably useless, now).
**/

__rayapi void raydium_rendering_finish (void);
/**
You must call this function at the end of each frame. This will flush all
commands to hardware, fire a lot off callbacks, and prepare next frame.
**/

__rayapi void raydium_rendering_wireframe (void);
/**
Switch to wireframe rendering.
**/

__rayapi void raydium_rendering_normal (void);
/**
Switch back to standard rendering.
**/

__rayapi void raydium_rendering_rgb_force (GLfloat r, GLfloat g, GLfloat b);
/**
Force all RGB colored vertices to take ##(r,g,b)## color. One example of this
use is for making "team colored" cars : Do not apply textures to some faces
while modelling, and force to team color each time you render a car.
**/

__rayapi void raydium_rendering_rgb_normal (void);
/**
Disable "rgb force" state. See above.
**/

__rayapi void raydium_rendering_displaylists_disable(void);
/**
Disable display lists usage.
Some old video cards and broken drivers may get better performances WITHOUT
display lists (on large objects, mainly).
**/

__rayapi void raydium_rendering_displaylists_enable(void);
/**
Enable display lists usage. default state.
**/

#endif
