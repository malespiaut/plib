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

// Status Window

#include <plib/pu.h>

#include "WidgetList.h"

// From the Main Window:

extern int max_layer ;

extern WidgetList *active_widget ;
extern puObject *active_object ;

extern bool main_window_changed ;

// Status window parameters
int status_window = 0 ;  // Window handle

// Status window widgets

puInput *object_size_x ;
puInput *object_size_y ;
puInput *object_position_x ;
puInput *object_position_y ;

puInput *object_label ;
puComboBox *object_vert_label_place ;
puComboBox *object_horz_label_place ;
puInput *object_legend ;
puComboBox *object_vert_legend_place ;
puComboBox *object_horz_legend_place ;

puInput *object_name ;
puButtonBox *object_callbacks ;

puButtonBox *object_visible ;
puOneShot *reveal_all_objects ;
puInput *object_layer ;

puInput *window_name ;
puInput *window_size_x ;
puInput *window_size_y ;
puInput *window_position_x ;
puInput *window_position_y ;

puInput *window_color_r ;
puInput *window_color_g ;
puInput *window_color_b ;
puInput *window_color_a ;

puDialogBox *dialog = (puDialogBox *)NULL ;

// Saving Window Parameters

puFileSelector *file_selector ;

int write_window = 0 ;

// Function to set the widgets from the active object

void setHorizontalPlaceWidget ( puComboBox *ob, int place )
{
  switch ( place )
  {
  case PUPLACE_TOP_LEFT          :
  case PUPLACE_ABOVE_LEFT        :
  case PUPLACE_UPPER_LEFT        :
  case PUPLACE_CENTERED_LEFT     :
  case PUPLACE_LOWER_LEFT        :
  case PUPLACE_BELOW_LEFT        :
  case PUPLACE_BOTTOM_LEFT       : ob->setValue ( "Left" ) ;  break ;
  case PUPLACE_TOP_CENTERED      :
  case PUPLACE_CENTERED_CENTERED :
  case PUPLACE_BOTTOM_CENTERED   : ob->setValue ( "Center" ) ;  break ;
  case PUPLACE_TOP_RIGHT         :
  case PUPLACE_ABOVE_RIGHT       :
  case PUPLACE_UPPER_RIGHT       :
  case PUPLACE_CENTERED_RIGHT    :
  case PUPLACE_LOWER_RIGHT       :
  case PUPLACE_BELOW_RIGHT       :
  case PUPLACE_BOTTOM_RIGHT      : ob->setValue ( "Right" ) ;  break ;
  }
}

void setVerticalPlaceWidget ( puComboBox *ob, int place )
{
  switch ( place )
  {
  case PUPLACE_TOP_LEFT          :
  case PUPLACE_TOP_CENTERED      :
  case PUPLACE_TOP_RIGHT         : ob->setValue ( "Top" ) ;  break ;
  case PUPLACE_ABOVE_LEFT        :
  case PUPLACE_ABOVE_RIGHT       : ob->setValue ( "Above" ) ;  break ;
  case PUPLACE_UPPER_LEFT        :
  case PUPLACE_UPPER_RIGHT       : ob->setValue ( "Upper" ) ;  break ;
  case PUPLACE_CENTERED_LEFT     :
  case PUPLACE_CENTERED_CENTERED :
  case PUPLACE_CENTERED_RIGHT    : ob->setValue ( "Center" ) ;  break ;
  case PUPLACE_LOWER_LEFT        :
  case PUPLACE_LOWER_RIGHT       : ob->setValue ( "Lower" ) ;  break ;
  case PUPLACE_BELOW_LEFT        :
  case PUPLACE_BELOW_RIGHT       : ob->setValue ( "Below" ) ;  break ;
  case PUPLACE_BOTTOM_LEFT       :
  case PUPLACE_BOTTOM_CENTERED   :
  case PUPLACE_BOTTOM_RIGHT      : ob->setValue ( "Bottom" ) ;  break ;
  }
}

