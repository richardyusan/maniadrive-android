// ManiaDrive Track Editor - CQFD Corp
// http://maniadrive.raydium.org/
char *version="0.40";
char *title="CQFD Corp. Mania2";

// since we play with our own camera and not Raydium's one:
#define RENDER_DEBUG_NO_CLIP
#define NO_ODE_SUPPORT
#include "raydium/index.c"
#include "mania.h"

#define POP_MODE_ELEM	1
#define POP_MODE_BOX	2

#ifdef WIN32
#define MANIA_BINARY "mania_drive.exe"
#else
#define MANIA_BINARY "./mania_drive.static"
#endif


int grid_load(char *filename);
void grid_save(char *filename);
void build_gui_access(raydium_gui_Object *w);
void build_gui_menu(raydium_gui_Object *w);
void grid_generate_obj(void);
void extract_data(char *from, char *to_name, char *to_author, char *to_gold, char *to_author_time, char *message_file);
void grid_init_all(void);
void box_init_all(void);
void grid_generate_obj(void);
void data_init(void);

GLfloat modl_zoom=10;
GLint px=0,py=0;
GLfloat pz=0;
GLint curobj=0;
GLint curbox=0;
GLint curangle=0;
int pop_mode=POP_MODE_ELEM;
signed char view_glue=0;


int n_boxpresets=0;	
Box boxpreset[MAX_ELEMS];

char current_track[RAYDIUM_MAX_NAME_LEN];

//////////////////////////////////////////////////////////// gui
void btnExitOk(raydium_gui_Object *w)
{
raydium_parser_db_set("Mania2-CurrentTrack",current_track);
exit(0);
}

void btnExitCancel(raydium_gui_Object *w)
{
raydium_gui_window_delete_name("exit");
}

void btnClearOk(raydium_gui_Object *w)
{
grid_init_all();
box_init_all();
data_init();
grid_generate_obj();
current_track[0]=0;
px=0;
py=0;
pz=0;
raydium_gui_window_delete_name("clear");
}

void btnClearCancel(raydium_gui_Object *w)
{
raydium_gui_window_delete_name("clear");
}


void gui_exit(void)
{
int handle;

if(raydium_gui_window_find("exit")>=0)
    {
    btnExitCancel(NULL);
    return;
    }

handle=raydium_gui_window_create("exit",25,45,60,15);
raydium_gui_widget_sizes(0,0,18);
raydium_gui_label_create("lblExit",handle,50,80,"Do you really want to exit ?",0,0,0);
raydium_gui_widget_sizes(15,5,18);
raydium_gui_button_create("btnExitOk",handle,20,15,"OK",btnExitOk);
raydium_gui_button_create("btnExitCancel",handle,50,15,"Cancel",btnExitCancel);

raydium_gui_widget_focus_name("btnExitOk","exit");
}

void btnLoad(raydium_gui_Object *w)
{
char str[RAYDIUM_MAX_NAME_LEN];
raydium_gui_read_name("menu","edtFilename",str);
if(grid_load(str))
    build_gui_access(NULL);
}

void btnSave(raydium_gui_Object *w)
{
char str[RAYDIUM_MAX_NAME_LEN];
raydium_gui_read_name("menu","edtFilename",str);
grid_save(str);
build_gui_access(NULL);
}

void btnExit(raydium_gui_Object *w)
{
gui_exit();
}

void btnMetaOk(raydium_gui_Object *w)
{
char d[5][512];

raydium_gui_read_name("menu","edtName",d[0]);
raydium_gui_read_name("menu","edtAuthor",d[1]);
raydium_gui_read_name("menu","edtGold",d[2]);
raydium_gui_read_name("menu","edtAuthorTime",d[3]);
raydium_gui_read_name("menu","edtMsg",d[4]);

sprintf(tdata,"%s;%s;%s;%s;%s",d[0],d[1],d[2],d[3],d[4]);
build_gui_menu(NULL);
}


