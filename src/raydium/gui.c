/*
    Raydium - CQFD Corp.
    http://raydium.org/
    License: GPL - GNU General Public License, see "gpl.txt" file.
*/

#ifndef DONT_INCLUDE_HEADERS
#include "index.h"
#else
#include "headers/gui.h"
#endif 

#include "gui.h"


// Disclaimer ;)
// Some part of this file are quite ugly, sorry about that.
// (mainly for combos management [strings and double drawing])
// There also a lot of duplicated code.
// Needs some internal work, but API is quite stable, now.

void raydium_gui_window_init(int window)
{
raydium_gui_Window *w;
int i;

w=&raydium_gui_windows[window];

w->state=0;
w->id=window;
w->name[0]=0;
w->pos[0]=w->pos[1]=0;
w->size[0]=w->size[1]=0;
w->focused_widget=-1;
w->old_focused=-1;
w->OnDelete=NULL;

for(i=0;i<RAYDIUM_GUI_MAX_OBJECTS;i++)
    {
    w->widgets[i].id=i;
    w->widgets[i].name[0]=0;
    w->widgets[i].state=0;
    w->widgets[i].window=window;
    w->widgets[i].pos[0]=w->widgets[i].pos[1]=0;
    w->widgets[i].size[0]=w->widgets[i].size[1]=0;
    w->widgets[i].font_size=20;
    if(w->widgets[i].widget!=NULL)
	{
	free(w->widgets[i].widget);
	w->widgets[i].widget=NULL;
	}
    }
}


// "One time init"
void raydium_gui_init(void)
{
int i,j;

raydium_gui_visible=0;
raydium_gui_window_focused=-1;
raydium_gui_oldstate=0;
raydium_gui_button_clicked_id=-1;
raydium_gui_widget_sizes(15,5,16);
raydium_gui_AfterGuiDrawCallback=NULL;
raydium_gui_theme_init();

// mark all windows's objetcs (widgets to NULL) ...
for(i=0;i<RAYDIUM_GUI_MAX_WINDOWS;i++)
    for(j=0;j<RAYDIUM_GUI_MAX_OBJECTS;j++)
	{
	raydium_gui_windows[i].widgets[j].state=0;
	raydium_gui_windows[i].widgets[j].widget=NULL;
	}

// ... and then, get back to a regular init :
for(i=0;i<RAYDIUM_GUI_MAX_WINDOWS;i++)
    raydium_gui_window_init(i);

raydium_log("gui: OK");
}


void raydium_gui_theme_init(void)
{
raydium_gui_theme_current.loaded=0;
raydium_gui_theme_current.filename[0]=0;
raydium_gui_theme_current.texture=0;
raydium_gui_theme_current.texture_size[0]=1;
raydium_gui_theme_current.texture_size[1]=1;
memset(raydium_gui_theme_current.background_uv,0,sizeof(GLfloat)*4);
strcpy(raydium_gui_theme_current.font,"font2.tga");
}

int raydium_gui_theme_load(char *filename)
{
FILE *fp;
int ret;
char var[RAYDIUM_MAX_NAME_LEN];
char val_s[RAYDIUM_MAX_NAME_LEN];
GLfloat val_f[4];
int size;
char done;

fp=raydium_file_fopen(filename,"rt");
if(!fp)
 {
 raydium_log("gui: ERROR: Cannot open %s theme file",filename);
 return 0;
 }

raydium_gui_theme_init();
strcpy(raydium_gui_theme_current.filename,filename);

while( (ret=raydium_parser_read(var,val_s,val_f,&size,fp))!=RAYDIUM_PARSER_TYPE_EOF)
    {
    done=0;

    if(!strcasecmp(var,"texture"))
	{
	int t;
	if(ret!=RAYDIUM_PARSER_TYPE_STRING)
	    {
	    raydium_log("gui: parser: texture: wrong type");
	    continue;
	    }
	
	t=raydium_texture_find_by_name(val_s);
	if(!t)
	    {
	    fclose(fp);
	    return 0;
	    }
	raydium_gui_theme_current.texture=t;
	done=1;
	}

    if(!strcasecmp(var,"font"))
	{
	int t;
	if(ret!=RAYDIUM_PARSER_TYPE_STRING)
	    {
	    raydium_log("gui: parser: font: wrong type");
	    continue;
	    }
	
	t=raydium_texture_find_by_name(val_s);
	if(t)
	    strcpy(raydium_gui_theme_current.font,val_s);
	else
	    raydium_log("gui: parser: load: cannt load font '%s'",val_s);
	done=1;
	}

    if(!strcasecmp(var,"texture_size"))
	{
	if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=2)
	    {
	    raydium_log("gui: parser: texture_size: wrong type");
	    continue;
	    }
	// cast floats to ints
	raydium_gui_theme_current.texture_size[0]=(int)val_f[0];
	raydium_gui_theme_current.texture_size[1]=(int)val_f[1];
	done=1;
	}

    if(!strcasecmp(var,"background"))
	{
	if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=4)
	    {
	    raydium_log("gui: parser: background: wrong type");
	    continue;
	    }
	memcpy(raydium_gui_theme_current.background_uv,val_f,sizeof(GLfloat)*4);
	done=1;
	}
    /*
    // We're only parsing "general" properties: no need to reports unknown ones
    if(!done)
	raydium_log("gui: parser: invalid or unsupported option '%s' (%s)",var,filename);
    */    
    }
fclose(fp);
raydium_gui_theme_current.loaded=1;
return 1;
}


signed char raydium_gui_window_isvalid(int i)
{
if(i>=0 && i<RAYDIUM_GUI_MAX_WINDOWS &&
   raydium_gui_windows[i].state)
    return 1;
return 0;
}

int raydium_gui_window_find(char *name)
{
int i;

for(i=0;i<RAYDIUM_GUI_MAX_WINDOWS;i++)
    if(!strcmp(name,raydium_gui_windows[i].name) && raydium_gui_window_isvalid(i))
     return i;
return -1;
}


signed char raydium_gui_widget_isvalid(int i, int window)
{
if(!raydium_gui_window_isvalid(window))
    return 0;
    
if(i>=0 && i<RAYDIUM_GUI_MAX_OBJECTS &&
   raydium_gui_windows[window].widgets[i].state)
    return 1;
return 0;
}


int raydium_gui_widget_find(char *name, int window)
{
int i;

if(!raydium_gui_window_isvalid(window))
    return -1;

for(i=0;i<RAYDIUM_GUI_MAX_OBJECTS;i++)
    if(!strcmp(name,raydium_gui_windows[window].widgets[i].name) && raydium_gui_widget_isvalid(i,window))
     return i;
return -1;
}


void raydium_gui_widget_next(void)
{
int i;
raydium_gui_Window *w;

if(!raydium_gui_window_isvalid(raydium_gui_window_focused))
    return;

w=&raydium_gui_windows[raydium_gui_window_focused];

for(i=w->focused_widget+1;i<RAYDIUM_GUI_MAX_OBJECTS;i++)
    if(raydium_gui_widget_isvalid(i,raydium_gui_window_focused) && w->widgets[i].type!=RAYDIUM_GUI_LABEL)
	{
	w->focused_widget=i;
	return;
	}

// Not found : rewind
for(i=0;i<RAYDIUM_GUI_MAX_OBJECTS;i++)
    if(raydium_gui_widget_isvalid(i,raydium_gui_window_focused) && w->widgets[i].type!=RAYDIUM_GUI_LABEL)
	{
	w->focused_widget=i;
	return;
	}
}


void raydium_gui_widget_focus(int widget, int window)
{
if(!raydium_gui_window_isvalid(raydium_gui_window_focused))
    {
    raydium_log("gui: error: cannot set focus: invalid window");
    return;
    }

if(raydium_gui_widget_isvalid(widget,raydium_gui_window_focused))
    {
    //printf("%i %i\n",widget,raydium_gui_window_focused);
    raydium_gui_windows[raydium_gui_window_focused].focused_widget=widget;
    }
else
    raydium_log("gui: error: cannot set focus: invalid widget");
}


void raydium_gui_widget_focus_name(char *widget, char *window)
{
int w;
w=raydium_gui_window_find(window);
raydium_gui_widget_focus(raydium_gui_widget_find(widget,w),w);
}



void raydium_gui_widget_draw_internal(GLfloat *uv, GLfloat *xy)
{
raydium_osd_start();
raydium_texture_current_set(raydium_gui_theme_current.texture);
raydium_rendering_internal_prepare_texture_render(raydium_gui_theme_current.texture);

glBegin(GL_QUADS);
glTexCoord2f(uv[0],uv[1]);
glVertex3f(xy[0],xy[3],0);
glTexCoord2f(uv[2],uv[1]);
glVertex3f(xy[2],xy[3],0);
glTexCoord2f(uv[2],uv[3]);
glVertex3f(xy[2],xy[1],0);
glTexCoord2f(uv[0],uv[3]);
glVertex3f(xy[0],xy[1],0);
glEnd();

raydium_osd_stop();
}

///////////////////////////
// final Widgets drawing //
///////////////////////////

