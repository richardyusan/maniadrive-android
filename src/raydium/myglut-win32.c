/*
    Raydium - CQFD Corp.
    http://raydium.org/
    License: GPL - GNU General Public License, see "gpl.txt" file.
*/

#include "headers/myglut.h"

#include <windows.h>
#include <gl/glaux.h>

static HINSTANCE       currInstance;
static HWND            currWnd;
static HDC             currDC;
static HGLRC           currGLRC;

LRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

// ---------------------- public API

void glutInit(int *argc, char **argv)
{
currInstance = NULL ;
currWnd      = NULL ;
currDC       = NULL ;
currGLRC     = NULL ;

glutReshapeFuncCB=NULL;
glutKeyboardFuncCB=NULL;
glutSpecialUpFuncCB=NULL;
glutSpecialFuncCB=NULL;
glutMotionFuncCB=NULL;
glutPassiveMotionFuncCB=NULL;
glutMouseFuncCB=NULL;
glutDisplayFuncCB=NULL;
glutIdleFuncCB=NULL;

_glutMouseVisible=1;
}


void mylgutCloseWindow(void)
{
wglMakeCurrent ( NULL, NULL ) ;
wglDeleteContext ( currGLRC ) ;
DestroyWindow ( currWnd ) ;
}



//glutSetCursor
void glutSetCursor(int cursor)
{

switch(cursor)
    {
    case GLUT_CURSOR_LEFT_ARROW:
	if(_glutMouseVisible) break;
	ShowCursor(1);
	_glutMouseVisible=1;
	break;
    case GLUT_CURSOR_NONE:
    default:
	if(!_glutMouseVisible) break;
	ShowCursor(0);
	_glutMouseVisible=0;
        break;
    break;
    }
//SetCursor( LoadCursor ( NULL, wcursor ) ) ;

}

//glutWarpPointer (move mouse)
void glutWarpPointer(int x, int y)
{
POINT point;

point.x = x;
point.y = y;
ClientToScreen(currWnd /* ? */, &point);
SetCursorPos(point.x, point.y);
}

//glutSwapBuffers
void glutSwapBuffers(void)
{
  glFlush () ;
  SwapBuffers ( currDC ) ;
}


//glutMainLoop is generic (myglut.c)

// ------------- private part




void myglutGetEvents (void)
{
MSG msg;
while ( PeekMessage( &msg, currWnd, 0, 0, PM_REMOVE ) )
    {
    TranslateMessage( &msg ) ;
    DispatchMessage ( &msg ) ;
    }
}