void btnTest(raydium_gui_Object *w)
{
char str[RAYDIUM_MAX_NAME_LEN*2];
char mni[RAYDIUM_MAX_NAME_LEN];
char *mni_simple="dyn_track.mni";
int ret;

// must protect mni from shell
raydium_file_home_path_cpy(mni_simple,mni);

strcpy(str,current_track);
grid_save(mni);
strcpy(current_track,str);

sprintf(str,"%s --mni \"%s\"",MANIA_BINARY,mni);
ret=system(str);
if(ret!=0)
    {
    raydium_log("CANNOT TEST MNI TRACK FROM '%s'",mni);
    }		
build_gui_access(NULL);
}

void build_gui_clear(raydium_gui_Object *w)
{
int handle;

handle=raydium_gui_window_create("clear",25,45,60,15);
raydium_gui_widget_sizes(0,0,18);
raydium_gui_label_create("lblClear",handle,50,80,"Do you really want to clear track ?",0,0,0);
raydium_gui_widget_sizes(15,5,18);
raydium_gui_button_create("btnClearOk",handle,20,15,"OK",btnClearOk);
raydium_gui_button_create("btnClearCancel",handle,50,15,"Cancel",btnClearCancel);

raydium_gui_widget_focus_name("btnClearOk","clear");
}


#undef YDEC
#define YDEC 5.f
void build_gui_help(raydium_gui_Object *w)
{
int handle;
float y;

raydium_gui_window_delete_name("menu");
handle=raydium_gui_window_create("menu",10,25,80,50);

raydium_gui_widget_sizes(0,0,18);
raydium_gui_label_create("lblTitle",handle,50,93,"Key help for Mania2 Track Editor",0.3,0,0);

y=90;
raydium_gui_widget_sizes(0,0,14);
raydium_gui_label_create("lblh0",handle,50,y-=YDEC,"mouse left click : move view",0,0,0);
raydium_gui_label_create("lblh1",handle,50,y-=YDEC,"mouse right click : move view (glue)",0,0,0);
raydium_gui_label_create("lblh2",handle,50,y-=YDEC,"+/- : change zoom",0,0,0);
raydium_gui_label_create("lblh3",handle,50,y-=YDEC,"arrows : move block",0,0,0);
raydium_gui_label_create("lblh4",handle,50,y-=YDEC,"pageUp/pageDown : change block",0,0,0);
raydium_gui_label_create("lbli4",handle,50,y-=YDEC,"home/end : move block up and down (move view to see)",0,0,0);
raydium_gui_label_create("lblh5",handle,50,y-=YDEC,"d : delete block",0,0,0);
raydium_gui_label_create("lblh6",handle,50,y-=YDEC,"tab : rotate block (90°)",0,0,0);
raydium_gui_label_create("lblh7",handle,50,y-=YDEC,"space : add block",0,0,0);
raydium_gui_label_create("lblh8",handle,50,y-=YDEC,"enter : switch between blocks and entities",0,0,0);
raydium_gui_label_create("lbli8",handle,50,y-=YDEC,"m : open menu",0,0,0);
raydium_gui_label_create("lblj8",handle,50,y-=YDEC,"shift + T : test track",0,0,0);
raydium_gui_label_create("lblh9",handle,50,y-=YDEC,"esc : quits",0,0,0);
y-=YDEC;
raydium_gui_label_create("lblh10",handle,50,y-=YDEC,"               entities: ^agreen^0=start, ^cred^0=end, ^9blue^0=checkpoint, ^6yellow^0=turbo",0,0,0);

raydium_gui_widget_sizes(10,3,14);
raydium_gui_button_create("btnOk",handle,45,3,"OK",build_gui_menu);

raydium_gui_widget_focus_name("btnOk","menu");
}


