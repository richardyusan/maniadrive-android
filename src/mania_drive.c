// ManiaDrive - CQFD Corp
// http://maniadrive.raydium.org/
char *version="ManiaDrive 1.2custom";

#include "raydium/index.c"

#include "mania.h"
#include "mania_creds.h"

#define NO_NETWORK_COLLISIONS

#ifdef WIN32
#define SERVER_BINARY "mania_server.exe"
#else
#define SERVER_BINARY "./mania_server.static"
#endif

#define STORY_FILE_BEG		"mania_drive.story.beg"
#define STORY_FILE_PRO		"mania_drive.story.pro"
#define HISTORY_STATE_FILE	raydium_file_home_path("mania_drive.state")

#define MUSIC_MENU	"mania_music/i_got_it_bad_-_The_Napoleon_Blown_Aparts.ogg"
#define RESOLUTION_LIST "320x240\n640x480\n800x600\n1024x768"

GLfloat sun[]={1.0,0.9,0.5,1.0};
GLfloat amb[]={1.0,0.0,0.0,1.0};
GLfloat tmp2[]={1.0,1.0,1.0,1.0};
GLfloat zero[]={0.0,0.0,0.0,0.0};

GLfloat temps[]={0,0,0};

int sound_car;
int sound_checkpoint;
int sound_owww;
int sound_wizz;
char vue=3;
GLfloat camx=-20;
GLfloat camy=50;
GLfloat camz=40;
char cam[255];
char draw_debug=-1;
char is_explosive_tag=29;
dReal cam_angle_h=0;
dReal cam_angle_v=90;
float scroll=-1;

signed char camera_lag;
float camera_lag_speed;
signed char shadow_support;
signed char windowed_mode;
char windowed_res[32]; // "%ix%i" format
signed char windowed_mode_save; // use to display "must restart game" message
char windowed_res_save[32]; // same
float music_volume;
signed joystick_enabled;

int object_ground,object_boxes;
int game_state;
GLfloat timer;
GLfloat countdown;
GLfloat partytime;
GLfloat yourbest;
char mni_current[RAYDIUM_MAX_NAME_LEN];
GLfloat track_bottom;
char mni_next[RAYDIUM_MAX_NAME_LEN];
char message[RAYDIUM_MAX_NAME_LEN];
signed char mode;
signed char simple_mni=0;
signed char congrats=0;

signed char music_popup_inc=-1;

dReal gears[]={-0.4,0.4,0.6,0.8,0.9,1.0};
int gear=1;

void change_game_state(int type);
void create_car(void);
void load_boxes(char *filename);
void dat_load(char *filename);
void hms(GLfloat t,int *h, int *m, int *s, int *ms);
int mni_load(char *mni);
signed char mni_network_get(char *mni);
void build_gui_Main(void);
void music_random(void);
void leave(void);
void build_gui_ErrorDownload(void);
void build_gui_Lan(void);
void btnBackToMainMenu(raydium_gui_Object *w);
void build_gui_NeedRestart(void);
void build_gui_Story(void);
void showMessage(char *file, int id);
void build_gui_Solo(void);


#define NET_SCORE_TRACK	(RAYDIUM_NETWORK_PACKET_BASE+1)
#define NET_RESTART	(RAYDIUM_NETWORK_PACKET_BASE+2)
#define NET_CHANGE_MAP	(RAYDIUM_NETWORK_PACKET_BASE+3)
#define NET_REMAINING	(RAYDIUM_NETWORK_PACKET_BASE+4)
#define NET_SCORE_BASE	(RAYDIUM_NETWORK_PACKET_BASE+10)


typedef struct Score
    {
    float time;
    char player[RAYDIUM_MAX_NAME_LEN];
    } Score;

typedef struct TrackData
    {
    char name[RAYDIUM_MAX_NAME_LEN];
    char author[RAYDIUM_MAX_NAME_LEN];
    float gold_time;
    float author_time;
    char message_file[RAYDIUM_MAX_NAME_LEN];
    } TrackData;

TrackData trackdata;
Score best_score[RAYDIUM_NETWORK_MAX_CLIENTS];
Score track_score;

pid_t server_pid=0;

#define GAME_COUNTDOWN	1
#define GAME_GAME	2
#define GAME_END	3
#define GAME_SCORE	4

#define MODE_NONE	0
#define MODE_SOLO	1
#define MODE_NET	2
#define MODE_MULTI	3

#define SHADOW_OFFSET	0.3


void change_music_volume(float vol)
{
raydium_sound_SetSourceGain(0,vol);
}

float get_score(char *track,char *player)
{
char ltrack[RAYDIUM_MAX_NAME_LEN];
float res=0;

if(!strcmp(track,"dyn_track.mni"))
    {
    player[0]=0;
    return 0;
    }

// since PHP will write to variables ..
strcpy(ltrack,track);
strcpy(player,"unavailable");

raydium_register_variable(ltrack,RAYDIUM_REGISTER_STR,"track");
raydium_register_variable(player,RAYDIUM_REGISTER_STR,"player");
raydium_register_variable(&res,RAYDIUM_REGISTER_FLOAT,"score");

raydium_php_exec("mania_score_get.php");

raydium_register_variable_unregister_last();
raydium_register_variable_unregister_last();
raydium_register_variable_unregister_last();
return res;
}


int post_score(char *track, char *name, char *score)
{
char ltrack[RAYDIUM_MAX_NAME_LEN];
char lname[RAYDIUM_MAX_NAME_LEN];
char lscore[RAYDIUM_MAX_NAME_LEN];
char lversion[RAYDIUM_MAX_NAME_LEN];
int position;

if(!strcmp(track,"dyn_track.mni"))
    return 0;

// since PHP will write to variables ..
strcpy(ltrack,track);
strcpy(lname,name);
strcpy(lscore,score);
strcpy(lversion,version);

raydium_register_variable(ltrack,RAYDIUM_REGISTER_STR,"track");
raydium_register_variable(lname,RAYDIUM_REGISTER_STR,"name");
raydium_register_variable(lscore,RAYDIUM_REGISTER_STR,"score");
raydium_register_variable(lversion,RAYDIUM_REGISTER_STR,"version");
raydium_register_variable(&position,RAYDIUM_REGISTER_INT,"position");

raydium_php_exec("mania_score.php");

raydium_register_variable_unregister_last();
raydium_register_variable_unregister_last();
raydium_register_variable_unregister_last();
raydium_register_variable_unregister_last();
raydium_register_variable_unregister_last();

return position;
}


void post_score_local(char *track, float time)
{
FILE *fp;

fp=fopen(HISTORY_STATE_FILE,"at");
if(!fp)
    {
    raydium_log("ERROR: cannot save story state");
    return;
    }
fprintf(fp,"%s;%f\n",track,time);
fclose(fp);
}

signed char get_tracklist(char *list)
{
int ok;

raydium_register_variable(list,RAYDIUM_REGISTER_STR,"list");
raydium_register_variable(&ok,RAYDIUM_REGISTER_INT,"ok");
raydium_php_exec("mania_tracklist.php");
raydium_register_variable_unregister_last();
raydium_register_variable_unregister_last();
return ok;
}


void restart_asked_from_network(int type,char *buff)
{
create_car();
}

void netcall_mni_change(int type, char *buff)
{
if(!mni_network_get(buff+RAYDIUM_NETWORK_PACKET_OFFSET))
	return;
mni_load(buff+RAYDIUM_NETWORK_PACKET_OFFSET);
}

void netcall_remaining_time(int type, char *buff)
{
float f;
char *trck;
memcpy(&f,buff+RAYDIUM_NETWORK_PACKET_OFFSET,sizeof(float));

trck=buff+RAYDIUM_NETWORK_PACKET_OFFSET+sizeof(float);
if(strlen(mni_current)==0 || strcmp(mni_current,trck))
    {
    if(!mni_network_get(trck))
	return;
    mni_load(trck);
    }
partytime=f;
}


signed char mni_network_get(char *mni)
{
// download track from server, and exit multiplayer mode in case of failure,
// displaying an error window
if(raydium_web_client_get(mni))
    return 1; // ok
// failed
leave();
raydium_gui_window_delete_name("menu");
build_gui_Lan();
build_gui_ErrorDownload();
return 0;
}


int mni_load(char *mni)
{
char tri[RAYDIUM_MAX_NAME_LEN];
char base[RAYDIUM_MAX_NAME_LEN];
GLfloat min[3];
GLfloat max[3];
int i,ret;

if(!mni || !strlen(mni))
    return 0;

ret=mni_generate(mni);
if(ret==0)
    {
    raydium_log("CANNOT GENERATE TRACK FROM MNI FILE '%s'",mni);

    if(simple_mni)
	exit(1);
    
    return 0;
    }

raydium_ode_object_delete_name("WATURE");

strcpy(mni_current,mni);
raydium_file_basename(base,mni);
sprintf(tri,"%s/mania_%s.tri",raydium_homedir,base);
rename(raydium_file_home_path("mania.tri"),tri);
raydium_ode_ground_set_name(tri);
unlink(tri);

load_boxes(raydium_file_home_path("mania.box"));
dat_load(raydium_file_home_path("mania.dat"));

raydium_object_find_minmax(raydium_object_find(tri),min,max);
track_bottom=min[2];

if(mode!=MODE_SOLO)
    track_score.time=get_score(mni,track_score.player);

raydium_network_propag_refresh(NET_SCORE_TRACK);
for(i=0;i<RAYDIUM_NETWORK_MAX_CLIENTS;i++)
    best_score[i].time=-1;

if(shadow_support)
    raydium_shadow_enable();
else
    raydium_shadow_disable();

create_car();

partytime=0;
yourbest=0;

raydium_gui_window_delete_name("menu");

if(strlen(trackdata.message_file)==0)
    {
    raydium_gui_hide();
    raydium_osd_cursor_set(NULL,0,0); // hide mouse cursor
    raydium_sound_load_music(NULL);
    }
else
    showMessage(trackdata.message_file,0);

raydium_video_delete_name("video");
music_random();
return 1;
}

void btnBestTimeOk(raydium_gui_Object *w)
{
raydium_gui_window_delete_name("besttime");
}

void btnCantDriveOk(raydium_gui_Object *w)
{
raydium_gui_window_delete_name("cantdrive");
}

void btnKeyHelpOk(raydium_gui_Object *w)
{
char str[RAYDIUM_MAX_NAME_LEN]; // useless
joystick_enabled=raydium_gui_read_name("keyhelp","chkJoy",str);
raydium_parser_db_set("ManiaDrive-JoystickEnabled",(joystick_enabled?"y":"n"));
raydium_gui_window_delete_name("keyhelp");
}


