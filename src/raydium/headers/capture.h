#ifndef _CAPTURE_H
#define _CAPTURE_H
/*=
Capture (2D)
700
**/

// Quickview
/**
Captures are made in TGA (without RLE compression) or JPEG formats and saved into
the current directory.
These functions may fail (garbage in resulting capture) if frame size if 
not "standard", mostly after a window resize.

Also there are "auto" functions that provide a simplest method to make an screen
capture. So,the following example (put into the ##display()## function), allows jpeg 
screenshots just pressing F9 key:
##
if(raydium_key_last==9)   raydium_capture_frame_jpeg_auto();
##

Raydium also allow you to capture movies: activate ##DEBUG_MOVIE## option
in ##raydium/config.h## with the needed framerate, and press F11. Raydium
will use a dedicated time line, allowing smooth capture. This system may cause
strange behaviours with movies providing network action.
The movie is stored in multiples files in ##movie## directory, and you can
use mencoder like this:
##mencoder -ovc lavc -lavcopts vcodec=mpeg4:vhq:vbitrate=780 
mf://\*.tga -vf scale=320:240 -mf fps=25 -o ~/ray.avi##
You can also use audio file adding this: 
## -audiofile audio.mp3 -oac copy## for example.
**/

__rayapi void raydium_capture_frame(char *filename);
/**
Capture current frame to ##filename##.
**/

__rayapi void raydium_capture_frame_auto(void);
/**
Same as above, but to an auto-generated filename (raycap*).
**/

__rayapi void raydium_capture_frame_jpeg(char *filename);
/**
Same as ##raydium_capture_frame()## but using JPEG image format.
See ##raydium/config.h## for quality setting.
**/

__rayapi void raydium_capture_frame_now(char *filename);
/**
Same as ##raydium_capture_frame()##, but without waiting the end of the frame,
saving the hardware color buffer, whatever it contains. Use with caution.
**/

__rayapi void raydium_capture_frame_jpeg_now(char *filename);
/**
Same as above, but using JPEG image format.
**/

__rayapi void raydium_capture_filename_auto(char *dest,char *format);
/**
Internal Use. Generates filenames for new screenshots.
**/

__rayapi void raydium_capture_frame_auto(void);
/**
Capture the current frame giving the resulting file and automatic name.
**/ 

__rayapi void raydium_capture_frame_jpeg_auto(void);
/**
Same as above, but using JPEG image format.
**/

#endif