void raydium_gui_button_draw(int w, int window)
{
GLfloat uv[4];
GLfloat xy[4];
GLfloat fxy[2];
GLfloat mxy[2];
GLfloat wfactor[2];
GLfloat *suv;
GLfloat tmp;
char style;
char substyle;
raydium_gui_Button *b;

if(!raydium_gui_window_isvalid(window))
    return;
if(!raydium_gui_widget_isvalid(w,window))
    return;

b=raydium_gui_windows[window].widgets[w].widget;

mxy[0]=    ((float)raydium_mouse_x/raydium_window_tx)*100;
mxy[1]=100-((float)raydium_mouse_y/raydium_window_ty)*100;


wfactor[0]=raydium_gui_windows[window].size[0]/100.;
wfactor[1]=raydium_gui_windows[window].size[1]/100.;

xy[0]=raydium_gui_windows[window].pos[0]+
     (raydium_gui_windows[window].widgets[w].pos[0]*wfactor[0]);
xy[1]=raydium_gui_windows[window].pos[1]+
     (raydium_gui_windows[window].widgets[w].pos[1]*wfactor[1]);
xy[2]=raydium_gui_windows[window].pos[0] +
     (raydium_gui_windows[window].widgets[w].pos[0]*wfactor[0]) +
      raydium_gui_windows[window].widgets[w].size[0];
xy[3]=raydium_gui_windows[window].pos[1] +
     (raydium_gui_windows[window].widgets[w].pos[1]*wfactor[1]) +
      raydium_gui_windows[window].widgets[w].size[1];


style=RAYDIUM_GUI_NORMAL;
substyle=RAYDIUM_GUI_NORMAL;

if(raydium_gui_windows[window].focused_widget==w)
    {
    style=RAYDIUM_GUI_FOCUS;
    substyle=RAYDIUM_GUI_FOCUS;
    }

if(raydium_gui_window_focused==window &&
  (mxy[0]>=xy[0] && mxy[1]>=xy[1] &&
   mxy[0]<=xy[2] && mxy[1]<=xy[3] ))
	style=RAYDIUM_GUI_HOVER;


// uv
switch(style)
    {
    case RAYDIUM_GUI_NORMAL:
	suv=b->uv_normal;
	break;
    case RAYDIUM_GUI_FOCUS:
	suv=b->uv_focus;
	break;
    case RAYDIUM_GUI_HOVER:
	suv=b->uv_hover;
	break;
    }

uv[0]=  suv[0]/raydium_gui_theme_current.texture_size[0];
uv[1]=1-suv[1]/raydium_gui_theme_current.texture_size[1];
uv[2]=  (suv[0] + suv[2]) / raydium_gui_theme_current.texture_size[0];
uv[3]=1-(suv[1] + suv[3]) / raydium_gui_theme_current.texture_size[1];

raydium_gui_widget_draw_internal(uv,xy);

if(strlen(b->caption)>0)
{
fxy[0]=xy[0]+(xy[2]-xy[0])/2;
fxy[1]=xy[1]+(xy[3]-xy[1])/2;
if(strlen(b->caption)>1)
    {
    tmp=raydium_gui_windows[window].widgets[w].font_size/RAYDIUM_OSD_FONT_SIZE_FACTOR;
    tmp=((strlen(b->caption)-1)*tmp)/2;
    }
else
    tmp=0;
raydium_osd_color_change(b->font_color[0],b->font_color[1],b->font_color[2]);
raydium_osd_printf(fxy[0]-tmp,fxy[1],
		   raydium_gui_windows[window].widgets[w].font_size,
		   0.5,raydium_gui_theme_current.font,"%s",b->caption);
} // end caption drawing

// Events management
// Here, I consider that HOVER (mouse) have priority over FOCUS for Enter key
if( raydium_gui_window_focused==window &&
    ((style==RAYDIUM_GUI_HOVER && raydium_mouse_click==1)  ||
     ((style==RAYDIUM_GUI_FOCUS || substyle==RAYDIUM_GUI_FOCUS) && raydium_key_last==1013)) )
    {
    void (*f)(raydium_gui_Object *);
    raydium_key_last=0;
    raydium_mouse_click=0;
    raydium_mouse_button[0]=0;
    raydium_gui_windows[window].focused_widget=w;
    f=b->OnClick;

    if(f)
	f(&raydium_gui_windows[window].widgets[w]);
    raydium_gui_button_clicked_id=window*1000+w;
    }
}

void raydium_gui_zone_draw(int w, int window)
{
raydium_gui_Zone *z;
GLfloat xy[4];
GLfloat mxy[2];
GLfloat wfactor[2];
char style;
GLfloat *col;

if(!raydium_gui_window_isvalid(window))
    return;
if(!raydium_gui_widget_isvalid(w,window))
    return;

z=raydium_gui_windows[window].widgets[w].widget;

mxy[0]=    ((float)raydium_mouse_x/raydium_window_tx)*100;
mxy[1]=100-((float)raydium_mouse_y/raydium_window_ty)*100;

wfactor[0]=raydium_gui_windows[window].size[0]/100.;
wfactor[1]=raydium_gui_windows[window].size[1]/100.;

xy[0]=raydium_gui_windows[window].pos[0]+
     (raydium_gui_windows[window].widgets[w].pos[0]*wfactor[0]);
xy[1]=raydium_gui_windows[window].pos[1]+
     (raydium_gui_windows[window].widgets[w].pos[1]*wfactor[1]);
xy[2]=raydium_gui_windows[window].pos[0] +
     (raydium_gui_windows[window].widgets[w].pos[0]*wfactor[0]) +
      raydium_gui_windows[window].widgets[w].size[0];
xy[3]=raydium_gui_windows[window].pos[1] +
     (raydium_gui_windows[window].widgets[w].pos[1]*wfactor[1]) +
      raydium_gui_windows[window].widgets[w].size[1];

//printf("%f %f\n",

style=RAYDIUM_GUI_NORMAL;

if(raydium_gui_windows[window].focused_widget==w)
    style=RAYDIUM_GUI_FOCUS;

if(raydium_gui_window_focused==window &&
  (mxy[0]>=xy[0] && mxy[1]>=xy[1] &&
   mxy[0]<=xy[2] && mxy[1]<=xy[3] ))
	style=RAYDIUM_GUI_HOVER;

// uv
switch(style)
    {
    case RAYDIUM_GUI_NORMAL:
	col=z->col_normal;
	break;
    case RAYDIUM_GUI_FOCUS:
	col=z->col_focus;
	break;
    case RAYDIUM_GUI_HOVER:
	col=z->col_hover;
	break;
    }

raydium_osd_start();
glDisable(GL_TEXTURE_2D);
glEnable(GL_BLEND);
glColor4fv(col);
glBegin(GL_QUADS);
glVertex3f(xy[0],xy[3],0);
glVertex3f(xy[2],xy[3],0);
glVertex3f(xy[2],xy[1],0);
glVertex3f(xy[0],xy[1],0);
glEnd();
glDisable(GL_BLEND);
glEnable(GL_TEXTURE_2D);
raydium_osd_stop();

if( raydium_gui_window_focused==window &&
    ((style==RAYDIUM_GUI_HOVER && raydium_mouse_click==1)  ||
     (style==RAYDIUM_GUI_FOCUS && raydium_key_last==1013)) )
    {
    void (*f)(raydium_gui_Object *);
    raydium_key_last=0;
    raydium_mouse_click=0;
    raydium_mouse_button[0]=0;
    raydium_gui_windows[window].focused_widget=w;
    f=z->OnClick;

    if(f)
	f(&raydium_gui_windows[window].widgets[w]);
    raydium_gui_button_clicked_id=window*1000+w;
    }
}


void raydium_gui_track_draw(int w, int window)
{
GLfloat uv[4];
GLfloat rxy[4];
GLfloat cxy[4];
GLfloat cdec[2];
GLfloat mxy[2];
GLfloat wfactor[2];
GLfloat rcfactor[2];
GLfloat *ruv;
GLfloat *cuv;
char style;
raydium_gui_Track *t;

if(!raydium_gui_window_isvalid(window))
    return;
if(!raydium_gui_widget_isvalid(w,window))
    return;

t=raydium_gui_windows[window].widgets[w].widget;

mxy[0]=    ((float)raydium_mouse_x/raydium_window_tx)*100;
mxy[1]=100-((float)raydium_mouse_y/raydium_window_ty)*100;


wfactor[0]=raydium_gui_windows[window].size[0]/100.;
wfactor[1]=raydium_gui_windows[window].size[1]/100.;


style=RAYDIUM_GUI_NORMAL;

if(raydium_gui_windows[window].focused_widget==w)
    style=RAYDIUM_GUI_FOCUS;

// rule  ...
rxy[0]=raydium_gui_windows[window].pos[0]+
      (raydium_gui_windows[window].widgets[w].pos[0]*wfactor[0]);
rxy[1]=raydium_gui_windows[window].pos[1]+
      (raydium_gui_windows[window].widgets[w].pos[1]*wfactor[1]);
rxy[2]=raydium_gui_windows[window].pos[0] +
      (raydium_gui_windows[window].widgets[w].pos[0]*wfactor[0]) +
       raydium_gui_windows[window].widgets[w].size[0];
rxy[3]=raydium_gui_windows[window].pos[1] +
      (raydium_gui_windows[window].widgets[w].pos[1]*wfactor[1]) +
       raydium_gui_windows[window].widgets[w].size[1];
// uv
ruv=t->uv_rule;

uv[0]=  ruv[0]/raydium_gui_theme_current.texture_size[0];
uv[1]=1-ruv[1]/raydium_gui_theme_current.texture_size[1];
uv[2]=  (ruv[0] + ruv[2]) / raydium_gui_theme_current.texture_size[0];
uv[3]=1-(ruv[1] + ruv[3]) / raydium_gui_theme_current.texture_size[1];

raydium_gui_widget_draw_internal(uv,rxy);
// ... end rule

// cursor  ...
// uv
switch(style)
    {
    case RAYDIUM_GUI_NORMAL:
	cuv=t->uv_cursor_normal;
	break;
    case RAYDIUM_GUI_FOCUS:
	cuv=t->uv_cursor_focus;
	break;
    }

uv[0]=  cuv[0]/raydium_gui_theme_current.texture_size[0];
uv[1]=1-cuv[1]/raydium_gui_theme_current.texture_size[1];
uv[2]=  (cuv[0] + cuv[2]) / raydium_gui_theme_current.texture_size[0];
uv[3]=1-(cuv[1] + cuv[3]) / raydium_gui_theme_current.texture_size[1];

rcfactor[0]=cuv[2]/ruv[2];
rcfactor[1]=cuv[3]/ruv[3];

//raydium_log("%f %f",cuv[4],ruv[4]);

cxy[0]=rxy[0];
cxy[1]=rxy[1];
cxy[2]=raydium_gui_windows[window].pos[0] +
      (raydium_gui_windows[window].widgets[w].pos[0]*wfactor[0]) +
      (raydium_gui_windows[window].widgets[w].size[0]*rcfactor[0]);
cxy[3]=raydium_gui_windows[window].pos[1] +
      (raydium_gui_windows[window].widgets[w].pos[1]*wfactor[1]) +
      (raydium_gui_windows[window].widgets[w].size[1]*rcfactor[1]);

cdec[0]=(rxy[2]-rxy[0]) * ((float)(t->current - t->min)/(t->max - t->min));
cdec[0]-=(cxy[2]-cxy[0])/2;
cdec[1]=(cxy[3]-cxy[1])/2 - (rxy[3]-rxy[1])/2;

cxy[0]+=cdec[0];
cxy[2]+=cdec[0];
cxy[1]-=cdec[1];
cxy[3]-=cdec[1];

raydium_gui_widget_draw_internal(uv,cxy);
// ... end cursor


// Events management

// hover
if(raydium_gui_window_focused==window &&
  (mxy[0]>=rxy[0] && mxy[1]>=rxy[1] &&
   mxy[0]<=rxy[2] && mxy[1]<=rxy[3] ) &&
   raydium_mouse_button[0])
    {
    GLfloat fact;
    raydium_gui_windows[window].focused_widget=w;
    fact=(mxy[0]-rxy[0])/(rxy[2]-rxy[0]);
    
    t->current= t->min + (fact*(t->max - t->min));
    }

if(raydium_gui_window_focused==window && style==RAYDIUM_GUI_FOCUS && raydium_key_last==100)
    {
    t->current--;
    raydium_key_last=0;
    }
if(raydium_gui_window_focused==window && style==RAYDIUM_GUI_FOCUS && raydium_key_last==102)
    {
    t->current++;
    raydium_key_last=0;
    }

if(t->current<t->min)
    t->current=t->min;
if(t->current>t->max)
    t->current=t->max;
}