void AfterFade(void)
{
int handle;

if(!mni_load(mni_next))
    {
    handle=raydium_gui_window_create("menu",25,45,50,15);
    raydium_gui_widget_sizes(0,0,18);
    raydium_gui_label_create("lblError",handle,50,80,"Error loading track ...",0,0,0);
    raydium_gui_widget_sizes(15,5,18);
    raydium_gui_button_create("btnErrorOk",handle,35,15,"OK",btnBackToMainMenu);
    raydium_sound_load_music(MUSIC_MENU);
    }
mni_next[0]=0;
}

void drive(char *track)
{
GLfloat from[4]={0,0,0,0};
GLfloat to[4]={0,0,0,1};

strcpy(mni_next,track);
raydium_sound_source_fade(0,1);
raydium_osd_fade_from(from,to,1,AfterFade);
raydium_gui_window_delete_name("menu");
raydium_gui_window_delete_name("besttime");
}

void gui_start(void)
{
if(raydium_gui_isvisible())
    return;

raydium_gui_show();
raydium_sound_load_music(MUSIC_MENU);
change_music_volume(music_volume);
raydium_video_open("mania_menu_v1.jpgs","video");
raydium_osd_cursor_set("BOXmania_cursor.tga",4,6);
raydium_ode_object_delete_name("WATURE");
}

void credits_start(void)
{
// shhhh ... :)
raydium_video_open("mania_creds.jpd","video2");
raydium_sound_source_fade_to(0,1,"mania_creds.ogd");
scroll=0;
}

void credits_stop(void)
{
raydium_video_delete_name("video2");
raydium_sound_source_fade_to(0,1,MUSIC_MENU);
build_gui_Main();
scroll=-1;
}

void after_gui(void)
{
const int px=14;
const int py=13;
const int size=30;
static float alpha=1;

if(raydium_video_find("video_beg")>=0)
    {
    if(!raydium_video_isplaying_name("video_beg"))
	alpha-=raydium_frame_time;

    raydium_live_texture_draw_name("video_beg",alpha,px,py,px+size,py+size);	
    
    if(alpha<=0)
	raydium_video_delete_name("video_beg");
    }
else
    alpha=1;
}

void video_beg_delete(void)
{
raydium_video_delete_name("video_beg");
}

void btnDriveNet(raydium_gui_Object *w)
{
char track[RAYDIUM_MAX_NAME_LEN];

raydium_gui_read_name("menu","cboTrack",track);
raydium_gui_read_name("menu","edtPlayerName",raydium_network_name_local);

raydium_parser_db_set("Generic-PlayerName",raydium_network_name_local);
raydium_parser_db_set("ManiaDrive-LastInternetTrack",track);

strcat(track,".mni");
drive(track);
mode=MODE_NET;
}

void btnDriveSolo(raydium_gui_Object *w)
{
drive(w->name);
mode=MODE_SOLO;
}


void btnOtherTracksSolo(raydium_gui_Object *w)
{
char track[RAYDIUM_MAX_NAME_LEN];

raydium_gui_read_name("menu","cboTrack",track);
raydium_parser_db_set("ManiaDrive-LastSoloTrack",track);

drive(track);
mode=MODE_SOLO;
}


void btnBackToMainMenu(raydium_gui_Object *w)
{
change_music_volume(music_volume);
raydium_gui_window_delete_name("menu");
build_gui_Main();
}

void btnCantDrive(raydium_gui_Object *w)
{
int handle;
char *str="You can't drive here yet !";

handle=raydium_gui_window_create("cantdrive",25,45,50,15);
raydium_gui_widget_sizes(0,0,18);
raydium_gui_label_create("lblCantDrive",handle,50,80,str,0,0,0);
raydium_gui_widget_sizes(15,5,18);
raydium_gui_button_create("btnCantDriveOk",handle,35,15,"OK",btnCantDriveOk);
}

void btnKeyHelp(raydium_gui_Object *w)
{
int handle;

handle=raydium_gui_window_create("keyhelp",5,45,50,50);

raydium_gui_widget_sizes(5,5,25);
raydium_gui_label_create("lblTitle",handle,50,90,"- Controls Help -",0.7,0,0);

raydium_gui_widget_sizes(0,0,16);
raydium_gui_label_create("lblKH10",handle,50,80,"Key Up : accelerate",0,0,0);
raydium_gui_label_create("lblKH20",handle,50,75,"Key Down : brake / reverse",0,0,0);

raydium_gui_label_create("lblKH30",handle,50,65,"Space : restart track",0,0,0);
raydium_gui_label_create("lblKH40",handle,50,60,"Backspace : restart LAN clients",0,0,0);

raydium_gui_label_create("lblKH50",handle,50,50,"F3 : default external view",0,0,0);
raydium_gui_label_create("lblKH60",handle,50,45,"F6 : in-board fixed view",0,0,0);
raydium_gui_label_create("lblKH65",handle,50,40,"C : switch camera",0,0,0);

raydium_gui_label_create("lblKH70",handle,50,30,"... and maybe hidden ones :)",0,0,0);

raydium_gui_widget_sizes(3,3,16);
if(raydium_joy)
    {
    raydium_gui_check_create("chkJoy",handle,15,17," joystick/joypad enabled",joystick_enabled);
    }
else
    {
    raydium_gui_label_create("lblKH80",handle,50,20,"(no joystick/joypad found)",0.3,0,0);
    }

raydium_gui_widget_sizes(15,5,18);
raydium_gui_button_create("btnCantDriveOk",handle,35,5,"OK",btnKeyHelpOk);
}


void btnErrorOkClick(raydium_gui_Object *w)
{
raydium_gui_window_delete_name("error");
}

void btnStoryCompletedOk(raydium_gui_Object *w)
{
raydium_gui_window_delete_name("storycompleted");
build_gui_Story();
}

void btnRestartClick(raydium_gui_Object *w)
{
raydium_gui_window_delete_name("restart");
}


void btnNetworkConnect(raydium_gui_Object *w)
{
char server[512];
int handle;

raydium_gui_read_name("menu","edtServer",server);
raydium_gui_read_name("menu","edtPlayerName",raydium_network_name_local);

raydium_parser_db_set("ManiaDrive-Server",server);
raydium_parser_db_set("Generic-PlayerName",raydium_network_name_local);


if(!raydium_network_client_connect_to(server))
    {
    handle=raydium_gui_window_create("error",25,45,50,15);
    raydium_gui_widget_sizes(0,0,18);
    raydium_gui_label_create("lblError",handle,50,80,"Cannot connect to server ...",0,0,0);
    raydium_gui_widget_sizes(15,5,18);
    raydium_gui_button_create("btnErrorOk",handle,35,15,"OK",btnErrorOkClick);
    return;
    }

mode=MODE_MULTI;
raydium_gui_window_delete_name("menu");
handle=raydium_gui_window_create("menu",25,45,50,15);
raydium_gui_widget_sizes(0,0,18);
raydium_gui_label_create("lblInfo",handle,50,50,"Please, wait ...",0,0,0);
}

void btnServerClick(raydium_gui_Object *w)
{
char str[RAYDIUM_MAX_NAME_LEN];
int zone;
char name[RAYDIUM_MAX_NAME_LEN];
char ip[RAYDIUM_MAX_NAME_LEN];
char info[RAYDIUM_MAX_NAME_LEN];
int player_count;
int player_max;

zone=raydium_gui_read_widget(w,str);
if(raydium_network_discover_getserver(zone,name,ip,info,&player_count,&player_max)<=0)
    return; // hu ?!

raydium_gui_widget_sizes(0,0,0);
raydium_gui_edit_create("edtServer",w->window,0,0,ip);
raydium_gui_edit_create("edtPlayerName",w->window,0,0,raydium_network_name_local);
btnNetworkConnect(NULL);
}

void btnSearchLAN(raydium_gui_Object *w)
{
int i,n;
int handle;
float y;
char str[RAYDIUM_MAX_NAME_LEN];

char name[RAYDIUM_MAX_NAME_LEN];
char ip[RAYDIUM_MAX_NAME_LEN];
char info[RAYDIUM_MAX_NAME_LEN];
int player_count;
int player_max;

raydium_gui_window_delete_name("menu");
handle=raydium_gui_window_create("menu",10,25,80,50);

raydium_gui_widget_sizes(0,0,18);
raydium_gui_label_create("lblLanServerSearch",handle,50,92,"- LAN Servers - ",0,0,0);

raydium_gui_widget_sizes(0,0,12);

sprintf(str,"%-30s","Name");
raydium_gui_label_create("lblCol1",handle,20,85,str,0.4,0,0);
sprintf(str,"%-15s","Track");
raydium_gui_label_create("lblCol2",handle,50,85,str,0.4,0,0);
sprintf(str,"%-15s","Address");
raydium_gui_label_create("lblCol3",handle,70,85,str,0.4,0,0);
sprintf(str,"%-15s","Players");
raydium_gui_label_create("lblCol4",handle,90,85,str,0.4,0,0);

sprintf(str,"%-30s","------------------------------");
raydium_gui_label_create("lblCol1s",handle,20,83,str,0.4,0,0);
sprintf(str,"%-15s","---------------");
raydium_gui_label_create("lblCol2s",handle,50,83,str,0.4,0,0);
sprintf(str,"%-15s","---------------");
raydium_gui_label_create("lblCol3s",handle,70,83,str,0.4,0,0);
sprintf(str,"%-15s","---------------");
raydium_gui_label_create("lblCol4s",handle,90,83,str,0.4,0,0);

y=80;
n=raydium_network_discover_numservers();

for(i=0;i<n;i++)
    {
    raydium_network_discover_getserver(i,name,ip,info,&player_count,&player_max);

    sprintf(str,"%30s",name);
    str[30]=0;
    raydium_gui_label_create("*lblCol1s",handle,20,y,str,0,0,0);
    sprintf(str,"%15s",info);
    str[15]=0;
    raydium_gui_label_create("*lblCol2s",handle,50,y,str,0,0,0);
    sprintf(str,"%15s",ip);
    raydium_gui_label_create("*lblCol3s",handle,70,y,str,0,0,0);
    sprintf(str,"          %02i/%02i",player_count,player_max);
    raydium_gui_label_create("*lblCol4s",handle,90,y,str,0,0,0);

    raydium_gui_zone_create("*zone",handle,0,y-2,100,5,i,btnServerClick);

    y-=6;
    }

raydium_gui_widget_sizes(3,3,12);
raydium_gui_button_create("btnBackToMain",handle,2,5,"<",btnBackToMainMenu);
}

// NON-ACTIVATED CODE (child "thread" is never killed)
void btnCreateServer(raydium_gui_Object *w)
{
// no fork under win32 !
/*
server_pid=fork();

if(server_pid==0)
    {
    // child
    execl(SERVER_BINARY,SERVER_BINARY,NULL);
    // never returns ...
    }
*/
}


