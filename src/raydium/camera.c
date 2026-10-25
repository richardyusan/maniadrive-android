/*
    Raydium - CQFD Corp.
    http://raydium.org/
    License: GPL - GNU General Public License, see "gpl.txt" file.
*/

#ifndef DONT_INCLUDE_HEADERS
#include "index.h"
#else
#include "headers/camera.h"
#endif 

// res3 is GLfloat[3]
void raydium_camera_vectors(GLfloat *res3)
{
GLfloat m[16];
GLfloat front[3]={1,0,0};
//GLfloat up[3]={0,0,1};
GLfloat res1[3];
GLfloat res2[3];


raydium_trigo_pos_get_modelview(res1); // get current position
raydium_trigo_pos_to_matrix(front,m); // create matrix using front
glPushMatrix();
glMultMatrixf(m); // project front
raydium_trigo_pos_get_modelview(res2); // get new position
glPopMatrix();

// create front vector
res3[1]=-(res1[0]-res2[0]); // x
res3[2]=(res1[1]-res2[1]); // y
res3[0]=(res1[2]-res2[2]); // z

// create up vector

// fallback
res3[3]=res3[4]=0;
res3[5]=1;

/*
raydium_trigo_pos_get_modelview(res1); // get current position
raydium_trigo_pos_to_matrix(up,m); // create matrix using front
glPushMatrix();
glMultMatrixf(m); // project front
raydium_trigo_pos_get_modelview(res2); // get new position
glPopMatrix();
res3[4]=(res1[0]-res2[0]); // x
res3[5]=-(res1[1]-res2[1]); // y
res3[3]=-(res1[2]-res2[2]); // z
*/

//raydium_log("%f %f %f",res3[3],res3[4],res3[5]);
}


void raydium_camera_internal_prepare(void)
{
GLfloat x,y,z;

glLoadIdentity();

if(raydium_camera_rumble_remaining>0)
    {
    x=raydium_random_f(-raydium_camera_rumble_amplitude,raydium_camera_rumble_amplitude);
    y=raydium_random_f(-raydium_camera_rumble_amplitude,raydium_camera_rumble_amplitude);
    z=raydium_random_f(-raydium_camera_rumble_amplitude,raydium_camera_rumble_amplitude);
	
    glRotatef(z,0,0,1);
    glRotatef(x,1,0,0);
    glRotatef(y,0,1,0);
	
    raydium_camera_rumble_remaining-=raydium_frame_time;
    raydium_camera_rumble_amplitude+=(raydium_camera_rumble_evolution*raydium_frame_time);
    if(raydium_camera_rumble_amplitude<=0)
	{
	raydium_camera_rumble_amplitude=0;
	raydium_camera_rumble_remaining=0;
	}
    }
else raydium_camera_rumble_remaining=0;
}


void raydium_camera_internal(GLfloat x, GLfloat y, GLfloat z)
{
if(raydium_frame_first_camera_pass)
    {
    float pos[3];
    float or[6];
    pos[0]=x;
    pos[1]=y;
    pos[2]=z;
    if(raydium_sound) 
	{
	raydium_camera_vectors(or); // get vectors
	raydium_sound_SetListenerPos(pos);
	raydium_sound_SetListenerOr(or);
	}
    
    if(raydium_sky_atmosphere_check())
	{
	raydium_sky_box_render(x,y,z);
	raydium_sky_atmosphere_render(x,y,z,RAYDIUM_SKY_SPHERE_DEFAULT_DETAIL);
	}
    else
	{
	raydium_sky_box_render(x,y,z);
	}
    
    //raydium_sky_box_render(x,y,z);
    //raydium_atmosphere_render(x,y,z,SPHERE_DEFAULT_DETAIL);
    raydium_frame_first_camera_pass=0;
    raydium_light_update_position_all();
    }

if(!raydium_camera_pushed)
    {
    raydium_camera_pushed=1;
    raydium_camera_x=x; 
    raydium_camera_y=y;
    raydium_camera_z=z;

    glPushMatrix();
    memset(raydium_camera_cursor_place,0,3*sizeof(GLfloat));
    }
else raydium_log("Warning: too many calls to camera_* ! (matrix already pushed)");
}


void raydium_camera_place(GLfloat x, GLfloat y, GLfloat z,
			  GLfloat lacet, GLfloat tangage, GLfloat roulis)
{
//glLoadIdentity();
raydium_camera_internal_prepare();
glRotatef(roulis,0,0,1);
glRotatef(tangage,1,0,0);
glRotatef(lacet,0,1,0);
glTranslatef(x, y, z);
glRotatef(90,0,0,1);
glRotatef(90,0,1,0);
raydium_camera_internal(z,x,-y);
}


void raydium_camera_look_at(GLfloat x, GLfloat y, GLfloat z,
			  GLfloat x_to, GLfloat y_to, GLfloat z_to)
{
//glLoadIdentity();
raydium_camera_internal_prepare();
glRotatef(raydium_camera_look_at_roll,0,0,1);
raydium_camera_look_at_roll=0;
gluLookAt(x,y,z,z_to,x_to,-y_to,0.,0.,1.);
raydium_camera_internal(x,y,z);
}