void setStatusWidgets ( WidgetList *wid )
{
  if ( wid )
  {
    puObject *ob = wid->obj ;

    int a, b ;
    ob->getSize ( &a, &b ) ;
    object_size_x->setValue ( a ) ;
    object_size_y->setValue ( b ) ;

    ob->getPosition ( &a, &b ) ;
    object_position_x->setValue ( a ) ;
    object_position_y->setValue ( b ) ;

    if ( wid->label_text )
      object_label->setValue ( wid->label_text ) ;
    else
      object_label->clrValue () ;

    setHorizontalPlaceWidget ( object_horz_label_place, ob->getLabelPlace () ) ;
    setVerticalPlaceWidget   ( object_vert_label_place, ob->getLabelPlace () ) ;

    if ( wid->legend_text )
      object_legend->setValue ( wid->legend_text ) ;
    else
      object_legend->clrValue () ;

    setHorizontalPlaceWidget ( object_horz_legend_place, ob->getLegendPlace () ) ;
    setVerticalPlaceWidget   ( object_vert_legend_place, ob->getLegendPlace () ) ;

    object_name->setValue ( wid->object_name ) ;
    object_callbacks->setValue ( wid->callbacks ) ;

    object_visible->setValue ( wid->visible ? 1 : 0 ) ;
    object_layer->setValue ( wid->layer ) ;
  }
  else
  {
    object_size_x->setValue ( 0 ) ;
    object_size_y->setValue ( 0 ) ;

    object_position_x->setValue ( 0 ) ;
    object_position_y->setValue ( 0 ) ;

    object_label->setValue ( "" ) ;
    setHorizontalPlaceWidget ( object_horz_label_place, PUPLACE_CENTERED_LEFT ) ;
    setVerticalPlaceWidget   ( object_vert_label_place, PUPLACE_CENTERED_LEFT ) ;

    object_legend->setValue ( "" ) ;
    setHorizontalPlaceWidget ( object_horz_legend_place, PUPLACE_CENTERED_LEFT ) ;
    setVerticalPlaceWidget   ( object_vert_legend_place, PUPLACE_CENTERED_LEFT ) ;

    object_name->setValue ( "" ) ;
    object_callbacks->setValue ( 0 ) ;

    object_visible->setValue ( 0 ) ;
    object_layer->setValue ( 0 ) ;
  }
}

// GLUT Status Window Callbacks

static void status_window_specialfn ( int key, int, int )
{
  puKeyboard ( key, PU_DOWN ) ;

  glutPostRedisplay () ;
}

static void status_window_keyfn ( unsigned char key, int, int )
{
  puKeyboard ( key, PU_DOWN ) ;

  glutPostRedisplay () ;
}

static void status_window_motionfn ( int x, int y )
{
  puMouse ( x, y ) ;

  glutPostRedisplay () ;
}

static void status_window_mousefn ( int button, int updown, int x, int y )
{
  puMouse ( button, updown, x, y ) ;

  glutPostRedisplay () ;
}


static void status_window_displayfn ( void )
{
  glutSetWindow ( status_window ) ;

  /* Clear the screen */

  glClearColor ( 0.1, 0.1, 0.1, 1.0 ) ;
  glClear      ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;

  puDisplay () ;

  /* Update GLUT */

  glutSwapBuffers   () ;
  glutPostRedisplay () ;
}

static void selection_window_displayfn ( void )
{
  glutSetWindow ( write_window ) ;

  /* Clear the screen */

  glClearColor ( 0.1, 0.1, 0.1, 1.0 ) ;
  glClear      ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;

  puDisplay () ;

  /* Update GLUT */

  glutSwapBuffers   () ;
  glutPostRedisplay () ;
}

// PUI Callback Functions:

extern void write_code ( puObject *ob ) ;

void write_window_reshapefn ( int w, int h )
{
  file_selector->setSize ( w, h ) ;
}

void write_code_cb ( puObject *ob )
{
  int w = 320, h = 270 ;
  if ( write_window )
    glutSetWindow ( write_window ) ;
  else
  {
    write_window = glutCreateWindow ( "Writing " ) ;
    glutDisplayFunc       ( selection_window_displayfn ) ;
    glutKeyboardFunc      ( status_window_keyfn     ) ;
    glutSpecialFunc       ( status_window_specialfn ) ;
    glutMouseFunc         ( status_window_mousefn   ) ;
    glutMotionFunc        ( status_window_motionfn  ) ;
    glutPassiveMotionFunc ( status_window_motionfn  ) ;
    glutIdleFunc          ( selection_window_displayfn ) ;
    glutReshapeFunc       ( write_window_reshapefn ) ;
  }

  glutShowWindow () ;
  glutReshapeWindow ( w, h ) ;
  glutPositionWindow ( ( 640 - w ) / 2, ( 480 - h ) / 2 ) ;

  file_selector = new puFileSelector ( 0, 0, w, h, 1, "\\", "Pick File to Write To" ) ;
  file_selector -> setCallback ( write_code ) ;
  file_selector->setChildStyle ( PUCLASS_ONESHOT, PUSTYLE_BOXED ) ;
  file_selector->setChildBorderThickness ( PUCLASS_ONESHOT, 5 ) ;
  file_selector->setChildColour ( PUCLASS_SLIDER, 0, 0.5, 0.5, 0.5 ) ;
}