void btnApplyOptions(raydium_gui_Object *w)
{
char str[RAYDIUM_MAX_NAME_LEN];

camera_lag=raydium_gui_read_name("menu","chkCameraLag",str);
camera_lag_speed=raydium_gui_read_name("menu","trkCameraLag",str);
raydium_gui_read_name("menu","edtPlayerName",raydium_network_name_local);
shadow_support=raydium_gui_read_name("menu","chkShadow",str);
windowed_mode=raydium_gui_read_name("menu","chkWindow",str);
raydium_gui_read_name("menu","cboWindow",windowed_res);
music_volume=raydium_gui_read_name("menu","trkMusicVol",str)/100.f;

str[1]=0;
str[0]=(camera_lag?'y':'n');
raydium_parser_db_set("ManiaDrive-CameraLagActive",str);

sprintf(str,"%f",camera_lag_speed);
raydium_parser_db_set("ManiaDrive-CameraSpeed",str);

raydium_parser_db_set("Generic-PlayerName",raydium_network_name_local);

sprintf(str,"%i",shadow_support);
raydium_parser_db_set("ManiaDrive-ShadowSupport",str);

sprintf(str,"%i",windowed_mode);
raydium_parser_db_set("ManiaDrive-Windowed",str);
raydium_parser_db_set("ManiaDrive-WindowedRes",windowed_res);

sprintf(str,"%f",music_volume);
raydium_parser_db_set("ManiaDrive-MusicVolume",str);
change_music_volume(music_volume);


raydium_gui_window_delete_name("menu");
build_gui_Main();
if(windowed_mode!=windowed_mode_save || strcmp(windowed_res,windowed_res_save))
    build_gui_NeedRestart();
}

void btnMusicVolTest(raydium_gui_Object *w)
{
float v;
char str[RAYDIUM_MAX_NAME_LEN];

v=raydium_gui_read_name("menu","trkMusicVol",str)/100.f;
change_music_volume(v);
}

void btnCredits(raydium_gui_Object *w)
{
btnStoryCompletedOk(NULL);
raydium_gui_window_delete_name("menu");
credits_start();
}

void btnSoloTracks(raydium_gui_Object *w)
{
raydium_gui_window_delete_name("menu");
build_gui_Solo();
}

void gui_menu_BestTime(raydium_gui_Object *w)
{
char track[RAYDIUM_MAX_NAME_LEN];
int handle;

float t;
char p[RAYDIUM_MAX_NAME_LEN];
char str[RAYDIUM_MAX_NAME_LEN];
int h,m,s,ms;

raydium_gui_read_name("menu","cboTrack",track);

strcat(track,".mni");
t=get_score(track,p);
hms(t,&h,&m,&s,&ms);

sprintf(str,"%s, with %i:%02i:%02i:%03i",p,h,m,s,ms);

handle=raydium_gui_window_create("besttime",25,45,50,15);
raydium_gui_widget_sizes(0,0,18);
raydium_gui_label_create("lblBestTime",handle,50,80,str,0,0,0);
raydium_gui_widget_sizes(15,5,18);
raydium_gui_button_create("btnBestTimeOkOk",handle,35,15,"OK",btnBestTimeOk);
}


void build_gui_ErrorDownload(void)
{
int handle;

handle=raydium_gui_window_create("error",25,45,60,15);
raydium_gui_widget_sizes(0,0,18);
raydium_gui_label_create("lblError",handle,50,80,"Can't download track from server !",0,0,0);
raydium_gui_widget_sizes(15,5,18);
raydium_gui_button_create("btnErrorOk",handle,35,15,"OK",btnErrorOkClick);
}

void build_gui_NeedRestart(void)
{
int handle;

handle=raydium_gui_window_create("restart",25,50,50,20);
raydium_gui_widget_sizes(0,0,18);
raydium_gui_label_create("lblRestart1",handle,50,80,"You need to restart the game",0,0,0);
raydium_gui_label_create("lblRestart2",handle,50,60,"to apply window settings",0,0,0);
raydium_gui_widget_sizes(15,5,18);
raydium_gui_button_create("btnRestartOk",handle,35,15,"OK",btnRestartClick);
}


void build_gui_StoryCompleted(void)
{
int handle;
char *str1= "  ^cCongratulations !";
char *str1s="  ^0Congratulations !";
char *str2="Story mode is completed !";

handle=raydium_gui_window_create("storycompleted",25,45,50,25);
raydium_gui_widget_sizes(0,0,28);
raydium_gui_label_create("lblStoryCompleted1s",handle,50,80,str1s,0,0,0);
raydium_gui_label_create("lblStoryCompleted1" ,handle,50.3,81,str1,0,0,0);
raydium_gui_widget_sizes(0,0,18);
raydium_gui_label_create("lblStoryCompleted2",handle,50,50,str2,0,0,0);

raydium_gui_widget_sizes(18,5,18);
raydium_gui_button_create("btnStoryCompletedOk",handle,15,15,"OK",btnStoryCompletedOk);
raydium_gui_button_create("btnCredits",handle,55,15,"Credits !",btnCredits);

raydium_gui_widget_focus_name("btnStoryCompletedOk","storycompleted");
}


void build_gui_InternetTracks(void)
{
int handle;
char l[RAYDIUM_GUI_DATASIZE];
char lasttrack[RAYDIUM_MAX_NAME_LEN];
int ilt;
signed char net;

raydium_parser_db_get("Generic-PlayerName",raydium_network_name_local,NULL);
raydium_parser_db_get("ManiaDrive-LastInternetTrack",lasttrack,"");

net=get_tracklist(l);
ilt=raydium_gui_list_id(lasttrack,l);
if(ilt<0) ilt=0;

handle=raydium_gui_window_create("menu",48,10,50,40);
raydium_gui_widget_sizes(0,0,18);
raydium_gui_label_create("lblPlayerName",handle,25,90,"Player Name :",0,0,0);
raydium_gui_widget_sizes(25,4,18);
raydium_gui_edit_create("edtPlayerName",handle,47,85,raydium_network_name_local);

raydium_gui_widget_sizes(0,0,18);
raydium_gui_label_create("lblTrack",handle,34,75,"Track :",0,0,0);
raydium_gui_widget_sizes(25,4,18);
raydium_gui_combo_create("cboTrack",handle,47,70,l,ilt);

raydium_gui_widget_sizes(25,4,18);
raydium_gui_button_create("btnBestTime",handle,47,55,"Get best time",gui_menu_BestTime);

raydium_gui_widget_sizes(15,5,18);
raydium_gui_button_create("btnDrive",handle,35,5,"Drive !",btnDriveNet);

raydium_gui_widget_sizes(6,3,14);
raydium_gui_button_create("btnBackToMain",handle,5,5,"<",btnBackToMainMenu);

raydium_gui_label_create("lblURL1",handle,50,40,"Live scores :",0,0,0);
raydium_gui_label_create("lblURL2",handle,50,30,"http://maniadrive.raydium.org/",0.7,0,0);

if(!net)
    {
    char *msg1="WARNING: Internet connection failed !";
    char *msg2="Now in offline mode. (set your proxy ?)";
    handle=raydium_gui_window_create("error",15,45,70,20);
    raydium_gui_widget_sizes(0,0,18);
    raydium_gui_label_create("lblError1",handle,50,80,msg1,0,0,0);
    raydium_gui_label_create("lblError2",handle,50,60,msg2,0,0,0);
    raydium_gui_widget_sizes(15,5,18);
    raydium_gui_button_create("btnErrorOk",handle,35,15,"OK",btnErrorOkClick);
    }

gui_start();
}

void build_gui_Solo(void)
{
int handle;
char l[RAYDIUM_GUI_DATASIZE];
char lasttrack[RAYDIUM_MAX_NAME_LEN];
int ilt;

handle=raydium_gui_window_create("menu",48,10,50,40);

raydium_parser_db_get("ManiaDrive-LastSoloTrack",lasttrack,"");

raydium_register_variable(l,RAYDIUM_REGISTER_STR,"list");
raydium_php_exec("mania_localtracks.php");
raydium_register_variable_unregister_last();

ilt=raydium_gui_list_id(lasttrack,l);
if(ilt<0) ilt=0;

raydium_gui_widget_sizes(0,0,18);
raydium_gui_label_create("lblMode",handle,50,90,"- Other Tracks -",0,0,0);

raydium_gui_widget_sizes(0,0,16);
raydium_gui_label_create("lbl1",handle,50,75,"Here, you can play offline with:",0.7,0,0);
raydium_gui_label_create("lbl2",handle,50,65,"- User tracks",0.7,0,0);
raydium_gui_label_create("lbl3",handle,50,55,"- Downloaded internet tracks",0.7,0,0);

raydium_gui_widget_sizes(0,0,18);
raydium_gui_label_create("lblTrack",handle,24,35,"Track :",0,0,0);
raydium_gui_widget_sizes(25,4,18);
raydium_gui_combo_create("cboTrack",handle,37,30,l,ilt);

raydium_gui_widget_sizes(15,5,18);
raydium_gui_button_create("btnDrive",handle,35,5,"Drive !",btnOtherTracksSolo);

raydium_gui_widget_sizes(6,3,14);
raydium_gui_button_create("btnBackToMain",handle,5,5,"<",btnBackToMainMenu);

gui_start();
}

// Return 1 if "filename" story is completed
signed char build_gui_Story_sub(int handle, char *filename, float x1, float x2, int id)
{
char ret[RAYDIUM_GUI_DATASIZE];
char file[RAYDIUM_MAX_NAME_LEN];
char part1[RAYDIUM_MAX_NAME_LEN];
char part2[RAYDIUM_MAX_NAME_LEN];
char part3[RAYDIUM_MAX_NAME_LEN];
signed char completed;
int i,last,len;
float pos;
FILE *fp;

strcpy(file,filename);

fp=raydium_file_fopen(file,"rt");
if(!fp) 
    {
    raydium_log("ERROR: cannot find story file ! abording ...");
    exit(1);
    }

while(fscanf(fp,"%s\n",ret)!=EOF )
    {
    FILE *fp2;
    fp2=raydium_file_fopen(ret,"rt");
    if(fp2) fclose(fp2);
    }
fclose(fp);

raydium_register_variable(file,RAYDIUM_REGISTER_STR,"story_file");
raydium_register_variable(ret,RAYDIUM_REGISTER_STR,"ret");
raydium_register_variable(&completed,RAYDIUM_REGISTER_SCHAR,"completed");
raydium_php_exec("mania_story.php");
raydium_register_variable_unregister_last();
raydium_register_variable_unregister_last();
raydium_register_variable_unregister_last();

pos=80;
last=0;
len=strlen(ret);
for(i=0;i<len;i++)
    if(ret[i]=='\n' || ret[i]==0)
    {
    signed char candrive;
    ret[i]=0;
    part1[0]=part2[0]=0;
    raydium_parser_cut(ret+last,part1,part2,';');
    raydium_gui_widget_sizes(10,4,12);
    candrive=(part2[1]!='4');
    raydium_gui_button_create(part1,handle,x1,pos,(candrive?"Drive":"-"),(candrive?btnDriveSolo:btnCantDrive));

    part1[0]='*'+id; // avoid name collision
    strcpy(part3,part2);
    part3[1]='0'; // change color
    raydium_gui_widget_sizes(0,0,12);
    raydium_gui_label_create(part1,handle,x2,pos+2.0,part3,0,0,0);
    part1[0]='#'+id; // avoid name collision
    raydium_gui_label_create(part1,handle,x2-0.2,pos+2.0+0.2,part2,0,0,0);

    last=i+1;
    pos-=6;
    }

return completed;
}

