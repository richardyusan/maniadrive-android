/*
    Raydium - CQFD Corp.
    http://raydium.org/
    License: GPL - GNU General Public License, see "gpl.txt" file.
*/

#ifndef DONT_INCLUDE_HEADERS
#include "index.h"
#else
#include "headers/hdr.h"
#endif 

void raydium_hdr_settings_color_local(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
raydium_hdr_color_local[0]=r;
raydium_hdr_color_local[1]=g;
raydium_hdr_color_local[2]=b;
raydium_hdr_color_local[3]=a;
}

void raydium_hdr_settings_color_ambient(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
raydium_hdr_color_ambient[0]=r;
raydium_hdr_color_ambient[1]=g;
raydium_hdr_color_ambient[2]=b;
raydium_hdr_color_ambient[3]=a;
}

void raydium_hdr_settings_eye(float speed, float alpha_max)
{
raydium_hdr_alpha_max=alpha_max;
raydium_hdr_eye_speed=speed;
}

void raydium_hdr_settings(GLfloat *color_local, GLfloat *color_ambient, float eye_speed, float alpha_max)
{
GLfloat r,g,b,a;

r=color_ambient[0];
g=color_ambient[1];
b=color_ambient[2];
a=color_ambient[3];
raydium_hdr_settings_color_ambient(r,g,b,a);
r=color_local[0];
g=color_local[1];
b=color_local[2];
a=color_local[3];
raydium_hdr_settings_color_local(r,g,b,a);
raydium_hdr_settings_eye(eye_speed,alpha_max);
}

void raydium_hdr_init(void)
{
glClearStencil(0); // default stencil "color"
glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
glStencilFunc(GL_ALWAYS, 0, 0xffffffff);
glEnable(GL_STENCIL_TEST);
raydium_hdr_eye=0;
raydium_hdr_state=0;
raydium_hdr_texture_id=-1;
raydium_hdr_generated=0;
raydium_hdr_settings_color_local(1,1,1,1);
raydium_hdr_settings_color_ambient(0.45,0.45,0.45,0.45);
raydium_hdr_settings_eye(RAYDIUM_HDR_EYE_SPEED_DEFAULT,1);
raydium_log("HDR: OK");
}

void raydium_hdr_internal_window_malloc(void)
{
if(!raydium_hdr_state)
    return;

if(raydium_hdr_mem)
    free(raydium_hdr_mem);
raydium_hdr_mem=malloc(raydium_window_tx*raydium_window_ty);
}

void raydium_hdr_enable(void)
{
raydium_hdr_state=1;

if(raydium_hdr_texture_id<0)
    {
    raydium_hdr_texture_id=raydium_texture_load_internal("","hdrmap",1,RAYDIUM_HDR_SIZE,RAYDIUM_HDR_SIZE,3,-1);
    raydium_hdr_mem=NULL;
    raydium_hdr_internal_window_malloc();
    raydium_hdr_mem_hdr=malloc(RAYDIUM_HDR_SIZE*RAYDIUM_HDR_SIZE);
    raydium_hdr_mem_hdr2=malloc(RAYDIUM_HDR_SIZE*RAYDIUM_HDR_SIZE);
    raydium_hdr_mem_hdr3=malloc(RAYDIUM_HDR_SIZE*RAYDIUM_HDR_SIZE*3);
    }
}

void raydium_hdr_disable(void)
{
raydium_hdr_state=0;
}

void raydium_hdr_block(signed char blocking)
{
if(blocking)
    glStencilFunc(GL_ALWAYS, 0, 0xffffffff);
else
    glStencilFunc(GL_ALWAYS, 1, 0xffffffff);
}

