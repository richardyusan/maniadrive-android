#ifndef _LIVE_H
#define _LIVE_H
#include "../live.h"


/*=
Live textures and videos API
3800
**/

// Introduction
/**
Live API features two distinct parts:

1 - It provides an easy way to create and manage dynamic textures, since you
just have to give a pointer to your image data, and call suitable function
each time this image is changing.

2 - This API also supports video4linux (aka V4L), as an extension of
the Live API. The main goal is to link a video4linux device (webcam,
tv card, ...) to a texture. A callback is also available if you want to
get (and transform) data of every capture.

You'll find detailed informations for each domain below.
**/


// Color conversion
/**
Live API used to work with RGB and RGBA color formats. Since some V4L 
devices use other patterns, Live API needs conversion functions.
You've no need to do color conversion by yourself, consider all this 
as internal functions.
**/

void v4l_copy_420_block (int yTL, int yTR, int yBL, int yBR, int u, int v, int rowPixels, unsigned char *rgb, int bits);
/**
YUV420P block copy.
This code is not native.
**/

int v4l_yuv420p2rgb (unsigned char *rgb_out, unsigned char *yuv_in, int width, int height, int bits);
/**
YUV420P to RGB conversion.
This code is not native.
**/

// Live Video API
/**
This part of the Live API id dedicated to video devices. For now, the
support is limited to Linux thru V4L API. Every V4L compatible device
should work with Live Video, but for any advanced setup of your video
device (tuner configuration, source, FPS, ...), you must use an external
tool.
By default, Live API supports up to 4 simultaneous devices.
**/

__rayapi signed char raydium_live_video_isvalid(int i);
/**
Internal use, but you can call this function if you want to verify if a
live video device id is valid (in bounds, open, and ready to capture).
**/

__rayapi int raydium_live_video_find_free(void);
/**
Internal use.
Finds a free live video device slot.
**/

__rayapi int raydium_live_video_open(char *device, int sizex, int sizey);
/**
This is where you should start. This function opens ##device## (something
like "/dev/video0"), requesting ##sizex## x ##sizey## resolution.
If ##device## is ##RAYDIUM_LIVE_DEVICE_AUTO##, Raydium will use a default device, 
hardcoded or given thru commande line (##--video-device##).
Same story for sizes, with ##RAYDIUM_LIVE_SIZE_AUTO##.
This function will try to detect a compatible palette (grayscale, rgb,
yuv420p, with 4, 6, 8, 15, 16 and 24 bits per pixel) and capture 
method (##read()## or ##mmap()##).
Returns -1 in case or error, device id otherwise.
**/

__rayapi int raydium_live_video_open_auto(void);
/**
Same as above, but with full autodetection.
**/

__rayapi int raydium_live_video_read(raydium_live_Device *dev);
/**
Internal V4L read function.
**/

__rayapi void raydium_internal_live_video_callback(void);
/**
internal frame callback.
**/

// Live API Core
/**
the main goal of the Live API is to allow you to create your own
dynamic textures. The first method is to provide your own picture data thru a
pointer, the second method is to use a Live Video device (see above) as
data source.
**/

__rayapi void raydium_internal_live_close(void);
/**
Internal close function.
**/

__rayapi void raydium_live_init(void);
/**
Internal init function.
**/

__rayapi signed char raydium_live_texture_isvalid(int i);
/**
Internal use, but you can call this function if you want to verify if a
live texture id is valid (in bounds, open, and ready to capture).
**/

__rayapi int raydium_live_texture_find_free(void);
/**
Internal use.
Finds a free live texture slot.
**/

__rayapi int raydium_live_texture_find(int original_texture);
/**
Resolvs ##original_texture## id (native Raydium texture id) to a
live texture id, if any.
**/

__rayapi int raydium_live_texture_create(char *as, unsigned char *data_source, int tx, int ty, int bpp);
/**
Create a new Live Texture with ##as## name. You must provide a ##data_source##
with RGB or RGBA format, with ##tx## and ##ty## size.
Possible bpp values are 24 (RGB) and 32 (RGBA).
Returns the live texture id, or -1 when it fails.
**/

__rayapi int raydium_live_texture_video(int device_id, char *as);
/**
This is another way to create a Live Texture, but using a Live Video device
for data source. Provide texture name (##as##) and Live ##device_id##.
**/

__rayapi void raydium_live_texture_refresh(int livetex);
/**
When your data source have changed, call this function to refresh new
data to hardware. Obviously, this function is useless for Live Video textures
since Raydium will automatically refresh data.
**/

__rayapi void raydium_live_texture_refresh_name(char *texture);
/**
Same as above, but using ##texture## name.
**/


__rayapi void raydium_live_texture_refresh_callback_set(int livetex, void *callback);
/**
You can create a "OnRefresh" callback for any Live Texture (##livetex## is an
id to this texture). This is mostly usefull for Live Video texture.
Your callback must follow this prototype :
##int refresh_callback(unsigned char *data, int tx, int ty, int bpp)##
You have full write access to ##data##, allowing you to draw over
the provided picture (warning: for non video Live textures, ##data## pointer
is not owned by Raydium and may be "read only")
You must return 1 to confirm data flushing, or 0 to cancel this refresh.
**/

__rayapi void raydium_live_texture_refresh_callback_set_name(char *texture, void *callback);
/**
Same as above, but using ##texture## name.
**/

__rayapi void raydium_live_texture_mask(int livetex, GLfloat alpha);
/**
This function will draw a fullscreen mask using ##livetex## Live Texture id and
##alpha## opacity (0 means transparent, 1 means fully opaque, allowing any
intermediate value). Use this function at any place of your rendering 
function AFTER camera call and obviously before ##raydium_rendering_finish##.
**/

__rayapi void raydium_live_texture_mask_name(char *texture, GLfloat alpha);
/**
Same as above, but using ##texture## name.
**/

__rayapi void raydium_live_texture_draw(int livetex, GLfloat alpha,GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
/**
This function is a clone of ##raydium_osd_draw()##, dedicated to live textures.
This function will draw the video ##livetex## on the screen, from (x1,y1) to
(x2,y2).
**/

__rayapi void raydium_live_texture_draw_name(char *texture, GLfloat alpha,GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
/**
Same as above, but using ##texture## name.
**/

#endif

