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

int          main_window ;
puMenuBar   *main_menu_bar ;
puButton    *hide_menu_button ;
puDialogBox *dialog_box ;
puText      *dialog_box_message ;
puOneShot   *dialog_box_ok_button ;
puText      *timer_text ;

int          slider_window ;
puSlider    *rspeedSlider ;
puSlider    *directSlider ;

int          save_window ;
puFilePicker *file_picker ;

fntTexFont *hel ;
fntTexFont *tim ;

/***********************************\
*                                   *
*  This is a generic tumbling cube  *
*                                   *
\***********************************/

GLfloat light_diffuse [] = {0.0, 1.0, 0.0, 1.0} ;  /* Red diffuse light. */
GLfloat light_position[] = {1.0, 1.0, 1.0, 0.0} ;  /* Infinite light location. */

GLfloat cube_n[6][3] =  /* Normals */
{
 {-1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {1.0, 0.0, 0.0},
 { 0.0,-1.0, 0.0}, {0.0, 0.0, 1.0}, {0.0, 0.0,-1.0}
} ;

GLint cube_i[6][4] =  /* Vertex indices */
{
  {0, 1, 2, 3}, {3, 2, 6, 7}, {7, 6, 5, 4},
  {4, 5, 1, 0}, {5, 6, 2, 1}, {7, 4, 0, 3}
} ;

GLfloat cube_v[8][3] =  /* Vertices */
{
  {-1.0,-1.0, 1.0}, {-1.0,-1.0,-1.0}, {-1.0, 1.0,-1.0}, {-1.0, 1.0, 1.0},
  { 1.0,-1.0, 1.0}, { 1.0,-1.0,-1.0}, { 1.0, 1.0,-1.0}, { 1.0, 1.0, 1.0}
} ;


static int firsttime;

void drawCube (void)
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
    glLightfv      ( GL_LIGHT0, GL_DIFFUSE , light_diffuse  ) ;
    glLightfv      ( GL_LIGHT0, GL_POSITION, light_position ) ;
    glEnable       ( GL_LIGHT0     ) ;
    glEnable       ( GL_LIGHTING   ) ;
    glEnable       ( GL_DEPTH_TEST ) ;
    glMatrixMode   ( GL_PROJECTION ) ;
    gluPerspective ( 40.0, 1.0, 1.0, 10.0 ) ;
    glMatrixMode   ( GL_MODELVIEW ) ;
    gluLookAt      ( 0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0 ) ;
    glTranslatef   ( 0.0, 0.0, -1.0 ) ;
    glRotatef      ( 60.0, 1.0, 0.0, 0.0 ) ;
  }

  glCullFace     ( GL_FRONT ) ;
  glEnable       ( GL_CULL_FACE ) ;
  //  glRotatef ( 1.0f, 0.0, 0.0, 1.0 ) ;  /* Tumble that cube! */

  glBegin ( GL_QUADS ) ;

  for ( int i = 0 ; i < 6 ; i++ )
  {
    glNormal3fv ( &cube_n[i][0] ) ;
    glVertex3fv ( cube_v[cube_i[i][0]] ) ; glVertex3fv ( cube_v[cube_i[i][1]] ) ;
    glVertex3fv ( cube_v[cube_i[i][2]] ) ; glVertex3fv ( cube_v[cube_i[i][3]] ) ;
  }

  glEnd () ;
}

/********************************\
*                                *
* End of cube renderer in OpenGL * 
*                                *
\********************************/


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

static void mousefn ( int button, int updown, int x, int y )
{
  puMouse ( button, updown, x, y ) ;
  glutPostRedisplay () ;
}

/**************************************\
*                                      *
* This function redisplays the PUI and *
* the tumbling cube, flips the double  *
* buffer and then asks GLUT to post a  *
* redisplay command - so we re-render  *
* at maximum rate.                     *
*                                      *
\**************************************/

static void displayfn (void)
{
  /* Clear the screen */

  glClearColor ( 0.4, 0.1, 0.1, 1.0 ) ;
  glClear      ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;

  /* Draw the tumbling cube */

  float val ; rspeedSlider->getValue ( &val ) ;
  float dir ; directSlider->getValue ( &dir ) ;

  glRotatef( 4*val, 3.0 * ( 2.0 * dir - 1.0 ) , 2.0 * ( 1.0 - 2.0 * dir ) , 1.0 );

  drawCube  () ;

  /* Update the 'timer' */

  time_t t = time ( NULL ) ;
  timer_text -> setLabel ( ctime ( & t ) ) ;

  /* Make PUI redraw */

  puDisplay () ;
  
  /* Off we go again... */

  glutSwapBuffers   () ;
  glutPostRedisplay () ;
}