void raydium_camera_replace(void)
{
if(raydium_shadow_rendering)
    {
    glLoadIdentity();
    return;
    }

if(!raydium_camera_pushed)
 raydium_log("Warning: no camera to replace (matrix stack's empty)");
else
 {
 glPopMatrix();
 glPushMatrix();
 memset(raydium_camera_cursor_place,0,3*sizeof(GLfloat));
 }
}

void raydium_camera_replace_go(GLfloat *pos, GLfloat *R)
{
// pos[3], R[3*3]

GLfloat matrix[16];

raydium_camera_replace();
matrix[0]=R[0];
matrix[1]=R[4];
matrix[2]=R[8];
matrix[3]=0;
matrix[4]=R[1];
matrix[5]=R[5];
matrix[6]=R[9];
matrix[7]=0;
matrix[8]=R[2];
matrix[9]=R[6];
matrix[10]=R[10];
matrix[11]=0;
matrix[12]=pos[0];
matrix[13]=pos[1];
matrix[14]=pos[2];
matrix[15]=1;
glMultMatrixf(matrix);
memcpy(raydium_camera_cursor_place,pos,3*sizeof(GLfloat));
}


void raydium_camera_rumble(GLfloat amplitude, GLfloat ampl_evo, GLfloat secs)
{
raydium_camera_rumble_amplitude=amplitude;
raydium_camera_rumble_evolution=ampl_evo;
raydium_camera_rumble_remaining=secs;
}

void raydium_camera_path_reset(void)
{
raydium_camera_path_reset_flag=1;
}

// if step is <= 0, moves will be instaneous
// camera will be placed only if step is >=0 (negative steps are used
// only to change internal vars)
void raydium_camera_smooth(GLfloat px, GLfloat py, GLfloat pz, 
			   GLfloat lx, GLfloat ly, GLfloat lz, 
			   GLfloat zoom, GLfloat roll, GLfloat step)
{
static GLfloat opx,opy,opz;
static GLfloat olx,oly,olz;
static GLfloat ozoom=90;
static GLfloat oroll=0;

//raydium_log("camera smooth (asked): %.2f %.2f %.2f | %.2f %.2f %.2f | %.2f %.2f",px,py,pz,lx,ly,lz,zoom,step);
    
if(step<=0 || // wow.. smells inf, do a instantaneous step. (and don't place cam)
   raydium_camera_path_reset_flag)
{
    opx=px;
    opy=py;
    opz=pz;
    olx=lx;
    oly=ly;
    olz=lz;
    ozoom=zoom;
    oroll=roll;
    raydium_camera_path_reset_flag=0;
}
else
{
    opx+=(px-opx)*step;
    opy+=(py-opy)*step;
    opz+=(pz-opz)*step;
    olx+=(lx-olx)*step;
    oly+=(ly-oly)*step;
    olz+=(lz-olz)*step;
    ozoom+=(zoom-ozoom)*step;
    oroll+=(roll-oroll)*step;

    if(ozoom<0) ozoom=0;
    if(ozoom>180) ozoom=270;

    if(ozoom!=raydium_projection_fov)
        {
        raydium_projection_fov=ozoom;
        raydium_window_view_update();
        }
}

raydium_camera_look_at_roll=oroll;

if(step>=0)
    raydium_camera_look_at(opx,opy,opz,olx,oly,olz);
//raydium_log("camera smooth (result): %.2f %.2f %.2f | %.2f %.2f %.2f | %.2f | %.2f",opx,opy,opz,olx,oly,olz,ozoom,step);
}


void raydium_camera_path_init(int p)
{
int i;
raydium_camera_path[p].name[0]=0;
raydium_camera_path[p].steps=-1;
for(i=0;i<RAYDIUM_MAX_CAMERA_PATH_STEPS;i++)
    {
    raydium_camera_path[p].x[i]=0;
    raydium_camera_path[p].y[i]=0;
    raydium_camera_path[p].z[i]=0;
    raydium_camera_path[p].zoom[i]=0;
    raydium_camera_path[p].roll[i]=0;
    }
}

void raydium_camera_path_init_all(void)
{
int i;
for(i=0;i<RAYDIUM_MAX_CAMERA_PATHS;i++)
    raydium_camera_path_init(i);
}


int raydium_camera_path_find(char *name)
{
int i;
for(i=0;i<RAYDIUM_MAX_CAMERA_PATHS;i++)
    if(!strcmp(raydium_camera_path[i].name,name) && raydium_camera_path[i].steps>-1)
	return i;
return -1;
}