void build_gui_Story(void)
{
int handle;
signed char completed;

handle=raydium_gui_window_create("menu",5,5,90,90);

raydium_gui_widget_sizes(0,0,25);
raydium_gui_label_create("lblMode",handle,55,93,"Story Mode",0,0,0);

raydium_gui_widget_sizes(0,0,18);
raydium_gui_label_create("lblBeg",handle,30,88,"Beginners",0,0.3,0);
raydium_gui_label_create("lblPro",handle,80,88,"Pro",0.3,0,0);

completed=build_gui_Story_sub(handle,STORY_FILE_BEG,2, 33,0);

if(completed)
    {
    raydium_gui_widget_sizes(0,0,16);
    raydium_video_open("begend.jpgs","video_beg");
    raydium_video_loop_name("video_beg",0);
    raydium_gui_AfterGuiDrawCallback=after_gui;    
    raydium_gui_window_OnDelete_name("menu",video_beg_delete);
    raydium_gui_label_create("lblOkBeg",handle,27,45,"Beginners mode completed !",0,0,0);
    }

completed=build_gui_Story_sub(handle,STORY_FILE_PRO,52,83,1);

raydium_gui_widget_sizes(6,3,14);
raydium_gui_button_create("btnBackToMain",handle,5,3,"<",btnBackToMainMenu);
raydium_gui_widget_sizes(20,4,18);
raydium_gui_button_create("btnSolo",handle,40,3,"Other tracks",btnSoloTracks);

gui_start();

if(completed && !congrats)
    {
    congrats=1;
    raydium_gui_window_delete_name("menu");
    raydium_log("Completed !");
    build_gui_StoryCompleted();
    }
}


void build_gui_Lan(void)
{
int handle;
char server[RAYDIUM_MAX_NAME_LEN];

raydium_parser_db_get("Generic-PlayerName",raydium_network_name_local,NULL);
raydium_parser_db_get("ManiaDrive-Server",server,"192.168.0.1");

raydium_network_client_discover("ManiaDrive",10);

handle=raydium_gui_window_create("menu",48,10,50,40);

raydium_gui_widget_sizes(0,0,18);
raydium_gui_label_create("lblMode",handle,50,90,"- Network Game - ",0,0,0);

raydium_gui_label_create("lblPlayerName",handle,25,75,"Player Name :",0,0,0);
raydium_gui_widget_sizes(25,4,18);
raydium_gui_edit_create("edtPlayerName",handle,47,70,raydium_network_name_local);

raydium_gui_widget_sizes(0,0,18);
raydium_gui_label_create("lblServer",handle,32.5,60,"Server :",0,0,0);
raydium_gui_widget_sizes(25,4,18);
raydium_gui_edit_create("edtServer",handle,47,55,server);

raydium_gui_widget_sizes(18,5,18);
raydium_gui_button_create("btnMulti",handle,55,35,"Connect",btnNetworkConnect);
raydium_gui_widget_sizes(18,5,18);
raydium_gui_button_create("btnSearchLAN",handle,10,35,"Search LAN",btnSearchLAN);

//raydium_gui_widget_sizes(18,5,15);
//raydium_gui_button_create("btnCreateServ",handle,55,15,"Create server",btnCreateServer);

raydium_gui_widget_sizes(6,3,14);
raydium_gui_button_create("btnBackToMain",handle,5,5,"<",btnBackToMainMenu);
gui_start();
}

void build_gui_Options(void)
{
int handle;

raydium_parser_db_get("Generic-PlayerName",raydium_network_name_local,NULL);

handle=raydium_gui_window_create("menu",48,10,50,60);

raydium_gui_widget_sizes(0,0,18);
raydium_gui_label_create("lblPlayerName",handle,25,90,"Player Name :",0,0,0);
raydium_gui_widget_sizes(25,4,18);
raydium_gui_edit_create("edtPlayerName",handle,47,87,raydium_network_name_local);

raydium_gui_widget_sizes(4,4,18);
raydium_gui_check_create("chkCameraLag",handle,10,70," Camera lagging support",camera_lag);

raydium_gui_widget_sizes(0,0,18);
raydium_gui_label_create("lblCamLagFact",handle,30,64,"Camera's speed :",0,0,0);
raydium_gui_widget_sizes(20,2,0);
raydium_gui_track_create("trkCameraLag",handle,56,62, 2,10,camera_lag_speed);


raydium_gui_widget_sizes(0,0,18);
raydium_gui_label_create("lblMusicVol",handle,25,44,"Music volume :",0,0,0);
raydium_gui_widget_sizes(15,2,0);
raydium_gui_track_create("trkMusicVol",handle,48,42, 0,100,music_volume*100);
raydium_gui_widget_sizes(6,3,14);
raydium_gui_button_create("btnMusicVolTest",handle,82,41,"test",btnMusicVolTest);

raydium_gui_widget_sizes(4,4,18);
raydium_gui_check_create("chkShadow",handle,2,28," Shadow support (experimental)",shadow_support);
raydium_gui_check_create("chkWindow",handle,2,18," Window mode with",windowed_mode);
raydium_gui_widget_sizes(17,4,18);
raydium_gui_combo_create("cboWindow",handle,63,18,RESOLUTION_LIST,raydium_gui_list_id(windowed_res,RESOLUTION_LIST));
// save this settings
windowed_mode_save=windowed_mode;
strcpy(windowed_res_save,windowed_res);

raydium_gui_widget_sizes(6,3,14);
raydium_gui_button_create("btnBackToMain",handle,5,5,"<",btnBackToMainMenu);

raydium_gui_widget_sizes(15,5,18);
raydium_gui_button_create("btnApply",handle,30,5,"Apply",btnApplyOptions);


raydium_gui_widget_sizes(15,5,18);
raydium_gui_button_create("btnKeyHelp",handle,65,5,"Controls",btnKeyHelp);


gui_start();
}

void btnModeNetTracks(raydium_gui_Object *w)
{
raydium_gui_window_delete_name("menu");
build_gui_InternetTracks();
}

void btnModeStory(raydium_gui_Object *w)
{
raydium_gui_window_delete_name("menu");
build_gui_Story();
}

void btnModeLan(raydium_gui_Object *w)
{
raydium_gui_window_delete_name("menu");
build_gui_Lan();
}

void btnQuit(raydium_gui_Object *w)
{
exit(0);
}

void btnOptions(raydium_gui_Object *w)
{
raydium_gui_window_delete_name("menu");
build_gui_Options();
}

void btnMessageOk(raydium_gui_Object *w)
{
char part1[RAYDIUM_MAX_NAME_LEN];
char part2[RAYDIUM_MAX_NAME_LEN];
int id;

raydium_parser_cut(w->name,part1,part2,';');
id=atoi(part2);
showMessage(trackdata.message_file,id);
}

void showMessage(char *file, int id)
{
int handle;
FILE *fp;
int count;
signed char found=0;
int lines[32]; // start index of each line

int ret;
char var[RAYDIUM_MAX_NAME_LEN];
char val_s[RAYDIUM_GUI_DATASIZE];
float val_f[10];
int size;

raydium_gui_window_delete_name("menu");

fp=raydium_file_fopen(file,"rt");
if(fp)
{
 count=0;

 while( (ret=raydium_parser_read(var,val_s,val_f,&size,fp))!=RAYDIUM_PARSER_TYPE_EOF)
    {
    if(!strcasecmp(var,"en"))
	{
        if(ret!=RAYDIUM_PARSER_TYPE_RAWDATA)
	    {
	    raydium_log("message is wrong type, must be RAWDATA.");
	    continue;
	    }							
	if(count!=id)
	    {
	    count++;
	    continue;
	    }

	found=1;
	break;
	}
    }
 fclose(fp);
} // fp was ok


if(found)
    {
    int nlines=0;
    int i;
    int start=0;
    int len;
    float width;
    float heigth;
    int mx;
    
    len=strlen(val_s);
    
    mx=0;
    for(i=0;i<len;i++)
	if(val_s[i]=='\n')
	    {
	    val_s[i]=0;
	    lines[nlines]=start;
	    if(strlen(val_s+start)>mx)
		mx=strlen(val_s+start);
	    start=i+1;
	    nlines++;
	    }

    width=mx*1.6;
    heigth=(nlines+2)*5;
    
    if(width<20) width=40;
    
    handle=raydium_gui_window_create("menu",((float)100-width)/2,((float)100-heigth)/2,width,heigth);
    raydium_gui_widget_sizes(0,0,18);
    for(i=0;i<nlines;i++)
	{
	sprintf(var,"lbl%i",i);
	raydium_gui_label_create(var,handle,50,100-((float)100/(nlines+2))*(i+1),val_s+lines[i],0.2,0,0);
	}

    // create button here
    sprintf(var,"btnMessageOk;%i",id+1);
    raydium_gui_widget_sizes(15,5,18);
    id=raydium_gui_button_create(var,handle,40,100-((float)100/(nlines+2))*(i+1.5),"OK",btnMessageOk);
    raydium_gui_widget_focus_name(var,"menu");
    raydium_gui_show();
    }
else
    {
    trackdata.message_file[0]=0;
    raydium_osd_cursor_set(NULL,0,0); // hide mouse cursor
    raydium_gui_hide();
    }
}



void build_gui_Main(void)
{
int handle;

handle=raydium_gui_window_create("menu",48,10,50,40);

raydium_gui_widget_sizes(0,0,18);
raydium_gui_label_create("lblMode",handle,50,90,"- Select Game Mode - ",0,0,0);

raydium_gui_widget_sizes(25,4,18);
raydium_gui_button_create("btnModeStory",handle,25,70,"Story",btnModeStory);
raydium_gui_button_create("btnModeNetTracks",handle,25,55,"Internet Tracks",btnModeNetTracks);
raydium_gui_button_create("btnModeLAN",handle,25,40,"LAN Multiplayer",btnModeLan);
raydium_gui_button_create("btnOptions",handle,25,25,"Options",btnOptions);

raydium_gui_widget_sizes(6,3,14);
raydium_gui_button_create("btnBackToMain",handle,3,5,"Quit",btnQuit);
gui_start();
}


void network_ask_restart(void)
{
char buff[RAYDIUM_NETWORK_PACKET_SIZE];
buff[RAYDIUM_NETWORK_PACKET_OFFSET]=0;
raydium_network_write(NULL,raydium_network_uid,NET_RESTART,buff);
}

