/*
     PLIB - A Suite of Portable Game Libraries
     Copyright (C) 2001  Steve Baker

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

     For further information visit http://plib.sourceforge.net

     $Id$
*/


#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef WIN32
#  include <windows.h>
#else
#  include <unistd.h>
#endif
#include <math.h>

#ifdef FREEGLUT_IS_PRESENT
#  include <GL/freeglut.h>
#else
#  ifdef __APPLE__
#    include <GLUT/glut.h>
#  else
#    include <GL/glut.h>
#  endif
#endif

#include <plib/pu.h>

//#define VOODOO 1

/***********************************\
*                                   *
* These are the PUI widget pointers *
*                                   *
\***********************************/

static int          main_window ;
static puMenuBar   *main_menu_bar ;
static puButton    *hide_menu_button ;
static puDialogBox *dialog_box ;
static puText      *dialog_box_message ;
static puOneShot   *dialog_box_ok_button ;
static puText      *timer_text ;
static puGroup *mygroup ;
static puInput *input1 ;
static puInput *input2 ;

static int          slider_window ;
static puSlider    *rspeedSlider ;
static puSlider    *directSlider ;

static int          save_window ;
static puFileSelector *file_selector ;

static int          coordinate_window ;
static puBiSlider  *x_coordinate ;
static puTriSlider *y_coordinate ;

//static fntTexFont *hel ;
//static fntTexFont *tim ;

/***********************************\
*                                   *
*  This is a generic tumbling cube  *
*                                   *
\***********************************/

static const GLfloat light_diffuse [] = {0.0, 1.0, 0.0, 1.0} ;  /* Red diffuse light. */
static const GLfloat light_position[] = {1.0, 1.0, 1.0, 0.0} ;  /* Infinite light location. */

static const GLfloat cube_n[6][3] =  /* Normals */
{
 {-1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {1.0, 0.0, 0.0},
 { 0.0,-1.0, 0.0}, {0.0, 0.0,-1.0}, {0.0, 0.0, 1.0}
} ;

static const GLint cube_i[6][4] =  /* Vertex indices */
{
  {0, 1, 2, 3}, {3, 2, 6, 7}, {7, 6, 5, 4},
  {4, 5, 1, 0}, {5, 6, 2, 1}, {7, 4, 0, 3}
} ;

static const GLfloat cube_v[8][3] =  /* Vertices */
{
  {-1.0,-1.0, 1.0}, {-1.0,-1.0,-1.0}, {-1.0, 1.0,-1.0}, {-1.0, 1.0, 1.0},
  { 1.0,-1.0, 1.0}, { 1.0,-1.0,-1.0}, { 1.0, 1.0,-1.0}, { 1.0, 1.0, 1.0}
} ;


static int firsttime;

static void drawCube (void)
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
    for ( int j = 0; j < 4; j++ )
    {
      float xmult =(float)(x_coordinate->getCurrentMax () - x_coordinate->getCurrentMin ()) /
                   (float)(x_coordinate->getMaxValue () - x_coordinate->getMinValue ()) ;
      float ymult =(float)(y_coordinate->getCurrentMax () - y_coordinate->getCurrentMin ()) /
                   (float)(y_coordinate->getMaxValue () - y_coordinate->getMinValue ()) ;

      float x = cube_v[cube_i[i][j]][0] * xmult ;
      float y = cube_v[cube_i[i][j]][1] * ymult ;
      float z = cube_v[cube_i[i][j]][2] ;
      glVertex3f ( x, y, z ) ;
    }
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

  glClearColor ( 0.1f, 0.3f, 0.5f, 1.0f ) ;
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

  glClearColor ( 0.1f, 0.3f, 0.5f, 1.0f ) ;
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

  glClearColor ( 0.1f, 0.1f, 5.0f, 1.0f ) ;
  glClear      ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;

  /* Make PUI redraw the save window */

  puDisplay ( save_window ) ;
  
  /* Off we go again... */

  glutSwapBuffers   () ;
  glutPostRedisplay () ;
}