void build_gui_meta(raydium_gui_Object *w)
{
int handle;
char d[5][512];

extract_data(tdata,d[0],d[1],d[2],d[3],d[4]);

raydium_gui_window_delete_name("menu");
handle=raydium_gui_window_create("menu",10,35,80,30);

raydium_gui_widget_sizes(0,0,18);
raydium_gui_label_create("lblTitle",handle,50,93,"Editing track meta informations",0.3,0,0);

raydium_gui_widget_sizes(0,0,14);
raydium_gui_label_create("lblName",handle,15,80,"Track name",0,0,0);
raydium_gui_widget_sizes(50,3,14);
raydium_gui_edit_create("edtName",handle,30,74,d[0]);

raydium_gui_widget_sizes(0,0,14);
raydium_gui_label_create("lblAuthor",handle,15,65,"Track author",0,0,0);
raydium_gui_widget_sizes(30,3,14);
raydium_gui_edit_create("edtAuthor",handle,30,59,d[1]);

raydium_gui_widget_sizes(0,0,14);
raydium_gui_label_create("lblGold",handle,15,50,"Gold time (secs)",0,0,0);
raydium_gui_widget_sizes(20,3,14);
raydium_gui_edit_create("edtGold",handle,30,44,d[2]);

raydium_gui_widget_sizes(0,0,14);
raydium_gui_label_create("lblAuthorTime",handle,15,35,"Author time (secs)",0,0,0);
raydium_gui_widget_sizes(20,3,14);
raydium_gui_edit_create("edtAuthorTime",handle,30,29,d[3]);

raydium_gui_widget_sizes(0,0,14);
raydium_gui_label_create("lblMsg",handle,15,20,"Message file (opt)",0,0,0);
raydium_gui_widget_sizes(20,3,14);
raydium_gui_edit_create("edtMsg",handle,30,14,d[4]);

raydium_gui_widget_sizes(10,3,14);
raydium_gui_button_create("btnLoad",handle,70,14,"OK",btnMetaOk);
raydium_gui_button_create("btnCancel",handle,85,14,"cancel",build_gui_menu);
}

void build_gui_load(raydium_gui_Object *w)
{
int handle;

raydium_gui_window_delete_name("menu");
handle=raydium_gui_window_create("menu",25,50,50,15);

raydium_gui_widget_sizes(0,0,14);
raydium_gui_label_create("lblAction",handle,25,80,"Load MNI : ",0,0,0);

raydium_gui_widget_sizes(25,3,14);
raydium_gui_edit_create("edtFilename",handle,40,70,current_track);

raydium_gui_widget_sizes(10,3,14);
raydium_gui_button_create("btnLoad",handle,20,20,"load",btnLoad);
raydium_gui_button_create("btnCancel",handle,60,20,"cancel",build_gui_menu);

raydium_gui_widget_focus_name("edtFilename","menu");
}

void build_gui_save(raydium_gui_Object *w)
{
int handle;

raydium_gui_window_delete_name("menu");
handle=raydium_gui_window_create("menu",25,50,50,15);

raydium_gui_widget_sizes(0,0,14);
raydium_gui_label_create("lblAction",handle,25,80,"Save MNI : ",0,0,0);

raydium_gui_widget_sizes(25,3,14);
raydium_gui_edit_create("edtFilename",handle,40,70,current_track);

raydium_gui_widget_sizes(10,3,14);
raydium_gui_button_create("btnSave",handle,20,20,"save",btnSave);
raydium_gui_button_create("btnCancel",handle,60,20,"cancel",build_gui_menu);

raydium_gui_widget_focus_name("edtFilename","menu");
}

#undef YDEC
#define YDEC 10.f
void build_gui_menu(raydium_gui_Object *w)
{
int handle;
float y;

raydium_gui_window_delete_name("menu");
handle=raydium_gui_window_create("menu",75,50,25,50);

y=95;
raydium_gui_widget_sizes(20,3.33,14);
raydium_gui_button_create("btnLoad",handle,10,y-=YDEC,"load",build_gui_load);
raydium_gui_button_create("btnSave",handle,10,y-=YDEC,"save",build_gui_save);
raydium_gui_button_create("btnClear",handle,10,y-=YDEC,"clear all",build_gui_clear);
raydium_gui_button_create("btnEdit",handle,10,y-=YDEC,"edit infos",build_gui_meta);
raydium_gui_button_create("btnTest",handle,10,y-=YDEC,"test track",btnTest);
raydium_gui_button_create("btnHelp",handle,10,y-=YDEC,"key help",build_gui_help);
raydium_gui_button_create("btnClose",handle,10,y-=YDEC,"close menu",build_gui_access);

raydium_gui_button_create("btnExit",handle,10,y-=(YDEC*2),"exit",btnExit);
}


