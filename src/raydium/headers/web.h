#ifndef _WEB_H
#define _WEB_H

#include "../web.h"

/*=
HTTP Web Tools
4200
**/

__rayapi void raydium_web_answer(char *message, int fd);
__rayapi void raydium_web_request(int fd);
__rayapi void raydium_web_start(char *title);
__rayapi void raydium_web_callback(void);
__rayapi void raydium_web_init(void);
__rayapi void raydium_web_extension_add(char *ext, char *mime, void *handler);
__rayapi signed char raydium_web_client_get(char *filename);


#endif