static void coorddisplayfn (void)
{
  /*
    Function to display only the slider window
    We must set the glut window first or we get an annoying flicker in the main window.
  */

  glutSetWindow ( coordinate_window ) ;

  /* Clear the screen */

  glClearColor ( 0.1f, 0.3f, 0.5f, 1.0f ) ;
  glClear      ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;

  /* Make PUI redraw the slider window */

  puDisplay ( coordinate_window ) ;
  
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

static void hide_menu_cb ( puObject *cb )
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


static void go_away_cb ( puObject * )
{
  // Delete the dialog box when its 'OK' button is pressed.

  puDeleteObject( dialog_box ) ;
  dialog_box = NULL ;
}

static void mk_dialog ( char *fmt, ... )
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

static void pick_cb ( puObject * )
{
  char* filename ;
  file_selector -> getValue ( &filename ) ;

  //NOTE: interface creation/deletion must be nested
  //the old interface must be deleted *before* a new one is created
  //otherwise the interface stack will be messed up
  puDeleteObject ( file_selector ) ;
  file_selector = 0 ;
  glutHideWindow () ;
  glutSetWindow ( main_window ) ;

  if ( filename[0] != 0 )
    mk_dialog ( "Saving File:\n%s", filename ) ;
  else
    mk_dialog ( "Save canceled" ) ;
}

static void savereshapefn ( int w, int h )
{
  file_selector->setSize ( w, h ) ;
}

static void coordreshapefn ( int w, int h )
{
  x_coordinate->setSize ( 20, h-40 ) ;
  y_coordinate->setSize ( 20, h-40 ) ;
  y_coordinate->setPosition ( w/2 - 5, 30 ) ;
}

static void save_cb ( puObject * )
{
  static int save_count = 0 ;  // Number of buttons in file picker
  int w = 320, h = 270 ;
  glutSetWindow ( save_window ) ;
  glutShowWindow () ;
  glutReshapeWindow ( w, h ) ;
  glutPositionWindow ( ( 640 - w ) / 2, ( 480 - h ) / 2 ) ;
  if ( ++save_count > 2 ) save_count = 0 ;

  file_selector = new puFileSelector ( 0, 0, w, h, save_count, ".", "Pick Place To Save" ) ;
  file_selector -> setCallback ( pick_cb ) ;
  file_selector->setChildStyle ( PUCLASS_ONESHOT, PUSTYLE_BOXED ) ;
  file_selector->setChildBorderThickness ( PUCLASS_ONESHOT, 5 ) ;
  file_selector->setChildColour ( PUCLASS_SLIDER, 0, 0.5, 0.5, 0.5 ) ;
}

static void ni_cb ( puObject * )
{
  mk_dialog ( "Warning:\nSorry, that function isn't implemented" ) ;
}

static void about_cb ( puObject * )
{
  mk_dialog ( "About:\nThis is the PUI 'complex' program" ) ;
}

static void help_cb ( puObject * )
{
  mk_dialog ( "Help:\nSorry, no help is available for this demo" ) ;
}

static void edit_cb ( puObject * )
{
}

static void exit_cb ( puObject * )
{
  fprintf ( stderr, "Exiting PUI demo program.\n" ) ;
  exit ( 1 ) ;
}

/* Menu bar entries: */

static char      *file_submenu    [] = {  "Exit", "Close", "------------", "Print - this is a long entry", "------------", "Save", "New", NULL } ;
static puCallback file_submenu_cb [] = { exit_cb, exit_cb, NULL, ni_cb, NULL, save_cb, ni_cb, NULL } ;

static char      *edit_submenu    [] = { "Do nothing", NULL } ;
static puCallback edit_submenu_cb [] = {     edit_cb, NULL } ;

static char      *help_submenu    [] = { "About...",  "Help", NULL } ;
static puCallback help_submenu_cb [] = {   about_cb, help_cb, NULL } ;


static void sliderCB( puObject *)
{
  glutPostRedisplay();
}

static void coordCB( puObject *)
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

#ifdef VOODOO
  glutPassiveMotionFunc ( motionfn  ) ;
#endif

  glutIdleFunc          ( displayfn ) ;

  puInit () ;

#ifdef VOODOO
  puShowCursor () ;
#endif

  //hel = new fntTexFont ;
  //tim = new fntTexFont ;
  //hel -> load ( "../fnt/data/old/helvetica_medium.txf" ) ;
  //tim -> load ( "../fnt/data/old/times_medium.txf" ) ;
  //puFont helvetica ( hel, 15 ) ;
  //puFont times_medium ( tim, 13 ) ;
  //puSetDefaultFonts        ( helvetica, times_medium ) ;
  puSetDefaultStyle        ( PUSTYLE_SMALL_SHADED ) ;
  puSetDefaultColourScheme ( 0.3f, 0.4f, 0.6f, 1.0f) ;

  timer_text = new puText ( 300, 10 ) ;
  timer_text -> setColour ( PUCOL_LABEL, 1.0, 1.0, 1.0 ) ;

  /* Make a button to hide the menu bar */

  hide_menu_button = new puButton     ( 10, 10, 150, 50 ) ;
  hide_menu_button->setValue          (    TRUE      ) ;
  hide_menu_button->setLegend         ( "Hide the Menu"  ) ;
  hide_menu_button->setCallback       ( hide_menu_cb ) ;
  // hide_menu_button->setLegendPlace    ( PUPLACE_CENTER ) ;
  hide_menu_button->makeReturnDefault (    TRUE      ) ;

  /* Make the menu bar */

  main_menu_bar = new puMenuBar () ;
  {
    main_menu_bar -> add_submenu ( "File", file_submenu, file_submenu_cb ) ;
    main_menu_bar -> add_submenu ( "Edit", edit_submenu, edit_submenu_cb ) ;
    main_menu_bar -> add_submenu ( "Help", help_submenu, help_submenu_cb ) ;
  }
  main_menu_bar -> close () ; 

  mygroup = new puGroup ( 40, 40 ) ;
  input1 = new puInput ( 10, 70, 90, 90 ) ;
  input2 = new puInput ( 10, 40, 90, 60 ) ;
  mygroup->close () ;

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
  //hel -> load ( "../fnt/data/old/helvetica_medium.txf" ) ;
  //tim -> load ( "../fnt/data/old/times_medium.txf" ) ;

  puGroup *slider_group = new puGroup ( 0, 0 ) ;  // Necessary so that "sliderdisplayfn" will draw all widgets

  rspeedSlider = new puSlider (10,30,150,TRUE);
  rspeedSlider->setDelta(0.1f);
  rspeedSlider->setCBMode( PUSLIDER_DELTA );
  rspeedSlider->setCallback(sliderCB);
  rspeedSlider->setLabel ( "Speed" ) ;
  rspeedSlider->setLabelPlace ( PUPLACE_BOTTOM_LEFT ) ;
  rspeedSlider->setValue ( 0.8f ) ;

  directSlider = new puSlider (80,30,150,TRUE);
  directSlider->setDelta(0.1f);
  directSlider->setCBMode( PUSLIDER_DELTA );
  directSlider->setCallback(sliderCB);
  directSlider->setLabel ( "Direction" ) ;
  directSlider->setLabelPlace ( PUPLACE_BOTTOM_LEFT ) ;

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
  //hel -> load ( "../fnt/data/old/helvetica_medium.txf" ) ;
  //tim -> load ( "../fnt/data/old/times_medium.txf" ) ;

  // Coordinate Selection Window

  coordinate_window = glutCreateWindow      ( "Coordinate Window"  ) ;
  glutPositionWindow    ( 420, 100 ) ;
  glutReshapeWindow     ( 250,  400 ) ;
  glutDisplayFunc       ( coorddisplayfn ) ;
  glutKeyboardFunc      ( keyfn     ) ;
  glutSpecialFunc       ( specialfn ) ;
  glutMouseFunc         ( mousefn   ) ;
  glutMotionFunc        ( motionfn  ) ;
  glutPassiveMotionFunc ( motionfn  ) ;
  glutIdleFunc          ( coorddisplayfn ) ;
  glutReshapeFunc       ( coordreshapefn ) ;

  //load the texture for the 2nd window
  //hel -> load ( "../fnt/data/old/helvetica_medium.txf" ) ;
  //tim -> load ( "../fnt/data/old/times_medium.txf" ) ;

  puGroup *coordinate_group = new puGroup ( 0, 0 ) ;  // Necessary so that "groupdisplayfn" will draw all widgets

  x_coordinate = new puBiSlider ( 50,10,350,TRUE ) ;
  x_coordinate->setMinValue ( -20 ) ;
  x_coordinate->setMaxValue (  20 ) ;
  x_coordinate->setCurrentMin ( -20 ) ;
  x_coordinate->setCurrentMax (  20 ) ;
//  x_coordinate->setDelta(0.1);
//  x_coordinate->setCBMode( PUSLIDER_DELTA );
  x_coordinate->setCallback(coordCB);
  x_coordinate->setLabel ( "X-coords" ) ;
  x_coordinate->setLabelPlace ( PUPLACE_TOP_CENTERED ) ;

  y_coordinate = new puTriSlider (200,30,350,TRUE);
  y_coordinate->setMinValue ( -20 ) ;
  y_coordinate->setMaxValue (  20 ) ;
  y_coordinate->setCurrentMin ( -20 ) ;
  y_coordinate->setCurrentMax (  20 ) ;
  y_coordinate->setFreezeEnds ( FALSE ) ;
  y_coordinate->setValue    (   0 ) ;
//  y_coordinate->setDelta(0.1);
//  y_coordinate->setCBMode( PUSLIDER_DELTA );
  y_coordinate->setCallback(coordCB);
  y_coordinate->setLabel ( "Y-coords" ) ;
  y_coordinate->setLabelPlace ( PUPLACE_BOTTOM_CENTERED ) ;

  coordinate_group -> close () ;

//  hel -> load ( "../fnt/data/old/helvetica_medium.txf" ) ;
//  tim -> load ( "../fnt/data/old/times_medium.txf" ) ;

  glutMainLoop () ;
  return 0 ;
}


