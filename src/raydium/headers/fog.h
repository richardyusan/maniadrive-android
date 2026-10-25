#ifndef _FOG_H
#define _FOG_H
/*=
Fog
500
**/

// Introduction
/**
Fog is usefull for two major reasons:

1. Realism: Just try, and you'll understand: 
amazing depth impression, no ?

2. Speed: For a correct fog effect (i'm talking 
about estetic aspect), you must bring near_clipping to a closer value,
reducing the overall number of triangles displayed at the same time. 

There are 3 types of fog. They are: 

    * Linear:

		      Far-z
		fog= --------- 
		     Far-Near

    * Exp:

		       (-density*z)
		fog= e^

    * Exp2:

		       (-density*z)^2
		fog= e^

Above ##z## is the distance to the calculated point from the camera.
As you can see, linear mode doesn't use ##Density##; and Exp & Exp2 modes don't
use near and far values. Remember that.
**/

__rayapi void raydium_fog_enable (void);
/**
Obvious
**/

__rayapi void raydium_fog_disable (void);
/**
Obvious
**/

__rayapi void raydium_fog_color_update (void);
/**
If you have modified ##raydium_background_color## array, you must 
call this function, applying the specified color to hardware.
See also: ##raydium_background_color_change##
**/

__rayapi void raydium_fog_mode(GLuint mode);
/**
The fog mode can be change with this function. There are 3 different ways 
to apply the fog:

1. ##RAYDIUM_FOG_MODE_LINEAR## - Used by default, the fog is directly applied 
according the distance. Not real world fog, but used to avoid drawing 
too distant objects.
##IMPORTANT##: EXP mode ignores the ##density## value, 
only uses ##near## and ##far##.

2. ##RAYDIUM_FOG_MODE_EXP## - The fog grows exponentially with the distance. 
Usual mist in the real world. 
##IMPORTANT##: EXP mode ignores the ##near## and ##far## values, 
only uses the ##density##.

3. ##RAYDIUM_FOG_MODE_EXP2## - The fog grows twice exponentially with the 
distance. Used when the observer is inside a cloud/mist.
##IMPORTANT##: EXP2 mode ignores the ##near## and ##far## values, 
only uses the ##density##.
**/

__rayapi void raydium_fog_density(GLfloat density);
/**
Sets the density of the fog.
Useless if you are using LINEAR mode.
**/

__rayapi void raydium_fog_near(GLfloat near);
/**
Sets the near point to apply the fog.
Useless if you are using EXP or EXP2 modes.
**/

__rayapi void raydium_fog_far(GLfloat far);
/**
Sets the far point of the fog.
Useless if you are using EXP or EXP2 modes.
**/

__rayapi void raydium_fog_apply(void);
/**
Used to apply changes in your setup of fog.
Also is used to continue a previously stopped fog. 
See: ##raydium_fog_wait()## below.
**/

__rayapi void raydium_fog_wait(void);
/**
With this function you can deactivate TEMPORALY the fog, but the internal state
of the fog in Raydium won't change, so when you use raydium_fog_apply, the fog 
will continue like it was before being stoped.
It's very usefull for certain rendering effects that need to 
stop the fog temporaly.
**/

__rayapi void raydium_fog_volumetric_support(void);
/**
With this function, you're saying to Raydium that you want a support
for volumetric fog in you application. Call this function as soon as possible
after engine init, since it will change the way Raydium renders objects (think
about display lists).
**/

__rayapi void raydium_fog_volumetric_enable(void);
/**
When you call this function, fog is no more applied using fragment depth,
but using ##RENDER_VOLUMETRIC_FOG_AXIS## (see config.h).
You must have called ##raydium_fog_volumetric_support()## before enabling
volumetric fog.
**/

__rayapi void raydium_fog_volumetric_disable(void);
/**
Reset fog sytem to default behavior (fragment depth).
**/


#endif
