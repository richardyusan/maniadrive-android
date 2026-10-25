/*=
Introduction to Raydium
100
**/

// About
/**
Well, first of all, let me talk about [[Raydium]] goals: this project 
aims to be simple, easy to use, portable, and quite fast.

[[Raydium]] is a C written abstract layer, on top of OpenGL, 
and [[GLU]]: this means you can write an entire 3D 
application without calling any OpenGL function. 
Want to draw an object ? call the suitable [[Raydium]] function, 
and all textures and vertices will be loaded, and your object drawn.
Want to make an explosion ? Same thing: call the right function.
Note that you can call OpenGL functions anyway, if necessary.

About portability, I can say a few things: [[Raydium]] was initially 
planned for Linux only, but with a "clean" (nearly [[ANSI]]) code,
and, in facts, we have been able to compile Raydium under Visual Studio (Windows)
and mingw with a very few modifications.
So you can expect a correct result on any system providing 
OpenGL (at least 1.2), [[GLU]] and a C compiler. Using Raydium as a shared
library (.so or DLL), you can also use C++ language for you own applications

As we ([[CQFD Corp]].) needed a library for our own games, demos,
and... and things like that, and as I was interested by OpenGL,
I starts to write [[Raydium]].

Raydium is perfect for outdoors spaces, integrating a landscape engine,
with suitable physic, supports dynamic lighting, fog, blending, water 
and waves, reflections, and more, but also provides everything for indoor,
with radiosity lightmaps for example.

Some other advanced features are available : physics, scripting,
live video, transparent networking, GUI, shaders, ...

This features list will probably grow up during Raydium developpement, see
Raydium website: http://raydium.org/

You'll find, in this document, a list of many functions and possibilities 
of [[Raydium]], but if it's your first view of Raydium, you should
start with tutorials ( http://wiki.raydium.org/wiki/RaydiumTutorials ) and
packaged demo programs.

After this short introduction, let's talk about the [[API]] itself, 
starting with the main file (from the programmer's point of vue) 
of [[Raydium]]: common.c
**/


// Defines
/**
As mentioned above, the file common.c is quite interesting, 
for several reasons: first, as this file includes all others [[Raydium]]'s
files, you can have an overview of the whole project, just by looking at this.

It can also be used as a "quick help", since all variables are declared 
here, and not in the corresponding files. I mean, for example, 
that "##raydium_light_intensity...##" will be declared in common.c, 
not in light.c . There's many reasons for using such "style",
but you must only retain that it is simpler for you :)

Ok, after this little disclaimer, we can have a look to the first part
of our file.

After usual #include (nothing interesting here), we find some #defines.

===generic limits===

The first #define block determine limits of your application,
and here you are the actual values for basic defines:
%%(c)
#define RAYDIUM_MAX_VERTICES 500000
#define RAYDIUM_MAX_TEXTURES 256
#define RAYDIUM_MAX_LIGHTS 8
#define RAYDIUM_MAX_NAME_LEN 255
#define RAYDIUM_MAX_OBJECTS 1024
%%

- As you may expect, ##MAX_VERTICES## defines the amount of memory you'll
waste with vertex tables. These tables will contain all loaded objects, 
then remember each time you draw something (object), 
[[Raydium]] loads it (if not already done). Currently, there is no "delete"
mechanism implemented (except by deleting all objects). 
Let me give you a scale: with an Athlon XP1900+, [[GeForce]] 3, 
actual [[Raydium]] devel. version 0.31, with around 100 000 vertices, 
losts of options (sky, blending, 2 lights, 15 textures, ...), 
Raydium renders ~ 45 FPS. Beyond this, a very correct object uses less
than 10 000 vertices. So 500 000 vertices, the actual default, 
is quite large. It's also important to talk about memory: Linux is 
very efficient on this point, and allocates only "really used" memory. 
Under Linux, with the above scene, Raydium used about 20 MB (data only), 
instead of "much more" (~ 5x). I haven't made any test about this under 
Windows, but we can expect worse results.
 
- There's nothing really important to say about ##MAX_TEXTURES##,
since that doesn't influence the amount of memory used. You are not 
limited to 8 bits values, but 256 seems very comfortable (and you must
pay attention to the capacities of your 3D hardware !)
 
- The next define, ##MAX_LIGHTS## is very important: OpenGL, for now 
(version 1.3 and lower), impose 8 lights at least, and all current 
hardware doesn't manage more. If this situation is likely to evolve, 
we will move this #define to a variable, and will ask hardware for its 
capacities at initialization, but, for the moment, do not exceed 8.
 
- Next, ##NAME_LEN##, limits the maximum length of strings (textures and 
objects names) used by Raydium. Default value should be perfect. 
(avoid higher values, since it could slow down name searches)
 
- ##MAX_OBJECTS## use the same mechanism as ##MAX_TEXTURES##, and addition 
with the fact that hardware is not concerned, it can be ignored. 
     
===Options and parameters===
     
This is the next part of our #define section, I will not explain these
constants here, but in respective sections, so you'll have just you to
remember they're declared here.
**/     