void raydium_gui_label_draw(int w, int window)
{
GLfloat xy[2];
GLfloat wfactor[2];
GLfloat tmp;
raydium_gui_Label *l;

if(!raydium_gui_window_isvalid(window))
    return;
if(!raydium_gui_widget_isvalid(w,window))
    return;

l=raydium_gui_windows[window].widgets[w].widget;

wfactor[0]=raydium_gui_windows[window].size[0]/100.;
wfactor[1]=raydium_gui_windows[window].size[1]/100.;

xy[0]=raydium_gui_windows[window].pos[0]+
     (raydium_gui_windows[window].widgets[w].pos[0]*wfactor[0]);
xy[1]=raydium_gui_windows[window].pos[1]+
     (raydium_gui_windows[window].widgets[w].pos[1]*wfactor[1]);


if(strlen(l->caption)>0)
{
if(strlen(l->caption)>1)
    {
    tmp=raydium_gui_windows[window].widgets[w].font_size/RAYDIUM_OSD_FONT_SIZE_FACTOR;
    tmp=((strlen(l->caption)-1)*tmp)/2;
    }
else
    tmp=0;
raydium_osd_color_change(l->font_color[0],l->font_color[1],l->font_color[2]);
raydium_osd_printf(xy[0]-tmp,xy[1],
		   raydium_gui_windows[window].widgets[w].font_size,
		   0.5,raydium_gui_theme_current.font,"%s",l->caption);
} // end caption drawing
}


void raydium_gui_edit_draw(int w, int window)
{
GLfloat uv[4];
GLfloat xy[4];
GLfloat mxy[2];
GLfloat wfactor[2];
GLfloat *suv;
char style;
int len;
raydium_gui_Edit *e;
// ugly temp vars
GLfloat fxy[2];
GLfloat tmp,tmp2;
char zone[RAYDIUM_MAX_NAME_LEN];


if(!raydium_gui_window_isvalid(window))
    return;
if(!raydium_gui_widget_isvalid(w,window))
    return;

e=raydium_gui_windows[window].widgets[w].widget;

mxy[0]=    ((float)raydium_mouse_x/raydium_window_tx)*100;
mxy[1]=100-((float)raydium_mouse_y/raydium_window_ty)*100;


wfactor[0]=raydium_gui_windows[window].size[0]/100.;
wfactor[1]=raydium_gui_windows[window].size[1]/100.;

xy[0]=raydium_gui_windows[window].pos[0]+
     (raydium_gui_windows[window].widgets[w].pos[0]*wfactor[0]);
xy[1]=raydium_gui_windows[window].pos[1]+
     (raydium_gui_windows[window].widgets[w].pos[1]*wfactor[1]);
xy[2]=raydium_gui_windows[window].pos[0] +
     (raydium_gui_windows[window].widgets[w].pos[0]*wfactor[0]) +
      raydium_gui_windows[window].widgets[w].size[0];
xy[3]=raydium_gui_windows[window].pos[1] +
     (raydium_gui_windows[window].widgets[w].pos[1]*wfactor[1]) +
      raydium_gui_windows[window].widgets[w].size[1];


style=RAYDIUM_GUI_NORMAL;

if(raydium_gui_windows[window].focused_widget==w)
    style=RAYDIUM_GUI_FOCUS;

// uv
switch(style)
    {
    case RAYDIUM_GUI_NORMAL:
	suv=e->uv_normal;
	break;
    case RAYDIUM_GUI_FOCUS:
	suv=e->uv_focus;
	break;
    }

uv[0]=  suv[0]/raydium_gui_theme_current.texture_size[0];
uv[1]=1-suv[1]/raydium_gui_theme_current.texture_size[1];
uv[2]=  (suv[0] + suv[2]) / raydium_gui_theme_current.texture_size[0];
uv[3]=1-(suv[1] + suv[3]) / raydium_gui_theme_current.texture_size[1];

raydium_gui_widget_draw_internal(uv,xy);

len=strlen(e->text);

fxy[0]=xy[0];
fxy[1]=xy[1]+(xy[3]-xy[1])/2;
tmp=raydium_gui_windows[window].widgets[w].font_size/RAYDIUM_OSD_FONT_SIZE_FACTOR;
tmp2=(xy[2]-xy[0])-tmp;
tmp2/=tmp;

if(tmp2>250)
    {
    raydium_log("GUI: Error: edit zone is too long");
    return;
    }

// asserts
if(e->cursor>len)
    e->cursor=len;
if(e->cursor<0)
    e->cursor=0;

if(e->cursor-e->offset>tmp2)
    e->offset=e->cursor-tmp2;
if(e->cursor<e->offset)
    e->offset=e->cursor;
    
strncpy(zone,e->text+e->offset,250);
zone[(int)tmp2+1]=0;

raydium_osd_color_change(e->font_color[0],e->font_color[1],e->font_color[2]);
raydium_osd_printf(fxy[0]+tmp,fxy[1],
		   raydium_gui_windows[window].widgets[w].font_size,
		   0.5,raydium_gui_theme_current.font,"%s",zone);

if(style==RAYDIUM_GUI_FOCUS)
raydium_osd_printf(fxy[0]+tmp+(tmp*e->cursor)-(tmp/2)-(e->offset*tmp),fxy[1],
		   raydium_gui_windows[window].widgets[w].font_size,
		   0.5,raydium_gui_theme_current.font,"|");
// end of "printf"

// Events management

if(raydium_gui_window_focused==window &&
  (mxy[0]>=xy[0] && mxy[1]>=xy[1] &&
   mxy[0]<=xy[2] && mxy[1]<=xy[3] ) &&
   raydium_mouse_click==1)
    {
    GLfloat fact;
    raydium_mouse_click=0;
    raydium_mouse_button[0]=0;
    raydium_gui_windows[window].focused_widget=w;
    fact=(mxy[0]-xy[0])/(xy[2]-xy[0]);
    e->cursor=(tmp2*fact)+(tmp/3.f)+e->offset; // need some work
    }

if(raydium_gui_window_focused==window && style==RAYDIUM_GUI_FOCUS && raydium_key_last==100)
    {
    e->cursor--;
    raydium_key_last=0;
    }
if(raydium_gui_window_focused==window && style==RAYDIUM_GUI_FOCUS && raydium_key_last==102)
    {
    e->cursor++;
    raydium_key_last=0;
    }
if(raydium_gui_window_focused==window && style==RAYDIUM_GUI_FOCUS && (raydium_key_last>=1032 && raydium_key_last<1127) &&
		       len<(RAYDIUM_GUI_DATASIZE-1))
    {
    memmove(e->text+e->cursor+1,e->text+e->cursor,strlen(e->text+e->cursor)+1);
    e->text[e->cursor]=raydium_key_last-1000;
    e->cursor++;
    raydium_key_last=0;
    }

if(raydium_gui_window_focused==window && style==RAYDIUM_GUI_FOCUS && raydium_key_last==1008 && len>0 && e->cursor>0)
    {
    memmove(e->text+e->cursor-1,e->text+e->cursor,strlen(e->text+e->cursor)+1);
    e->cursor--;
    raydium_key_last=0;
    }

if(raydium_gui_window_focused==window && style==RAYDIUM_GUI_FOCUS && raydium_key_last==1127 && e->cursor<len)
    {
    memmove(e->text+e->cursor,e->text+e->cursor+1,strlen(e->text+e->cursor)+1);
    raydium_key_last=0;
    }


if(raydium_gui_window_focused==window && style==RAYDIUM_GUI_FOCUS && raydium_key_last==106)
    {
    e->cursor=0;
    raydium_key_last=0;
    }
if(raydium_gui_window_focused==window && style==RAYDIUM_GUI_FOCUS && raydium_key_last==107)
    {
    e->cursor=len;
    raydium_key_last=0;
    }    
}


