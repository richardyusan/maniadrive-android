#ifndef _OBJECT_H
#define _OBJECT_H
/*=
Objects
2300
**/

// Introduction
/**
With the following functions, you can easily draw and manage
mesh objects (.tri file).
**/

__rayapi GLint raydium_object_find (char *name);
/**
Lookups an object by its ##name##. This function will return -1 if the
object's not found, and will not try to load the .tri file.
**/

__rayapi signed char raydium_object_isvalid(int obj);
/**
Internal use, but you can call this function if you want to verify if an
object id is valid (in bounds).
**/

__rayapi GLint raydium_object_find_load (char *name);
/**
Same as above (##raydium_object_load##), but will try to load object.
**/

__rayapi void raydium_object_reset (GLuint o);
/**
Internal use. Do not call.
**/

__rayapi int raydium_object_load (char *filename);
/**
Load ##filename## as a .tri file, and returns corresponding id, or
-1 in case of error.
**/

__rayapi void raydium_object_draw (GLuint o);
/**
Draws ##o## (index) object, using current matrixes.
**/

__rayapi void raydium_object_draw_name (char *name);
/**
Same as above, but you only have to provide object's ##name## (".tri file"). 
If this object was not already loaded, this function will do it for you.
**/

__rayapi void raydium_object_deform (GLuint obj, GLfloat ampl);
/**
Early devel state. Useless as is.
**/

__rayapi void raydium_object_deform_name (char *name, GLfloat ampl);
/**
Early devel state. Useless as is.
**/

__rayapi GLfloat raydium_object_find_dist_max (GLuint obj);
/**
This function will return will return the distance form (0,0,0) 
to the farest point of ##obj## object.
**/

__rayapi void raydium_object_find_axes_max (GLuint obj, GLfloat * tx, GLfloat * ty, GLfloat * tz);
/**
This function returns the (maximum) size of the bounding box 
of ##obj## (relative to (0,0,0)).
**/

__rayapi void raydium_object_find_minmax(GLuint obj, GLfloat *min, GLfloat *max);
/**
Returns min and max values for ##obj##. No memory allocation is done, you must
provide two GLfloat[3] array.
**/

__rayapi void raydium_object_find_center_factors(GLuint obj, GLfloat *tx, GLfloat *ty, GLfloat *tz);
/**
Returns "centering" factors for ##obj##. A centered object will return (0,0,0).
**/

__rayapi void raydium_object_callback(void);
/**
Internal (frame callback).
**/


// Animations
/**
Raydium now supports mesh animation, thru MD2 (Quake 2) files. Raydium file
format was extended to version 2. If you want to create an animated mesh
for Raydium from a MD2 file, you may use Blender with "import-md2-0.14.py" 
script ( by Bob Holcomb, http://67.22.114.230:8082/programming/blender/index.html ) 
and export it back to a tri file using provided "triEXP-MD2-*.py" script.
All other tasks (loading, transformations, ...) are done the same way as
regular static mesh.

For Raydium, an animation is a set of "anims", and each "anim" is a set 
of "frames". Each "anim" gets its own name (see header of a version 2 file
for more informations), and since an animated object may be use for many 
players, Raydium provides an "instances" based system: setting things like
anim and frame for an object is done only for one instance of this object.
Instances are always available, no need to create or declare them.
That's all you need to use animation simple API.
**/

__rayapi GLint raydium_object_anim_find(int object, char *name);
/**
Lookups an animation by its ##name##. This function will return -1 if the
animation's not found. Mostly for internal use.
**/

__rayapi void raydium_object_anim_generate_internal(int object, int instance);
/**
Internal. Transformed mesh generation.
**/

__rayapi void raydium_object_anim_frame(int object, int instance, GLfloat frame);
/**
Sets current ##frame## for one ##instance## of ##object##. ##frame## is
automatically bounded and looped.
Warning, change anim **before** anim's frame.
**/

__rayapi void raydium_object_anim_frame_name(char *object, int instance, GLfloat frame);
/**
Same as above, but using ##object##'s name.
**/

__rayapi void raydium_object_anim(int object, int instance, int anim);
/**
Sets current ##anim## for one ##instance## of ##object##.
Again, change anim **before** anim's frame.
**/

__rayapi void raydium_object_anim_name(char *object, int instance, char *anim);
/**
Same as above, but using ##object##'s name and ##anim##'s name.
**/

__rayapi void raydium_object_anim_instance(int object, int instance);
/**
With this function, you must set what instance will be drawn when
##raydium_object_draw()## will be called with ##object## argument.

Default is set to instance 0.
**/

__rayapi void raydium_object_anim_instance_name(char *object, int instance);
/**
Same as above, but using ##object##'s name.
**/

__rayapi void raydium_object_anim_automatic(int object, int anim, GLfloat factor);
/**
With this function, you can set an automatic frame increment for a specific 
##anim## of an ##object##. This increment is based on frame time and ##factor##.
**/

__rayapi void raydium_object_anim_automatic_name(char *object, char *anim, GLfloat factor);
/**
Same as above, but using ##object##'s name and ##anim##'s name.
**/

// "Punctually" anims
/**
When using animations, you're switching for an "anim" to another, and an
"anim" will loop forever. "Punctually" support will allow you to set a
default "anim" for an object and to do switch punctually to another "anim",
and automatically return back to default value when this "anim" is finished,
usefull for animations like jumps, kick, ...
**/

__rayapi void raydium_object_anim_default(int object, int anim);
/**
This function will set default ##anim## for ##object##.
**/

__rayapi void raydium_object_anim_punctually(int object, int anim, int instance);
/**
This function will trigger a punctually ##anim## for ##object##'s ##instance##.
**/

__rayapi void raydium_object_anim_punctually_name(char *object, char *anim, int instance);
/**
Same as above, but with object's name.
**/

__rayapi signed char raydium_object_anim_ispunctually(int object, int instance);
/**
Will return true (1) if ##object## is currently running a punctually animation,
or false (0) otherwise.
**/

__rayapi signed char raydium_object_anim_ispunctually_name(char *object, int instance);
/**
Same as above, but with object's name.
**/

#endif