void load_boxes(char *filename)
{
FILE *fp;
int j,a;
char name[32];

fp=fopen(filename,"rt");
if(!fp)
    {
    raydium_log("cannot open '%s' for reading",filename);
    return;
    }

raydium_ode_object_delete_name("BOXES");
a=raydium_ode_object_create("BOXES");

for(j=0;j<MAX_ELEMS;j++)
    box[j].state=0;

j=0;

while(fscanf(fp,"%f %f %f %f %f %f %i\n",&box[j].x,
				       &box[j].y,
				       &box[j].z,
				       &box[j].tx,
				       &box[j].ty,
				       &box[j].tz,
				       &box[j].type)!=EOF)
{
    box[j].state=1;
    raydium_ode_name_auto("box",name);
    // MUST sets this object as "local only"
    raydium_ode_network_next_local_only=1;
    box[j].id=raydium_ode_object_box_add(name,a,1,box[j].tx,box[j].ty,box[j].tz,RAYDIUM_ODE_STATIC,box[j].type,"");
    raydium_ode_element_move_name_3f(name,box[j].x,box[j].y,box[j].z);        

    j++;
    if(j==MAX_ELEMS)
	{
	raydium_log("MAX_ELEMS reached, loading not completed");
	break;
	}
}

raydium_log("%s: %i box(es) loaded",filename,j);
fclose(fp);
}

// load associated track data
void dat_load(char *filename)
{
FILE *fp;
int ret;
char var[RAYDIUM_MAX_NAME_LEN];
char val_s[RAYDIUM_MAX_NAME_LEN];
float val_f[10];
int size;
int done;

fp=fopen(filename,"rt");

if(!fp)
    {
    raydium_log("error: cannot open '%s'",filename);
    return;
    }

// reset
trackdata.name[0]=0;
trackdata.author[0]=0;
trackdata.gold_time=0;
trackdata.author_time=0;
trackdata.message_file[0]=0;

while( (ret=raydium_parser_read(var,val_s,val_f,&size,fp))!=RAYDIUM_PARSER_TYPE_EOF)
    {
    done=0;

    if(!strcasecmp(var,"name"))
	{
	if(ret!=RAYDIUM_PARSER_TYPE_STRING)
	    {
	    raydium_log("'name' is wrong type");
	    continue;
	    }
	strcpy(trackdata.name,val_s);
	done=1;
	}

    if(!strcasecmp(var,"author"))
	{
	if(ret!=RAYDIUM_PARSER_TYPE_STRING)
	    {
	    raydium_log("'author' is wrong type");
	    continue;
	    }
	strcpy(trackdata.author,val_s);
	done=1;
	}

    if(!strcasecmp(var,"message"))
	{
	if(ret!=RAYDIUM_PARSER_TYPE_STRING)
	    {
	    raydium_log("'message' is wrong type");
	    continue;
	    }
	strcpy(trackdata.message_file,val_s);
	done=1;
	}

    if(!strcasecmp(var,"gold_time"))
	{
	if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=1)
	    {
	    raydium_log("'gold_time' is wrong type");
	    continue;
	    }
	trackdata.gold_time=val_f[0];
	done=1;
	}

    if(!strcasecmp(var,"author_time"))
	{
	if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=1)
	    {
	    raydium_log("'author_time' is wrong type");
	    continue;
	    }
	trackdata.author_time=val_f[0];
	done=1;
	}

    if(!done)
	raydium_log("invalid statement '%s'",var);

    }
fclose(fp);
}


void frame_step(GLfloat step)
{
float old;

old=partytime;

partytime-=step;
if(partytime<0)
    partytime=0;

if(old>0 && partytime==0) // end of this party
    change_game_state(GAME_SCORE);

if(game_state==GAME_GAME)
    timer+=step;
    
if(game_state==GAME_COUNTDOWN && strlen(trackdata.message_file)==0)
    {
    if(step<1) // wow .. this a a "track loading" lag
	countdown-=step;

    if(countdown<=0)
	change_game_state(GAME_GAME);
    }
}

void change_game_state(int type)
{
game_state=type;
countdown=0;

raydium_log("Changing game state to %i",type);

if(type==GAME_END)
    {
    int h,m,s,ms;
    char score[64];
    hms(timer,&h,&m,&s,&ms);
    sprintf(score,"%i:%02i:%02i:%03i",h,m,s,ms);
    

    if((timer<best_score[raydium_network_uid].time || 
        best_score[raydium_network_uid].time==-1) &&
	raydium_network_uid>=0)
	{
	best_score[raydium_network_uid].time=timer;
	strcpy(best_score[raydium_network_uid].player,raydium_network_name_local);
	raydium_network_propag_refresh(NET_SCORE_BASE+raydium_network_uid);
	}

    if(timer<track_score.time)
	{
	track_score.time=timer;
	strcpy(track_score.player,raydium_network_name_local);
	raydium_network_propag_refresh(NET_SCORE_TRACK);
	}

    if(timer<yourbest || yourbest==0)
	yourbest=timer;

    if(mode==MODE_SOLO)
	{
	if(timer<=trackdata.gold_time)
	    {
	    // win
	    if(timer<trackdata.author_time)
		strcpy(message,"Impressive ! Quicker than the author !");
	    else
		strcpy(message,"Right ! You got it.");
	    
	    // (add score to local database)
	    post_score_local(mni_current,timer);
	    }
	else
	    {
	    // try again
	    strcpy(message,"- Try again (press space key) -");
	    }
	}

    if(mode!=MODE_SOLO)
	{
	int p;
	p=post_score(mni_current,raydium_network_name_local,score);
	if(p)
	    sprintf(message,"Your place: %i",p);
	else
	    sprintf(message,"(unofficial track or connection failure)");
	
	}
    }

if(type==GAME_GAME)
    {
    raydium_ode_time_change(100);
    }

if(type==GAME_COUNTDOWN)
    {
    countdown=2;
    raydium_ode_time_change(0);
    }

if(type==GAME_SCORE)
    {
    raydium_ode_time_change(0);
    }
}


void hms(GLfloat t,int *h, int *m, int *s, int *ms)
{
*h=*m=0;
*ms=(t-(int)t)*1000;
*s=t;
while(*s>=60)
    {
    (*m)++;
    (*s)-=60;
    }

while(*m>=60)
    {
    (*h)++;
    (*m)-=60;
    }
}

void draw_timer(void)
{
int h,m,s,ms;
int h2,m2,s2,ms2;

hms(timer,&h,&m,&s,&ms);
raydium_osd_color_change(0.89,0.85,0.66);
raydium_osd_printf(55,4,12,0.5,"font-impact.tga","   lap time:");
raydium_osd_printf(68,4,12,0.7,"font-lcdmono.tga","%i:%02i:%02i:%03i",h,m,s,ms);


if(mode==MODE_MULTI)
    {
    hms(partytime,&h2,&m2,&s2,&ms2);
    raydium_osd_color_change(0.89,0.85,0.66);
    raydium_osd_printf(10,4,12,0.5,"font-impact.tga"," party time:");
    raydium_osd_printf(23,4,12,0.7,"font-lcdmono.tga","^f%i:%02i:%02i:%03i",h2,m2,s2,ms2);
    }
else
    {
    int off=0;
    
    if(mode==MODE_NET || simple_mni)
	off=10;
	
    hms(yourbest,&h2,&m2,&s2,&ms2);
    raydium_osd_color_change(0.89,0.85,0.66);
    raydium_osd_printf(20-off,4,12,0.5,"font-impact.tga","  your best:");
    raydium_osd_printf(33-off,4,12,0.7,"font-lcdmono.tga","^f%i:%02i:%02i:%03i",h2,m2,s2,ms2);
    }

if(mode==MODE_SOLO)
    {
    hms(trackdata.gold_time,&h2,&m2,&s2,&ms2);
    raydium_osd_color_change(0.89,0.85,0.66);
    raydium_osd_printf(20,7,12,0.5,"font-impact.tga","  gold time:");
    raydium_osd_printf(33,7,12,0.7,"font-lcdmono.tga","^e%i:%02i:%02i:%03i",h2,m2,s2,ms2);
    }    
}

void draw_best_score(void)
{
int h,m,s,ms;
char ok[RAYDIUM_NETWORK_MAX_CLIENTS];
float best;
float pos;
int i,j,bestid;
int cpt=1;


if(mode==MODE_SOLO)
    {
    return;
    }

if(track_score.time>=0)
    {
    hms(track_score.time,&h,&m,&s,&ms);
    raydium_osd_color_change(0.89,0.85,0.66);
    raydium_osd_printf(10,7,12,0.5,"font-impact.tga","     record:");
    raydium_osd_printf(23,7,12,0.7,"font-lcdmono.tga","^e%i:%02i:%02i:%03i",h,m,s,ms);
    raydium_osd_printf(39,7,12,0.5,"font-impact.tga","^f- %s",track_score.player);
    }

for(i=0;i<RAYDIUM_NETWORK_MAX_CLIENTS;i++)
    ok[i]=0;

pos=90;
for(i=0;i<RAYDIUM_NETWORK_MAX_CLIENTS;i++)
    {
    bestid=-1;
    for(j=0;j<RAYDIUM_NETWORK_MAX_CLIENTS;j++)
	if(!ok[j] && (best_score[j].time<best || bestid==-1))
	{
	best=best_score[j].time;
	bestid=j;
	}

    ok[bestid]=1;
    hms(best_score[bestid].time,&h,&m,&s,&ms);
    if(best_score[bestid].time>=0)
	{
	raydium_osd_printf(2,pos,10,0.5,"font-impact.tga","^c%i ^f-",cpt);
	raydium_osd_color_change(0.89,0.85,0.66);
	raydium_osd_printf(5,pos,10,0.5,"font-lcdmono.tga","%i:%02i:%02i:%03i",h,m,s,ms);
	raydium_osd_printf(15,pos-0.05,12,0.5,"font-impact.tga","^f%s",best_score[bestid].player);
	pos-=3;
	cpt++;
	}
    }

if(game_state==GAME_SCORE)
    {
    raydium_osd_printf(35+SHADOW_OFFSET,30-SHADOW_OFFSET,40,0.5,"font-impact.tga","^0Game over !");
    raydium_osd_printf(35,30,40,0.5,"font-impact.tga","^cGame over !");
    }

}


int music_playlist(char *newfile)
{
raydium_register_variable(newfile,RAYDIUM_REGISTER_STR,"raydium_ogg_file");
raydium_php_exec("mania_playlist.php");
raydium_register_variable_unregister_last();
change_music_volume(music_volume);
return strlen(newfile)!=0;
}


void music_random(void)
{
char var[RAYDIUM_MAX_NAME_LEN];
if(!music_volume)
    return;
music_playlist(var);
raydium_sound_load_music(var);
change_music_volume(music_volume);
}

void music_change(void)
{
music_popup_inc=1;
}