void build_gui_access(raydium_gui_Object *w)
{
int handle;

raydium_gui_window_delete_name("menu");
handle=raydium_gui_window_create("menu",85,95,15,5);
raydium_gui_widget_sizes(10,3.33,14);
raydium_gui_button_create("btnMenu",handle,18,15,"menu",build_gui_menu);
}

/////////////////////////////////////////////////////////// end gui



void grid_init_elem(int e)
{
grid[e].state=0;
grid[e].flags=0;
}

void box_init_elem(int e)
{
box[e].state=0;
box[e].type=0;
}


void grid_init_all(void)
{
int i;

for(i=0;i<MAX_ELEMS;i++)
    grid_init_elem(i);

raydium_log("Grid ok");
}

void box_init_all(void)
{
int i;

for(i=0;i<MAX_ELEMS;i++)
    box_init_elem(i);

raydium_log("Boxes ok");
}


void grid_save(char *filename)
{
FILE *fp;
int i;

fp=raydium_file_fopen(filename,"wt");
if(!fp)
    {
    raydium_log("cannot open '%s' for writing",filename);
    return;
    }

fprintf(fp,"d %s\n",tdata);

for(i=0;i<MAX_ELEMS;i++)
  if(grid[i].state)
    fprintf(fp,"g %f %f %f %i %i %s\n",grid[i].x,
				     grid[i].y,
				     grid[i].z,
				     grid[i].rot,
				     grid[i].flags,
				     raydium_object_name[grid[i].obj]);

for(i=0;i<MAX_ELEMS;i++)
  if(box[i].state)
    fprintf(fp,"b %f %f %f %f %f %f %i\n",box[i].x,
				          box[i].y,
				          box[i].z,
				          box[i].tx,
				          box[i].ty,
				          box[i].tz,
				          box[i].type);


fclose(fp);
strcpy(current_track,filename);
}

int grid_load(char *filename)
{
FILE *fp;
int i,j;
int c;
char name[RAYDIUM_MAX_NAME_LEN];

fp=raydium_file_fopen(filename,"rt");
if(!fp)
    {
    raydium_log("cannot open '%s' for reading",filename);
    return 0;
    }

grid_init_all();
box_init_all();
data_init();

i=j=0;

while(1)
{
c=fgetc(fp);
fgetc(fp);

if(c==EOF) break;

if(c=='g')
{
    fscanf(fp,"%f %f %f %i %i %s\n",&grid[i].x,
				      &grid[i].y,
				      &grid[i].z,
				      &grid[i].rot,
				      &grid[i].flags,
				      name);
    grid[i].state=1;
    grid[i].obj=raydium_object_find_load(name);
    i++;
}

if(c=='b')
{
    fscanf(fp,"%f %f %f %f %f %f %i\n",&box[j].x,
				       &box[j].y,
				       &box[j].z,
				       &box[j].tx,
				       &box[j].ty,
				       &box[j].tz,
				       &box[j].type);
    box[j].state=1;
    j++;
}

if(c=='d')
{
    fgets(tdata,4000,fp);
    if(tdata[strlen(tdata)-1]=='\n')
	tdata[strlen(tdata)-1]=0;
}

    if(i==MAX_ELEMS || j==MAX_ELEMS)
	{
	raydium_log("MAX_ELEMS reached, loading not completed");
	break;
	}
}
raydium_log("%s: %i grid elements loaded, %i box(es)",filename,i,j);
fclose(fp);
grid_generate_obj();
strcpy(current_track,filename);
px=0;
py=0;
pz=0;
return 1;
}

float xcos(int i)
{
return( (float)cos(i*PI/180) );
}

float xsin(int i)
{
return( (float)sin(i*PI/180) );
}


void rotatez(GLfloat angle, GLint obj)
{
int i;
GLfloat x,y;

for(i=raydium_object_start[obj];i<raydium_object_end[obj];i++)
 {
 x=raydium_vertex_x[i]-0.5;
 y=raydium_vertex_y[i]-0.5;

 raydium_vertex_x[i]=x*raydium_trigo_cos(angle) - y*raydium_trigo_sin(angle);
 raydium_vertex_y[i]=x*raydium_trigo_sin(angle) + y*raydium_trigo_cos(angle);
 raydium_vertex_x[i]+=0.5;
 raydium_vertex_y[i]+=0.5;
 }
}