void raydium_hdr_blur(unsigned char *in, unsigned char *out)
{
int x,y;
float p;

// 1st row and last row
for(x=1;x<RAYDIUM_HDR_SIZE-1;x++)
    {
    p=0;
    p+=in[ (x-1) + (RAYDIUM_HDR_SIZE*0) ];
    p+=in[ (x+1) + (RAYDIUM_HDR_SIZE*0) ];
    p+=in[ (x-1) + (RAYDIUM_HDR_SIZE*1) ];
    p+=in[ (x+0) + (RAYDIUM_HDR_SIZE*1) ];
    p+=in[ (x+1) + (RAYDIUM_HDR_SIZE*1) ];
    out[x]=p/5;

    p=0;
    p+=in[ (x-1) + (RAYDIUM_HDR_SIZE*(RAYDIUM_HDR_SIZE-2)) ];
    p+=in[ (x+0) + (RAYDIUM_HDR_SIZE*(RAYDIUM_HDR_SIZE-2)) ];
    p+=in[ (x+1) + (RAYDIUM_HDR_SIZE*(RAYDIUM_HDR_SIZE-2)) ];
    p+=in[ (x-1) + (RAYDIUM_HDR_SIZE*(RAYDIUM_HDR_SIZE-1)) ];
    p+=in[ (x+1) + (RAYDIUM_HDR_SIZE*(RAYDIUM_HDR_SIZE-1)) ];
    out[x+(RAYDIUM_HDR_SIZE*(RAYDIUM_HDR_SIZE-1))]=p/5;
    }

// 1st col and last col
for(y=1;y<RAYDIUM_HDR_SIZE-1;y++)
    {
    p=0;
    p+=in[ 1 + ((y-1)*RAYDIUM_HDR_SIZE) ];
    p+=in[ 1 + ((y+0)*RAYDIUM_HDR_SIZE) ];
    p+=in[ 1 + ((y+1)*RAYDIUM_HDR_SIZE) ];
    p+=in[ 0 + ((y-1)*RAYDIUM_HDR_SIZE) ];
    p+=in[ 0 + ((y+1)*RAYDIUM_HDR_SIZE) ];
    out[y*RAYDIUM_HDR_SIZE]=p/5;

    p=0;
    p+=in[ (RAYDIUM_HDR_SIZE-2) + ((y-1)*RAYDIUM_HDR_SIZE) ];
    p+=in[ (RAYDIUM_HDR_SIZE-2) + ((y+0)*RAYDIUM_HDR_SIZE) ];
    p+=in[ (RAYDIUM_HDR_SIZE-2) + ((y+1)*RAYDIUM_HDR_SIZE) ];
    p+=in[ (RAYDIUM_HDR_SIZE-1) + ((y-1)*RAYDIUM_HDR_SIZE) ];
    p+=in[ (RAYDIUM_HDR_SIZE-1) + ((y+1)*RAYDIUM_HDR_SIZE) ];
    out[(y*RAYDIUM_HDR_SIZE) + (RAYDIUM_HDR_SIZE-1) ]=p/5;
    }

// body
for(x=1;x<RAYDIUM_HDR_SIZE-1;x++)
    for(y=1;y<RAYDIUM_HDR_SIZE-1;y++)
	{
	p=0;
	p+=in[(x+0)+((y+0)*RAYDIUM_HDR_SIZE)];
	p+=in[(x-1)+((y-1)*RAYDIUM_HDR_SIZE)];
	p+=in[(x+0)+((y-1)*RAYDIUM_HDR_SIZE)];
	p+=in[(x+1)+((y-1)*RAYDIUM_HDR_SIZE)];
	p+=in[(x-1)+((y+0)*RAYDIUM_HDR_SIZE)];
	p+=in[(x+1)+((y+0)*RAYDIUM_HDR_SIZE)];
	p+=in[(x-1)+((y+1)*RAYDIUM_HDR_SIZE)];
	p+=in[(x+0)+((y+1)*RAYDIUM_HDR_SIZE)];
	p+=in[(x+1)+((y+1)*RAYDIUM_HDR_SIZE)];
	out[x+(y*RAYDIUM_HDR_SIZE)]=p/9;
	}

// upper left pixel
p=0;
p+=in[1+(0*RAYDIUM_HDR_SIZE)];
p+=in[1+(1*RAYDIUM_HDR_SIZE)];
p+=in[0+(1*RAYDIUM_HDR_SIZE)];
out[0]=p/3;

// upper right pixel
p=0;
p+=in[(RAYDIUM_HDR_SIZE-2)+(0*RAYDIUM_HDR_SIZE)];
p+=in[(RAYDIUM_HDR_SIZE-2)+(1*RAYDIUM_HDR_SIZE)];
p+=in[(RAYDIUM_HDR_SIZE-1)+(1*RAYDIUM_HDR_SIZE)];
out[RAYDIUM_HDR_SIZE-1]=p/3;

// bottom left pixel
p=0;
p+=in[ ((RAYDIUM_HDR_SIZE-1)*RAYDIUM_HDR_SIZE) + 1 ];
p+=in[ ((RAYDIUM_HDR_SIZE-2)*RAYDIUM_HDR_SIZE) + 1 ];
p+=in[ ((RAYDIUM_HDR_SIZE-2)*RAYDIUM_HDR_SIZE) + 0 ];
out[(RAYDIUM_HDR_SIZE-1)*RAYDIUM_HDR_SIZE]=p/3;

// bottom right pixel
p=0;
p+=in[ ((RAYDIUM_HDR_SIZE-1)*RAYDIUM_HDR_SIZE) + (RAYDIUM_HDR_SIZE-2) ];
p+=in[ ((RAYDIUM_HDR_SIZE-2)*RAYDIUM_HDR_SIZE) + (RAYDIUM_HDR_SIZE-1) ];
p+=in[ ((RAYDIUM_HDR_SIZE-2)*RAYDIUM_HDR_SIZE) + (RAYDIUM_HDR_SIZE-2) ];
out[((RAYDIUM_HDR_SIZE-1)*RAYDIUM_HDR_SIZE) + (RAYDIUM_HDR_SIZE-1) ]=p/3;
}


