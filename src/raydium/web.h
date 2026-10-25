/*
    Raydium - CQFD Corp.
    http://raydium.org/
    License: GPL - GNU General Public License, see "gpl.txt" file.
*/

#ifndef WEB_H
#define WEB_H

#define RAYDIUM_MAX_EXTENSIONS		32
#define RAYDIUM_WEB_BUFSIZE		8096
#define RAYDIUM_WEB_CLIENT_TEMP 	raydium_file_home_path("temp.delme.file")

#define RAYDIUM_WEB_ERROR 42
#define RAYDIUM_WEB_SORRY 43
#define RAYDIUM_WEB_LOG   44


typedef struct raydium_web_Extension
{
char ext[RAYDIUM_MAX_NAME_LEN];
char filetype[RAYDIUM_MAX_NAME_LEN];
void *handler;
} raydium_web_Extension;


__global raydium_web_Extension raydium_web_extensions[RAYDIUM_MAX_EXTENSIONS];
__global int raydium_web_extension_count;

__global int raydium_web_listenfd; 
__global struct sockaddr_in raydium_web_serv_addr;
__global signed char raydium_web_active;
__global char raydium_web_title[RAYDIUM_MAX_NAME_LEN];

__global char *raydium_web_header;
__global char *raydium_web_footer;
__global char *raydium_web_body_default;

#endif