void raydium_gui_check_draw(int w, int window)
{
GLfloat uv[4];
GLfloat xy[4];
GLfloat fxy[2];
GLfloat mxy[2];
GLfloat wfactor[2];
GLfloat *suv;
GLfloat *scol;
char style;
raydium_gui_Check *c;

if(!raydium_gui_window_isvalid(window))
    return;
if(!raydium_gui_widget_isvalid(w,window))
    return;

c=raydium_gui_windows[window].widgets[w].widget;

mxy[0]=    ((float)raydium_mouse_x/raydium_window_tx)*100;
mxy[1]=100-((float)raydium_mouse_y/raydium_window_ty)*100;


wfactor[0]=raydium_gui_windows[window].size[0]/100.;
wfactor[1]=raydium_gui_windows[window].size[1]/100.;

xy[0]=raydium_gui_windows[window].pos[0]+
     (raydium_gui_windows[window].widgets[w].pos[0]*wfactor[0]);
xy[1]=raydium_gui_windows[window].pos[1]+
     (raydium_gui_windows[window].widgets[w].pos[1]*wfactor[1]);
xy[2]=raydium_gui_windows[window].pos[0] +
     (raydium_gui_windows[window].widgets[w].pos[0]*wfactor[0]) +
      raydium_gui_windows[window].widgets[w].size[0];
xy[3]=raydium_gui_windows[window].pos[1] +
     (raydium_gui_windows[window].widgets[w].pos[1]*wfactor[1]) +
      raydium_gui_windows[window].widgets[w].size[1];


style=RAYDIUM_GUI_NORMAL;

if(raydium_gui_windows[window].focused_widget==w)
    style=RAYDIUM_GUI_FOCUS;


if(!c->checked)
    suv=c->uv_normal;
else
    suv=c->uv_checked;

if(style==RAYDIUM_GUI_NORMAL)
    scol=c->font_color_normal;
else
    scol=c->font_color_focus;

uv[0]=  suv[0]/raydium_gui_theme_current.texture_size[0];
uv[1]=1-suv[1]/raydium_gui_theme_current.texture_size[1];
uv[2]=  (suv[0] + suv[2]) / raydium_gui_theme_current.texture_size[0];
uv[3]=1-(suv[1] + suv[3]) / raydium_gui_theme_current.texture_size[1];

raydium_gui_widget_draw_internal(uv,xy);

if(strlen(c->caption)>0)
{
fxy[0]=xy[0]+(xy[2]-xy[0]);
fxy[1]=xy[1]+(xy[3]-xy[1])/2;
raydium_osd_color_change(scol[0],scol[1],scol[2]);
raydium_osd_printf(fxy[0],fxy[1],
		   raydium_gui_windows[window].widgets[w].font_size,
		   0.5,raydium_gui_theme_current.font,"%s",c->caption);
} // end caption drawing

if(raydium_gui_window_focused==window &&
  (mxy[0]>=xy[0] && mxy[1]>=xy[1] &&
   mxy[0]<=xy[2] && mxy[1]<=xy[3] ))
    style=RAYDIUM_GUI_HOVER;

// Events management
if(raydium_gui_window_focused==window && 
    ((style==RAYDIUM_GUI_HOVER && raydium_mouse_click==1)  ||
     (style==RAYDIUM_GUI_FOCUS && raydium_key_last==1013)) )
    {
    raydium_mouse_click=0;
    raydium_mouse_button[0]=0;
    raydium_key_last=0;
    raydium_gui_windows[window].focused_widget=w;
    c->checked=!c->checked;
    }
}


void raydium_gui_combo_draw(int w, int window)
{
GLfloat uv[4];
GLfloat xy[4];
GLfloat fxy[2];
GLfloat mxy[2];
GLfloat wfactor[2];
GLfloat *suv;
GLfloat tmp,tmp2,tmp3;
char style;
raydium_gui_Combo *c;
int nitems;
int i;
char str[RAYDIUM_MAX_NAME_LEN];
int start,end,cpt;
char arrow;
int inc=0;

if(!raydium_gui_window_isvalid(window))
    return;
if(!raydium_gui_widget_isvalid(w,window))
    return;

c=raydium_gui_windows[window].widgets[w].widget;

mxy[0]=    ((float)raydium_mouse_x/raydium_window_tx)*100;
mxy[1]=100-((float)raydium_mouse_y/raydium_window_ty)*100;


wfactor[0]=raydium_gui_windows[window].size[0]/100.;
wfactor[1]=raydium_gui_windows[window].size[1]/100.;

xy[0]=raydium_gui_windows[window].pos[0]+
     (raydium_gui_windows[window].widgets[w].pos[0]*wfactor[0]);
xy[1]=raydium_gui_windows[window].pos[1]+
     (raydium_gui_windows[window].widgets[w].pos[1]*wfactor[1]);
xy[2]=raydium_gui_windows[window].pos[0] +
     (raydium_gui_windows[window].widgets[w].pos[0]*wfactor[0]) +
      raydium_gui_windows[window].widgets[w].size[0];
xy[3]=raydium_gui_windows[window].pos[1] +
     (raydium_gui_windows[window].widgets[w].pos[1]*wfactor[1]) +
      raydium_gui_windows[window].widgets[w].size[1];


style=RAYDIUM_GUI_NORMAL;

if(raydium_gui_windows[window].focused_widget==w)
    style=RAYDIUM_GUI_FOCUS;

/*
if(raydium_gui_window_focused==window &&
  (mxy[0]>=xy[0] && mxy[1]>=xy[1] &&
   mxy[0]<=xy[2] && mxy[1]<=xy[3] ))
	style=RAYDIUM_GUI_HOVER;
*/

// uv
switch(style)
    {
    case RAYDIUM_GUI_NORMAL:
	suv=c->uv_body_normal;
	break;
    case RAYDIUM_GUI_FOCUS:
	suv=c->uv_body_focus;
	break;
    }

uv[0]=  suv[0]/raydium_gui_theme_current.texture_size[0];
uv[1]=1-suv[1]/raydium_gui_theme_current.texture_size[1];
uv[2]=  (suv[0] + suv[2]) / raydium_gui_theme_current.texture_size[0];
uv[3]=1-(suv[1] + suv[3]) / raydium_gui_theme_current.texture_size[1];

raydium_gui_widget_draw_internal(uv,xy);

// count items
for(i=0,nitems=0;c->items[i];i++)
    if(c->items[i]==RAYDIUM_GUI_ITEM_SEPARATOR)
	nitems++;
nitems++;

if(c->current>=0 && c->current<nitems)
{
fxy[0]=xy[0];
fxy[1]=xy[1]+(xy[3]-xy[1])/2;
tmp=raydium_gui_windows[window].widgets[w].font_size/RAYDIUM_OSD_FONT_SIZE_FACTOR;
tmp2=(xy[2]-xy[0])-tmp;
tmp2/=tmp;

// find current string
str[0]=0;
for(i=0,cpt=0,start=0;c->items[i];i++)
    {
    if(c->items[i]==RAYDIUM_GUI_ITEM_SEPARATOR || !c->items[i])
	cpt++;
    
    if(cpt==c->current)
	{
	start=i+1;

	// first value without previous empty workaround
	if(cpt==0)
	    start=0;

	break;
	}
    }

for(end=start;c->items[end] && c->items[end]!=RAYDIUM_GUI_ITEM_SEPARATOR;end++)
    {
    // nothing ;)
    }

for(i=0;i<end-start;i++)
    str[i]=c->items[i+start];
str[end-start]=0;

strcpy(c->current_str,str);


// find body max str len (tmp3)
tmp3=c->body_border_right*((xy[2]-xy[0])/(c->uv_body_normal[2]));
tmp3=tmp2-(tmp3/tmp);

// limit str size using tmp3
if(tmp3 < strlen(str))
    str[(int)tmp3]=0;

raydium_osd_color_change(c->font_color[0],c->font_color[1],c->font_color[2]);
raydium_osd_printf(fxy[0]+tmp,fxy[1],
		   raydium_gui_windows[window].widgets[w].font_size,
		   0.5,raydium_gui_theme_current.font,"%s",str);
} // end caption drawing


if(raydium_gui_windows[window].focused_widget!=w && c->expanded)
    c->expanded=0;

// Events management of body
if(raydium_gui_window_focused==window &&
   mxy[0]>=xy[0] && mxy[1]>=xy[1] &&
   mxy[0]<=xy[2] && mxy[1]<=xy[3] &&
   raydium_mouse_click==1 )
    {
    raydium_mouse_click=0;
    raydium_mouse_button[0]=0;
    c->expanded=!c->expanded;
    raydium_gui_windows[window].focused_widget=w;
    }

if(raydium_gui_window_focused==window && 
   style==RAYDIUM_GUI_FOCUS &&
   raydium_key_last==1013)
   {
    raydium_key_last=0;
    c->expanded=!c->expanded;
   }


// draw expanded part, if needed
if(c->expanded && raydium_gui_windows[window].focused_widget==w)
{
if((raydium_key_last==101 || raydium_mouse_click==4) && c->current>0)
    {
    c->current--;
    raydium_key_last=0;
    raydium_mouse_click=0;
    raydium_mouse_button[0]=0;
    if(c->current < c->offset)
	c->offset=c->current;
    }
if((raydium_key_last==103 || raydium_mouse_click==5) && c->current<nitems-1)
    {
    c->current++;
    raydium_key_last=0;
    raydium_mouse_click=0;
    raydium_mouse_button[0]=0;
    if(c->current >= c->offset+RAYDIUM_GUI_COMBO_LIST_HEIGHT)
	c->offset=c->current-RAYDIUM_GUI_COMBO_LIST_HEIGHT+1;
    }


/*
if(raydium_key_last==1032)
    {
    c->offset++;
    raydium_key_last=0;
    }
*/

if(c->offset>nitems-RAYDIUM_GUI_COMBO_LIST_HEIGHT)
    c->offset=nitems-RAYDIUM_GUI_COMBO_LIST_HEIGHT;
if(c->offset<0)
    c->offset=0;

// Apply offset (result's stored in 'i')
for(i=0,cpt=0;i<strlen(c->items)+1;i++)
    {
    if(cpt==c->offset)
	break;
    if(c->items[i]==RAYDIUM_GUI_ITEM_SEPARATOR || !c->items[i])
	cpt++;
    }

// foreach element
    start=i;
    cpt=0;
    for(;i<strlen(c->items)+1;i++)
	if(c->items[i]==RAYDIUM_GUI_ITEM_SEPARATOR || !c->items[i])
	    {
	    end=i;
	    memcpy(str,c->items+start,end-start);
	    str[end-start]=0;
	    //raydium_log("%i %i -%s-",start,end,str);
	    
	    //	suv
	    arrow=0;	    
	    suv=c->uv_list_middle;
	    if(cpt==0)
		{
		suv=c->uv_list_top;
		arrow=-1;
		}
	    if(cpt==nitems-1 || cpt==RAYDIUM_GUI_COMBO_LIST_HEIGHT-1)
		{
		suv=c->uv_list_bottom;
		arrow=1;
		}

	    uv[0]=  suv[0]/raydium_gui_theme_current.texture_size[0];
	    uv[1]=1-suv[1]/raydium_gui_theme_current.texture_size[1];
	    uv[2]=  (suv[0] + suv[2]) / raydium_gui_theme_current.texture_size[0];
	    uv[3]=1-(suv[1] + suv[3]) / raydium_gui_theme_current.texture_size[1];

	    xy[1]-=raydium_gui_windows[window].widgets[w].size[1];
	    xy[3]-=raydium_gui_windows[window].widgets[w].size[1];
	    //	draw
	    raydium_gui_widget_draw_internal(uv,xy);

	    //	test current
	    //	    suv current and draw
	    if(cpt+c->offset==c->current)
		{
		suv=c->uv_list_current;
		uv[0]=  suv[0]/raydium_gui_theme_current.texture_size[0];
		uv[1]=1-suv[1]/raydium_gui_theme_current.texture_size[1];
		uv[2]=  (suv[0] + suv[2]) / raydium_gui_theme_current.texture_size[0];
		uv[3]=1-(suv[1] + suv[3]) / raydium_gui_theme_current.texture_size[1];
		raydium_gui_widget_draw_internal(uv,xy);		
		}

	    if(arrow)
		{
		GLfloat axy[4];
		GLfloat sw;
		suv=c->uv_arrow;
		uv[0]=  suv[0]/raydium_gui_theme_current.texture_size[0];
		uv[1]=1-suv[1]/raydium_gui_theme_current.texture_size[1];
		uv[2]=  (suv[0] + suv[2]) / raydium_gui_theme_current.texture_size[0];		uv[3]=1-(suv[1] + suv[3]) / raydium_gui_theme_current.texture_size[1];
		if(arrow<0)
		    {
		    sw=uv[1];
		    uv[1]=uv[3];
		    uv[3]=sw;
		    }
		axy[1]=xy[1];
		axy[3]=xy[3];
		axy[0]=xy[2];
		axy[2]=xy[2]+(axy[3]-axy[1]);
		raydium_gui_widget_draw_internal(uv,axy);
		if(raydium_gui_window_focused==window &&
		   mxy[0]>=axy[0] && mxy[1]>=axy[1] &&
		   mxy[0]<=axy[2] && mxy[1]<=axy[3] &&
		   raydium_mouse_click==1 )
		    {
	    	    raydium_mouse_click=0;
		    raydium_mouse_button[0]=0;
	    	    raydium_gui_windows[window].focused_widget=w;
		    inc=arrow;
	    	    }
		} // end if arrow

	    fxy[0]=xy[0];
	    fxy[1]=xy[1]+(xy[3]-xy[1])/2;
	    tmp=raydium_gui_windows[window].widgets[w].font_size/RAYDIUM_OSD_FONT_SIZE_FACTOR;
	    tmp2=(xy[2]-xy[0])-tmp;
	    tmp2/=tmp;

	    if(tmp2<strlen(str))
	        str[(int)tmp2]=0;

	    raydium_osd_color_change(c->font_color[0],c->font_color[1],c->font_color[2]);
	    raydium_osd_printf(fxy[0]+tmp,fxy[1],
		   raydium_gui_windows[window].widgets[w].font_size,
		   0.5,raydium_gui_theme_current.font,"%s",str);

	    //	events (change current ?)
	    if(raydium_gui_window_focused==window &&
	       mxy[0]>=xy[0] && mxy[1]>=xy[1] &&
	       mxy[0]<=xy[2] && mxy[1]<=xy[3] &&
	       raydium_mouse_click==1 )
		{
		c->current=cpt+c->offset;
	        raydium_mouse_click=0;
		raydium_mouse_button[0]=0;
	        c->expanded=0;
	        raydium_gui_windows[window].focused_widget=w;
	        }

	    if(cpt==RAYDIUM_GUI_COMBO_LIST_HEIGHT-1)
		break;

	    start=end+1;
	    cpt++;
	    } // end SEPARATOR
} // end expanded

c->offset+=inc;
}