int raydium_object_new_empty(char *name)
{
raydium_object_start[raydium_object_index]=raydium_vertex_index;
raydium_object_end[raydium_object_index]=raydium_vertex_index;
strcpy(raydium_object_name[raydium_object_index],name);
return(raydium_object_index++);
}

void grid_generate_obj(void)
{
int obj;
int i,j;

obj=raydium_object_find("mania.tri");
raydium_vertex_index=raydium_object_start[obj];

for(i=0;i<MAX_ELEMS;i++)
    if(grid[i].state)
    {
    rotatez(grid[i].rot,grid[i].obj);
	for(j=raydium_object_start[grid[i].obj];j<raydium_object_end[grid[i].obj];j++)
	{
	raydium_vertex_x[raydium_vertex_index]=raydium_vertex_x[j]+grid[i].x;
	raydium_vertex_y[raydium_vertex_index]=raydium_vertex_y[j]+grid[i].y;
	raydium_vertex_z[raydium_vertex_index]=raydium_vertex_z[j]+grid[i].z;
	raydium_vertex_texture_u[raydium_vertex_index]=raydium_vertex_texture_u[j];
	raydium_vertex_texture_v[raydium_vertex_index]=raydium_vertex_texture_v[j];
	raydium_vertex_texture[raydium_vertex_index]=raydium_vertex_texture[j];
	raydium_vertex_texture_multi[raydium_vertex_index]=raydium_vertex_texture_multi[j];
	raydium_vertex_texture_multi_u[raydium_vertex_index]=raydium_vertex_texture_multi_u[j];
	raydium_vertex_texture_multi_v[raydium_vertex_index]=raydium_vertex_texture_multi_v[j];
	raydium_vertex_index++;
	}
    rotatez(-grid[i].rot,grid[i].obj);
    }
raydium_object_end[obj]=raydium_vertex_index;
}


void data_init(void)
{
sprintf(tdata,"no name;unknown;0;0");
}

void draw_axes(void)
{
GLfloat size=modl_zoom/3;
GLfloat font=modl_zoom/30;

glBegin(GL_LINES);

glVertex3f(0,0,0);
glVertex3f(size,0,0);
glVertex3f(size+font,-font,0);
glVertex3f(size+font+font,font,0);
glVertex3f(size+font,font,0);
glVertex3f(size+font+font,-font,0);

glVertex3f(0,0,0);
glVertex3f(0,size,0);
glVertex3f(0,size+font+(font/2),0);
glVertex3f(0,size+font,0);
glVertex3f(0,size+font+(font/2),0);
glVertex3f(-font/2,size+font+font+(font/2),0);
glVertex3f(0,size+font+(font/2),0);
glVertex3f(font/2,size+font+font+(font/2),0);

glVertex3f(0,0,0);
glVertex3f(0,0,size);
glVertex3f(0,font,size+font-(font/2));
glVertex3f(0,font,size+font+(font/2));
glVertex3f(0,-font,size+font-(font/2));
glVertex3f(0,-font,size+font+(font/2));
glVertex3f(0,font,size+font-(font/2));
glVertex3f(0,-font,size+font+(font/2));

glEnd();
}


void draw_grid(void)
{
GLfloat x;
GLfloat y;
GLfloat z;

glBegin(GL_POINTS);
for(x=-modl_zoom;x<=modl_zoom;x++)
for(y=-modl_zoom;y<=modl_zoom;y++)
for(z=-modl_zoom;z<=modl_zoom;z++)
 {
 glColor3f(0.5,0.5,0.5);
 glVertex3f(x+px,y+py,z);
 }

glColor3f(0.7,0.7,0.7);
glEnd();
}

