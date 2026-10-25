/*
    Raydium - CQFD Corp.
    http://raydium.org/
    License: GPL - GNU General Public License, see "gpl.txt" file.
*/

#ifndef DONT_INCLUDE_HEADERS
#include "index.h"
#else
#include "headers/object.h"
#endif 

GLint raydium_object_find(char *name)
{
GLint i;
for(i=0;i<raydium_object_index;i++)
if(!strcmp(raydium_object_name[i],name)) return i;

return -1;
}

signed char raydium_object_isvalid(int obj)
{
if(obj>=0 && obj<raydium_object_index)
    return 1;
return 0;
}


void raydium_object_reset(GLuint o)
{
raydium_object_name[o][0]=0;
raydium_object_start[o]=0;
raydium_object_end[o]=0;
raydium_object_anims[o]=0;
}


int raydium_object_load(char *filename)
{
if(raydium_object_find(filename)>=0)
    {
    raydium_log("ERROR: object: %s already loaded",filename);
    return -1;
    }
raydium_object_start[raydium_object_index]=raydium_vertex_index;
read_vertex_from(filename);
raydium_object_end[raydium_object_index]=raydium_vertex_index;
strcpy(raydium_object_name[raydium_object_index],filename);
return(raydium_object_index++);
}

GLint raydium_object_find_load(char *name)
{
int ret;

ret=raydium_object_find(name);
if(ret<0) return raydium_object_load(name);
return ret;
}


void raydium_object_draw(GLuint o)
{
#ifndef DEBUG_RENDER_DISABLE_DISPLAYLISTS
static GLuint dl[RAYDIUM_MAX_OBJECTS];
static char dl_state[RAYDIUM_MAX_OBJECTS];
static int first=0;
int i;
#endif

if(!raydium_object_isvalid(o))
    {
    raydium_log("object: draw: ERROR: id or name is invalid");
    return;
    }


// if animated : draw "only" first (generated) frame
if(raydium_object_anims[o]>0)
    {
    raydium_object_anim_generate_internal(o,raydium_object_anim_instance_current[o]);
    raydium_rendering_from_to(raydium_object_start[o],raydium_object_start[o]+raydium_object_anim_len[o]);
    return;
    }
// ...


#ifndef DEBUG_RENDER_DISABLE_DISPLAYLISTS
if(first)
    for(i=0;i<RAYDIUM_MAX_OBJECTS;i++)
	dl_state[i]=-1;


if(raydium_render_displaylists_tag && !raydium_shadow_rendering)
{
 if(!dl_state[o])
    {
    // build DL
    dl_state[o]=1;
    dl[o]=glGenLists(1);
    raydium_log("Object: creating display list for object %s",raydium_object_name[o]);
    glNewList(dl[o],GL_COMPILE);
    raydium_rendering_from_to(raydium_object_start[o],raydium_object_end[o]);
    glEndList();
    }
  glCallList(dl[o]);
}
else raydium_rendering_from_to(raydium_object_start[o],raydium_object_end[o]);

#else
// No display lists, draw 
raydium_rendering_from_to(raydium_object_start[o],raydium_object_end[o]);
#endif
}

void raydium_object_draw_name(char *name)
{
GLint i;

i=raydium_object_find(name);

if(i>=0) raydium_object_draw(i); 
else 
raydium_object_draw(raydium_object_load(name));
}


void raydium_object_deform(GLuint obj,GLfloat ampl)
{
GLuint i;

if(!raydium_object_isvalid(obj))
    {
    raydium_log("object: deform: ERROR: id or name is invalid");
    return;
    }

for(i=raydium_object_start[obj];i<raydium_object_end[obj];i++)
    {
    raydium_vertex_x[i]+=raydium_random_neg_pos_1()*ampl;
    raydium_vertex_y[i]+=raydium_random_neg_pos_1()*ampl;
    raydium_vertex_z[i]+=raydium_random_neg_pos_1()*ampl;
    }

/*
GLfloat ox,oy,oz;
GLfloat nx,ny,nz;
GLint j;

for(i=raydium_object_start[obj];i<raydium_object_end[obj];i++)
    {
    ox=raydium_vertex_x[i];
    oy=raydium_vertex_y[i];
    oz=raydium_vertex_z[i];

    nx=ox+raydium_random_neg_pos_1()*ampl;
    ny=oy+raydium_random_neg_pos_1()*ampl;
    nz=oz+raydium_random_neg_pos_1()*ampl;

    for(j=i;j<raydium_object_end[obj];j++)
	if(raydium_vertex_x[j]==ox && 
	   raydium_vertex_y[j]==oy && 
	   raydium_vertex_z[j]==oz)
	{
	raydium_vertex_x[j]=nx;
	raydium_vertex_y[j]=ny;
	raydium_vertex_z[j]=nz;
	}
    }*/
}


