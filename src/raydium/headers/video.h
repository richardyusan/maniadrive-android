#ifndef _VIDEO_H
#define _VIDEO_H

#include "../video.h"

/*=
Video playback
4100
**/

// Introduction
/**
Raydium supports simple video playback, thru a special video codec (JPGS),
useful for menus enhancements, "speaking" thumbnails, ...
This codec only supports video, use sound API if needed.
You will find an small utility, ##mk_jpgs## in Raydium source tree, didacted to
movie creation.
**/

// How to create a movie ?
/**
First, compile ##mk_jpgs##: example: ##gcc mk_jpgs.c -o mk_jpgs## or any other
standard build command.
Then, generate JPEG pictures (using a temporary directory, if possible):
##mplayer movie.avi -vo jpeg:quality=50 -vf scale=256:256##, where you may
change quality factor and output size. Use "hardware friendly" sizes (64,
128,256,...) !
You can now build JPGS file:
##./mk_jpgs 25 256 256 video.jpgs## (fps, size x, size y, output file)
**/

__rayapi void raydium_video_init(void);
/**
Internal use.
**/

__rayapi signed char raydium_video_isvalid(int i);
/**
Internal use, but you can call this function if you want to verify if a
video id is valid (in bounds and open).
**/

__rayapi int raydium_video_find_free(void);
/**
Internal use.
Finds a free video slot.
**/

__rayapi int raydium_video_find(char *name);
/**
Resolvs video ##name##, returning video id.
Returns -1 when video is not found.
**/

__rayapi void raydium_video_jpeg_decompress(FILE *fp,unsigned char *to);
/**
Internal.
**/

__rayapi int raydium_video_open(char *filename, char *as);
/**
This function will open and prepare video ##filename##, and will attach
this video to a "live texture" (see Live API chapter, if needed).
**/

__rayapi void raydium_video_callback_video(int id);
/**
Internal use.
**/

__rayapi void raydium_video_callback(void);
/**
Internal use. Frame callback.
**/

__rayapi void raydium_video_delete(int id);
/**
Will delete video ##id##. Warning: this function will not delete
associated Live texture, so you may open a new video with the same
texture name, but video size must be the same a the previous one.
**/

__rayapi void raydium_video_delete_name(char *name);
/**
Same as above, using video name.
**/

__rayapi void raydium_video_loop(int id, signed char loop);
/**
Sets loop attribute for the video ##id##. By defaults, video loops. Call
this function with loop=0 to disable this behavior.
**/

__rayapi void raydium_video_loop_name(char *name, signed char loop);
/**
Same as above, using video name.
**/

__rayapi signed char raydium_video_isplaying(int id);
/**
Returns **1** is video ##id## is playing, **0** if this video is stopped,
and **-1** if function failed.
**/

__rayapi signed char raydium_video_isplaying_name(char *name);
/**
Same as above, using video name.
**/


#endif
