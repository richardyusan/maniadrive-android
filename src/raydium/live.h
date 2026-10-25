/*
    Raydium - CQFD Corp.
    http://raydium.org/
    License: GPL - GNU General Public License, see "gpl.txt" file.
*/

// Live textures functions

// TODO:
// 4 bpp live textures support

#ifndef LIVE_H
#define LIVE_H

#ifndef WIN32
#include <linux/types.h>
#include <linux/videodev.h>
#include <sys/mman.h>
#endif

#define RAYDIUM_LIVE_DEVICE_DEFAULT	"/dev/video0"
#define RAYDIUM_LIVE_SIZEX_DEFAULT	352
#define RAYDIUM_LIVE_SIZEY_DEFAULT	288

#define RAYDIUM_LIVE_DEVICE_AUTO	NULL
#define RAYDIUM_LIVE_SIZE_AUTO		-1

#define RAYDIUM_LIVE_FREE		0
#define RAYDIUM_LIVE_CAPTURE_NONE	-1
#define RAYDIUM_LIVE_CAPTURE_READ	1
#define RAYDIUM_LIVE_CAPTURE_MMAP	2


typedef struct raydium_live_Device 
{
#ifndef WIN32
  int fd;
  struct video_capability cap;
  struct video_window win;
  struct video_picture vpic;

  // for mmap captures
  struct video_mbuf gb_buffers;
  struct video_mmap gb_buf;
#endif

  unsigned char *buffer;  // capture buffer
  unsigned char *src;     // intermediate buffer
  unsigned char *buffer2; // final buffer

  signed char capture_style;
  int frame;
  char name[RAYDIUM_MAX_NAME_LEN];
} raydium_live_Device;


typedef struct raydium_live_Texture
{
  signed char state;
  raydium_live_Device *device;
  int texture;
  void *OnRefresh;
  int tx;
  int ty;
  int hardware_tx;
  int hardware_ty;
  int bpp;
  unsigned char *data_source;
} raydium_live_Texture;

__global raydium_live_Device raydium_live_device[RAYDIUM_MAX_VIDEO_DEVICES];
__global raydium_live_Texture raydium_live_texture[RAYDIUM_MAX_LIVE_TEXTURES];

// mostly from tvset.c (thanks to vgrabber.c)
#define READ_VIDEO_PIXEL(buf, format, depth, r, g, b)			\
{									\
	switch (format)							\
	{								\
		case VIDEO_PALETTE_GREY:				\
			switch (depth)					\
			{						\
				case 4:					\
				case 6:					\
				case 8:					\
					(r) = (g) = (b) = (*buf++ << 8);\
					break;				\
									\
				case 16:				\
					(r) = (g) = (b) = 		\
						*((unsigned short *) buf);	\
					buf += 2;			\
					break;				\
			}						\
			break;						\
									\
									\
		case VIDEO_PALETTE_RGB565:				\
		{							\
			unsigned short tmp = *(unsigned short *)buf;	\
			(r) = tmp&0xF800;				\
			(g) = (tmp<<5)&0xFC00;				\
			(b) = (tmp<<11)&0xF800;				\
			buf += 2;					\
		}							\
		break;							\
									\
		case VIDEO_PALETTE_RGB555:				\
			(r) = (buf[0]&0xF8)<<8;				\
			(g) = ((buf[0] << 5 | buf[1] >> 3)&0xF8)<<8;	\
			(b) = ((buf[1] << 2 ) & 0xF8)<<8;		\
			buf += 2;					\
			break;						\
									\
		case VIDEO_PALETTE_RGB24:				\
			(r) = buf[0] << 8; (g) = buf[1] << 8; 		\
			(b) = buf[2] << 8;				\
			buf += 3;					\
			break;						\
		case VIDEO_PALETTE_YUV420P:				\
			(r) = buf[0]+1.140*buf[2];		 	\
			(g) = buf[0]-0.394*buf[1]-0.581*buf[2]; 	\
			(b) = buf[0]+2.028*buf[1];			\
			buf += 3;					\
			break;						\
									\
		default:						\
			raydium_log("live: error: palette unknown");	\
	}								\
}						


// endif LIVE_H
#endif
