/*
    Raydium - CQFD Corp.
    http://raydium.org/
    License: GPL - GNU General Public License, see "gpl.txt" file.
*/

#ifndef DONT_INCLUDE_HEADERS
#include "index.h"
#else
#include "headers/osd.h"
#endif

// need proto
void raydium_camera_replace(void);

// OSD color could be changed from one frame to the next one by Raydium
// itself (console, internal uses, ...) so set your OSD color
// in the display loop, no before.

void raydium_osd_color_change(GLfloat r, GLfloat g, GLfloat b)
{
raydium_osd_color[0]=r;
raydium_osd_color[1]=g;
raydium_osd_color[2]=b;
}

void raydium_osd_alpha_change(GLfloat a)
{
raydium_osd_color[3]=a;
}

void raydium_osd_color_rgba(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
raydium_osd_color_change(r,g,b);
raydium_osd_alpha_change(a);
}

void raydium_osd_color_ega(char hexa)
{
int i=15;

if(hexa>='0' && hexa<='9') i=hexa - '0';
if(hexa>='a' && hexa<='f') i=hexa - 'a'+10;
if(hexa>='A' && hexa<='F') i=hexa - 'A'+10;
i*=3;
raydium_osd_color_change(raydium_osd_ega[i],raydium_osd_ega[i+1],raydium_osd_ega[i+2]);
}

void raydium_osd_start(void)
{
glMatrixMode(GL_PROJECTION);
glPushMatrix();
glLoadIdentity();

//glViewport(0, 0, raydium_window_tx, raydium_window_ty);
glOrtho(0,100, 0,100, -100,100);

//glPushMatrix();
raydium_rendering_internal_prepare_texture_render(0);
glMatrixMode(GL_MODELVIEW);
glPushMatrix();
glLoadIdentity();
glDisable(GL_DEPTH_TEST);
glDepthMask(GL_FALSE);
glDisable(GL_LIGHTING);
glDisable(GL_FOG);
glColor4f(1.f,1.f,1.f,1.f);
}

void raydium_osd_stop(void)
{
raydium_window_resize_callback(raydium_window_tx, raydium_window_ty);
glEnable(GL_DEPTH_TEST);
glDepthMask(GL_TRUE);
if(raydium_light_enabled_tag) glEnable(GL_LIGHTING);
if(raydium_fog_enabled_tag)   glEnable(GL_FOG);

glMatrixMode(GL_PROJECTION);
glPopMatrix();

glMatrixMode(GL_MODELVIEW);
glPopMatrix();
}


void raydium_osd_draw(int tex ,GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
raydium_osd_start();

raydium_texture_current_set(tex);
raydium_rendering_internal_prepare_texture_render(tex);
glColor4fv(raydium_osd_color);
glBegin(GL_QUADS);
glTexCoord2f(0,0);
glVertex3f(x1,y1,0);
glTexCoord2f(1,0);
glVertex3f(x2,y1,0);
glTexCoord2f(1,1);
glVertex3f(x2,y2,0);
glTexCoord2f(0,1);
glVertex3f(x1,y2,0);
glEnd();
raydium_rendering_internal_restore_render_state();

raydium_osd_stop();
}

void raydium_osd_draw_name(char *tex ,GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
raydium_osd_draw(raydium_texture_find_by_name(tex),x1,y1,x2,y2);
}