int raydium_camera_path_load(char *filename)
{
FILE *fp;
int i=0;
GLfloat x,y,z,zoom,roll;
int p;

fp=raydium_file_fopen(filename,"rt");
if(!fp)
    {
    raydium_log("camera: cannot open camera path '%s'",filename);
    return -1;
    }

for(p=0;p<RAYDIUM_MAX_CAMERA_PATHS;p++)
    if(raydium_camera_path[p].steps==-1)
	break;

if(p==RAYDIUM_MAX_CAMERA_PATHS)
    {
    raydium_log("camera: cannot find any free slot !",filename);
    return -1;
    }

strcpy(raydium_camera_path[p].name,filename);

while( fscanf(fp,"%f %f %f %f %f\n",&x,&y,&z,&zoom,&roll)!=EOF )
 {
  raydium_camera_path[p].x[i]=x;
  raydium_camera_path[p].y[i]=y;
  raydium_camera_path[p].z[i]=z;
  raydium_camera_path[p].zoom[i]=zoom;
  raydium_camera_path[p].roll[i]=roll;
  i++;
 }

raydium_camera_path[p].steps=i;
raydium_log("camera path '%s' loaded (slot %i, %i steps)",filename,p,i);
return p;
}


void raydium_camera_path_draw(int p)
{
int i;

if(p>=0 && p<RAYDIUM_MAX_CAMERA_PATHS)
    {
    glDisable(GL_LIGHTING);
    raydium_texture_current_set_name("rgb(1,0,0)");
    raydium_rendering_internal_prepare_texture_render(raydium_texture_current_main);
    glLineWidth(1.f);
    glBegin(GL_LINE_LOOP);

    for(i=0;i<raydium_camera_path[p].steps;i++)
	{
	glVertex3f(
		raydium_camera_path[p].x[i],
		raydium_camera_path[p].y[i],
		raydium_camera_path[p].z[i]);
	}

    glEnd();
    if(raydium_light_enabled_tag)
	glEnable(GL_LIGHTING);
	     
    return;
    }
raydium_log("camera path draw failed : invalid index");
}

void raydium_camera_path_draw_name(char *path)
{
int p;

p=raydium_camera_path_find(path);
if(p==-1) p=raydium_camera_path_load(path);

raydium_camera_path_draw(p);
}


signed char raydium_camera_smooth_path(char *path, GLfloat step, GLfloat *x, GLfloat *y, GLfloat *z, GLfloat *zoom, GLfloat *roll)
{
int p;
int step1,step2;
GLfloat vx,vy,vz,vzoom,vroll;
GLfloat diff;

p=raydium_camera_path_find(path);

if(p==-1)
    p=raydium_camera_path_load(path);
    
if(p==-1)
    return 0;

step1=(int)step;
step2=step1+1;
diff=step-step1;

while(step1>=raydium_camera_path[p].steps)
    {
    step1-=raydium_camera_path[p].steps;
    }

while(step2>=raydium_camera_path[p].steps)
    {
    step2-=raydium_camera_path[p].steps;
    }

vx=raydium_camera_path[p].x[step2]-raydium_camera_path[p].x[step1];
vy=raydium_camera_path[p].y[step2]-raydium_camera_path[p].y[step1];
vz=raydium_camera_path[p].z[step2]-raydium_camera_path[p].z[step1];
vzoom=raydium_camera_path[p].zoom[step2]-raydium_camera_path[p].zoom[step1];
vroll=raydium_camera_path[p].roll[step2]-raydium_camera_path[p].roll[step1];

*x=raydium_camera_path[p].x[step1]+(vx*diff);
*y=raydium_camera_path[p].y[step1]+(vy*diff);
*z=raydium_camera_path[p].z[step1]+(vz*diff);
*zoom=raydium_camera_path[p].zoom[step1]+(vzoom*diff);
*roll=raydium_camera_path[p].roll[step1]+(vroll*diff);

return 1;
}


void raydium_camera_smooth_path_to_pos(char *path, GLfloat lx, GLfloat ly, GLfloat lz, GLfloat path_step, GLfloat smooth_step)
{
GLfloat x,y,z,zoom,roll;

if(raydium_camera_smooth_path(path,path_step,&x,&y,&z,&zoom,&roll)==-1)
    raydium_log("camera path error with '%s'",path);

raydium_camera_smooth(x,y,z,ly,-lz,lx,zoom,roll,smooth_step);
}

void raydium_camera_smooth_pos_to_path(GLfloat lx, GLfloat ly, GLfloat lz, char *path, GLfloat path_step, GLfloat smooth_step)
{
GLfloat x,y,z,zoom,roll;

if(raydium_camera_smooth_path(path,path_step,&x,&y,&z,&zoom,&roll)==-1)
    raydium_log("camera path error with '%s'",path);

raydium_camera_smooth(lx,ly,lz,y,-z,x,zoom,roll,smooth_step);
}

void raydium_camera_smooth_path_to_path(char *path_from, GLfloat path_step_from, char *path_to, GLfloat path_step_to, GLfloat smooth_step)
{
GLfloat fx,fy,fz,fzoom,froll;
GLfloat tx,ty,tz,tzoom,troll;

if(raydium_camera_smooth_path(path_from,path_step_from,&fx,&fy,&fz,&fzoom,&froll)==-1)
    raydium_log("camera path error with '%s' (from)",path_from);

if(raydium_camera_smooth_path(path_to,path_step_to,&tx,&ty,&tz,&tzoom,&troll)==-1)
    raydium_log("camera path error with '%s' (to)",path_to);

raydium_camera_smooth(fx,fy,fz, ty,-tz,tx,fzoom,froll,smooth_step);
}