void raydium_gui_window_draw(int window)
{
int i;
GLfloat uv[4];
GLfloat xy[4];

if(!raydium_gui_window_isvalid(window))
    return;

uv[0]=  raydium_gui_theme_current.background_uv[0]/raydium_gui_theme_current.texture_size[0];
uv[1]=1-raydium_gui_theme_current.background_uv[1]/raydium_gui_theme_current.texture_size[1];
uv[2]=
    (raydium_gui_theme_current.background_uv[0] +
     raydium_gui_theme_current.background_uv[2] ) /
     raydium_gui_theme_current.texture_size[0];
uv[3]=1-
    (raydium_gui_theme_current.background_uv[1] +
     raydium_gui_theme_current.background_uv[3] ) /
     raydium_gui_theme_current.texture_size[1];

xy[0]=raydium_gui_windows[window].pos[0];
xy[1]=raydium_gui_windows[window].pos[1];
xy[2]=raydium_gui_windows[window].pos[0]+raydium_gui_windows[window].size[0];
xy[3]=raydium_gui_windows[window].pos[1]+raydium_gui_windows[window].size[1];

raydium_gui_widget_draw_internal(uv,xy);

// draw combos, first pass (capture input events first)
for(i=0;i<RAYDIUM_GUI_MAX_OBJECTS;i++)
    if(raydium_gui_widget_isvalid(i,window))
	switch(raydium_gui_windows[window].widgets[i].type)
	{
	case RAYDIUM_GUI_COMBO:
	    raydium_gui_combo_draw(i,window);
	    break;
	}

// draw other components
for(i=0;i<RAYDIUM_GUI_MAX_OBJECTS;i++)
    if(raydium_gui_widget_isvalid(i,window))
	switch(raydium_gui_windows[window].widgets[i].type)
	{
	case RAYDIUM_GUI_BUTTON:
	    raydium_gui_button_draw(i,window);
	    break;
	case RAYDIUM_GUI_LABEL:
	    raydium_gui_label_draw(i,window);
	    break;
	case RAYDIUM_GUI_TRACK:
	    raydium_gui_track_draw(i,window);
	    break;
	case RAYDIUM_GUI_EDIT:
	    raydium_gui_edit_draw(i,window);
	    break;
	case RAYDIUM_GUI_CHECK:
	    raydium_gui_check_draw(i,window);
	    break;
	case RAYDIUM_GUI_ZONE:
	    raydium_gui_zone_draw(i,window);
	    break;
	}

// draw combos last (overlay all components)
for(i=0;i<RAYDIUM_GUI_MAX_OBJECTS;i++)
    if(raydium_gui_widget_isvalid(i,window))
	switch(raydium_gui_windows[window].widgets[i].type)
	{
	case RAYDIUM_GUI_COMBO:
	    raydium_gui_combo_draw(i,window);
	    break;
	}

// unfocus component
if(raydium_mouse_click==1)
    {
    raydium_gui_windows[window].focused_widget=-1;
    }
}

void raydium_gui_draw(void)
{
int i;
void (*f)(void);

if(!raydium_gui_visible)
    return;
if(!raydium_gui_theme_current.loaded)
    return;

if(!raydium_gui_window_isvalid(raydium_gui_window_focused))
    raydium_gui_window_focused=-1;

if(raydium_key_last==1009)
    {
    raydium_gui_widget_next();
    raydium_key_last=0;
    }

// non focused windows (~ creation order)
for(i=0;i<RAYDIUM_GUI_MAX_WINDOWS;i++)
    if(raydium_gui_windows[i].state && i!=raydium_gui_window_focused)
	raydium_gui_window_draw(i);

// focus window
for(i=0;i<RAYDIUM_GUI_MAX_WINDOWS;i++)
    if(raydium_gui_windows[i].state && i==raydium_gui_window_focused)
	{
	raydium_gui_window_draw(i);
	break;
	}

raydium_osd_color_ega('f');

f=raydium_gui_AfterGuiDrawCallback;
if(f)
    f();
}


////////////////////////
// Internal accessors //
////////////////////////

int raydium_gui_button_read(int window, int widget, char *str)
{
if(!raydium_gui_widget_isvalid(widget,window))
    {
    raydium_log("gui: error: cannot read widget value: invalid name or index");
    return 0;
    }
// Nothing is readable on a button :
str[0]=0;
return 0;
}

int raydium_gui_button_write(int window, int widget, char *str)
{
raydium_gui_Button *w;

if(!raydium_gui_widget_isvalid(widget,window))
    {
    raydium_log("gui: error: cannot write widget value: invalid name or index");
    return 0;
    }

w=raydium_gui_windows[window].widgets[widget].widget;
strcpy(w->caption,str);
return 1;
}


int raydium_gui_label_read(int window, int widget, char *str)
{
if(!raydium_gui_widget_isvalid(widget,window))
    {
    raydium_log("gui: error: cannot read widget value: invalid name or index");
    return 0;
    }
// Nothing is readable on a label :
str[0]=0;
return 0;
}

int raydium_gui_label_write(int window, int widget, char *str)
{
raydium_gui_Label *w;

if(!raydium_gui_widget_isvalid(widget,window))
    {
    raydium_log("gui: error: cannot write widget value: invalid name or index");
    return 0;
    }

w=raydium_gui_windows[window].widgets[widget].widget;
strcpy(w->caption,str);
return 1;
}


