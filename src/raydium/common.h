#ifndef _CORE_COMMON_H
#define _CORE_COMMON_H
/*
* Raydium - CQFD Corp.
* http://raydium.org/
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#define RAYDIUM_MAJOR	0
#define RAYDIUM_MINOR	705

#ifdef WIN32
# ifdef RAYDLL
#   ifdef MAIN_H
#       define __global __declspec(dllimport)
#       define __rayapi __declspec(dllimport)
#   endif
#   ifdef MAIN_C
#       define __global __declspec(dllexport)
#       define __rayapi __declspec(dllexport)
#   endif
# endif
#else // Not under WIN32
# ifdef LIBRAY
#    define __global
#    define __rayapi
# else
#    ifdef MAIN_H
#       define __global extern
#       define __rayapi
#    endif
#    ifdef MAIN_C
#       define __global
#       define __rayapi
#    endif
# endif
#endif

#ifdef SWIG
#define __global
#endif

#ifdef WIN32
#include "windows.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <math.h>
#include <time.h>
#include <memory.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <stdarg.h>

#include <AL/al.h>
#include <AL/alut.h>
#include <vorbis/vorbisfile.h>

#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

// need to separate LINUX & others, using glut for joystick..
#ifndef WIN32
#include <linux/joystick.h>
#include <sys/ioctl.h>
#include <linux/rtc.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pwd.h>
#endif

#ifdef linux
#include <linux/if.h>
#include <linux/sockios.h>
#endif

#include <errno.h>
#include "config.h"

#ifdef PHP_SUPPORT
#ifdef WIN32
#define PHP_WIN32
#define ZTS
#define ZEND_WIN32
#endif
#include "php.h"
#include "php_version.h"
#include "php_globals.h"
#include "php_variables.h"
#include "zend_modules.h"

#include "SAPI.h"

#include "php.h"
//#include "build-defs.h"
#include "zend.h"
#include "zend_extensions.h"
#include "php_ini.h"
#include "php_globals.h"
#include "php_main.h"
#include "fopen_wrappers.h"
#include "ext/standard/php_standard.h"
#endif

#include <jpeglib.h>

#define PI 					(3.1415926535f)

// Our turn, let's define all about Raydium ...
#define RAYDIUM_APP_SHORTNAME			"raydium"

#define RAYDIUM_MAX_VERTICES  			2000000
#define RAYDIUM_MAX_TEXTURES    	   	256
#define RAYDIUM_MAX_LIGHTS         	     	8
#define RAYDIUM_MAX_NAME_LEN     	   	255
#define RAYDIUM_MAX_DIR_LEN     	   	4096
#define RAYDIUM_MAX_OBJECTS		  	1024
#define RAYDIUM_MAX_OBJECT_ANIMS		20
#define RAYDIUM_MAX_OBJECT_ANIM_INSTANCES	64
#define RAYDIUM_MAX_TIMECALLS 			16
#define RAYDIUM_MAX_REG_VARIABLES  		256
#define RAYDIUM_MAX_REG_FUNCTION		256
#define RAYDIUM_MAX_LOG_FOPEN			256
#define RAYDIUM_MAX_CAMERA_PATHS		16
#define RAYDIUM_MAX_CAMERA_PATH_STEPS		512
#define RAYDIUM_MAX_VIDEO_DEVICES		4
#define RAYDIUM_MAX_LIVE_TEXTURES		8
#define RAYDIUM_MAX_VIDEOS			4
#define RAYDIUM_MAX_SHADERS			32

#define RAYDIUM_LIGHT_ON           	     	1
#define RAYDIUM_LIGHT_BLINKING     	     	2
#define RAYDIUM_LIGHT_OFF         	    	-1

#define RAYDIUM_TEXTURE_FILTER_NONE	     	0
#define RAYDIUM_TEXTURE_FILTER_BILINEAR	     	1
#define RAYDIUM_TEXTURE_FILTER_TRILINEAR     	2

#define RAYDIUM_TEXTURE_BLEND_NONE		0
#define RAYDIUM_TEXTURE_BLEND_BLENDED		1
#define RAYDIUM_TEXTURE_BLEND_CUTOUT		2 
#define RAYDIUM_TEXTURE_PHANTOM			3

#define RAYDIUM_RENDERING_WINDOW      	    	0
#define RAYDIUM_RENDERING_FULLSCREEN  	     	1
#define RAYDIUM_RENDERING_NONE  	     	2

#define RAYDIUM_PROJECTION_ORTHO       	     	0
#define RAYDIUM_PROJECTION_PERSPECTIVE 	     	1

#define RAYDIUM_KEYBOARD_SIZE	       	   	256
#define RAYDIUM_JOY_MAX_BUTTONS			16
#define RAYDIUM_JOY_MAX_AXIS			8

#define RAYDIUM_NETWORK_PORT			29104
#define RAYDIUM_NETWORK_BEACON_PORT		(RAYDIUM_NETWORK_PORT+1)
#define RAYDIUM_NETWORK_PACKET_SIZE		512
#define RAYDIUM_NETWORK_TIMEOUT			10
#define RAYDIUM_NETWORK_PACKET_OFFSET		4
#define RAYDIUM_NETWORK_MAX_CLIENTS		8
#define RAYDIUM_NETWORK_MAX_SERVERS		32 // LAN server list
#define RAYDIUM_NETWORK_TX_QUEUE_SIZE		128
#define RAYDIUM_NETWORK_MAX_TRIES		8
#define RAYDIUM_NETWORK_MAX_NETCALLS 		32
#define RAYDIUM_NETWORK_MAX_PROPAGS 		32
#define RAYDIUM_NETWORK_ACK_DELAY_MAX		2
#define RAYDIUM_NETWORK_PROPAG_HEAD		sizeof(int)
#define RAYDIUM_NETWORK_BEACON_DELAY		5
#define RAYDIUM_NETWORK_BEACON_DEFAULT_TTL	15
#define RAYDIUM_NETWORK_BEACON_INFO_MAX_LEN	100
#define RAYDIUM_NETWORK_MODE_NONE		0
#define RAYDIUM_NETWORK_MODE_CLIENT		1
#define RAYDIUM_NETWORK_MODE_SERVER		2
#define RAYDIUM_NETWORK_MODE_DISCOVER		3
//#define RAYDIUM_NETWORK_TCP			SOCK_STREAM
#define RAYDIUM_NETWORK_UDP			SOCK_DGRAM
#define RAYDIUM_NETWORK_DATA_OK			1
#define RAYDIUM_NETWORK_DATA_NONE		0
#define RAYDIUM_NETWORK_DATA_ERROR		-1

#define RAYDIUM_NETWORK_PACKET_DATA			1
#define RAYDIUM_NETWORK_PACKET_ERROR_NO_MORE_PLACE	2
#define RAYDIUM_NETWORK_PACKET_ATTRIB_UID		3
#define RAYDIUM_NETWORK_PACKET_REQUEST_UID		4
#define RAYDIUM_NETWORK_PACKET_INFO_NAME		5
#define RAYDIUM_NETWORK_PACKET_ACK			6
#define RAYDIUM_NETWORK_PACKET_SERVER_BEACON		7
#define RAYDIUM_NETWORK_PACKET_ODE_DATA			10
#define RAYDIUM_NETWORK_PACKET_ODE_NEWELEM		11
#define RAYDIUM_NETWORK_PACKET_ODE_REMELEM		12
#define RAYDIUM_NETWORK_PACKET_ODE_NIDWHO		13
#define RAYDIUM_NETWORK_PACKET_ODE_EXPLOSION		14
#define	RAYDIUM_NETWORK_PACKET_BASE			20

#define RAYDIUM_SOUND_NUM_BUFFERS 		30
#define RAYDIUM_SOUND_NUM_SOURCES 		30
#define SOUNDDATASIZE 				(4096*20)
//#define RAYDIUM_SOUND_NUM_ENVIRONMENTS 		1

#define RAYDIUM_CONSOLE_MAX_LINES		18
#define RAYDIUM_CONSOLE_MAX_HISTORY		1000
#define RAYDIUM_CONSOLE_FONT_SIZE		16
#define RAYDIUM_CONSOLE_FONT_SPACER		0.5
#define RAYDIUM_CONSOLE_MAX_COMPLETION		10

#define RAYDIUM_TIMECALL_METHOD_NONE		0
#define RAYDIUM_TIMECALL_METHOD_CLOCK		1
#define RAYDIUM_TIMECALL_METHOD_DEVRTC 		2

#define RAYDIUM_TIMECALL_FREQ_MIN 		100
#define RAYDIUM_TIMECALL_FREQ_PREFERED 		8192
#define RAYDIUM_TIMECALL_W32_MODULO_MIN		15

#define RAYDIUM_REGISTER_INT    		1
#define RAYDIUM_REGISTER_FLOAT  		2
#define RAYDIUM_REGISTER_STR    		3
#define RAYDIUM_REGISTER_ICONST			4
#define RAYDIUM_REGISTER_FCONST			5
#define RAYDIUM_REGISTER_SCHAR			6

#define RAYDIUM_PARSER_TYPE_EOF			0
#define RAYDIUM_PARSER_TYPE_FLOAT		1
#define RAYDIUM_PARSER_TYPE_STRING		2
#define RAYDIUM_PARSER_TYPE_DEBUG		3
#define RAYDIUM_PARSER_TYPE_RAWDATA		4

#define RAYDIUM_OSD_FONT_SIZE_FACTOR		(12.f)

#define RAYDIUM_RENDER_MULTITEX_AUTO_UV_FACT	(50.f)
#define RAYDIUM_RENDER_REFLECTION_FACT		(0.1f)

#define RAYDIUM_SKY_SPHERE_MAX_DETAIL 		30
#define RAYDIUM_SKY_SPHERE_DEFAULT_DETAIL 	25

#define RAYDIUM_CAPTURE_NONE			0
#define RAYDIUM_CAPTURE_TGA			1
#define RAYDIUM_CAPTURE_JPG			2

#define RAYDIUM_DB_FILENAME			raydium_file_home_path("raydium.db")
#define RAYDIUM_DB_TEMP				raydium_file_home_path("raydium.db.temp")
#define RAYDIUM_DB_SEPARATOR			';'

#define RAYDIUM_HDR_SIZE			64
#define RAYDIUM_HDR_PASS			8
#define RAYDIUM_HDR_EYE_SPEED_DEFAULT		0.1f

#define RAYDIUM_FOG_MODE_LINEAR			GL_LINEAR
#define RAYDIUM_FOG_MODE_EXP			GL_EXP
#define RAYDIUM_FOG_MODE_EXP2			GL_EXP2


__global int 	 raydium_init_argc;
__global char  **raydium_init_argv;
__global char    raydium_init_wd[RAYDIUM_MAX_DIR_LEN];
__global char	 raydium_homedir[RAYDIUM_MAX_DIR_LEN];

__global int     raydium_key_last;
__global signed char    raydium_key[RAYDIUM_KEYBOARD_SIZE];
__global signed char    raydium_key_trace; // change this from ingame console !
__global signed char    raydium_mouse_click;
__global signed char    raydium_mouse_button[3];
__global GLuint  raydium_mouse_x;
__global GLuint  raydium_mouse_y;
__global signed char    raydium_joy_button[RAYDIUM_JOY_MAX_BUTTONS];
__global GLfloat 	raydium_joy_axis[RAYDIUM_JOY_MAX_AXIS];
__global signed char	raydium_joy_click;
__global GLfloat raydium_joy_x;
__global GLfloat raydium_joy_y;
__global GLfloat raydium_joy_z;
__global int     raydium_joy;
__global char    raydium_joy_n_axes;     // read only
__global char    raydium_joy_n_buttons;  // read only


__global GLuint   raydium_texture_index;
__global GLuint   raydium_texture_current_main;
__global GLuint   raydium_texture_current_multi;
__global GLfloat  raydium_texture_current_multi_u;
__global GLfloat  raydium_texture_current_multi_v;
__global GLuint   raydium_texture_current_env;
__global signed char     raydium_texture_filter;
__global GLint	  raydium_texture_size_max;
__global GLint	  raydium_texture_units;
__global GLuint   raydium_internal_size_vector_float_4;
__global GLuint   raydium_texture_to_replace;
__global GLfloat  raydium_texture_used_memory;

__global signed char    raydium_projection;
__global GLfloat raydium_projection_fov;  	// perspective only
__global GLfloat raydium_projection_near; 	// perspective & ortho
__global GLfloat raydium_projection_far; 	// perspective & ortho
__global GLfloat raydium_projection_left; 	// ortho only
__global GLfloat raydium_projection_right; 	// ortho only
__global GLfloat raydium_projection_bottom; 	// ortho only
__global GLfloat raydium_projection_top; 	// ortho only
__global GLfloat raydium_background_color[4];

__global signed char raydium_sky_force;
__global signed char raydium_sky_atmosphere_enable_tag;
__global GLfloat     raydium_sky_sphere_angle_orbit_u;
__global GLfloat     raydium_sky_sphere_angle_orbit_v;
__global GLfloat     raydium_sky_sphere_x_vel;
__global GLfloat     raydium_sky_sphere_y_vel;
__global GLfloat     raydium_sky_sphere_x_pos;
__global GLfloat     raydium_sky_sphere_y_pos;
__global GLfloat     raydium_sky_sphere_quality;
__global GLfloat     raydium_sky_sphere_heigth;
__global signed char raydium_sky_sphere_generated;

__global GLsizei raydium_window_tx;
__global GLsizei raydium_window_ty;
__global signed char	 raydium_window_mode;

__global GLuint  raydium_vertex_index;
__global GLuint  raydium_vertex_counter;
__global signed char    raydium_vertex_offset_triangle;

__global GLfloat *raydium_vertex_x;
__global GLfloat *raydium_vertex_y;
__global GLfloat *raydium_vertex_z;
__global GLfloat *raydium_vertex_normal_x;
__global GLfloat *raydium_vertex_normal_y;
__global GLfloat *raydium_vertex_normal_z;
__global GLfloat *raydium_vertex_normal_visu_x; //
__global GLfloat *raydium_vertex_normal_visu_y; // used for smoothing
__global GLfloat *raydium_vertex_normal_visu_z; //
__global GLfloat *raydium_vertex_texture_u;
__global GLfloat *raydium_vertex_texture_v;
__global GLuint  *raydium_vertex_texture;
__global GLuint  *raydium_vertex_texture_multi;
__global GLfloat *raydium_vertex_texture_multi_u;
__global GLfloat *raydium_vertex_texture_multi_v;
__global GLuint  *raydium_vertex_texture_env;
__global signed char	 *raydium_vertex_tag;

__global signed char     raydium_texture_islightmap[RAYDIUM_MAX_TEXTURES];
__global signed char     raydium_texture_nolight[RAYDIUM_MAX_TEXTURES];
__global signed char     raydium_texture_blended[RAYDIUM_MAX_TEXTURES];
__global signed char     raydium_texture_hdr[RAYDIUM_MAX_TEXTURES];
__global signed char     raydium_texture_env[RAYDIUM_MAX_TEXTURES];
__global GLint    raydium_texture_shader[RAYDIUM_MAX_TEXTURES];
__global char     raydium_texture_name[RAYDIUM_MAX_TEXTURES][RAYDIUM_MAX_NAME_LEN];
__global GLfloat  raydium_texture_rgb[RAYDIUM_MAX_TEXTURES][4];

__global GLuint	raydium_object_index;
__global GLuint	raydium_object_start[RAYDIUM_MAX_OBJECTS];
__global GLuint	raydium_object_end[RAYDIUM_MAX_OBJECTS];
__global char	raydium_object_name[RAYDIUM_MAX_OBJECTS][RAYDIUM_MAX_NAME_LEN];
__global signed char raydium_object_anims[RAYDIUM_MAX_OBJECTS]; // number of anims
__global GLuint raydium_object_anim_len[RAYDIUM_MAX_OBJECTS]; // len of each anim
__global GLuint raydium_object_anim_start[RAYDIUM_MAX_OBJECTS][RAYDIUM_MAX_OBJECT_ANIMS];
__global GLuint raydium_object_anim_end[RAYDIUM_MAX_OBJECTS][RAYDIUM_MAX_OBJECT_ANIMS];
__global char   raydium_object_anim_names[RAYDIUM_MAX_OBJECTS][RAYDIUM_MAX_OBJECT_ANIMS][RAYDIUM_MAX_NAME_LEN]; // anims name array
__global GLuint raydium_object_anim_default_anim[RAYDIUM_MAX_OBJECTS];
__global GLuint raydium_object_anim_instance_current[RAYDIUM_MAX_OBJECTS]; // current "rendering" instance
__global GLfloat raydium_object_anim_automatic_factor[RAYDIUM_MAX_OBJECTS][RAYDIUM_MAX_OBJECT_ANIMS]; // frame automatic factor
__global GLfloat raydium_object_anim_time_factor;
// states
__global int	 raydium_object_anim_current[RAYDIUM_MAX_OBJECTS][RAYDIUM_MAX_OBJECT_ANIM_INSTANCES]; // current anim
__global GLfloat raydium_object_anim_frame_current[RAYDIUM_MAX_OBJECTS][RAYDIUM_MAX_OBJECT_ANIM_INSTANCES]; // current frame
__global int	 raydium_object_anim_previous[RAYDIUM_MAX_OBJECTS][RAYDIUM_MAX_OBJECT_ANIM_INSTANCES]; // anim switch blending (last anim)
__global GLfloat raydium_object_anim_frame_previous[RAYDIUM_MAX_OBJECTS][RAYDIUM_MAX_OBJECT_ANIM_INSTANCES]; // anim switch blending (last frame)
__global GLfloat raydium_object_anim_frame_previous_timeout[RAYDIUM_MAX_OBJECTS][RAYDIUM_MAX_OBJECT_ANIM_INSTANCES]; // "timer" for anim switch blending
__global int 	 raydium_object_anim_punctually_flag[RAYDIUM_MAX_OBJECTS][RAYDIUM_MAX_OBJECT_ANIM_INSTANCES];

__global int	 raydium_render_fps;
__global GLfloat raydium_render_rgb_force[4];
__global GLfloat raydium_render_lightmap_color_value[4];
__global signed char	 raydium_render_rgb_force_tag;
__global char	 raydium_render_displaylists_tag;
__global GLboolean raydium_render_internal_light_previous_step;

__global signed char	 raydium_fog_enabled_tag;
__global signed char	 raydium_fog_volumetric_enabled_tag;
__global int		 raydium_fog_mode_value;
__global float		 raydium_fog_far_value;
__global float		 raydium_fog_near_value;
__global float		 raydium_fog_density_value;
__global GLfloat *	 raydium_fog_volumetric_array;

__global signed char	raydium_light_enabled_tag;
__global signed char    raydium_light_internal_state[RAYDIUM_MAX_LIGHTS];
__global GLfloat raydium_light_position[RAYDIUM_MAX_LIGHTS][4];
__global GLfloat raydium_light_color[RAYDIUM_MAX_LIGHTS][4];
__global GLfloat raydium_light_intensity[RAYDIUM_MAX_LIGHTS];
__global GLfloat raydium_light_blink_low[RAYDIUM_MAX_LIGHTS];
__global GLfloat raydium_light_blink_high[RAYDIUM_MAX_LIGHTS];
__global GLfloat raydium_light_blink_increment[RAYDIUM_MAX_LIGHTS];
// TODO: light_spot

__global signed char    raydium_internal_vertex_next_extras;
__global GLfloat raydium_internal_vertex_next_u;
__global GLfloat raydium_internal_vertex_next_v;
__global GLfloat raydium_internal_vertex_next_nx;
__global GLfloat raydium_internal_vertex_next_ny;
__global GLfloat raydium_internal_vertex_next_nz;

__global signed char	 raydium_frame_first_camera_pass;
__global float	 raydium_frame_time;
__global GLfloat raydium_camera_x;
__global GLfloat raydium_camera_y; // read only, undocumented.
__global GLfloat raydium_camera_z;
__global signed char	 raydium_camera_pushed;
__global GLfloat raydium_camera_cursor_place[3];
__global GLfloat raydium_camera_look_at_roll;
__global GLfloat raydium_camera_rumble_amplitude;
__global GLfloat raydium_camera_rumble_evolution;
__global GLfloat raydium_camera_rumble_remaining;

typedef struct raydium_camera_Path
    {
    char    name[RAYDIUM_MAX_NAME_LEN];
    GLfloat x[RAYDIUM_MAX_CAMERA_PATH_STEPS];
    GLfloat y[RAYDIUM_MAX_CAMERA_PATH_STEPS];
    GLfloat z[RAYDIUM_MAX_CAMERA_PATH_STEPS];
    GLfloat zoom[RAYDIUM_MAX_CAMERA_PATH_STEPS];
    GLfloat roll[RAYDIUM_MAX_CAMERA_PATH_STEPS];
    int steps;
    } raydium_camera_Path;
__global raydium_camera_Path raydium_camera_path[RAYDIUM_MAX_CAMERA_PATHS];
__global signed char raydium_camera_path_reset_flag;

__global int 	raydium_network_socket;
__global int 	raydium_network_uid;
__global signed char	raydium_network_mode;
__global signed char	raydium_network_client[RAYDIUM_NETWORK_MAX_CLIENTS];
__global time_t raydium_network_start;
__global struct sockaddr
	raydium_network_client_addr[RAYDIUM_NETWORK_MAX_CLIENTS];
__global time_t raydium_network_keepalive[RAYDIUM_NETWORK_MAX_CLIENTS];
__global char   raydium_network_name_local[RAYDIUM_MAX_NAME_LEN];
__global char   raydium_network_name[RAYDIUM_NETWORK_MAX_CLIENTS][RAYDIUM_MAX_NAME_LEN];
__global char   raydium_network_connected_server[RAYDIUM_MAX_NAME_LEN];
__global int    raydium_network_netcall_type[RAYDIUM_NETWORK_MAX_NETCALLS];
__global void * raydium_network_netcall_func[RAYDIUM_NETWORK_MAX_NETCALLS];
__global signed char   raydium_network_netcall_tcp[RAYDIUM_NETWORK_MAX_NETCALLS];
__global void * raydium_network_on_connect;
__global void * raydium_network_on_disconnect;
__global int    raydium_network_stat_rx;
__global int    raydium_network_stat_tx;
__global int    raydium_network_stat_reemitted;
__global int    raydium_network_stat_double;
__global int    raydium_network_stat_lost;
__global int    raydium_network_stat_bogus_ack;

typedef struct raydium_network_Tcp
    {
    signed char state;
    unsigned short tcpid;
    char packet[RAYDIUM_NETWORK_PACKET_SIZE];
    unsigned long time;
    unsigned short retries_left;
    struct sockaddr to;
    int to_player;
    } raydium_network_Tcp;

__global raydium_network_Tcp raydium_network_queue[RAYDIUM_NETWORK_TX_QUEUE_SIZE];
__global int raydium_network_queue_index;

__global unsigned short raydium_network_tcpid_i[RAYDIUM_NETWORK_TX_QUEUE_SIZE]; // ID
__global int            raydium_network_tcpid_p[RAYDIUM_NETWORK_TX_QUEUE_SIZE]; // Player
__global int		raydium_network_tcpid_index;

__global unsigned long raydium_netwok_queue_ack_delay_client;
__global unsigned long raydium_netwok_queue_ack_delay_server[RAYDIUM_NETWORK_MAX_CLIENTS];
__global signed char	       raydium_network_write_notcp;

typedef struct raydium_network_Propag
    {
    signed char state;
    int type;
    unsigned short size;
    unsigned int version;
    void *data;    
    } raydium_network_Propag;

__global raydium_network_Propag raydium_network_propag[RAYDIUM_NETWORK_MAX_PROPAGS];

typedef struct raydium_network_Beacon
    {
    int id;
    char name[RAYDIUM_MAX_NAME_LEN];
    char ip[RAYDIUM_MAX_NAME_LEN];
    char info[RAYDIUM_NETWORK_BEACON_INFO_MAX_LEN];
    int player_count;
    int player_max;
    unsigned long when; // 0 means "free"
    } raydium_network_Beacon;

typedef struct raydium_network_BeaconSearch
    {
    signed char active;
    char app_or_mod[RAYDIUM_MAX_NAME_LEN];
    int version;
    } raydium_network_BeaconSearch;

__global raydium_network_Beacon raydium_network_server_list[RAYDIUM_NETWORK_MAX_SERVERS];
__global char raydium_network_beacon[RAYDIUM_NETWORK_PACKET_SIZE];
__global int  raydium_network_beacon_info_offset;
__global raydium_network_BeaconSearch raydium_network_beacon_search;

#ifdef linux
#define RAYDIUM_NETWORK_BROADCAST_INTERFACE_MAX	8
__global int raydium_network_broadcast_interface_index;
__global struct sockaddr_in raydium_network_broadcast_interfaces[RAYDIUM_NETWORK_BROADCAST_INTERFACE_MAX];
#endif

__global ALuint  raydium_sound_buffer[RAYDIUM_SOUND_NUM_BUFFERS];
__global ALuint  raydium_sound_source[RAYDIUM_SOUND_NUM_SOURCES];
__global ALfloat raydium_sound_source_fade_factor[RAYDIUM_SOUND_NUM_SOURCES];
__global char    raydium_sound_source_fade_tofile[RAYDIUM_SOUND_NUM_SOURCES][RAYDIUM_MAX_NAME_LEN];
__global ALfloat raydium_sound_DefaultReferenceDistance;
__global int 	 raydium_sound;
__global int 	 raydium_sound_top_buffer;
__global char    raydium_sound_music_buf[SOUNDDATASIZE];
__global FILE   *raydium_sound_music_file;
__global OggVorbis_File raydium_sound_vf;
__global vorbis_info   *raydium_sound_ogginfo;
__global int 	(*raydium_sound_music_eof_callback)(char *);
__global void 	(*raydium_sound_music_changed_callback)(void);

typedef struct raydium_sound_music_Info {
    char artist[RAYDIUM_MAX_NAME_LEN];
    char title [RAYDIUM_MAX_NAME_LEN];
    char album [RAYDIUM_MAX_NAME_LEN];
} raydium_sound_music_Info;

__global raydium_sound_music_Info raydium_sound_music_info;

__global GLfloat raydium_osd_logo_angle;
__global GLuint  raydium_osd_cursor_texture;
__global GLfloat raydium_osd_cursor_xsize;
__global GLfloat raydium_osd_cursor_ysize;
__global GLfloat raydium_osd_color[4];
#ifdef MAIN_C
__global GLfloat raydium_osd_ega[]=
         {
         0.0f, 0.0f, 0.0f, // 0: black
         0.0f, 0.0f, 0.6f, // 1: blue
         0.0f, 0.6f, 0.0f, // 2: green
         0.0f, 0.6f, 0.6f, // 3: cyan
         0.6f, 0.0f, 0.0f, // 4: red
         0.6f, 0.0f, 0.6f, // 5: purple
         0.6f, 0.3f, 0.0f, // 6: brown
         0.6f, 0.6f, 0.6f, // 7: white
         0.3f, 0.3f, 0.3f, // 8: grey
         0.3f, 0.3f, 1.0f, // 9: light blue
         0.3f, 1.0f, 0.3f, // A: light green
         0.3f, 1.0f, 1.0f, // B: light cyan
         1.0f, 0.3f, 0.3f, // C: light red
         1.0f, 0.3f, 1.0f, // D: light purple
         1.0f, 1.0f, 0.3f, // E: light yellow
         1.0f, 1.0f, 1.0f  // F: light white
	 };
#else
__global GLfloat raydium_osd_ega[48];
#endif
__global GLfloat raydium_osd_fade_color_timeleft;
__global GLfloat raydium_osd_fade_color_increment[4];
__global GLfloat raydium_osd_fade_color_current[4];
__global void *  raydium_osd_fade_OnFadeEnd;

__global GLfloat raydium_console_pos;
__global GLfloat raydium_console_inc;
__global GLfloat raydium_console_config_max;
__global GLfloat raydium_console_config_speed;
__global char    raydium_console_config_texture[RAYDIUM_MAX_NAME_LEN];
__global char    raydium_console_config_font[RAYDIUM_MAX_NAME_LEN];
__global GLfloat raydium_console_cursor_blink;
__global char	 raydium_console_lines[RAYDIUM_CONSOLE_MAX_LINES][RAYDIUM_MAX_NAME_LEN];
__global int	 raydium_console_line_last;
__global char    raydium_console_get_string[RAYDIUM_MAX_NAME_LEN];
__global char    raydium_console_get_string_last[RAYDIUM_MAX_NAME_LEN];
__global void *	 raydium_console_gets_callback;
__global char	 raydium_console_history[RAYDIUM_CONSOLE_MAX_HISTORY][RAYDIUM_MAX_NAME_LEN];
__global int	 raydium_console_history_index; // store
__global int	 raydium_console_history_index_current; // user
__global char    raydium_console_history_filename[RAYDIUM_MAX_NAME_LEN];

__global int		raydium_timecall_index;
__global signed char          	raydium_timecall_method;
__global unsigned long 	raydium_timecall_max_frequency;
__global unsigned long 	raydium_timecall_clocks_per_sec;
__global int           	raydium_timecall_devrtc_handle;
__global unsigned long  raydium_timecall_devrtc_clocks;
__global void *	      	raydium_timecall_funct[RAYDIUM_MAX_TIMECALLS];
__global GLint 		raydium_timecall_soft_call[RAYDIUM_MAX_TIMECALLS];
__global clock_t 	raydium_timecall_interval[RAYDIUM_MAX_TIMECALLS];
__global clock_t 	raydium_timecall_next[RAYDIUM_MAX_TIMECALLS];
__global int            raydium_timecall_w32_divmodulo;

__global signed char raydium_shadow_tag;
__global signed char raydium_shadow_rendering;
__global int raydium_shadow_ground_mesh;

__global signed char raydium_capture_asked;
__global char raydium_capture_filename[RAYDIUM_MAX_NAME_LEN];

__global signed char raydium_hdr_state;
__global signed char raydium_hdr_generated;
__global int   raydium_hdr_texture_id;
__global float raydium_hdr_eye;
__global float raydium_hdr_eye_speed;
__global float raydium_hdr_alpha_max;
__global GLfloat raydium_hdr_color_local[4];
__global GLfloat raydium_hdr_color_ambient[4];
__global unsigned char *raydium_hdr_mem;
__global unsigned char *raydium_hdr_mem_hdr;
__global unsigned char *raydium_hdr_mem_hdr2;
__global unsigned char *raydium_hdr_mem_hdr3;

__global int    raydium_register_variable_index;
__global int    raydium_register_function_index;
__global char   raydium_register_variable_name[RAYDIUM_MAX_REG_VARIABLES][RAYDIUM_MAX_NAME_LEN];
__global void * raydium_register_variable_addr[RAYDIUM_MAX_REG_VARIABLES];
__global int    raydium_register_variable_type[RAYDIUM_MAX_REG_VARIABLES];
#ifdef PHP_SUPPORT
#define ZFE zend_function_entry
#else
#define ZFE void *
#endif
__global ZFE raydium_register_function_list[RAYDIUM_MAX_REG_FUNCTION];

__global char raydium_php_rayphp_path[RAYDIUM_MAX_NAME_LEN];

__global FILE *raydium_log_file;
__global char raydium_file_log_fopen[RAYDIUM_MAX_LOG_FOPEN][RAYDIUM_MAX_NAME_LEN];
__global int raydium_file_log_fopen_index;

typedef struct matrix4x4
{
  double  ray[16];
} matrix4x4;

#endif
// EOF