// need to be secured
void raydium_osd_printf(GLfloat x, GLfloat y, GLfloat size, GLfloat spacer,char *texture, char *format, ...)
{
char str[RAYDIUM_MAX_NAME_LEN];
va_list argptr;
int i,texsave;
GLfloat dx=0;
unsigned char ligne,offset;
GLfloat u,v;
char c;


size/=RAYDIUM_OSD_FONT_SIZE_FACTOR;
va_start(argptr,format);
vsprintf(str,format,argptr);
va_end(argptr);

raydium_osd_start();
texsave=raydium_texture_current_main;
raydium_texture_current_set_name(texture);
raydium_rendering_internal_prepare_texture_render(raydium_texture_current_main);

glTranslatef(x,y,0);

// strlen is slooow :)
for( i=0; str[i]; i++ )
 {
 if(str[i]=='\n' || str[i]=='\t') continue;
 if(str[i]=='^' && i+1<RAYDIUM_MAX_NAME_LEN && str[i+1]!=0) {
                 // oh ! ... you cannot draw '^' char since i'm
		 // too lazy to code it for now :)
                 raydium_osd_color_ega(str[++i]);
		 continue;
		 }
 c=str[i]; // c=str[i]-32;
 ligne=c/16;
 offset=c-(ligne*16);
 u=offset/16.f;
 v=1-(ligne/16.f); // /*1-*/

 glColor4fv(raydium_osd_color);
 glBegin(GL_QUADS);
 glTexCoord2f(u,v);
 glVertex3f(-size+dx,size,0);
 glTexCoord2f(u+(1/16.f),v);
 glVertex3f(size+dx,size,0);
 glTexCoord2f(u+(1/16.f),v-(1/16.f));
 glVertex3f(size+dx,-size,0);
 glTexCoord2f(u,v-(1/16.f));
 glVertex3f(-size+dx,-size,0);
 glEnd();
 dx+=(size*2*spacer);

 }

//raydium_rendering_internal_restore_render_state();
raydium_osd_stop();
//raydium_texture_current_set(texsave);
}


void raydium_osd_printf_3D(GLfloat x, GLfloat y, GLfloat z, GLfloat size, GLfloat spacer,char *texture, char *format, ...)
{
char str[RAYDIUM_MAX_NAME_LEN];
va_list argptr;

GLdouble sx,sy,sz;
GLdouble modelMatrix[16];
GLdouble projectionMatrix[16];
GLint   viewport[4];

va_start(argptr,format);
vsprintf(str,format,argptr);
va_end(argptr);

raydium_camera_replace();
glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
glGetDoublev(GL_PROJECTION_MATRIX, projectionMatrix);
glGetIntegerv(GL_VIEWPORT, viewport);
gluProject(x,y,z,modelMatrix,projectionMatrix,viewport,&sx,&sy,&sz);

sx=sx/raydium_window_tx*100;
sy=sy/raydium_window_ty*100;
if(sz<=1.0) raydium_osd_printf(sx,sy,size,spacer,texture,str);
}

void raydium_osd_logo(char *texture)
{
//GLuint vertex_index_save;
raydium_osd_start();
raydium_texture_current_set_name(texture);

glTranslatef(85,10,0);
glRotatef(raydium_osd_logo_angle+=(raydium_frame_time*60),0,1,0); //must be callbacked !
if(raydium_osd_logo_angle>90) raydium_osd_logo_angle=-90;

raydium_rendering_internal_prepare_texture_render(raydium_texture_current_main);
glBegin(GL_QUADS);
glTexCoord2f(0,0);
glVertex3f(-10,5,0);
glTexCoord2f(1,0);
glVertex3f(10,5,0);
glTexCoord2f(1,1);
glVertex3f(10,-5,0);
glTexCoord2f(0,1);
glVertex3f(-10,-5,0);
glEnd();
//raydium_rendering_internal_restore_render_state();

raydium_osd_stop();
}


void raydium_osd_cursor_set(char *texture,GLfloat xsize, GLfloat ysize)
{
raydium_mouse_hide();
if(texture && strlen(texture))
    raydium_osd_cursor_texture=raydium_texture_find_by_name(texture);
else
    raydium_osd_cursor_texture=0;

raydium_osd_cursor_xsize=xsize;
raydium_osd_cursor_ysize=ysize;
}

