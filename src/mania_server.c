// This is ManiaDrive mini server.

#define RAYDIUM_NETWORK_ONLY
#include "raydium/index.c"

#define PARTY_TIMEOUT   	(5*60) // default : 5
#define PARTY_SCORE_TIME	(10)
#define REMAINING_SEND		5


#define NET_SCORE_TRACK	(RAYDIUM_NETWORK_PACKET_BASE+1)
#define NET_RESTART	(RAYDIUM_NETWORK_PACKET_BASE+2)
#define NET_CHANGE_MAP	(RAYDIUM_NETWORK_PACKET_BASE+3)
#define NET_REMAINING	(RAYDIUM_NETWORK_PACKET_BASE+4)
#define NET_SCORE_BASE	(RAYDIUM_NETWORK_PACKET_BASE+10)


char str[RAYDIUM_NETWORK_PACKET_SIZE];
char track[RAYDIUM_NETWORK_PACKET_SIZE];
float steps=0;
//char str_send[RAYDIUM_NETWORK_PACKET_SIZE];
//int freq;

char *index_text="\
<br/><center>This is a <b>ManiaServer</b>. You can connect to this server using \
<a href=\"http://maniadrive.raydium.org/\">the ManiaDrive game</a>.</center><br/>\
Here, you can find :<br/>\
- <a href=\"data.dyn\">Informations about current party</a><br/>\
- <a href=\"console.dyn\">Server console</a><br/>\
<br>\
- <a href=\"http://maniadrive.raydium.org/index.php?scores=all\">Live scores on the web</a><br/>\
- <a href=\"http://maniadrive.raydium.org/\">Maniadrive website</a><br/>\
- <a href=\"http://raydium.org/\">Game Engine: Raydium website</a><br/>\
";



signed char switch_track(void)
{
static int last=-1;
FILE *fp;

raydium_register_variable(track,RAYDIUM_REGISTER_STR,"track");
raydium_register_variable(&last,RAYDIUM_REGISTER_INT,"last");

raydium_php_exec("mania_server_tracks.php");

raydium_register_variable_unregister_last();
raydium_register_variable_unregister_last();

if(! (fp=raydium_file_fopen(track,"rt")) )
    {
    raydium_log("ERROR ! cannot switch to track '%s' !",track);    
    return 0;
    }
fclose(fp);
raydium_log("Switching to track '%s'",track);
raydium_network_server_broadcast_info(track);
return 1;
}

void change_track(char *track)
{
char buff[RAYDIUM_NETWORK_PACKET_SIZE];

raydium_log("Broadcasting MNI track '%s'",track);
strcpy(buff+RAYDIUM_NETWORK_PACKET_OFFSET,track);
raydium_network_broadcast(NET_CHANGE_MAP,buff);
}


void send_time(GLfloat step)
{
char buff[RAYDIUM_NETWORK_PACKET_SIZE];
static float t=0;
float tt;

t+=step;
if(t>=REMAINING_SEND)
    {
    t=0;
    tt=PARTY_TIMEOUT-steps;
    //raydium_log("remaining time : broadcast (%.2f)",tt);
    memcpy(buff+RAYDIUM_NETWORK_PACKET_OFFSET,&tt,sizeof(float));
    strcpy(buff+RAYDIUM_NETWORK_PACKET_OFFSET+sizeof(float),track);
    raydium_network_broadcast(NET_REMAINING,buff);
    }
}


/*
void newClient(int client)
{
char buff[RAYDIUM_NETWORK_PACKET_SIZE];
float tt=0;
memcpy(buff+RAYDIUM_NETWORK_PACKET_OFFSET,&tt,sizeof(float));
strcpy(buff+RAYDIUM_NETWORK_PACKET_OFFSET+sizeof(float),track);
raydium_network_write(&raydium_network_client_addr[client],-1,NET_REMAINING,buff);
}
*/

void change_track_netcall(GLfloat step)
{
steps+=step;
//printf("%f\n",steps);
if(steps>=(PARTY_TIMEOUT+PARTY_SCORE_TIME))
    {
    steps=0;
    // change map
    if(switch_track())
	change_track(track);
    }
}


