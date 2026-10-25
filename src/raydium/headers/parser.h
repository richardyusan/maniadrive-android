#ifndef _PARSER_H
#define _PARSER_H

/*=
Text file parsing
3700
**/

// Introduction
/**
Raydium provides a set of functions dedicated to text files parsing. These
files must follow a simple syntax:
%%(c)
// strings
variable_s="string value";

// float (or integer, i.e.)
variable_f=10.5;

// float array
variable_a={1,2,10.5,};

// raw data
variable_r=[
xxxxxxxx
#  oo  #
#      #
#  oo  #
xxxxxxxx
];
%%
Semi-colon are purely esthetic.
**/

__rayapi void raydium_parser_trim (char *org);
/**
Strip whitespace (or other characters) from the beginning and end of a string.
So far, ' ', '\n' and ';' are deleted.
**/

__rayapi signed char raydium_parser_isdata (char *str);
/**
Returns true (1) if ##str## contains data, false (0) otherwise (comments and
blank lines).
**/

__rayapi signed char raydium_parser_cut (char *str, char *part1, char *part2, char separator);
/**
This function will cut ##str## in two parts (##part1## and ##part2##) on
##separator##. No memory allocation will be done by this functions.
First occurence of ##separator## is used (left cut).
Return true (##i##+1) if ##str## was cut, where ##i## is the separator position.
**/

__rayapi void raydium_parser_replace (char *str, char what, char with);
/**
Will replace all occurence of ##what## with ##with##.
**/

__rayapi int raydium_parser_read (char *var, char *val_s, GLfloat *val_f, int *size, FILE *fp);
/**
Reads a new data line in ##fp##.
##var## will contain variable name. You'll find associated value in ##val_s##
if it's a string, or ##val_f## if it's a float (or a float array). In this last
case, ##size## will return the number of elements if the array.
%%(c)
FILE *fp;
int ret;
char var[RAYDIUM_MAX_NAME_LEN];
char val_s[RAYDIUM_MAX_NAME_LEN];
GLfloat val_f[MY_ARRAY_SIZE];
int size;

fp=raydium_file_fopen("foobar.txt","rt");

while( (ret=raydium_parser_read(var,val_s,val_f,&size,fp))!=RAYDIUM_PARSER_TYPE_EOF)
    {
    if(!strcasecmp(var,"foobar_variable"))
        {
        if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=2)
            {
	    raydium_log("error: foobar_variable is not float array");
            continue;
            }
        memcpy(...);
        }

    ...

    }
%%
**/

__rayapi signed char raydium_parser_db_get(char *key, char *value, char *def);
/**
This function will copy the value of ##key## from Raydium's database to
##value##. If ##key## is not found, ##def## is used as a default value.

If you do not want to use a default value, give ##NULL## to ##def##,
and the function will return 0 when ##key## was not found.

No memory allocation is done for you.
**/

__rayapi signed char raydium_parser_db_set(char *key, char *value);
/**
Sets ##key## in the Raydium's database to ##value##.
This function will return 0 if failed.
**/


#endif