void raydium_osd_cursor_draw(void)
{
if(!raydium_osd_cursor_texture || !raydium_window_tx) return;

raydium_osd_start();

glTranslatef(((GLfloat)raydium_mouse_x/raydium_window_tx)*100.f,
((GLfloat)(raydium_window_ty-raydium_mouse_y)/raydium_window_ty)*100.f,0);

raydium_texture_current_set(raydium_osd_cursor_texture);
raydium_rendering_internal_prepare_texture_render(raydium_texture_current_main);
glBegin(GL_QUADS);
glTexCoord2f(0,0);
glVertex3f(0,0,0);
glTexCoord2f(1,0);
glVertex3f(raydium_osd_cursor_xsize,0,0);
glTexCoord2f(1,1);
glVertex3f(raydium_osd_cursor_xsize,-raydium_osd_cursor_ysize,0);
glTexCoord2f(0,1);
glVertex3f(0,-raydium_osd_cursor_ysize,0);
glEnd();
raydium_rendering_internal_restore_render_state();

raydium_osd_stop();

}

void raydium_osd_internal_vertex(GLfloat x, GLfloat y, GLfloat top)
{
if(y>top) y=top;
glVertex3f(x,y,0);
}

void raydium_osd_network_stat_draw(GLfloat px, GLfloat py, GLfloat size)
{
#define RAYDIUM_OSD_NET_SAMPLES		32
#define RAYDIUM_OSD_NET_STEP		0.3
static GLfloat past_delay[RAYDIUM_OSD_NET_SAMPLES];
static GLfloat past_rx[RAYDIUM_OSD_NET_SAMPLES];
static GLfloat past_tx[RAYDIUM_OSD_NET_SAMPLES];
static GLfloat step=0;
static int last_rx,last_tx;

static GLfloat past_reemitted[RAYDIUM_OSD_NET_SAMPLES];
static GLfloat past_double[RAYDIUM_OSD_NET_SAMPLES];
static GLfloat past_lost[RAYDIUM_OSD_NET_SAMPLES];
static GLfloat past_bogus[RAYDIUM_OSD_NET_SAMPLES];

static GLfloat last_reemitted;
static GLfloat last_double;
static GLfloat last_lost;
static GLfloat last_bogus;


int i;
GLfloat fact_x,fact_y_delay,fact_y_rxtx;

fact_x=size/RAYDIUM_OSD_NET_SAMPLES;
fact_y_delay=size/(RAYDIUM_NETWORK_ACK_DELAY_MAX*1000);
fact_y_rxtx=size/50; // 50 KB
step+=raydium_frame_time;

if(step>=RAYDIUM_OSD_NET_STEP)
    {
    step=0;

    // shift array to the left
    for(i=1;i<RAYDIUM_OSD_NET_SAMPLES;i++)
	past_delay[i-1]=past_delay[i];    
    past_delay[RAYDIUM_OSD_NET_SAMPLES-1]=raydium_netwok_queue_ack_delay_client;

    for(i=1;i<RAYDIUM_OSD_NET_SAMPLES;i++)
	past_rx[i-1]=past_rx[i];    
    past_rx[RAYDIUM_OSD_NET_SAMPLES-1]=(raydium_network_stat_rx-last_rx)/1024;
    last_rx=raydium_network_stat_rx;

    for(i=1;i<RAYDIUM_OSD_NET_SAMPLES;i++)
	past_tx[i-1]=past_tx[i];    
    past_tx[RAYDIUM_OSD_NET_SAMPLES-1]=(raydium_network_stat_tx-last_tx)/1024;
    last_tx=raydium_network_stat_tx;

    for(i=1;i<RAYDIUM_OSD_NET_SAMPLES;i++)
	past_reemitted[i-1]=past_reemitted[i];    
    past_reemitted[RAYDIUM_OSD_NET_SAMPLES-1]=(raydium_network_stat_reemitted-last_reemitted)*(size/10);
    last_reemitted=raydium_network_stat_reemitted;

    for(i=1;i<RAYDIUM_OSD_NET_SAMPLES;i++)
	past_double[i-1]=past_double[i];
    past_double[RAYDIUM_OSD_NET_SAMPLES-1]=(raydium_network_stat_double-last_double)*(size/10);
    last_double=raydium_network_stat_double;

    for(i=1;i<RAYDIUM_OSD_NET_SAMPLES;i++)
	past_lost[i-1]=past_lost[i];
    past_lost[RAYDIUM_OSD_NET_SAMPLES-1]=(raydium_network_stat_lost-last_lost)*(size/10);
    last_lost=raydium_network_stat_lost;

    for(i=1;i<RAYDIUM_OSD_NET_SAMPLES;i++)
	past_bogus[i-1]=past_bogus[i];
    past_bogus[RAYDIUM_OSD_NET_SAMPLES-1]=(raydium_network_stat_bogus_ack-last_bogus)*(size/10);
    last_bogus=raydium_network_stat_bogus_ack;

    }

// draw data
raydium_osd_color_ega('0');
raydium_osd_draw_name("rgb(0,0,0)",px,py,px+size,py+size);
raydium_osd_start();

raydium_osd_color_ega('9');
glColor4fv(raydium_osd_color);
glBegin(GL_LINE_STRIP);
for(i=0;i<RAYDIUM_OSD_NET_SAMPLES;i++)
    {
    raydium_osd_internal_vertex(
	px+(i*fact_x),
	py+(past_delay[i]/(double)raydium_timecall_clocks_per_sec*1000*fact_y_delay),
	py+size);
    }
glEnd();

raydium_osd_color_ega('f');
glColor4fv(raydium_osd_color);
glBegin(GL_LINE_STRIP);
for(i=0;i<RAYDIUM_OSD_NET_SAMPLES;i++)
    {
    raydium_osd_internal_vertex(
	px+(i*fact_x),
	py+(past_tx[i]*fact_y_rxtx),
	py+size);
    }
glEnd();

raydium_osd_color_ega('c');
glColor4fv(raydium_osd_color);
glBegin(GL_LINE_STRIP);
for(i=0;i<RAYDIUM_OSD_NET_SAMPLES;i++)
    {
    raydium_osd_internal_vertex(
	px+(i*fact_x),
	py+(past_rx[i]*fact_y_rxtx),
	py+size);
    }
glEnd();

raydium_osd_color_ega('d');
glColor4fv(raydium_osd_color);
glBegin(GL_LINES);
for(i=0;i<RAYDIUM_OSD_NET_SAMPLES;i++)
    {
    raydium_osd_internal_vertex(
	px+(i*fact_x),
	py,
	py+size);
    raydium_osd_internal_vertex(
	px+(i*fact_x),
	py+past_reemitted[i],
	py+size);
    }
glEnd();

raydium_osd_color_ega('e');
glColor4fv(raydium_osd_color);
glBegin(GL_LINES);
for(i=0;i<RAYDIUM_OSD_NET_SAMPLES;i++)
    {
    raydium_osd_internal_vertex(
	px+(i*fact_x),
	py,
	py+size);
    raydium_osd_internal_vertex(
	px+(i*fact_x),
	py+past_double[i],
	py+size);
    }
glEnd();

raydium_osd_color_ega('a');
glColor4fv(raydium_osd_color);
glBegin(GL_LINES);
for(i=0;i<RAYDIUM_OSD_NET_SAMPLES;i++)
    {
    raydium_osd_internal_vertex(
	px+(i*fact_x),
	py,
	py+size);
    raydium_osd_internal_vertex(
	px+(i*fact_x),
	py+past_lost[i],
	py+size);
    }
glEnd();

raydium_osd_color_ega('b');
glColor4fv(raydium_osd_color);
glBegin(GL_LINES);
for(i=0;i<RAYDIUM_OSD_NET_SAMPLES;i++)
    {
    raydium_osd_internal_vertex(
	px+(i*fact_x),
	py,
	py+size);
    raydium_osd_internal_vertex(
	px+(i*fact_x),
	py+past_bogus[i],
	py+size);
    }
glEnd();

raydium_rendering_internal_restore_render_state();
raydium_osd_stop();
raydium_osd_color_ega('f');
}

