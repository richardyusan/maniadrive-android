/*
    Raydium - CQFD Corp.
    http://raydium.org/
    License: GPL - GNU General Public License, see "gpl.txt" file.
*/

#include "headers/myglut.h"

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
//#include <X11/Xutil.h> // provided by glx

#include <GL/glx.h>

#ifdef HAVE_XINERAMA
#include <X11/extensions/Xinerama.h>
#endif

#ifdef HAVE_MOTIF
#include <X11/Xm/MwmUtil.h>
#else
/* bit definitions for MwmHints.flags */
#define MWM_HINTS_FUNCTIONS	(1L << 0)
#define MWM_HINTS_DECORATIONS	(1L << 1)
#define MWM_HINTS_INPUT_MODE	(1L << 2)
#define MWM_HINTS_STATUS	(1L << 3)

/* bit definitions for MwmHints.decorations */
#define MWM_DECOR_ALL		(1L << 0)
#define MWM_DECOR_BORDER	(1L << 1)
#define MWM_DECOR_RESIZEH	(1L << 2)
#define MWM_DECOR_TITLE		(1L << 3)
#define MWM_DECOR_MENU		(1L << 4)
#define MWM_DECOR_MINIMIZE	(1L << 5)
#define MWM_DECOR_MAXIMIZE	(1L << 6)

typedef struct
{
  unsigned long flags       ;
  unsigned long functions   ;
  unsigned long decorations ;
           long inputMode   ;
  unsigned long status      ;
} PropMotifWmHints ;

#define PROP_MOTIF_WM_HINTS_ELEMENTS	5

#endif

#ifndef GLX_SAMPLE_BUFFERS_ARB
#define GLX_SAMPLE_BUFFERS_ARB 100000
#define GLX_SAMPLES_ARB        100001
#endif


Display     *currDisplay ;
XVisualInfo *visualInfo  ;
Window       currHandle  ;
GLXContext   currContext ;
Window       rootWindow  ;
Atom         delWinAtom  ;

int currScreen;
int currConnect;

typedef struct PixelFormat
{
  int num_samples ;
  int bits_per_pixel ;
  int z_bits ;
  int stencil_bits ;
} PixelFormat;

signed char XineramaAndFullscreenFocusHack=0;
signed char FullscreenFlag=0;

PixelFormat preferred_pixel_formats [] =
{
  /* NumSamples, RGB_bits, Z_bits, Stencil */

  {  0, 24, 24,  1 },  /* Progressively nastier image formats */
  {  0, 16, 24,  1 },
  {  0, 16, 16,  1 },
  {  0, 16, 16,  0 },
  { -1, -1, -1, -1 }   /* Magic end marker */
} ;

void pwInit ( int x, int y, int w, int h, int multisample,
              char *title, int border, int num_samples );
int raydium_init_cli_option(char *option, char *value);


// ---------------------- public API

void glutInit(int *argc, char **argv)
{
currDisplay = NULL ;
visualInfo  = NULL ;

currScreen  = 0;
currConnect = 0;

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
  glXDestroyContext ( currDisplay, currContext ) ;
  XDestroyWindow    ( currDisplay, currHandle  ) ;
  XFlush            ( currDisplay ) ;
}



//glutSetCursor
void glutSetCursor(int cursor)
{
int currCursor;
Pixmap pix ;
char   blank_cursor[16*16];
XColor bcol = { 0 } ;

switch(cursor)
    {
    case GLUT_CURSOR_LEFT_ARROW:
	currCursor = XC_left_ptr;
	XDefineCursor( currDisplay, currHandle,
                 XCreateFontCursor ( currDisplay, currCursor ) ) ;
	_glutMouseVisible=1;
	break;
    case GLUT_CURSOR_NONE:	
    default:
	memset(blank_cursor,0,16*16);
      
        pix = XCreateBitmapFromData ( currDisplay,
                                      rootWindow,
                                      blank_cursor, 16, 16 ) ;
        XDefineCursor ( currDisplay, currHandle,
                        XCreatePixmapCursor ( currDisplay,
                            pix, pix, &bcol, &bcol, 0, 0 ) ) ;
        XFreePixmap   ( currDisplay, pix ) ;
	_glutMouseVisible=1;
    }
}