void draw_box(float x,float y,float z,float ox, float oy, float oz, float tx,float ty,float tz,int type)
{
float lx,ly,lz;
char dir='*';


lx=tx*0.5;
ly=ty*0.5;
lz=tz*0.5;
x+=ox;
y+=oy;
z+=oz;


 switch(type)
  {
  case TYPE_CHECKPOINT:
	    glColor3f(0,0,1);
	    break;

  case TYPE_START_E:
	    dir='e';
	    glColor3f(0,1,0);
	    break;
  case TYPE_START_W:
	    dir='w';
	    glColor3f(0,1,0);
	    break;
  case TYPE_START_N:
	    dir='n';
	    glColor3f(0,1,0);
	    break;
  case TYPE_START_S:
	    dir='s';
	    glColor3f(0,1,0);
	    break;

  case TYPE_END:
	    glColor3f(1,0,0);
	    break;
  case TYPE_LOOP:
	    glColor3f(1,1,1);
	    break;

  case TYPE_TURBO_E:
	    dir='e';
	    glColor3f(1,1,0);
	    break;
  case TYPE_TURBO_W:
	    dir='w';
	    glColor3f(1,1,0);
	    break;
  case TYPE_TURBO_N:
	    dir='n';
	    glColor3f(1,1,0);
	    break;
  case TYPE_TURBO_S:
	    dir='s';
	    glColor3f(1,1,0);
	    break;

  } // end switch

//glColor3f(1,1,1);
glBegin(GL_LINES);
glVertex3f (x,y,z);
switch(dir)
  {
  case 'e':
    glVertex3f (x+1,y,z);
    break;
  case 'w':
    glVertex3f (x-1,y,z);
    break;
  case 'n':
    glVertex3f (x,y+1,z);
    break;
  case 's':
    glVertex3f (x,y-1,z);
    break;
  }
glEnd();


glBegin(GL_TRIANGLE_STRIP);
glNormal3f (-1,0,0);
glVertex3f (-lx+x,-ly+y,-lz+z);
glVertex3f (-lx+x,-ly+y,lz+z);
glVertex3f (-lx+x,ly+y,-lz+z);
glVertex3f (-lx+x,ly+y,lz+z);
glNormal3f (0,1,0);
glVertex3f (lx+x,ly+y,-lz+z);
glVertex3f (lx+x,ly+y,lz+z);
glNormal3f (1,0,0);
glVertex3f (lx+x,-ly+y,-lz+z);
glVertex3f (lx+x,-ly+y,lz+z);
glNormal3f (0,-1,0);
glVertex3f (-lx+x,-ly+y,-lz+z);
glVertex3f (-lx+x,-ly+y,lz+z);
glEnd();

// top face
glBegin (GL_TRIANGLE_FAN);
glNormal3f (0,0,1);
glVertex3f (-lx+x,-ly+y,lz+z);
glVertex3f (lx+x,-ly+y,lz+z);
glVertex3f (lx+x,ly+y,lz+z);
glVertex3f (-lx+x,ly+y,lz+z);
glEnd();

// bottom face
glBegin (GL_TRIANGLE_FAN);
glNormal3f (0,0,-1);
glVertex3f (-lx+x,-ly+y,-lz+z);
glVertex3f (-lx+x,ly+y,-lz+z);
glVertex3f (lx+x,ly+y,-lz+z);
glVertex3f (lx+x,-ly+y,-lz+z);
glEnd();

}

void draw_selection(void)
{

if(pop_mode==POP_MODE_BOX)
 {
 draw_box(px,py,pz,
 boxpreset[curbox].x,
 boxpreset[curbox].y,
 boxpreset[curbox].z,
 boxpreset[curbox].tx,
 boxpreset[curbox].ty,
 boxpreset[curbox].tz,
 boxpreset[curbox].type);
 }

glBegin(GL_LINES);
glColor3f(0,1,0);
glVertex3f(px,py,pz);
glVertex3f(px,py+1,pz);
glVertex3f(px,py+1,pz);
glVertex3f(px+1,py+1,pz);
glVertex3f(px+1,py+1,pz);
glVertex3f(px+1,py,pz);
glVertex3f(px+1,py,pz);
glVertex3f(px,py,pz);
glEnd();

if(pop_mode==POP_MODE_ELEM)
 {
 glPushMatrix();
 glTranslatef(px,py,pz);
 rotatez(curangle,curobj);
 raydium_object_draw(curobj);
 rotatez(-curangle,curobj);
 glPopMatrix();
 }

}

