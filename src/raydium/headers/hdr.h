#ifndef _HDR_H
#define _HDR_H

/*=
Pseudo HDR
4300
**/

// Introduction
/**
**/

__rayapi void raydium_hdr_init(void);
/**
Internal use.
**/

__rayapi void raydium_hdr_enable(void);
/**
**/

__rayapi void raydium_hdr_disable(void);
/**
**/

__rayapi void raydium_hdr_internal_window_malloc(void);
/**
Internal use.
**/

__rayapi void raydium_hdr_block(signed char blocking);
/**
**/

__rayapi void raydium_hdr_blur(unsigned char *in, unsigned char *out);
/**
**/

__rayapi void raydium_hdr_map(void);
/**
**/

__rayapi void raydium_hdr_map_apply(void);
/**
**/

__rayapi void raydium_hdr_settings_color_local(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
/**
**/

__rayapi void raydium_hdr_settings_color_ambient(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
/**
**/

__rayapi void raydium_hdr_settings_eye(float speed, float alpha_max);
/**
**/

__rayapi void raydium_hdr_settings(GLfloat *color_local, GLfloat *color_ambient, float eye_speed, float alpha_max);
/**
**/


__rayapi signed char raydium_hdr_texture(int texture, signed char hdr); // display lists !!
/**
**/

__rayapi signed char raydium_hdr_texture_name(char *texture, signed char hdr);
/**
Same as above, but using ##texture## name.
**/

__rayapi void raydium_hdr_texture_reset(void); // display lists !!
/**
**/

#endif