//glutWarpPointer (move mouse)
void glutWarpPointer(int x, int y)
{
  XWarpPointer(currDisplay, None, currHandle, 0, 0, 0, 0, x, y);
  XFlush(currDisplay);
}

//glutSwapBuffers
void glutSwapBuffers(void)
{
  glFlush () ;
  glXSwapBuffers ( currDisplay, currHandle ) ;
  // get events ?
}


//glutMainLoop is generic (myglut.c)

// ------------- private part

void chooseVisual (PixelFormat *pf)
{
  int attribs [ 100 ] ;
  int n = 0 ;

  attribs [n++] = GLX_RGBA ;

  switch ( pf->bits_per_pixel )
  {
    case 3 :
      attribs [n++] = GLX_RED_SIZE   ; attribs [n++] = 1 ;
      attribs [n++] = GLX_GREEN_SIZE ; attribs [n++] = 1 ;
      attribs [n++] = GLX_BLUE_SIZE  ; attribs [n++] = 1 ;
      break ;

    case 16 :
      attribs [n++] = GLX_RED_SIZE   ; attribs [n++] = 5 ;
      attribs [n++] = GLX_GREEN_SIZE ; attribs [n++] = 6 ;
      attribs [n++] = GLX_BLUE_SIZE  ; attribs [n++] = 5 ;
      break ;

    case 24 :
      attribs [n++] = GLX_RED_SIZE   ; attribs [n++] = 8 ;
      attribs [n++] = GLX_GREEN_SIZE ; attribs [n++] = 8 ;
      attribs [n++] = GLX_BLUE_SIZE  ; attribs [n++] = 8 ;
      break ;
  }

  switch ( pf->z_bits )
  {
    case  1 : attribs [n++] = GLX_DEPTH_SIZE ; attribs [n++] =  1 ; break ; 
    case 16 : attribs [n++] = GLX_DEPTH_SIZE ; attribs [n++] = 16 ; break ; 
    case 24 : attribs [n++] = GLX_DEPTH_SIZE ; attribs [n++] = 24 ; break ;
    case 32 : attribs [n++] = GLX_DEPTH_SIZE ; attribs [n++] = 32 ; break ;
  }

  switch ( pf->stencil_bits )
  {
    case  1 : attribs [n++] = GLX_STENCIL_SIZE ; attribs [n++] = 1 ; break ; 
    case  8 : attribs [n++] = GLX_STENCIL_SIZE ; attribs [n++] = 8 ; break ; 
  }

  if ( pf->num_samples > 0 )
  {
    attribs [n++] = GLX_SAMPLE_BUFFERS_ARB ; attribs [n++] = 1 ;
    attribs [n++] = GLX_SAMPLES_ARB        ; attribs [n++] = pf->num_samples ;
  }

  attribs [n++] = GLX_DOUBLEBUFFER ;
  attribs [n++] = None ;

  visualInfo = glXChooseVisual ( currDisplay, currScreen, attribs ) ;
}