void add(void)
{
GLint i;
raydium_log("pop !");

for(i=0;i<MAX_ELEMS;i++)
    if(!grid[i].state)
	{
	grid[i].state=1;
	grid[i].x=px;
	grid[i].y=py;
	grid[i].z=pz;
	grid[i].flags=0;
	grid[i].rot=curangle;
	grid[i].obj=curobj;	
	grid_generate_obj();
	return;
	}

raydium_log("MAX_ELEMS reached");
}

void add_box(void)
{
GLint i;
float x,y,z;
float tx,ty,tz;
int type;

x=px+boxpreset[curbox].x;
y=py+boxpreset[curbox].y;
z=pz+boxpreset[curbox].z;
tx=boxpreset[curbox].tx;
ty=boxpreset[curbox].ty;
tz=boxpreset[curbox].tz;
type=boxpreset[curbox].type;

raydium_log("pop the box !");

for(i=0;i<MAX_ELEMS;i++)
    if(!box[i].state)
	{
	box[i].state=1;
	box[i].x=x;
	box[i].y=y;
	box[i].z=z;
	box[i].tx=tx;
	box[i].ty=ty;
	box[i].tz=tz;
	box[i].type=type;
	return;
	}
raydium_log("MAX_ELEMS reached for boxes");
}

void del(void)
{
GLint i;

if(pop_mode==POP_MODE_ELEM)
for(i=0;i<MAX_ELEMS;i++)
    if( grid[i].state && 
        grid[i].x==px &&
        grid[i].y==py &&
        grid[i].z==pz )
	    grid[i].state=0;

if(pop_mode==POP_MODE_BOX)
for(i=0;i<MAX_ELEMS;i++)
    if( box[i].state   && 
        box[i].x>=px   &&
        box[i].x<=px+1 &&
        box[i].y>=py   &&
        box[i].y<=py+1 &&
        box[i].z>=pz   &&
        box[i].z<=pz+1 )
	    box[i].state=0;
	    
grid_generate_obj();
}


void mouse_n_keys_event(void)
{
static GLfloat rotx,roty;
int window;

if(raydium_key_last==1027) gui_exit();

window=raydium_gui_window_find("menu");

if(raydium_gui_widget_find("btnMenu",window)>=0)
{
 if(raydium_key_last==1045)  modl_zoom--;
 if(raydium_key_last==1043)  modl_zoom++;
 if(raydium_key_last==1100)  del();
 if(raydium_key_last==1009)  { curangle+=90; raydium_key_last=0; }
 if(raydium_key_last==1032 && pop_mode==POP_MODE_ELEM)  add();
 if(raydium_key_last==1032 && pop_mode==POP_MODE_BOX)  add_box();
// if(raydium_key_last==2) export_all();

 if(raydium_key_last==101) py++;
 if(raydium_key_last==103) py--;
 if(raydium_key_last==100) px--;
 if(raydium_key_last==102) px++;

 if(raydium_key_last==105 && pop_mode==POP_MODE_ELEM) curobj--;
 if(raydium_key_last==104 && pop_mode==POP_MODE_ELEM) curobj++;
 if(raydium_key_last==105 && pop_mode==POP_MODE_BOX) curbox--;
 if(raydium_key_last==104 && pop_mode==POP_MODE_BOX) curbox++;
 if(raydium_key_last==106) pz+=0.25;
 if(raydium_key_last==107) pz-=0.25;

 if(raydium_key_last==1013 && pop_mode==POP_MODE_ELEM) pop_mode=POP_MODE_BOX;
 else
 if(raydium_key_last==1013 && pop_mode==POP_MODE_BOX) pop_mode=POP_MODE_ELEM;

 if(raydium_key_last==1109) build_gui_menu(NULL);

 if(raydium_key_last==1084) btnTest(NULL);

 if(curobj<0) curobj=0;
 if(curbox<0) curbox=0;
 if((curobj+1)>=raydium_object_index) curobj=raydium_object_index-2;
 if(curbox>=n_boxpresets) curbox=n_boxpresets-1;

 curangle%=360;
 if(modl_zoom<1) modl_zoom=1;

 if(!view_glue)
    rotx=roty=0;

 if(raydium_mouse_button[1])
 {
  view_glue=1;
  rotx=((float)raydium_mouse_x-((float)raydium_window_tx/(float)2)) * ((float)360/(float)raydium_window_tx);
  roty=((float)raydium_mouse_y-((float)raydium_window_ty/(float)2)) * ((float)360/(float)raydium_window_ty);
 }

 if(raydium_mouse_button[0]) 
 {
  view_glue=0;
  rotx=((float)raydium_mouse_x-((float)raydium_window_tx/(float)2)) * ((float)360/(float)raydium_window_tx);
  roty=((float)raydium_mouse_y-((float)raydium_window_ty/(float)2)) * ((float)360/(float)raydium_window_ty);
 }

  glRotatef(roty,1,0,0);
  glRotatef(rotx,0,1,0);
} // end if "acess menu visible"

 glTranslatef(-px,-py,-pz);
}