// Clear Objects callbacks

void clear_ok_cb ( puObject *ob )
{
  // Clear the widget list
  extern WidgetList *widgets ;

  WidgetList *wid = widgets ;

  while ( wid )
  {
    widgets = wid->next ;
    delete wid->object_type_name ;
    delete wid->legend_text ;
    delete wid->label_text ;
    delete wid->obj ;
    delete wid ;
    wid = widgets ;
  }

  active_widget = (WidgetList *)NULL ;
  active_object = (puObject *)NULL ;
  main_window_changed = false ;

  setStatusWidgets ( active_widget ) ;

  puDeleteObject ( dialog ) ;  // Delete the dialog box
  dialog = (puDialogBox *)NULL ;
}

void cancel_cb ( puObject *ob )
{
  // Don't do whatever you were planning to do (clear the widget list or quit)
  // but do delete the dialog box.
  puDeleteObject ( dialog ) ;
  dialog = (puDialogBox *)NULL ;
}

void clear_cb ( puObject *ob )
{
  if ( main_window_changed )
  {
    dialog = new puDialogBox ( 180, 20 ) ;
    new puFrame ( 0, 0, 125, 70 ) ;
    puText *text = new puText ( 10, 40 ) ;
    text->setLabel ( "Are you sure?" ) ;
    puOneShot *ok = new puOneShot ( 10, 10, "Yes" ) ;
    ok->setCallback ( clear_ok_cb ) ;
    puOneShot *cancel = new puOneShot ( 60, 10, "Cancel" ) ;
    cancel->setCallback ( cancel_cb ) ;
    dialog->close () ;
    dialog->reveal () ;
  }
  else
    clear_ok_cb ( (puObject *)NULL ) ;
}

void quit_ok_cb ( puObject *ob )
{
  exit ( 0 ) ;
}

void quit_cb ( puObject *ob )
{
  if ( main_window_changed )
  {
    dialog = new puDialogBox ( 280, 20 ) ;
    new puFrame ( 0, 0, 125, 70 ) ;
    puText *text = new puText ( 10, 40 ) ;
    text->setLabel ( "Are you sure?" ) ;
    puOneShot *ok = new puOneShot ( 10, 10, "Yes" ) ;
    ok->setCallback ( quit_ok_cb ) ;
    puOneShot *cancel = new puOneShot ( 60, 10, "Cancel" ) ;
    cancel->setCallback ( cancel_cb ) ;
    dialog->close () ;
    dialog->reveal () ;
  }
  else
    quit_ok_cb ( (puObject *)NULL ) ;
}

void object_size_cb ( puObject *ob )
{
  if ( active_object )
  {
    int w, h ;
    active_object->getSize ( &w, &h ) ;
    if ( ob == object_size_x )
    {
      w = ob->getIntegerValue () ;
      if ( w < 1 ) w = 1 ;
      ob->setValue ( w ) ;
    }
    else
    {
      h = ob->getIntegerValue () ;
      if ( h < 1 ) h = 1 ;
      ob->setValue ( h ) ;
    }

    active_object->setSize ( w, h ) ;
  }
}

void object_position_cb ( puObject *ob )
{
  if ( active_object )
  {
    int x, y ;
    active_object->getPosition ( &x, &y ) ;
    if ( ob == object_position_x )
      x = ob->getIntegerValue () ;
    else
      y = ob->getIntegerValue () ;

    ob->setValue ( ob->getIntegerValue () ) ;
    active_object->setPosition ( x, y ) ;
  }
}

