/*
    Raydium - CQFD Corp.
    http://raydium.org/
    License: GPL - GNU General Public License, see "gpl.txt" file.
*/

#ifndef DONT_INCLUDE_HEADERS
#include "index.h"
#else
#include "headers/console.h"
#endif 

// proto
void raydium_console_exec_last_command(void);
void raydium_console_history_add(char *str);


void raydium_console_init(void)
{
int i;
FILE *fp;
char line[RAYDIUM_MAX_NAME_LEN];

raydium_console_pos=0;
raydium_console_inc=0;
raydium_console_config_max=50;
raydium_console_config_speed=3;
raydium_init_cli_option_default("consoletexture",raydium_console_config_texture,"rgb(0.2,0.2,0.2)");
raydium_init_cli_option_default("consolefont",raydium_console_config_font,"font2.tga");
raydium_console_line_last=-1;
raydium_console_cursor_blink=0;
for(i=0;i<RAYDIUM_CONSOLE_MAX_LINES;i++)
 raydium_console_lines[i][0]=0;
raydium_console_get_string[0]=0;
raydium_console_get_string_last[0]=0;

raydium_init_cli_option_default("history",raydium_console_history_filename,
				raydium_file_home_path("raydium_history"));

for(i=0;i<RAYDIUM_CONSOLE_MAX_HISTORY;i++)
    raydium_console_history[i][0]=0;
raydium_console_history_index_current=-1;
raydium_console_history_index=0;

fp=fopen(raydium_console_history_filename,"rt");
if(fp)
    {
    while(fgets(line,RAYDIUM_MAX_NAME_LEN,fp))
	{
	line[strlen(line)-1]=0;
	raydium_console_history_add(line);
        }
    fclose(fp);
    }
}


