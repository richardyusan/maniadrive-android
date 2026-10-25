/*
    Raydium - CQFD Corp.
    http://raydium.org/
    License: GPL - GNU General Public License, see "gpl.txt" file.
*/

#ifndef DONT_INCLUDE_HEADERS
#include "index.h"
#else
#include "headers/web.h"
#endif 

#include "web.h"


void raydium_web_answer(char *message, int fd)
{
char buffer[RAYDIUM_WEB_BUFSIZE*2]; // for header
char title[RAYDIUM_WEB_BUFSIZE];
char *body_start;

body_start=strchr(message,'\n');

// WARNING: do not change "Type: message" header offset !
// See raydium_web_client_get() otherwise.
sprintf(buffer,"HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nType: message\r\n\r\n");
send(fd,buffer,strlen(buffer),0);
buffer[0]=0;
sprintf(buffer+strlen(buffer),raydium_web_header,raydium_web_title);

if(!body_start)
    {
    sprintf(buffer+strlen(buffer),"%s",message);
    sprintf(buffer+strlen(buffer),raydium_web_footer,raydium_web_body_default);
    raydium_log("web: %s",message);
    }
else
    {
    strncpy(title,message,body_start-message);
    title[body_start-message]=0;
    sprintf(buffer+strlen(buffer),"%s",title);
    sprintf(buffer+strlen(buffer),raydium_web_footer,body_start+1);
    raydium_log("web: %s",title);
    }

send(fd,buffer,strlen(buffer),0);
}


/* this is a child web server process, so we can exit on errors */
void raydium_web_request(int fd)
{
	int j, file_fd, buflen, len;
	long i, ret;
	char * fstr;
	static char buffer[RAYDIUM_WEB_BUFSIZE+1]; /* static so zero filled */
	static char answer[RAYDIUM_WEB_BUFSIZE+1]; /* static so zero filled */
	signed char (*handler)(char *,char *, int);
    
    ret=recv(fd,buffer,RAYDIUM_WEB_BUFSIZE,0);
    
    if(ret == 0 || ret == -1)
	    {
	    /* read failure stop now */
        perror("read");
	    raydium_web_answer("error: Failed to read browser request",fd);
	    return;
	    }

	if(ret > 0 && ret < RAYDIUM_WEB_BUFSIZE)	/* return code is valid chars */
	    buffer[ret]=0;				/* terminate the buffer */
	else 
	    buffer[0]=0;

	for(i=0;i<ret;i++)	/* remove CR and LF characters */
	  if(buffer[i] == '\r' || buffer[i] == '\n')
	    buffer[i]='*';

	raydium_log("web: request from client ...");

	if( strncmp(buffer,"GET ",4) && strncmp(buffer,"get ",4) )
	    {
	    raydium_web_answer("error: Only simple GET operation supported",fd);
	    return;
	    }

	for(i=4;i<RAYDIUM_WEB_BUFSIZE;i++) 
	{ 
	/* null terminate after the second space to ignore extra stuff */
	if(buffer[i] == ' ')
	    {
	    /* string is "GET URL " +lots of other stuff */
	    buffer[i] = 0;
	    break;
	    }
	}

	for(j=0;j<i-1;j++) 	/* check for illegal parent directory use .. */
	  if(buffer[j] == '.' && buffer[j+1] == '.')
	    {
	    raydium_web_answer("error: Invalid path",fd);
	    return;
	    }

	if( !strncmp(&buffer[0],"GET /\0",6) || !strncmp(&buffer[0],"get /\0",6) ) /* convert no filename to index file */
	    {
	    char msg[RAYDIUM_MAX_NAME_LEN];
	    sprintf(msg,"Welcome to the embedded %s webserver.",raydium_web_title);
	    raydium_web_answer(msg,fd);
	    return;
	    }

	/* work out the file type and check we support it */
	buflen=strlen(buffer);
	fstr = (char *)0;
	handler=NULL;
	for(i=0;i<raydium_web_extension_count;i++) 
	    {
	    len = strlen(raydium_web_extensions[i].ext);
	    if( !strncmp(&buffer[buflen-len], raydium_web_extensions[i].ext, len))
		{
		fstr=raydium_web_extensions[i].filetype;
		handler=raydium_web_extensions[i].handler;
		break;
		}
	    }


	if(fstr == 0)
	    {
	    raydium_web_answer("error: Invalid target request",fd);
	    return;
	    }


	if(handler)
	    {
	    answer[0]=0;
	    if(!handler(&buffer[5],answer,RAYDIUM_WEB_BUFSIZE))
		{
		raydium_web_answer("error: Handler denied this request",fd);
		return;
		}
	    
	    // if there's no filetype, use web_answer
	    if(!strlen(fstr))
		raydium_web_answer(answer,fd);
	    // else let the user control the whole output
	    else
		{
		// WARNING: do not change "Type: message" header offset !
		// See raydium_web_client_get() otherwise.
		sprintf(buffer,"HTTP/1.0 200 OK\r\nContent-Type: %s\r\n\r\n",fstr);
		send(fd,buffer,strlen(buffer),0);
		send(fd,answer,strlen(answer),0);
		}
	    return;
	    }


// POSIX layer, hmmm ?
#ifdef WIN32
#define _RAYDIUM_FILE_MODE (O_RDONLY|O_BINARY)
#else
#define _RAYDIUM_FILE_MODE O_RDONLY
#endif

	if(( file_fd = open(&buffer[5],_RAYDIUM_FILE_MODE)) == -1) /* open the file for reading */
	    {
	    raydium_web_answer("error: Not found",fd);
	    return;
	    }

#undef _RAYDIUM_FILE_MODE

	raydium_log("web: ... sending '%s'",&buffer[5]);

	sprintf(buffer,"HTTP/1.0 200 OK\r\nContent-Type: %s\r\n\r\n", fstr);
	send(fd,buffer,strlen(buffer),0);

	/* send file in 8KB block - last block may be smaller */
	while (	(ret = read(file_fd, buffer, RAYDIUM_WEB_BUFSIZE)) > 0 )
	    {
    	    send(fd,buffer,ret,0);
	    }
}

