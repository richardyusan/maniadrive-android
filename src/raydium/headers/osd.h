#ifndef _OSD_H
#define _OSD_H
/*=
OSD (On Screen Display)
2900
**/

// Introduction
/**
Raydium provides some high level function for "On Screen Display", 
as string drawing (2D and 3D), application's logo, mouse cursor, and other
various 2D displaying tools.

In most cases, these functions must be called after any other object
drawing function, to avoid overlapping problems.

Most functions will use a percentage system, and origin is at lower-left corner.
**/


__rayapi void raydium_osd_color_change (GLfloat r, GLfloat g, GLfloat b);
/**
This function will change the font color for the next ##raydium_osd_printf*## 
calls.
As usual: 0 <= (##r##,##g## and ##b##) <= 1.
**/

__rayapi void raydium_osd_alpha_change (GLfloat a);
/**
Same as above, but will change font transparency.
**/

__rayapi void raydium_osd_color_rgba (GLfloat r, GLfloat g, GLfloat b, GLfloat a);
/**
This is a mix of ##raydium_osd_color_change## and ##raydium_osd_alpha_change##.
**/

__rayapi void raydium_osd_color_ega (char hexa);
/**
This function will change font color with the corresponding 
##hexa##decimal code (as a char: '0' to 'F') in the standard EGA palette.

Here is this palette:
""
<div class="table" align="center">
<table class="tableListing" summary=" " cellspacing="0" cellpadding="3" border="1">
<tr class="wiki"><td class="wiki"> <b>Hexa</b> </td><td class="wiki"> <b>Color</b> </td></tr>
<tr class="wiki"><td class="wiki"> 0 </td><td class="wiki"> Black </td></tr>
<tr class="wiki"><td class="wiki"> 1 </td><td class="wiki"> Blue </td></tr>
<tr class="wiki"><td class="wiki"> 2 </td><td class="wiki"> Green </td></tr>
<tr class="wiki"><td class="wiki"> 3 </td><td class="wiki"> Cyan </td></tr>
<tr class="wiki"><td class="wiki"> 4 </td><td class="wiki"> Red </td></tr>
<tr class="wiki"><td class="wiki"> 5 </td><td class="wiki"> Purple </td></tr>
<tr class="wiki"><td class="wiki"> 6 </td><td class="wiki"> Brown </td></tr>
<tr class="wiki"><td class="wiki"> 7 </td><td class="wiki"> White </td></tr>
<tr class="wiki"><td class="wiki"> 8 </td><td class="wiki"> Grey </td></tr>
<tr class="wiki"><td class="wiki"> 9 </td><td class="wiki"> Light Blue </td></tr>
<tr class="wiki"><td class="wiki"> A </td><td class="wiki"> Light Green </td></tr>
<tr class="wiki"><td class="wiki"> B </td><td class="wiki"> Light Cyan </td></tr>
<tr class="wiki"><td class="wiki"> C </td><td class="wiki"> Light Red </td></tr>
<tr class="wiki"><td class="wiki"> D </td><td class="wiki"> Light Purple </td></tr>
<tr class="wiki"><td class="wiki"> E </td><td class="wiki"> Light Yellow </td></tr>
<tr class="wiki"><td class="wiki"> F </td><td class="wiki"> Light White </td></tr>
</table>
</div>
""
**/

__rayapi void raydium_osd_start (void);
/**
Mostly for internal uses. (will configure screen for OSD operations)
**/

__rayapi void raydium_osd_stop (void);
/**
Mostly for internal uses. (see above)
**/

