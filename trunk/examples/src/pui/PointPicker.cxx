// Point Picker Program by John Fay, October 2000.
// This is a demonstration program for PUI.  It displays a set of points in a window
// and allows the user to pick them with the right mouse button.  It demonstrates
// the following PUI capabilities:
//    - puInput widget
//    - puOneShot widget
//    - active widget functionality, with up, active, and down callbacks
//    - valuators

// To use it, the user does the following:
//   First Method:  Click in the input widget and type the number of the point
//   Second Method: Click in the input widget and right-click on a point to select it.

//   For both methods, clicking the left (one-shot) button moves the point to the
//   right.  When it gets far enough to the right, it wraps around to the left and
//   moves slightly upwards.

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <math.h>
#include <GL/glut.h>
#include "plib/pu.h"

//#define VOODOO 1

/***********************************\
*                                   *
* These are the PUI widget pointers *
*                                   *
\***********************************/

int           main_window ;
puText       *instruction ;
puInput      *point_no    ;
puOneShot    *move_point  ;
puButton     *exit_button ;
puLargeInput *input ;

int picked_point ;
int mouse_x, mouse_y ;  // Mouse coordinates

fntTexFont *tim ;

/**********************************\
*                                  *
*  Here are the point coordinates  *
*                                  *
\**********************************/

typedef struct {
  float x, y, z ;
} node ;

#define NUMBER_POINTS     30

node point[NUMBER_POINTS] ;

static int firsttime;

void drawSinglePickableObject ( int i, float x, float y, float z )
{
  // I realize that putting the point size, normal, and begin/end inside the for-loop
  // is less efficient, but for the purposes of the demo I want everything required
  // to draw a single pickable object in this routine.

  glPointSize ( 10.0f ) ;
  glBegin ( GL_POINTS ) ;
  glNormal3f ( 1.0, 0.0, 0.0 ) ;  // Needed so the points show color

  if ( i == picked_point )
    glColor4f ( 1.0, 0.0, 0.0, 1.0 ) ;
  else
    glColor4f ( 0.0, 1.0, 0.0, 1.0 ) ;

  glVertex3f ( x, y, z ) ;

  glEnd () ;
}

void drawPoints (void)
{

  if ( firsttime )
  {
    /*
      Deliberately do this only once - it's a better test of
      PUI's attempts to leave the OpenGL state undisturbed
    */
    glEnable       ( GL_BLEND     ) ;
    glBlendFunc    ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glAlphaFunc(GL_GREATER,0.1f);

    firsttime = FALSE ;

    glMatrixMode   ( GL_PROJECTION ) ;
    gluPerspective ( 40.0, 1.0, 1.0, 10.0 ) ;
    glMatrixMode   ( GL_MODELVIEW ) ;
    gluLookAt      ( 0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0 ) ;

    // Assign point coordinates (random values between zero and one)

    for ( int i = 0; i < NUMBER_POINTS; i++ )
    {
      point[i].x = ( (float)    i                             / (float)NUMBER_POINTS * 2 ) - 1 ;
      point[i].y = ( (float)( ( i * i * i  ) % NUMBER_POINTS) / (float)NUMBER_POINTS * 2 ) - 1 ;
      point[i].z = ( (float)( ( i * i      ) % NUMBER_POINTS) / (float)NUMBER_POINTS * 2 ) - 1 ;
    }
  }

  GLfloat matrix[16] ;
  glGetFloatv ( GL_MODELVIEW_MATRIX, matrix ) ;
  glGetFloatv ( GL_PROJECTION_MATRIX, matrix ) ;

  for ( int i = 0 ; i < NUMBER_POINTS ; i++ )
  {
    drawSinglePickableObject ( i, point[i].x, point[i].y, point[i].z ) ;
  }
}


/**************************************\
*                                      *
* These three functions capture mouse  *
* and keystrokes (special and mundane) *
* from GLUT and pass them on to PUI.   *
*                                      *
\**************************************/

static void specialfn ( int key, int, int )
{
  puKeyboard ( key + PU_KEY_GLUT_SPECIAL_OFFSET, PU_DOWN ) ;
  glutPostRedisplay () ;
}

static void keyfn ( unsigned char key, int, int )
{
  puKeyboard ( key, PU_DOWN ) ;
  glutPostRedisplay () ;
}