void raydium_web_start(char *title)
{
char     opt[32];

if(raydium_web_active)
    {
    raydium_log("web: warning: server already started");
    return;
    }

raydium_log("web: starting Raydium HTTP server on port %i",RAYDIUM_NETWORK_PORT);
	
if((raydium_web_listenfd = socket(AF_INET, SOCK_STREAM,0)) <0)
    {
    raydium_log("web: error: socket failed");
    return;
    }

// avoiding bind's "Address already in use" error
setsockopt(raydium_web_listenfd, SOL_SOCKET, SO_REUSEADDR, opt, 32);

raydium_web_serv_addr.sin_family=AF_INET;
raydium_web_serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
raydium_web_serv_addr.sin_port=htons(RAYDIUM_NETWORK_PORT);

if(bind(raydium_web_listenfd, (struct sockaddr *)&raydium_web_serv_addr,sizeof(raydium_web_serv_addr)) <0)
    {
    raydium_log("web: error: bind failed");
    perror("bind");
    return;
    }

if(listen(raydium_web_listenfd,64) <0)
    {
    raydium_log("web: error: listen failed");
    return;
    }

strcpy(raydium_web_title,title);
raydium_web_active=1;
}



void raydium_web_callback(void)
{
static int socketfd;
static struct sockaddr_in cli_addr; /* static = initialised to zeros */
size_t length;

if(!raydium_web_active)
    return;

if(!raydium_network_socket_is_readable(raydium_web_listenfd))
    return;

length = sizeof(cli_addr);
if((socketfd = accept(raydium_web_listenfd, (struct sockaddr *)&cli_addr, (socklen_t *)&length)) < 0)
    return;


// /!\ FIXME ! must fork here. (see original nweb for details)
raydium_web_request(socketfd);
raydium_network_socket_close(socketfd);
}

#ifdef RAYDIUM_NETWORK_ONLY
void raydium_web_sigpipe_hack(int sig)
{
raydium_log("SIGPIPE (%i)",sig);
}
#endif