void raydium_object_deform_name(char *name,GLfloat ampl)
{
raydium_object_deform(raydium_object_find(name),ampl);
}


GLfloat raydium_object_find_dist_max(GLuint obj)
{
GLfloat max=0,val;
int i;
int start,end;

if(!raydium_object_isvalid(obj))
    {
    raydium_log("object: find_dist_max: ERROR: id or name is invalid");
    return -1;
    }

if(raydium_object_anims[obj]>0)
    {
    raydium_object_anim_generate_internal(obj,raydium_object_anim_instance_current[obj]);
    start=raydium_object_start[obj];
    end=raydium_object_start[obj]+raydium_object_anim_len[obj];
    }
else
    {
    start=raydium_object_start[obj];
    end=raydium_object_end[obj];
    }

for(i=start;i<end;i++)
    {
    val=sqrt((raydium_vertex_x[i]*raydium_vertex_x[i])+
	     (raydium_vertex_y[i]*raydium_vertex_y[i])+
	    (raydium_vertex_z[i]*raydium_vertex_z[i]) );
    if(val>max) max=val;
    }
return max;
}

void raydium_object_find_axes_max(GLuint obj, GLfloat *tx, GLfloat *ty, GLfloat *tz)
{
int i;
int start,end;

if(!raydium_object_isvalid(obj))
    {
    raydium_log("object: find_axes_max: ERROR: id or name is invalid");
    return;
    }

if(raydium_object_anims[obj]>0)
    {
    raydium_object_anim_generate_internal(obj,raydium_object_anim_instance_current[obj]);
    start=raydium_object_start[obj];
    end=raydium_object_start[obj]+raydium_object_anim_len[obj];
    }
else
    {
    start=raydium_object_start[obj];
    end=raydium_object_end[obj];
    }


*tx=*ty=*tz=0;

for(i=start;i<end;i++)
    {
    if(raydium_trigo_abs(raydium_vertex_x[i])>*tx) *tx=raydium_trigo_abs(raydium_vertex_x[i]);
    if(raydium_trigo_abs(raydium_vertex_y[i])>*ty) *ty=raydium_trigo_abs(raydium_vertex_y[i]);
    if(raydium_trigo_abs(raydium_vertex_z[i])>*tz) *tz=raydium_trigo_abs(raydium_vertex_z[i]);
    }
*tx*=2;
*ty*=2;
*tz*=2;
}


void raydium_object_find_minmax(GLuint obj, GLfloat *min, GLfloat *max)
{
int i;
int start,end;
//GLfloat min[3];
//GLfloat max[3];

if(!raydium_object_isvalid(obj))
    {
    raydium_log("object: find_size: ERROR: id or name is invalid");
    return;
    }

if(raydium_object_anims[obj]>0)
    {
    raydium_object_anim_generate_internal(obj,raydium_object_anim_instance_current[obj]);
    start=raydium_object_start[obj];
    end=raydium_object_start[obj]+raydium_object_anim_len[obj];
    }
else
    {
    start=raydium_object_start[obj];
    end=raydium_object_end[obj];
    }

min[0]=max[0]=raydium_vertex_x[start];
min[1]=max[1]=raydium_vertex_y[start];
min[2]=max[2]=raydium_vertex_z[start];

for(i=start+1;i<end;i++)
    {
    if(raydium_vertex_x[i]<min[0])
	min[0]=raydium_vertex_x[i];
    if(raydium_vertex_y[i]<min[1])
	min[1]=raydium_vertex_y[i];
    if(raydium_vertex_z[i]<min[2])
	min[2]=raydium_vertex_z[i];

    if(raydium_vertex_x[i]>max[0])
	max[0]=raydium_vertex_x[i];
    if(raydium_vertex_y[i]>max[1])
	max[1]=raydium_vertex_y[i];
    if(raydium_vertex_z[i]>max[2])
	max[2]=raydium_vertex_z[i];
    }
/*tx=(max[0]-min[0]);
*ty=(max[1]-min[1]);
*tz=(max[2]-min[2]);*/
}


