//!NOBINDINGS
#ifndef _RAYPHP_H
#define _RAYPHP_H

/*=
RayPHP (internals)
3600
**/

// Introduction
/**
Raydium also use RayPHP (Raydium/PHP interface) for its own needs.
For PHP part of these functions, see "rayphp/" directory.
So far, RayPHP is dedicated to R3S (Raydium Server Side Scripts) access.
All this is mostly usefull for internal uses, since Raydium provides ##fopen##
wrappers, thru ##raydium_file_fopen##.

R3S is able to work with HTTP and FTP, and supports proxy using ##raydium.db##
configuration database. Example : %%Generic-Proxy;http://proxy:3128/%%
The trailing ##/## (slash) must be present.
**/

__rayapi void raydium_php_rayphp_path_change(char *path);
/**
By default, Raydium search for a "rayphp/" subdirectory in the application
directory. It's possible to change this using a --rayphp command line
switch, or using this function. You must call the function as soon as possible,
before any use of RayPHP.
**/

__rayapi int raydium_rayphp_repository_file_get (char *path);
/**
Will contact R3S servers for downloading ##path## file.
**/

__rayapi int raydium_rayphp_repository_file_put (char *path, int depends);
/**
Will contact R3S servers for uploading ##path## file. Set ##depends## to
true (1) if you also want to upload dependencies, false (0) otherwise.
**/

__rayapi int raydium_rayphp_repository_file_list(char *filter);
/**
Will contact R3S servers to get file list, using ##filter## (shell-like
syntax). Default ##filter## is ##*##.
**/

__rayapi signed char raydium_rayphp_http_test(void);
/**
Test if Internet connection is available using Raydium website.
(0 means 'not available', 1 means 'OK')
**/

__rayapi signed char raydium_rayphp_repository_defaults(char *def);
/**
Gives the default repositories for this applications

This function will create two files, ##repositories.list## and
##repositories.upload## in game user home directory, if these files
don't alreay exist, and will fill the files with ##def##.
This argument is an URL, or a list of URLs (use \n separator). See R3S doc.
**/

#endif
