/*
     PLIB - A Suite of Portable Game Libraries
     Copyright (C) 2001  Steve Baker
 
     This library is free software; you can redistribute it and/or
     modify it under the terms of the GNU Library General Public
     License as published by the Free Software Foundation; either
     version 2 of the License, or (at your option) any later version.
 
     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Library General Public License for more details.
 
     You should have received a copy of the GNU Library General Public
     License along with this library; if not, write to the Free
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 
     For further information visit http://plib.sourceforge.net

     $Id$
*/

#include "puLocal.h"

#ifndef WIN32
#  ifndef macintosh
#    include <GL/glx.h>
#  else
#    include <agl.h>
#  endif
#endif

#ifdef PU_NOT_USING_GLUT
#include <iostream.h>
#endif

#define PU_STRING_X_FUDGE 6
#define PU_STRING_Y_FUDGE 6

int puRefresh = TRUE ;

static int puWindowWidth  = 400 ;
static int puWindowHeight = 400 ;

static int openGLSize = 0 ;

#ifdef PU_NOT_USING_GLUT

int puGetWindow       () { return 0              ; }
void puSetWindow ( int w ) {}
int puGetWindowHeight () { return puWindowHeight ; }
int puGetWindowWidth  () { return puWindowWidth  ; }

void puSetWindowSize ( int width, int height )
{
  puWindowWidth  = width  ;
  puWindowHeight = height ;
}

static int fontBase = 0;
static int fontSize[257];
#else


void puSetResizeMode ( int mode ) {
  openGLSize = mode ;
}


int puGetWindow ()
{
  return glutGetWindow () ;
}


void puSetWindow ( int w ) {  glutSetWindow ( w ) ;  }

int puGetWindowHeight ()
{
  if ( ! openGLSize )
        return glutGet ( (GLenum) GLUT_WINDOW_HEIGHT ) ;
  else
        return puWindowHeight ;
}


int puGetWindowWidth ()
{
  if ( ! openGLSize )
        return glutGet ( (GLenum) GLUT_WINDOW_WIDTH ) ;
  else
        return puWindowWidth ;
}


void puSetWindowSize ( int width, int height )
{
  if ( ! openGLSize )
    ulSetError ( UL_WARNING, "PUI: puSetWindowSize shouldn't be used with GLUT." ) ;
  else
  {
    glutReshapeWindow ( width, height ) ;
    puWindowWidth  = width  ;
    puWindowHeight = height ;
  }
}

#endif

puColour _puDefaultColourTable[] =
{
  { 0.5f, 0.5f, 0.5f, 1.0f }, /* PUCOL_FOREGROUND */
  { 0.3f, 0.3f, 0.3f, 1.0f }, /* PUCOL_BACKGROUND */
  { 0.7f, 0.7f, 0.7f, 1.0f }, /* PUCOL_HIGHLIGHT  */
  { 0.0f, 0.0f, 0.0f, 1.0f }, /* PUCOL_LABEL      */
  { 1.0f, 1.0f, 1.0f, 1.0f }, /* PUCOL_TEXT       */

  { 0.0f, 0.0f, 0.0f, 0.0f }  /* ILLEGAL */
} ;
 

static int glIsValidContext ()
{
#if defined(CONSOLE)
  return true ;
#elif defined(WIN32)
  return ( wglGetCurrentContext () != NULL ) ;
#elif defined(macintosh)
  return ( aglGetCurrentContext() != NULL ) ;
#else
  return ( glXGetCurrentContext() != NULL ) ;
#endif
}

puValue::~puValue () {}  

static int _puCursor_enable = FALSE ;
static int _puCursor_x      = 0 ;
static int _puCursor_y      = 0 ;
static float _puCursor_bgcolour [4] = { 1.0f, 1.0f, 1.0f, 1.0f } ; 
static float _puCursor_fgcolour [4] = { 0.0f, 0.0f, 0.0f, 1.0f } ;  

