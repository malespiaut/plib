/*
     P-GUIDE - PUI-based Graphical User Interface Designer
     Copyright (C) 2002  John F. Fay

     This program is free software; you can redistribute it and/or
     modify it under the terms of the GNU General Public License as
     published by the Free Software Foundation; either version 2 of
     the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
*/

// Widget Window

#include <stdio.h>

#include <plib/pu.h>

// Widget Window Data

bool selected_object_sticky = false ;
int selected_object_type ;
char *selected_type_string ;
puButton *active_button = (puButton *)NULL ;

// Widget window parameters
int widget_window = 0 ;  // Window handle

// GLUT Widget Window Callbacks

static void widget_window_specialfn ( int key, int, int )
{
  glutPostRedisplay () ;
}

static void widget_window_keyfn ( unsigned char key, int, int )
{
  if ( selected_object_type )
  {
    if ( key == 27 )  // Escape key, deactivate the selected button
    {
      selected_object_type = 0 ;
      active_button->setValue ( 0 ) ;
    }
  }

  glutPostRedisplay () ;
}

static void widget_window_motionfn ( int x, int y )
{
  puMouse ( x, y ) ;

  glutPostRedisplay () ;
}

static void widget_window_mousefn ( int button, int updown, int x, int y )
{
  if ( ! puMouse ( button, updown, x, y ) )
  {
    // PUI didn't absorb the mouse click; deactivate the selected object type
    if ( selected_object_type )
    {
      selected_object_type = 0 ;
      active_button->setValue ( 0 ) ;
    }
  }

  glutPostRedisplay () ;
}


static void widget_window_displayfn ( void )
{
  glutSetWindow ( widget_window ) ;

  /* Clear the screen */

  glClearColor ( 0.1, 0.1, 0.1, 1.0 ) ;
  glClear      ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;

  puDisplay () ;

  /* Update GLUT */

  glutSwapBuffers   () ;
  glutPostRedisplay () ;
}

// PUI Callback Functions:

void frame_cb ( puObject *ob )
{
  selected_object_sticky = ( glutGetModifiers () & GLUT_ACTIVE_CTRL ) ? 1 : 0 ;
  selected_object_type = PUCLASS_FRAME ;
  strcpy ( selected_type_string, ob->getLegend () ) ;
  active_button = (puButton *)ob ;
}

void text_cb ( puObject *ob )
{
  selected_object_sticky = ( glutGetModifiers () & GLUT_ACTIVE_CTRL ) ? 1 : 0 ;
  selected_object_type = PUCLASS_TEXT ;
  strcpy ( selected_type_string, ob->getLegend () ) ;
  active_button = (puButton *)ob ;
}

void menu_bar_cb ( puObject *ob )
{
  selected_object_sticky = ( glutGetModifiers () & GLUT_ACTIVE_CTRL ) ? 1 : 0 ;
  selected_object_type = PUCLASS_MENUBAR ;
  strcpy ( selected_type_string, ob->getLegend () ) ;
  active_button = (puButton *)ob ;
}

void vertical_menu_cb ( puObject *ob )
{
  selected_object_sticky = ( glutGetModifiers () & GLUT_ACTIVE_CTRL ) ? 1 : 0 ;
  selected_object_type = PUCLASS_VERTMENU ;
  strcpy ( selected_type_string, ob->getLegend () ) ;
  active_button = (puButton *)ob ;
}

void input_cb ( puObject *ob )
{
  selected_object_sticky = ( glutGetModifiers () & GLUT_ACTIVE_CTRL ) ? 1 : 0 ;
  selected_object_type = PUCLASS_INPUT ;
  strcpy ( selected_type_string, ob->getLegend () ) ;
  active_button = (puButton *)ob ;
}

void large_input_cb ( puObject *ob )
{
  selected_object_sticky = ( glutGetModifiers () & GLUT_ACTIVE_CTRL ) ? 1 : 0 ;
  selected_object_type = PUCLASS_LARGEINPUT ;
  strcpy ( selected_type_string, ob->getLegend () ) ;
  active_button = (puButton *)ob ;
}

void select_box_cb ( puObject *ob )
{
  selected_object_sticky = ( glutGetModifiers () & GLUT_ACTIVE_CTRL ) ? 1 : 0 ;
  selected_object_type = PUCLASS_SELECTBOX ;
  strcpy ( selected_type_string, ob->getLegend () ) ;
  active_button = (puButton *)ob ;
}

void file_selector_cb ( puObject *ob )
{
  selected_object_sticky = ( glutGetModifiers () & GLUT_ACTIVE_CTRL ) ? 1 : 0 ;
  selected_object_type = PUCLASS_FILESELECTOR ;
  strcpy ( selected_type_string, ob->getLegend () ) ;
  active_button = (puButton *)ob ;
}