void raydium_osd_mask(GLfloat *color4)
{
raydium_osd_start();

// invalidate cache
//raydium_texture_internal_loaded=0;
raydium_rendering_internal_prepare_texture_render(0);
glDisable(GL_TEXTURE_2D);
glEnable(GL_BLEND);
glDepthMask(GL_FALSE);
glColor4f(color4[0],color4[1],color4[2],color4[3]);

glBegin(GL_QUADS);
glVertex3f(0,0,0);
glVertex3f(100,0,0);
glVertex3f(100,100,0);
glVertex3f(0,100,0);
glEnd();
raydium_rendering_internal_restore_render_state();

raydium_osd_stop();
}


void raydium_osd_mask_texture_clip(int texture,GLfloat alpha, GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
x1/=100;
y1/=100;
x2/=100;
y2/=100;

raydium_osd_start();

// invalidate cache
//raydium_texture_internal_loaded=0;
raydium_rendering_internal_prepare_texture_render(texture);
glEnable(GL_TEXTURE_2D);
glEnable(GL_BLEND);
glDepthMask(GL_FALSE);
glColor4f(1,1,1,alpha);

glBegin(GL_QUADS);
glTexCoord2f(x1,y1);
glVertex3f(0,0,0);

glTexCoord2f(x2,y1);
glVertex3f(100,0,0);

glTexCoord2f(x2,y2);
glVertex3f(100,100,0);

glTexCoord2f(x1,y2);
glVertex3f(0,100,0);
glEnd();
raydium_rendering_internal_restore_render_state();

raydium_osd_stop();
}