#define sizepp 50.f
#define maxpp	0.f
#define minpp  -sizepp
#define posypp 5.f
#define ratiopp 3.f
#define rollbackpp 200.f
#define speedpp	100.f
#define printoffsetpp 4.f
#define printcolpp "^0"
#define printcol2pp "^f"
#define printlenpp 24
#define shadowoffpp 0.2f
void trc(char *in, char *out, int len)
{
strcpy(out,in);
if(strlen(out)>len)
    {
    out[len-3]=0;
    strcat(out,"...");
    }
}

void draw_music_popup(void)
{
char str[RAYDIUM_MAX_NAME_LEN];
static GLfloat pos=0;
GLfloat real;

pos+=(raydium_frame_time*speedpp*music_popup_inc);

real=pos;

if(real>maxpp)
    real=maxpp;

if(real<minpp)
    {
    real=minpp;
    pos=minpp;
    }

if(pos>rollbackpp)
    {
    music_popup_inc=-1;
    }

raydium_osd_draw_name("BOXmania_music_popup.tga",real,posypp,sizepp+real,posypp+sizepp/ratiopp);
trc(raydium_sound_music_info.title,str,printlenpp);
raydium_osd_printf(real+2+shadowoffpp,posypp+sizepp/ratiopp-printoffsetpp-shadowoffpp,  20,0.4,"font2.tga","%s%s",printcol2pp,str);
raydium_osd_printf(real+2,posypp+sizepp/ratiopp-printoffsetpp,  20,0.4,"font2.tga","%s%s",printcolpp,str);
trc(raydium_sound_music_info.artist,str,printlenpp);
raydium_osd_printf(real+2+shadowoffpp,posypp+sizepp/ratiopp-printoffsetpp*2-shadowoffpp,  20,0.4,"font2.tga","%s%s",printcol2pp,str);
raydium_osd_printf(real+2,posypp+sizepp/ratiopp-printoffsetpp*2,20,0.4,"font2.tga","%s%s",printcolpp,str);
trc(raydium_sound_music_info.album,str,printlenpp);
raydium_osd_printf(real+2+shadowoffpp,posypp+sizepp/ratiopp-printoffsetpp*3-shadowoffpp,  20,0.4,"font2.tga","%s%s",printcol2pp,str);
raydium_osd_printf(real+2,posypp+sizepp/ratiopp-printoffsetpp*3,20,0.4,"font2.tga","%s%s",printcolpp,str);
}

void col_car_box(int car, int box_id)
{
int id;
int i;
int n,m;
dReal vect[3];

id=raydium_ode_element_tag_get(box_id);

switch(id)
    {
    case TYPE_CHECKPOINT:
	for(i=0;i<MAX_ELEMS;i++)
	    if(box[i].state && box[i].id==box_id)
		{
		if(box[i].state!=2)
		    {
		    raydium_sound_SetSourcePosCamera(sound_checkpoint);
		    raydium_sound_SourcePlay(sound_checkpoint);
		    box[i].state=2;
		    }
		break;
		}
	break;
    case TYPE_LOOP:
	break;
/*
    case TYPE_START_E:
	break;
    case TYPE_START_W:
	break;
    case TYPE_START_N:
	break;
    case TYPE_START_S:
	break;
*/
    case TYPE_END:
	if(game_state!=GAME_GAME)
	    return;
	    
	n=m=0;
	for(i=0;i<MAX_ELEMS;i++)
	    if(box[i].state && box[i].type==TYPE_CHECKPOINT)
		{
		n++;
		if(box[i].state==2)
		    m++;
		}
	if(n==m)
	    {
	    raydium_sound_SetSourcePosCamera(sound_owww);
	    raydium_sound_SourcePlay(sound_owww);
	    change_game_state(GAME_END);
	    }
	break;
    case TYPE_TURBO_E:
	vect[0]=TURBO_POWA;
	vect[1]=0;
	vect[2]=0;
	raydium_ode_object_addforce(raydium_ode_element_object_get(car),vect);
	raydium_sound_SetSourcePosCamera(sound_wizz);
	raydium_sound_SourcePlay(sound_wizz);
	break;
    case TYPE_TURBO_W:
	vect[0]=-TURBO_POWA;
	vect[1]=0;
	vect[2]=0;
	raydium_ode_object_addforce(raydium_ode_element_object_get(car),vect);
	raydium_sound_SetSourcePosCamera(sound_wizz);
	raydium_sound_SourcePlay(sound_wizz);
	break;
    case TYPE_TURBO_N:
	vect[0]=0;
	vect[1]=TURBO_POWA;
	vect[2]=0;
	raydium_ode_object_addforce(raydium_ode_element_object_get(car),vect);
	raydium_sound_SetSourcePosCamera(sound_wizz);
	raydium_sound_SourcePlay(sound_wizz);
	break;
    case TYPE_TURBO_S:
	vect[0]=0;
	vect[1]=-TURBO_POWA;
	vect[2]=0;
	raydium_sound_SetSourcePosCamera(sound_wizz);
	raydium_sound_SourcePlay(sound_wizz);
	raydium_ode_object_addforce(raydium_ode_element_object_get(car),vect);
	break;
    }
}

char is_box(int id)
{
switch(id)
    {
    case TYPE_CHECKPOINT:
    case TYPE_LOOP:
    case TYPE_START_E:
    case TYPE_START_W:
    case TYPE_START_N:
    case TYPE_START_S:
    case TYPE_END:
    case TYPE_TURBO_E:
    case TYPE_TURBO_W:
    case TYPE_TURBO_N:
    case TYPE_TURBO_S:
    return 1;    
    }
return 0;
}

char collide(int e1, int e2, dContact *c)
{
int i1,i2;
i1=raydium_ode_element_tag_get(e1);
i2=raydium_ode_element_tag_get(e2);

if(i1==TYPE_BALANCIER || i2==TYPE_BALANCIER)
    return 0;

#ifdef NO_NETWORK_COLLISIONS
if( (i1==TYPE_CAR || IS_CAR_BODY(i1)) && (i2==TYPE_CAR || IS_CAR_BODY(i2) ) )
    return 0;
#endif


if( (i1==TYPE_CAR || IS_CAR_BODY(i1)) && 
    is_box(i2) && !raydium_ode_network_element_isdistant(e1))
    {
    col_car_box(e1,e2);
    return 0;
    }

if( (i2==TYPE_CAR|| IS_CAR_BODY(i2)) &&
    is_box(i1) && !raydium_ode_network_element_isdistant(e2))
    {
    col_car_box(e2,e1);
    return 0;    
    }

if(is_box(i1)) return 0;
if(is_box(i2)) return 0;

return 1;
}

char nearcall(int obj1, int obj2)
{
if(obj1==object_ground && obj2==object_boxes) return 0;
if(obj2==object_ground && obj1==object_boxes) return 0;
return 1;
}

char draw_element_after(int elem)
{
int type;
dReal *pos;

type=raydium_ode_element_tag_get(elem);
if(IS_CAR_BODY(type) && raydium_ode_network_element_isdistant(elem))
    {
    int player;
    pos=raydium_ode_element_pos_get(elem);
    player=raydium_ode_network_element_distantowner(elem);
    raydium_osd_printf_3D(pos[0],pos[1],pos[2]+0.2,12,0.5,"font2.tga","^f%s",raydium_network_name[player]);
    raydium_ode_element[elem].mesh=raydium_object_find_load((type==TYPE_CAR_BODY)?"clio_wheels.tri":"clio_sp_wheels.tri");
    }
return 1;
}

int find_start(float *x, float *y, float *z, char *dir)
{
int i;

for(i=0;i<MAX_ELEMS;i++)
 if(box[i].state)
    {
    *x=box[i].x;
    *y=box[i].y;
    *z=box[i].z;
    
    if(box[i].type==TYPE_START_E)
	{
	*dir='e';
	return i;
	}

    if(box[i].type==TYPE_START_W)
	{
	*dir='w';
	return i;
	}

    if(box[i].type==TYPE_START_N)
	{
	*dir='n';
	return i;
	}

    if(box[i].type==TYPE_START_S)
	{
	*dir='s';
	return i;
	}
    }
return -1;
}


int find_car_meshes(char *body, char *wheel)
{
signed char completed=0;
char file[RAYDIUM_MAX_NAME_LEN];

strcpy(file,STORY_FILE_PRO);

raydium_register_variable(file,RAYDIUM_REGISTER_STR,"story_file");
raydium_register_variable(&completed,RAYDIUM_REGISTER_SCHAR,"completed");
raydium_php_exec("mania_story.php");
raydium_register_variable_unregister_last();
raydium_register_variable_unregister_last();

if(!completed)
    {
    strcpy(body, "clio.tri");
    strcpy(wheel,"roue5.tri");
    return TYPE_CAR_BODY;
    }
else
    {
    strcpy(body, "clio_sp.tri");
    strcpy(wheel,"roue6.tri");
    return TYPE_CAR_BODY_SP;
    }
}


