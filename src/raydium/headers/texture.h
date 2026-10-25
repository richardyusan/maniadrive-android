#ifndef _TEXTURE_H
#define _TEXTURE_H
/*=
Textures
1200
**/

// Introduction
/**
For now, Raydium only handles TGA uncompressed texture.
As explainded in the first part of this guide, Raydium provides three 
texture filters (none, bilinear, trilinear using MipMaps ).

Texture sizes must be a power of two, 8 (alpha mask), 24 (RGB) or 32 (RGBA) bits.

Raydium supports simple color materials,  using a "rgb(r,g,b)" string 
as texture name, where r, g and b are 0 <= x <= 1 (floats).
With 3 negative values, you will generate a "phantom texture". Phantom textures
are only drawn into the z-buffer (and not color buffer).
Texture clamping and advanced multitexturing effects are supported by Raydium, 
but not documented here for now. If you're interested, have a look at source 
code, or take a look at the Wiki. Tips: "BOX", "ENV", "HDR", ";", "|".

Effective environment mapping (one pass, two texture units) is available using
a special filename separator for texture field in TRI files : #
See this example:
##0.232258 0.225387 -0.149804 0.012198 -0.274925 0.961388 0.731411 0.980236 fiesta_diffuse.tga#ENV_map.tga##
Environment texture name must start with "ENV" to allow spherical mapping, wich
is needed for such effect. See also ##RAYDIUM_RENDER_REFLECTION_FACT## in
file ##common.h## if you want reflection to be more or less visible.
**/


__rayapi signed char raydium_texture_size_is_correct (GLuint size);
/**
Returns true if ##size## is a correct texture size, depending of
hardware capacities and "power of 2" constraint.
**/

__rayapi GLuint raydium_texture_load_internal(char *filename, char *as, signed char faked, int faked_tx, int faked_ty, int faked_bpp, int or_live_id_fake);
/**
Internal use.
**/

__rayapi GLuint raydium_texture_load (char *filename);
/**
Loads "filename" texture into hardware memory. Function results 
texture index, but in most cases, you can identify later a texture 
by his name, without providing his index, so you can probably ignore 
this value.

0 is returned if texture loading have failed.
**/

__rayapi GLuint raydium_texture_load_erase (char *filename, GLuint to_replace);
/**
Same as above, but ##to_replace## texture (index) is erased with ##filename##.
**/

__rayapi signed char raydium_texture_current_set (GLuint current);
/**
Switch active texture to "current" index. Mostly used for runtime object 
creation:
"set current texture, add vertices, set another texture, 
add vertices, ... and save all to an objet" 
(see below for vertices management).
**/

__rayapi signed char raydium_texture_current_set_name (char *name);
/**
Same as above, but using texture name. This function will load ##name## 
if not alread done.
**/

__rayapi GLuint raydium_texture_find_by_name (char *name);
/**
Returns index for texture "name", and load it if not already done.
**/

__rayapi GLuint raydium_texture_exists(char *name);
/**
Same as above, but don't load texture if ##name## isn't already loaded and
then returns -1. Returns texture id otherwise.
**/

__rayapi void raydium_texture_filter_change (GLuint filter);
/**

Change texture filter. The new filter will apply on all "next" textures,
but will not change already loaded ones (this was the case in old Raydium
releases), since it may generate strange bugs with dynamic (aka "faked")
textures, and it was very slow.

%%(c)
// will switch to bilinear filter for next textures
raydium_texture_filter_change(RAYDIUM_TEXTURE_FILTER_BILINEAR)%%
**/

#endif