int raydium_gui_track_read(int window, int widget, char *str)
{
raydium_gui_Track *t;

if(!raydium_gui_widget_isvalid(widget,window))
    {
    raydium_log("gui: error: cannot read widget value: invalid name or index");
    return 0;
    }
t=raydium_gui_windows[window].widgets[widget].widget;
// value|ratio|min|max
sprintf(str,"%i|%f|%i|%i",t->current,
			 (float)(t->current-t->min)/(float)(t->max-t->min),
			 t->min,
			 t->max);
return t->current;
}

int raydium_gui_track_write(int window, int widget, int value)
{
raydium_gui_Track *w;

if(!raydium_gui_widget_isvalid(widget,window))
    {
    raydium_log("gui: error: cannot write widget value: invalid name or index");
    return 0;
    }

w=raydium_gui_windows[window].widgets[widget].widget;

if(value<w->min || value>w->max)
    return 0;

w->current=value;
return 1;
}

int raydium_gui_edit_read(int window, int widget, char *str)
{
raydium_gui_Edit *e;

if(!raydium_gui_widget_isvalid(widget,window))
    {
    raydium_log("gui: error: cannot read widget value: invalid name or index");
    return 0;
    }
e=raydium_gui_windows[window].widgets[widget].widget;
strcpy(str,e->text);
return strlen(e->text);
}

int raydium_gui_edit_write(int window, int widget, char *str)
{
raydium_gui_Edit *w;

if(!raydium_gui_widget_isvalid(widget,window))
    {
    raydium_log("gui: error: cannot write widget value: invalid name or index");
    return 0;
    }

w=raydium_gui_windows[window].widgets[widget].widget;
strcpy(w->text,str);
return 1;
}


int raydium_gui_check_read(int window, int widget, char *str)
{
raydium_gui_Check *c;

if(!raydium_gui_widget_isvalid(widget,window))
    {
    raydium_log("gui: error: cannot read widget value: invalid name or index");
    return 0;
    }
c=raydium_gui_windows[window].widgets[widget].widget;
sprintf(str,"%s",(c->checked?"true":"false"));
return c->checked;
}

int raydium_gui_check_write(int window, int widget, int value)
{
raydium_gui_Check *w;

if(!raydium_gui_widget_isvalid(widget,window))
    {
    raydium_log("gui: error: cannot write widget value: invalid name or index");
    return 0;
    }

w=raydium_gui_windows[window].widgets[widget].widget;

w->checked=(value?1:0);
return 1;
}

int raydium_gui_combo_read(int window, int widget, char *str)
{
raydium_gui_Combo *c;

if(!raydium_gui_widget_isvalid(widget,window))
    {
    raydium_log("gui: error: cannot read widget value: invalid name or index");
    return 0;
    }
c=raydium_gui_windows[window].widgets[widget].widget;
strcpy(str,c->current_str);
return c->current;
}

int raydium_gui_combo_write(int window, int widget, int value)
{
raydium_gui_Combo *w;

if(!raydium_gui_widget_isvalid(widget,window))
    {
    raydium_log("gui: error: cannot write widget value: invalid name or index");
    return 0;
    }

w=raydium_gui_windows[window].widgets[widget].widget;
w->current=value;
return 1;
}

int raydium_gui_zone_read(int window, int widget, char *str)
{
raydium_gui_Zone *z;

if(!raydium_gui_widget_isvalid(widget,window))
    {
    raydium_log("gui: error: cannot read widget value: invalid name or index");
    return 0;
    }
// Nothing is readable on a zone :
z=raydium_gui_windows[window].widgets[widget].widget;
sprintf(str,"%i",z->tag);
return z->tag;
}


////////////////////////////
// "public" GUI functions //
////////////////////////////

void raydium_gui_show(void)
{
raydium_gui_visible=1;
if(!raydium_osd_cursor_texture)
    {
    raydium_gui_oldstate=raydium_mouse_isvisible();
    raydium_mouse_show();
    }
}

void raydium_gui_hide(void)
{
raydium_gui_visible=0;
if(!raydium_osd_cursor_texture && !raydium_gui_oldstate)
    {
    raydium_mouse_hide();
    raydium_gui_oldstate=0;
    }
}

signed char raydium_gui_isvisible(void)
{
return raydium_gui_visible;
}

void raydium_gui_window_delete(int window)
{
void (*f)(void);

if(!raydium_gui_window_isvalid(window))
    {
    raydium_log("GUI: Error: Cannot delete window: invalid window");
    return;
    }

f=raydium_gui_windows[window].OnDelete;
if(f)
    f();

// focus another window
raydium_gui_window_focused=raydium_gui_windows[window].old_focused;
raydium_gui_window_init(window);
}

void raydium_gui_window_delete_name(char *window)
{
raydium_gui_window_delete(raydium_gui_window_find(window));
}


void raydium_gui_window_OnDelete(int window, void *OnDelete)
{
if(!raydium_gui_window_isvalid(window))
    {
    raydium_log("GUI: Error: Cannot set OnDelete: invalid window");
    return;
    }

raydium_gui_windows[window].OnDelete=OnDelete;
}

void raydium_gui_window_OnDelete_name(char *window, void *OnDelete)
{
raydium_gui_window_OnDelete(raydium_gui_window_find(window),OnDelete);
}

void raydium_gui_widget_sizes(GLfloat sizex, GLfloat sizey, GLfloat font_size)
{
raydium_gui_widget_sizes_default[0]=sizex;
raydium_gui_widget_sizes_default[1]=sizey;
raydium_gui_widget_sizes_default[2]=font_size;
}

int raydium_gui_window_create(char *name, GLfloat px, GLfloat py, GLfloat sizex, GLfloat sizey)
{
int i;

if(raydium_gui_window_find(name)>=0)
    {
    raydium_log("GUI: Error: Cannot create \"%s\" window: name already exists",name);
    return -1;
    }

for(i=0;i<RAYDIUM_GUI_MAX_WINDOWS;i++)
    if(!raydium_gui_windows[i].state)
     {
     strcpy(raydium_gui_windows[i].name,name);
     raydium_gui_windows[i].state=1;
     raydium_gui_windows[i].pos[0]=px;
     raydium_gui_windows[i].pos[1]=py;
     raydium_gui_windows[i].size[0]=sizex;
     raydium_gui_windows[i].size[1]=sizey;
     raydium_gui_windows[i].old_focused=raydium_gui_window_focused;
     
     raydium_gui_window_focused=i; // take focus
     return i;
     }
raydium_log("GUI: Error: No more window slots ! aborting \"%s\" creation",name);
return -1;
}

int raydium_gui_internal_object_create(char *name, int window, signed char type, GLfloat px, GLfloat py, GLfloat sizex, GLfloat sizey, GLfloat font_size)
{
int i;

if(!raydium_gui_window_isvalid(window))
    {
    raydium_log("GUI: Error: Cannot create \"%s\" widget: invalid window",name);
    return -1;
    }

if(!(strlen(name)>0 && name[0]=='*'))
    if(raydium_gui_widget_find(name,window)>=0)
	{
	raydium_log("GUI: Error: Cannot create \"%s\" widget: name already exists in this window",name);
	return -1;
	}

for(i=0;i<RAYDIUM_GUI_MAX_OBJECTS;i++)
    if(!raydium_gui_windows[window].widgets[i].state)
     {
     strcpy(raydium_gui_windows[window].widgets[i].name,name);
     raydium_gui_windows[window].widgets[i].state=1;
     raydium_gui_windows[window].widgets[i].type=type;
     raydium_gui_windows[window].widgets[i].pos[0]=px;
     raydium_gui_windows[window].widgets[i].pos[1]=py;
     raydium_gui_windows[window].widgets[i].font_size=font_size;
     raydium_gui_windows[window].widgets[i].size[0]=sizex;
     raydium_gui_windows[window].widgets[i].size[1]=sizey;
     return i;
     }

raydium_log("GUI: Error: Cannot create \"%s\" widget: no more slots",name);
return -1;
}

int raydium_gui_button_create(char *name, int window,  GLfloat px, GLfloat py, char *caption, void *OnClick)
{
int wid;
raydium_gui_Button *b;
GLfloat sx,sy,fs;
// used for parsing
FILE *fp;
int ret;
char var[RAYDIUM_MAX_NAME_LEN];
char val_s[RAYDIUM_MAX_NAME_LEN];
GLfloat val_f[4];
int size;


b=malloc(sizeof(raydium_gui_Button));
if(!b)
    {
    raydium_log("GUI: Error: Cannot create \"%s\" button: malloc failed",name);
    return -1;
    }

fp=raydium_file_fopen(raydium_gui_theme_current.filename,"rt");
if(!fp)
    {
    raydium_log("gui: ERROR: Cannot open current theme file");
    return -1;
    }

sx=raydium_gui_widget_sizes_default[0];
sy=raydium_gui_widget_sizes_default[1];
fs=raydium_gui_widget_sizes_default[2];
wid=raydium_gui_internal_object_create(name,window,RAYDIUM_GUI_BUTTON,px,py,sx,sy,fs);
if(wid<0)
    {
    raydium_log("GUI: Error: early init failed for button '%s'",name);
    return -1;
    }

strcpy(b->caption,caption);
b->OnClick=OnClick;

memset(b->uv_normal, 0,sizeof(GLfloat)*4);
memset(b->uv_focus,  0,sizeof(GLfloat)*4);
memset(b->uv_hover,  0,sizeof(GLfloat)*4);
memset(b->font_color,0,sizeof(GLfloat)*3);

while( (ret=raydium_parser_read(var,val_s,val_f,&size,fp))!=RAYDIUM_PARSER_TYPE_EOF)    
    {
    if(!strcasecmp(var,"button_normal"))
        {
        if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=4)
            {
	    raydium_log("gui: parser: button_normal: wrong type");
            continue;
            }
        memcpy(b->uv_normal,val_f,sizeof(GLfloat)*4);
        }

    if(!strcasecmp(var,"button_focus"))
        {
        if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=4)
            {
	    raydium_log("gui: parser: button_focus: wrong type");
            continue;
            }
        memcpy(b->uv_focus,val_f,sizeof(GLfloat)*4);
        }

    if(!strcasecmp(var,"button_hover"))
        {
        if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=4)
            {
	    raydium_log("gui: parser: button_hover: wrong type");
            continue;
            }
        memcpy(b->uv_hover,val_f,sizeof(GLfloat)*4);
        }

    if(!strcasecmp(var,"button_font"))
        {
        if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=3)
            {
	    raydium_log("gui: parser: button_font: wrong type");
            continue;
            }
        memcpy(b->font_color,val_f,sizeof(GLfloat)*3);
        }
    }
