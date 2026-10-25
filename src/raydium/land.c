/*
    Raydium - CQFD Corp.
    http://raydium.org/
    License: GPL - GNU General Public License, see "gpl.txt" file.
*/

#ifndef DONT_INCLUDE_HEADERS
#include "index.h"
#else
#include "headers/land.h"
#endif

// mostly unusable functions (old Raydium's core) ...

GLfloat raydium_land_internal_landtmp(GLfloat x,GLfloat y,GLfloat phase,GLfloat ampl, GLfloat periode)
{
GLfloat a,b;

a=(x/periode)*360;
b=(y/periode)*360;
a+=phase;
b+=phase;

while(a>=360) a-=360;
while(b>=360) b-=360;

a=raydium_trigo_cos(a);
b=raydium_trigo_cos(b);

b=b*a;
b*=ampl;

return(b);
}

// must recode all this function
void raydium_land_draw_water(GLfloat phase, GLfloat ampl, GLfloat periode,
                                 int sub, GLfloat pas, char *texture)
{
int ix,iy;
//GLfloat pas;
//GLfloat wpas,border,dep=0;
GLfloat x1,x2,y1,y2;
GLuint vertex_index_save;

//pas=(SUBDIV*FACT)/sub;


raydium_texture_current_set_name(texture);
vertex_index_save=raydium_vertex_index;

// angle=(x*ampl)/360

for(iy=0;iy<sub;iy++)
for(ix=0;ix<sub;ix++)
{

x1=(GLfloat)(ix)*pas;
x2=(GLfloat)(ix+1)*pas;
y1=(GLfloat)(iy)*pas;
y2=(GLfloat)(iy+1)*pas;

//#define FACTW (FACT/2)
#define FACTW 10

raydium_vertex_uv_add(x1,y1, raydium_land_internal_landtmp(x1,y1,phase,ampl,periode), 0,0);
raydium_vertex_uv_add(x2,y1, raydium_land_internal_landtmp(x2,y1,phase,ampl,periode), FACTW,0);
raydium_vertex_uv_add(x2,y2, raydium_land_internal_landtmp(x2,y2,phase,ampl,periode), FACTW,FACTW);

raydium_vertex_uv_add(x2,y2, raydium_land_internal_landtmp(x2,y2,phase,ampl,periode), FACTW,FACTW);
raydium_vertex_uv_add(x1,y2, raydium_land_internal_landtmp(x1,y2,phase,ampl,periode), 0,FACTW);
raydium_vertex_uv_add(x1,y1, raydium_land_internal_landtmp(x1,y1,phase,ampl,periode), 0,0);

}

/*
// expand water space
#define EXPAND 5000.f
#define EX_Z (-ampl)
border=SUBDIV*FACT;


raydium_vertex_uv_add(0,-EXPAND,EX_Z, 0,0);
raydium_vertex_uv_add(0,border+EXPAND,EX_Z, 0,(EXPAND*2+border)/FACT);
raydium_vertex_uv_add(-EXPAND,border+EXPAND,EX_Z, -EXPAND/FACT,(EXPAND*2+border)/FACT);
*/

raydium_rendering_from_to(vertex_index_save,raydium_vertex_index);
raydium_vertex_index=vertex_index_save;
}


// unsable
GLfloat raydium_land_surface(GLfloat x, GLfloat y, GLfloat *nx, GLfloat *ny, GLfloat *nz)
{
//int cx,cy;
GLint n;
GLfloat dx,dy;
GLfloat a,b,c,d;
//cx=x/FACT;
//cy=y/FACT;
//n=((cx*(SUBDIV-1))+cy)*6;

//dy=x-(float)(cx*FACT);
//dx=y-(float)(cy*FACT);

if(dy>=dx) n+=3;

#define Ax raydium_vertex_x[n]
#define Bx raydium_vertex_x[n+1]
#define Cx raydium_vertex_x[n+2]

#define Ay raydium_vertex_y[n]
#define By raydium_vertex_y[n+1]
#define Cy raydium_vertex_y[n+2]

#define Az raydium_vertex_z[n]
#define Bz raydium_vertex_z[n+1]
#define Cz raydium_vertex_z[n+2]

a = ((Az - Bz) * (Ay - Cy)) + ((Az - Cz) * (By - Ay));
b = ((Ax - Bx) * (Az - Cz)) + ((Ax - Cx) * (Bz - Az));
c = ((Bx - Ax) * (Ay - Cy)) + ((Ax - Cx) * (Ay - By));
d = - a*Ax - b*Ay - c*Az;

*nx=raydium_vertex_normal_x[n];
*ny=raydium_vertex_normal_y[n];
*nz=raydium_vertex_normal_z[n];

return(-(a*y+b*x+d)/c);
}