void raydium_object_find_center_factors(GLuint obj, GLfloat *tx, GLfloat *ty, GLfloat *tz)
{
GLfloat min[3];
GLfloat max[3];

raydium_object_find_minmax(obj,min,max);
*tx=(max[0]-((max[0]-min[0])/2));
*ty=(max[1]-((max[1]-min[1])/2));
*tz=(max[2]-((max[2]-min[2])/2));

(*tx)/=(max[0]-min[0]);
(*ty)/=(max[1]-min[1]);
(*tz)/=(max[2]-min[2]);
}



//////////////////////////////////////////////////////////
// Animation support starts here


GLint raydium_object_anim_find(int object, char *name)
{
int i;

if(!raydium_object_isvalid(object))
    {
    raydium_log("object: anim_find: ERROR: id or name is invalid");
    return -1;
    }

for(i=0;i<raydium_object_anims[object];i++)
    if(!strcmp(raydium_object_anim_names[object][i],name))
	return i;

return -1;
}

#define _pondavg(a,b,f) ( (a)+(((b)-(a))*(f)) )

// There's always a bug here, causing jitters when switching animation:
// frame_b jumps from "destination frame" to "destination frame"+1, as
// during normal operation
void raydium_object_anim_generate_internal(int object, int instance)
{
int i;
GLfloat factor;
int anim_frames;
int frame_a,frame_b;
int anim_current;
GLfloat anim_frame_current;
char flag;

if(!raydium_object_isvalid(object))
    {
    raydium_log("object: generate_internal: ERROR: id or name is invalid");
    return;
    }

// here starts the big job ! :)
anim_current=raydium_object_anim_current[object][instance];
anim_frame_current=raydium_object_anim_frame_current[object][instance];


// How many frames for the current anim ?
anim_frames=
    raydium_object_anim_end[object][anim_current] - 
    raydium_object_anim_start[object][anim_current];

flag=0;
// slow ... :( (any good idea to make a modulo on a float ?)
while(anim_frame_current>(anim_frames+1))
    {
    anim_frame_current-=(anim_frames+1);
    flag=1;
    }

// end of ponctually anim ?
if(flag && raydium_object_anim_punctually_flag[object][instance]>=0)
    {
    raydium_object_anim_punctually_flag[object][instance]=-1;
    raydium_object_anim(object,instance,raydium_object_anim_default_anim[object]);
    raydium_object_anim_frame(object,instance,0);
    // recursive call with regular situation
    raydium_object_anim_generate_internal(object,instance);
    return;
    }

//printf("frame (int): %i\n",(int)raydium_object_anim_frame_current[object][instance]);
//printf("%f %i\n",raydium_object_anim_frame_current[object],anim_frames);

factor=anim_frame_current-(int)anim_frame_current;

frame_a=raydium_object_start[object]+
       (raydium_object_anim_start[object][anim_current] *
        raydium_object_anim_len[object]) +
       (((int)anim_frame_current) *
        raydium_object_anim_len[object]) +
       raydium_object_anim_len[object];


if( ((int)anim_frame_current) >= anim_frames)
    {
    frame_b=raydium_object_start[object] +
           (raydium_object_anim_start[object][anim_current] *
            raydium_object_anim_len[object]) +
    	    raydium_object_anim_len[object];
    }
else    
    frame_b=frame_a+raydium_object_anim_len[object];


// if we come from another anim, let's erase frame_a
if(raydium_object_anim_previous[object][instance]>=0)
    {

    // is it the first pass right after anim change ?
    if(raydium_object_anim_frame_previous_timeout[object][instance]==-1)
	{
	// save current frame
	raydium_object_anim_frame_previous_timeout[object][instance]=raydium_object_anim_frame_current[object][instance];
	//printf("*** start\n");
	}

    // We're now in current anim, cancel previous one
    if(raydium_object_anim_frame_current[object][instance]-raydium_object_anim_frame_previous_timeout[object][instance]>=1)
	{
	raydium_object_anim_previous[object][instance]=-1;
	//printf("*** end\n");
	}
    else
	{
	// ... erase frame_a
	//printf("%f| erasing %i (%f)",anim_frame_current,anim_current,anim_frame_current);
	anim_current=raydium_object_anim_previous[object][instance];
	anim_frame_current=raydium_object_anim_frame_previous[object][instance];

	anim_frames=
	    raydium_object_anim_end[object][anim_current] - 
	    raydium_object_anim_start[object][anim_current];

	// slow ... :( (any good idea to make a modulo on a float ?)
	while(anim_frame_current>(anim_frames+1))
	    anim_frame_current-=(anim_frames+1);
	
	//printf(" with %i (%f)\n",anim_current,anim_frame_current);

	factor=(raydium_object_anim_frame_current[object][instance]-raydium_object_anim_frame_previous_timeout[object][instance]);

	frame_a=raydium_object_start[object]+
    	    (raydium_object_anim_start[object][anim_current] *
    	    raydium_object_anim_len[object]) +
    	    (((int)anim_frame_current) *
    	    raydium_object_anim_len[object]) +
    	    raydium_object_anim_len[object];
	}
    
    //printf("refresh from %i/%i (a) and %i/%i (b), factor = %.2f (%i af)\n",frame_a,frame_a/raydium_object_anim_len[object],frame_b,frame_b/raydium_object_anim_len[object],factor,anim_frames);
    }

//printf("refresh from %i/%i (a) and %i/%i (b), factor = %.2f (%i af)\n",frame_a,frame_a/raydium_object_anim_len[object],frame_b,frame_b/raydium_object_anim_len[object],factor,anim_frames);


for(i=0;i<raydium_object_anim_len[object];i++)
    {
    raydium_vertex_x[raydium_object_start[object]+i]=_pondavg(raydium_vertex_x[frame_a+i],raydium_vertex_x[frame_b+i],factor);
    raydium_vertex_y[raydium_object_start[object]+i]=_pondavg(raydium_vertex_y[frame_a+i],raydium_vertex_y[frame_b+i],factor);
    raydium_vertex_z[raydium_object_start[object]+i]=_pondavg(raydium_vertex_z[frame_a+i],raydium_vertex_z[frame_b+i],factor);

    raydium_vertex_normal_visu_x[raydium_object_start[object]+i]=_pondavg(raydium_vertex_normal_visu_x[frame_a+i],raydium_vertex_normal_visu_x[frame_b+i],factor);
    raydium_vertex_normal_visu_y[raydium_object_start[object]+i]=_pondavg(raydium_vertex_normal_visu_y[frame_a+i],raydium_vertex_normal_visu_y[frame_b+i],factor);
    raydium_vertex_normal_visu_z[raydium_object_start[object]+i]=_pondavg(raydium_vertex_normal_visu_z[frame_a+i],raydium_vertex_normal_visu_z[frame_b+i],factor);

    raydium_vertex_texture_u[raydium_object_start[object]+i]=_pondavg(raydium_vertex_texture_u[frame_a+i],raydium_vertex_texture_u[frame_b+i],factor);
    raydium_vertex_texture_v[raydium_object_start[object]+i]=_pondavg(raydium_vertex_texture_v[frame_a+i],raydium_vertex_texture_v[frame_b+i],factor);

    raydium_vertex_texture[raydium_object_start[object]+i]=raydium_vertex_texture[frame_a+i];
    }
}

