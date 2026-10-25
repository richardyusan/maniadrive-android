/*
    Raydium - CQFD Corp.
    http://raydium.org/
    License: GPL - GNU General Public License, see "gpl.txt" file.
*/

#ifndef DONT_INCLUDE_HEADERS
#include "index.h"
#else
#include "headers/light.h"
#endif 

GLuint raydium_texture_find_by_name(char *name);

void raydium_light_enable(void)
{
glEnable(GL_LIGHTING);
raydium_light_enabled_tag=1;
}

void raydium_light_disable(void)
{
glDisable(GL_LIGHTING);
raydium_light_enabled_tag=0;
}


GLuint raydium_light_to_GL_light(GLuint l)
{
return GL_LIGHT0+l;
}

void raydium_light_on(GLuint l)
{
raydium_light_internal_state[l]=RAYDIUM_LIGHT_ON;
glEnable(raydium_light_to_GL_light(l));
}

void raydium_light_off(GLuint l)
{
raydium_light_internal_state[l]=RAYDIUM_LIGHT_OFF;
glDisable(raydium_light_to_GL_light(l));
}

void raydium_light_switch(GLuint l)
{
if(raydium_light_internal_state[l]<0)
raydium_light_on(l);
else
raydium_light_off(l);
}


void raydium_light_update_position(GLuint l)
{
glLightfv(raydium_light_to_GL_light(l),GL_POSITION,raydium_light_position[l]);
}

void raydium_light_update_position_all(void)
{
int i;
for(i=0;i<RAYDIUM_MAX_LIGHTS;i++)
    if(raydium_light_internal_state[i]!=RAYDIUM_LIGHT_OFF)
        raydium_light_update_position(i); 
}


void raydium_light_update_intensity(GLuint l)
{
glLightf(raydium_light_to_GL_light(l),GL_QUADRATIC_ATTENUATION,1.0/raydium_light_intensity[l]);
}


void raydium_light_update_all(GLuint l)
{
GLuint GLl=raydium_light_to_GL_light(l);
//GLfloat zero[4]={0.0,0.0,0.0,1.0};
//GLfloat one[4]={0.3,0.3,0.3,1.};
//glLightfv(GLl,GL_AMBIENT, /*raydium_light_color[l]*//*zero*/one);
glLightfv(GLl,GL_DIFFUSE, raydium_light_color[l]);
glLightfv(GLl,GL_SPECULAR,raydium_light_color[l]);

raydium_light_update_intensity(l);
raydium_light_update_position(l);

}

signed char raydium_light_texture(int texture, signed char enable)
{
if(texture>=0 && texture<raydium_texture_index)
    {
    raydium_texture_nolight[texture]=!enable;
    return 1;
    }

raydium_log("light: cannot set 'no light' attribute on texture: invalid name or index");
return 0;
}

signed char raydium_light_texture_name(char *name, signed char enable)
{
return raydium_light_texture(raydium_texture_find_by_name(name),enable);
}


void raydium_light_move(GLuint l,GLfloat *vect)
{
memcpy(raydium_light_position[l],vect,raydium_internal_size_vector_float_4);
//raydium_light_update_position(l);
}

void raydium_light_move_3f(GLuint l,GLfloat px, GLfloat py, GLfloat pz)
{
GLfloat pos[4];
pos[0]=px;
pos[1]=py;
pos[2]=pz;
pos[3]=0;
raydium_light_move(l,pos);
}

void raydium_light_conf_7f(GLuint l,GLfloat px, GLfloat py, GLfloat pz, GLfloat intensity, GLfloat r, GLfloat g, GLfloat b)
{
raydium_light_intensity[l]=intensity;
raydium_light_color[l][0]=r;
raydium_light_color[l][1]=g;
raydium_light_color[l][2]=b;
raydium_light_color[l][3]=1.0;
raydium_light_move_3f(l,px,py,pz);
raydium_light_update_all(l);
}

void raydium_light_reset(GLuint l)
{
GLfloat pos[]={0,0,0,1};
GLfloat color[]={1.0, 1.0, 1.0, 1.0};
GLfloat intensity=10000000;

memcpy(raydium_light_position[l],pos,raydium_internal_size_vector_float_4);
memcpy(raydium_light_color[l],color,raydium_internal_size_vector_float_4);
raydium_light_intensity[l]=intensity;
raydium_light_off(l);
raydium_light_update_all(l);
}


void raydium_light_blink_internal_update(GLuint l)
{
raydium_light_intensity[l]+=raydium_light_blink_increment[l];

if(raydium_light_intensity[l]>raydium_light_blink_high[l])
 { 
 raydium_light_intensity[l]=raydium_light_blink_high[l];
 raydium_light_blink_increment[l]*=-1.0;
 }

if(raydium_light_intensity[l]<raydium_light_blink_low[l])
 { 
 raydium_light_intensity[l]=raydium_light_blink_low[l];
 raydium_light_blink_increment[l]*=-1.0;
 }

raydium_light_update_intensity(l);
}


void raydium_light_blink_start(GLuint l,int fpc)
{
raydium_light_on(l);
raydium_light_internal_state[l]=RAYDIUM_LIGHT_BLINKING;
//fpc = frames per cycle
fpc/=2;
raydium_light_blink_low[l]=raydium_light_intensity[l]/fpc;
raydium_light_blink_high[l]=raydium_light_intensity[l];
raydium_light_blink_increment[l]=raydium_light_intensity[l]/fpc;
raydium_light_blink_internal_update(l);
}


void raydium_light_callback(void)
{
GLuint i;

for(i=0;i<RAYDIUM_MAX_LIGHTS;i++)
 if(raydium_light_internal_state[i]==RAYDIUM_LIGHT_BLINKING) 
  raydium_light_blink_internal_update(i);
}

