/*
    Raydium - CQFD Corp.
    http://raydium.org/
    License: GPL - GNU General Public License, see "gpl.txt" file.
*/

#ifndef DONT_INCLUDE_HEADERS
#include "index.h"
#else
#include "headers/video.h"
#endif

#include "video.h"
    

void raydium_video_init(void)
{
int i;

for(i=0;i<RAYDIUM_MAX_VIDEOS;i++)
    {
    raydium_video_video[i].state=0;
    }

}

signed char raydium_video_isvalid(int i)
{
if(i>=0 && i<RAYDIUM_MAX_VIDEOS &&
    raydium_video_video[i].state)
	return 1;
return 0;

}

int raydium_video_find_free(void)
{
int i;

for(i=0;i<RAYDIUM_MAX_VIDEOS;i++)
    if(!raydium_video_video[i].state)
	return i;
return -1;
}


int raydium_video_find(char *name)
{
int live,i;

//live=raydium_live_texture_find(raydium_texture_find_by_name(name));
live=raydium_live_texture_find(raydium_texture_exists(name));

for(i=0;i<RAYDIUM_MAX_VIDEOS;i++)
    if(raydium_video_video[i].state &&
       raydium_video_video[i].live_id==live)
	return i;

return -1;
}

void raydium_video_jpeg_decompress(FILE *fp,unsigned char *to)
{
struct jpeg_decompress_struct cinfo;
struct jpeg_error_mgr jerr;
JSAMPARRAY buffer;
int row_stride;
long count;

//printf(".\n");

cinfo.err=jpeg_std_error(&jerr);
jpeg_create_decompress(&cinfo);
jpeg_stdio_src(&cinfo, fp);
jpeg_read_header(&cinfo, TRUE);
jpeg_start_decompress(&cinfo);

row_stride = cinfo.output_width * cinfo.output_components;
buffer = (*cinfo.mem->alloc_sarray)
	((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

count=cinfo.output_height-1;
while (cinfo.output_scanline < cinfo.output_height) 
    {
    jpeg_read_scanlines(&cinfo, buffer, 1);
    memcpy(to+cinfo.image_width*cinfo.output_components*(cinfo.output_height-count-1),buffer[0],row_stride);
    count--;
    }
jpeg_finish_decompress(&cinfo);
jpeg_destroy_decompress(&cinfo);
}


int raydium_video_open(char *filename, char *as)
{
int id;
char head[100];
int head_end;
int i,j,c;

/*
// we must find any previous load of this video
for(i=0;i<RAYDIUM_MAX_VIDEOS;i++)
    if( raydium_video_video[i].state && 
	!strcmp(raydium_video_video[i].name,filename))
	    return i;
*/

id=raydium_video_find_free();

if(id<0)
    {
    raydium_log("video: ERROR no more free slot (%i max)",RAYDIUM_MAX_VIDEOS);
    return -1;
    }

raydium_video_video[id].fp=raydium_file_fopen(filename,"rb");

if(!raydium_video_video[id].fp)
    {
    raydium_log("video: ERROR: file '%s': open failed",filename);
    return -1;
    }

fread(head,90,1,raydium_video_video[id].fp);
head_end=0;
for(i=0;i<90;i++)
    if(head[i]=='|')
	{
	head_end=i;
	break;
	}

if(!head_end)
    {
    fclose(raydium_video_video[id].fp);
    raydium_log("video: ERROR: file '%s': invalid header or not a JPGS file. see docs",filename);
    return -1;
    }

head[head_end]=0;
fseek(raydium_video_video[id].fp,head_end+1,SEEK_SET);

sscanf(head,"%f %i %i %i",
&raydium_video_video[id].fps,
&raydium_video_video[id].sizex,
&raydium_video_video[id].sizey,
&raydium_video_video[id].frames_total);

raydium_video_video[id].state=1;
raydium_video_video[id].elapsed=0;
raydium_video_video[id].data=malloc(raydium_video_video[id].sizex*raydium_video_video[id].sizey*3);
raydium_video_video[id].live_id=raydium_live_texture_create(as,raydium_video_video[id].data,raydium_video_video[id].sizex,raydium_video_video[id].sizey,24);

raydium_video_video[id].offsets=malloc(sizeof(long)*raydium_video_video[id].frames_total);
for(i=0;i<raydium_video_video[id].frames_total;i++)
    {
    head[0]=0;
    j=0;
    while((c=fgetc(raydium_video_video[id].fp))!='|')
	{
	head[j++]=c;
	head[j]=0;
	}
    raydium_video_video[id].offsets[i]=atol(head);
    }

raydium_video_video[id].start=ftell(raydium_video_video[id].fp);
raydium_video_video[id].last_decoded=-1;
raydium_video_video[id].loop=1;
raydium_video_video[id].playing=1;
strcpy(raydium_video_video[id].name,filename);

raydium_log("video: %s (%i) as live texture %s (%i), %ix%i %.2f fps (%i frames)",
filename,id,as,raydium_video_video[id].live_id,
raydium_video_video[id].sizex,raydium_video_video[id].sizey,
raydium_video_video[id].fps,
raydium_video_video[id].frames_total);

return id;
}


void raydium_video_callback_video(int id)
{
int current;

if(!raydium_video_video[id].playing)
    return;

raydium_video_video[id].elapsed+=raydium_frame_time;

current=raydium_video_video[id].elapsed*raydium_video_video[id].fps;

if(current>=raydium_video_video[id].frames_total)
    {
    if(!raydium_video_video[id].loop)
	{
	raydium_video_video[id].playing=0;
	return;
	}
	
    raydium_video_video[id].elapsed=0;
    current=0;
    }

if(current==raydium_video_video[id].last_decoded)
    {
    //printf("not again\n");
    return;
    }

fseek(raydium_video_video[id].fp,raydium_video_video[id].start+raydium_video_video[id].offsets[current],SEEK_SET);
raydium_video_jpeg_decompress(raydium_video_video[id].fp,raydium_video_video[id].data);

raydium_video_video[id].last_decoded=current;
raydium_live_texture_refresh(raydium_video_video[id].live_id);
}

void raydium_video_callback(void)
{
int i;

for(i=0;i<RAYDIUM_MAX_VIDEOS;i++)
    if(raydium_video_video[i].state)
	{
	raydium_video_callback_video(i);
	}
}

void raydium_video_delete(int id)
{
if(!raydium_video_isvalid(id))
    {
    raydium_log("video: ERROR: cannot delete video: invalid index or name");
    return;
    }
fclose(raydium_video_video[id].fp);
free(raydium_video_video[id].offsets);
free(raydium_video_video[id].data);
raydium_video_video[id].state=0;
}

void raydium_video_delete_name(char *name)
{
raydium_video_delete(raydium_video_find(name));
}


void raydium_video_loop(int id, signed char loop)
{
if(!raydium_video_isvalid(id))
    {
    raydium_log("video: ERROR: cannot set loop attrib: invalid index or name");
    return;
    }
raydium_video_video[id].loop=loop;
}

void raydium_video_loop_name(char *name, signed char loop)
{
raydium_video_loop(raydium_video_find(name),loop);
}

signed char raydium_video_isplaying(int id)
{
if(!raydium_video_isvalid(id))
    {
    raydium_log("video: ERROR: cannot get playing state: invalid index or name");
    return -1;
    }
return raydium_video_video[id].playing;
}

signed char raydium_video_isplaying_name(char *name)
{
return raydium_video_isplaying(raydium_video_find(name));
}