void raydium_object_anim_frame(int object, int instance, GLfloat frame)
{
if(!raydium_object_isvalid(object))
    {
    raydium_log("object: anim_frame: ERROR: id or name is invalid");
    return;
    }

raydium_object_anim_frame_current[object][instance]=frame;
}

void raydium_object_anim_frame_name(char *object, int instance, GLfloat frame)
{
raydium_object_anim_frame(raydium_object_find_load(object),instance,frame);
}


void raydium_object_anim(int object, int instance, int anim)
{
if(!raydium_object_isvalid(object))
    {
    raydium_log("object: anim: ERROR: id or name is invalid");
    return;
    }

if(anim<0 || anim>=raydium_object_anims[object])
    {
    raydium_log("object: anim: ERROR: id or name is invalid for animation");
    return;    
    }

if(raydium_object_anim_current[object][instance]==anim)
    return;

//printf("%i asked %i\n",raydium_object_anim_current[object][instance],anim);
// warning, change anim BEFORE anim's frame
raydium_object_anim_previous[object][instance]=raydium_object_anim_current[object][instance];
raydium_object_anim_frame_previous[object][instance]=raydium_object_anim_frame_current[object][instance];
raydium_object_anim_frame_previous_timeout[object][instance]=-1;

raydium_object_anim_current[object][instance]=anim;
}