void window_size_cb ( puObject *ob )
{
  extern int main_window_width  ;
  extern int main_window_height ;

  extern int main_window ;

  glutSetWindow ( main_window ) ;
  if ( ob == window_size_x )
    main_window_width = ob->getIntegerValue () ;
  else
    main_window_height = ob->getIntegerValue () ;

  ob->setValue ( ob->getIntegerValue () ) ;
  glutReshapeWindow ( main_window_width, main_window_height ) ;

  glutSetWindow ( status_window ) ;
}

void window_name_cb ( puObject *ob )
{
  extern char main_window_name [ PUSTRING_MAX ] ;

  extern int main_window ;

  glutSetWindow ( main_window ) ;
  strcpy ( main_window_name, ob->getStringValue () ) ;
  glutSetWindowTitle ( main_window_name ) ;
  glutSetWindow ( status_window ) ;
}

void label_cb ( puObject *ob )
{
  if ( active_widget )
  {
    delete active_widget->label_text ;
    active_widget->label_text = new char [ strlen ( ob->getStringValue () ) + 1 ] ;
    strcpy ( active_widget->label_text, ob->getStringValue () ) ;
    active_widget->obj->setLabel ( active_widget->label_text ) ;
  }
}

void label_place_cb ( puObject *ob )
{
  if ( active_object )
  {
    int horz = object_horz_label_place->getCurrentItem () ;
    int vert = object_vert_label_place->getCurrentItem () ;

    switch ( horz )
    {
    case 0 :  // Left
      switch ( vert )
      {
      case 0 :  // Top
        active_object->setLabelPlace ( PUPLACE_TOP_LEFT ) ;       break ;
      case 1 :  // Above
        active_object->setLabelPlace ( PUPLACE_ABOVE_LEFT ) ;     break ;
      case 2 :  // Upper
        active_object->setLabelPlace ( PUPLACE_UPPER_LEFT ) ;     break ;
      case 3 :  // Center
        active_object->setLabelPlace ( PUPLACE_CENTERED_LEFT ) ;  break ;
      case 4 :  // Lower
        active_object->setLabelPlace ( PUPLACE_LOWER_LEFT ) ;     break ;
      case 5 :  // Below
        active_object->setLabelPlace ( PUPLACE_BELOW_LEFT ) ;     break ;
      case 6 :  // Bottom
        active_object->setLabelPlace ( PUPLACE_BOTTOM_LEFT ) ;    break ;
      }

      break ;

    case 1 :  // Center
      switch ( vert )
      {
      case 0 :  // Top
        active_object->setLabelPlace ( PUPLACE_TOP_CENTERED ) ;       break ;
      case 1 :  // Above  -- not allowed
      case 2 :  // Upper  -- not allowed
      case 3 :  // Center -- not allowed
      case 4 :  // Lower  -- not allowed
      case 5 :  // Below  -- not allowed
        if ( ob == object_horz_label_place )
        {
          ob->setValue ( "Left" ) ;  // Left
          active_object->setLabelPlace ( PUPLACE_CENTERED_LEFT ) ;
        }
        else
        {
          ob->setValue ( "Top" ) ;  // Top
          active_object->setLabelPlace ( PUPLACE_TOP_CENTERED ) ;
        }

        break ;
      case 6 :  // Bottom
        active_object->setLabelPlace ( PUPLACE_BOTTOM_CENTERED ) ;    break ;
      }

      break ;

    case 2 :  // Right
      switch ( vert )
      {
      case 0 :  // Top
        active_object->setLabelPlace ( PUPLACE_TOP_RIGHT ) ;       break ;
      case 1 :  // Above
        active_object->setLabelPlace ( PUPLACE_ABOVE_RIGHT ) ;     break ;
      case 2 :  // Upper
        active_object->setLabelPlace ( PUPLACE_UPPER_RIGHT ) ;     break ;
      case 3 :  // Center
        active_object->setLabelPlace ( PUPLACE_CENTERED_RIGHT ) ;  break ;
      case 4 :  // Lower
        active_object->setLabelPlace ( PUPLACE_LOWER_RIGHT ) ;     break ;
      case 5 :  // Below
        active_object->setLabelPlace ( PUPLACE_BELOW_RIGHT ) ;     break ;
      case 6 :  // Bottom
        active_object->setLabelPlace ( PUPLACE_BOTTOM_RIGHT ) ;    break ;
      }

      break ;
    }
  }
}

