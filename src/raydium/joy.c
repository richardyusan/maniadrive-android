/*
    Raydium - CQFD Corp.
    http://raydium.org/
    License: GPL - GNU General Public License, see "gpl.txt" file.
*/

#ifndef DONT_INCLUDE_HEADERS
#include "index.h"
#else
#include "headers/joy.h"
#endif 

// proto
int raydium_init_cli_option_default(char *option, char *value, char *default_value);

// This file needs a lot of work, again ...
// (will use Glut for windows joy support soon ...)

//#define joy_debug 1			//do we display debug infos ?

#define JS_EVENT_BUTTON         0x01    /* button pressed/released */
#define JS_EVENT_AXIS           0x02    /* joystick moved */
#define JS_EVENT_INIT           0x80    /* initial state of device */
/*
			// function                     3rd arg
#define JSIOCGAXES      // get number of axes           char
#define JSIOCGBUTTONS   // get number of buttons        char
#define JSIOCGVERSION   // get driver version           int
#define JSIOCGNAME(len) // get identifier string        char
#define JSIOCSCORR      // set correction values        &js_corr
#define JSIOCGCORR      // get correction values        &js_corr
*/

//int joy;				//file handle

//struct input_event play;
//struct input_event stop;
//struct ff_effect effect;
int raydium_joy_event_handle;
#ifndef WIN32
struct ff_effect effect_tremble;
#endif
char effect_tremble_state=0;
clock_t last_event;


void raydium_joy_init_vars(void)
{
memset(raydium_joy_button,0,RAYDIUM_JOY_MAX_BUTTONS);
raydium_joy_x=raydium_joy_y=raydium_joy_z=0.f;
raydium_joy_click=0;
}


void raydium_joy_key_emul(void)
{
if(raydium_joy) return;

if(raydium_key[GLUT_KEY_UP]) raydium_joy_y=1.f;
if(raydium_key[GLUT_KEY_DOWN]) raydium_joy_y=-1.f;

if(raydium_key[GLUT_KEY_LEFT]) raydium_joy_x=-1.f;
if(raydium_key[GLUT_KEY_RIGHT]) raydium_joy_x=1.f;

//buttons
}

#ifndef WIN32
int raydium_joy_process_event(struct js_event e)
{
    switch(e.type)
    {
        case JS_EVENT_BUTTON:
		if(e.number<RAYDIUM_JOY_MAX_BUTTONS)
		{
		    if(e.value==1)
		    {
			raydium_joy_click=e.number+1;
			raydium_joy_button[e.number]=e.value;
			#ifdef joy_debug
			raydium_log("Button %d pushed",e.number);
			#endif
		    }
		    else
		    {
			// release
			raydium_joy_button[e.number]=e.value;
		    }
		}
        	break;
        case JS_EVENT_AXIS:
		if(e.number<RAYDIUM_JOY_MAX_AXIS)
		{
		    #ifdef joy_debug
		    raydium_log("Axis Moved: %i",e.value);
		    #endif
		    raydium_joy_axis[e.number]=e.value/(float)32767;
		    //here we invert values from the y axis: we want 
		    //1 for up and -1 for down
			if(e.value<0)
			{
		          #ifdef joy_debug
			   if(e.number==1)raydium_log("Up");
			   if(e.number==0)raydium_log("Left");
			  #endif
			  if(e.number==2)
			  {
			   raydium_joy_z=e.value/(float)32767*-1;
			  }
			  if(e.number==1)
			  {
			   raydium_joy_y=e.value/(float)32767*-1;
			  }
			  if(e.number==0)
			  {
			   raydium_joy_x=e.value/(float)32767;
			  }
			}
			if(e.value>0)
			{
			  #ifdef joy_debug
			   if(e.number==1)raydium_log("Down");
			   if(e.number==0)raydium_log("Right");
			  #endif
			  if(e.number==2)
			  {
			   raydium_joy_z=e.value/(float)32767*-1;
			  }
			  if(e.number==1)
			  {
			   raydium_joy_y=e.value/(float)32767*-1;
			  }
			  if(e.number==0)
			  {
			   raydium_joy_x=e.value/(float)32767;
			  }
			}
			if(e.value==0)
			{
			  if(e.number==1)
			  {
			   raydium_joy_y=0.0;
			  }
			  if(e.number==0)
			  {
			   raydium_joy_x=0.0;
			  }
		 	}
		}
		break;
        case JS_EVENT_INIT:
		    //raydium_log("Joystick returned its initial state\n");
        	    break;

    }
    return(e.type);
}
#endif

void raydium_joy_callback(void)
{
#ifndef WIN32
 struct js_event e;			//structure for storing an event
 
	if(!raydium_joy) { raydium_joy_init_vars(); return; }

	raydium_joy_click=0;	
	while (read (raydium_joy, &e, sizeof(struct js_event)) > 0)
	{
            raydium_joy_process_event (e);
	    //raydium_log("joy_DEBUG number:%d, value:%d",e.number,e.value);
        }
#else
raydium_joy_init_vars();
#endif
}

void raydium_joy_ff_autocenter(int perc)
{
#ifndef WIN32
struct input_event ie;

if(raydium_joy_event_handle<0) return;

ie.type = EV_FF;
ie.code = FF_AUTOCENTER;
ie.value = 0xFFFFUL * perc / 100;

if (write(raydium_joy_event_handle, &ie, sizeof(ie)) == -1)
        perror("set auto-center");

#endif
}


