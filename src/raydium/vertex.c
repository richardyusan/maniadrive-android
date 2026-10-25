/*
    Raydium - CQFD Corp.
    http://raydium.org/
    License: GPL - GNU General Public License, see "gpl.txt" file.
*/

#ifndef DONT_INCLUDE_HEADERS
#include "index.h"
#else
#include "headers/vertex.h"
#endif

void raydium_vertex_add(GLfloat x, GLfloat y, GLfloat z/*, GLfloat nx, GLfloat ny, GLfloat nz*/)
{

raydium_vertex_x[raydium_vertex_index]=x;
raydium_vertex_y[raydium_vertex_index]=y;
raydium_vertex_z[raydium_vertex_index]=z;
raydium_vertex_texture[raydium_vertex_index]=raydium_texture_current_main;
raydium_vertex_texture_multi[raydium_vertex_index]=raydium_texture_current_multi;
raydium_vertex_texture_env[raydium_vertex_index]=raydium_texture_current_env;
if(raydium_texture_current_multi)
{
    if(raydium_texture_current_multi_u==-99999 && 
       raydium_texture_current_multi_v==-99999 )
    {
    raydium_vertex_texture_multi_u[raydium_vertex_index]=raydium_internal_vertex_next_u*RAYDIUM_RENDER_MULTITEX_AUTO_UV_FACT;
    raydium_vertex_texture_multi_v[raydium_vertex_index]=raydium_internal_vertex_next_v*RAYDIUM_RENDER_MULTITEX_AUTO_UV_FACT;
    }
    else
    {
    raydium_vertex_texture_multi_u[raydium_vertex_index]=raydium_texture_current_multi_u;
    raydium_vertex_texture_multi_v[raydium_vertex_index]=raydium_texture_current_multi_v;
    }
}
else
{
raydium_vertex_texture_multi_u[raydium_vertex_index]=0;
raydium_vertex_texture_multi_v[raydium_vertex_index]=0;
}


if(raydium_internal_vertex_next_extras)
{
 raydium_vertex_texture_u[raydium_vertex_index]=raydium_internal_vertex_next_u;
 raydium_vertex_texture_v[raydium_vertex_index]=raydium_internal_vertex_next_v;
}
else 
{
 if(raydium_vertex_offset_triangle==0)
 {
 raydium_vertex_texture_u[raydium_vertex_index]=0.0;
 raydium_vertex_texture_v[raydium_vertex_index]=0.0;
 }

 if(raydium_vertex_offset_triangle==1)
 {
 raydium_vertex_texture_u[raydium_vertex_index]=1.0;
 raydium_vertex_texture_v[raydium_vertex_index]=0.0;
 }

 if(raydium_vertex_offset_triangle==2)
 {
 raydium_vertex_texture_u[raydium_vertex_index]=1.0;
 raydium_vertex_texture_v[raydium_vertex_index]=1.0;
 }
}


raydium_vertex_normal_visu_x[raydium_vertex_index]=raydium_internal_vertex_next_nx;
raydium_vertex_normal_visu_y[raydium_vertex_index]=raydium_internal_vertex_next_ny;
raydium_vertex_normal_visu_z[raydium_vertex_index]=raydium_internal_vertex_next_nz;

raydium_vertex_index++;

 if(++raydium_vertex_offset_triangle>=3)
 {
  if(raydium_internal_vertex_next_extras<2) raydium_normal_generate_lastest_triangle(1);
  else raydium_normal_generate_lastest_triangle(0);
  raydium_vertex_offset_triangle=0;
 }

 raydium_internal_vertex_next_extras=0;
if(raydium_vertex_index>=RAYDIUM_MAX_VERTICES) {raydium_log("out of vertex table #%i!",raydium_texture_current_main); exit(29); }/* !!! TO CHANGE */
}


void raydium_vertex_uv_add(GLfloat x, GLfloat y, GLfloat z,GLfloat u, GLfloat v)
{
raydium_internal_vertex_next_extras=1;
raydium_internal_vertex_next_u=u;
raydium_internal_vertex_next_v=v;
raydium_vertex_add(x,y,z);
}

void raydium_vertex_uv_normals_add(GLfloat x, GLfloat y, GLfloat z, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat u, GLfloat v)
{
raydium_internal_vertex_next_extras=2;
raydium_internal_vertex_next_u=u;
raydium_internal_vertex_next_v=v;

raydium_internal_vertex_next_nx=nx;
raydium_internal_vertex_next_ny=ny;
raydium_internal_vertex_next_nz=nz;
raydium_vertex_add(x,y,z);
}
