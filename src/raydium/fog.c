/*
    Raydium - CQFD Corp.
    http://raydium.org/
    License: GPL - GNU General Public License, see "gpl.txt" file.
*/

#ifndef DONT_INCLUDE_HEADERS
#include "index.h"
#else
#include "headers/fog.h"

#endif 

void raydium_fog_enable(void)
{
raydium_fog_enabled_tag=1;
raydium_fog_apply();
}

void raydium_fog_disable(void)
{
raydium_fog_enabled_tag=0;
raydium_fog_apply();
}

void raydium_fog_color_update(void)
{
glFogfv(GL_FOG_COLOR,raydium_background_color);
}

void raydium_fog_mode(GLuint mode)
{
raydium_fog_mode_value=mode;
}

void raydium_fog_density(GLfloat density)
{
raydium_fog_density_value=density;
}

void raydium_fog_near(GLfloat fnear)
{
raydium_fog_near_value=fnear;
}

void raydium_fog_far(GLfloat ffar)
{
raydium_fog_far_value=ffar;
}

void raydium_fog_apply(void)
{

if(raydium_fog_enabled_tag)
{
    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE,raydium_fog_mode_value);
    raydium_fog_color_update();
    glFogf(GL_FOG_DENSITY, raydium_fog_density_value);
    glHint(GL_FOG_HINT, GL_FASTEST);

    if(raydium_fog_far_value==0)
	{	
	raydium_fog_far_value=raydium_projection_far;
	raydium_fog_near_value=raydium_projection_far/4.f;
	}		

    glFogf(GL_FOG_START,raydium_fog_near_value);
    glFogf(GL_FOG_END,raydium_fog_far_value);		
    }
    else
    {
    glDisable(GL_FOG);
    }
}


void raydium_fog_wait(void)
{
glDisable(GL_FOG);
}

void raydium_fog_init(void)
{
raydium_fog_far_value=0;
raydium_fog_near_value=0;
raydium_fog_density_value=0;
raydium_fog_mode_value=RAYDIUM_FOG_MODE_LINEAR;
raydium_fog_volumetric_enabled_tag=0;

switch(RENDER_VOLUMETRIC_FOG_AXIS)
    {
    case 0:
	raydium_fog_volumetric_array=raydium_vertex_x;
	break;
    case 1:
	raydium_fog_volumetric_array=raydium_vertex_y;
	break;
    case 2:
	raydium_fog_volumetric_array=raydium_vertex_z;
	break;	
    default:
	raydium_log("fog: FAILED: RENDER_VOLUMETRIC_FOG_AXIS is broken !");
	exit(100);
    }

raydium_fog_enable();
raydium_log("fog: OK");
}

void raydium_fog_volumetric_support(void)
{
raydium_fog_volumetric_enabled_tag=1;
}

void raydium_fog_volumetric_enable(void)
{
glFogi(GL_FOG_COORDINATE_SOURCE_EXT, GL_FOG_COORDINATE_EXT);
}

void raydium_fog_volumetric_disable(void)
{
glFogi(GL_FOG_COORDINATE_SOURCE_EXT, GL_FRAGMENT_DEPTH_EXT);
}