void legend_cb ( puObject *ob )
{
  if ( active_widget )
  {
    delete active_widget->legend_text ;
    active_widget->legend_text = new char [ strlen ( ob->getStringValue () ) + 1 ] ;
    strcpy ( active_widget->legend_text, ob->getStringValue () ) ;
  }
}

void legend_place_cb ( puObject *ob )
{
  if ( active_object )
  {
    int horz = object_horz_legend_place->getCurrentItem () ;
    int vert = object_vert_legend_place->getCurrentItem () ;

    switch ( horz )
    {
    case 0 :  // Left
      switch ( vert )
      {
      case 0 :  // Top
        active_object->setLegendPlace ( PUPLACE_TOP_LEFT ) ;       break ;
      case 1 :  // Above -- not allowed
      case 2 :  // Upper -- not allowed
        object_vert_legend_place->setValue ( "Top" ) ;
        active_object->setLegendPlace ( PUPLACE_TOP_LEFT ) ;       break ;
      case 3 :  // Center
        active_object->setLegendPlace ( PUPLACE_CENTERED_LEFT ) ;  break ;
      case 4 :  // Lower -- not allowed
      case 5 :  // Below -- not allowed
        object_vert_legend_place->setValue ( "Bottom" ) ;
        active_object->setLegendPlace ( PUPLACE_BOTTOM_LEFT ) ;    break ;
      case 6 :  // Bottom
        active_object->setLegendPlace ( PUPLACE_BOTTOM_LEFT ) ;    break ;
      }

      break ;

    case 1 :  // Center
      switch ( vert )
      {
      case 0 :  // Top
        active_object->setLegendPlace ( PUPLACE_TOP_CENTERED ) ;       break ;
      case 1 :  // Above  -- not allowed
      case 2 :  // Upper  -- not allowed
        object_vert_legend_place->setValue ( "Top" ) ;
        active_object->setLegendPlace ( PUPLACE_TOP_CENTERED ) ;       break ;
      case 3 :  // Center
        active_object->setLegendPlace ( PUPLACE_CENTERED_CENTERED ) ;  break ;
      case 4 :  // Lower  -- not allowed
      case 5 :  // Below  -- not allowed
        object_vert_legend_place->setValue ( "Bottom" ) ;
        active_object->setLegendPlace ( PUPLACE_BOTTOM_CENTERED ) ;    break ;
      case 6 :  // Bottom
        active_object->setLegendPlace ( PUPLACE_BOTTOM_CENTERED ) ;    break ;
      }

      break ;

    case 2 :  // Right
      switch ( vert )
      {
      case 0 :  // Top
        active_object->setLegendPlace ( PUPLACE_TOP_RIGHT ) ;       break ;
      case 1 :  // Above  -- not allowed
      case 2 :  // Upper  -- not allowed
        object_vert_legend_place->setValue ( "Top" ) ;
        active_object->setLegendPlace ( PUPLACE_TOP_RIGHT ) ;       break ;
      case 3 :  // Center
        active_object->setLegendPlace ( PUPLACE_CENTERED_RIGHT ) ;  break ;
      case 4 :  // Lower  -- not allowed
      case 5 :  // Below  -- not allowed
        object_vert_legend_place->setValue ( "Bottom" ) ;
        active_object->setLegendPlace ( PUPLACE_BOTTOM_RIGHT ) ;    break ;
      case 6 :  // Bottom
        active_object->setLegendPlace ( PUPLACE_BOTTOM_RIGHT ) ;    break ;
      }

      break ;
    }
  }
}

void name_cb ( puObject *ob )
{
  if ( active_widget )
  {
    strcpy ( active_widget->object_name, ob->getStringValue () ) ;

    // Remove spaces from the widget name
    char *spc = strchr ( active_widget->object_name, ' ' ) ;
    while ( spc )
    {
      *spc = '_' ;
      spc = strchr ( active_widget->object_name, ' ' ) ;
    }
  }
}

void callback_cb ( puObject *ob )
{
  if ( active_widget ) active_widget->callbacks = ob->getIntegerValue () ;
}

void reveal_all_cb ( puObject *ob )
{
  extern WidgetList *widgets ;
  WidgetList *wid = widgets ;
  while ( wid )
  {
    wid->visible = true ;
    wid = wid->next ;
  }
}