void list_box_cb ( puObject *ob )
{
  selected_object_sticky = ( glutGetModifiers () & GLUT_ACTIVE_CTRL ) ? 1 : 0 ;
  selected_object_type = PUCLASS_LISTBOX ;
  strcpy ( selected_type_string, ob->getLegend () ) ;
  active_button = (puButton *)ob ;
}



void button_cb ( puObject *ob )
{
  selected_object_sticky = ( glutGetModifiers () & GLUT_ACTIVE_CTRL ) ? 1 : 0 ;
  selected_object_type = PUCLASS_BUTTON ;
  strcpy ( selected_type_string, ob->getLegend () ) ;
  active_button = (puButton *)ob ;
}

void one_shot_cb ( puObject *ob )
{
  selected_object_sticky = ( glutGetModifiers () & GLUT_ACTIVE_CTRL ) ? 1 : 0 ;
  selected_object_type = PUCLASS_ONESHOT ;
  strcpy ( selected_type_string, ob->getLegend () ) ;
  active_button = (puButton *)ob ;
}

void arrow_button_cb ( puObject *ob )
{
  selected_object_sticky = ( glutGetModifiers () & GLUT_ACTIVE_CTRL ) ? 1 : 0 ;
  selected_object_type = PUCLASS_ARROW ;
  strcpy ( selected_type_string, ob->getLegend () ) ;
  active_button = (puButton *)ob ;
}

void button_box_cb ( puObject *ob )
{
  selected_object_sticky = ( glutGetModifiers () & GLUT_ACTIVE_CTRL ) ? 1 : 0 ;
  selected_object_type = PUCLASS_BUTTONBOX ;
  strcpy ( selected_type_string, ob->getLegend () ) ;
  active_button = (puButton *)ob ;
}

void combo_box_cb ( puObject *ob )
{
  selected_object_sticky = ( glutGetModifiers () & GLUT_ACTIVE_CTRL ) ? 1 : 0 ;
  selected_object_type = PUCLASS_COMBOBOX ;
  strcpy ( selected_type_string, ob->getLegend () ) ;
  active_button = (puButton *)ob ;
}



void dial_cb ( puObject *ob )
{
  selected_object_sticky = ( glutGetModifiers () & GLUT_ACTIVE_CTRL ) ? 1 : 0 ;
  selected_object_type = PUCLASS_DIAL ;
  strcpy ( selected_type_string, ob->getLegend () ) ;
  active_button = (puButton *)ob ;
}

void slider_cb ( puObject *ob )
{
  selected_object_sticky = ( glutGetModifiers () & GLUT_ACTIVE_CTRL ) ? 1 : 0 ;
  selected_object_type = PUCLASS_SLIDER ;
  strcpy ( selected_type_string, ob->getLegend () ) ;
  active_button = (puButton *)ob ;
}

void bislider_cb ( puObject *ob )
{
  selected_object_sticky = ( glutGetModifiers () & GLUT_ACTIVE_CTRL ) ? 1 : 0 ;
  selected_object_type = PUCLASS_BISLIDER ;
  strcpy ( selected_type_string, ob->getLegend () ) ;
  active_button = (puButton *)ob ;
}

void trislider_cb ( puObject *ob )
{
  selected_object_sticky = ( glutGetModifiers () & GLUT_ACTIVE_CTRL ) ? 1 : 0 ;
  selected_object_type = PUCLASS_TRISLIDER ;
  strcpy ( selected_type_string, ob->getLegend () ) ;
  active_button = (puButton *)ob ;
}

void spinbox_cb ( puObject *ob )
{
  selected_object_sticky = ( glutGetModifiers () & GLUT_ACTIVE_CTRL ) ? 1 : 0 ;
  selected_object_type = PUCLASS_SPINBOX ;
  strcpy ( selected_type_string, ob->getLegend () ) ;
  active_button = (puButton *)ob ;
}

// Function to define the window

