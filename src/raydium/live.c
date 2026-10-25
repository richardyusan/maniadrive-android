/*
    Raydium - CQFD Corp.
    http://raydium.org/
    License: GPL - GNU General Public License, see "gpl.txt" file.
*/

#ifndef DONT_INCLUDE_HEADERS
#include "index.h"
#else
#include "headers/live.h"
#endif

#include "live.h"



// YUV420P to RGB code (next 2 functions) from:
// Peopletracking with an omnicamera
// Daniel Hammarin & Mihajlo Miladinovic

// !! Since this code is not native, no "raydium_" prefix is used !!

/* LIMIT: convert a 16.16 fixed-point value to a byte, with clipping. */
#define LIMIT(x) ((x)>0xffffff?0xff: ((x)<=0xffff?0:((x)>>16)))
void
v4l_copy_420_block (int yTL, int yTR, int yBL, int yBR,
		    int u, int v, int rowPixels, unsigned char *rgb, int bits)
{
  const int rvScale = 91881;
  const int guScale = -22553;
  const int gvScale = -46801;
  const int buScale = 116129;
  const int yScale = 65536;
  int r, g, b;
  g = guScale * u + gvScale * v;
  b = rvScale * v;
  r = buScale * u;
  yTL *= yScale;
  yTR *= yScale;
  yBL *= yScale;
  yBR *= yScale;
  if (bits == 24)
    {
/* Write out top two pixels */
      rgb[0] = LIMIT (b + yTL);
      rgb[1] = LIMIT (g + yTL);
      rgb[2] = LIMIT (r + yTL);
      rgb[3] = LIMIT (b + yTR);
      rgb[4] = LIMIT (g + yTR);
      rgb[5] = LIMIT (r + yTR);
/* Skip down to next line to write out bottom two pixels */
      rgb += 3 * rowPixels;
      rgb[0] = LIMIT (b + yBL);
      rgb[1] = LIMIT (g + yBL);
      rgb[2] = LIMIT (r + yBL);
      rgb[3] = LIMIT (b + yBR);
      rgb[4] = LIMIT (g + yBR);
      rgb[5] = LIMIT (r + yBR);
    }
  else if (bits == 16)
    {
/* Write out top two pixels */
      rgb[0] =
	((LIMIT (b + yTL) >> 3) & 0x1F) | ((LIMIT (g + yTL) << 3) & 0xE0);
      rgb[1] = ((LIMIT (g + yTL) >> 5) & 0x07) | (LIMIT (r + yTL) & 0xF8);
      rgb[2] =
	((LIMIT (b + yTR) >> 3) & 0x1F) | ((LIMIT (g + yTR) << 3) & 0xE0);
      rgb[3] = ((LIMIT (g + yTR) >> 5) & 0x07) | (LIMIT (r + yTR) & 0xF8);
/* Skip down to next line to write out bottom two pixels */
      rgb += 2 * rowPixels;
      rgb[0] =
	((LIMIT (b + yBL) >> 3) & 0x1F) | ((LIMIT (g + yBL) << 3) & 0xE0);
      rgb[1] = ((LIMIT (g + yBL) >> 5) & 0x07) | (LIMIT (r + yBL) & 0xF8);
      rgb[2] =
	((LIMIT (b + yBR) >> 3) & 0x1F) | ((LIMIT (g + yBR) << 3) & 0xE0);
      rgb[3] = ((LIMIT (g + yBR) >> 5) & 0x07) | (LIMIT (r + yBR) & 0xF8);
    }
}

int
v4l_yuv420p2rgb (unsigned char *rgb_out, unsigned char *yuv_in, int width, int
		 height, int bits)
{
  const int numpix = width * height;
  const unsigned int bytes = bits >> 3;
  int h, w, y00, y01, y10, y11, u, v;
  unsigned char *pY = yuv_in;
  unsigned char *pU = pY + numpix;
  unsigned char *pV = pU + numpix / 4;
  unsigned char *pOut = rgb_out;
  if (!rgb_out || !yuv_in)
    return -1;
  for (h = 0; h <= height - 2; h += 2)
    {
      for (w = 0; w <= width - 2; w += 2)
	{
	  y00 = *(pY);
	  y01 = *(pY + 1);
	  y10 = *(pY + width);
	  y11 = *(pY + width + 1);
	  u = (*pU++) - 128;
	  v = (*pV++) - 128;
	  v4l_copy_420_block (y00, y01, y10, y11, u, v, width, pOut, bits);
	  pY += 2;
	  pOut += bytes << 1;
	}
      pY += width;
      pOut += width * bytes;
    }
  return 0;
}