__rayapi void raydium_osd_draw (int tex, GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
/**
Will draw ##tex## texture using (##x1##,##y1##) and (##x2##,##y2##) points.
**/

__rayapi void raydium_osd_draw_name (char *tex, GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
/**
Same as above, but using texture filename.
**/

__rayapi void raydium_osd_printf (GLfloat x, GLfloat y, GLfloat size, GLfloat spacer, char *texture, char *format, ...);
/**
This function is an OpenGL equivalent to the standard "printf" C function.

- (##x##,##y##) is the position of the text's beginning, as a screen 
percentage, with origin at lower left.

- ##size## is the font size, using an arbitrary unit. This size is always 
proportionnal to frame size (font size will grow up with screen size,
in other words).

- ##spacer## is the factor of spacing between 2 consecutive letters. With 
standard fonts, 0.5 is a correct value (relatively condensed text).

- ##texture## is obviously the texture filename to use (font*.tga are 
often provided with Raydium distribution, and by R3S).

- ##format## is the standard printf format string, followed by 
corresponding arguments: ##"^9Player ^Fname is: %10s", player_name##
This format can use '^' char to change color text, followed by a color, 
indicated by a hexadecimal letter (EGA palette). See ##raydium_osd_color_ega##
function, above.

Here you are a simple example:

%%(c)
strcpy(version,"^Ctest 0.1^F");
raydium_osd_printf(2,98,16,0.5,"font2.tga","- %3i FPS - tech demo %s for Raydium %s, CQFD Corp.",
                   raydium_render_fps,version,raydium_version);
%%
**/

__rayapi void raydium_osd_printf_3D (GLfloat x, GLfloat y, GLfloat z, GLfloat size, GLfloat spacer, char *texture, char *format, ...);
/**
Same as above, but you can place your text in your application 3D space, 
using ##x##, ##y## and ##z## values.
**/

__rayapi void raydium_osd_logo (char *texture);
/**
Will draw a logo for the current frame with texture filename.
For now, you've no control over rotation speed of the logo.
**/

__rayapi void raydium_osd_cursor_set (char *texture, GLfloat xsize, GLfloat ysize);
/**
This function will set mouse cursor with texture filename and 
with (##xsize##,##ysize##) size (percent of screen size).
You should use a RGB**A** texture for better results.
example:
%%(c)
raydium_osd_cursor_set("BOXcursor.tga",4,4);
%%

You can set ##texture## to NULL or empty string to cancel OSD cursor texture.
**/

__rayapi void raydium_osd_cursor_draw (void);
/**
Internal use.
**/

__rayapi void raydium_osd_internal_vertex (GLfloat x, GLfloat y, GLfloat top);
/**
Internal use.
**/

__rayapi void raydium_osd_network_stat_draw (GLfloat px, GLfloat py, GLfloat size);
/**
Will draw network stats (if available) in a box.
%%(c) raydium_osd_network_stat_draw(5,30,20); %%
**/

__rayapi void raydium_osd_mask (GLfloat * color4);
/**
Will draw a uniform mask using ##color4## (RGBA color) for this frame.
**/

__rayapi void raydium_osd_mask_texture(int texture,GLfloat alpha);
/**
Will draw a textured mask, with ##alpha## opacity (1 is full opacity).
**/

__rayapi void raydium_osd_mask_texture_name(char *texture,GLfloat alpha);
/**
Same as above, but resolving texture by name.
**/

__rayapi void raydium_osd_mask_texture_clip(int texture,GLfloat alpha, GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
/**
Same as ##raydium_osd_mask_texture##, but (x1,y1),(x2,y2) will be used as 
texture coords, in a [0,100] range.
**/

__rayapi void raydium_osd_mask_texture_clip_name(char *texture,GLfloat alpha, GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
/**
Same as above, but resolving texture by name.
**/

__rayapi void raydium_osd_fade_callback (void);
/**
Internal use.
**/

__rayapi void raydium_osd_fade_init (void);
/**
Internal use.
**/

__rayapi void raydium_osd_fade_from (GLfloat * from4, GLfloat * to4, GLfloat time_len, void *OnFadeEnd);
/**
This function will configure a fading mask from ##from4## color to ##to4##.
This fade will last ##time_len## seconds, and will call ##OnFadeEnd## callback
when finished.
This callback signature must be ##void callback(void)##.

A standard fade-to-black-and-restore example:
%%(c)
// back to normal rendering
void restorefade(void)
{
   GLfloat from[4]={0,0,0,2};
   GLfloat to[4]={0,0,0,0};
   raydium_osd_fade_from(from,to,1,NULL);
   // do things (like moving camera to another place, for example).
}
				
...

// If space key : fade to black
if(raydium_key_last==1032)
    {
    GLfloat from[4]={0,0,0,0};
    GLfloat to[4]={0,0,0,1};
    raydium_osd_fade_from(from,to,0.3,restorefade);
    }
%%				 
**/

#endif