void   puHideCursor ( void ) { _puCursor_enable = FALSE ; }
void   puShowCursor ( void ) { _puCursor_enable = TRUE  ; }
int    puCursorIsHidden ( void ) { return ! _puCursor_enable ; }

void puCursor ( int x, int y )
{
  _puCursor_x = x ;
  _puCursor_y = y ;
}


static void puDrawCursor ( int x, int y )
{
  glColor4fv ( _puCursor_bgcolour ) ;  

  glBegin    ( GL_TRIANGLES ) ;
  glVertex2i ( x, y ) ;
  glVertex2i ( x + 13, y -  4 ) ;
  glVertex2i ( x +  4, y - 13 ) ;

  glVertex2i ( x +  8, y -  3 ) ;
  glVertex2i ( x + 17, y - 12 ) ;
  glVertex2i ( x + 12, y - 17 ) ;

  glVertex2i ( x + 12, y - 17 ) ;
  glVertex2i ( x +  3, y -  8 ) ;
  glVertex2i ( x +  8, y -  3 ) ;
  glEnd      () ;

  glColor4fv ( _puCursor_fgcolour ) ;  

  glBegin    ( GL_TRIANGLES ) ;
  glVertex2i ( x+1, y-1 ) ;
  glVertex2i ( x + 11, y -  4 ) ;
  glVertex2i ( x +  4, y - 11 ) ;

  glVertex2i ( x +  8, y -  5 ) ;
  glVertex2i ( x + 15, y - 12 ) ;
  glVertex2i ( x + 12, y - 15 ) ;

  glVertex2i ( x + 12, y - 15 ) ;
  glVertex2i ( x +  5, y -  8 ) ;
  glVertex2i ( x +  8, y -  5 ) ;
  glEnd      () ;
}


// Pointer to linked list of objects to delete
// as a result of keyboarding or mouse clicking

static puObject *objects_to_delete = NULL;


void puDeleteObject ( puObject *ob )
{
  puGroup *parent = ob->getParent () ;
  
  // Remove from parent interface
  if ( parent != ob && parent != NULL )
    parent -> remove ( ob ) ;

  // Pop live interface
  if ( ob -> getType () & PUCLASS_INTERFACE )
    puPopLiveInterface ( (puInterface*)ob ) ;

  ob -> prev = NULL ;                       // Add to linked list to be deleted
  ob -> next = objects_to_delete ;
  objects_to_delete = ob ;
  ob -> setParent ( NULL ) ;
}


static void puCleanUpJunk ()
{
  /* Step through the linked list of objects to delete, removing them. */
  while ( objects_to_delete != NULL )
  {
    puObject *next_ob = objects_to_delete ->next ;
    delete objects_to_delete ;
    objects_to_delete = next_ob ;
  }
}


static puObject *active_widget ;   /* Widget which is currently receiving user input */
static char *input_paste_buffer ;  /* Cut/Copy/Paste buffer for input widgets */