/////// video (devices) part


signed char raydium_live_video_isvalid(int i)
{
if(i>=0 && i<RAYDIUM_MAX_VIDEO_DEVICES &&
    raydium_live_device[i].capture_style!=RAYDIUM_LIVE_FREE)
	return 1;
return 0;
}


int raydium_live_video_find_free(void)
{
int i;

for(i=0;i<RAYDIUM_MAX_VIDEO_DEVICES;i++)
    if(raydium_live_device[i].capture_style==RAYDIUM_LIVE_FREE)
	return i;
return -1;
}


int raydium_live_video_open(char *device, int sizex, int sizey)
{
#ifndef WIN32
char *default_device=RAYDIUM_LIVE_DEVICE_DEFAULT;
int id;
int capture_style = RAYDIUM_LIVE_FREE;
char palette[128];
raydium_live_Device *dev;
char force_read=0;
char cli_device[RAYDIUM_MAX_NAME_LEN];


strcpy(palette,"(none)");

if(!device)
    {
    raydium_init_cli_option_default("video-device",cli_device,default_device);
    device=cli_device;
    }

id=raydium_live_video_find_free();
if(id<0)
    {
    raydium_log("live: ERROR: no more free device slot available (max: %i)",RAYDIUM_MAX_VIDEO_DEVICES);
    return -1;
    }

dev=&raydium_live_device[id];

dev->fd=open(device, O_RDWR);

if (dev->fd<0)
    {
    perror("open");
    raydium_log("live: ERROR: cannot open device '%s'",device);
    return -1;
    }


if (ioctl(dev->fd, VIDIOCGCAP, &dev->cap) < 0) 
    {
    perror("VIDIOGCAP");
    raydium_log("live: ERROR: not a video4linux device '%s'",device);
    close(dev->fd);
    return -1;
  }


if (ioctl(dev->fd, VIDIOCGWIN, &dev->win) < 0) 
    {
    perror("VIDIOCGWIN");
    raydium_log("live: ERROR: early error");
    close(dev->fd);
    return -1;
    }

if (ioctl(dev->fd, VIDIOCGPICT, &dev->vpic) < 0) 
    {
    perror("VIDIOCGPICT");
    raydium_log("live: ERROR: early error");
    close(dev->fd);
    return -1;
    }

raydium_log("live: device '%s' (%s)",dev->cap.name,device);

raydium_log("live: min %ix%i, max %ix%i, default %ix%i",
dev->cap.minwidth,dev->cap.minheight,
dev->cap.maxwidth,dev->cap.maxheight,
dev->win.width,dev->win.height);

dev->win.x=0;
dev->win.y=0;

if(sizex<0 || sizey<0)
    {
    char s[RAYDIUM_MAX_NAME_LEN];
    char sx[RAYDIUM_MAX_NAME_LEN];
    char sy[RAYDIUM_MAX_NAME_LEN];

    dev->win.width=RAYDIUM_LIVE_SIZEX_DEFAULT;
    dev->win.height=RAYDIUM_LIVE_SIZEY_DEFAULT;

    if(raydium_init_cli_option("video-size",s))
	{
	if(!raydium_parser_cut(s,sx,sy,'x'))
	    {
	    // apply to every video device ... :/
	    raydium_log("live: ERROR: --video-size format invalid (ex: 352x288)");
	    }
	else
	    {	    
	    dev->win.width=atoi(sx);
	    dev->win.height=atoi(sy);
	    }
	}
    }
else
    {
    dev->win.width=sizex;
    dev->win.height=sizey;
    }

dev->win.flags=0;
dev->win.clips=NULL;
dev->win.clipcount=0;
dev->win.chromakey=0;

if (ioctl(dev->fd, VIDIOCSWIN, &dev->win) < 0) 
    {
    perror("VIDIOCSWIN");
    raydium_log("live: ERROR: video mode refused: %ix%i",dev->win.width,dev->win.height);
    close(dev->fd);
    return -1;
    }

// read back
if (ioctl(dev->fd, VIDIOCGWIN, &dev->win) < 0) 
    {
    perror("VIDIOCGWIN");
    raydium_log("live: ERROR: cannot read back window settings. Should never happen.");
    close(dev->fd);
    return -1;
    }


if (dev->cap.type & VID_TYPE_MONOCHROME) 
    {
    dev->vpic.depth=8;
    dev->vpic.palette=VIDEO_PALETTE_GREY;	// 8bit grey
    strcpy(palette,"grey, 8 bpp");
    if(ioctl(dev->fd, VIDIOCSPICT, &dev->vpic) < 0) 
	{
	strcpy(palette,"grey, 6 bpp");
	dev->vpic.depth=6;
	if(ioctl(dev->fd, VIDIOCSPICT, &dev->vpic) < 0) 
	    {
	    strcpy(palette,"grey, 4 bpp");
	    dev->vpic.depth=4;
	    if(ioctl(dev->fd, VIDIOCSPICT, &dev->vpic) < 0) 
		{
		raydium_log("live: ERROR: cannot found suitable grey palette");
		close(dev->fd);
		return -1;
		}
	    }
	}
    } 
else 
    {
    strcpy(palette,"RGB, 24 bpp");
    dev->vpic.depth=24;
    dev->vpic.palette=VIDEO_PALETTE_RGB24;
    if(ioctl(dev->fd, VIDIOCSPICT, &dev->vpic) < 0) 
	{
	strcpy(palette,"RGB565, 16 bpp");
	dev->vpic.palette=VIDEO_PALETTE_RGB565;
	dev->vpic.depth=16;      
	if(ioctl(dev->fd, VIDIOCSPICT, &dev->vpic)==-1) 
	    {
	    strcpy(palette,"RGB555, 15 bpp");
	    dev->vpic.palette=VIDEO_PALETTE_RGB555;
	    dev->vpic.depth=15;
	    if(ioctl(dev->fd, VIDIOCSPICT, &dev->vpic)==-1) 
		{
		strcpy(palette,"YUV420P, 24 bpp");
		dev->vpic.palette=VIDEO_PALETTE_YUV420P;
		dev->vpic.depth=24;
		if(ioctl(dev->fd, VIDIOCSPICT, &dev->vpic)==-1) 
		    {
		    raydium_log("live: ERROR: cannot found suitable color palette");
		    close(dev->fd);
		    return -1;
		    }
		}
	    }
	}
    }


dev->buffer2  = malloc(dev->win.width * dev->win.height * dev->vpic.depth);
if (!dev->buffer2) 
    {
    raydium_log("live: ERROR: buffer2: out of memory (!?)");
    close(dev->fd);
    return -1;
    }

do // just to allow break in this if :)
{ 
if(dev->cap.type & VID_TYPE_CAPTURE)
    {
    capture_style=RAYDIUM_LIVE_CAPTURE_MMAP;
    if(ioctl(dev->fd,VIDIOCGMBUF,&dev->gb_buffers)==-1)
	{
	perror("VIDIOCGMBUF");
	raydium_log("live: ERROR: hardware refuse our mmap capture style (but is claiming this feature ...)");
	force_read=1;
	break;
	}

    dev->buffer = mmap(0,dev->gb_buffers.size,PROT_READ|PROT_WRITE,MAP_SHARED,dev->fd,0);
    if(dev->buffer==(void *) -1)
	{
	perror("mmap");
	raydium_log("live: ERROR: mmap failed");
	force_read=1;
	break;
	}

    dev->gb_buf.frame=0;
    dev->gb_buf.height = dev->win.height;
    dev->gb_buf.width = dev->win.width;
    dev->gb_buf.format = dev->vpic.palette;

    if(ioctl(dev->fd, VIDIOCMCAPTURE, &dev->gb_buf)==-1)
	{
	perror("VIDIOCMCAPTURE");
	raydium_log("live: ERROR: mmap capture test failed");
	munmap(dev->buffer,dev->gb_buffers.size);
	force_read=1;
	break;
	}
    }
} while(0); // i'm not proud of this ... :)

if(force_read)
    {
    raydium_log("live: fallback to read method. MAY BE SLOW !");
    }

if( (!(dev->cap.type & VID_TYPE_CAPTURE)) || force_read )
    {
    capture_style=RAYDIUM_LIVE_CAPTURE_READ;
  
    dev->buffer  = malloc(dev->win.width * dev->win.height * dev->vpic.depth);
    if (!dev->buffer) 
	{
	raydium_log("live: ERROR: buffer2: out of memory (!?)");
	close(dev->fd);
	return -1;
	}
    }

raydium_log("live: current: %ix%i, palette %s, %s",
dev->win.width,dev->win.height,palette,
(capture_style==RAYDIUM_LIVE_CAPTURE_READ?"read method":"mmap method") );

// reserve slot
dev->capture_style=capture_style;
strcpy(dev->name,device);
raydium_log("live: video init for this device is ok");
return id;
#else
raydium_log("live: Live API is not supported under win32 yet");
return -1;
#endif
}


