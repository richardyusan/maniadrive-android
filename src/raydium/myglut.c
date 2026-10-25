/*
    Raydium - CQFD Corp.
    http://raydium.org/
    License: GPL - GNU General Public License, see "gpl.txt" file.
*/

// avoid "real GLUT"
#ifndef GLUT_API_VERSION

#ifndef DONT_INCLUDE_HEADERS
#include "index.h"
#else
#include "headers/myglut.h"
#endif

#ifndef raydium_log
void raydium_log(char *format, ...);
#endif

int _glutWindowSize[2];
signed char _glutMouseVisible;

#ifdef WIN32
#include "myglut-win32.c"
#else
#include "myglut-x11.c"
#endif

//glutInitDisplayMode
//glutGameModeString
//glutEnterGameMode
//glutInitWindowSize
//glutCreateWindow
// are replaced by .. myglutCreateWindow
void myglutCreateWindow(GLuint tx, GLuint ty, signed char rendering, char *name)
{
switch(rendering)
    {
    case RAYDIUM_RENDERING_NONE:
	return;
    case RAYDIUM_RENDERING_WINDOW:
	pwInit(-1,-1,tx,ty,0,name,1,0);
	break;
    case RAYDIUM_RENDERING_FULLSCREEN:
	pwInit(0,0,-1,-1,0,name,0,0);
	break;    
    }
}

//glutMainLoop
void glutMainLoop(void)
{
//#ifdef WIN32
    // since windows is firing WM_SIZE too quickly ...
    if(glutReshapeFuncCB)
        glutReshapeFuncCB(_glutWindowSize[0],_glutWindowSize[1]);
//#endif
do{
    glutIdleFuncCB();
    myglutGetEvents();
  }while(1);
}

// glutWireSphere
void glutWireSphere(GLdouble radius, GLint slices, GLint stacks)
{
  static GLUquadricObj *quadObj=NULL;

  if(!quadObj)
    quadObj=gluNewQuadric();

  gluQuadricDrawStyle(quadObj, GLU_LINE);
  gluQuadricNormals(quadObj, GLU_SMOOTH);
  gluSphere(quadObj, radius, slices, stacks);
}

//glutGet
int glutGet(int enu)
{
switch(enu)
    {
    case GLUT_WINDOW_CURSOR:
	return _glutMouseVisible;
    case GLUT_WINDOW_WIDTH:
	return _glutWindowSize[0];
    case GLUT_WINDOW_HEIGHT:
	return _glutWindowSize[1];
    default:
	raydium_log("(my)glutGet: ERROR: unknown 'enu' %i",enu);
    }
return 0;
}


//glutIgnoreKeyRepeat (1 = yes)
void glutIgnoreKeyRepeat(int ignore)
{
//glutIgnoreKeyRepeatFlag=(ignore?1:0);
}

//glutReshapeFunc - void (GLUTCALLBACK *func)(int width, int height)
void glutReshapeFunc(void *func)
{
glutReshapeFuncCB=func;
}

//glutKeyboardFunc - void (GLUTCALLBACK *func)(unsigned char key, int x, int y)
void glutKeyboardFunc(void *func)
{
glutKeyboardFuncCB=func;
}

//glutSpecialUpFunc - void (GLUTCALLBACK *func)(int key, int x, int y)
void glutSpecialUpFunc(void *func)
{
glutSpecialUpFuncCB=func;
}

//glutSpecialFunc - void (GLUTCALLBACK *func)(int key, int x, int y)
void glutSpecialFunc(void *func)
{
glutSpecialFuncCB=func;
}

//glutMotionFunc - void (GLUTCALLBACK *func)(int x, int y)
void glutMotionFunc(void *func)
{
glutMotionFuncCB=func;
}

//glutPassiveMotionFunc - void (GLUTCALLBACK *func)(int x, int y)
void glutPassiveMotionFunc(void *func)
{
glutPassiveMotionFuncCB=func;
}

//glutMouseFunc - void (GLUTCALLBACK *func)(int button, int state, int x, int y)
void glutMouseFunc(void *func)
{
glutMouseFuncCB=func;
}

//glutDisplayFunc - void (GLUTCALLBACK *func)(void)
void glutDisplayFunc(void *func)
{
glutDisplayFuncCB=func;
}

//glutIdleFunc - void (GLUTCALLBACK *func)(void)
void glutIdleFunc(void *func)
{
glutIdleFuncCB=func;
}

//glutExtensionSupported - int (GLUTCALLBACK *func)(const char *name);
int glutExtensionSupported(const char *name)
{
char *ext;
int i;
int start=0;
char curr[128];

ext=(char *)glGetString(GL_EXTENSIONS);
if(!ext) return 0;

for(i=0;i<=strlen(ext);i++)
    {
    if(ext[i]==' ' || ext[i]==0)
	{
	strncpy(curr,ext+start,i-start);
	curr[i-start]=0;
	//printf("-%s-\n",curr);
	if(!strcasecmp(curr,name))
	    return 1;
	start=i+1;
	}
    }
return 0;
}

#endif