void raydium_web_init(void)
{
static char *header="\
<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\
<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\" lang=\"en\">\
<head>\
<title>Raydium 3D Game Engine</title>\
<style type=\"text/css\">\
<!--\
BODY {color: #424242; font-family: Verdana,Arial,Helvetica,sans-serif,monospace; margin: 0; padding: 0;  }\
A.blk {color: black;}\
A {color: #F19137;}\
A:HOVER {color: #227CBF;}\
.topbanner {background-color: #FFCC00;	border: 0; border-bottom: 1px dashed #5E5E5E; text-align: right;margin: 0; height: 15px; font-size: x-small; padding: 0;}\
.topbanner A {color: Black;}\
.topbanner A:HOVER {color: #F19137;	text-decoration: none;}\
.topbanner UL {list-style: none; border: 0; margin: 0;}\
.topbanner LI {display: inline; margin: 3px;}\
#contenu {margin: 0 10%% 0 170px; position: absolute; left:5px; top: 45px; width: 800px;}\
.publi_bloc { border-bottom: 2px dotted #FFCC00;  margin-bottom: 20px;}\
.publi_head {border-bottom: 1px dashed #A9A9A9;  border-left: 10px solid #FFCC00;}\
.publi_head h2 { margin: 0px;  padding-left: 10px;}\
.publi_head h2 a { color: #424242;  text-decoration: none;  }\
.publi_head h2 a:hover { color: #727272;  text-decoration: none;  }\
.publi_info {text-align: right; color: #A9A9A9;}\
.publi_info a {color: #A9A9A9;  text-decoration: none;}\
.publi_info a:hover {color: #696969;  text-decoration: none;}\
.publi_corps{ padding: 10px;}\
\
IMG {border: 1px solid; margin: 5px;}\
.tables {background: #f3f3f3;border-collapse:collapse;margin-left: auto; margin-right: auto;}\
.tables TD {border-style: solid; border-color:black; border-width:1px; text-align: center; padding-left: 5px; padding-right: 5px;}\
.redfont { color: #dd0000;}\
.greenfont { color: #00dd00;}\
.noborder { border : 0;}\
.border_one { border: 1px dashed #ACACAC; background-color: #EEEEEE; }\
-->\
</style>\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-15\"></head><body>\
<div class=\"topbanner\"><ul><li>Raydium embedded HTTP webserver - based on IBM's nweb - CQFD Corp.</li></ul></div>\
<div id=\"contenu\">\
<div class=\"contenu\"><div class=\"publi_bloc\"><div class=\"publi_head\"><strong>%s</strong> &gt; message<h2>\
";

static char *body="\
This server is used to be an entry point to application data. Only simple requests (GET) are supported yet, \
with a limited set of file types and directories. Right now, this server is able to send static and dynamic data, \
and dynamic scripted page support is to come, using Raydium's PHP parser.\
<br/><br/>\
This server is a modified version of IBM's nweb server, from Nigel Griffiths (nag@uk.ibm.com).\
<br/><br/>\
For more information, see Raydium website <a href=\"http://raydium.org/\">http://raydium.org/</a><br/>\
See also <i>raydium/web.h</i>,  <i>raydium/web.c</i> and <i>raydium/headers/web.h</i> files from the source directory.\
";

static char *footer="\
</h2></div><div class=\"publi_corps\">\
%s\
</div></body></html>\
";

raydium_web_header=header;
raydium_web_body_default=body;
raydium_web_footer=footer;

#ifdef RAYDIUM_NETWORK_ONLY
// ... and ...
#ifndef WIN32
// since we will not install our default signal handlers ...
signal(SIGPIPE,raydium_web_sigpipe_hack);
#endif
#endif
memset(&raydium_web_serv_addr,0,sizeof(struct sockaddr_in));
raydium_web_active=0;
raydium_web_extension_count=0;
strcpy(raydium_web_title,"Default");
raydium_log("webserver: OK");
}