int raydium_live_video_open_auto(void)
{
return raydium_live_video_open(RAYDIUM_LIVE_DEVICE_AUTO,RAYDIUM_LIVE_SIZE_AUTO,RAYDIUM_LIVE_SIZE_AUTO);
}


int raydium_live_video_read(raydium_live_Device *dev)
{
#ifndef WIN32
fd_set fds;
struct timeval tv;
int r;


if(dev->capture_style==RAYDIUM_LIVE_FREE)
    {
    raydium_log("live: ERROR: read failed: invalid device");
    return -1;
    }

// polling
FD_ZERO (&fds);
FD_SET (dev->fd, &fds);
tv.tv_sec=0;
tv.tv_usec=0;
r = select(dev->fd + 1, &fds, NULL, NULL, &tv);

// nothing to read, we'll try later (next frame)
if(r<=0)
    return 0;

dev->src = dev->buffer;

if(dev->capture_style==RAYDIUM_LIVE_CAPTURE_READ)
    {
    read(dev->fd, dev->buffer, dev->win.width * dev->win.height * dev->vpic.depth);
    }
else
    {
    dev->frame=dev->gb_buf.frame;
    dev->gb_buf.height = dev->win.height;
    dev->gb_buf.width = dev->win.width;
    dev->gb_buf.format = dev->vpic.palette;

    dev->gb_buf.frame=!dev->frame;
    ioctl(dev->fd, VIDIOCMCAPTURE, &dev->gb_buf);
    if(ioctl(dev->fd, VIDIOCSYNC, &dev->frame)==-1)
	{
	perror("mmap");
	return 0;
	}
    dev->src+=dev->gb_buffers.offsets[dev->frame];
    }


if(dev->vpic.palette==VIDEO_PALETTE_YUV420P)
    {
    // YUV420P style
    v4l_yuv420p2rgb (dev->buffer2,dev->src,dev->win.width,dev->win.height,dev->vpic.depth);
    }
else
    {
    // RGB style
    int i,j;
    int r,g,b;
    for (i = j = 0; i < dev->win.width * dev->win.height; i++) 
	{
	READ_VIDEO_PIXEL(dev->src, dev->vpic.palette, dev->vpic.depth, r, g, b);
	dev->buffer2[j++]=b>>8;
	dev->buffer2[j++]=g>>8;
	dev->buffer2[j++]=r>>8;
	}
    }


return 1;
#endif
}


