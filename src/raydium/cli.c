/*
    Raydium - CQFD Corp.
    http://raydium.org/
    License: GPL - GNU General Public License, see "gpl.txt" file.
*/

#ifndef DONT_INCLUDE_HEADERS
#include "index.h"
#else
#include "headers/cli.h"

#endif 

char *raydium_version(void);

int raydium_init_cli_option(char *option, char *value)
{
int i;
int found;
char full_option[RAYDIUM_MAX_NAME_LEN];
char head[3];

strcpy(full_option  ,"--");
strcpy(full_option+2,option);

found=0;

for(i=1;i<raydium_init_argc;i++)
    if(!strcasecmp(full_option,raydium_init_argv[i]))
	found=i;

if(!found) return 0; 	// option not found

if((found+1) >= raydium_init_argc)
    {
    if(value) strcpy(value,"");
    return 1;	// found, no value (last option of cli)
    }

strncpy(head,raydium_init_argv[found+1],3);
head[2]=0;

if(!strcasecmp(head,"--"))
    {
    if(value) strcpy(value,"");
    return 1;	// found, no value (value starts with -- : is a option)
    }

if(value) strcpy(value,raydium_init_argv[found+1]);
return 1;
}


int raydium_init_cli_option_default(char *option, char *value, char *default_value)
{
if(!raydium_init_cli_option(option,value))
    strcpy(value,default_value);
return 1;
}

void raydium_init_internal_homedir_find(char *app_name)
{
FILE *fp;
char *str;
char *str2;
        
raydium_homedir[0]=0;
#ifndef WIN32
str=getenv("HOME");
str2="";
if(!str) // strange session :/
    {
    raydium_log("ERROR ! Unable to find HOME variable !");
    exit(100);
    }
#else
str=getenv("HOMEDRIVE");
str2=getenv("HOMEPATH");
if(!str || !str2)
    {
    raydium_log("ERROR ! Unable to find HOMEDRIVE and HOMEPATH variables !");
    exit(100);
    }
#endif

// right, 'str' is now the absolute home dir of user, let's build Raydium's one
// if not already given by user
if(!raydium_init_cli_option("home",raydium_homedir))
    {
    sprintf(raydium_homedir,"%s%s/.%s",str,str2,app_name);
    }

// is writable ?
fp=fopen(raydium_file_home_path("flag"),"wt");
if(!fp)
    {
    if(mkdir(raydium_homedir,S_IRUSR|S_IWUSR|S_IRWXU)<0)
	{
	raydium_log("ERROR ! Unable to create home dir: '%s'",raydium_homedir);
	exit(101);
	}
    }
else
    {
    fclose(fp);
    unlink(raydium_file_home_path("flag"));
    }

raydium_log("using '%s' as home dir",raydium_homedir);
}


#ifndef RAYDLL
void raydium_init_args_name(int argc, char **argv, char *app_name)
#else
void raydium_init_args_name_hack(int argc, char **argv, char *app_name)
#endif
{
int i;
char logfile[RAYDIUM_MAX_NAME_LEN];

raydium_init_argc=argc;
raydium_init_argv=malloc(argc*sizeof(char *));

for(i=0;i<argc;i++)
    {
    raydium_init_argv[i]=malloc(strlen(argv[i])+1);
    strcpy(raydium_init_argv[i],argv[i]);
    }
raydium_log("Raydium 3D Game Engine");

if(raydium_init_cli_option("logfile",logfile))
    {
    raydium_log_file=fopen(logfile,"wt");
    if(!raydium_log_file) raydium_log("init: Warning: cannot open logfile (%s) with rw mode",logfile);
    }
else raydium_log_file=NULL;

raydium_log("version %s",raydium_version());
raydium_log("command line args: OK");

#ifndef RAYDIUM_NETWORK_ONLY
raydium_file_dirname(raydium_init_wd,raydium_init_argv[0]);
if(!chdir(raydium_init_wd))
    raydium_log("chdir to '%s': OK",raydium_init_wd);
else
    perror("chdir");    
#endif

// Find user's home directory
raydium_init_internal_homedir_find(app_name);
raydium_atexit_init();
}

#ifndef RAYDLL
void raydium_init_args (int argc, char **argv)
{
raydium_init_args_name(argc,argv,RAYDIUM_APP_SHORTNAME);
}
#else
void raydium_init_args_hack (int argc, char **argv)
{
raydium_init_args_name_hack(argc,argv,RAYDIUM_APP_SHORTNAME);
}
#endif