void pwInit ( int x, int y, int w, int h, int multisample,
              char *title, int border, int num_samples )
{
  WNDCLASS wc;
  signed char fullscn;
  RECT rect;
  DWORD style = 0;
  int iPixelFormat;
  int origin [2] = {   0,   0 };
  int size   [2] = { 640, 480 };
  PIXELFORMATDESCRIPTOR pfd;

  
  
  currInstance = GetModuleHandleA( NULL ) ;
  /* Register the window class */
  wc.style         = CS_OWNDC | CS_VREDRAW | CS_HREDRAW ;
  wc.lpfnWndProc   = WndProc ;
  wc.cbClsExtra    = 0 ;
  wc.cbWndExtra    = 0 ;
  wc.hInstance     = currInstance ;
  wc.hIcon         = LoadIcon ( NULL, IDI_APPLICATION ) ;
  wc.hCursor       = LoadCursor ( NULL, IDC_ARROW ) ;
  wc.hbrBackground = ( HBRUSH ) ( COLOR_WINDOW + 1 ) ;
  wc.lpszMenuName  = NULL ;
  wc.lpszClassName = "RaydiumAppClass" ;
  if ( !RegisterClass( &wc ) )
  {
    raydium_log("(my)glut: ERROR: Can't register window class" ) ;
    exit ( 1 ) ;
  }

  /* Setup window style */
  fullscn = (w == -1 && h == -1 ) ? 1 : 0 ;

  if ( w == -1 ) w = GetSystemMetrics( SM_CXSCREEN ) ;
  if ( h == -1 ) h = GetSystemMetrics( SM_CYSCREEN ) ;

  if ( x == -1 ) x = (GetSystemMetrics( SM_CXSCREEN ) / 2) - (w/2) ;
  if ( y == -1 ) y = (GetSystemMetrics( SM_CYSCREEN ) / 2) - (h/2);
  
  origin [ 0 ] = x ;
  origin [ 1 ] = y ;
  size   [ 0 ] = w ;
  size   [ 1 ] = h ;


  rect.left   = x ;
  rect.top    = y ;
  rect.right  = rect.left + w ;
  rect.bottom = rect.top + h ;


  if ( !fullscn )
    style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS ;
  else
    style = WS_POPUP ;

  AdjustWindowRect( &rect, style, 0 ) ;

  /* Create the window */
  currWnd = CreateWindow(
    "RaydiumAppClass", title, style,
    rect.left, rect.top,
    rect.right - rect.left, rect.bottom - rect.top,
    NULL, NULL, currInstance, NULL ) ;

  if ( !currWnd )
  {
    raydium_log("(my)glut: ERROR: Can't create window" ) ;
    exit ( 1 ) ;
  }

  /* Get DC for this window */
  currDC = GetDC ( currWnd ) ;
  if ( !currDC )
  {
    raydium_log("(my)glut: ERROR: Can't get window DC" ) ;
    exit ( 1 ) ;
  }

//  wglGetExtensionsStringARBtype *wglGetExtensionsStringARB = (wglGetExtensionsStringARBtype *)wglGetProcAddress ( "wglGetExtensionsStringARB" ) ;
//  const char *extensionsString = wglGetExtensionsStringARB ( currDC ) ;

//  printf ( "%s %x %s\n", glGetString ( GL_EXTENSIONS ), wglGetExtensionsStringARB, extensionsString ) ;

//  if (!GLExtensionExists("WGL_ARB_multisample "))
//    return suggestedFormat;


  pfd.nSize=sizeof(PIXELFORMATDESCRIPTOR);
  pfd.nVersion=1;
  pfd.dwFlags=PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pfd.iPixelType=PFD_TYPE_RGBA;
  pfd.cColorBits=24;
  pfd.cRedBits=0; 
  pfd.cRedShift=0; 
  pfd.cGreenBits=0; 
  pfd.cGreenShift=0; 
  pfd.cBlueBits=0; 
  pfd.cBlueShift=0;
  pfd.cAlphaBits=0; 
  pfd.cAlphaShift=0; 
  pfd.cAccumBits=0; 
  pfd.cAccumRedBits=0; 
  pfd.cAccumGreenBits=0; 
  pfd.cAccumBlueBits=0; 
  pfd.cAccumAlphaBits=0;
  pfd.cDepthBits=32;
  pfd.cStencilBits=1;
  pfd.cAuxBuffers=0;
  pfd.iLayerType=PFD_MAIN_PLANE;
  pfd.bReserved=0; 
  pfd.dwLayerMask=0; 
  pfd.dwVisibleMask=0; 
  pfd.dwDamageMask=0; 

  
  /* Get best available match of pixel format for DC */
  iPixelFormat = ChoosePixelFormat ( currDC, &pfd ) ;
  if ( iPixelFormat == 0 )
  {
    raydium_log("(my)glut: ERROR: Can't get OpenGL pixel format" ) ;
    exit ( 1 ) ;
  }

  /* Set the pixel format of the DC */
  if ( !SetPixelFormat( currDC, iPixelFormat, &pfd ) )
  {
    raydium_log("(my)glut: ERROR: Can't set OpenGL pixel format" ) ;
    exit ( 1 ) ;
  }

  /* Show the window and paint its contents */
  ShowWindow ( currWnd, SW_SHOW ) ;
  UpdateWindow ( currWnd ) ;

  /* Create openGL context and make it current */
  currGLRC = wglCreateContext ( currDC ) ;
  if ( !currGLRC )
  {
    raydium_log("(my)glut: ERROR: Unable to open a suitable window");
    exit ( 1 ) ;
  }
  if ( !wglMakeCurrent ( currDC, currGLRC ) )
  {
    raydium_log("(my)glut: ERROR: Unable to open a suitable window");
    exit ( 1 ) ;
  }
  

  glClear ( GL_COLOR_BUFFER_BIT ) ;
  glutSwapBuffers () ;
  glClear ( GL_COLOR_BUFFER_BIT ) ;
  glutSwapBuffers () ;
  
//  if (glutReshapeFuncCB)
//      glutReshapeFuncCB(w, h);
  DescribePixelFormat(currDC, iPixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd); 
  raydium_log("Found %ix%i with %i bpp color and %i bits zbuffer (stencil is %i)",w,h,pfd.cColorBits,pfd.cDepthBits,pfd.cStencilBits);
}



LRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
  static int key = -1 ;
  static int updown = GLUT_UP ;
  static int old_key = -1 ;  // We need this because "key" changes case
  int button = -1 ;
  static int mb = 0 ;
  static int lastx = 0 ;
  static int lasty = 0 ;
  BYTE state[256];
  WORD code[2];
  signed char b;
  signed char special=0;
  signed char repeating=0;
  int size[2];

  switch (uMsg)
  {
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
      updown = GLUT_DOWN ;
      if      ( uMsg == WM_LBUTTONDOWN ) { button = GLUT_LEFT_BUTTON   ; mb |= 1 ; }
      else if ( uMsg == WM_MBUTTONDOWN ) { button = GLUT_MIDDLE_BUTTON ; mb |= 2 ; }
      else                               { button = GLUT_RIGHT_BUTTON  ; mb |= 4 ; }
      SetCapture ( hWnd ) ;
      lastx = (LOWORD( lParam ) ) ;
      lasty = (HIWORD( lParam ) ) ;
      if ( button != -1 && glutMouseFuncCB )
        (*glutMouseFuncCB) ( button, updown, lastx, lasty ) ;
      break;

    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
      updown = GLUT_UP ;
      if      ( uMsg == WM_LBUTTONUP ) { button = GLUT_LEFT_BUTTON   ; mb &= ~1 ; }
      else if ( uMsg == WM_MBUTTONUP ) { button = GLUT_MIDDLE_BUTTON ; mb &= ~2 ; }
      else                             { button = GLUT_RIGHT_BUTTON  ; mb &= ~4 ; }
      if ( mb == 0 ) ReleaseCapture () ;
      lastx = (LOWORD( lParam ) ) ;
      lasty = (HIWORD( lParam ) ) ;
      if ( button != -1 && glutMouseFuncCB )
        (*glutMouseFuncCB) ( button, updown, lastx, lasty ) ;
      break;

    case WM_MOUSEMOVE:
      lastx = (LOWORD( lParam ) );
      lasty = (HIWORD( lParam ) );
      if ( glutPassiveMotionFuncCB )
        (*glutPassiveMotionFuncCB) ( lastx, lasty ) ;
      break;

    case WM_MOUSEWHEEL:
    if ( ( short ) HIWORD( wParam ) > 0 )
    {
    if ( glutPassiveMotionFuncCB )    
        (*glutMouseFuncCB) ( 3, GLUT_DOWN, lastx, lasty ) ;
    if ( glutPassiveMotionFuncCB )    
        (*glutMouseFuncCB) ( 3, GLUT_UP, lastx, lasty ) ;
    }
    else
    {
    if ( glutPassiveMotionFuncCB )    
        (*glutMouseFuncCB) ( 4, GLUT_DOWN, lastx, lasty ) ;
    if ( glutPassiveMotionFuncCB )    
        (*glutMouseFuncCB) ( 4, GLUT_UP, lastx, lasty ) ;
    }
      
    case WM_KEYDOWN:
      // If the key is already down, we are on auto-repeat.  Break if the autorepeat is disabled.
      if ( ( updown == GLUT_DOWN ) && ( (int)wParam == old_key ) )
      {
/*        if ( autoRepeat )
        {*/
          // Disable CTRL, SHIFT, CapsLock keys from making a callback
          if ( ( key == VK_CONTROL ) || ( key == VK_SHIFT ) || ( key == VK_CAPITAL ) )
            break ;

/*          if ( key != -1 && kbCB )  // Autorepeat enabled, call the callback with an "up" setting
            (*kbCB) ( key, PW_UP, lastx, lasty ) ;
        }
        else*/
          repeating=1;
        }

      updown = GLUT_DOWN ;
      old_key = wParam ;
      // FALLTHROUGH
    case WM_KEYUP:
      if ( uMsg == WM_KEYUP ) updown = GLUT_UP ;
      key = wParam ;

      // Disable CTRL, SHIFT, CapsLock keys from making a callback
      if ( ( key == VK_CONTROL ) || ( key == VK_SHIFT ) || ( key == VK_CAPITAL ) )
        break ;

      switch ( key )
      {
        case VK_F1:     key = GLUT_KEY_F1;   special=1;     break;
        case VK_F2:     key = GLUT_KEY_F2;   special=1;     break;
        case VK_F3:     key = GLUT_KEY_F3;   special=1;     break;
        case VK_F4:     key = GLUT_KEY_F4;   special=1;     break;
        case VK_F5:     key = GLUT_KEY_F5;   special=1;     break;
        case VK_F6:     key = GLUT_KEY_F6;   special=1;     break;
        case VK_F7:     key = GLUT_KEY_F7;   special=1;     break;
        case VK_F8:     key = GLUT_KEY_F8;   special=1;     break;
        case VK_F9:     key = GLUT_KEY_F9;   special=1;     break;
        case VK_F10:    key = GLUT_KEY_F10;  special=1;     break;
        case VK_F11:    key = GLUT_KEY_F11;  special=1;     break;
        case VK_F12:    key = GLUT_KEY_F12;  special=1;     break;

        case VK_LEFT:   key = GLUT_KEY_LEFT;   special=1;   break;
        case VK_RIGHT:  key = GLUT_KEY_RIGHT;  special=1;   break;
        case VK_UP:     key = GLUT_KEY_UP;     special=1;   break;
        case VK_DOWN:   key = GLUT_KEY_DOWN;   special=1;   break;

        case VK_PRIOR:  key = GLUT_KEY_PAGE_UP;   special=1; break;
        case VK_NEXT:   key = GLUT_KEY_PAGE_DOWN; special=1; break;
        case VK_HOME:   key = GLUT_KEY_HOME;      special=1; break;
        case VK_END:    key = GLUT_KEY_END;       special=1; break;
        case VK_INSERT: key = GLUT_KEY_INSERT;    special=1; break;

				default:
					// don't do this for WinCE


          b = GetKeyboardState( state );
					assert(b);

					code [ 0 ] = 0; // WK: I need to do this, or on my Win2k box, the upper bits remain unchanged.
          if( ToAscii( key, 0, state, code, 0 ) == 1 )
						if((0xFF00 & code[0]) == 0) // setting a high bit in key causes crashes later on (out of range array access)
							key=code[ 0 ];

      }
      if ( key != -1)
        {

	    // special down
	    if(special && updown==GLUT_DOWN && glutSpecialFuncCB && !repeating)
		glutSpecialFuncCB(key,lastx,lasty);

	    // special up
	    if(special && updown==GLUT_UP && glutSpecialUpFuncCB && !repeating)
		glutSpecialUpFuncCB(key,lastx,lasty);
        
        // normal
	    if(!special && updown==GLUT_DOWN && glutKeyboardFuncCB)
		glutKeyboardFuncCB(key,lastx,lasty);

        
        /*if(!special && repeating && glutKeyboardFuncCB)
        glutKeyboardFuncCB(key,lastx,lasty);*/
        
        //(*kbCB) ( key, updown, lastx, lasty ) ;
        }
      break;

    case WM_SIZE:
      size[0] = LOWORD(lParam) ;
      size[1] = HIWORD(lParam) ;
      _glutWindowSize[0]=size[0];
      _glutWindowSize[1]=size[1];    
      if (glutReshapeFuncCB)
    	    glutReshapeFuncCB(size[0], size[1]);
      break;

    case WM_CLOSE:
      exit(0);
      break;

    default:
      return ( DefWindowProc ( hWnd, uMsg, wParam, lParam ) ) ;
  }

  return 0;
}
