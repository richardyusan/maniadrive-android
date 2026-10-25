#ifndef _FILE_H
#define _FILE_H
/*=
Files (generic)
2100
**/

// Introduction
/**
File support is now splitted in two parts: generic functions and TRI format
specific functions. This chapter talks about generic part, where you'll find
some libc replacements and wrappers, and functions dealing with
"private directory" of the current user.
**/


__rayapi void raydium_file_dirname(char *dest,char *from);
/**
Reliable and portable version of libc's ##dirname## function.
This function extracts directory from ##from## filename, and writes it
to ##dest##.
No memory allocation will be done by the function.
**/

__rayapi void raydium_file_basename(char *dest,char *from);
/**
Another libc clone, for ##basename## function. Extracts file name from a
path into ##dest## string.
**/

__rayapi void raydium_file_ext(char *dest, char *from);
/**
Return the extension of ##from## filename (can be a complete path), without
the . (dot), or an empty string if extension is not found.
**/

__rayapi signed char raydium_file_directory_writable(char *path);
/**
Return **1** if ##path## directory is writable, **0** otherwise.
**/

__rayapi signed char raydium_file_readable(char *filename);
/**
Return **1** if ##filename## exists and is readable, **0** otherwise.
**/

__rayapi void raydium_file_log_fopen_display(void);
/**
Display (console) all filenames that were opened before the call.
##--files## command line option will call this function at the application's
exit, closed or not.
**/

__rayapi FILE *raydium_file_fopen(char *file, char *mode);
/**
Raydium wrapper to libc's ##fopen## function.
This function will:
- Update some stats
- Try to download the file from repositories if no local version is found, or
will try to update the file if asked (##--repository-refresh## or 
##repository-force##). See R3S on Raydium's Wiki.
- You can disable R3S client (for a "local only" file) adding a 'l' 
in ##mode## ("rl" or "rbl" for example).
- Use Raydium paths (see suitable chapter)
**/

#ifdef PHP_SUPPORT
__rayapi int raydium_rayphp_repository_file_get(char *file);
#else
#define raydium_php_repository_file_get fopen
#endif


__rayapi unsigned long raydium_file_sum_simple(char *filename);
/**
This function will generate a very simple checksum on ##filename##.
**/

unsigned long raydium_file_sum_simple_mode(char *filename,char *mode);
/**
Same as above, but you can pass a fopen ##mode## ("rt", or "rbl" for example).
See ##raydium_file_fopen()## for more informations about ##mode##.
**/

__rayapi char * raydium_file_home_path(char *file);
/**
This function will return an absolute file path for ##file## in the home
directory of the current user. 
Returned value is a pointer to static memory. Do not free this memory and use
it before any other call to this function, since it will be overwritten.
Example:
for ##test.cfg##, this function will return ##/home/me/.raydium/test.cfg##
See also ##raydium_init_args_name()## if you want to tune this result.
**/

__rayapi void raydium_file_home_path_cpy(char *file, char *dest);
/**
Same as above, but you must provide memory with ##dest##.
**/

__rayapi char *raydium_file_load(char *filename);
/**
This function loads ##filename## (as a binary file under win32, no matter 
under Linux) in a string, and returns its address. **You** must free this
memory when finished.
**/

#endif
