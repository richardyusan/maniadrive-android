/*
    Raydium - CQFD Corp.
    http://raydium.org/
    License: GPL - GNU General Public License, see "gpl.txt" file.
*/

#ifndef DONT_INCLUDE_HEADERS
#include "index.h"
#else
#include "headers/init.h"

#endif 

// proto
void raydium_ode_init(void);
void raydium_register_api(void);
#ifndef WIN32
void raydium_live_init(void);
#endif
void raydium_fog_init(void);
void raydium_video_init(void);
void raydium_internal_live_close(void);
void raydium_shadow_init(void);
void raydium_hdr_init(void);
void raydium_hdr_texture_reset(void);
void raydium_shader_init(void);
void raydium_web_init(void);

char *raydium_version(void)
{
static char version[RAYDIUM_MAX_NAME_LEN];
static signed char first=1;

if(first)
    {
    first=0;
    sprintf(version,"%i.%03i",RAYDIUM_MAJOR,RAYDIUM_MINOR);
    }

return version;
}

void raydium_init_lights(void)
{
GLuint i;

for(i=0;i<RAYDIUM_MAX_LIGHTS;i++)
raydium_light_reset(i);
raydium_log("lights: OK");
}

void raydium_init_objects(void)
{
GLuint i;

raydium_object_anim_time_factor=1.f;

for(i=0;i<RAYDIUM_MAX_OBJECTS;i++)
raydium_object_reset(i);
raydium_log("objects: OK");
}


void raydium_init_key(void)
{
if(raydium_window_mode==RAYDIUM_RENDERING_NONE)
    return;
glutIgnoreKeyRepeat(1);
memset(raydium_key,0,RAYDIUM_KEYBOARD_SIZE);
raydium_key_last=0;
raydium_key_trace=0;
raydium_log("keyboard: OK");
}

// NEVER tested as it should be ! (used once only for now)
void raydium_init_reset(void)
{
GLuint i;

/*
for(i=1;i<raydium_texture_index;i++) // free all texture buffers
free(raydium_texture_ptr[i]);
*/

raydium_init_lights();
raydium_fog_init();
raydium_init_objects();
raydium_network_init();
raydium_timecall_init();
raydium_particle_init();
raydium_camera_path_init_all();
raydium_osd_fade_init();
raydium_console_init();
raydium_gui_init();
#ifndef WIN32
raydium_live_init();
#endif
raydium_video_init();
raydium_shadow_init();
raydium_hdr_init();
raydium_shader_init();
raydium_web_init();

// Must find a way to delete textures from video card's memory, too...

for(i=0;i<RAYDIUM_MAX_TEXTURES;i++) // reset all textures
{
 raydium_texture_name[i][0]=0;
 raydium_texture_blended[i]=0;
 raydium_texture_nolight[i]=0;
 raydium_texture_env[i]=0;
 raydium_texture_islightmap[i]=0;
 raydium_texture_shader[i]=-1;
 raydium_texture_rgb[0][i]=-1.f;
 raydium_texture_rgb[1][i]=-1.f;
 raydium_texture_rgb[2][i]=-1.f;
 raydium_texture_rgb[3][i]=1.f;
}

raydium_hdr_texture_reset();

raydium_vertex_index=0;
raydium_vertex_offset_triangle=0;

strcpy(raydium_texture_name[0],"dummy.null");

raydium_texture_index=1; // no more texture loaded (0 is not a texture)
raydium_texture_current_main=0; // sets an "invalid" current texture
raydium_texture_current_multi=0; // sets an "invalid" current texture
raydium_texture_current_multi_u=0;
raydium_texture_current_multi_v=0;
raydium_texture_current_env=0; // sets an "invalid" current texture
raydium_texture_to_replace=0; // No texture to erase.. just load it :)
raydium_texture_used_memory=0;
//raydium_texture_filter=RAYDIUM_TEXTURE_FILTER_NONE;
raydium_texture_filter_change(RAYDIUM_TEXTURE_FILTER_NONE);
raydium_rendering_rgb_normal();
raydium_rendering_displaylists_enable();
raydium_render_rgb_force_tag=0;
raydium_render_lightmap_color_value[0]=1;
raydium_render_lightmap_color_value[1]=1;
raydium_render_lightmap_color_value[2]=1;
raydium_render_lightmap_color_value[3]=1;
raydium_render_internal_light_previous_step=-1;
raydium_vertex_counter=0;
raydium_projection=RAYDIUM_PROJECTION_PERSPECTIVE;
raydium_projection_fov=60;
raydium_projection_near=1;
raydium_projection_far=1000;
raydium_projection_left=0;
raydium_projection_right=0;
raydium_projection_bottom=0;
raydium_projection_top=0;
raydium_camera_pushed=0;
raydium_camera_look_at_roll=0;
raydium_camera_path_reset_flag=1;
raydium_camera_rumble_amplitude=0;
raydium_camera_rumble_evolution=0;
raydium_camera_rumble_remaining=0;
raydium_window_view_update();
raydium_internal_vertex_next_extras=0;
raydium_sky_force=0;
raydium_osd_logo_angle=0;
raydium_osd_cursor_texture=0;
raydium_register_variable_index=0;
raydium_register_function_index=0;
raydium_file_log_fopen_index=0;
raydium_frame_time=0;
raydium_capture_asked=RAYDIUM_CAPTURE_NONE;

for(i=0;i<4;i++)
raydium_osd_color[i]=1.f;

raydium_background_color_change(1,1,1,1);

glEnable(GL_TEXTURE_2D);
glEnable(GL_DEPTH_TEST);
glEnable(GL_COLOR_MATERIAL);
glShadeModel(GL_SMOOTH);
glDepthFunc(GL_LEQUAL); // LESS only ? shadow maps loves lequal, so ..
glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);


