#ifndef _CAMERA_H
#define _CAMERA_H

/*=
Camera
2200
**/

// Introduction
/**
Raydium provides camera management functions, allowing the coder to
move camera with very simple functions, even for complex moves.
You have to place your camera once per frame (not more, not less).

"look_at" style functions can be affected by ##raydium_camera_look_at_roll##
global variable, if needed.

A few words about camera path: Take a look to a .cam file if you want to
understand this simple file format, but you probably only need the ##cam.c##
application, dedicated to camera path creation.

Some camera functions are provided by physics module, see suitable chapter.
**/


__rayapi void raydium_camera_vectors (GLfloat * res3);
/**
This function will return two vectors (2 * 3 * GLfloat), giving the camera 
orientation (front vector and up vector). At this day, the up vector is 
always the same as the world up vector, even if the camera is rotated 
or upside down (and yes, this MUST be corrected :).

Designed for internal uses, before all.
**/

__rayapi void raydium_camera_internal_prepare(void);
/**
Internal use. (pre)
**/

__rayapi void raydium_camera_internal (GLfloat x, GLfloat y, GLfloat z);
/**
Internal use. (post)
**/

__rayapi void raydium_camera_place (GLfloat x, GLfloat y, GLfloat z, GLfloat lacet, GLfloat tangage, GLfloat roulis);
/**
Sets the camera at (x,y,z) position, and using (lacet,tangage,roulis) 
as rotation angles.
**/

__rayapi void raydium_camera_look_at (GLfloat x, GLfloat y, GLfloat z, GLfloat x_to, GLfloat y_to, GLfloat z_to);
/**
Sets the camera at (x,y,z) position, and looks at (x_to,y_to,z_to).
**/

__rayapi void raydium_camera_replace (void);
/**
You'll need to reset camera position and orientation after each object drawing.
If this is unclear to you, read the "example" section, below.

You will need to make your own 3D transformations (GLRotate, GLTranslate, 
...) to draw your objects, or you can use the following function.
**/

__rayapi void raydium_camera_replace_go (GLfloat * pos, GLfloat * R);
/**
This function will replace the camera, as ##raydium_camera_replace()##,
but will place "3D drawing cursor" at position ##pos## (3 GLfloat) with 
rotation ##R## (4 GLfloat quaternion).

No eulers (rotx, roty, rotz) version of this function is provided for now.. 
Do you really need it ?
**/

// Example of camera use
/**
1. place camera
2. move "drawing cursor" to object's place
3. draw object
4. reset camera to initial place (the one given at step 1)
5. move "drawing cursor" to another object's place
6. draw another object
7. [...]

Steps 4 and 5 can be done with raydium_camera_replace_go().
**/

__rayapi void raydium_camera_rumble(GLfloat amplitude, GLfloat ampl_evo, GLfloat secs);
/**
Camera (any type) will rumble for ##secs## seconds, with ##amplitude## (radians).
This ##amplitude## will be incremented of ##ampl_evo## every second (negative
values are allowed for ##ampl_evo##).
An ##amplitude## is always positive.
**/

__rayapi void raydium_camera_smooth (GLfloat px, GLfloat py, GLfloat pz, GLfloat lx, GLfloat ly, GLfloat lz, GLfloat zoom, GLfloat roll, GLfloat step);
/**
Smooth style clone of ##raydium_camera_look_at##.
Roll is given by ##roll## and not global variable ##raydium_camera_look_at_roll##
as for regular look_at function.
##zoom## is the requested FOV.
Play with step to modify smoothing level of the movement. A good way to use
this function is the following usage :
%%(c) raydium_camera_smooth(cam[0],cam[1],cam[2],pos[1],-pos[2],pos[0],70,0,raydium_frame_time*3); %%
**/

__rayapi void raydium_camera_path_init (int p);
/**
Internal use.
**/

__rayapi void raydium_camera_path_init_all (void);
/**
Internal use.
**/

__rayapi int raydium_camera_path_find (char *name);
/**
Lookups path's id using filename ##name##.
This function will not try to load a camera path if it's not found, and
will return -1.
**/

__rayapi int raydium_camera_path_load (char *filename);
/**
Obvious : use this function to load a camera path.
**/

__rayapi void raydium_camera_path_draw (int p);
/**
Draws ##p## camera path, as red lines. This must be done at each frame. 
**/

__rayapi void raydium_camera_path_draw_name (char *path);
/**
Same as above, but using camera path's name.
**/

__rayapi signed char raydium_camera_smooth_path (char *path, GLfloat step, GLfloat * x, GLfloat * y, GLfloat * z, GLfloat * zoom, GLfloat * roll);
/**
Returns the (##x,y,z##) point of the camera path for step ##step##, using
provided ##zoom## (FOV) and ##roll## angle.
It's important to note that ##step## is a float.
Mostly for internal use.
**/

__rayapi void raydium_camera_path_reset(void);
/**
Next smooth call will be instantaneous.
**/

__rayapi void raydium_camera_smooth_path_to_pos (char *path, GLfloat lx, GLfloat ly, GLfloat lz, GLfloat path_step, GLfloat smooth_step);
/**
"Camera on path looking at a point".
Simple ##raydium_camera_smooth## version: give a path name, a "look_at" 
point (##lx,ly,lz##), a current ##step##, anda ##smooth_step## time 
factor (see ##raydium_camera_smooth## example above).
**/

__rayapi void raydium_camera_smooth_pos_to_path (GLfloat lx, GLfloat ly, GLfloat lz, char *path, GLfloat path_step, GLfloat smooth_step);
/**
"Camera on point looking at a path".
Same style as previous function.
**/

__rayapi void raydium_camera_smooth_path_to_path (char *path_from, GLfloat path_step_from, char *path_to, GLfloat path_step_to, GLfloat smooth_step);
/**
"Camera on a path looking at another path".
Same style as previous functions.
**/

#endif