static void sliderdisplayfn (void)
{
  /*
    Function to display only the slider window
    We must set the glut window first or we get an annoying flicker in the main window.
  */

  glutSetWindow ( slider_window ) ;

  /* Clear the screen */

  glClearColor ( 0.4, 0.1, 0.1, 1.0 ) ;
  glClear      ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;

  /* Make PUI redraw the slider window */

  puDisplay ( slider_window ) ;
  
  /* Off we go again... */

  glutSwapBuffers   () ;
  glutPostRedisplay () ;
}


static void savedisplayfn (void)
{
  /*
    Function to display only the save window
    We must set the glut window first or we get an annoying flicker in the main window.
  */

  glutSetWindow ( save_window ) ;

  /* Clear the screen */

  glClearColor ( 0.0, 0.0, 1.0, 1.0 ) ;
  glClear      ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;

  /* Make PUI redraw the save window */

  puDisplay ( save_window ) ;
  
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

void hide_menu_cb ( puObject *cb )
{
  if ( cb -> getValue () )
  {
    main_menu_bar -> reveal () ;
    hide_menu_button->setLegend ( "Hide Menu" ) ;
  }
  else
  {
    main_menu_bar -> hide   () ;
    hide_menu_button->setLegend ( "Show Menu" ) ;
  }
}


void go_away_cb ( puObject * )
{
  // Delete the dialog box when its 'OK' button is pressed.

  puDeleteObject( dialog_box ) ;
  dialog_box = NULL ;
}

void mk_dialog ( char *fmt, ... )
{
  static char txt [ PUSTRING_MAX ] ;

  va_list argptr ;
  va_start(argptr, fmt) ;
  vsprintf( txt, fmt, argptr ) ;
  va_end(argptr) ;

  dialog_box = new puDialogBox ( 150, 50 ) ;
  {
    new puFrame ( 0, 0, 400, 100 ) ;
    dialog_box_message   = new puText         ( 10, 70 ) ;
    dialog_box_message   -> setLabel          ( txt ) ;
    dialog_box_ok_button = new puOneShot      ( 180, 10, 240, 50 ) ;
    dialog_box_ok_button -> setLegend         ( "OK" ) ;
    dialog_box_ok_button -> makeReturnDefault ( TRUE ) ;
    dialog_box_ok_button -> setCallback       ( go_away_cb ) ;
  }
  dialog_box -> close  () ;
  dialog_box -> reveal () ;
}

void pick_cb ( puObject * )
{
  char* filename ;
  file_picker -> getValue ( &filename ) ;

  //NOTE: interface creation/deletion must be nested
  //the old interface must be deleted *before* a new one is created
  //otherwise the interface stack will be messed up
  puDeleteObject ( file_picker ) ;
  file_picker = 0 ;
  glutHideWindow () ;
  glutSetWindow ( main_window ) ;

  if ( filename[0] != 0 )
    mk_dialog ( "Saving File:\n%s", filename ) ;
  else
    mk_dialog ( "Save canceled" ) ;
}

void savereshapefn ( int w, int h )
{
  file_picker->setSize ( w, h ) ;
}

void save_cb ( puObject * )
{
  int w = 320, h = 270 ;
  glutSetWindow ( save_window ) ;
  glutShowWindow () ;
  glutReshapeWindow ( w, h ) ;
  glutPositionWindow ( ( 640 - w ) / 2, ( 480 - h ) / 2 ) ;

  file_picker = new puFilePicker ( 0, 0, w, h, ".", "Pick Place To Save" ) ;
  file_picker -> setCallback ( pick_cb ) ;
}

void ni_cb ( puObject * )
{
  mk_dialog ( "Warning:\nSorry, that function isn't implemented" ) ;
}

void about_cb ( puObject * )
{
  mk_dialog ( "About:\nThis is the PUI 'complex' program" ) ;
}

void help_cb ( puObject * )
{
  mk_dialog ( "Help:\nSorry, no help is available for this demo" ) ;
}

void edit_cb ( puObject * )
{
}

void exit_cb ( puObject * )
{
  fprintf ( stderr, "Exiting PUI demo program.\n" ) ;
  exit ( 1 ) ;
}

/* Menu bar entries: */

char      *file_submenu    [] = {  "Exit", "Close", "========", "Print", "========", "Save", "New", NULL } ;
puCallback file_submenu_cb [] = { exit_cb, exit_cb,       NULL, ni_cb  ,       NULL,  save_cb, ni_cb, NULL } ;

char      *edit_submenu    [] = { "Do nothing.", NULL } ;
puCallback edit_submenu_cb [] = {     edit_cb, NULL } ;

char      *help_submenu    [] = { "About...",  "Help", NULL } ;
puCallback help_submenu_cb [] = {   about_cb, help_cb, NULL } ;


void sliderCB( puObject *)
{
  glutPostRedisplay();
}

int main ( int argc, char **argv )
{
  firsttime = TRUE;

  glutInitWindowPosition( 100,   0 ) ;
  glutInitWindowSize    ( 640, 480 ) ;
  glutInit              ( &argc, argv ) ;
  glutInitDisplayMode   ( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH ) ;
  main_window = glutCreateWindow      ( "Complex PUI Application"  ) ;
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

  hel = new fntTexFont ;
  tim = new fntTexFont ;
  hel -> load ( "../fnt/data/helvetica_medium.txf" ) ;
  tim -> load ( "../fnt/data/times_medium.txf" ) ;
  puFont helvetica ( hel, 12 ) ;
  puFont times_medium ( tim, 15 ) ;
  puSetDefaultFonts        ( helvetica, times_medium ) ;
  puSetDefaultStyle        ( PUSTYLE_SMALL_SHADED ) ;
  puSetDefaultColourScheme ( 0.1, 0.8, 0.1, 1.0) ;

  timer_text = new puText ( 300, 10 ) ;
  timer_text -> setColour ( PUCOL_LABEL, 1.0, 1.0, 1.0 ) ;

  /* Make a button to hide the menu bar */

  hide_menu_button = new puButton     ( 10, 10, 150, 50 ) ;
  hide_menu_button->setValue          (    TRUE      ) ;
  hide_menu_button->setLegend         ( "Hide\nthe Menu"  ) ;
  hide_menu_button->setCallback       ( hide_menu_cb ) ;
  hide_menu_button->makeReturnDefault (    TRUE      ) ;

  /* Make the menu bar */

  main_menu_bar = new puMenuBar () ;
  {
    main_menu_bar -> add_submenu ( "File", file_submenu, file_submenu_cb ) ;
    main_menu_bar -> add_submenu ( "Edit", edit_submenu, edit_submenu_cb ) ;
    main_menu_bar -> add_submenu ( "Help", help_submenu, help_submenu_cb ) ;
  }
  main_menu_bar -> close () ; 

  slider_window = glutCreateWindow      ( "Slider Window"  ) ;
  glutPositionWindow    (  20, 100 ) ;
  glutReshapeWindow     ( 150,  200 ) ;
  glutDisplayFunc       ( sliderdisplayfn ) ;
  glutKeyboardFunc      ( keyfn     ) ;
  glutSpecialFunc       ( specialfn ) ;
  glutMouseFunc         ( mousefn   ) ;
  glutMotionFunc        ( motionfn  ) ;
  glutPassiveMotionFunc ( motionfn  ) ;
  glutIdleFunc          ( sliderdisplayfn ) ;

  //load the texture for the 2nd window
  hel -> load ( "../fnt/data/helvetica_medium.txf" ) ;
  tim -> load ( "../fnt/data/times_medium.txf" ) ;

  puGroup *slider_group = new puGroup ( 0, 0 ) ;  // Necessary so that "sliderdisplayfn" will draw all widgets

  rspeedSlider = new puSlider (10,30,150,TRUE);
  rspeedSlider->setDelta(0.1);
  rspeedSlider->setCBMode( PUSLIDER_DELTA );
  rspeedSlider->setCallback(sliderCB);
  rspeedSlider->setLabel ( "Speed" ) ;
  rspeedSlider->setLabelPlace ( PUPLACE_BELOW ) ;

  directSlider = new puSlider (80,30,150,TRUE);
  directSlider->setDelta(0.1);
  directSlider->setCBMode( PUSLIDER_DELTA );
  directSlider->setCallback(sliderCB);
  directSlider->setLabel ( "Direction" ) ;
  directSlider->setLabelPlace ( PUPLACE_BELOW ) ;

  slider_group -> close () ;

  save_window = glutCreateWindow ( "Saving" ) ;
  glutDisplayFunc       ( savedisplayfn ) ;
  glutKeyboardFunc      ( keyfn     ) ;
  glutSpecialFunc       ( specialfn ) ;
  glutMouseFunc         ( mousefn   ) ;
  glutMotionFunc        ( motionfn  ) ;
  glutPassiveMotionFunc ( motionfn  ) ;
  glutIdleFunc          ( savedisplayfn ) ;
  glutReshapeFunc       ( savereshapefn ) ;
  glutHideWindow        () ;

  //load the texture for the save window
  hel -> load ( "../fnt/data/helvetica_medium.txf" ) ;
  tim -> load ( "../fnt/data/times_medium.txf" ) ;

  glutMainLoop () ;
  return 0 ;
}