void create_car(void)
{
#define BREAK_FORCE	0
#define ROTFRICTION	0.0005
#define ERP_CFM		0.3,0.8
int a,i;
float x,y,z;
char dir;
dReal pos[3];
dReal rot[3];
dReal partoffset1[]={-0.6,-0.134,-0.207};
char body[RAYDIUM_MAX_NAME_LEN];
char wheel[RAYDIUM_MAX_NAME_LEN];
int  body_id;

change_game_state(GAME_COUNTDOWN);

body_id=find_car_meshes(body,wheel);

timer=0;
for(i=0;i<MAX_ELEMS;i++)
    if(box[i].state==2)
	box[i].state=1;

    raydium_ode_object_delete_name("WATURE");

  a=raydium_ode_object_create("WATURE");
    raydium_ode_object_box_add("corps",a,0.2,1.2,0.6,0.4,RAYDIUM_ODE_STANDARD,body_id,body);
    raydium_ode_element_slip_name("corps",RAYDIUM_ODE_SLIP_ICE);

    raydium_ode_network_next_local_only=1;
    raydium_ode_object_box_add("balancier",a,0.8,0.1,0.1,0.1,RAYDIUM_ODE_STANDARD,TYPE_BALANCIER,""); // crate.tri
    raydium_ode_element_move_name_3f("balancier",0,0,-0.5);
    raydium_ode_joint_attach_fixed_name("balanfixed","corps","balancier");


    raydium_ode_network_next_local_only=1;
    raydium_ode_object_sphere_add("pneu_ag",a,0.5,RAYDIUM_ODE_AUTODETECT,RAYDIUM_ODE_STANDARD,TYPE_CAR,wheel);
    raydium_ode_element_rotfriction_name("pneu_ag",ROTFRICTION);
    raydium_ode_element_move_name_3f("pneu_ag",0.42,0.253,-0.180);
    raydium_ode_joint_attach_hinge2_name("suspet_ag","corps","pneu_ag",RAYDIUM_ODE_JOINT_SUSP_DEFAULT_AXES);
    raydium_ode_joint_break_force_name("suspet_ag",BREAK_FORCE);
    raydium_ode_joint_suspension_name("suspet_ag",ERP_CFM);

    raydium_ode_network_next_local_only=1;
    raydium_ode_object_sphere_add("pneu_ad",a,0.5,RAYDIUM_ODE_AUTODETECT,RAYDIUM_ODE_STANDARD,TYPE_CAR,wheel);
    raydium_ode_element_rotfriction_name("pneu_ad",ROTFRICTION);
    raydium_ode_element_move_name_3f("pneu_ad",0.42,-0.253,-0.180);
    raydium_ode_joint_attach_hinge2_name("suspet_ad","corps","pneu_ad",RAYDIUM_ODE_JOINT_SUSP_DEFAULT_AXES);
    raydium_ode_joint_break_force_name("suspet_ad",BREAK_FORCE);
    raydium_ode_joint_suspension_name("suspet_ad",ERP_CFM);

    raydium_ode_network_next_local_only=1;
    raydium_ode_object_sphere_add("pneu_rg",a,0.5,RAYDIUM_ODE_AUTODETECT,RAYDIUM_ODE_STANDARD,TYPE_CAR,wheel);
    raydium_ode_element_rotfriction_name("pneu_rg",ROTFRICTION);
    raydium_ode_element_move_name_3f("pneu_rg",-0.444,0.253,-0.180);
    raydium_ode_joint_attach_hinge2_name("suspet_rg","corps","pneu_rg",RAYDIUM_ODE_JOINT_SUSP_DEFAULT_AXES);
    raydium_ode_joint_hinge2_block_name("suspet_rg",1);
    raydium_ode_joint_break_force_name("suspet_rg",BREAK_FORCE);
    raydium_ode_joint_suspension_name("suspet_rg",ERP_CFM);

    raydium_ode_network_next_local_only=1;
    raydium_ode_object_sphere_add("pneu_rd",a,0.5,RAYDIUM_ODE_AUTODETECT,RAYDIUM_ODE_STANDARD,TYPE_CAR,wheel);
    raydium_ode_element_rotfriction_name("pneu_rd",ROTFRICTION);
    raydium_ode_element_move_name_3f("pneu_rd",-0.444,-0.253,-0.180);
    raydium_ode_joint_attach_hinge2_name("suspet_rd","corps","pneu_rd",RAYDIUM_ODE_JOINT_SUSP_DEFAULT_AXES);
    raydium_ode_joint_hinge2_block_name("suspet_rd",1);
    raydium_ode_joint_break_force_name("suspet_rd",BREAK_FORCE);
    raydium_ode_joint_suspension_name("suspet_rd",ERP_CFM);
    
    raydium_ode_motor_create("moteur",a,RAYDIUM_ODE_MOTOR_ENGINE);
    raydium_ode_motor_attach_name("moteur","suspet_ag",1);
    raydium_ode_motor_attach_name("moteur","suspet_ad",1);
//    raydium_ode_motor_power_max_name("moteur",0.1);
//    raydium_ode_motor_gears_set_name("moteur",gears,6);
//    raydium_ode_motor_gear_change_name("moteur",gear);
    
    raydium_ode_motor_create("direction",a,RAYDIUM_ODE_MOTOR_ANGULAR);
    raydium_ode_motor_attach_name("direction","suspet_ag",0);
    raydium_ode_motor_attach_name("direction","suspet_ad",0);
    raydium_ode_motor_power_max_name("direction",0.2);

raydium_sound_SourcePlay(sound_car);
raydium_ode_element_particle_offset_name("corps","smokeexhaustpipe.prt", partoffset1);


if(find_start(&x,&y,&z,&dir)<0) return;
pos[0]=x;
pos[1]=y;
pos[2]=z;
raydium_ode_object_move(a,pos);
switch(dir)
    {
    case 'e':
	rot[2]=0;
	break;
    case 'w':
	rot[2]=M_PI;
	break;
    case 'n':
	rot[2]=-M_PI/2;
	break;
    case 's':
        rot[2]=M_PI/2;
	break;
    }
rot[0]=rot[1]=0;
raydium_ode_object_rotate_name("WATURE",rot);
message[0]=0;
}

void leave(void)
{
int old;

old=mode;
mni_current[0]=0;
message[0]=0;
raydium_sound_SourceStop(sound_car);
raydium_network_client_disconnect();
raydium_ode_time_change(0);
mode=MODE_NONE;

raydium_clear_frame();
raydium_camera_look_at(0.1,0.1,0,0,1,0);
raydium_osd_draw_name("mania_logo2.tga",0,0,100,100);
raydium_rendering_finish();

if(raydium_gui_window_find("menu")>=0)
    {
    raydium_gui_window_delete_name("menu");
    raydium_gui_hide();
    }

if(old==MODE_SOLO)
    build_gui_Story();
else
if(old==MODE_NET)
    build_gui_InternetTracks();
else
if(old==MODE_MULTI)
    build_gui_Lan();
}



void display(void)
{
dReal speed,accel;
dReal direct;
dReal *tmp;
dReal *pos;

if(scroll>=0)
    {
    int i,cpt;
    float y,tmp;
    
    if(scroll!=99999)
	scroll+=raydium_frame_time*10; // must #define factor

    if(raydium_key_last==1027)
	credits_stop();
    
    raydium_clear_frame();
    raydium_camera_look_at(0.1,0.1,0,0,1,0);

    raydium_live_texture_mask_name("video2",1);

    // draw credits
    y=scroll;
    cpt=0;
    for(i=0;i<credits_nlines();i++)
	{
	if(y>-10 && y<110)
	    {
	    // centering ...
	    tmp=30/RAYDIUM_OSD_FONT_SIZE_FACTOR;
	    tmp=((strlen(credits_lines[i])-1)*tmp)/2;
	    tmp=50-tmp;
	    raydium_osd_printf(tmp+(SHADOW_OFFSET*3),y-(SHADOW_OFFSET*3),30,0.5,"font-impact.tga","^0%s",credits_lines[i]);
	    raydium_osd_printf(tmp,y,30,0.5,"font-impact.tga","^f%s",credits_lines[i]);
	    cpt++;
	    }
	y-=10; // per line step (must #define it)
	}

    draw_music_popup();

//    raydium_log("%i %f",cpt,y);

    if(cpt==0 && scroll!=99999)
	{
	GLfloat from[4]={0,0,0,0};
	GLfloat to[4]={0,0,0,1};	
	scroll=99999; // ugly :)
	raydium_osd_fade_from(from,to,1,credits_stop);
	}
    raydium_rendering_finish();

    return;
    }



if(strlen(mni_current)==0)
    {
    if(raydium_key_last==1027) exit(0);
    
    raydium_clear_frame();
    raydium_camera_look_at(0.1,0.1,0,0,1,0);

    if(!raydium_gui_isvisible())
	build_gui_Main(); // MODE_NONE

    raydium_live_texture_mask_name("video",1);
    {
    static float a=1.5;
    float b;
    a-=(raydium_frame_time/6);
    b=a;
    if(b<0.9) b=0.9;
    if(b>1) b=1;
    raydium_osd_mask_texture_name("mania_logo2.tga",b);
    }

    //raydium_osd_logo("logoc.tga");
    draw_music_popup();
    raydium_rendering_finish();

    if(raydium_gui_widget_find("lblLanServerSearch",raydium_gui_window_find("menu"))>=0)
	{
	static GLfloat timer;
	timer+=raydium_frame_time;

	if(timer>RAYDIUM_NETWORK_BEACON_DELAY)
	    {
	    timer=0;
	    btnSearchLAN(NULL);
	    }
	}

    
    return;
    }

if(raydium_key_last==1027)
    {
    if(simple_mni)
	exit(0);    
    leave();
    return;
    }


if(!joystick_enabled)
    {
    // all this should be Raydium's job, not our ...
    int sav;
    sav=raydium_joy;
    raydium_joy=0;
    raydium_joy_x=raydium_joy_y=0;
    raydium_joy_key_emul();
    raydium_joy=sav;
    }
else
    raydium_joy_key_emul();


if(raydium_key[GLUT_KEY_F1]) { raydium_projection_fov/=(1.04); raydium_window_view_update(); }
if(raydium_key[GLUT_KEY_F2]) { raydium_projection_fov*=(1.04); raydium_window_view_update(); }

// sets motor speed
if(vue==3 || vue==4 || vue==5 || vue==6)
{
if(game_state==GAME_END)
    raydium_joy_y=raydium_joy_x=0;
speed=0;
accel=0.12;

    if(raydium_joy_y>0.3)
	{
        speed=raydium_joy_y*55;
	raydium_ode_motor_power_max_name("moteur",accel);
	raydium_particle_generator_enable_name("corps",1);
	}
    else
    if(raydium_joy_y<0.3)
	{
        speed=raydium_joy_y*5;
	raydium_ode_motor_power_max_name("moteur",0.2 * -raydium_joy_y);
	raydium_particle_generator_enable_name("corps",0);
	}
    else
	{
	speed=0;
	raydium_ode_motor_power_max_name("moteur",0.2 * -raydium_joy_y); // 0.2 ? joy_y ?
	raydium_particle_generator_enable_name("corps",0);
	}

direct=raydium_joy_x*0.3;

raydium_ode_motor_speed_name("moteur",-speed);
raydium_ode_motor_angle_name("direction",direct);
}

if(raydium_key_last==1116) draw_debug*=-1;

if(raydium_key_last==1032)
    create_car();

if(raydium_key_last==1008)
    network_ask_restart();

if(raydium_key_last==1097) { raydium_ode_time_change(0); }
if(raydium_key_last==1122) { raydium_ode_time_change(10); }
if(raydium_key_last==1101) { raydium_ode_time_change(100); }

if(raydium_key_last==1000+'c')
    {
    if(vue==3)
	raydium_key_last=6;
    else
	raydium_key_last=3;
    }

if(raydium_key_last==5)
{
vue=5;
raydium_projection_near=10;
raydium_projection_fov=2;
raydium_window_view_update();
}
                                                                                 
if(raydium_key_last==6)
{
vue=6;
raydium_projection_near=0.05 ;
raydium_projection_fov=60;
raydium_window_view_update();                                                                 
}

if(raydium_key_last==3 && vue!=3)
{
vue=3;
raydium_projection_near=0.05 ;
raydium_projection_fov=70;
raydium_window_view_update();                                                                 
raydium_camera_path_reset();
}

if(raydium_key_last==4)
{
vue=4;
raydium_projection_near=0.05;
raydium_projection_fov=40;
raydium_window_view_update();                                                                                 
}

/*
if(raydium_key_last==7)
{
vue=7;
raydium_projection_near=0.05;
raydium_projection_fov=70;
raydium_window_view_update();
}
*/
//F9 to capture screen in jpg
if(raydium_key_last==9) raydium_capture_frame_jpeg_auto();
raydium_background_color_change(sun[0],sun[1],sun[2],sun[3]);

raydium_light_position[0][0]=50;
raydium_light_position[0][1]=150;
raydium_light_position[0][2]=200;
raydium_light_position[0][3]=1.0;

raydium_clear_frame();
glLoadIdentity();

pos=raydium_ode_element_pos_get_name("corps");
if(pos[2]<track_bottom)
    {
    create_car();
    return;
    }

// camera + drawing
tmp=raydium_ode_element_pos_get_name(cam);
if(vue==3) 
    {
    dReal mpos[3];
    dReal *vel;
    dReal cam[3];
    // get element position
    pos=raydium_ode_element_pos_get_name("corps");
    vel=raydium_ode_element_linearvelocity_get_name("corps");

    // get position of camera in world coords
    raydium_ode_element_RelPointPos_name("corps",-1,0,0,cam);

    // correct z pos (always at top of the car, for example)
    cam[2]=pos[2]+0.4;

    mpos[0]=pos[0]+vel[0];
    mpos[1]=pos[1]+vel[1];
    mpos[2]=pos[2]+vel[2];
    
    // standard smooth lookat camera
    if(camera_lag)
      raydium_camera_smooth(cam[0],cam[1],cam[2],mpos[1],-mpos[2],mpos[0],
			    70,0,raydium_frame_time*camera_lag_speed);
    else
      raydium_camera_look_at(cam[0],cam[1],cam[2],pos[1],-pos[2],pos[0]);

    }

if(vue==4) raydium_ode_element_camera_inboard_name("corps",0,0.35,-0.2,2,0,-0.2);
if(vue==5) raydium_camera_look_at(camx,camy,camz,tmp[1],-tmp[2],tmp[0]);
if(vue==6) raydium_ode_element_camera_inboard_name("corps",0.2,0,0.1,2,0,0);

raydium_ode_draw_all(0);
if(draw_debug==1) 
    {
    raydium_osd_network_stat_draw(5,30,20);
    raydium_ode_draw_all(1);
    }

raydium_ode_element_sound_update_name("corps",sound_car);

speed=raydium_ode_motor_speed_get_name("moteur",0);
raydium_joy_ff_autocenter(speed/2.f);

speed=raydium_ode_motor_speed_get_name("moteur",1);


///////////////////////////////////////////////// OSD
raydium_osd_color_ega('f');
raydium_osd_draw_name("BOXmaniadrive-bottom.tga",0,0,100,16);
raydium_osd_draw_name("BOXmaniadrive-bottom.tga",100,100,0,94);


raydium_osd_color_change(0.7,0.7,0.7);
raydium_osd_printf(85,5,26,0.6,"font-lcdmono.tga","%3i",(int)(speed*3));
raydium_osd_printf(93,4,12,0.6,"font-impact.tga","KMH");

raydium_osd_printf(88,98,16,0.6,"font-lcdmono.tga","%3i",raydium_render_fps);
raydium_osd_printf(93,97.5,12,0.6,"font-impact.tga","FPS");

// player's name
raydium_osd_printf(14,12,22,0.5,"font-marcelle.tga","^f%s",raydium_network_name_local);


speed*=0.1; // wheel radius
speed/=4;
speed+=0.5;
speed+=(raydium_random_neg_pos_1()/15);
raydium_sound_SetSourcePitch(sound_car,raydium_trigo_abs(speed));

change_music_volume(music_volume); // since eof callback will reset volume to 1

if( ((int)(timer/5.f)) % 2 )
    {
    raydium_osd_printf(2+SHADOW_OFFSET,98-SHADOW_OFFSET,16,0.5,"font-impact.tga","^0track %s by %s",mni_current,trackdata.author);
    raydium_osd_printf(2,98,16,0.5,"font-impact.tga","^ftrack ^c%s^f by ^c%s^f",mni_current,trackdata.author);
    }
else
    {
    raydium_osd_printf(2+SHADOW_OFFSET,98-SHADOW_OFFSET,16,0.5,"font-impact.tga","^0- %s -",trackdata.name);
    raydium_osd_printf(2,98,16,0.5,"font-impact.tga","^f- ^c%s^f -",trackdata.name);
    }


if(strlen(message))
    {
    raydium_osd_printf(10+SHADOW_OFFSET,30-SHADOW_OFFSET,24,0.5,"font2.tga","^0%s",message);
    raydium_osd_printf(10,30,24,0.5,"font2.tga","^f%s",message);
    }

/*{
GLfloat rx,ry,rz;
raydium_ode_element_rot_get_name("corps",&rx,&ry,&rz);
raydium_osd_printf(2,4,12,0.5,"font2.tga","^f%f %f %f",rx*180/PI,ry*180/PI,rz*180/PI);
}*/

    {
    int i,total,n;
    total=n=0;
    for(i=0;i<MAX_ELEMS;i++)
	if(box[i].state && box[i].type==TYPE_CHECKPOINT)
	{
	total++;
	if(box[i].state==2)
	    n++;
	}
	raydium_osd_color_change(0.89,0.85,0.66);
	raydium_osd_printf(55,7,12,0.5,"font-impact.tga","checkpoints:");
	raydium_osd_printf(68,7,12,0.7,"font-lcdmono.tga","%02i/%02i",n,total);
    }

draw_timer();
draw_best_score();

if(countdown>0)
    {
    raydium_osd_printf(10+SHADOW_OFFSET,50-SHADOW_OFFSET,40,0.5,"font-impact.tga","^0Ready ? Start in %i seconds",(int)countdown+1);
    raydium_osd_printf(10,50,40,0.5,"font-impact.tga","^cReady ? Start in ^f%i^c seconds",(int)countdown+1);
    }
raydium_osd_color_ega('f');
//raydium_osd_logo("logoc.tga");
draw_music_popup();

raydium_rendering_finish();

raydium_ode_network_element_send_iterative(RAYDIUM_ODE_NETWORK_OPTIMAL);
}

