/*
    Raydium - CQFD Corp.
    http://raydium.org/
    License: GPL - GNU General Public License, see "gpl.txt" file.
*/

#ifndef DONT_INCLUDE_HEADERS
#include "index.h"
#else
#include "headers/key.h"
#endif


// proto
void raydium_console_event(void);
void raydium_console_line_add(char *format, ...);
void raydium_console_exec_last_command(void);
void raydium_console_complete(char *str);
void raydium_console_history_previous(void);
void raydium_console_history_next(void);


void raydium_key_normal_callback(GLuint key, int x, int y)
{
int i;
key%=65536;

// key below esc :
// 178 (ex: fr), 176 (ex: us), 186 (ex: spa)
if(key==178 || key==176 || key==186) raydium_console_event();
if(key==126) raydium_capture_frame_auto(); // glut@w32 won't return this key...

if(raydium_console_pos && ( (key>=32 && key<127) 
		       || key==8 
		       || key==9 
		       || key==13) )
 {
 i=strlen(raydium_console_get_string);
// printf("%s\n",raydium_console_get_string);

 if(key==13)
  {
  if(!i) return;
  raydium_console_get_string[i]=key; // lag from last frame...
  raydium_console_get_string[i+1]=0;
  // there is a bug: raydium_console_get_string '\n' is cleaned, but
  // after being copied into raydium_console_get_string_last.
  // Must take some time to clean all this (key, console, test4 and modler)
  strcpy(raydium_console_get_string_last,raydium_console_get_string);
  raydium_console_get_string[i]=0;
  raydium_console_line_add("%s",raydium_console_get_string);
  raydium_console_get_string[0]=0;
  raydium_console_exec_last_command();
  return;
  }
 
 if(key==8) // delete last char
    { 
    if(i>0) i--; 
    key=0;
    }

 if(key==9) // completion
    {
    raydium_console_complete(raydium_console_get_string);
    return;
    }

 if(i<RAYDIUM_MAX_NAME_LEN-3)
  {
  raydium_console_get_string[i]=key;
  raydium_console_get_string[i+1]=0; 
  raydium_console_cursor_blink=1;
  }
 }
else
 {
 raydium_key_last=key+1000;
 if(raydium_key_trace)
    raydium_log("normal key %i pressed",key+1000);
 }

}

void raydium_key_special_callback(GLuint key, int x, int y)
{

if(raydium_console_pos && key==GLUT_KEY_UP)
    {
    raydium_console_history_previous();
    return;
    }

if(raydium_console_pos && key==GLUT_KEY_DOWN)
    {
    raydium_console_history_next();
    return;
    }

key%=65536;
raydium_key[key]=2;
raydium_key_last=key;
if(raydium_key_trace)
    raydium_log("special key %i down (normal key updated too)",key);
}

void raydium_key_special_up_callback(GLuint key, int x, int y)
{
key%=65536;
raydium_key[key]=0;
if(raydium_key_trace)
    raydium_log("special key %i up",key);
}

// moslty used for php
int raydium_key_pressed(GLuint key)
{
return raydium_key[key];
}
