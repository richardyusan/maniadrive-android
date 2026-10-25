/*
    Raydium - CQFD Corp.
    http://raydium.org/
    License: GPL - GNU General Public License, see "gpl.txt" file.
*/

#ifndef DONT_INCLUDE_HEADERS
#include "index.h"
#else
#include "headers/shadow.h"
#endif 

#include "shadow.h"

void raydium_shadow_init(void)
{
raydium_shadow_tag=0;
raydium_shadow_ground_mesh=-1;
raydium_shadow_rendering=0;
raydium_log("shadow: OK");
}

void raydium_shadow_ground_change(int object)
{
//modelsize, center factors
GLfloat min[3],max[3];
GLfloat tx,ty,tz;
GLfloat x,y,z;

if(!raydium_object_isvalid(object))
    {
    raydium_log("shadow: ERROR: ground id or name is invalid");
    return;
    }

raydium_shadow_ground_mesh=object;
raydium_object_find_center_factors(object,&x,&y,&z);
raydium_shadow_ground_center_factor_x=0.5-x;
raydium_shadow_ground_center_factor_y=0.5-y;

raydium_object_find_minmax(object,min,max);
tx=max[0]-min[0];
ty=max[1]-min[1];
tz=max[2]-min[2];

raydium_shadow_ground_modelsize=raydium_trigo_max(tx,raydium_trigo_max(ty,tz));
raydium_shadow_ground_modelsize/=2;

raydium_log("shadow: ground (%i) modelsize is %.2f, center factors : %.2f/%.2f",
    raydium_shadow_ground_mesh,
    raydium_shadow_ground_modelsize,
    raydium_shadow_ground_center_factor_x,
    raydium_shadow_ground_center_factor_y);

}


void raydium_shadow_enable(void)
{
float S[]={1.0, 0.0, 0.0, 0.0};
float T[]={0.0, 1.0, 0.0, 0.0};
float R[]={0.0, 0.0, 1.0, 0.0};
float Q[]={0.0, 0.0, 0.0, 1.0};
int tmp;

if(raydium_shadow_tag)
    return;

raydium_shadow_tag=1;

// There was a previous call
if(raydium_texture_exists(RAYDIUM_SHADOW_TEXTURE)!=-1)
    return;

tmp=1;
while(tmp<=raydium_window_tx && 
      tmp<=raydium_window_ty && 
      tmp<=raydium_texture_size_max)
      {
      tmp*=2;
      }
raydium_shadow_map_size=tmp/2;
raydium_log("shadow: shadow map size detected to %ix%i",raydium_shadow_map_size,raydium_shadow_map_size);

glPushMatrix();
glLoadIdentity();
glTexGenfv(GL_S,GL_EYE_PLANE,S);
glTexGenfv(GL_T,GL_EYE_PLANE,T);
glTexGenfv(GL_R,GL_EYE_PLANE,R);
glTexGenfv(GL_Q,GL_EYE_PLANE,Q);
glPopMatrix();

raydium_shadow_texture=raydium_texture_load_internal("",RAYDIUM_SHADOW_TEXTURE,1,raydium_shadow_map_size,raydium_shadow_map_size,4,-1);
//raydium_shadow_texture=raydium_texture_load("shadowmap1.tga"); // debug

}

void raydium_shadow_disable(void)
{
raydium_shadow_tag=0;
}


signed char raydium_shadow_isenabled(void)
{
return raydium_shadow_tag;
}

void raydium_shadow_light_main(GLuint l)
{
raydium_shadow_light=l;
}


void raydium_shadow_object_draw(GLuint o)
{
#ifndef DEBUG_RENDER_DISABLE_DISPLAYLISTS
static GLuint dl[RAYDIUM_MAX_OBJECTS];
static char dl_state[RAYDIUM_MAX_OBJECTS];
static int first=0;
int i;

if(first)
    for(i=0;i<RAYDIUM_MAX_OBJECTS;i++)
	dl_state[i]=-1;

if(raydium_render_displaylists_tag && raydium_object_anims[o]==0)
{
 if(!dl_state[o])
    {
    // build DL
    dl_state[o]=1;
    dl[o]=glGenLists(1);
    raydium_log("Object: creating **shadow** display list for object %s",raydium_object_name[o]);
    glNewList(dl[o],GL_COMPILE);
    raydium_rendering_from_to_simple(raydium_object_start[o],raydium_object_end[o]);
    glEndList();
    }
  glCallList(dl[o]);
}
else
    raydium_rendering_from_to_simple(raydium_object_start[o],raydium_object_end[o]);
#else
    raydium_rendering_from_to_simple(raydium_object_start[o],raydium_object_end[o]);
#endif
}