void pwInit ( int x, int y, int w, int h, int multisample,
              char *title, int border, int num_samples )
{
  char *displayName = NULL;
  int i;
  int origin[2];
  int size[2];
  int DispX,DispY; // X screen size

  XSetWindowAttributes attribs ;
  XTextProperty   textProperty ;
  XSizeHints         sizeHints ;
  XWMHints             wmHints ;
  unsigned int            mask ;
  PixelFormat               pf ;
  
#ifdef HAVE_XINERAMA
  int i_d1, i_d2;
#endif

  PropMotifWmHints hints  ;
  Atom prop_t ;
  Atom prop   ;

  displayName=getenv ( "DISPLAY" ) ;

  if ( displayName == NULL ) displayName = ":0.0" ;

  currDisplay = XOpenDisplay ( displayName ) ;

  if ( currDisplay == NULL )
  {
    raydium_log("(my)glut: ERROR: Can't open display '%s'",
                            XDisplayName ( displayName ) ) ;
    exit ( 1 ) ;
  }

  /* OpenGL GLX extension availability? */

  if ( ! glXQueryExtension ( currDisplay, NULL, NULL ) )
  {
    raydium_log("(my)glut: ERROR: GLX extension not available on display '%s'",
                                             XDisplayName ( displayName ) ) ;
    exit ( 1 ) ;
  }

  currScreen   = DefaultScreen    ( currDisplay ) ;
  rootWindow   = RootWindow       ( currDisplay, currScreen ) ;
  currConnect  = ConnectionNumber ( currDisplay ) ;
  delWinAtom   = XInternAtom      ( currDisplay, "WM_DELETE_WINDOW", 0 ) ;

  DispX = DisplayWidth  ( currDisplay, currScreen ) ;
  DispY = DisplayHeight ( currDisplay, currScreen ) ;

#ifdef HAVE_XINERAMA
  if(XineramaQueryExtension( currDisplay, &i_d1, &i_d2 ) 
  && XineramaIsActive( currDisplay ) )
  {
  XineramaScreenInfo *screens;
  int num_screens;
  int selected;
  char str[RAYDIUM_MAX_NAME_LEN];

  
  screens = XineramaQueryScreens(currDisplay,&num_screens);
  raydium_log("Xinerama detected with %i screens:",num_screens);

  for(i=0;i<num_screens;i++)
    {
    raydium_log("*** screen %i : %ix%i at (%i,%i)",i,screens[i].width,screens[i].height,screens[i].x_org,screens[i].y_org);
    }

  if(raydium_init_cli_option("xinerama-fullscreen",NULL))
    {
    raydium_log("... but using Xinerama fullscreen anyway !");
    }
    else
    {
    if(raydium_init_cli_option("xinerama-screen",str))
	selected=atoi(str);
    else
	selected=0;

    if(selected<0 || selected >=num_screens)
	{
	raydium_log("invalid screen id !");
	selected=0;
	}
    raydium_log("using Xinerama screen %i",selected);

    x+=screens[selected].x_org;
    y+=screens[selected].y_org;
    DispX=screens[selected].width;
    DispY=screens[selected].height;
    if(w==-1 && h==-1) XineramaAndFullscreenFocusHack=1;
    }

  XFree(screens);
  } // end Xinerama
  else
  {
  raydium_log("no Xinerama on this display");
  }
#else
  raydium_log("no Xinerama support. See config.h for HAVE_XINERAMA symbol");
#endif

  if ( w == -1 && h == -1)
  {
  w = DispX;
  h = DispY;
  FullscreenFlag = 1;
  }

  origin [ 0 ] = x ;
  origin [ 1 ] = y ;
  size   [ 0 ] = w ;
  size   [ 1 ] = h ;

  for (i = 0 ; preferred_pixel_formats [ i ] . num_samples >= 0 ; i++ )
  {
    pf = preferred_pixel_formats [ i ] ;
    pf . num_samples = num_samples ;

    chooseVisual ( &pf ) ;

    if ( visualInfo != NULL )
      break ;
  }

  if ( visualInfo == NULL )
  {
    num_samples = 0 ;

    for (i = 0 ; preferred_pixel_formats [ i ] . num_samples >= 0 ; i++ )
    {
      pf = preferred_pixel_formats [ i ] ;
      pf . num_samples = num_samples ;

      chooseVisual ( &pf ) ;

      if ( visualInfo != NULL )
        break ;
    }

    if ( visualInfo == NULL )
    {
      raydium_log("(my)glut: ERROR: Unable to open a suitable window");
      exit ( 1 ) ;
    }
  }


  attribs.event_mask = StructureNotifyMask | ExposureMask         |
                       ButtonPressMask     | ButtonReleaseMask    |
                       KeyPressMask        | KeyReleaseMask       |
		       EnterWindowMask     | LeaveWindowMask      |
                       PointerMotionMask   | ButtonMotionMask     |
                       VisibilityChangeMask ;

  attribs.background_pixmap = None ;
  attribs.background_pixel  = 0    ;
  attribs.border_pixel      = 0    ;

  attribs.colormap = XCreateColormap ( currDisplay, rootWindow,
                                       visualInfo->visual, AllocNone ) ;

  mask = CWBackPixmap | CWBorderPixel | CWColormap | CWEventMask;

  if(FullscreenFlag)
    {
    attribs.override_redirect = True;
    mask |= CWOverrideRedirect;
    }

  currHandle = XCreateWindow ( currDisplay, rootWindow,
                           x, y, w, h, 0, visualInfo->depth,
                           InputOutput, visualInfo->visual,
                           mask, &attribs ) ;

  currContext = glXCreateContext ( currDisplay, visualInfo, NULL, 1 ) ;

  glXMakeCurrent ( currDisplay, currHandle, currContext ) ;

  if ( ! glXIsDirect ( currDisplay, glXGetCurrentContext() ) )
  {
    raydium_log("(my)glut: WARNING: This is an *INDIRECT* rendering context.") ;
  }

  sizeHints.flags = 0 ;

  if ( x >= 0 && y >= 0 )
    sizeHints.flags |= USPosition ;

  sizeHints.flags |= USSize ;

  sizeHints.x      = x ; sizeHints.y      = y ;
  sizeHints.width  = w ; sizeHints.height = h ;

  if(FullscreenFlag)
    {
    // make this window unresizable
    sizeHints.flags |= PMinSize;
    sizeHints.flags |= PMaxSize;
    sizeHints.min_width=w;
    sizeHints.max_width=w;
    sizeHints.min_height=h;
    sizeHints.max_height=h;
    }


  wmHints.flags = StateHint;
  wmHints.initial_state = NormalState ;


  hints . flags = MWM_HINTS_DECORATIONS ;
  hints . decorations = border ? MWM_DECOR_ALL : 0 ;

  prop_t = prop = XInternAtom ( currDisplay, "_MOTIF_WM_HINTS", True ) ;

  if ( prop != 0 )
    XChangeProperty ( currDisplay, currHandle, prop, prop_t, 32,
                      PropModeReplace, (unsigned char *) &hints,
                      PROP_MOTIF_WM_HINTS_ELEMENTS) ;

  XStringListToTextProperty ( (char **) &title, 1, &textProperty ) ;

  XSetWMProperties ( currDisplay, currHandle,
	                  &textProperty, &textProperty, 0, 0,
                          &sizeHints, &wmHints, NULL ) ;
  XSetWMProtocols  ( currDisplay, currHandle, &delWinAtom  , 1 );
  XMapWindow       ( currDisplay, currHandle ) ;
  glXMakeCurrent   ( currDisplay, currHandle, currContext ) ;

  glutSetCursor(GLUT_CURSOR_LEFT_ARROW);


  glClear ( GL_COLOR_BUFFER_BIT ) ;
  glutSwapBuffers();
  glClear ( GL_COLOR_BUFFER_BIT ) ;
  glutSwapBuffers();

  raydium_log("Found %ix%i with %i bpp color and %i bits zbuffer (stencil is %i)",sizeHints.width,sizeHints.height,pf.bits_per_pixel,pf.z_bits,pf.stencil_bits);

  _glutWindowSize[0]=sizeHints.width;
  _glutWindowSize[1]=sizeHints.height;

  if(FullscreenFlag)
       XGrabKeyboard(currDisplay,currHandle,False,GrabModeAsync,GrabModeAsync,CurrentTime);
//     XSetInputFocus(currDisplay,currHandle,RevertToNone,CurrentTime);
}