void raydium_web_extension_add(char *ext, char *mime, void *handler)
{
if(raydium_web_extension_count==RAYDIUM_MAX_EXTENSIONS)
    {
    raydium_log("web: extension: error: no more free slot (%i max)",RAYDIUM_MAX_EXTENSIONS);
    return;
    }

strcpy(raydium_web_extensions[raydium_web_extension_count].ext,ext);
if(mime)
    strcpy(raydium_web_extensions[raydium_web_extension_count].filetype,mime);
else
    raydium_web_extensions[raydium_web_extension_count].filetype[0]=0;

raydium_web_extensions[raydium_web_extension_count].handler=handler;

raydium_web_extension_count++;
}

signed char raydium_web_client_get(char *filename)
{
int i,sockfd;
char buffer[RAYDIUM_WEB_BUFSIZE];
char *data;
char req[RAYDIUM_MAX_NAME_LEN];
char complete[RAYDIUM_MAX_NAME_LEN];
struct sockaddr_in serv_addr;
struct hostent *hst;
int chunk;
FILE *fp;

if(raydium_network_mode!=RAYDIUM_NETWORK_MODE_CLIENT)
    {
    raydium_log("web client: cannot get file: not connected to a server");
    return 0;
    }

if((sockfd = socket(AF_INET, SOCK_STREAM,0)) <0)
    {
    raydium_log("web: client: socket failure");
    return 0;
    }

hst=gethostbyname(raydium_network_connected_server);
memcpy((char*)(&(serv_addr.sin_addr.s_addr)), hst->h_addr, hst->h_length);
serv_addr.sin_family = AF_INET;
serv_addr.sin_port = htons(RAYDIUM_NETWORK_PORT);

if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) <0)
    {
    raydium_log("web: client: can't connect to server (%s)",raydium_network_connected_server);
    return 0;
    }

sprintf(req,"GET /%s \r\n",filename);			
send(sockfd,req,strlen(req),0);

chunk=0;
while( (i=recv(sockfd,buffer,RAYDIUM_WEB_BUFSIZE,0)) > 0)
    {
    data=buffer;
    if(chunk==0)
	{
	int x;
	//check: "HTTP/1.x 200 OK"
	if(buffer[9]!='2' || buffer[10]!='0' || buffer[11]!='0')
	    {
	    buffer[12]=0;
	    raydium_log("web: client: error: server said %s",buffer);
	    raydium_network_socket_close(sockfd);
	    return 0;
	    }

	// is this real data or an simple message from Raydium webserver ?
	// See "Type" header here :
	// "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nType: message\r\n\r\n"
	strncpy(req,buffer,60);
	req[55]=0;
	if(!strcmp(req+42,"Type: message"))
	    {
	    raydium_log("web: client: error: no data, this is a server message (not found ?)");
	    raydium_network_socket_close(sockfd);
	    return 0;
	    }

	// ok, now search for \r\n\r\n	
	for(x=12;x<i;x++)
	    if(buffer[x-3]=='\r' && 
	       buffer[x-2]=='\n' && 
	       buffer[x-1]=='\r' && 
	       buffer[x-0]=='\n')
	        break;
	
	if(x==i)
	    {
	    raydium_log("web: client: error: cannot found header end");
	    raydium_network_socket_close(sockfd);
	    return 0;
	    }
	// found, adjust offset 1 byte after
	x++;	
	data+=x;
	i-=x;
	
	fp=fopen(RAYDIUM_WEB_CLIENT_TEMP,"wb");
	if(!fp)
	    {
	    raydium_log("web: client: error: cannot create temporary file");
	    raydium_network_socket_close(sockfd);
	    return 0;
	    }	
	}

    fwrite(data,i,1,fp);
    chunk++;
    }
			
fclose(fp);
raydium_network_socket_close(sockfd);

raydium_path_resolv(filename,complete,'w');

// compare files and rename if not the same
if(raydium_file_sum_simple_mode(complete,"rbl")!=
   raydium_file_sum_simple_mode(RAYDIUM_WEB_CLIENT_TEMP,"rbl"))
    {    
    unlink(complete);
    if(rename(RAYDIUM_WEB_CLIENT_TEMP,complete)==-1)
	{
	raydium_log("web: client: cannot rename downloaded file !");
	perror("rename");
	return 0;
	}
    raydium_log("web: client: file '%s': download ok",filename);
    }
else
    raydium_log("web: client: local file '%s' is the same, canceled",filename);

return 1;
}