void raydium_object_anim_name(char *object, int instance, char *anim)
{
int o;
o=raydium_object_find_load(object);
raydium_object_anim(o,instance,raydium_object_anim_find(o,anim));
}


void raydium_object_anim_instance(int object, int instance)
{
if(!raydium_object_isvalid(object))
    {
    raydium_log("object: anim_instance: ERROR: id or name is invalid");
    return;
    }
raydium_object_anim_instance_current[object]=instance;
}

void raydium_object_anim_instance_name(char *object, int instance)
{
raydium_object_anim_instance(raydium_object_find_load(object),instance);
}

void raydium_object_anim_automatic(int object, int anim, GLfloat factor)
{
if(!raydium_object_isvalid(object))
    {
    raydium_log("object: anim_automatic: ERROR: id or name is invalid");
    return;
    }

if(anim<0 || anim>=raydium_object_anims[object])
    {
    raydium_log("object: anim_automatic: ERROR: id or name is invalid for animation");
    return;    
    }

//printf("%i %i %f\n",object,anim,factor);
raydium_object_anim_automatic_factor[object][anim]=factor;
}


void raydium_object_anim_automatic_name(char *object, char *anim, GLfloat factor)
{
int o;

o=raydium_object_find_load(object);

raydium_object_anim_automatic(o,raydium_object_anim_find(o,anim),factor);
}

void raydium_object_callback(void)
{
GLfloat f;
int o,i;

for(o=0;o<raydium_object_index;o++)
    if(raydium_object_anims[o]>0)
	for(i=0;i<RAYDIUM_MAX_OBJECT_ANIM_INSTANCES;i++)    
	    {
	    f=raydium_frame_time * raydium_object_anim_time_factor *
	      raydium_object_anim_automatic_factor[o][raydium_object_anim_current[o][i]];
	    raydium_object_anim_frame_current[o][i]+=f;    
	    }
}


void raydium_object_anim_default(int object, int anim)
{
if(!raydium_object_isvalid(object))
    {
    raydium_log("object: anim_default: ERROR: id or name is invalid");
    return;
    }

if(anim<0 || anim>=raydium_object_anims[object])
    {
    raydium_log("object: anim_default: ERROR: id or name is invalid for animation");
    return;    
    }

raydium_object_anim_default_anim[object]=anim;
}


signed char raydium_object_anim_ispunctually(int object, int instance)
{
if(!raydium_object_isvalid(object))
    {
    raydium_log("object: anim_isdefault: ERROR: id or name is invalid");
    return 0;
    }

return (raydium_object_anim_punctually_flag[object][instance]>=0?1:0);
}

signed char raydium_object_anim_ispunctually_name(char *object, int instance)
{
return raydium_object_anim_ispunctually(raydium_object_find_load(object),instance);
}

// punctually (change current and set flag. set frame to 0 [in,out])
void raydium_object_anim_punctually(int object, int anim, int instance)
{
if(!raydium_object_isvalid(object))
    {
    raydium_log("object: anim_punctually: ERROR: id or name is invalid");
    return;
    }

if(anim<0 || anim>=raydium_object_anims[object])
    {
    raydium_log("object: anim_punctually: ERROR: id or name is invalid for animation");
    return;    
    }

raydium_object_anim(object,instance,anim);
raydium_object_anim_frame(object,instance,0);
raydium_object_anim_punctually_flag[object][instance]=anim;
}


void raydium_object_anim_punctually_name(char *object, char *anim, int instance)
{
int o;

o=raydium_object_find_load(object);
raydium_object_anim_punctually(o,raydium_object_anim_find(o,anim),instance);
}

// no loops for some animation (death)