void update_vars(void)
{
raydium_projection=RAYDIUM_PROJECTION_ORTHO;
raydium_projection_near=-modl_zoom;
raydium_projection_far=modl_zoom;
raydium_projection_left=-modl_zoom;
raydium_projection_right=modl_zoom;
raydium_projection_bottom=-modl_zoom;
raydium_projection_top=modl_zoom;
raydium_window_resize_callback(raydium_window_tx,raydium_window_ty);
}



void display(void)
{
int i;
raydium_clear_frame();

//glLoadIdentity();
update_vars();

mouse_n_keys_event();

glDisable(GL_TEXTURE_2D);

if(!view_glue)
    {
    draw_grid();
    draw_axes();
    }

for(i=0;i<MAX_ELEMS;i++)
  if(box[i].state)
    {
    // draw boxes here
    draw_box(0,0,0,
             box[i].x,box[i].y,box[i].z,
             box[i].tx,box[i].ty,box[i].tz,
	     box[i].type);
    }

draw_selection();
glColor3f(1.0,1.0,1.0);
glEnable(GL_TEXTURE_2D);

//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
raydium_object_draw_name("mania.tri");

raydium_rendering_finish();
raydium_light_disable();
}

void load_all_in(char *name)
{
char trifile[255]; // eeeerk :)
FILE *fp;
fp=raydium_file_fopen(name,"rt");
if(!fp)
    {
    printf("Need a %s config file, with all available tri files..\n",name);
    exit(1);
    }

while(fgets(trifile,255,fp)!=NULL)
    {
//    if(!strlen(trifile)) continue;
//    trifile[strlen(trifile)-1]=0;
    raydium_parser_trim(trifile);
    if(!strlen(trifile)) continue;
    if(trifile[0]=='#') continue;
    if(trifile[0]=='*')
	{
	boxpreset[n_boxpresets].state=1;
	sscanf(trifile+2,"%f %f %f %f %f %f %i",
				       &boxpreset[n_boxpresets].x,
				       &boxpreset[n_boxpresets].y,
				       &boxpreset[n_boxpresets].z,
				       &boxpreset[n_boxpresets].tx,
				       &boxpreset[n_boxpresets].ty,
				       &boxpreset[n_boxpresets].tz,
				       &boxpreset[n_boxpresets].type);
	n_boxpresets++;
	}
    else raydium_object_load(trifile);
    }
fclose(fp);
}

char file_exists(char *name)
{
FILE *fp;
fp=fopen(name,"r");
if(fp) { fclose(fp); return 1; }
return 0;
}


int main(int argc, char **argv)
{
char window[256];

sprintf(window,"%s - %s",title,version);

data_init();
current_track[0]=0;

raydium_init_args_name(argc,argv,"mania_drive");

raydium_window_create(700,700,RAYDIUM_RENDERING_WINDOW,window);

raydium_background_color_change(0,0,0,1);
raydium_rendering_displaylists_disable();
raydium_texture_filter=RAYDIUM_TEXTURE_FILTER_TRILINEAR;

// place here all available objets:
load_all_in("mania.cfg");
raydium_object_new_empty("mania.tri");
grid_init_all();
box_init_all();

// stop here (must be LAST object)
raydium_parser_db_get("Mania2-CurrentTrack",current_track,"");

if(file_exists(current_track))
    grid_load(current_track);

raydium_mouse_show();
raydium_gui_theme_load("theme-raydium2.gui");
raydium_gui_show();
build_gui_access(NULL);
//build_gui_help(NULL);

raydium_callback(&display);

return(0);
}