void raydium_osd_mask_texture_clip_name(char *texture,GLfloat alpha, GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
raydium_osd_mask_texture_clip(raydium_texture_find_by_name(texture),alpha,x1,y1,x2,y2);
}

void raydium_osd_mask_texture(int texture,GLfloat alpha)
{
raydium_osd_mask_texture_clip(texture,alpha,0,0,100,100);
}

void raydium_osd_mask_texture_name(char *texture,GLfloat alpha)
{
raydium_osd_mask_texture(raydium_texture_find_by_name(texture),alpha);
}


void raydium_osd_fade_callback(void)
{
int i;
void (*f)(void);

if(raydium_osd_fade_color_timeleft>0)
    {
    raydium_osd_fade_color_timeleft-=raydium_frame_time;
    for(i=0;i<4;i++)
	raydium_osd_fade_color_current[i]+=
	    raydium_osd_fade_color_increment[i]*raydium_frame_time;
    
    raydium_osd_mask(raydium_osd_fade_color_current);
    if(raydium_osd_fade_color_timeleft<=0 && raydium_osd_fade_OnFadeEnd)
	{
	f=raydium_osd_fade_OnFadeEnd;
	f();
	return; // fade may have changed during playback ...
	}
    }
}


void raydium_osd_fade_init(void)
{
raydium_osd_fade_color_timeleft=-1;
memset(raydium_osd_fade_color_increment,0,raydium_internal_size_vector_float_4);
memset(raydium_osd_fade_color_current,0,raydium_internal_size_vector_float_4);
raydium_osd_fade_OnFadeEnd=NULL;
}

void raydium_osd_fade_from(GLfloat *from4, GLfloat *to4, GLfloat time_len, void *OnFadeEnd)
{
int i;

raydium_osd_fade_color_timeleft=time_len;
memcpy(raydium_osd_fade_color_current,from4,raydium_internal_size_vector_float_4);
raydium_osd_fade_OnFadeEnd=OnFadeEnd;
for(i=0;i<4;i++)
    raydium_osd_fade_color_increment[i]=(to4[i]-from4[i])/time_len; // per sec
}