int define_widget_window ()
{
  widget_window = glutCreateWindow      ( "Widget List"  ) ;
  int ln = 90 ;  // Button length
  int ht = 20 ;  // Button height
  int sp = 20 ;  // Button spacing
  int total_screen_width = glutGet( GLUT_SCREEN_WIDTH ) ;

  glutReshapeWindow     ( 4 * ln + 5 * sp,  5 * ht + 6 * sp ) ;
  glutPositionWindow    ( total_screen_width - (4 * ln + 5 * sp) - 20, 20 ) ;
  glutDisplayFunc       ( widget_window_displayfn ) ;
  glutKeyboardFunc      ( widget_window_keyfn     ) ;
  glutSpecialFunc       ( widget_window_specialfn ) ;
  glutMotionFunc        ( widget_window_motionfn  ) ;
  glutMouseFunc         ( widget_window_mousefn   ) ;

  // Global variable initialization (type string)
  selected_type_string = new char [ PUSTRING_MAX ] ;

  // Set up the widgets

  puGroup *widget_group = new puGroup ( 0, 0 ) ;

  puFrame *widget_frame = new puFrame ( 0, 0, 4 * ln + 5 * sp,  5 * ht + 6 * sp ) ;

  puButton *button = (puButton *)NULL ;
  button = new puButton ( sp, 5*sp+4*ht, sp+ln, 5*sp+5*ht ) ;
  button->setLegend ( "puFrame" ) ;
  button->setCallback ( frame_cb ) ;

  button = new puButton ( sp, 4*sp+3*ht, sp+ln, 4*sp+4*ht ) ;
  button->setLegend ( "puText" ) ;
  button->setCallback ( text_cb ) ;

  button = new puButton ( sp, 2*sp+ht, sp+ln, 2*sp+2*ht ) ;
  button->setLegend ( "puMenuBar" ) ;
  button->setCallback ( menu_bar_cb ) ;

  button = new puButton ( sp, sp, sp+ln, sp+ht ) ;
  button->setLegend ( "puVerticalMenu" ) ;
  button->setCallback ( vertical_menu_cb ) ;



  button = new puButton ( 2*sp+ln, 5*sp+4*ht, 2*sp+2*ln, 5*sp+5*ht ) ;
  button->setLegend ( "puInput" ) ;
  button->setCallback ( input_cb ) ;

  button = new puButton ( 2*sp+ln, 4*sp+3*ht, 2*sp+2*ln, 4*sp+4*ht ) ;
  button->setLegend ( "puLargeInput" ) ;
  button->setCallback ( large_input_cb ) ;

  button = new puButton ( 2*sp+ln, 3*sp+2*ht, 2*sp+2*ln, 3*sp+3*ht ) ;
  button->setLegend ( "puSelectBox" ) ;
  button->setCallback ( select_box_cb ) ;

  button = new puButton ( 2*sp+ln, 2*sp+ht, 2*sp+2*ln, 2*sp+2*ht ) ;
  button->setLegend ( "puFileSelector" ) ;
  button->setCallback ( file_selector_cb ) ;

  button = new puButton ( 2*sp+ln, sp, 2*sp+2*ln, sp+ht ) ;
  button->setLegend ( "puListBox" ) ;
  button->setCallback ( list_box_cb ) ;



  button = new puButton ( 3*sp+2*ln, 5*sp+4*ht, 3*sp+3*ln, 5*sp+5*ht ) ;
  button->setLegend ( "puButton" ) ;
  button->setCallback ( button_cb ) ;

  button = new puButton ( 3*sp+2*ln, 4*sp+3*ht, 3*sp+3*ln, 4*sp+4*ht ) ;
  button->setLegend ( "puOneShot" ) ;
  button->setCallback ( one_shot_cb ) ;

  button = new puButton ( 3*sp+2*ln, 3*sp+2*ht, 3*sp+3*ln, 3*sp+3*ht ) ;
  button->setLegend ( "puArrowButton" ) ;
  button->setCallback ( arrow_button_cb ) ;

  button = new puButton ( 3*sp+2*ln, 2*sp+ht, 3*sp+3*ln, 2*sp+2*ht ) ;
  button->setLegend ( "puButtonBox" ) ;
  button->setCallback ( button_box_cb ) ;

  button = new puButton ( 3*sp+2*ln, sp, 3*sp+3*ln, sp+ht ) ;
  button->setLegend ( "puComboBox" ) ;
  button->setCallback ( combo_box_cb ) ;



  button = new puButton ( 4*sp+3*ln, 5*sp+4*ht, 4*sp+4*ln, 5*sp+5*ht ) ;
  button->setLegend ( "puDial" ) ;
  button->setCallback ( dial_cb ) ;

  button = new puButton ( 4*sp+3*ln, 4*sp+3*ht, 4*sp+4*ln, 4*sp+4*ht ) ;
  button->setLegend ( "puSlider" ) ;
  button->setCallback ( slider_cb ) ;

  button = new puButton ( 4*sp+3*ln, 3*sp+2*ht, 4*sp+4*ln, 3*sp+3*ht ) ;
  button->setLegend ( "puBiSlider" ) ;
  button->setCallback ( bislider_cb ) ;

  button = new puButton ( 4*sp+3*ln, 2*sp+ht, 4*sp+4*ln, 2*sp+2*ht ) ;
  button->setLegend ( "puTriSlider" ) ;
  button->setCallback ( trislider_cb ) ;

  button = new puButton ( 4*sp+3*ln, 0*sp+ht, 4*sp+4*ln, 0*sp+2*ht ) ;
  button->setLegend ( "puSpinBox" ) ;
  button->setCallback ( spinbox_cb ) ;

  widget_group->close () ;

  return 0 ;
}

