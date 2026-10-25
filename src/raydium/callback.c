/*
    Raydium - CQFD Corp.
    http://raydium.org/
    License: GPL - GNU General Public License, see "gpl.txt" file.
*/

#ifndef DONT_INCLUDE_HEADERS
#include "index.h"
#else
#include "headers/callback.h"
#endif 

void raydium_osd_cursor_draw(void);
void raydium_console_draw(void);
void raydium_gui_draw(void);
void raydium_osd_fade_callback(void);
#ifdef PHP_SUPPORT
int  raydium_php_exec(char *);
#endif
#ifdef ODE_SUPPORT
void raydium_ode_network_read(void);
#endif
void raydium_network_read_faked(void);
void raydium_video_callback(void);
void raydium_internal_live_video_callback(void);
void raydium_object_callback(void);
void raydium_web_callback(void);
void raydium_hdr_map(void);
void raydium_hdr_map_apply(void);


void raydium_callback_image(void)
{
raydium_timecall_callback();
raydium_light_callback();
raydium_particle_draw_all();
raydium_hdr_map_apply();
raydium_particle_callback();
raydium_osd_fade_callback();
raydium_gui_draw();
raydium_console_draw();
raydium_osd_cursor_draw();
raydium_joy_callback();
raydium_sound_callback();
#ifdef ODE_SUPPORT
// 0hz ODE callback workaround
raydium_ode_network_read();
#endif

if(raydium_network_mode==RAYDIUM_NETWORK_MODE_DISCOVER)
    raydium_network_read_faked();

#ifndef WIN32
raydium_internal_live_video_callback();
#endif
raydium_video_callback();
raydium_web_callback();
raydium_object_callback();
}


void raydium_callback_set(void)
{
if(raydium_window_mode==RAYDIUM_RENDERING_NONE)
    return;
glutReshapeFunc(&raydium_window_resize_callback);
glutKeyboardFunc((void *)raydium_key_normal_callback);
glutSpecialUpFunc((void *)raydium_key_special_up_callback);
glutSpecialFunc((void *)raydium_key_special_callback);
glutMotionFunc((void *)raydium_mouse_move_callback);
glutPassiveMotionFunc((void *)raydium_mouse_move_callback);
glutMouseFunc((void *)raydium_mouse_click_callback);
}


void raydium_callback(void (*loop) )
{
#ifdef PHP_SUPPORT
char autoexec[RAYDIUM_MAX_NAME_LEN];

if(raydium_init_cli_option("autoexec2",autoexec))
    raydium_php_exec(autoexec);
#endif
glutDisplayFunc(loop);
glutIdleFunc(loop);
glutMainLoop();
}