void raydium_joy_init(void)
{
 int ret;					//test var for ioctls
 char name[128];				//driver String (and temp things)

int autocenter=5;         /* default value. between 0 and 100 */

	raydium_joy_init_vars();
#ifndef WIN32
	if(raydium_init_cli_option_default("joydev",name,"/dev/js0"))	    
	    raydium_joy=open(name,O_RDONLY|O_NONBLOCK);
	else
	{
	raydium_joy=open("/dev/js0",O_RDONLY|O_NONBLOCK);
	if(raydium_joy==-1)
	    raydium_joy=open("/dev/input/js0",O_RDONLY|O_NONBLOCK);
	}

	raydium_init_cli_option_default("evdev",name,"/dev/input/event0");
	    
	raydium_joy_event_handle = open(name, O_RDWR);
	if(raydium_joy_event_handle==-1) 
	  raydium_log("%s: cannot open (rw), no Force Feedback.",name);
	last_event=clock();

	raydium_joy_ff_autocenter(autocenter);
	
	if(raydium_joy==-1)
	{
		raydium_log("joy: FAILED (cannot open /dev/js0 and /dev/input/js0)");
		raydium_joy=0;
	}
	else
	{
		raydium_log("joy: OK (found)");
	}

	if(raydium_joy)
	{
		ret=ioctl (raydium_joy,JSIOCGNAME(sizeof(name)),name);
		if(ret==-1)
		{
	    		raydium_log("Error reading joystick driver's signature");
	    		strncpy(name,"Unknown",sizeof(name));
		}
		else
		{
		    	raydium_log("Joystick driver's signature: %s",name);
		}

		ret=ioctl (raydium_joy,JSIOCGAXES,&raydium_joy_n_axes);
		if(ret==-1)
		{
	    		raydium_log("Error reading number of axes");
		}
		else
		{
	    		raydium_log("This joystick has %d axes",raydium_joy_n_axes);
		}

		ret=ioctl (raydium_joy,JSIOCGBUTTONS,&raydium_joy_n_buttons);
		if(ret==-1)
		{
	    		raydium_log("Error reading number of buttons");
		}
		else
		{
	    		raydium_log("This joystick has %d buttons",raydium_joy_n_buttons);
		}
	}
#else
    raydium_log("joy: FAILED\n\t No Joy support under Win32 yet");
#endif
}

void raydium_joy_close(void)
{
	if(raydium_joy)	close(raydium_joy);
}

void raydium_joy_ff(void)
{
//memset(&effect,0,sizeof(struct ff_effect));
// effect.type=FF_CONSTANT;
// effect.u.constant.level=0xEFF0;
// effect.u.constant.direction=0x2000;
// effect.replay.length=65535;
// effect.u.constant.shape.attack_length=40000;
// effect.u.constant.shape.attack_level=0x8888;
// effect.u.constant.shape.fade_length=20000;
// effect.u.constant.shape.fade_level=0x2222;
/*	effect.type = FF_SPRING;
	effect.u.interactive.direction=0x4000;
	effect.u.interactive.right_saturation = 32767;
	effect.u.interactive.left_saturation = 32767;
	effect.u.interactive.right_coeff = 32767;
	effect.u.interactive.left_coeff = 32767;
	effect.u.interactive.deadband = 0x0;
	effect.u.interactive.center = 0x0;*/
//    effect.type = FF_PERIODIC;
/*
#define FF_SQUARE	0x58
#define FF_TRIANGLE	0x59
#define FF_SINE		0x5a
#define FF_SAW_UP	0x5b
#define FF_SAW_DOWN	0x5c
#define FF_CUSTOM	0x5d
*/
/*    effect.u.periodic.waveform=FF_TRIANGLE;
    effect.u.periodic.period=50;
    effect.u.periodic.magnitude=16000;
    effect.u.periodic.direction=0x4000;*/
    
	




// raydium_log("upload effect valeur=%d", ioctl(fd, EVIOCSFF, &effect));
// perror("merde:");

// play.type = EV_FF;
// play.code = effect.id;
//raydium_log("effect_id=%d",effect.id);
// play.value = 10000;
// write(fd, (const void*) &play, sizeof(play));

/* stop.type = EV_FF;
 stop.code = effect.id;
 stop.value = 0;

 write(fd, (const void*) &play, sizeof(stop));*/
//raydium_log("remove effectvaleur=%d", ioctl(fd, EVIOCRMFF, effect.id));
}

void raydium_joy_ff_tremble_set(GLfloat period, GLfloat force)
{
#ifndef WIN32
struct input_event play;
struct input_event stop;

if (clock() < last_event + CLOCKS_PER_SEC/10)
return;


if(effect_tremble_state)
{
 stop.type = EV_FF;
 stop.code = effect_tremble.id;
 stop.value = 0;
 write(raydium_joy_event_handle, (const void*) &stop, sizeof(stop));
// perror("ff: stop tremble");
 ioctl(raydium_joy_event_handle, EVIOCRMFF, effect_tremble.id);
// perror("ff: free tremble");
}

memset(&effect_tremble,0,sizeof(struct ff_effect));
effect_tremble.type = FF_PERIODIC;
effect_tremble.u.periodic.waveform=FF_TRIANGLE;
effect_tremble.u.periodic.period=period;
effect_tremble.u.periodic.magnitude=force;
//effect_tremble.u.periodic.direction=0x4000; // NEED SOME WORK HERE !!
effect_tremble.replay.length=65535;
ioctl(raydium_joy_event_handle, EVIOCSFF, &effect_tremble);
//perror("ff: upload tremble");
play.type = EV_FF;
play.code = effect_tremble.id;
play.value = 1;
write(raydium_joy_event_handle, (const void*) &play, sizeof(play));
//perror("ff: play tremble");
effect_tremble_state=1;
last_event=clock();
//printf("ff event refreshed\n");
#endif
}