void puInit ( void )
{
  static int firsttime = TRUE ;

  if ( firsttime )
  {
    if ( glIsValidContext () == 0 )
    {
      ulSetError ( UL_FATAL,
        "puInit called without a valid OpenGL context.");
    }

    puInterface *base_interface = new puInterface ( 0, 0 ) ;
    puPushGroup         ( base_interface ) ;
    puPushLiveInterface ( base_interface ) ;

    active_widget = NULL ;
    input_paste_buffer = NULL ;

    firsttime = FALSE ;
#ifdef PU_NOT_USING_GLUT

    // No GLUT fonts, try some corresponding "fnt" fonts

    static fntTexFont typewriter  ( "c:/plib/examples/src/fnt/data/typewriter.txf" ) ;
    static fntTexFont times_roman ( "c:/plib/examples/src/fnt/data/times_bold.txf" ) ;
    static fntTexFont helvetica   ( "c:/plib/examples/src/fnt/data/helvetica_bold.txf" ) ;

    PUFONT_8_BY_13.initialize        ( &typewriter, 13 ) ;
    PUFONT_9_BY_15.initialize        ( &typewriter, 15 ) ;
    PUFONT_TIMES_ROMAN_10.initialize ( &times_roman, 10 ) ;
    PUFONT_TIMES_ROMAN_24.initialize ( &times_roman, 24 ) ;
    PUFONT_HELVETICA_10.initialize   ( &helvetica, 10 ) ;
    PUFONT_HELVETICA_12.initialize   ( &helvetica, 12 ) ;
    PUFONT_HELVETICA_18.initialize   ( &helvetica, 18 ) ;

    /* Create bitmaps for the device context font's first 256 glyphs */

//    fontBase = glGenLists(256);
//    assert(fontBase);
//    HDC hdc = wglGetCurrentDC();

    /* Make the system font the device context's selected font */

//    SelectObject (hdc, GetStockObject (SYSTEM_FONT)); 

//    int *tempSize = &fontSize[1];

//    if ( ! GetCharWidth32 ( hdc, 1, 255, tempSize ) &&
//         ! GetCharWidth   ( hdc, 1, 255, tempSize ) )
//    {
//      LPVOID lpMsgBuf ;

//      FormatMessage ( FORMAT_MESSAGE_ALLOCATE_BUFFER |
//                      FORMAT_MESSAGE_FROM_SYSTEM,
//                      NULL,
//                      GetLastError(),
//                      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
//                      (LPTSTR) &lpMsgBuf,
//                      0, NULL ) ;

//      ulSetError ( UL_WARNING, "PUI: Error: %s", (char *)lpMsgBuf ) ;
//      LocalFree ( lpMsgBuf ) ;
//    }

//    wglUseFontBitmaps ( hdc, 0, 256, fontBase ) ;
#endif
  }
}

static void puSetOpenGLState ( void )
{
  int w = puGetWindowWidth  () ;
  int h = puGetWindowHeight () ;

  if ( ! openGLSize )
    glPushAttrib   ( GL_ENABLE_BIT | GL_TRANSFORM_BIT | GL_LIGHTING_BIT ) ;
  else
  glPushAttrib   ( GL_ENABLE_BIT | GL_VIEWPORT_BIT | GL_TRANSFORM_BIT | GL_LIGHTING_BIT ) ;

  glDisable      ( GL_LIGHTING   ) ;
  glDisable      ( GL_FOG        ) ;
  glDisable      ( GL_TEXTURE_2D ) ;
  glDisable      ( GL_DEPTH_TEST ) ;
  glDisable      ( GL_CULL_FACE  ) ;
 
  if ( ! openGLSize )
  glViewport     ( 0, 0, w, h ) ;
 
  glMatrixMode   ( GL_PROJECTION ) ;
  glPushMatrix   () ;
  glLoadIdentity () ;
  gluOrtho2D     ( 0, w, 0, h ) ;
  glMatrixMode   ( GL_MODELVIEW ) ;
  glPushMatrix   () ;
  glLoadIdentity () ;
}

static void puRestoreOpenGLState ( void )
{
  glMatrixMode   ( GL_PROJECTION ) ;
  glPopMatrix    () ;
  glMatrixMode   ( GL_MODELVIEW ) ;
  glPopMatrix    () ;
  glPopAttrib    () ;
}


void  puDisplay ( void )
{
  puCleanUpJunk () ;

  puSetOpenGLState () ;
  puGetUltimateLiveInterface () -> draw ( 0, 0 ) ;

  int h = puGetWindowHeight () ;

  if ( _puCursor_enable )
    puDrawCursor ( _puCursor_x,
                   h - _puCursor_y ) ;

  puRestoreOpenGLState () ;
}


