/*
    Raydium - CQFD Corp.
    http://raydium.org/
    License: GPL - GNU General Public License, see "gpl.txt" file.
*/

#ifndef DONT_INCLUDE_HEADERS
#include "index.h"
#else
#include "headers/file.h"
#endif 

// proto
void raydium_path_resolv(char *in, char *out, char mode);


// far better than glibc's 'dirname' (and portable)
void raydium_file_dirname(char *dest,char *from)
{
char *c;
int n;

c=strrchr(from,'/'); // Unix
if(!c)
      c=strrchr(from,'\\'); // win32

if(!c)
    {
    strcpy(dest,"./");
    return;
    }
n=c-from;
memcpy(dest,from,n+1);
dest[n+1]=0;
}

void raydium_file_basename(char *dest,char *from)
{
char *c;
int n;

c=strrchr(from,'/'); // Unix
if(!c)
    c=strrchr(from,'\\'); // win32
      
if(!c)
    {
    strcpy(dest,from);
    return;
    }
n=(c-from+1);

if(n==strlen(from))
    {
    dest[0]=0;
    return;
    }

strcpy(dest,from+n);
}

void raydium_file_ext(char *dest, char *from)
{
char name[RAYDIUM_MAX_DIR_LEN];
char *c;

dest[0]=0;

raydium_file_basename(name,from);
if( (c=strrchr(name,'.')) )
    if(c[1]!=0)
	strcpy(dest,c+1);
}


signed char raydium_file_directory_writable(char *path)
{
char file[RAYDIUM_MAX_NAME_LEN];
FILE *fp;

sprintf(file,"%s/RAYDIUM-WRITE-TEST.delme",path);
fp=fopen(file,"wb");
if(!fp)
    return 0;

fclose(fp);
unlink(file);
return 1;
}

signed char raydium_file_readable(char *filename)
{
FILE *fp;
fp=fopen(filename,"r");
if(!fp)
    return 0;
fclose(fp);
return 1;
}

void raydium_file_log_fopen_display(void)
{
int i;

raydium_log("List of all opended files:");

for(i=0;i<raydium_file_log_fopen_index;i++)
    raydium_log("%s",raydium_file_log_fopen[i]);

}

FILE *raydium_file_fopen(char *file, char *mode)
{
FILE *fp;
int i;
char found=0;
char file2[RAYDIUM_MAX_DIR_LEN];

if(!file || !strlen(file))
    return NULL;

for(i=0;i<raydium_file_log_fopen_index;i++)
    if(!strcmp(raydium_file_log_fopen[i],file))
	{
	found=1;
	break;
	}

if(!found) strcpy(raydium_file_log_fopen[raydium_file_log_fopen_index++],file);

// use paths
raydium_path_resolv(file,file2,mode[0]);

// local mode ?
if(strchr(mode,'l') || raydium_init_cli_option("repository-disable",NULL))
    {
    return fopen(file2,mode);
    }

if(strchr(mode,'w'))
    {
    return fopen(file2,mode);
    }

if( !raydium_init_cli_option("repository-refresh",NULL) && 
    !raydium_init_cli_option("repository-force",NULL) )
{
 fp=fopen(file2,mode);
 if(fp) return fp;
}
raydium_rayphp_repository_file_get(file2);
fp=fopen(file2,mode);

return fp;
}

unsigned long raydium_file_sum_simple_mode(char *filename,char *mode)
{
unsigned long total=0;
unsigned long cpt=0;
int c;
FILE *fp;

fp=raydium_file_fopen(filename,mode);
if(!fp)
    {
    raydium_log("file simple sum: error: cannot open file '%s'",filename);
    return 0;
    }

while( (c=fgetc(fp))!=EOF )
    {
    c*=cpt;
    total+=c;
    cpt++;
    }

fclose(fp);
return total;
}

unsigned long raydium_file_sum_simple(char *filename)
{
return raydium_file_sum_simple_mode(filename,"rb");
}

char * raydium_file_home_path(char *file)
{
static char path[RAYDIUM_MAX_DIR_LEN];
sprintf(path,"%s/%s",raydium_homedir,file);
return path;
}

void raydium_file_home_path_cpy(char *file, char *dest)
{
strcpy(dest,raydium_file_home_path(file));
}

char *raydium_file_load(char *filename)
{
int len;
FILE *fp;
char *mem;

fp=raydium_file_fopen(filename,"rb");
if(!fp)
    return NULL;

fseek(fp,0,SEEK_END);
len=ftell(fp);
mem=malloc(len+1);
if(!mem)
    {
    fclose(fp);
    return NULL;
    }

mem[len]=0;
fseek(fp,0,SEEK_SET);

if(fread(mem,len,1,fp)!=1)
    {
    fclose(fp);
    free(mem);
    return NULL;
    }

fclose(fp);
return mem;
}