raydium_light_enable();
glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);  // mmmmm...
glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
raydium_log("Raydium engine reseted to original state");
}



void raydium_init_engine(void)
{
GLenum err;
#ifdef PHP_SUPPORT
char autoexec[RAYDIUM_MAX_NAME_LEN];
#endif

raydium_signal_install_trap();
err=glewInit();
if(err==GLEW_OK)
    raydium_log("OpenGL extensions: OK");
else
    raydium_log("OpenGL extensions: FAILED");
raydium_internal_size_vector_float_4=sizeof(GLfloat)*4;
raydium_log("Platform \"4xfloat\" vector size is: %i byte(s) long",raydium_internal_size_vector_float_4);
glGetIntegerv(GL_MAX_TEXTURE_SIZE, &raydium_texture_size_max);
raydium_log("OpenGL implementation maximum texture size: %ix%i",raydium_texture_size_max,raydium_texture_size_max);
glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &raydium_texture_units);
raydium_log("OpenGL hardware providing %i texture unit(s)",raydium_texture_units);
raydium_vertex_x=malloc(RAYDIUM_MAX_VERTICES*sizeof(GLfloat));
raydium_vertex_y=malloc(RAYDIUM_MAX_VERTICES*sizeof(GLfloat));
raydium_vertex_z=malloc(RAYDIUM_MAX_VERTICES*sizeof(GLfloat));
raydium_vertex_normal_x=malloc(RAYDIUM_MAX_VERTICES*sizeof(GLfloat));
raydium_vertex_normal_y=malloc(RAYDIUM_MAX_VERTICES*sizeof(GLfloat));
raydium_vertex_normal_z=malloc(RAYDIUM_MAX_VERTICES*sizeof(GLfloat));
raydium_vertex_normal_visu_x=malloc(RAYDIUM_MAX_VERTICES*sizeof(GLfloat));
raydium_vertex_normal_visu_y=malloc(RAYDIUM_MAX_VERTICES*sizeof(GLfloat));
raydium_vertex_normal_visu_z=malloc(RAYDIUM_MAX_VERTICES*sizeof(GLfloat));
raydium_vertex_texture_u=malloc(RAYDIUM_MAX_VERTICES*sizeof(GLfloat));
raydium_vertex_texture_v=malloc(RAYDIUM_MAX_VERTICES*sizeof(GLfloat));
raydium_vertex_texture=malloc(RAYDIUM_MAX_VERTICES*sizeof(GLuint));
raydium_vertex_texture_multi=malloc(RAYDIUM_MAX_VERTICES*sizeof(GLuint));
raydium_vertex_texture_multi_u=malloc(RAYDIUM_MAX_VERTICES*sizeof(GLfloat));
raydium_vertex_texture_multi_v=malloc(RAYDIUM_MAX_VERTICES*sizeof(GLfloat));
raydium_vertex_texture_env=malloc(RAYDIUM_MAX_VERTICES*sizeof(GLuint));
raydium_vertex_tag=malloc(RAYDIUM_MAX_VERTICES);
// must test more than just the last "big" malloc result..
if(!raydium_vertex_texture) { raydium_log("Out of memory..."); exit(29); }
raydium_log("vertex arrays memory: OK");
raydium_path_init(); // do this ASAP, before any file is opened
raydium_random_randomize();
raydium_init_key();
raydium_mouse_init();
raydium_joy_init(); // and not init_joy, since defined in joy.c, not init.c
raydium_sound=0;
raydium_sound_init();
raydium_callback_set();
#ifdef PHP_SUPPORT
raydium_php_init();
#endif

raydium_atexit(raydium_sound_close);
raydium_atexit(raydium_joy_close);
raydium_atexit(raydium_network_close);
raydium_atexit(raydium_internal_dump);
raydium_atexit(raydium_console_history_save);
#ifndef WIN32
raydium_atexit(raydium_internal_live_close);
#endif
raydium_log("atexit functions: OK");
raydium_init_reset();
#ifdef ODE_SUPPORT
raydium_ode_init();
#endif
raydium_register_api();
raydium_log("Engine is now ready.\n\t -----------------------------------------------------------");

#ifdef PHP_SUPPORT
if(raydium_init_cli_option("autoexec",autoexec))
    raydium_php_exec(autoexec);
#endif
}



