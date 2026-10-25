/*
    Raydium - CQFD Corp.
    http://raydium.org/
    License: GPL - GNU General Public License, see "gpl.txt" file.
*/

#ifndef PATH_H
#define PATH_H

#define RAYDIUM_MAX_PATHS	32

#define RAYDIUM_PATH_MODE_READ	1
#define RAYDIUM_PATH_MODE_WRITE	2 // not used yet

typedef struct raydium_path_Path
{
signed char state;
char path[RAYDIUM_MAX_DIR_LEN];
char ext[RAYDIUM_MAX_NAME_LEN];
int  priority; // not used yet, see path deletion
signed char mode;
} raydium_path_Path;

raydium_path_Path raydium_path_paths[RAYDIUM_MAX_PATHS];
char raydium_path_write_current[RAYDIUM_MAX_DIR_LEN];

#endif