static void motionfn ( int x, int y )
{
  puMouse ( x, y ) ;
  glutPostRedisplay () ;
}

puObject *active_widget = (puObject *)NULL ;
static void mousefn ( int button, int updown, int x, int y )
{
  if ( updown == PU_UP ) active_widget = puActiveWidget () ;
  if ( !puMouse ( button, updown, x, y ) )
  {
    // PUI didn't take the mouseclick, try the main window

    mouse_x = x ;
    mouse_y = puGetWindowHeight () - y ;

    // Check for an active widget.  If there is one, call its active callback

    if ( active_widget )
    {
      active_widget -> invokeActiveCallback () ;
      // Make sure PUI keeps the active widget active
      puSetActiveWidget ( active_widget, 0, 0 ) ;
    }
  }

  glutPostRedisplay () ;
}

/**************************************\
*                                      *
* This function redisplays the PUI and *
* the points, flips the double buffer  *
* and then asks GLUT to post a         *
* redisplay command - so we re-render  *
* at maximum rate.                     *
*                                      *
\**************************************/

static void displayfn (void)
{
  /* Clear the screen */

  glClearColor ( 0.4, 0.1, 0.1, 1.0 ) ;
  glClear      ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;

  /* Draw the points */

  drawPoints  () ;

  /* Make PUI redraw */

  puDisplay () ;
  
  /* Off we go again... */

  glutSwapBuffers   () ;
  glutPostRedisplay () ;
}

/***********************************\
*                                   *
* Here are the PUI widget callback  *
* functions.                        *
*                                   *
\***********************************/

// Point Number Input Widget Up Callback

void point_no_up_cb ( puObject *ob )
{
  printf ( "Calling the up-callback with partial data: %d\n", picked_point ) ;
  if ( input->inputDisabled () )
    input->enableInput () ;
  else
    input->disableInput () ;
}

// Point Number Input Widget Active Callback

void point_no_active_cb ( puObject *ob )
{
  int viewport[4] ;  // Viewport
  glGetIntegerv ( GL_VIEWPORT, viewport ) ;

  unsigned int buffer[512];
  glSelectBuffer ( 512L, buffer );    // Set up the buffer to put hits into
  glRenderMode ( GL_SELECT ) ;      // Go into Select mode

  glInitNames ();              // Empty the name stack

  glMatrixMode ( GL_PROJECTION ) ;  // Set the Projection matrix for picking
  glPushMatrix () ;  // Save the current matrix
  glLoadIdentity();

  // Many thanks to Dave McClurg for getting the transformation matrices to work correctly
  /*  create 10x10 pixel picking region near cursor location */
  gluPickMatrix ( (double)mouse_x, (double)mouse_y,
    10.0f, 10.0f, viewport ) ;

  gluPerspective ( 40.0, 1.0, 1.0, 10.0 ) ;

  for ( int i = 0 ; i < NUMBER_POINTS ; i++ )
  {
    glPushName ( i ) ;
    drawSinglePickableObject ( i, point[i].x, point[i].y, point[i].z ) ;
    glPopName () ;
  }

  glPopMatrix();
  glFlush();

  long hits = glRenderMode ( GL_RENDER ) ;  // Back to Rendering mode, get number of hits

  if ( hits )
    point_no->setValue ( (int)buffer[3] ) ;

  printf ( "Active point %d %d %d\n", mouse_x, mouse_y, hits ) ;

  glMatrixMode ( GL_MODELVIEW );  // got set to GL_PROJECTION earlier
}

// Point Number Input Widget Down Callback

void point_no_down_cb ( puObject *ob )
{
  printf ( "Calling the down callback with full data: %d\n", picked_point ) ;
  if ( picked_point < 0 )
  {
    printf ( "Value too small, setting it to zero\n" ) ;
    picked_point = 0 ;
  }

  if ( picked_point > NUMBER_POINTS - 1 )
  {
    printf ( "Value too big, setting it to %d\n", NUMBER_POINTS - 1 ) ;
    picked_point = NUMBER_POINTS - 1 ;
  }

  point_no->setValue ( picked_point ) ;
}

// Move Point Button Callback

void move_point_cb ( puObject *ob )
{
  point[picked_point].x += 0.1 ;
  if ( point[picked_point].x > 1.0 )
  {
    point[picked_point].x = -1.0 ;
    point[picked_point].y += 0.1 ;
  }
}