void myglutGetEvents (void)
{
  static int size[2]={-1,-1};
  XEvent event ;

  XComposeStatus composeStatus ;
  char           asciiCode [ 32 ] ;
  KeySym         keySym ;

  int len,result;
  signed char special=0;

//  insideCallback = true ;

  while ( XPending ( currDisplay ) )
  {
    int updown = GLUT_DOWN ;

    XNextEvent ( currDisplay, &event ) ;

//    refreshModifiers ( &event ) ;

    switch ( event.type )
    {
      case ClientMessage   : exit(0) ; break ;
      case DestroyNotify   : exit(0) ; break ;

      case ConfigureNotify :      
        if ( currHandle == event.xconfigure.window &&
              ( size[0] != event.xconfigure.width ||
                size[1] != event.xconfigure.height ) )
        {
          size[0] = event.xconfigure.width ;
          size[1] = event.xconfigure.height ;

          glXMakeCurrent ( currDisplay, currHandle, currContext ) ;
          glXWaitX    () ;

          if (glutReshapeFuncCB)
    	    glutReshapeFuncCB(size[0], size[1]);
        }
	break;

      case MappingNotify:
	XRefreshKeyboardMapping ( (XMappingEvent *) &event ) ;
	break;

      case EnterNotify     :
        if(XineramaAndFullscreenFocusHack)
    	    {
	    XSetInputFocus(currDisplay,currHandle,RevertToParent,CurrentTime);
	    XRaiseWindow(currDisplay,currHandle);
	    }
	break;
      case LeaveNotify     :
      case VisibilityNotify:
      case Expose          : break ;

      case MotionNotify    :
        if (glutPassiveMotionFuncCB)
         glutPassiveMotionFuncCB( event.xmotion.x, event.xmotion.y);
        break ;

      case ButtonRelease   :
        updown = GLUT_UP ;
        // FALLTHROUGH

      case ButtonPress     :
        {
          if (glutMouseFuncCB)
            glutMouseFuncCB(event.xbutton.button-1, updown, event.xbutton.x, event.xbutton.y ) ;
        }
        break ;

      case KeyRelease      :
        updown = GLUT_UP ;
        // FALLTHROUGH 

      case KeyPress        :

        len = XLookupString( &event.xkey, asciiCode, sizeof(asciiCode),
                                 &keySym, &composeStatus ) ;
        result = -1 ;

        if( len > 0 )
          result = asciiCode[ 0 ] ;
        else
        {
	special=1;
          switch( keySym )
          {
            case XK_F1:     result = GLUT_KEY_F1;     break;
            case XK_F2:     result = GLUT_KEY_F2;     break;
            case XK_F3:     result = GLUT_KEY_F3;     break;
            case XK_F4:     result = GLUT_KEY_F4;     break;
            case XK_F5:     result = GLUT_KEY_F5;     break;
            case XK_F6:     result = GLUT_KEY_F6;     break;
            case XK_F7:     result = GLUT_KEY_F7;     break;
            case XK_F8:     result = GLUT_KEY_F8;     break;
            case XK_F9:     result = GLUT_KEY_F9;     break;
            case XK_F10:    result = GLUT_KEY_F10;    break;
            case XK_F11:    result = GLUT_KEY_F11;    break;
            case XK_F12:    result = GLUT_KEY_F12;    break;

	    case XK_KP_Left:
            case XK_Left:   result = GLUT_KEY_LEFT;   break;

	    case XK_KP_Right:
            case XK_Right:  result = GLUT_KEY_RIGHT;  break;

	    case XK_KP_Up:
            case XK_Up:     result = GLUT_KEY_UP;     break;

	    case XK_KP_Down:
            case XK_Down:   result = GLUT_KEY_DOWN;   break;

            case XK_KP_Prior:
            case XK_Prior:  result = GLUT_KEY_PAGE_UP; break;
            case XK_KP_Next:
            case XK_Next:   result = GLUT_KEY_PAGE_DOWN; break;
            case XK_KP_Home:
            case XK_Home:   result = GLUT_KEY_HOME;   break;
            case XK_KP_End:
            case XK_End:    result = GLUT_KEY_END;    break;
            case XK_KP_Insert:
            case XK_Insert: result = GLUT_KEY_INSERT; break;
          }
        }

	if(result!=-1)
	    {
	    // check autorepeat with current KeyRelease and next event
	    if (special && XEventsQueued(currDisplay, QueuedAfterReading))
	    {
	    XEvent ahead;
	    XPeekEvent(currDisplay, &ahead);
	    if (ahead.type == KeyPress && event.type == KeyRelease
	        && ahead.xkey.window == event.xkey.window
	        && ahead.xkey.keycode == event.xkey.keycode
	        && ahead.xkey.time == event.xkey.time) 
		{
		// repeating key. Discard event.
		break;
		}
	    }

	    // special down
	    if(special && updown==GLUT_DOWN && glutSpecialFuncCB && !raydium_key[result])
		glutSpecialFuncCB(result,event.xkey.x, event.xkey.y);

	    // special up
	    if(special && updown==GLUT_UP && glutSpecialUpFuncCB && raydium_key[result])
		glutSpecialUpFuncCB(result,event.xkey.x, event.xkey.y);

	    // normal
	    if(!special && updown==GLUT_DOWN && glutKeyboardFuncCB)
		glutKeyboardFuncCB(result,event.xkey.x, event.xkey.y);
	    
	    }

        break ;
    }
  }

//  insideCallback = false ;

  glXMakeCurrent ( currDisplay, currHandle, currContext ) ;
}