void raydium_hdr_map(void)
{
int x,y,i;
float fx,fy;
float incx,incy;
int offset;
unsigned char pixel;
int total;
float hdr_exposure;

if(!raydium_hdr_state || raydium_hdr_generated)
    return;

glDisable(GL_STENCIL_TEST);

// download stencil to RAM ...
glReadPixels (0,0,raydium_window_tx,raydium_window_ty,GL_STENCIL_INDEX,GL_UNSIGNED_BYTE,raydium_hdr_mem);

incx=raydium_window_tx/(RAYDIUM_HDR_SIZE*1.f);
incy=raydium_window_ty/(RAYDIUM_HDR_SIZE*1.f);
fx=0;
fy=0;
total=0;

// ... downscaling ...
for(y=0;y<RAYDIUM_HDR_SIZE;y++)
    {
    for(x=0;x<RAYDIUM_HDR_SIZE;x++)
	{
	offset=(x+(RAYDIUM_HDR_SIZE*y));
	pixel=raydium_hdr_mem[raydium_trigo_round(fx)+(raydium_window_tx*(raydium_trigo_round(fy)))];
	//slooooooooooooow ! (?!)
	//glReadPixels(raydium_trigo_round(fx),raydium_trigo_round(fy),1,1,GL_STENCIL_INDEX,GL_UNSIGNED_BYTE,&pixel);
	raydium_hdr_mem_hdr2[offset]=(pixel?255:0);
	total+=pixel;
        fx+=incx;
	}
    fx=0;
    fy+=incy;
    }


hdr_exposure=(float)total/(RAYDIUM_HDR_SIZE*RAYDIUM_HDR_SIZE);

// dec intensity using exposure factor
if(raydium_hdr_eye>0)
    {
    raydium_hdr_eye-=(hdr_exposure*(raydium_hdr_eye_speed*raydium_frame_time));
    if(raydium_hdr_eye<=0) 
	raydium_hdr_eye=-9999; // the eye is now ok
    }

// we're in "total" drak
if(hdr_exposure==0)
    raydium_hdr_eye=0; // be ready for another "flash"

if(hdr_exposure>0 && raydium_hdr_eye==0)
    raydium_hdr_eye=raydium_hdr_alpha_max;

//printf("%i/%i (%f)\n",total,RAYDIUM_HDR_SIZE*RAYDIUM_HDR_SIZE,hdr_exposure*100);

if(raydium_hdr_eye>0)
// if(!raydium_key[GLUT_KEY_F11])
  for(x=0;x<RAYDIUM_HDR_PASS;x++)
    {
    raydium_hdr_blur(raydium_hdr_mem_hdr2,raydium_hdr_mem_hdr);
    raydium_hdr_blur(raydium_hdr_mem_hdr,raydium_hdr_mem_hdr2);
    }

hdr_exposure=(raydium_hdr_eye>0?raydium_hdr_eye:0); // clamp


for(i=0;i<RAYDIUM_HDR_SIZE*RAYDIUM_HDR_SIZE;i++)
    {
    raydium_hdr_mem_hdr3[0 + i*3] = raydium_hdr_mem_hdr2[i] * hdr_exposure;
    raydium_hdr_mem_hdr3[1 + i*3] = raydium_hdr_mem_hdr2[i] * hdr_exposure;
    raydium_hdr_mem_hdr3[2 + i*3] = raydium_hdr_mem_hdr2[i] * hdr_exposure;
    }

// ... and we upload to hdr_texture
glBindTexture(GL_TEXTURE_2D,raydium_hdr_texture_id);
glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,RAYDIUM_HDR_SIZE,RAYDIUM_HDR_SIZE,0,GL_RGB,GL_UNSIGNED_BYTE,raydium_hdr_mem_hdr3);
raydium_hdr_generated=1;
}

