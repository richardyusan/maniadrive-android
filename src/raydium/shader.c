/*
    Raydium - CQFD Corp.
    http://raydium.org/
    License: GPL - GNU General Public License, see "gpl.txt" file.
*/

#ifndef DONT_INCLUDE_HEADERS
#include "index.h"
#else
#include "headers/shader.h"
#endif 

#include "shader.h"

char *raydium_file_load(char *filename);

void raydium_shader_init(void)
{
int i;

raydium_shader_support=
    (glutExtensionSupported("GL_ARB_shader_objects") &&
     glutExtensionSupported("GL_ARB_shading_language_100") );

for(i=0;i<RAYDIUM_MAX_SHADERS;i++)
    {
    raydium_shader_shaders[i].state=0;
    raydium_shader_shaders[i].id=i;
    }

if(raydium_shader_support)
    raydium_log("shaders: OK");
else
    raydium_log("shaders: FAILED (GLSL 1.0 not found)");
}


signed char raydium_shader_isvalid(int shader)
{
if(shader>=0 && shader<RAYDIUM_MAX_SHADERS && 
   raydium_shader_shaders[shader].state)
    return 1;

return 0;
}


int raydium_shader_find(char *name)
{
int i;

for(i=0;i<RAYDIUM_MAX_SHADERS;i++)
    if(raydium_shader_isvalid(i) && !strcmp(name,raydium_shader_shaders[i].name))
     return i;
return -1;
}


void raydium_shader_infolog(GLhandleARB shader)
{
char log[4096];
int len;

if(!raydium_shader_support)
    return;

glGetInfoLogARB(shader,4096,&len,log);
raydium_log("==== Shader Log ====");
raydium_log("%s",log);
raydium_log("====================");
}

int raydium_shader_load(char *name, char *file_vert, char *file_frag)
{
int i;
int ret;
char *str_vert;
char *str_frag;


if(!raydium_shader_support)
    return -1;

if(raydium_shader_find(name)>=0)
    {
    raydium_log("shader: Error: Cannot create shader \"%s\": name already exists",name);
    return -1;
    }

for(i=0;i<RAYDIUM_MAX_SHADERS;i++)
    if(!raydium_shader_shaders[i].state)
    {
    str_vert=raydium_file_load(file_vert);
    str_frag=raydium_file_load(file_frag);

    if(!str_vert)
	raydium_log("shader: unable to load vertex shader file \"%s\"",file_vert);
    if(!str_frag)
	{
	free(str_vert);
	raydium_log("shader: unable to load fragment shader file \"%s\"",file_frag);
	}
    
    if(!str_vert || !str_frag)
	{
	raydium_log("shader: ERROR: aborting \"%s\" creation",name);
	return -1;
	}


    raydium_shader_shaders[i].vert=glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
    raydium_shader_shaders[i].frag=glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

    glShaderSourceARB(raydium_shader_shaders[i].vert, 1, (const char **)&str_vert, NULL);
    glShaderSourceARB(raydium_shader_shaders[i].frag, 1, (const char **)&str_frag, NULL);

    glCompileShaderARB(raydium_shader_shaders[i].vert);
    glGetObjectParameterivARB(raydium_shader_shaders[i].vert,GL_OBJECT_COMPILE_STATUS_ARB,&ret);
    if(ret!=1)
	{
	raydium_log("shader: '%s': Compilation FAILED (vertex)",file_vert);
	printf("-%s-",str_vert);
	raydium_shader_infolog(raydium_shader_shaders[i].vert);
	glDeleteObjectARB(raydium_shader_shaders[i].vert);
	glDeleteObjectARB(raydium_shader_shaders[i].frag);
	free(str_vert);
	free(str_frag);
	return -1;
	}
    
    glCompileShaderARB(raydium_shader_shaders[i].frag);
    glGetObjectParameterivARB(raydium_shader_shaders[i].frag,GL_OBJECT_COMPILE_STATUS_ARB,&ret);
    if(ret!=1)
	{
	raydium_log("shader: '%s': Compilation FAILED (fragment)",file_frag);
	raydium_shader_infolog(raydium_shader_shaders[i].frag);
	glDeleteObjectARB(raydium_shader_shaders[i].vert);
	glDeleteObjectARB(raydium_shader_shaders[i].frag);
	free(str_vert);
	free(str_frag);
	return -1;
	}

    free(str_vert);
    free(str_frag);

    raydium_shader_shaders[i].prog=glCreateProgramObjectARB();
    glAttachObjectARB(raydium_shader_shaders[i].prog,raydium_shader_shaders[i].vert);
    glAttachObjectARB(raydium_shader_shaders[i].prog,raydium_shader_shaders[i].frag);

    glLinkProgramARB(raydium_shader_shaders[i].prog);
    glGetObjectParameterivARB(raydium_shader_shaders[i].prog,GL_OBJECT_LINK_STATUS_ARB,&ret);
    if(ret!=1)
	{
	raydium_log("shader: '%s': Linking FAILED",name);
	raydium_shader_infolog(raydium_shader_shaders[i].prog);
	glDeleteObjectARB(raydium_shader_shaders[i].vert);
	glDeleteObjectARB(raydium_shader_shaders[i].frag);
	glDeleteObjectARB(raydium_shader_shaders[i].prog);
	return -1;
	}


    raydium_shader_shaders[i].state=1;
    strcpy(raydium_shader_shaders[i].name,name);
    raydium_log("shader: shader %i (%s) loaded (%s,%s)",i,name,file_vert,file_frag);
    return i;
    }

raydium_log("shader: Error: No more slots ! aborting \"%s\" creation",name);
return -1;
}

