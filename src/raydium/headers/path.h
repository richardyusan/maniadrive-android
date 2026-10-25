#ifndef _PATH__H
#define _PATH__H
#include "../path.h"

/*=
File path
2110
**/

// Introduction
/**
No doc yet.
**/

__rayapi int raydium_path_find_free(void);
__rayapi signed char raydium_path_ext(char *dir, char *ext);
__rayapi signed char raydium_path_add(char *dir);
__rayapi signed char raydium_path_write(char *dir);
__rayapi signed char raydium_path_string_from(char *str);
__rayapi int raydium_path_string_to(char *out);
__rayapi void raydium_path_resolv(char *in, char *out, char mode);
__rayapi void raydium_path_dump(void);
__rayapi void raydium_path_reset(void);
__rayapi void raydium_path_init(void);

#endif