void raydium_console_history_save(void)
{
int i;
FILE *fp;
char last[RAYDIUM_MAX_NAME_LEN];

last[0]=0;

fp=fopen(raydium_console_history_filename,"wt");
if(!fp)
    {
    raydium_log("console: error: cannot save history file ('%s')",raydium_console_history_filename);
    return;
    }

for(i=0;i<raydium_console_history_index;i++)
    if(strcmp(raydium_console_history[i],last))
	{
	strcpy(last,raydium_console_history[i]);
	fprintf(fp,"%s\n",raydium_console_history[i]);
	}
fclose(fp);
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// Now DISABLED, 
// use raydium_console_gets_callback fptr instead.
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
int raydium_console_gets(char *where)
{
raydium_log("ERROR: raydium_console_gets is now DISABLED ! See API doc.");
// always here for compat' only, do not use.
/*if(strlen(raydium_console_get_string_last))
 {
 strcpy(where,raydium_console_get_string_last);
 raydium_console_get_string_last[0]=0;
 return 1;
 } else*/ return 0;
}



void raydium_console_history_previous(void)
{
raydium_console_history_index_current--;

if(raydium_console_history_index_current<0) 
    {
    raydium_console_history_index_current=0;
    return;
    }
strcpy(raydium_console_get_string,raydium_console_history[raydium_console_history_index_current]);
}

void raydium_console_history_next(void)
{
raydium_console_history_index_current++;

if(raydium_console_history_index_current>=raydium_console_history_index)
    {
    raydium_console_history_index_current=raydium_console_history_index;
    strcpy(raydium_console_get_string,"");
    return;
    }
strcpy(raydium_console_get_string,raydium_console_history[raydium_console_history_index_current]);
}


void raydium_console_history_add(char *str)
{
int i;


if(raydium_console_history_index==RAYDIUM_CONSOLE_MAX_HISTORY)
    {
    raydium_console_history_index_current=raydium_console_history_index;
    /*
    printf("-----\n");
    for(i=0;i<RAYDIUM_CONSOLE_MAX_HISTORY;i++)
	printf("%s\n",raydium_console_history[i]);
    */

    for(i=0;i<RAYDIUM_CONSOLE_MAX_HISTORY-1;i++)
	strcpy(raydium_console_history[i],raydium_console_history[i+1]);

    strcpy(raydium_console_history[RAYDIUM_CONSOLE_MAX_HISTORY-1],str);
    /*
    printf("-----\n");
    for(i=0;i<RAYDIUM_CONSOLE_MAX_HISTORY;i++)
	printf("%s\n",raydium_console_history[i]);
    */
    return;
    }
raydium_console_history_index_current=raydium_console_history_index+1;
strcpy(raydium_console_history[raydium_console_history_index],str);
raydium_console_history_index++;
}


void raydium_console_exec_script(char *file)
{
FILE *fp;
char command[RAYDIUM_MAX_NAME_LEN];

if(!raydium_console_gets_callback)
    {
    raydium_log("ERROR: console: script: no command callback is defined, aborded.");
    return;
    }

fp=raydium_file_fopen(file,"rt");
if(!fp)
    {
    raydium_log("ERROR: console: script: file not found \"%s\"",file);
    return;
    }
while(fgets(command,RAYDIUM_MAX_NAME_LEN,fp))
    {
    strcpy(raydium_console_get_string_last,command);
    raydium_console_exec_last_command();
    }

fclose(fp);
}


void raydium_console_exec_last_command(void)
{
int treated=0;
void (*f)(char *);
char temp[RAYDIUM_MAX_NAME_LEN];

raydium_console_get_string_last[strlen(raydium_console_get_string_last)-1]=0;
raydium_console_history_add(raydium_console_get_string_last);

if(raydium_console_get_string_last[0]=='!')
    {
    treated=1;
    raydium_console_exec_script(raydium_console_get_string_last+1);
    }

if(raydium_console_get_string_last[0]=='>')
    {
    treated=1;
#ifdef PHP_SUPPORT
    raydium_php_exec(raydium_console_get_string_last+1);
#else
    raydium_log("ERROR: No PHP support compiled");
#endif
    }

if(raydium_console_get_string_last[0]!='/' && !treated)
{
#ifdef PHP_SUPPORT
#define TEMPFILE raydium_file_home_path("temp.delme.php")
FILE *fp;
fp=fopen(TEMPFILE,"wt");
if(!fp) { raydium_log("console: php call: cannot create %s temporary file",TEMPFILE); return; }
fprintf(fp,"<? %s; ?>",raydium_console_get_string_last);
fclose(fp);
raydium_php_exec(TEMPFILE);
treated=1; // all is sended to PHP for now
#endif
}


if(!treated && raydium_console_gets_callback)
    {
    f=raydium_console_gets_callback;
    strcpy(temp,raydium_console_get_string_last+1);
    f(temp);
    }

}

void raydium_console_line_add(char *format, ...)
{
va_list argptr;
int retlen;

raydium_console_line_last++;
if(raydium_console_line_last>=RAYDIUM_CONSOLE_MAX_LINES)
   raydium_console_line_last=0;

va_start(argptr,format);
retlen = vsnprintf(raydium_console_lines[raydium_console_line_last],RAYDIUM_MAX_NAME_LEN - 1,format,argptr);
va_end(argptr);

if(retlen < 0) retlen = 0;
raydium_console_lines[raydium_console_line_last][retlen] = '\0';
}

int raydium_console_history_read(char **hist)
{
int i,start,cpt=0;

cpt=0;
start=raydium_console_line_last;

//for(i=RAYDIUM_CONSOLE_MAX_LINES-1;i>raydium_console_line_last;i--)
for(i=raydium_console_line_last+1;i<RAYDIUM_CONSOLE_MAX_LINES;i++)
 {
 hist[cpt]=raydium_console_lines[i];
 cpt++;
 }

//for(i=start;i>=0;i--)
for(i=0;i<=start;i++)
 {
 hist[cpt]=raydium_console_lines[i];
 cpt++;
 }


return cpt++;
}

void raydium_console_event(void)
{
#ifdef PHP_SUPPORT
static signed char first=1;

if(first)
    {
    raydium_texture_find_by_name(raydium_console_config_texture); // cache
//    raydium_texture_find_by_name(RAYDIUM_CONSOLE_FONT_SIZE); // cache
    raydium_log("    --- This console provides a PHP parser and text completion ---");
    first=0;
    }
#endif

if(raydium_console_inc!=0) raydium_console_inc*=-1;
else {
	if(raydium_console_pos==0) raydium_console_inc=raydium_console_config_speed;
	else raydium_console_inc=-raydium_console_config_speed;
     }
}

void raydium_console_draw(void)
{
GLfloat y,off;
int i,cpt;
int texsave;
char *hist[RAYDIUM_CONSOLE_MAX_LINES];

raydium_console_pos+=raydium_console_inc*(raydium_frame_time*100);

if(raydium_console_pos<0)
 {
 raydium_console_pos=0;
 raydium_console_inc=0;
 }
 
if(raydium_console_pos>raydium_console_config_max)
 {
 raydium_console_pos=raydium_console_config_max;
 raydium_console_inc=0;
 }

if(!raydium_console_pos) return;

raydium_osd_start();
texsave=raydium_texture_current_main;
raydium_texture_current_set_name(raydium_console_config_texture);
raydium_rendering_internal_prepare_texture_render(raydium_texture_current_main);

off=raydium_console_config_max-raydium_console_pos;
glBegin(GL_QUADS);
glTexCoord2f(0,0);
glVertex3f(0,100-raydium_console_pos,0);
glTexCoord2f(1,0);
glVertex3f(100,100-raydium_console_pos,0);
glTexCoord2f(1,1);
glVertex3f(100,100+off,0);
glTexCoord2f(0,1);
glVertex3f(0,100+off,0);
glEnd();

raydium_osd_stop();

y=100-raydium_console_pos+(RAYDIUM_CONSOLE_FONT_SIZE/6.f);

raydium_osd_color_ega('f');
raydium_console_cursor_blink+=(raydium_frame_time*2);
raydium_osd_printf(1,y,RAYDIUM_CONSOLE_FONT_SIZE,RAYDIUM_CONSOLE_FONT_SPACER,raydium_console_config_font,"%s%c",raydium_console_get_string,( (((int)raydium_console_cursor_blink)%2)?'_':' '));
y+=(RAYDIUM_CONSOLE_FONT_SIZE/6.f);

cpt=raydium_console_history_read(hist);

for(i=cpt-1;i>=0;i--)
 {
 raydium_osd_color_ega('f');
 raydium_osd_printf(1,y,RAYDIUM_CONSOLE_FONT_SIZE,RAYDIUM_CONSOLE_FONT_SPACER,raydium_console_config_font,"%s",hist[i]);
 y+=(RAYDIUM_CONSOLE_FONT_SIZE/6.f);
 }

//raydium_texture_current_set(texsave); 
//raydium_rendering_internal_prepare_texture_render(raydium_texture_current);
}


// is alpha, num or '_' ?
int raydium_console_internal_isalphanumuscore(char c)
{
if(c=='_' || isalnum(c)) return 1;
return 0;
}


// "str": RAYDIUM_MAX_NAME_LEN only

void raydium_console_complete(char *str)
{
#ifdef PHP_SUPPORT
char candidates[RAYDIUM_CONSOLE_MAX_COMPLETION][RAYDIUM_MAX_NAME_LEN];
char candidates_type[RAYDIUM_CONSOLE_MAX_COMPLETION];
int n_candidates=0;
char word[RAYDIUM_MAX_NAME_LEN];
char candidate[RAYDIUM_MAX_NAME_LEN];
int word_offset;
int i,j;
int len;
int candidate_min_len;
char c;

// 0 - find last word, and store start offset
len=strlen(str);
//if(len==0) return;
for(i=(len-1);i>=0;i--)
    {
    if(!raydium_console_internal_isalphanumuscore(str[i]))
	{
	i++;
	break;
	}
    }
if(i==-1) i=0; // first word of sentence

//if(i<0 || !raydium_console_internal_isalphanumuscore(str[i])) 
    //return; // empty word

word_offset=i;
strcpy(word,str+i);
len=strlen(word);

// 1 - build candidates list
for(i=0;i<raydium_register_variable_index;i++)
    {
    strcpy(candidate,raydium_register_variable_name[i]);
    candidate[len]=0;
    if(!strcmp(candidate,word))
	{
	candidates_type[n_candidates]=0; // 0 = variable
	strcpy(candidates[n_candidates++],raydium_register_variable_name[i]);
	}
    if(n_candidates==RAYDIUM_CONSOLE_MAX_COMPLETION) break;
    }

if(n_candidates<RAYDIUM_CONSOLE_MAX_COMPLETION)
for(i=0;i<raydium_register_function_index;i++)
    {
    strcpy(candidate,raydium_register_function_list[i].fname);
    candidate[len]=0;
    if(!strcmp(candidate,word))
	{
	candidates_type[n_candidates]=1; // 1 = function
	strcpy(candidates[n_candidates++],raydium_register_function_list[i].fname);
	}
    if(n_candidates==RAYDIUM_CONSOLE_MAX_COMPLETION) break;
    }

// 2 - no candidate ? only one ?
if(!n_candidates)
    return;

if(n_candidates==1)
    {
    str[word_offset]=0;
    if(strlen(str)+strlen(candidates[0]) >= (RAYDIUM_MAX_NAME_LEN-1))
	return;	
    strcat(str,candidates[0]);
    if(candidates_type[0])
	strcat(str,"(");
    else
	strcat(str," ");
    return;
    }
    
// 3 - more than one candidate : display candidates and find the common root
raydium_console_line_add("> %s",str);

// display
for(i=0;i<n_candidates;i++)
    {
    if(candidates_type[i])
	raydium_console_line_add("%s()",candidates[i]);
    else
	raydium_console_line_add("$%s",candidates[i]);
    }
if(n_candidates==RAYDIUM_CONSOLE_MAX_COMPLETION)
    raydium_console_line_add("..."); // limited results

// root: find shortest candidate
candidate_min_len=RAYDIUM_MAX_NAME_LEN+1;
for(i=0;i<n_candidates;i++)
  if(strlen(candidates[i])<candidate_min_len)
    candidate_min_len=strlen(candidates[i]);

// root: find common chars
for(i=strlen(word);i<=candidate_min_len;i++) // '\0' must be tested, too
    {
    c=candidates[0][i];
    for(j=1;j<n_candidates;j++)
      if(c!=candidates[j][i]) // last equiv
	{
	candidates[0][i]=0;
	strcpy(candidate,candidates[0]);
	str[word_offset]=0;
	if(strlen(str)+strlen(candidate) >= (RAYDIUM_MAX_NAME_LEN-1))
	    return;
	strcat(str,candidate);
	return;
	}
    }
#endif
}