void raydium_hdr_map_apply(void)
{
if(!raydium_hdr_state)
    return;

if(!raydium_hdr_generated)
    raydium_hdr_map();

raydium_hdr_generated=0;

#ifdef DEBUG_HDR_STENCIL
if(raydium_key[GLUT_KEY_F10])
{
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
raydium_osd_start();
glStencilFunc(GL_EQUAL, 1, 0xffffffff);
glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
glEnable(GL_STENCIL_TEST);
glDisable(GL_TEXTURE_2D);
glColor4f(1,1,1,1);
glBegin(GL_QUADS);
glVertex3f(0,0,0);
glVertex3f(100,0,0);
glVertex3f(100,100,0);
glVertex3f(0,100,0);
glEnd();
raydium_osd_stop();
glEnable(GL_TEXTURE_2D);
glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
glStencilFunc(GL_ALWAYS, 0, 0xffffffff);
glDisable(GL_STENCIL_TEST);
}
else
{
#endif
raydium_osd_start();
glBindTexture(GL_TEXTURE_2D,raydium_hdr_texture_id);
glEnable(GL_TEXTURE_2D);
glEnable(GL_BLEND);
glDepthMask(GL_FALSE);
glBlendFunc(GL_ONE,GL_ONE); 
//localized hdr "glow" effect
glColor4fv(raydium_hdr_color_local);
glBegin(GL_QUADS);
glTexCoord2f(0,0);
glVertex3f(0,0,0);

glTexCoord2f(1,0);
glVertex3f(100,0,0);

glTexCoord2f(1,1);
glVertex3f(100,100,0);

glTexCoord2f(0,1);
glVertex3f(0,100,0);
glEnd();

//ambiental hdr "glow" effect
glColor4fv(raydium_hdr_color_ambient);
glBegin(GL_QUADS);
glTexCoord2f(0,0);
glVertex3f(-50,-50,0);

glTexCoord2f(1,0);
glVertex3f(150,-50,0);

glTexCoord2f(1,1);
glVertex3f(150,150,0);

glTexCoord2f(0,1);
glVertex3f(-50,150,0);
glEnd();

glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
raydium_rendering_internal_restore_render_state();
raydium_osd_stop();
#ifdef DEBUG_HDR_STENCIL
}
#endif
}

signed char raydium_hdr_texture(int texture, signed char hdr)
{
if(texture>=0 && texture<raydium_texture_index)
    {
    raydium_texture_hdr[texture]=hdr;
    return 1;
    }

raydium_log("HDR: cannot set HDR attribute on texture: invalid name or index");
return 0;
}

signed char raydium_hdr_texture_name(char *name, signed char hdr)
{
return raydium_hdr_texture(raydium_texture_find_by_name(name),hdr);
}

void raydium_hdr_texture_reset(void)
{
int i;

for(i=0;i<RAYDIUM_MAX_TEXTURES;i++)
 raydium_texture_hdr[i]=0;
}