void visible_cb ( puObject *ob )
{
  if ( active_widget ) active_widget->visible = ( ob->getIntegerValue () != 0 ) ;
}

void layer_cb ( puObject *ob )
{
  if ( ob->getIntegerValue () < 0 ) ob->setValue ( 0 ) ;
  if ( active_widget )
  {
    active_widget->layer = ob->getIntegerValue () ;
    if ( max_layer <= active_widget->layer ) max_layer = active_widget->layer + 1 ;
  }
}

// Function to define the window

int define_status_window ()
{
  status_window = glutCreateWindow      ( "Status Window"  ) ;

  glutPositionWindow    ( 420, 300 ) ;
  glutReshapeWindow     ( 500, 380 ) ;
  glutDisplayFunc       ( status_window_displayfn ) ;
  glutKeyboardFunc      ( status_window_keyfn     ) ;
  glutSpecialFunc       ( status_window_specialfn ) ;
  glutMotionFunc        ( status_window_motionfn  ) ;
  glutMouseFunc         ( status_window_mousefn   ) ;

  // Set up the widgets

  puGroup *status_group = new puGroup ( 0, 0 ) ;
  puFrame *status_frame = new puFrame ( 0, 0, 500, 380 ) ;

  puOneShot *oneshot = (puOneShot *)NULL ;
  oneshot = new puOneShot ( 10, 10, 200, 30 ) ;
  oneshot->setLegend ( "Write Window Code" ) ;
  oneshot->setCallback ( write_code_cb ) ;

  oneshot = new puOneShot ( 210, 10, 300, 30 ) ;
  oneshot->setLegend ( "Clear" ) ;
  oneshot->setCallback ( clear_cb ) ;

  oneshot = new puOneShot ( 310, 10, 400, 30 ) ;
  oneshot->setLegend ( "Quit" ) ;
  oneshot->setCallback ( quit_cb ) ;

  object_size_x = new puInput ( 360, 40, 410, 60 ) ;
  object_size_x->setLabel ( "Object Size :" ) ;
  object_size_x->setLabelPlace ( PUPLACE_LEFT ) ;
  object_size_x->setCallback ( object_size_cb ) ;
  object_size_x->setDownCallback ( object_size_cb ) ;

  object_size_y = new puInput ( 410, 40, 460, 60 ) ;
  object_size_y->setCallback ( object_size_cb ) ;
  object_size_y->setDownCallback ( object_size_cb ) ;

  object_position_x = new puInput ( 160, 40, 210, 60 ) ;
  object_position_x->setLabel ( "Object Position :" ) ;
  object_position_x->setLabelPlace ( PUPLACE_LEFT ) ;
  object_position_x->setCallback ( object_position_cb ) ;
  object_position_x->setDownCallback ( object_position_cb ) ;

  object_position_y = new puInput ( 210, 40, 260, 60 ) ;
  object_position_y->setCallback ( object_position_cb ) ;
  object_position_y->setDownCallback ( object_position_cb ) ;


  static char *vert_place_entries [] = { "Top", "Above", "Upper", "Center",
                                         "Lower", "Below", "Bottom", NULL } ;

  static char *horz_place_entries [] = { "Left", "Center", "Right", NULL } ;

  object_label = new puInput ( 60, 70, 260, 90 ) ;
  object_label->setLabel ( "Label" ) ;
  object_label->setLabelPlace ( PUPLACE_LEFT ) ;
  object_label->setCallback ( label_cb ) ;
  object_label->setDownCallback ( label_cb ) ;

  object_vert_label_place = new puComboBox ( 310, 70, 400, 90, vert_place_entries, FALSE ) ;
  object_vert_label_place->setLabel ( "Place" ) ;
  object_vert_label_place->setLabelPlace ( PUPLACE_LEFT ) ;
  object_vert_label_place->setCallback ( label_place_cb ) ;

  object_horz_label_place = new puComboBox ( 400, 70, 490, 90, horz_place_entries, FALSE ) ;
  object_horz_label_place->setCallback ( label_place_cb ) ;

  object_legend = new puInput ( 60, 100, 260, 120 ) ;
  object_legend->setLabel ( "Legend" ) ;
  object_legend->setLabelPlace ( PUPLACE_LEFT ) ;
  object_legend->setCallback ( legend_cb ) ;
  object_legend->setDownCallback ( legend_cb ) ;

  object_vert_legend_place = new puComboBox ( 310, 100, 400, 120, vert_place_entries, FALSE ) ;
  object_vert_legend_place->setLabel ( "Place" ) ;
  object_vert_legend_place->setLabelPlace ( PUPLACE_LEFT ) ;
  object_vert_legend_place->setCallback ( legend_place_cb ) ;

  object_horz_legend_place = new puComboBox ( 400, 100, 490, 120, horz_place_entries, FALSE ) ;
  object_horz_legend_place->setCallback ( legend_place_cb ) ;

  object_name = new puInput ( 110, 130, 310, 150 ) ;
  object_name->setLabel ( "Widget Name:" ) ;
  object_name->setLabelPlace ( PUPLACE_CENTERED_LEFT ) ;
  object_name->setCallback ( name_cb ) ;
  object_name->setDownCallback ( name_cb ) ;

  static char *callback_entries [] = { "Up", "Active", "Down", NULL } ;
  object_callbacks = new puButtonBox ( 10, 160, 150, 230, callback_entries, FALSE ) ;
  object_callbacks->setLabel ( "Widget Callbacks" ) ;
  object_callbacks->setLabelPlace ( PUPLACE_TOP_LEFT ) ;
  object_callbacks->setCallback ( callback_cb ) ;

  static char *visible_entries [] = { "Visible", NULL } ;
  object_visible = new puButtonBox ( 350, 160, 490, 190, visible_entries, FALSE ) ;
  object_visible->setCallback ( visible_cb ) ;

  reveal_all_objects = new puOneShot ( 350, 130, 490, 150 ) ;
  reveal_all_objects->setLegend ( "Reveal All Widgets" ) ;
  reveal_all_objects->setCallback ( reveal_all_cb ) ;

  object_layer = new puInput ( 400, 210, 490, 230 ) ;
  object_layer->setLabel ( "Widget Layer" ) ;
  object_layer->setLabelPlace ( PUPLACE_CENTERED_LEFT ) ;
  object_layer->setCallback ( layer_cb ) ;


  window_name = new puInput ( 130, 350, 430, 370 ) ;
  window_name->setLabel ( "Window Name :" ) ;
  window_name->setLabelPlace ( PUPLACE_CENTERED_LEFT ) ;
  window_name->setCallback ( window_name_cb ) ;
  window_name->setDownCallback ( window_name_cb ) ;

  window_size_x = new puInput ( 130, 320, 180, 340 ) ;
  window_size_x->setLabel ( "Window Size :" ) ;
  window_size_x->setLabelPlace ( PUPLACE_CENTERED_LEFT ) ;
  window_size_x->setCallback ( window_size_cb ) ;
  window_size_x->setDownCallback ( window_size_cb ) ;

  window_size_y = new puInput ( 180, 320, 230, 340 ) ;
  window_size_y->setCallback ( window_size_cb ) ;
  window_size_y->setDownCallback ( window_size_cb ) ;

  extern int main_window_x, main_window_y ;

  window_position_x = new puInput ( 130, 290, 180, 310 ) ;
  window_position_x->setLabel ( "Window Position:" ) ;
  window_position_x->setLabelPlace ( PUPLACE_CENTERED_LEFT ) ;
  window_position_x->setValuator ( &main_window_x ) ;

  window_position_y = new puInput ( 180, 290, 230, 310 ) ;
  window_position_y->setValuator ( &main_window_y ) ;

  extern float main_window_color_r, main_window_color_g,
               main_window_color_b, main_window_color_a ;

  window_color_r = new puInput ( 120, 260, 160, 280 ) ;
  window_color_r->setLabel ( "Window Color:" ) ;
  window_color_r->setLabelPlace ( PUPLACE_CENTERED_LEFT ) ;
  window_color_r->setValuator ( &main_window_color_r ) ;

  window_color_g = new puInput ( 160, 260, 200, 280 ) ;
  window_color_g->setValuator ( &main_window_color_g ) ;

  window_color_b = new puInput ( 200, 260, 240, 280 ) ;
  window_color_b->setValuator ( &main_window_color_b ) ;

  window_color_a = new puInput ( 240, 260, 280, 280 ) ;
  window_color_a->setValuator ( &main_window_color_a ) ;

  status_group->close () ;

  return 0 ;
}