int main(int argc, char **argv)
{
//char server[RAYDIUM_MAX_NAME_LEN];
int i;
char str[RAYDIUM_MAX_NAME_LEN];
char lagActive[RAYDIUM_MAX_NAME_LEN];
char lagSpeed[RAYDIUM_MAX_NAME_LEN];

int full_sx_i,full_sy_i;
int mode;

raydium_init_args_name(argc,argv,"mania_drive");

raydium_parser_db_get("ManiaDrive-Windowed",str,"0");
windowed_mode=atoi(str);
raydium_parser_db_get("ManiaDrive-WindowedRes",windowed_res,"800x600");
mode=(windowed_mode?RAYDIUM_RENDERING_WINDOW:RAYDIUM_RENDERING_FULLSCREEN);
sscanf(windowed_res,"%ix%i",&full_sx_i,&full_sy_i);

raydium_window_create(full_sx_i,full_sy_i,mode,version);

raydium_parser_db_get("Generic-PlayerName",raydium_network_name_local,NULL);

raydium_texture_filter_change(RAYDIUM_TEXTURE_FILTER_TRILINEAR);
raydium_projection_near=0.01;
raydium_projection_far=1000;	
raydium_projection_fov=70;
//raydium_light_disable();
raydium_fog_disable();
raydium_light_on(0);
//raydium_light_on(1);
memcpy(raydium_light_color[0],sun,raydium_internal_size_vector_float_4);
//raydium_light_intensity[0]=400;
raydium_light_intensity[0]=1000000;
raydium_light_update_all(0);


memcpy(raydium_light_color[1],amb,raydium_internal_size_vector_float_4);
raydium_light_intensity[1]=10000;
raydium_light_update_all(1);
raydium_window_view_update();

/*
if(raydium_init_cli_option("server",server))
     if(!raydium_network_client_connect_to(server)) 
        exit(1);
*/

raydium_sound_DefaultReferenceDistance=4.f;
sound_car=raydium_sound_LoadWav("murcielago.wav");
raydium_sound_SetSourcePitch(sound_car,0);
raydium_sound_SetSourceGain(sound_car,0.1);  // Engine Gain

sound_checkpoint=raydium_sound_LoadWav("touched.wav");
raydium_sound_SetSourceLoop(sound_checkpoint,0);
                                                                                
sound_owww=raydium_sound_LoadWav("owww.wav");
raydium_sound_SetSourceLoop(sound_owww,0);

sound_wizz=raydium_sound_LoadWav("wizz.wav");
raydium_sound_SetSourceLoop(sound_wizz,0);

strcpy(raydium_console_config_texture,"logo_console.tga");
raydium_sky_box_cache();

strcpy(cam,"corps");
raydium_register_variable(&camx,RAYDIUM_REGISTER_FLOAT,"camx");
raydium_register_variable(&camy,RAYDIUM_REGISTER_FLOAT,"camy");
raydium_register_variable(&camz,RAYDIUM_REGISTER_FLOAT,"camz");
raydium_register_variable(&cam,RAYDIUM_REGISTER_STR,"cam");
raydium_register_variable(&vue,RAYDIUM_REGISTER_INT,"vue");

// read options
raydium_parser_db_get("ManiaDrive-CameraLagActive",lagActive,"y");
raydium_parser_db_get("ManiaDrive-CameraSpeed",lagSpeed,"5.0");
camera_lag=(lagActive[0]=='y'?1:0);
sscanf(lagSpeed,"%f",&camera_lag_speed);

raydium_parser_db_get("ManiaDrive-ShadowSupport",str,"1");
shadow_support=atoi(str);

raydium_parser_db_get("ManiaDrive-MusicVolume",str,"1.0");
sscanf(str,"%f",&music_volume);

raydium_parser_db_get("ManiaDrive-JoystickEnabled",str,"y");
joystick_enabled=(str[0]=='y'?1:0);

raydium_gui_theme_load("maniadrive.gui");

if(raydium_init_cli_option("mni",mni_current))
    {
    simple_mni=1;
    mni_load(mni_current);
    }

raydium_timecall_add(frame_step,-1);
change_game_state(GAME_COUNTDOWN);
message[0]=0;
mode=MODE_NONE;

raydium_sound_music_eof_callback=music_playlist;
raydium_sound_music_changed_callback=music_change;
raydium_ode_AfterElementDrawCallback=draw_element_after;
raydium_ode_CollideCallback=collide;
raydium_ode_ObjectNearCollide=nearcall;
//raydium_ode_StepCallback=ode_step;
object_ground=raydium_ode_object_find("GLOBAL");
object_boxes=raydium_ode_object_find("BOXES");

raydium_network_propag_add(NET_SCORE_TRACK,&track_score,sizeof(Score));
raydium_network_netcall_add(restart_asked_from_network,NET_RESTART,1);
raydium_network_netcall_add(netcall_mni_change,NET_CHANGE_MAP,1);
raydium_network_netcall_add(netcall_remaining_time,NET_REMAINING,1);
for(i=0;i<RAYDIUM_NETWORK_MAX_CLIENTS;i++)
    raydium_network_propag_add(NET_SCORE_BASE+i,&best_score[i],sizeof(Score));

raydium_callback(&display);
return 0;
}