void raydium_internal_live_video_callback(void)
{
int i;

for(i=0;i<RAYDIUM_MAX_VIDEO_DEVICES;i++)
 if(raydium_live_device[i].capture_style!=RAYDIUM_LIVE_FREE)
    if(raydium_live_video_read(&raydium_live_device[i]))
	raydium_live_texture_refresh(i);
}


/////////////// live API core

void raydium_internal_live_close(void)
{
int i;

for(i=0;i<RAYDIUM_MAX_VIDEO_DEVICES;i++)
 if(raydium_live_device[i].capture_style!=RAYDIUM_LIVE_FREE)
 {
#ifndef WIN32
    munmap(raydium_live_device[i].buffer, raydium_live_device[i].gb_buffers.size);
    close(raydium_live_device[i].fd);
#endif
 }    
}

void raydium_live_init(void)
{
int i;

for(i=0;i<RAYDIUM_MAX_VIDEO_DEVICES;i++)
    {
    raydium_live_device[i].capture_style=RAYDIUM_LIVE_FREE;
    raydium_live_device[i].buffer=NULL;
    raydium_live_device[i].frame=0;
    }    

for(i=0;i<RAYDIUM_MAX_LIVE_TEXTURES;i++)
    {
    raydium_live_texture[i].state=0;
    raydium_live_texture[i].device=NULL;
    raydium_live_texture[i].OnRefresh=NULL;
    raydium_live_texture[i].data_source=NULL;
    }    

raydium_log("video (live): OK");
}