// Basic vars
/**

This section aims to describe each variable [[Raydium]] use, one by one. 
Some (most ?) of them are used internaly only, but you could need to access 
it. Moreover, you'll better understand how Raydium works by looking at 
these variables.

===Keyboard input===

Following variables can be found:

##raydium_key_last## will always contains the last key (normal or special) 
pressed down. You'll find a explanation about normal and special keys above.

##raydium_key[]## hosts all special keys state. Currently, you must use 
[[GLUT]] define's (Raydium aliases will come soon), limited to
following keys:

- ##GLUT_KEY_F1## to ##GLUT_KEY_F12##
- ##GLUT_KEY_LEFT##, ##GLUT_KEY_RIGHT##, ##GLUT_KEY_UP##, ##GLUT_KEY_DOWN##
- ##GLUT_KEY_PAGE_UP##, ##GLUT_KEY_PAGE_DOWN##
- ##GLUT_KEY_HOME##, ##GLUT_KEY_END##, ##GLUT_KEY_INSERT##
    
These are "special" keys: they have 2 states. released (0), 
and pressed (non zero). It means you can do something
(move an object, turn on a light) **UNTIL** user stops to press the key. 
"Normal" keys have a different behavior: you can do something **IF** user 
press a key (exit from application if ESC is pressed, for example).
You'll have no information about key's release.
    
A normal key is sent through ##raydium_key_last##, a special one through 
##raydium_key[]## AND ##raydium_key_last##.
    
You must see ##raydium_key_last## as an "event", fired when the user press 
a key (ANY key: special or not). When a normal key is pressed, you'll get
the ASCII value + 1000 assigned to ##raydium_key_last##. (1027 for "ESC", for 
example)
    
Here is a method to use special keys: 
%%(c) if(raydium_key[GLUT_KEY_UP]) move_car(); %%
    
Yes, it's easy. You can also use 
%%(c) if(raydium_key_last""==""GLUT_KEY_UP) explose(); %%
for example, if you need to carry out a specific action.
    
It's ok for you ? use ##raydium_key[]## to keep the car moving until 
user release UP key, or use ##raydium_key_last## to explode the car 
when the user tries to start it :)
    
===Mouse input===
    
Easy.
    
You can get actual mouse position on the window (relative to window's
position on screen, I mean) with ##raydium_mouse_x## and ##raydium_mouse_y##
(GLuint), starting at (0,0) for upper left 
(Warning: some [[GLUT]] implementations can give mouse position even 
when mouse is out of the window ! Check boundaries before using these values).
    
Raydium use: 1 for left button, 2 for right button, and 3 for 
middle button (0 for none) with ##raydium_mouse_click## for the last click 
value. (generated one time per click)
Raydium will now use 4 (up) and 5 (down) for mouse wheel, if any.
    
You can permanently get a button's state, up (0) or down (non zero), 
using ##raydium_mouse_button[x]##, where x is 0 for left button, 1 for right 
one, and 2 for middle button.
    
===Textures===
    
##raydium_texture_index## and ##raydium_texture_current_main## (GLuint) are used
internaly to determine repectively how many textures are loaded,
wich is the current one.
    
The next variable, ##raydium_texture_filter##, is very important. You can
assign ##RAYDIUM_TEXTURE_FILTER_NONE## (default), ##RAYDIUM_TEXTURE_FILTER_BILINEAR##
or ##RAYDIUM_TEXTURE_FILTER_TRILINEAR## (recommended).
    
Using no texture filter can gives you higher framerate on old 3D hardware,
but this is quite ugly.
    
You can activate bilinear filtering without any framerate impact on
most recent video cards, and get a much more attractive rendering.
    
Trilinear filtering uses Bilinear filtering and MipMaps. A MipMaped
texture is a duplicated texture (3 times, with Raydium), but at different 
sizes. A 512x512 texture will generate, for example, a (smoothed) 
256x256 texture, and a (smoothed) 128x128 one. Your video card will 
use these textures according to distance from POV (point of vue),
reducing flickering effect.
    
This is the best filtering Raydium can use, for a great rendering quality.
Good and recent 3D hardware can do trilinear filtering in a single pass, 
so it must be the default setting for your application.
    
About ##raydium_texture_filter## itself: changing this variable will not modify
the rendering, but the way to load textures. It means you can (for example)
use trilinear only for landscape textures, and bilinear for others.
It also means you must reload (erase) a texture to change it's filter.
    
Note that Raydium will never use trilinear filter with blended (transparent)
textures, for good reasons :)
    
Let's talk quickly about next (internal) texture variables:
##raydium_texture_blended[]## is a flag table, where each element is
non zero for a blended (RGBA) texture, and 0 for an RGB one.
    
For Raydium, when a texture does not contain a "bitmap" (texture file, 
for example), it contains a plain color, and this color is stored in 
##raydium_texture_rgb[][4]## (4 is for RGBA, values between 0 and 1).
You can load an rgb texture with "rgb" keyword. For example, instead of 
loading "red.tga", you can load "rgb(0.8,0.1,0.1)".
    
##raydium_texture_name[]## table simply contains texture filenames.
    
Last thing, ##raydium_texture_to_replace##,
can be used to erase an already loaded texture. 
Set the variable to n, and load a new texture: texture number "n" will be
replaced in memory.
    
===Projection===
    
Raydium supports 2 types of projection: ##RAYDIUM_PROJECTION_ORTHO##
(orthographic) and ##RAYDIUM_PROJECTION_PERSPECTIVE##.

First of all, let us point out what "projection" is. Using a "perspective" 
projection, closest objects will looks larger than the orthers. It is 
typically used in video games (since human eye runs like that), 
by opposition to orthographic projection, wich is mostly used by 3D 
modeling tools. The principle is simple, discover it by yourself :)
    
Raydium reads ##raydium_projection## to determine wich method to use. 
Each projection is configured with ##raydium_projection_*## variables. 
Some of these variables are used both by "perspective" and "orthographic" 
projections.
    
Here is what common.c says:
    
%%(c)
GLFLOAT RAYDIUM_PROJECTION_FOV; // PERSPECTIVE ONLY
GLFLOAT RAYDIUM_PROJECTION_NEAR; // PERSPECTIVE & ORTHO
GLFLOAT RAYDIUM_PROJECTION_FAR; // PERSPECTIVE & ORTHO
GLFLOAT RAYDIUM_PROJECTION_LEFT; // ORTHO ONLY
GLFLOAT RAYDIUM_PROJECTION_RIGHT; // ORTHO ONLY
GLFLOAT RAYDIUM_PROJECTION_BOTTOM; // ORTHO ONLY
GLFLOAT RAYDIUM_PROJECTION_TOP; // ORTHO ONLY
%%
    
You've probably noticed that orthographic projection defines a "box" 
with your screen: near, far, left, right, bottom. Everything out ouf 
this box will never be displayed.
    
Perspective projection is based on FOV: Field Of Vision, given in degrees. 
A common "human" fov is 60°, up to 90° without any noticeable deformation.
"near" and "far" are used for many things: Z-Buffer precision is affected, 
and clipping too: as with "orthographic", nothing will be displayed beyond 
"far", and fog, if enabled, will hide this "limit". This is right for "near",
too, but without fog, obviously :)
    
Also remember that decreasing FOV will zoom in.
    
You must call ##raydium_window_view_update()## after any modification on one
(or more) of these variables (see "Window Managment" section for more
information)
    
===Frame size and color===
    
##raydium_window_tx## and ##raydium_window_ty## are read-only variables,
providing you actual frame size.
    
##raydium_background_color[4]## is a RGBA table, and will be used for
frame clearing, and fog color. You can change this variable, and call 
respective update functions (frame and fog), or simply use 
##raydium_background_color_change(GLfloat r, GLfloat g, GLfloat b, GLfloat a)##.
    
More informations in corresponding sections.
    
===Vertices===
    
Vertices data structure is distributed in 4 parts:
    
- ##raydium_vertex_*## : these tables will simply contains vertices coordinates

- ##raydium_vertex_normal_*## : vertices normals. Raydium will maintain 
two distinct normal tables, and this one will be used for calculations.

- ##raydium_vertex_normal_visu_*## : the other normal table, used for 
lighting. Smoothing "visu" normals will provides a better rendering, and Raydium includes 
all necessary functions to automate this task.

- ##raydium_vertex_texture_u##, ##*raydium_vertex_texture_v##, 
##*raydium_vertex_texture## contains, for each vertex stored 
in the vertices data structure, u and v mapping information, 
and associated texture number. U and V are texture mapping coordinates. 
	
Raydium can automatically generates some of these data 
(normals and uv coords, that is), Read "Vertices" section above 
for more information.
	
PLEASE, do not write directly in these tables, use dedicated functions.
	
===Objects===
	
Objects are loaded in Vertices stream, identified by a "start" and an "end" 
(##raydium_object_start[]## and ##raydium_object_end[]##) in this stream.
An index is incremented each time you load an object 
(##GLuint raydium_object_index##). Filename is also stored in 
##raydium_object_name[][]##. Go to "Objects" section to know more.
	
===Lights===
	
First of all, ##raydium_light_enabled_tag## contains 0 when light is
disabled, non-zero otherwise. This is a read-only variable, so use 
suitable functions.
	
Currently, for Raydium, a light can have 3 states: on, off, or blinking.
##raydium_light_internal_state[]## stores this.
	
Next comes all light's features: position, color, intensity. You can 
modify directly these variables, and call update fonctions, 
if needed (not recommended).
	
Next, ##raydium_light_blink_*## are used internaly for blinking lights, 
setting lowest, higher light intensity, and blinking speed. 
Do noy modify these variables, use suitable functions.
	
You should read the chapter dedicated to lights for more information.
	
===Fog===
	
Only one variable, here: ##raydium_fog_enabled_tag##, switching from zero 
to non zero if fog is enabled. Do NOT use this variable to enable or 
disable fog, but suitable functions, this variable is just a tag.
	
===Camera===
	
Since many calls to camera functions are done during one frame,
Raydium must track if any call to these functions was already done, 
using ##raydium_frame_first_camera_pass## boolean.
	
##raydium_camera_pushed##, also used as a boolean, stores stack state. 
When you place your camera in the scene with Raydium, it pushes matrix 
on top of the stack, so you can modify it (the matrix), placing an object
for example, an restore it quickly after, by popping matrix off.

**/
