/*
    Raydium - CQFD Corp.
    http://raydium.org/
    License: GPL - GNU General Public License, see "gpl.txt" file.
*/

// -------------
// doc
// mania_drive ! (current "home" functions, and network [net tracks & lan])

#ifndef DONT_INCLUDE_HEADERS
#include "index.h"
#else
#include "headers/path.h"
#endif 

#include "path.h"



int raydium_path_find_free(void)
{
int i;

for(i=0;i<RAYDIUM_MAX_PATHS;i++)
    if(!raydium_path_paths[i].state)
	return i;
return -1;
}


signed char raydium_path_ext(char *dir, char *ext)
{
int id;

id=raydium_path_find_free();
if(id<0)
    {
    raydium_log("path: ERROR: no more free path slot");
    return 0;
    }

if(dir==NULL || strlen(dir)==0)
    {
    raydium_log("path: ERROR: directory name is not valid");
    return 0;
    }

if(ext==NULL || strlen(ext)==0)
    {
    raydium_log("path: ERROR: extension is not valid for this path");
    return 0;
    }

if(strlen(ext)>=2 && ext[0]=='*' && ext[1]=='.')
    ext+=2;

strcpy(raydium_path_paths[id].ext,ext);
strcpy(raydium_path_paths[id].path,dir);

if(dir[strlen(dir)-1]=='/')
    raydium_path_paths[id].path[strlen(dir)-1]=0;

raydium_path_paths[id].mode=RAYDIUM_PATH_MODE_READ;
raydium_path_paths[id].state=1;
return 1;
}


signed char raydium_path_add(char *dir)
{
int id;

id=raydium_path_find_free();
if(id<0)
    {
    raydium_log("path: ERROR: no more free path slot");
    return 0;
    }

if(dir==NULL || strlen(dir)==0)
    {
    raydium_log("path: ERROR: directory name is not valid");
    return 0;
    }

if(strchr(dir,'*'))
    {
    char path[RAYDIUM_MAX_DIR_LEN];
    char ext[RAYDIUM_MAX_DIR_LEN];

    raydium_file_dirname(path,dir);
    raydium_file_basename(ext,dir);
    return raydium_path_ext(path,ext);
    }

strcpy(raydium_path_paths[id].path,dir);
if(dir[strlen(dir)-1]=='/')
    raydium_path_paths[id].path[strlen(dir)-1]=0;

raydium_path_paths[id].mode=RAYDIUM_PATH_MODE_READ;
raydium_path_paths[id].state=1;
raydium_path_paths[id].ext[0]=0;
return 1;
}

signed char raydium_path_write(char *dir)
{
if(!raydium_file_directory_writable(dir))
    {
    raydium_log("path: ERROR: '%s' is not a writable directory !",dir);
    return 0;
    }

strcpy(raydium_path_write_current,dir);

if(dir[strlen(dir)-1]=='/')
    raydium_path_write_current[strlen(dir)-1]=0;

return 1;
}

signed char raydium_path_string_from(char *str)
{
// cut the string on ':'
char part[RAYDIUM_MAX_DIR_LEN];
char *c;
int end;
signed char last=0;

if(!str || strlen(str)==0)
    return 0;

raydium_path_reset();

do
 {
    if((c=strchr(str,':')))
	end=c-str;
    else
	{
	end=strlen(str);
	last=1;
	}

    strncpy(part,str,end);
    part[end]=0;
    raydium_path_add(part);
    str+=(end+1);
 } while(!last);

return 1;
}

int raydium_path_string_to(char *out)
{
int i;

out[0]=0;
for(i=0;i<RAYDIUM_MAX_PATHS;i++)
    if( raydium_path_paths[i].state && 
	raydium_path_paths[i].mode==RAYDIUM_PATH_MODE_READ )
	    {
	    strcat(out,raydium_path_paths[i].path);
	    if(strlen(raydium_path_paths[i].ext))
		{
		strcat(out,"/*.");
		strcat(out,raydium_path_paths[i].ext);
		}
	    
	    strcat(out,":");
	    }

if(strlen(out))
    out[strlen(out)-1]=0; // trim last ':'

return strlen(out);
}

/*
signed char raydium_path_delete(char *path)
{
}
*/

void raydium_path_resolv(char *in, char *out, char mode)
{

strcpy(out,in); // this is the default behavior

if(strchr(in,'/'))
    return;

if(mode=='r')
    {
    char path[RAYDIUM_MAX_DIR_LEN];
    char ext[RAYDIUM_MAX_NAME_LEN];
    int i;

    if(raydium_file_readable(in))
	return;

    // ok ... file does not exists, search paths
    raydium_file_ext(ext,in);
    for(i=0;i<RAYDIUM_MAX_PATHS;i++)
	if( raydium_path_paths[i].state && 
	    raydium_path_paths[i].mode==RAYDIUM_PATH_MODE_READ )
	    {
	    if(raydium_path_paths[i].ext[0] &&
	       strcmp(raydium_path_paths[i].ext,ext))
	        continue; // not the good extension

	    sprintf(path,"%s/%s",raydium_path_paths[i].path,in);
	    if(raydium_file_readable(path))
		{
		strcpy(out,path);
		return;
		}
	    }

    // ok, if we're here, the file does not exists at all, let's
    // prepare R3S job:
    mode='w';
    }

if(mode=='w')
    {
    if(raydium_file_directory_writable("."))
	return;

    sprintf(out,"%s/%s",raydium_path_write_current,in);
    }

}

void raydium_path_dump(void)
{
char path[1024];
raydium_path_string_to(path);
raydium_log("path: read: \"%s\"",path);
raydium_log("path: write: \"%s\"",raydium_path_write_current);
}

void raydium_path_reset(void)
{
int i;

for(i=0;i<RAYDIUM_MAX_PATHS;i++)
    raydium_path_paths[i].state=0;
}

void raydium_path_init(void)
{
char path[1024];
signed char ok;

raydium_path_reset();
raydium_atexit(raydium_path_dump);

if(raydium_init_cli_option("path",path))
    raydium_path_string_from(path);

ok=0;
if(raydium_init_cli_option("write-path",path))
    ok=raydium_path_write(path);

if(!ok)
    {
    strcpy(path,raydium_file_home_path("data"));
    if(!raydium_file_directory_writable(path))
	{
	mkdir(path,0755);
	if(!raydium_file_directory_writable(path))
	    {
	    ok=0;
	    raydium_log("ERROR: path: cannot create '%s'",path);
	    }
	else
	    {
	    ok=1;
	    raydium_log("path: created writable path '%s'",path);
	    }
	}
    else ok=1;
    }

if(ok)
    {
    raydium_path_write(path);
    raydium_path_add(path);    
    raydium_log("path: OK");
    }
else
    raydium_log("ERROR: path: unable to find a writable path. Try using --write-path");
}