void  puDisplay ( int window_number )  /* Redraw only the current window */
{
  puCleanUpJunk () ;

  puSetOpenGLState () ;
  puInterface *base_interface = puGetUltimateLiveInterface () ;
  puObject *ob = base_interface -> getFirstChild () ;
  while ( ob )
  {
    if ( ob -> getWindow () == window_number )
      ob -> draw ( 0, 0 ) ;

    ob = ob -> getNextObject () ;
  }

  int h = puGetWindowHeight () ;

  if ( _puCursor_enable )
    puDrawCursor ( _puCursor_x,
                   h - _puCursor_y ) ;

  puRestoreOpenGLState () ;
}


int puKeyboard ( int key, int updown )
{
  int return_value = puGetBaseLiveInterface () -> checkKey ( key, updown ) ;
  
  puCleanUpJunk () ;
  
  return return_value ;
}


static int last_buttons = 0 ;
static int pu_mouse_x = 0 ;
static int pu_mouse_y = 0 ;
static int pu_mouse_offset_x = 0 ;
static int pu_mouse_offset_y = 0 ;

int puMouse ( int button, int updown, int x, int y )
{
  puCursor ( x, y ) ;
  
  int h = puGetWindowHeight () ;
  
  if ( updown == PU_DOWN )
    last_buttons |=  ( 1 << button ) ;
  else
    last_buttons &= ~( 1 << button ) ;

  pu_mouse_x = x ;
  pu_mouse_y = h - y ;
  int return_value =  puGetBaseLiveInterface () -> checkHit ( button,
    updown, pu_mouse_x, pu_mouse_y ) ;
  
  puCleanUpJunk () ;
  
  if ( ( last_buttons == 0 ) && puActiveWidget () )
  {
    if ( puActiveWidget ()->deactivateWhenLeaving () )
      puDeactivateWidget () ;
  }

  return return_value ;
}


int puMouse ( int x, int y )
{
  puCursor ( x, y ) ;
  
  if ( last_buttons == 0 )
    return FALSE ;

  int button =
    (last_buttons & (1<<PU_LEFT_BUTTON  )) ?  PU_LEFT_BUTTON   :
    (last_buttons & (1<<PU_MIDDLE_BUTTON)) ?  PU_MIDDLE_BUTTON :
    (last_buttons & (1<<PU_RIGHT_BUTTON )) ?  PU_RIGHT_BUTTON  : 0 ;

  int h = puGetWindowHeight () ;

  pu_mouse_x = x ;
  pu_mouse_y = h - y ;

  /*
    When you drag over an ACTIVE widget, you don't
    affect any other widgets until you release the
    mouse button.
  */

  if ( puActiveWidget () )
  {
    puActiveWidget()->doHit(button, PU_DRAG, pu_mouse_x - pu_mouse_offset_x,
                                             pu_mouse_y - pu_mouse_offset_y) ;
    return TRUE ;
  }

  int return_value = puGetBaseLiveInterface () -> checkHit ( button,
    PU_DRAG, x, h - y ) ;
  
  puCleanUpJunk () ;
  
  return return_value ;
}

void puMoveToLast (puObject *ob)
{
  puGroup *parent = ob -> getParent () ;

  /* If no parent interface, return. */

  if ( ! parent ) return;

  /* Remove "ob" from present place in the "dlist" list */

  parent -> remove (ob) ;

  /* Place at the end of the list */

  parent -> add (ob) ;

  /*
    Now repeat the process for the parent interface so that the interface will
    be drawn last of all interfaces.
  */

  puMoveToLast ( parent );
}

void puDeactivateWidget ()  {  active_widget = NULL ; }
void puSetActiveWidget ( puObject *w, int x, int y )
{
  active_widget = w ;
  pu_mouse_offset_x = pu_mouse_x - x ;
  pu_mouse_offset_y = pu_mouse_y - y ;
}

puObject *puActiveWidget () {   return active_widget ; }

void puSetPasteBuffer ( char *ch )
{
  if ( input_paste_buffer ) delete input_paste_buffer ;
  input_paste_buffer = new char [ strlen(ch) + 1 ] ;
  strcpy ( input_paste_buffer, ch ) ;
}

char *puGetPasteBuffer ()  {  return input_paste_buffer ;  }