signed char raydium_live_texture_isvalid(int i)
{
if(i>=0 && i<RAYDIUM_MAX_LIVE_TEXTURES && raydium_live_texture[i].state)
	return 1;
return 0;
}

int raydium_live_texture_find_free(void)
{
int i;

for(i=0;i<RAYDIUM_MAX_LIVE_TEXTURES;i++)
    if(!raydium_live_texture[i].state)
	return i;
return -1;
}

int raydium_live_texture_find(int original_texture)
{
int i;

for(i=0;i<RAYDIUM_MAX_LIVE_TEXTURES;i++)
    if(raydium_live_texture[i].state &&
       raydium_live_texture[i].texture==original_texture)
	    return i;
return -1;
}


int raydium_live_texture_video(int device_id, char *as)
{
#ifndef WIN32
int id;
raydium_live_Device *dev;
raydium_live_Texture *tex;

if(!raydium_live_video_isvalid(device_id))
    {
    raydium_log("live: ERROR: invalid device id, cannot create live source");
    return -1;
    }

dev=&raydium_live_device[device_id];
id=raydium_live_texture_find_free();

if(id<0)
    {
    raydium_log("live: ERROR: no more free live texture slot available (max: %i)",RAYDIUM_MAX_LIVE_TEXTURES);
    return -1;    
    }

tex=&raydium_live_texture[id];

tex->tx=dev->win.width;
tex->ty=dev->win.height;
tex->hardware_tx=raydium_trigo_pow2_next(dev->win.width);
tex->hardware_ty=raydium_trigo_pow2_next(dev->win.height);
tex->bpp=dev->vpic.depth;
tex->texture=raydium_texture_load_internal("not needed :)",as,1,0,0,0,id);


if(tex->texture<=0)
    {
    raydium_log("live: ERROR: cannot create 'faked' texture (see above)");
    return -1;    
    }

tex->device=dev;
tex->data_source=dev->buffer2;
tex->state=1;
raydium_log("live: %s linked to %s (live)",dev->name,as);
return id;
#else
raydium_log("live: Live API is not supported under win32 yet");
return -1;
#endif
}


void raydium_live_texture_refresh(int livetex)
{
raydium_live_Texture *tex;
int (*f)(unsigned char *data, int tx, int ty, int bpp);

if(!raydium_live_texture_isvalid(livetex))
    {
    raydium_log("live: cannot refresh live texture: wrong name or id");
    return;
    }

tex=&raydium_live_texture[livetex];

f=tex->OnRefresh;
if(f && !f(tex->data_source,tex->tx,tex->ty,tex->bpp))
    return;

//printf("%i - %ix%i %i bpp\n",tex->texture,tex->tx,tex->ty,tex->bpp);

glEnable(GL_TEXTURE_2D);
glBindTexture(GL_TEXTURE_2D,tex->texture);
glTexSubImage2D(GL_TEXTURE_2D,0,0,0,
		tex->tx,
		tex->ty,
		(tex->bpp==24?GL_RGB:GL_RGBA),
		GL_UNSIGNED_BYTE,
		tex->data_source);
}



void raydium_live_texture_refresh_name(char *texture)
{
raydium_live_texture_refresh(raydium_live_texture_find(raydium_texture_find_by_name(texture)));
}


