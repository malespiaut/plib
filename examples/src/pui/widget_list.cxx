// Sample program to create a copy of each widget in PUI

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

int button_box_window ;
puButtonBox *button_box ;

int frame_window ;
puFrame *frame ;

int text_window ;
puText *text ;

int button_window ;
puButton *button ;

int one_shot_window ;
puOneShot *one_shot ;

int popup_menu_window ;
puPopupMenu *popup_menu ;

int menu_bar_window ;
puMenuBar *menu_bar ;

int input_window ;
puInput *input ;

int slider_window ;
puSlider *slider ;

int arrow_button_window ;
puArrowButton *arrow_button ;

int dial_window ;
puDial *dial ;

int file_picker_window ;
puFilePicker *file_picker ;

int bislider_window ;
puBiSlider *bislider ;

int trislider_window ;
puTriSlider *trislider ;

int vertical_menu_window ;
puVerticalMenu *vertical_menu ;

int dialog_box_window ;
puDialogBox *dialog_box ;

int large_input_window ;
puLargeInput *large_input ;

fntTexFont *tim ;



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

static void mousefn ( int btn, int updown, int x, int y )
{
  puMouse ( btn, updown, x, y ) ;
  glutPostRedisplay () ;
}

/**************************************\
*                                      *
* This function redisplays the PUI     *
*                                      *
\**************************************/

static void displayfn (void)
{
  /* Clear the screen */

  glClearColor ( 0.4, 1.0, 1.0, 1.0 ) ;
  glClear      ( GL_COLOR_BUFFER_BIT ) ;

  /* Make PUI redraw */

  puDisplay () ;

  /* Swap buffers */

  glutSwapBuffers   () ;
}


/***********************************\
*                                   *
* Here are the PUI widget callback  *
* functions.                        *
*                                   *
\***********************************/

void exit_cb ( puObject * )
{
  fprintf ( stderr, "Exiting PUI demo program.\n" ) ;
  exit ( 1 ) ;
}

/* Menu bar entries: */

char      *file_submenu    [] = {  "Exit", "Close", "========", "Print", "========", "Save", "New", NULL } ;
puCallback file_submenu_cb [] = { exit_cb, exit_cb,       NULL, NULL  ,       NULL,  NULL, NULL, NULL } ;

char      *edit_submenu    [] = { "Do nothing.", NULL } ;
puCallback edit_submenu_cb [] = {     NULL, NULL } ;

char      *help_submenu    [] = { "About...",  "Help", NULL } ;
puCallback help_submenu_cb [] = {   NULL, NULL, NULL } ;