int raydium_shader_variable(int shader, char *name)
{
int ret;

if(!raydium_shader_support)
    return -1;

if(!raydium_shader_isvalid(shader))
    {
    raydium_log("shader: cannot get variable: Invalid shader index or name");
    return -1;
    }

ret=glGetUniformLocationARB(raydium_shader_shaders[shader].prog,name);
if(ret<0)
    {
    raydium_log("shader: cannot get variable: Invalid variable name '%s'",name);
    return -1;
    }
return ret;
}

signed char raydium_shader_var_i(int var_id, int value)
{
if(!raydium_shader_support)
    return 0;

glUniform1iARB(var_id,value);
return 1;
}

signed char raydium_shader_var_i_name(char *shader, char *variable, int value)
{
signed char ret;
GLhandleARB curr;
int sid;

if(!raydium_shader_support)
    return 0;

curr=glGetHandleARB(GL_PROGRAM_OBJECT_ARB);
sid=raydium_shader_find(shader);
if(sid>=0) glUseProgramObjectARB(raydium_shader_shaders[sid].prog);
ret=raydium_shader_var_i(raydium_shader_variable(sid,variable),value);
glUseProgramObjectARB(curr);
return ret;
}

signed char raydium_shader_var_f(int var_id, float value)
{
if(!raydium_shader_support)
    return 0;

glUniform1fARB(var_id,value);
return 1;
}

signed char raydium_shader_var_f_name(char *shader, char *variable, float value)
{
signed char ret;
GLhandleARB curr;
int sid;

if(!raydium_shader_support)
    return 0;

curr=glGetHandleARB(GL_PROGRAM_OBJECT_ARB);
sid=raydium_shader_find(shader);
if(sid>=0) glUseProgramObjectARB(raydium_shader_shaders[sid].prog);
ret=raydium_shader_var_f(raydium_shader_variable(sid,variable),value);
glUseProgramObjectARB(curr);
return ret;
}

signed char raydium_shader_var_2f(int var_id, float value1, float value2)
{
if(!raydium_shader_support)
    return 0;

glUniform2fARB(var_id,value1,value2);
return 1;
}

signed char raydium_shader_var_2f_name(char *shader, char *variable, float value1, float value2)
{
signed char ret;
GLhandleARB curr;
int sid;

if(!raydium_shader_support)
    return 0;

curr=glGetHandleARB(GL_PROGRAM_OBJECT_ARB);
sid=raydium_shader_find(shader);
if(sid>=0) glUseProgramObjectARB(raydium_shader_shaders[sid].prog);
ret=raydium_shader_var_2f(raydium_shader_variable(sid,variable),value1,value2);
glUseProgramObjectARB(curr);
return ret;
}

signed char raydium_shader_var_3f(int var_id, float value1, float value2, float value3)
{
if(!raydium_shader_support)
    return 0;

glUniform3fARB(var_id,value1,value2,value3);
return 1;
}

signed char raydium_shader_var_3f_name(char *shader, char *variable, float value1, float value2, float value3)
{
signed char ret;
GLhandleARB curr;
int sid;

if(!raydium_shader_support)
    return 0;

curr=glGetHandleARB(GL_PROGRAM_OBJECT_ARB);
sid=raydium_shader_find(shader);
if(sid>=0) glUseProgramObjectARB(raydium_shader_shaders[sid].prog);
ret=raydium_shader_var_3f(raydium_shader_variable(sid,variable),value1,value2,value3);
glUseProgramObjectARB(curr);
return ret;
}

signed char raydium_shader_var_4f(int var_id, float value1, float value2, float value3, float value4)
{
if(!raydium_shader_support)
    return 0;

glUniform4fARB(var_id,value1,value2,value3,value4);
return 1;
}

signed char raydium_shader_var_4f_name(char *shader, char *variable, float value1, float value2, float value3, float value4)
{
signed char ret;
GLhandleARB curr;
int sid;

if(!raydium_shader_support)
    return 0;

curr=glGetHandleARB(GL_PROGRAM_OBJECT_ARB);
sid=raydium_shader_find(shader);
if(sid>=0) glUseProgramObjectARB(raydium_shader_shaders[sid].prog);
ret=raydium_shader_var_4f(raydium_shader_variable(sid,variable),value1,value2,value3,value4);
glUseProgramObjectARB(curr);
return ret;
}


// -1 = off
signed char raydium_shader_current(int shader)
{
if(!raydium_shader_support)
    return 0;

if(shader==-1)
    {
    glUseProgramObjectARB(0);
    return 1;
    }

if(!raydium_shader_isvalid(shader))
    {
    raydium_log("shader: cannot use shader: Invalid index or name");
    return 0;
    }

glUseProgramObjectARB(raydium_shader_shaders[shader].prog);
return 1;
}

signed char raydium_shader_current_name(char *shader)
{
return raydium_shader_current(raydium_shader_find(shader));
}

signed char raydium_shader_attach_texture(int shader, int texture)
{
if(shader!=-1 && !raydium_shader_isvalid(shader)) // -1 to disable shader
    {
    raydium_log("shader: cannot attach shader: Invalid shader");
    return 0;
    }
if(texture<=0 || texture>=raydium_texture_index)
    {
    raydium_log("shader: cannot attach shader: Invalid texture");
    return 0;
    }
raydium_texture_shader[texture]=shader;
return 1;
}

signed char raydium_shader_attach_texture_name(char *shader, char *texture)
{
return raydium_shader_attach_texture(raydium_shader_find(shader),
				     raydium_texture_find_by_name(texture));
}