void raydium_shadow_map_generate(void)
{
// test shadow support and ground
if(!raydium_shadow_tag || raydium_shadow_ground_mesh<0)
    return;

glViewport(0,0,raydium_shadow_map_size,raydium_shadow_map_size);
glClearColor(0,0,0,1);
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
glMatrixMode(GL_PROJECTION);
glPushMatrix();
glLoadIdentity();

glTranslatef((raydium_shadow_ground_center_factor_x-0.5)*2,(raydium_shadow_ground_center_factor_y-0.5)*2,0);
glOrtho(-raydium_shadow_ground_modelsize,raydium_shadow_ground_modelsize,
	-raydium_shadow_ground_modelsize,raydium_shadow_ground_modelsize,
	-1000,1000); // should probably use far clipping
glMatrixMode(GL_MODELVIEW);
glLoadIdentity();
gluLookAt(raydium_light_position[raydium_shadow_light][0]*0,
	  raydium_light_position[raydium_shadow_light][1]*0,
	  raydium_light_position[raydium_shadow_light][2], 0,0,0, 0,1,0);

glDisable(GL_LIGHTING);
glDisable(GL_TEXTURE_2D);
    
glColor4f(RAYDIUM_SHADOW_OPACITY,RAYDIUM_SHADOW_OPACITY,RAYDIUM_SHADOW_OPACITY,1);

raydium_shadow_rendering=1;
glPushMatrix();
#ifdef ODE_SUPPORT 
//raydium_ode_draw_all(RAYDIUM_ODE_DRAW_SHADOWERS); // static compile time linking disallow using this constant
raydium_ode_draw_all(4);
#endif
glPopMatrix();
raydium_shadow_rendering=0;
raydium_clear_color_update();
if(raydium_light_enabled_tag)
    glEnable(GL_LIGHTING);

#ifdef DEBUG_SHADOW_MAP_VIEW
glutSwapBuffers();
#endif

glBindTexture(GL_TEXTURE_2D,raydium_shadow_texture);
glCopyTexSubImage2D(GL_TEXTURE_2D,0,0,0,0,0,raydium_shadow_map_size,raydium_shadow_map_size);
    
glColor4f(1,1,1,1);
    
glViewport(0,0,raydium_window_tx,raydium_window_ty);
glEnable(GL_TEXTURE_2D);

glMatrixMode(GL_PROJECTION);
glPopMatrix();
glMatrixMode(GL_MODELVIEW);
}

// apply shadow map to ground
void raydium_shadow_map_render(void)
{
float mview[16],imview[16];

// test shadow support and ground
if(!raydium_shadow_tag || raydium_shadow_ground_mesh<0)
    return;

raydium_camera_replace();
glEnable(GL_TEXTURE_GEN_S);
glEnable(GL_TEXTURE_GEN_T);
glEnable(GL_TEXTURE_GEN_R);
glEnable(GL_TEXTURE_GEN_Q);
glTexGeni(GL_S,GL_TEXTURE_GEN_MODE,GL_EYE_LINEAR);
glTexGeni(GL_T,GL_TEXTURE_GEN_MODE,GL_EYE_LINEAR);
glTexGeni(GL_R,GL_TEXTURE_GEN_MODE,GL_EYE_LINEAR);
glTexGeni(GL_Q,GL_TEXTURE_GEN_MODE,GL_EYE_LINEAR);

glGetFloatv(GL_MODELVIEW_MATRIX,mview);
_raydium_trigo_MatrixInverse(mview,imview);

glMatrixMode(GL_TEXTURE);
glLoadIdentity();
glTranslatef(raydium_shadow_ground_center_factor_x,raydium_shadow_ground_center_factor_y,0);
glScalef(0.5,0.5,1.0);
glColor4f(1,1,1,1);
glOrtho(-raydium_shadow_ground_modelsize,raydium_shadow_ground_modelsize,
	-raydium_shadow_ground_modelsize,raydium_shadow_ground_modelsize,
	-1,1);

gluLookAt(raydium_light_position[raydium_shadow_light][0]*0,
	  raydium_light_position[raydium_shadow_light][1]*0,
	  raydium_light_position[raydium_shadow_light][2], 0,0,0, 0,1,0);

glMultMatrixf(imview);

glDisable(GL_LIGHTING);
glEnable(GL_TEXTURE_2D);
glEnable(GL_BLEND);
glBlendFunc(GL_ZERO,GL_ONE_MINUS_SRC_COLOR);
glBindTexture(GL_TEXTURE_2D,raydium_shadow_texture);
//glBindTexture(GL_TEXTURE_2D,raydium_texture_find_by_name("shadowmap1.tga")); // debug

raydium_shadow_object_draw(raydium_shadow_ground_mesh);

glDisable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    
glMatrixMode(GL_TEXTURE);
glLoadIdentity(); // reset GL_TEXTURE matrix
glMatrixMode(GL_MODELVIEW);
glDisable(GL_TEXTURE_GEN_S);
glDisable(GL_TEXTURE_GEN_T);
glDisable(GL_TEXTURE_GEN_R);
glDisable(GL_TEXTURE_GEN_Q);
}