void broad(int type,char *buff)
{
    raydium_network_broadcast(type,buff);
//    raydium_log("broadcasting %i",type);
}


void loop(void)
{
signed char type;
int id;

if(raydium_network_read(&id, &type, str)==RAYDIUM_NETWORK_DATA_OK)
    {
    raydium_network_broadcast(type,str);
//    printf(".");
    }
raydium_timecall_callback();
raydium_web_callback();
}

signed char http_req(char *req, char *response, int max_size)
{
int i;

raydium_log("-%s-",req);
if(!strcmp("data.dyn",req))
    {
    sprintf(response,"Party informations\n\
    Running on track <b>%s</b> since <b>%.2f</b> seconds (<b>%.2f</b> left)\
    <br/><br/>",track,steps,PARTY_TIMEOUT-steps);

    sprintf(response+strlen(response),"<table class=\"tables\">\
    <tr><td><div class=\"greenfont\">Connected players (%i max)</div></td></tr>\
    ",RAYDIUM_NETWORK_MAX_CLIENTS);

    for(i=0;i<RAYDIUM_NETWORK_MAX_CLIENTS;i++)
	if(raydium_network_client[i])
	    sprintf(response+strlen(response),"<tr><td>%s</td></tr>",raydium_network_name[i]);
    
    sprintf(response+strlen(response),"</table><br/>");
    sprintf(response+strlen(response),"<center><a href=\"http://maniadrive.raydium.org/index.php?scores=track&track=%s\">\
    You can find bests online scores for '%s' track here !</a></center>\
    ",track,track);
    sprintf(response+strlen(response),"<a href=\"/\">Back</a>");
    
    return 1;
    }


if(!strcmp("console.dyn",req))
    {
    char *hist[RAYDIUM_CONSOLE_MAX_LINES];
    int i,n;
    
    n=raydium_console_history_read(hist);

    sprintf(response,"Server Console\n\
    <i>Raydium console:</i><pre class=\"border_one\">");

    for(i=0;i<n;i++)
	sprintf(response+strlen(response),"%s\n",hist[i]);

    sprintf(response+strlen(response),"</pre>");
    sprintf(response+strlen(response),"<br/><a href=\"/\">Back</a>");
    return 1;
    }

return 0;
}


int main(int argc, char **argv)
{
int i;
char title[RAYDIUM_MAX_NAME_LEN];

raydium_network_only_init(argc,argv);

raydium_web_init();
raydium_web_start("ManiaDrive Server");
raydium_web_extension_add("mni","raw/unknown",NULL);
raydium_web_extension_add("dyn",NULL,http_req); // use default "writer"
raydium_web_body_default=index_text;

raydium_init_cli_option_default("title",title,"ManiaDrive Server. Use --title to change :)");

raydium_network_server_broadcast(title,"ManiaDrive",10);

// Ugly hack, for now...
raydium_network_netcall_add(broad,RAYDIUM_NETWORK_PACKET_ODE_EXPLOSION,1);
raydium_network_netcall_add(broad,RAYDIUM_NETWORK_PACKET_ODE_NEWELEM,1);
raydium_network_netcall_add(broad,RAYDIUM_NETWORK_PACKET_ODE_REMELEM,1);
raydium_network_netcall_add(broad,RAYDIUM_NETWORK_PACKET_ODE_NIDWHO,1);
raydium_network_netcall_add(broad,NET_SCORE_TRACK,1);
raydium_network_netcall_add(broad,NET_RESTART,1);
for(i=0;i<RAYDIUM_NETWORK_MAX_CLIENTS;i++)
    raydium_network_netcall_add(broad,NET_SCORE_BASE+i,1);


raydium_timecall_add(change_track_netcall,-1);
raydium_timecall_add(send_time,-1);

switch_track();

do{
loop();
#ifndef WIN32
usleep(1); // MUST disable this with real servers
#else
sleep(0);
#endif
//printf(".\n");
}while(1);

}