void exit_cb ( puObject * )
{
  fprintf ( stderr, "Exiting PUI demo program.\n" ) ;
  exit ( 1 ) ;
}


int main ( int argc, char **argv )
{
  firsttime = TRUE;
  picked_point = 0 ;

  // Set up the window

  glutInitWindowPosition( 100,   0 ) ;
  glutInitWindowSize    ( 640, 480 ) ;
  glutInit              ( &argc, argv ) ;
  glutInitDisplayMode   ( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH ) ;
  main_window = glutCreateWindow      ( "Point Picker PUI Application"  ) ;
  glutDisplayFunc       ( displayfn ) ;
  glutKeyboardFunc      ( keyfn     ) ;
  glutSpecialFunc       ( specialfn ) ;
  glutMouseFunc         ( mousefn   ) ;
  glutMotionFunc        ( motionfn  ) ;
  glutPassiveMotionFunc ( motionfn  ) ;
  glutIdleFunc          ( displayfn ) ;

  puInit () ;

#ifdef VOODOO
  puShowCursor () ;
#endif

  // Set up the font

  tim = new fntTexFont ;
  tim -> load ( "../fnt/data/times_medium.txf" ) ;
  puFont times_medium ( tim, 15, .5 ) ;
  puSetDefaultFonts        ( times_medium, times_medium ) ;
  puSetDefaultStyle        ( PUSTYLE_SMALL_SHADED ) ;
  puSetDefaultColourScheme ( 0.1, 0.8, 0.1, 1.0) ;

  // Set up the widgets in the window

  instruction = new puText ( 10, 450 ) ;
  instruction -> setColour ( PUCOL_LABEL, 1.0, 1.0, 1.0 ) ;
  instruction -> setLabel ( "Enter a point number in the input box, or\n"
                            "left-click in the input box and right-click on a point." ) ;

  point_no = new puInput ( 150, 60, 240, 80 ) ;
  point_no->setValuator ( &picked_point ) ;
  point_no->setLabel ( "Point Number" ) ;
  point_no->setLabelPlace ( PUPLACE_LEFT_CENTER ) ;
  point_no->setCallback ( point_no_up_cb ) ;
  point_no->setActiveCallback ( point_no_active_cb ) ;
  point_no->setDownCallback ( point_no_down_cb ) ;

  move_point = new puOneShot     ( 10, 10, 250, 50 ) ;
  move_point->setLegend         ( "Move Selected Point"  ) ;
  move_point->setCallback       ( move_point_cb ) ;
  move_point->makeReturnDefault (    TRUE      ) ;

  exit_button = new puButton     ( 260, 10, 360, 50 ) ;
  exit_button->setLegend         ( "Exit"  ) ;
  exit_button->setCallback       ( exit_cb ) ;
  exit_button->setStyle ( PUSTYLE_BOXED ) ;
  exit_button->setLegendPlace ( PUPLACE_CENTERED_LEFT ) ;

  input = new puLargeInput ( 440, 0, 200, 300, 1, 20, TRUE ) ;
  input->setChildStyle ( PUCLASS_ARROW, PUSTYLE_BOXED ) ;
  input->setChildBorderThickness ( PUCLASS_ARROW, 15 ) ;
  input->setText ( "This is a large input box\n"
                   "This is line two of a large input box\n"
                   "Line 3\n"
                   "Line 4\n"
                   "This is an extremely long line in the large input box, so long it passes the box width\n"
                   "Line 6\n"
                   "Line 7\n"
                   "Line 8\n"
                   "Line 9\n"
                   "Line 10\n"
                   "Line 11\n"
                   "Line 12\n"
                   "Line 13\n"
                   "Line 14\n"
                   "Line 15\n"
                   "Line 16\n"
                   "Line 17\n"
                   "Line 18\n"
                   "Line 19\n"
                   "Line 20\n"
                   "Line 21\n"
                   "Line 22\n"
                   "Line 23\n"
                   "Line 24\n"
                   "Line 25\n"
                   "Line 26\n"
                   "Line 27\n"
                   "Line 28\n"
                   "Line 29\n"
                   "Line 30\n" ) ;

  glutMainLoop () ;
  return 0 ;
}