fclose(fp);

raydium_gui_windows[window].widgets[wid].widget=b;
return wid;
}


int raydium_gui_button_create_simple(char *name, int window,  GLfloat px, GLfloat py, char *caption)
{
return raydium_gui_button_create(name,window,px,py,caption,NULL);
}


int raydium_gui_label_create(char *name, int window,  GLfloat px, GLfloat py, char *caption, GLfloat r, GLfloat g, GLfloat b)
{
int wid;
raydium_gui_Label *l;
GLfloat fs;

l=malloc(sizeof(raydium_gui_Label));
if(!l)
    {
    raydium_log("GUI: Error: Cannot create \"%s\" label: malloc failed",name);
    return -1;
    }

fs=raydium_gui_widget_sizes_default[2];
wid=raydium_gui_internal_object_create(name,window,RAYDIUM_GUI_LABEL,px,py,0,0,fs);
if(wid<0)
    {
    raydium_log("GUI: Error: early init failed for label '%s'",name);
    return -1;
    }

strcpy(l->caption,caption);
l->font_color[0]=r;
l->font_color[1]=g;
l->font_color[2]=b;
raydium_gui_windows[window].widgets[wid].widget=l;
return wid;
}


int raydium_gui_track_create(char *name, int window,  GLfloat px, GLfloat py, int min, int max, int current)
{
int wid;
raydium_gui_Track *t;
GLfloat sx,sy,fs;
// used for parsing
FILE *fp;
int ret;
char var[RAYDIUM_MAX_NAME_LEN];
char val_s[RAYDIUM_MAX_NAME_LEN];
GLfloat val_f[4];
int size;


t=malloc(sizeof(raydium_gui_Track));
if(!t)
    {
    raydium_log("GUI: Error: Cannot create \"%s\" track: malloc failed",name);
    return -1;
    }

fp=raydium_file_fopen(raydium_gui_theme_current.filename,"rt");
if(!fp)
    {
    raydium_log("gui: ERROR: Cannot open current theme file");
    return -1;
    }

sx=raydium_gui_widget_sizes_default[0];
sy=raydium_gui_widget_sizes_default[1];
fs=raydium_gui_widget_sizes_default[2];
wid=raydium_gui_internal_object_create(name,window,RAYDIUM_GUI_TRACK,px,py,sx,sy,fs);
if(wid<0)
    {
    raydium_log("GUI: Error: early init failed for track '%s'",name);
    return -1;
    }

t->min=min;
t->max=max;
t->current=current;

memset(t->uv_rule,0,sizeof(GLfloat)*4);
memset(t->uv_cursor_normal,0,sizeof(GLfloat)*4);
memset(t->uv_cursor_focus,0,sizeof(GLfloat)*4);

while( (ret=raydium_parser_read(var,val_s,val_f,&size,fp))!=RAYDIUM_PARSER_TYPE_EOF)
    {
    if(!strcasecmp(var,"track_rule"))
        {
        if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=4)
            {
	    raydium_log("gui: parser: track_rule: wrong type");
            continue;
            }
        memcpy(t->uv_rule,val_f,sizeof(GLfloat)*4);
        }

    if(!strcasecmp(var,"track_cursor_normal"))
        {
        if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=4)
            {
	    raydium_log("gui: parser: track_cursor_normal: wrong type");
            continue;
            }
        memcpy(t->uv_cursor_normal,val_f,sizeof(GLfloat)*4);
        }

    if(!strcasecmp(var,"track_cursor_focus"))
        {
        if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=4)
            {
	    raydium_log("gui: parser: track_cursor_focus: wrong type");
            continue;
            }
        memcpy(t->uv_cursor_focus,val_f,sizeof(GLfloat)*4);
        }
    }
fclose(fp);

raydium_gui_windows[window].widgets[wid].widget=t;
return wid;
}



int raydium_gui_edit_create(char *name, int window,  GLfloat px, GLfloat py, char *default_text)
{
int wid;
raydium_gui_Edit *e;
GLfloat sx,sy,fs;
// used for parsing
FILE *fp;
int ret;
char var[RAYDIUM_MAX_NAME_LEN];
char val_s[RAYDIUM_MAX_NAME_LEN];
GLfloat val_f[4];
int size;


e=malloc(sizeof(raydium_gui_Edit));
if(!e)
    {
    raydium_log("GUI: Error: Cannot create \"%s\" edit: malloc failed",name);
    return -1;
    }

fp=raydium_file_fopen(raydium_gui_theme_current.filename,"rt");
if(!fp)
    {
    raydium_log("gui: ERROR: Cannot open current theme file");
    return -1;
    }

sx=raydium_gui_widget_sizes_default[0];
sy=raydium_gui_widget_sizes_default[1];
fs=raydium_gui_widget_sizes_default[2];
wid=raydium_gui_internal_object_create(name,window,RAYDIUM_GUI_EDIT,px,py,sx,sy,fs);
if(wid<0)
    {
    raydium_log("GUI: Error: early init failed for edit '%s'",name);
    return -1;
    }

e->offset=0;
strcpy(e->text,default_text);
e->cursor=strlen(e->text);
//e->cursor=0;
memset(e->uv_normal,0,sizeof(GLfloat)*4);
memset(e->uv_focus,0,sizeof(GLfloat)*4);
memset(e->font_color,0,sizeof(GLfloat)*3);

while( (ret=raydium_parser_read(var,val_s,val_f,&size,fp))!=RAYDIUM_PARSER_TYPE_EOF)
    {
    if(!strcasecmp(var,"edit_normal"))
        {
        if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=4)
            {
	    raydium_log("gui: parser: edit_normal: wrong type");
            continue;
            }
        memcpy(e->uv_normal,val_f,sizeof(GLfloat)*4);
        }

    if(!strcasecmp(var,"edit_focus"))
        {
        if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=4)
            {
	    raydium_log("gui: parser: edit_focus: wrong type");
            continue;
            }
        memcpy(e->uv_focus,val_f,sizeof(GLfloat)*4);
        }

    if(!strcasecmp(var,"font_color"))
        {
        if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=3)
            {
	    raydium_log("gui: parser: font_color: wrong type");
            continue;
            }
        memcpy(e->font_color,val_f,sizeof(GLfloat)*3);
        }
    }
fclose(fp);

raydium_gui_windows[window].widgets[wid].widget=e;
return wid;
}

int raydium_gui_check_create(char *name, int window,  GLfloat px, GLfloat py, char *caption, signed char checked)
{
int wid;
raydium_gui_Check *c;
GLfloat sx,sy,fs;
// used for parsing
FILE *fp;
int ret;
char var[RAYDIUM_MAX_NAME_LEN];
char val_s[RAYDIUM_MAX_NAME_LEN];
GLfloat val_f[4];
int size;


c=malloc(sizeof(raydium_gui_Check));
if(!c)
    {
    raydium_log("GUI: Error: Cannot create \"%s\" check: malloc failed",name);
    return -1;
    }

fp=raydium_file_fopen(raydium_gui_theme_current.filename,"rt");
if(!fp)
    {
    raydium_log("gui: ERROR: Cannot open current theme file");
    return -1;
    }

sx=raydium_gui_widget_sizes_default[0];
sy=raydium_gui_widget_sizes_default[1];
fs=raydium_gui_widget_sizes_default[2];
wid=raydium_gui_internal_object_create(name,window,RAYDIUM_GUI_CHECK,px,py,sx,sy,fs);
if(wid<0)
    {
    raydium_log("GUI: Error: early init failed for check '%s'",name);
    return -1;
    }

strcpy(c->caption,caption);
c->checked=checked;
memset(c->uv_normal,0,sizeof(GLfloat)*4);
memset(c->uv_checked,0,sizeof(GLfloat)*4);
memset(c->font_color_normal,0,sizeof(GLfloat)*3);
memset(c->font_color_focus,0,sizeof(GLfloat)*3);

while( (ret=raydium_parser_read(var,val_s,val_f,&size,fp))!=RAYDIUM_PARSER_TYPE_EOF)
    {
    if(!strcasecmp(var,"check_normal"))
        {
        if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=4)
            {
	    raydium_log("gui: parser: check_normal: wrong type");
            continue;
            }
        memcpy(c->uv_normal,val_f,sizeof(GLfloat)*4);
        }

    if(!strcasecmp(var,"check_checked"))
        {
        if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=4)
            {
	    raydium_log("gui: parser: check_checked: wrong type");
            continue;
            }
        memcpy(c->uv_checked,val_f,sizeof(GLfloat)*4);
        }

    if(!strcasecmp(var,"font_color_normal"))
        {
        if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=3)
            {
	    raydium_log("gui: parser: font_color_normal: wrong type");
            continue;
            }
        memcpy(c->font_color_normal,val_f,sizeof(GLfloat)*3);
        }

    if(!strcasecmp(var,"font_color_focus"))
        {
        if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=3)
            {
	    raydium_log("gui: parser: font_color_focus: wrong type");
            continue;
            }
        memcpy(c->font_color_focus,val_f,sizeof(GLfloat)*3);
        }
    }
fclose(fp);

raydium_gui_windows[window].widgets[wid].widget=c;
return wid;
}