int raydium_live_texture_create(char *as, unsigned char *data_source, int tx, int ty, int bpp)
{
int id,i;
raydium_live_Texture *tex;

id=raydium_live_texture_find_free();

if(id<0)
    {
    raydium_log("live: ERROR: no more free live texture slot available (max: %i)",RAYDIUM_MAX_LIVE_TEXTURES);
    return -1;    
    }

if(bpp!=24 && bpp!=32)
    {
    raydium_log("live: ERROR: live textures are limited to 24 or 32 bpp color depth only, for now");
    return -1;    
    }


// duplicated ?
for(i=0;i<raydium_texture_index;i++)
    if(!strcmp(raydium_texture_name[i],as)) 
    {
    raydium_log("live: WARNING ! %s is duplicated",as);
    // this is the right answer only if duplicated texture
    // is already a "live" one ! (to fix ?)

    id=raydium_live_texture_find(i);
    // ... but reset/update some values
    tex=&raydium_live_texture[id];
    tex->device=NULL;
    tex->data_source=data_source;

    raydium_live_texture_refresh(id);
    return id;
    }


tex=&raydium_live_texture[id];
tex->tx=tx;
tex->ty=ty;
tex->hardware_tx=raydium_trigo_pow2_next(tex->tx);
tex->hardware_ty=raydium_trigo_pow2_next(tex->ty);
tex->bpp=bpp;
tex->texture=raydium_texture_load_internal("not needed :)",as,1,0,0,0,id);

if(tex->texture<=0)
    {
    raydium_log("live: ERROR: cannot create 'faked' texture (see above)");
    return -1;    
    }

tex->device=NULL;
tex->data_source=data_source;
tex->state=1;
raydium_live_texture_refresh(id);
raydium_log("live: texture '%s' created",as);
return id;
}

void raydium_live_texture_mask(int livetex, GLfloat alpha)
{
raydium_live_Texture *tex;
GLfloat u,v;

if(!raydium_live_texture_isvalid(livetex))
    {
    raydium_log("live: cannot draw live mask: wrong name or id");
    return;
    }

tex=&raydium_live_texture[livetex];

u=tex->tx/(float)tex->hardware_tx;
v=tex->ty/(float)tex->hardware_ty;

//raydium_osd_mask_texture_clip(tex->texture,alpha,u*100,v*100,0,0);
raydium_osd_mask_texture_clip(tex->texture,alpha,0,v*100,u*100,0);
}


void raydium_live_texture_mask_name(char *texture, GLfloat alpha)
{
raydium_live_texture_mask(raydium_live_texture_find(raydium_texture_find_by_name(texture)),alpha);
}


// (the current code of this function is quiet a hack ...)
void raydium_live_texture_draw(int livetex, GLfloat alpha,GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
raydium_live_Texture *tex;
GLfloat u,v;

if(!raydium_live_texture_isvalid(livetex))
    {
    raydium_log("live: cannot draw live mask: wrong name or id");
    return;
    }

tex=&raydium_live_texture[livetex];

u=tex->tx/(float)tex->hardware_tx;
v=tex->ty/(float)tex->hardware_ty;

raydium_osd_start();

raydium_texture_current_set(tex->texture);
raydium_rendering_internal_prepare_texture_render(tex->texture);
glColor4f(1,1,1,alpha);
glEnable(GL_BLEND);
glDepthMask(GL_FALSE);
glBegin(GL_QUADS);
glTexCoord2f(0,v);
glVertex3f(x1,y1,0);
glTexCoord2f(u,v);
glVertex3f(x2,y1,0);
glTexCoord2f(u,0);
glVertex3f(x2,y2,0);
glTexCoord2f(0,0);
glVertex3f(x1,y2,0);
glEnd();
raydium_rendering_internal_restore_render_state();

raydium_osd_stop();
}


void raydium_live_texture_draw_name(char *texture, GLfloat alpha,GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
raydium_live_texture_draw(raydium_live_texture_find(raydium_texture_find_by_name(texture)),alpha,x1,y1,x2,y2);
}

void raydium_live_texture_refresh_callback_set(int livetex, void *callback)
{
raydium_live_Texture *tex;

if(!raydium_live_texture_isvalid(livetex))
    {
    raydium_log("live: cannot set OnRefresh callback: wrong name or id");
    return;
    }

tex=&raydium_live_texture[livetex];
tex->OnRefresh=callback;
}

void raydium_live_texture_refresh_callback_set_name(char *texture, void *callback)
{
raydium_live_texture_refresh_callback_set(raydium_live_texture_find(raydium_texture_find_by_name(texture)),callback);
}


