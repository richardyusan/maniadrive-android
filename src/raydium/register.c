/*
    Raydium - CQFD Corp.
    http://raydium.org/
    License: GPL - GNU General Public License, see "gpl.txt" file.
*/

#ifndef DONT_INCLUDE_HEADERS
#include "index.h"
#else
#include "headers/register.h"
#endif 

// script registering: varibles and functions

int raydium_register_find_name(char *name)
{
int i;
if(!strlen(name)) return -1;
for(i=0;i<raydium_register_variable_index;i++)
    if(!strcmp(name,raydium_register_variable_name[i]))
        return i;
// not found:
return -1;
}

signed char raydium_register_name_isvalid(char *name)
{
int i;
for(i=0;i<strlen(name);i++)
    if(! ((name[i]>='a' && name[i]<='z') || 
          (name[i]>='A' && name[i]<='Z') ||
           name[i]=='_') )
           return 0; // this char is not valid
return 1;
}

// ex: int titi; ...(&titi,INT,"titi");
int raydium_register_variable(void *addr, int type, char *name)
{
int i;


if(raydium_register_variable_index==RAYDIUM_MAX_REG_VARIABLES)
    {
    raydium_log("register: ERROR: no more empty slots for variables",name);
    return -1;
    }

if( type!=RAYDIUM_REGISTER_INT &&
    type!=RAYDIUM_REGISTER_SCHAR &&
    type!=RAYDIUM_REGISTER_FLOAT &&
    type!=RAYDIUM_REGISTER_STR)
    {
    raydium_log("register: ERROR: use 'raydium_register_variable_const_*' for constants",name);
    return -1;
    }

if(!raydium_register_name_isvalid(name))
    {
    raydium_log("register: ERROR: \"%s\" is not a valid name",name);
    return -1;
    }
if(raydium_register_find_name(name)>=0)
    {
    raydium_log("register: variable: ERROR: \"%s\" already registered",name);
    return -1;
    }

i=raydium_register_variable_index++;
strcpy(raydium_register_variable_name[i],name);
raydium_register_variable_addr[i]=addr;
raydium_register_variable_type[i]=type;
return i;
}

int raydium_register_variable_const_i(int val, char *name)
{
int i;
int type=RAYDIUM_REGISTER_ICONST;

if(raydium_register_variable_index==RAYDIUM_MAX_REG_VARIABLES)
    {
    raydium_log("register: ERROR: no more empty slots for variables",name);
    return -1;
    }

if(!raydium_register_name_isvalid(name))
    {
    raydium_log("register: \"%s\" is not a valid name",name);
    return -1;
    }
if(raydium_register_find_name(name)>=0)
    {
    raydium_log("register: variable: ERROR: \"%s\" already registered",name);
    return -1;
    }

i=raydium_register_variable_index++;
strcpy(raydium_register_variable_name[i],name);
raydium_register_variable_addr[i]=malloc(sizeof(val));
(*(int *)raydium_register_variable_addr[i])=val;
raydium_register_variable_type[i]=type;
return i;
}

int raydium_register_variable_const_f(float val, char *name)
{
int i;
int type=RAYDIUM_REGISTER_FCONST;

if(raydium_register_variable_index==RAYDIUM_MAX_REG_VARIABLES)
    {
    raydium_log("register: ERROR: no more empty slots for variables",name);
    return -1;
    }

if(!raydium_register_name_isvalid(name))
    {
    raydium_log("register: \"%s\" is not a valid name",name);
    return -1;
    }
if(raydium_register_find_name(name)>=0)
    {
    raydium_log("register: variable: ERROR: \"%s\" already registered",name);
    return -1;
    }

i=raydium_register_variable_index++;
strcpy(raydium_register_variable_name[i],name);
raydium_register_variable_addr[i]=malloc(sizeof(val));
(*(float *)raydium_register_variable_addr[i])=val;
raydium_register_variable_type[i]=type;
return i;
}




void raydium_register_variable_unregister_last(void)
{
if(raydium_register_variable_index)
    raydium_register_variable_index--;
else
    raydium_log("register: cannot unregister last variable: stack empty");
}

int raydium_register_modifiy(char *var, char *args)
{
int i;
int *p_i;
float *f_i;
char *f_str;

raydium_log("WARNING: raydium_register_modifiy is deprecated !");

if(!raydium_register_name_isvalid(var))
    {
    raydium_log("register: modify: \"%s\" is not a valid name",var);
    return -1;
    }
i=raydium_register_find_name(var);
if(i<0)
    {
    raydium_log("register: modify: %s not found",var);
    return -1;
    }

if(raydium_register_variable_type[i]==RAYDIUM_REGISTER_INT)
    {
    p_i=raydium_register_variable_addr[i];
    (*p_i)=atoi(args);
    return i;
    }

if(raydium_register_variable_type[i]==RAYDIUM_REGISTER_SCHAR)
    {
    f_str=raydium_register_variable_addr[i];
    (*f_str)=atoi(args);
    return i;
    }

if(raydium_register_variable_type[i]==RAYDIUM_REGISTER_FLOAT)
    {
    float a;
    f_i=raydium_register_variable_addr[i];
    a=atof(args);
    (*f_i)=a;
    return i;
    }

if(raydium_register_variable_type[i]==RAYDIUM_REGISTER_STR)
    {
    f_str=raydium_register_variable_addr[i];
    strcpy(f_str,args);
    return i;
    }
raydium_log("register: modify: unknown type (%i) for \"%s\"",raydium_register_variable_type[i],var);
return -1;
}

void raydium_register_function(void *addr,char *name)
{
#ifdef PHP_SUPPORT
char *str;
str=malloc(strlen(name)+1);
if(raydium_register_function_index+2>=RAYDIUM_MAX_REG_FUNCTION)
    {
    raydium_log("register function: ERROR: no more room");
    return;
    }
strcpy(str,name);
raydium_register_function_list[raydium_register_function_index].fname=str;
raydium_register_function_list[raydium_register_function_index].handler=addr;
//raydium_register_function_list[raydium_register_function_index].func_arg_types=NULL;
raydium_register_function_index++;
raydium_register_function_list[raydium_register_function_index].fname=NULL;
raydium_register_function_list[raydium_register_function_index].handler=NULL;
//raydium_register_function_list[raydium_register_function_index].func_arg_types=NULL;
#else
raydium_log("register function: ERROR: need PHP support");
#endif
}


void raydium_register_dump(void)
{
#ifdef PHP_SUPPORT
int i;
char type[6][16]={"","int ","float ","char *","cont int ","const float "};
raydium_log("Registered data:");
raydium_log("----------------");

for(i=0;i<raydium_register_variable_index;i++)
    raydium_log("var: %s%s;",type[raydium_register_variable_type[i]],raydium_register_variable_name[i]);

for(i=0;i<raydium_register_function_index;i++)
    raydium_log("func: %s();",raydium_register_function_list[i].fname);
#endif
}