int raydium_gui_combo_create(char *name, int window,  GLfloat px, GLfloat py, char *items, int current)
{
int wid;
raydium_gui_Combo *c;
GLfloat sx,sy,fs;
// used for parsing
FILE *fp;
int ret;
char var[RAYDIUM_MAX_NAME_LEN];
char val_s[RAYDIUM_MAX_NAME_LEN];
GLfloat val_f[4];
int size;


c=malloc(sizeof(raydium_gui_Combo));
if(!c)
    {
    raydium_log("GUI: Error: Cannot create \"%s\" combo: malloc failed",name);
    return -1;
    }

fp=raydium_file_fopen(raydium_gui_theme_current.filename,"rt");
if(!fp)
    {
    raydium_log("gui: ERROR: Cannot open current theme file");
    return -1;
    }

sx=raydium_gui_widget_sizes_default[0];
sy=raydium_gui_widget_sizes_default[1];
fs=raydium_gui_widget_sizes_default[2];
wid=raydium_gui_internal_object_create(name,window,RAYDIUM_GUI_COMBO,px,py,sx,sy,fs);
if(wid<0)
    {
    raydium_log("GUI: Error: early init failed for combo '%s'",name);
    return -1;
    }

strcpy(c->items,items);
c->current=current;
c->expanded=0;
c->offset=current;
c->body_border_right=0;
memset(c->uv_body_normal,0,sizeof(GLfloat)*4);
memset(c->uv_body_focus,0,sizeof(GLfloat)*4);
memset(c->uv_list_top,0,sizeof(GLfloat)*4);
memset(c->uv_list_middle,0,sizeof(GLfloat)*4);
memset(c->uv_list_bottom,0,sizeof(GLfloat)*4);
memset(c->uv_list_current,0,sizeof(GLfloat)*4);
memset(c->uv_arrow,0,sizeof(GLfloat)*4);
memset(c->font_color,0,sizeof(GLfloat)*3);

while( (ret=raydium_parser_read(var,val_s,val_f,&size,fp))!=RAYDIUM_PARSER_TYPE_EOF)
    {
    if(!strcasecmp(var,"combo_body_normal"))
        {
        if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=4)
            {
	    raydium_log("gui: parser: combo_body_normal: wrong type");
            continue;
            }
        memcpy(c->uv_body_normal,val_f,sizeof(GLfloat)*4);
        }

    if(!strcasecmp(var,"combo_body_focus"))
        {
        if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=4)
            {
	    raydium_log("gui: parser: combo_body_focus: wrong type");
            continue;
            }
        memcpy(c->uv_body_focus,val_f,sizeof(GLfloat)*4);
        }

    if(!strcasecmp(var,"combo_body_border_right"))
        {
        if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=1)
            {
	    raydium_log("gui: parser: combo_body_border_right: wrong type");
            continue;
            }
        c->body_border_right=(int)val_f[0];
        }


    if(!strcasecmp(var,"combo_list_top"))
        {
        if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=4)
            {
	    raydium_log("gui: parser: combo_list_top: wrong type");
            continue;
            }
        memcpy(c->uv_list_top,val_f,sizeof(GLfloat)*4);
        }

    if(!strcasecmp(var,"combo_list_middle"))
        {
        if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=4)
            {
	    raydium_log("gui: parser: combo_list_middle: wrong type");
            continue;
            }
        memcpy(c->uv_list_middle,val_f,sizeof(GLfloat)*4);
        }

    if(!strcasecmp(var,"combo_list_bottom"))
        {
        if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=4)
            {
	    raydium_log("gui: parser: combo_list_bottom: wrong type");
            continue;
            }
        memcpy(c->uv_list_bottom,val_f,sizeof(GLfloat)*4);
        }

    if(!strcasecmp(var,"combo_list_current"))
        {
        if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=4)
            {
	    raydium_log("gui: parser: combo_list_current: wrong type");
            continue;
            }
        memcpy(c->uv_list_current,val_f,sizeof(GLfloat)*4);
        }

    if(!strcasecmp(var,"combo_arrow"))
        {
        if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=4)
            {
	    raydium_log("gui: parser: combo_arrow: wrong type");
            continue;
            }
        memcpy(c->uv_arrow,val_f,sizeof(GLfloat)*4);
        }

    if(!strcasecmp(var,"font_color"))
        {
        if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=3)
            {
	    raydium_log("gui: parser: font_color: wrong type");
            continue;
            }
        memcpy(c->font_color,val_f,sizeof(GLfloat)*3);
        }
    }
fclose(fp);

raydium_gui_windows[window].widgets[wid].widget=c;
return wid;
}

int raydium_gui_zone_create(char *name, int window,  GLfloat px, GLfloat py, GLfloat sx, GLfloat sy, int tag, void *OnClick)
{
int wid;
raydium_gui_Zone *z;
// used for parsing
FILE *fp;
int ret;
char var[RAYDIUM_MAX_NAME_LEN];
char val_s[RAYDIUM_MAX_NAME_LEN];
GLfloat val_f[4];
int size;


z=malloc(sizeof(raydium_gui_Zone));
if(!z)
    {
    raydium_log("GUI: Error: Cannot create \"%s\" zone: malloc failed",name);
    return -1;
    }

fp=raydium_file_fopen(raydium_gui_theme_current.filename,"rt");
if(!fp)
    {
    raydium_log("gui: ERROR: Cannot open current theme file");
    return -1;
    }


if(raydium_gui_window_isvalid(window))
    {
    sx*=raydium_gui_windows[window].size[0]/100.;
    sy*=raydium_gui_windows[window].size[1]/100.;
    }

wid=raydium_gui_internal_object_create(name,window,RAYDIUM_GUI_ZONE,px,py,sx,sy,0);
if(wid<0)
    {
    raydium_log("GUI: Error: early init failed for zone '%s'",name);
    return -1;
    }

z->OnClick=OnClick;
z->tag=tag;
memset(z->col_normal, 0,sizeof(GLfloat)*4);
memset(z->col_focus,  1,sizeof(GLfloat)*4);
memset(z->col_hover,  1,sizeof(GLfloat)*4);

while( (ret=raydium_parser_read(var,val_s,val_f,&size,fp))!=RAYDIUM_PARSER_TYPE_EOF)    
    {
    if(!strcasecmp(var,"zone_normal"))
        {
        if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=4)
            {
	    raydium_log("gui: parser: zone_normal: wrong type");
            continue;
            }
        memcpy(z->col_normal,val_f,sizeof(GLfloat)*4);
        }

    if(!strcasecmp(var,"zone_focus"))
        {
        if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=4)
            {
	    raydium_log("gui: parser: zone_focus: wrong type");
            continue;
            }
        memcpy(z->col_focus,val_f,sizeof(GLfloat)*4);
        }

    if(!strcasecmp(var,"zone_hover"))
        {
        if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=4)
            {
	    raydium_log("gui: parser: zone_hover: wrong type");
            continue;
            }
        memcpy(z->col_hover,val_f,sizeof(GLfloat)*4);
        }
    }
fclose(fp);

raydium_gui_windows[window].widgets[wid].widget=z;
return wid;

}

// str may be big !
int raydium_gui_read(int window, int widget, char *str)
{
    if(raydium_gui_widget_isvalid(widget,window))
    {
	switch(raydium_gui_windows[window].widgets[widget].type)
	{
	case RAYDIUM_GUI_BUTTON:
	    return raydium_gui_button_read(window,widget,str);
	case RAYDIUM_GUI_LABEL:
	    return raydium_gui_label_read(window,widget,str);
	case RAYDIUM_GUI_TRACK:
	    return raydium_gui_track_read(window,widget,str);
	case RAYDIUM_GUI_EDIT:
	    return raydium_gui_edit_read(window,widget,str);
	case RAYDIUM_GUI_CHECK:
	    return raydium_gui_check_read(window,widget,str);
	case RAYDIUM_GUI_COMBO:
	    return raydium_gui_combo_read(window,widget,str);
	case RAYDIUM_GUI_ZONE:
	    return raydium_gui_zone_read(window,widget,str);
	}
    }
    else 
	raydium_log("gui: error: cannot read widget value: invalid name or index");

return 0;
}

int raydium_gui_read_name(char *window, char *widget, char *str)
{
int w;
w=raydium_gui_window_find(window);
return raydium_gui_read(w,raydium_gui_widget_find(widget,w),str);
}

int raydium_gui_read_widget(raydium_gui_Object *w, char *str)
{
if(!w)
    {
    raydium_log("gui: error: cannot read widget value: invalid name or index");
    return 0;
    }
return raydium_gui_read(w->window,w->id,str);
}


signed char raydium_gui_write(int window, int widget, char *str, int value)
{
if(!raydium_gui_widget_isvalid(widget,window))
    {
    raydium_log("gui: error: cannot write widget value: invalid name or index");
    return 0;
    }

switch(raydium_gui_windows[window].widgets[widget].type)
{
case RAYDIUM_GUI_BUTTON:
    return raydium_gui_button_write(window,widget,str);
case RAYDIUM_GUI_LABEL:
    return raydium_gui_label_write(window,widget,str);
case RAYDIUM_GUI_TRACK:
    return raydium_gui_track_write(window,widget,value);
case RAYDIUM_GUI_EDIT:
    return raydium_gui_edit_write(window,widget,str);
case RAYDIUM_GUI_CHECK:
    return raydium_gui_check_write(window,widget,value);
case RAYDIUM_GUI_COMBO:
    return raydium_gui_combo_write(window,widget,value);
//case RAYDIUM_GUI_ZONE:
    // ...
}

return -1; // nothing was done
}

int raydium_gui_write_name(char *window, char *widget, char *str, int value)
{
int w;
w=raydium_gui_window_find(window);
return raydium_gui_write(w,raydium_gui_widget_find(widget,w),str,value);
}


int raydium_gui_button_clicked(void)
{
int ret;
ret=raydium_gui_button_clicked_id;
raydium_gui_button_clicked_id=-1;
return ret;
}


int raydium_gui_list_id(char *item, char *list)
{
int i,cpt,len,start;
char items[RAYDIUM_GUI_DATASIZE];

strcpy(items,list);

len=strlen(items);
for(i=0,cpt=0,start=0;i<=len;i++)
    if(items[i]==RAYDIUM_GUI_ITEM_SEPARATOR || items[i]==0)
    {
    items[i]=0;
    if(!strcmp(item,items+start))
	{
	return cpt;
	}
    cpt++;
    start=i+1;
    }
return -1;
}
