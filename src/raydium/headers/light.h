#ifndef _LIGHT_H
#define _LIGHT_H
/*=
Lights
900
**/

// Introduction to Raydium light system
/**
When we starts Raydium development, the main idea was to use native OpenGL
lights, and not lightmaps or another method.

This method (native lights) provides 8 simultaneous movable lights, 
and is quite effective with recent OpenGL hardware.

You can modify intensity, position, color, you can turn on any light at 
any time, make them blinking... Mixing all theses features can result 
many effects, as realtime sunset, flashing lights for cars, explosions, ...

Usage is very easy: no need to create lights, just turn them on. 

See also: LightMaps
**/

__rayapi void raydium_light_enable (void);
/**
Obvious.
**/

__rayapi void raydium_light_disable (void);
/**
Obvious.
**/

__rayapi signed char raydium_light_texture(int texture, signed char enable);
/**
Texture ##l## will not use lighting if ##enable## is set to 0. Call this
function **before** loading any object using this texture, because
of display lists. Same way, it's not possible to change back this value 
after the first object drawing without disabling display lists.
**/

__rayapi signed char raydium_light_texture_name(char *name, signed char enable);
/**
Same as above, but using texture ##name##.
**/

__rayapi GLuint raydium_light_to_GL_light (GLuint l);
/**
Probably useless for end user. (internal uses)
**/

__rayapi void raydium_light_on (GLuint l);
/**
Turns ##l## light on ( 0 <= l <= RAYDIUM_MAX_LIGHTS )
**/

__rayapi void raydium_light_off (GLuint l);
/**
Turns ##l## light off
**/

__rayapi void raydium_light_switch (GLuint l);
/**
Will swith ##l## light state (from "on" to "off", for example).
**/

__rayapi void raydium_light_update_position (GLuint l);
/**
Updates ##raydium_light_position[l]## array changes to hardware.
This function is now used internaly by Raydium,
so you have no reasons to call it by yourself.
**/

__rayapi void raydium_light_update_position_all (void);
/**
See above.
**/

__rayapi void raydium_light_update_intensity (GLuint l);
/**
See above.
**/

__rayapi void raydium_light_update_all (GLuint l);
/**
See above.
**/

__rayapi void raydium_light_move (GLuint l, GLfloat * vect);
/**
Moves light to position ##vect## for light ##l## (vect is GLfloat[4]: x,y,z,dummy).

Just move your lights before camera placement, or your changes 
will be applied to the next frame only.
**/

__rayapi void raydium_light_move_3f(GLuint l,GLfloat px, GLfloat py, GLfloat pz);
/*
Same as above, but using 3 GLfloat values
*/

__rayapi void raydium_light_conf_7f(GLuint l,GLfloat px, GLfloat py, GLfloat pz, GLfloat intensity, GLfloat r, GLfloat g, GLfloat b);
/*
Full settings for light ##l##: position, intensity and color, using GLfloat
values.
*/

__rayapi void raydium_light_reset (GLuint l);
/**
This function will restore all defaults for ##l## light.
**/

__rayapi void raydium_light_blink_internal_update (GLuint l);
/**
Useless for end-user.
**/

__rayapi void raydium_light_blink_start (GLuint l, int fpc);
/**
Makes ##l## light blinking at ##fpc## (frames per cycle) rate.
This function will use timecalls soon ("fpc" -> "hertz")
**/

__rayapi void raydium_light_callback (void);
/**
Useless for end-user.
**/

#endif
