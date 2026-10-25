/*
    Raydium - CQFD Corp.
    http://raydium.org/
    License: GPL - GNU General Public License, see "gpl.txt" file.
*/

// avoid "real GLUT"
#ifndef GLUT_API_VERSION
#ifndef MYGLUT
#define MYGLUT

#include <GL/gl.h>
#include <GL/glu.h>

#define GLUT_LEFT_BUTTON		0
#define GLUT_MIDDLE_BUTTON		1
#define GLUT_RIGHT_BUTTON		2

#define GLUT_DOWN			0
#define GLUT_UP				1

#define GLUT_KEY_F1			1
#define GLUT_KEY_F2			2
#define GLUT_KEY_F3			3
#define GLUT_KEY_F4			4
#define GLUT_KEY_F5			5
#define GLUT_KEY_F6			6
#define GLUT_KEY_F7			7
#define GLUT_KEY_F8			8
#define GLUT_KEY_F9			9
#define GLUT_KEY_F10			10
#define GLUT_KEY_F11			11
#define GLUT_KEY_F12			12
#define GLUT_KEY_LEFT			100
#define GLUT_KEY_UP			101
#define GLUT_KEY_RIGHT			102
#define GLUT_KEY_DOWN			103
#define GLUT_KEY_PAGE_UP		104
#define GLUT_KEY_PAGE_DOWN		105
#define GLUT_KEY_HOME			106
#define GLUT_KEY_END			107
#define GLUT_KEY_INSERT			108

#define GLUT_WINDOW_WIDTH		102
#define GLUT_WINDOW_HEIGHT		103
#define GLUT_WINDOW_DEPTH_SIZE		106
#define GLUT_WINDOW_CURSOR		122

#define GLUT_CURSOR_LEFT_ARROW		1
#define GLUT_CURSOR_NONE		101

#define GLUT_RGB			0
#define GLUT_DOUBLE			2
#define GLUT_DEPTH			16

#define GLUT_GAME_MODE_POSSIBLE         1

// ------------------- variables

// callbacks:
void (*glutReshapeFuncCB)(int width, int height);
void (*glutKeyboardFuncCB)(unsigned char key, int x, int y);
void (*glutSpecialUpFuncCB)(int key, int x, int y);
void (*glutSpecialFuncCB)(int key, int x, int y);
void (*glutMotionFuncCB)(int x, int y);
void (*glutPassiveMotionFuncCB)(int x, int y);
void (*glutMouseFuncCB)(int button, int state, int x, int y);
void (*glutDisplayFuncCB)(void);
void (*glutIdleFuncCB)(void);

// protos
__rayapi void glutInit(int *argc, char **argv);
__rayapi int  glutGet(int enu);
__rayapi void glutSetCursor(int cursor);
__rayapi void glutWarpPointer(int x, int y);
__rayapi void glutSwapBuffers(void);
__rayapi void glutIgnoreKeyRepeat(int ignore);
__rayapi void glutReshapeFunc(void *func);
__rayapi void glutKeyboardFunc(void *func);
__rayapi void glutSpecialUpFunc(void *func);
__rayapi void glutSpecialFunc(void *func);
__rayapi void glutMotionFunc(void *func);
__rayapi void glutPassiveMotionFunc(void *func);
__rayapi void glutMouseFunc(void *func);
__rayapi void glutDisplayFunc(void *func);
__rayapi void glutIdleFunc(void *func);
__rayapi int  glutExtensionSupported(const char *name);
__rayapi void glutMainLoop(void);
__rayapi void glutWireSphere(GLdouble radius, GLint slices, GLint stacks);
__rayapi void myglutCreateWindow(GLuint tx, GLuint ty, signed char rendering, char *name);


#endif
#endif