int main ( int argc, char **argv )
{
  glutInitWindowPosition( 100,   0 ) ;
  glutInitWindowSize    ( 640, 480 ) ;
  glutInit              ( &argc, argv ) ;
  glutInitDisplayMode   ( GLUT_RGB | GLUT_DOUBLE ) ;
  main_window = glutCreateWindow      ( "PUI Widget List"  ) ;
  glutDisplayFunc       ( displayfn ) ;
  glutKeyboardFunc      ( keyfn     ) ;
  glutSpecialFunc       ( specialfn ) ;
  glutMouseFunc         ( mousefn   ) ;
  glutMotionFunc        ( motionfn  ) ;

  puInit () ;

#ifdef VOODOO
  puShowCursor () ;
#endif

  tim = new fntTexFont ;
  tim -> load ( "../fnt/data/times_bold.txf" ) ;
  puFont times_medium ( tim, 12 ) ;
  puSetDefaultFonts        ( times_medium, times_medium ) ;
  puSetDefaultStyle        ( PUSTYLE_SMALL_SHADED ) ;
  puSetDefaultColourScheme ( 0.8, 0.8, 0.8, 1.0) ;

  /* Set up the secondary windows and their widgets */

  button_box_window = glutCreateWindow      ( "Button Box Window"  ) ;
  glutPositionWindow    ( 200, 100 ) ;
  glutReshapeWindow     ( 180,  100 ) ;
  glutDisplayFunc       ( displayfn ) ;
  glutKeyboardFunc      ( keyfn     ) ;
  glutSpecialFunc       ( specialfn ) ;
  glutMouseFunc         ( mousefn   ) ;
  glutMotionFunc        ( motionfn  ) ;

  tim -> load ( "../fnt/data/times_bold.txf" ) ;

  char *button_box_entries [] = { "First Entry", "Second Entry", "Third Entry", NULL } ;
  button_box = new puButtonBox ( 10, 10, 130, 80, button_box_entries, TRUE ) ;
  button_box->setLabel ( "Label" ) ;
  button_box->setLegend ( "Legend" ) ;

  frame_window = glutCreateWindow      ( "Frame Window"  ) ;
  glutPositionWindow    ( 200, 100 ) ;
  glutReshapeWindow     ( 180,  100 ) ;
  glutDisplayFunc       ( displayfn ) ;
  glutKeyboardFunc      ( keyfn     ) ;
  glutSpecialFunc       ( specialfn ) ;
  glutMouseFunc         ( mousefn   ) ;
  glutMotionFunc        ( motionfn  ) ;

  tim -> load ( "../fnt/data/times_bold.txf" ) ;

  frame = new puFrame ( 10, 10, 130, 80 ) ;
  frame->setLabel ( "Label" ) ;
  frame->setLegend ( "Legend" ) ;

  text_window = glutCreateWindow      ( "Text Window"  ) ;
  glutPositionWindow    ( 200, 100 ) ;
  glutReshapeWindow     ( 180,  100 ) ;
  glutDisplayFunc       ( displayfn ) ;
  glutKeyboardFunc      ( keyfn     ) ;
  glutSpecialFunc       ( specialfn ) ;
  glutMouseFunc         ( mousefn   ) ;
  glutMotionFunc        ( motionfn  ) ;

  tim -> load ( "../fnt/data/times_bold.txf" ) ;

  text = new puText ( 10, 10 ) ;
  text->setLabel ( "Label" ) ;
  text->setLegend ( "Legend" ) ;

  button_window = glutCreateWindow      ( "Button Window"  ) ;
  glutPositionWindow    ( 200, 100 ) ;
  glutReshapeWindow     ( 180,  100 ) ;
  glutDisplayFunc       ( displayfn ) ;
  glutKeyboardFunc      ( keyfn     ) ;
  glutSpecialFunc       ( specialfn ) ;
  glutMouseFunc         ( mousefn   ) ;
  glutMotionFunc        ( motionfn  ) ;

  tim -> load ( "../fnt/data/times_bold.txf" ) ;

  button = new puButton ( 10, 10, 90, 30 ) ;
  button->setLabel ( "Label" ) ;
  button->setLegend ( "Legend" ) ;
  button->setValue ( 1 ) ;
  puText *button_text = new puText ( 10, 40 ) ;
  button_text->setLabel ( "(Button pressed in)" ) ;

  one_shot_window = glutCreateWindow      ( "One Shot Window"  ) ;
  glutPositionWindow    ( 200, 100 ) ;
  glutReshapeWindow     ( 180,  100 ) ;
  glutDisplayFunc       ( displayfn ) ;
  glutKeyboardFunc      ( keyfn     ) ;
  glutSpecialFunc       ( specialfn ) ;
  glutMouseFunc         ( mousefn   ) ;
  glutMotionFunc        ( motionfn  ) ;

  tim -> load ( "../fnt/data/times_bold.txf" ) ;

  one_shot = new puOneShot ( 10, 10, 90, 30 ) ;
  one_shot->setLabel ( "Label" ) ;
  one_shot->setLegend ( "Legend" ) ;

  popup_menu_window = glutCreateWindow      ( "Popup Menu Window"  ) ;
  glutPositionWindow    ( 200, 100 ) ;
  glutReshapeWindow     ( 80,  100 ) ;
  glutDisplayFunc       ( displayfn ) ;
  glutKeyboardFunc      ( keyfn     ) ;
  glutSpecialFunc       ( specialfn ) ;
  glutMouseFunc         ( mousefn   ) ;
  glutMotionFunc        ( motionfn  ) ;

  tim -> load ( "../fnt/data/times_bold.txf" ) ;

  popup_menu = new puPopupMenu ( 10, 10 ) ;
  popup_menu->setLabel ( "Label" ) ;
  popup_menu->setLegend ( "Legend" ) ;
  popup_menu->add_item ( "Cut", NULL ) ;
  popup_menu->add_item ( "Copy", NULL ) ;
  popup_menu->add_item ( "Paste", NULL ) ;
  popup_menu->add_item ( "Delete", NULL ) ;
  popup_menu->close () ;
  popup_menu->reveal () ;

  menu_bar_window = glutCreateWindow      ( "Menu Bar Window"  ) ;
  glutPositionWindow    ( 200, 100 ) ;
  glutReshapeWindow     ( 180,  200 ) ;
  glutDisplayFunc       ( displayfn ) ;
  glutKeyboardFunc      ( keyfn     ) ;
  glutSpecialFunc       ( specialfn ) ;
  glutMouseFunc         ( mousefn   ) ;
  glutMotionFunc        ( motionfn  ) ;

  tim -> load ( "../fnt/data/times_bold.txf" ) ;

  menu_bar = new puMenuBar () ;
  {
    menu_bar -> add_submenu ( "File", file_submenu, file_submenu_cb ) ;
    menu_bar -> add_submenu ( "Edit", edit_submenu, edit_submenu_cb ) ;
    menu_bar -> add_submenu ( "Help", help_submenu, help_submenu_cb ) ;
  }
  menu_bar -> close () ; 
  menu_bar->setLabel ( "Label" ) ;
  menu_bar->setLegend ( "Legend" ) ;

  input_window = glutCreateWindow      ( "Input Window"  ) ;
  glutPositionWindow    ( 200, 100 ) ;
  glutReshapeWindow     ( 180,  100 ) ;
  glutDisplayFunc       ( displayfn ) ;
  glutKeyboardFunc      ( keyfn     ) ;
  glutSpecialFunc       ( specialfn ) ;
  glutMouseFunc         ( mousefn   ) ;
  glutMotionFunc        ( motionfn  ) ;

  tim -> load ( "../fnt/data/times_bold.txf" ) ;

  input = new puInput ( 10, 10, 90, 30 ) ;
  input->setLabel ( "Label" ) ;
  input->setLegend ( "Legend" ) ;

  slider_window = glutCreateWindow      ( "Slider Window"  ) ;
  glutPositionWindow    ( 200, 100 ) ;
  glutReshapeWindow     ( 180,  100 ) ;
  glutDisplayFunc       ( displayfn ) ;
  glutKeyboardFunc      ( keyfn     ) ;
  glutSpecialFunc       ( specialfn ) ;
  glutMouseFunc         ( mousefn   ) ;
  glutMotionFunc        ( motionfn  ) ;

  tim -> load ( "../fnt/data/times_bold.txf" ) ;

  slider = new puSlider ( 10, 10, 120, 0 ) ;
  slider->setLabel ( "Label" ) ;
  slider->setLegend ( "Legend" ) ;

  arrow_button_window = glutCreateWindow      ( "Arrow Button Window"  ) ;
  glutPositionWindow    ( 200, 100 ) ;
  glutReshapeWindow     ( 180,  100 ) ;
  glutDisplayFunc       ( displayfn ) ;
  glutKeyboardFunc      ( keyfn     ) ;
  glutSpecialFunc       ( specialfn ) ;
  glutMouseFunc         ( mousefn   ) ;
  glutMotionFunc        ( motionfn  ) ;

  tim -> load ( "../fnt/data/times_bold.txf" ) ;

  arrow_button = new puArrowButton ( 10, 10, 50, 50, PUARROW_RIGHT ) ;
  arrow_button->setLabel ( "Label" ) ;
  arrow_button->setLegend ( "Legend" ) ;

  dial_window = glutCreateWindow      ( "Dial Window"  ) ;
  glutPositionWindow    ( 200, 100 ) ;
  glutReshapeWindow     ( 180,  100 ) ;
  glutDisplayFunc       ( displayfn ) ;
  glutKeyboardFunc      ( keyfn     ) ;
  glutSpecialFunc       ( specialfn ) ;
  glutMouseFunc         ( mousefn   ) ;
  glutMotionFunc        ( motionfn  ) ;

  tim -> load ( "../fnt/data/times_bold.txf" ) ;

  dial = new puDial ( 10, 10, 60 ) ;
  dial->setLabel ( "Label" ) ;
  dial->setLegend ( "Legend" ) ;

  file_picker_window = glutCreateWindow      ( "File Picker Window"  ) ;
  glutPositionWindow    ( 200, 100 ) ;
  glutReshapeWindow     ( 300,  200 ) ;
  glutDisplayFunc       ( displayfn ) ;
  glutKeyboardFunc      ( keyfn     ) ;
  glutSpecialFunc       ( specialfn ) ;
  glutMouseFunc         ( mousefn   ) ;
  glutMotionFunc        ( motionfn  ) ;

  tim -> load ( "../fnt/data/times_bold.txf" ) ;

  file_picker = new puFilePicker ( 10, 10, 280, 180, "." ) ;
  file_picker->setLabel ( "Label" ) ;
  file_picker->setLegend ( "Legend" ) ;

  bislider_window = glutCreateWindow      ( "BiSlider Window"  ) ;
  glutPositionWindow    ( 200, 100 ) ;
  glutReshapeWindow     ( 180,  100 ) ;
  glutDisplayFunc       ( displayfn ) ;
  glutKeyboardFunc      ( keyfn     ) ;
  glutSpecialFunc       ( specialfn ) ;
  glutMouseFunc         ( mousefn   ) ;
  glutMotionFunc        ( motionfn  ) ;

  tim -> load ( "../fnt/data/times_bold.txf" ) ;

  bislider = new puBiSlider ( 10, 10, 120, 0 ) ;
  bislider->setLabel ( "Label" ) ;
  bislider->setLegend ( "Legend" ) ;
  bislider->setMaxValue ( 20 ) ;
  bislider->setCurrentMin ( 4 ) ;
  bislider->setCurrentMax ( 15 ) ;

  trislider_window = glutCreateWindow      ( "TriSlider Window"  ) ;
  glutPositionWindow    ( 200, 100 ) ;
  glutReshapeWindow     ( 100,  200 ) ;
  glutDisplayFunc       ( displayfn ) ;
  glutKeyboardFunc      ( keyfn     ) ;
  glutSpecialFunc       ( specialfn ) ;
  glutMouseFunc         ( mousefn   ) ;
  glutMotionFunc        ( motionfn  ) ;

  tim -> load ( "../fnt/data/times_bold.txf" ) ;

  trislider = new puTriSlider ( 20, 10, 180, 1 ) ;
  trislider->setLabel ( "Label" ) ;
  trislider->setLegend ( "Legend" ) ;
  trislider->setMaxValue ( 20 ) ;
  trislider->setCurrentMin ( 4 ) ;
  trislider->setCurrentMax ( 15 ) ;
  trislider->setValue ( 12 ) ;

  vertical_menu_window = glutCreateWindow      ( "Vertical Menu Window"  ) ;
  glutPositionWindow    ( 200, 100 ) ;
  glutReshapeWindow     ( 180,  200 ) ;
  glutDisplayFunc       ( displayfn ) ;
  glutKeyboardFunc      ( keyfn     ) ;
  glutSpecialFunc       ( specialfn ) ;
  glutMouseFunc         ( mousefn   ) ;
  glutMotionFunc        ( motionfn  ) ;

  tim -> load ( "../fnt/data/times_bold.txf" ) ;

  vertical_menu = new puVerticalMenu () ;
  {
    vertical_menu -> add_submenu ( "File", file_submenu, file_submenu_cb ) ;
    vertical_menu -> add_submenu ( "Edit", edit_submenu, edit_submenu_cb ) ;
    vertical_menu -> add_submenu ( "Help", help_submenu, help_submenu_cb ) ;
  }
  vertical_menu -> close () ; 
  vertical_menu->setLabel ( "Label" ) ;
  vertical_menu->setLegend ( "Legend" ) ;

  dialog_box_window = glutCreateWindow      ( "Dialog Box Window"  ) ;
  glutPositionWindow    ( 200, 100 ) ;
  glutReshapeWindow     ( 180,  100 ) ;
  glutDisplayFunc       ( displayfn ) ;
  glutKeyboardFunc      ( keyfn     ) ;
  glutSpecialFunc       ( specialfn ) ;
  glutMouseFunc         ( mousefn   ) ;
  glutMotionFunc        ( motionfn  ) ;

  tim -> load ( "../fnt/data/times_bold.txf" ) ;

  dialog_box = new puDialogBox ( 10, 10 ) ;
  dialog_box->setLabel ( "Label" ) ;
  dialog_box->setLegend ( "Legend" ) ;
  new puOneShot ( 20, 20, "OK" ) ;
  new puOneShot ( 100, 20, "Cancel" ) ;
  puText *dialog_text = new puText ( 20, 60 ) ;
  dialog_text->setLabel ( "Dialog Box Text\nand Widgets" ) ;
  dialog_box->close () ;
  dialog_box->reveal () ;

  large_input_window = glutCreateWindow      ( "Large Input Window"  ) ;
  glutPositionWindow    ( 200, 100 ) ;
  glutReshapeWindow     ( 300,  200 ) ;
  glutDisplayFunc       ( displayfn ) ;
  glutKeyboardFunc      ( keyfn     ) ;
  glutSpecialFunc       ( specialfn ) ;
  glutMouseFunc         ( mousefn   ) ;
  glutMotionFunc        ( motionfn  ) ;

  tim -> load ( "../fnt/data/times_bold.txf" ) ;

  large_input = new puLargeInput ( 10, 10, 240, 180, 2, 20 ) ;
  large_input->setLabel ( "Label" ) ;
  large_input->setLegend ( "Legend" ) ;
  large_input->setText ( "This is text in the Large Input widget.\n"
                         "This is a second line of text" ) ;

  glutMainLoop () ;
  return 0 ;
}


