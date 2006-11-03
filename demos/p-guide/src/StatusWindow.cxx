/*
     This file is part of P-GUIDE -
     PUI-based Graphical User Interface Designer.
     Copyright (C) 2002, 2006  John F. Fay

     P-GUIDE is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     P-GUIDE is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with P-GUIDE; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

     $Id$
*/

// Status Window

#include "WidgetList.h"

// From the Main Window:

extern int max_layer ;

extern WidgetList *active_widget ;
extern puObject *active_object ;

extern char pguide_current_directory [ PUSTRING_MAX ] ;

extern bool main_window_changed ;

extern char main_window_name [ PUSTRING_MAX ] ;

extern bool autolock ;

// Status window parameters
int status_window = 0 ;  // Window handle
extern int main_window ;

// Status window widgets

puInput *object_size_x ;
puInput *object_size_y ;
puInput *object_position_x ;
puInput *object_position_y ;

static puFrame *widget_section_frame ;

static puInput *object_label ;
static puaComboBox *object_vert_label_place ;
static puaComboBox *object_horz_label_place ;
static puInput *object_legend ;
static puaComboBox *object_vert_legend_place ;
static puaComboBox *object_horz_legend_place ;

static puInput *object_name ;
static puButtonBox *object_callbacks ;

static puFrame *layer_manipulation_frame ;
static puButtonBox *object_visible ;
static puOneShot *reveal_all_objects ;
static puOneShot *hide_all_objects ;

static puOneShot *reveal_all_layer ;
static puOneShot *hide_all_layer ;

static puaSpinBox *layer_to_act_on ;
static puaSpinBox *object_layer ;

static puText *object_layer_text ;
static puFrame *layer_to_act_on_frame ;
static puFrame *layer_all_frame ;

puInput *window_name ;
puInput *window_size_x ;
puInput *window_size_y ;
puInput *window_position_x ;
puInput *window_position_y ;

static puText *window_color_label ;
puaSpinBox *window_color_r ;
puaSpinBox *window_color_g ;
puaSpinBox *window_color_b ;
puaSpinBox *window_color_a ;

static puButtonBox *autolock_toggle ;

static puDialogBox *dialog = (puDialogBox *)NULL ;
static puInput *newname = (puInput *)NULL ;

static puMenuBar *menubar ;

// Saving Window Parameters

puaFileSelector *file_selector ;

static int write_window = 0 ;

// Extra needed prototyping
static void dupname_ok_cb ( puObject *ob ) ;

// Function to set the widgets from the active object

static void setHorizontalPlaceWidget ( puaComboBox *ob, int place )
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

static void setVerticalPlaceWidget ( puaComboBox *ob, int place )
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
    autolock_toggle->setValue ( autolock );
    object_layer->setValue ( (int) wid->layer ) ;
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
    autolock_toggle->setValue ( autolock );
    object_layer->setValue ( 0 ) ;
  }
}

// GLUT Status Window Callbacks

static void status_window_specialfn ( int key, int, int )
{
  puKeyboard ( key + PU_KEY_GLUT_SPECIAL_OFFSET, PU_DOWN ) ;

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

  glClearColor ( 0.1f, 0.1f, 0.1f, 1.0f ) ;
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

  glClearColor ( 0.1f, 0.1f, 0.1f, 1.0f ) ;
  glClear      ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;

  puDisplay () ;

  /* Update GLUT */

  glutSwapBuffers   () ;
  glutPostRedisplay () ;
}

// PUI Callback Functions:

extern void write_code ( puObject *ob ) ;

extern void saveProject ( puObject *ob ) ;

extern void loadProject ( puObject *ob ) ;

static void write_window_reshapefn ( int w, int h )
{
  file_selector->setSize ( w, h ) ;
}

// Clear Objects callbacks

static void window_wiper ( puObject *ob )
{

  // Prepare to clear the properties window 
  extern int properties_window ;
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
  window_color_r->setValue(1.0f) ;
  window_color_g->setValue(1.0f) ;
  window_color_b->setValue(1.0f) ;
  window_color_a->setValue(1.0f) ;
  window_name->setValue("PUI GUI Builder");
  window_size_x->setValue(600) ;
  window_size_y->setValue(600) ;
  window_position_x->setValue(0) ;
  window_position_y->setValue(0) ;
  main_window_changed = false ;
  autolock = false ;
  autolock_toggle->setValue(0);

  glutSetWindow ( main_window ) ;
  glutReshapeWindow ( 600, 600 ) ;

  if (properties_window)
  {
      extern puGroup *properties_group; 
      /* Delete the widgets */
      puDeleteObject( properties_group );
      glutDestroyWindow( properties_window );
      properties_window = 0;
  }

  glutSetWindow ( status_window ) ;

  setStatusWidgets ( active_widget ) ;
}

static void write_code_cb ( puObject *ob )
{
  int w = 320, h = 270 ;
  if ( write_window )
  {
    glutSetWindow ( write_window ) ;
    glutSetWindowTitle( "Writing " );
  }
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

  file_selector = new puaFileSelector ( 0, 0, w, h, 1, pguide_current_directory, "Pick File to Write To" ) ;
  file_selector -> setCallback ( write_code ) ;
  file_selector->setChildStyle ( PUCLASS_ONESHOT, PUSTYLE_BOXED ) ;
  file_selector->setChildBorderThickness ( PUCLASS_ONESHOT, 5 ) ;
  file_selector->setChildColour ( PUCLASS_SLIDER, 0, 0.5, 0.5, 0.5 ) ;
}

static void saveProject_cb ( puObject *ob )
{
  int w = 320, h = 270 ;
  if ( write_window )
  {
    glutSetWindow ( write_window ) ;
    glutSetWindowTitle( "Saving " );
  }
  else
  {
    write_window = glutCreateWindow ( "Saving " ) ;
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

  file_selector = new puaFileSelector ( 0, 0, w, h, 1, pguide_current_directory, "Pick .XML File to Save To" ) ;
  file_selector -> setCallback ( saveProject ) ;
  file_selector->setChildStyle ( PUCLASS_ONESHOT, PUSTYLE_BOXED ) ;
  file_selector->setChildBorderThickness ( PUCLASS_ONESHOT, 5 ) ;
  file_selector->setInitialValue(".xml");
  file_selector->setChildColour ( PUCLASS_SLIDER, 0, 0.5, 0.5, 0.5 ) ;
}

static void loadProject_ok_cb ( puObject *ob )
{
  puDeleteObject ( dialog ) ;  // Delete the dialog box
  dialog = (puDialogBox *)NULL ;

  window_wiper( ob );

  int w = 320, h = 270 ;
  if ( write_window )
  {
    glutSetWindow ( write_window ) ;
    glutSetWindowTitle( "Loading " );
  }
  else
  {
    write_window = glutCreateWindow ( "Loading " ) ;
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

  file_selector = new puaFileSelector ( 0, 0, w, h, 1, pguide_current_directory, "Pick P-Guide .XML File to load" ) ;
  file_selector -> setCallback ( loadProject ) ;
  file_selector->setChildStyle ( PUCLASS_ONESHOT, PUSTYLE_BOXED ) ;
  file_selector->setChildBorderThickness ( PUCLASS_ONESHOT, 5 ) ;
  file_selector->setInitialValue("*.xml");
  file_selector->setChildColour ( PUCLASS_SLIDER, 0, 0.5, 0.5, 0.5 ) ;
}

static void clear_ok_cb ( puObject *ob )
{
  window_wiper( ob );
  puDeleteObject ( dialog ) ;  // Delete the dialog box
  dialog = (puDialogBox *)NULL ;
}

static void cancel_cb ( puObject *ob )
{
  // Don't do whatever you were planning to do (clear the widget list or quit)
  // but do delete the dialog box.
  puDeleteObject ( dialog ) ;
  dialog = (puDialogBox *)NULL ;
}

static void clear_cb ( puObject *ob )
{
  if ( main_window_changed )
  {
    dialog = new puDialogBox ( 50, 300 ) ;
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

static void loadProject_cb ( puObject *ob )
{
  if ( main_window_changed )
  {
    dialog = new puDialogBox ( 50, 300 ) ;
    new puFrame ( 0, 0, 125, 70 ) ;
    puText *text = new puText ( 10, 40 ) ;
    text->setLabel ( "Are you sure?" ) ;
    puOneShot *ok = new puOneShot ( 10, 10, "Yes" ) ;
    ok->setCallback ( loadProject_ok_cb ) ;
    puOneShot *cancel = new puOneShot ( 60, 10, "Cancel" ) ;
    cancel->setCallback ( cancel_cb ) ;
    dialog->close () ;
    dialog->reveal () ;
  }
  else
    loadProject_ok_cb ( (puObject *)NULL ) ;
}

static void quit_ok_cb ( puObject *ob )
{
  exit ( 0 ) ;
}

static void quit_cb ( puObject *ob )
{
  if ( main_window_changed )
  {
    dialog = new puDialogBox ( 50, 300 ) ;
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

static void object_size_cb ( puObject *ob )
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

static void object_position_cb ( puObject *ob )
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

static void window_size_cb ( puObject *ob )
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

static void window_name_cb ( puObject *ob )
{
  extern int main_window ;

  glutSetWindow ( main_window ) ;
  strcpy ( main_window_name, ob->getStringValue () ) ;
  glutSetWindowTitle ( main_window_name ) ;
  glutSetWindow ( status_window ) ;
}

static void label_cb ( puObject *ob )
{
  if ( active_widget )
  {
    if (active_widget->label_text)
        delete active_widget->label_text ;
    active_widget->label_text = new char [ strlen ( ob->getStringValue () ) + 1 ] ;
    strcpy ( active_widget->label_text, ob->getStringValue () ) ;
    active_widget->obj->setLabel ( active_widget->label_text ) ;
  }
}

static void label_place_cb ( puObject *ob )
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

static void legend_cb ( puObject *ob )
{
  if ( active_widget )
  {
    if (active_widget->legend_text)
        delete active_widget->legend_text ;
    active_widget->legend_text = new char [ strlen ( ob->getStringValue () ) + 1 ] ;
    strcpy ( active_widget->legend_text, ob->getStringValue () ) ;
  }
}

static void legend_place_cb ( puObject *ob )
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

static void name_cb ( puObject *ob )
{
  if ( active_widget )
  {
    //strcpy ( active_widget->object_name, ob->getStringValue () ) ; 
    strcpy ( active_widget->object_name, object_name->getStringValue () ) ;/* Because ob is not _always_ the right puInput! */

    // Remove spaces from the widget name
    char *spc = strchr ( active_widget->object_name, ' ' ) ;
    while ( spc )
    {
      *spc = '_' ;
      spc = strchr ( active_widget->object_name, ' ' ) ;
    }
    /* Make sure the widget's name doesn't already exist. If so, prompt user */
    /* This really should be made to use the chk_dupname function from LoadSave.cxx */
    extern WidgetList *widgets ;
    WidgetList *wid = widgets ;
    while ( wid )
    {
        if ( (strcmp(active_widget->object_name,wid->object_name) == 0) && (active_widget != wid) )
        {
            /* Popup a dialog telling user something's bad!  */
            dialog = new puDialogBox ( 20, 20 ) ;
            new puFrame ( 0, 0, 460, 120 ) ;
            puText *text = new puText ( 80, 85 ) ;
            text->setLabel ( "ERROR: Name already used." ) ;
            text->setLabelFont(PUFONT_TIMES_ROMAN_24);
            puText *directions = new puText ( 90, 65 ) ;
            directions->setLabel ( "Please type in a new, unique name to continue." ) ;
            directions->setLabelFont(PUFONT_HELVETICA_12);
            puInput *newname = new puInput (20,40,440,60) ;
            newname->setValuator(active_widget->object_name);
            newname->setValidData("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-_012345679");
            puOneShot *ok = new puOneShot ( 200, 10, "Accept" ) ;
            ok->setCallback ( dupname_ok_cb ) ;
            dialog->close () ;
            dialog->reveal () ;
            break ;
        }
        wid = wid->next ;
    }
    ob->setValue(active_widget->object_name);
  }
}

static void dupname_ok_cb ( puObject *ob )
{
  /* The user's new name from the dialog has already been set to the object_name*/
  /* Close dialogbox and update main window ...*/
  puDeleteObject ( dialog ) ;
  dialog = (puDialogBox *)NULL ;
  newname = (puInput *)NULL ;
  setStatusWidgets(active_widget);
  /*but force a recheck afterwards to ensure it's an okay substitution. */
  name_cb(ob);
}

static void callback_cb ( puObject *ob )
{
  if ( active_widget ) active_widget->callbacks = ob->getIntegerValue () ;
}

static void reveal_all_cb ( puObject *ob )
{
  extern WidgetList *widgets ;
  WidgetList *wid = widgets ;
  while ( wid )
  {
    wid->visible = true ;
    wid = wid->next ;
  }
}

static void hide_all_cb ( puObject *ob )
{
  extern WidgetList *widgets ;
  WidgetList *wid = widgets ;
  while ( wid )
  {
    wid->visible = false ;
    wid = wid->next ;
  }
}

static void hide_all_layer_cb ( puObject *ob )
{
  extern WidgetList *widgets ;
  WidgetList *wid = widgets ;
  while ( wid )
    {
      if ( wid->layer == layer_to_act_on->getIntegerValue() )
        wid->visible = false ;
      wid = wid->next;
    }
}

static void reveal_all_layer_cb ( puObject *ob )
{
  extern WidgetList *widgets ;
  WidgetList *wid = widgets ;
  while ( wid )
    {
      if ( wid->layer == layer_to_act_on->getIntegerValue() )
        wid->visible = true ;
      wid = wid->next;
    }
}  

static void visible_cb ( puObject *ob )
{
  if ( active_widget ) active_widget->visible = ( ob->getIntegerValue () != 0 ) ;
}

static void layer_cb ( puObject *ob )
{
  if ( ob->getIntegerValue () < 0 ) ob->setValue ( 0 ) ;
  if ( active_widget )
  {
    active_widget->layer = ob->getIntegerValue () ;
    if ( max_layer <= active_widget->layer ) max_layer = active_widget->layer + 1 ;
  }
}

static void autolock_cb ( puObject *ob )
{
    if ( ob->getIntegerValue () == 1 )
        autolock = true ;
    else
        autolock = false ;
}

// Setup Menubar

static char      *file_submenu    [] = {  "Exit", "------------", "Export Code", "------------", "Save Project", "Load Project", "New Project", NULL } ;
static puCallback file_submenu_cb [] = { quit_cb, NULL, write_code_cb, NULL, saveProject_cb, loadProject_cb, clear_cb, NULL } ;

// Function to define the window

int define_status_window ()
{
  status_window = glutCreateWindow      ( "Status Window"  ) ;

  int total_screen_width = glutGet( GLUT_SCREEN_WIDTH ) ;

  glutPositionWindow    ( total_screen_width-520, 266 ) ;
  glutReshapeWindow     ( 500, 380 ) ;
  glutDisplayFunc       ( status_window_displayfn ) ;
  glutKeyboardFunc      ( status_window_keyfn     ) ;
  glutSpecialFunc       ( status_window_specialfn ) ;
  glutMotionFunc        ( status_window_motionfn  ) ;
  glutMouseFunc         ( status_window_mousefn   ) ;

  // Set up the widgets

  puGroup *status_group = new puGroup ( 0, 0 ) ;
  new puFrame ( 0, 0, 500, 380 ) ;

  menubar = new puMenuBar () ; 
  {
    menubar -> add_submenu ( "File", file_submenu, file_submenu_cb ) ;
  }
  menubar -> close() ;

  widget_section_frame = new puFrame ( 0, 1, 500, 250 ) ;
  widget_section_frame->setLegendFont ( PUFONT_HELVETICA_18 ) ;
  widget_section_frame->setLegend ( "Widget Options:" ) ;
  widget_section_frame->setLegendPlace ( PUPLACE_TOP_LEFT ) ;

  object_size_x = new puInput ( 380, 10, 430, 30 ) ;
  object_size_x->setLabel ( "Object Size:" ) ;
  object_size_x->setLabelPlace ( PUPLACE_LEFT ) ;
  object_size_x->setCallback ( object_size_cb ) ;
  object_size_x->setDownCallback ( object_size_cb ) ;

  object_size_y = new puInput ( 430, 10, 480, 30 ) ;
  object_size_y->setCallback ( object_size_cb ) ;
  object_size_y->setDownCallback ( object_size_cb ) ;

  object_position_x = new puInput ( 160, 10, 210, 30 ) ;
  object_position_x->setLabel ( "Object Position:" ) ;
  object_position_x->setLabelPlace ( PUPLACE_LEFT ) ;
  object_position_x->setCallback ( object_position_cb ) ;
  object_position_x->setDownCallback ( object_position_cb ) ;

  object_position_y = new puInput ( 210, 10, 260, 30 ) ;
  object_position_y->setCallback ( object_position_cb ) ;
  object_position_y->setDownCallback ( object_position_cb ) ;


  static char *vert_place_entries [] = { "Top", "Above", "Upper", "Center",
                                         "Lower", "Below", "Bottom", NULL } ;

  static char *horz_place_entries [] = { "Left", "Center", "Right", NULL } ;

  object_label = new puInput ( 65, 40, 260, 60 ) ;
  object_label->setLabel ( "Label:" ) ;
  object_label->setLabelPlace ( PUPLACE_LEFT ) ;
  object_label->setCallback ( label_cb ) ;
  object_label->setDownCallback ( label_cb ) ;

  object_vert_label_place = new puaComboBox ( 310, 40, 400, 60, vert_place_entries, FALSE ) ;
  object_vert_label_place->setLabel ( "Place" ) ;
  object_vert_label_place->setLabelPlace ( PUPLACE_LEFT ) ;
  object_vert_label_place->setCallback ( label_place_cb ) ;

  object_horz_label_place = new puaComboBox ( 400, 40, 490, 60, horz_place_entries, FALSE ) ;
  object_horz_label_place->setCallback ( label_place_cb ) ;

  object_legend = new puInput ( 65, 70, 260, 90 ) ;
  object_legend->setLabel ( "Legend:" ) ;
  object_legend->setLabelPlace ( PUPLACE_LEFT ) ;
  object_legend->setCallback ( legend_cb ) ;
  object_legend->setDownCallback ( legend_cb ) ;

  object_vert_legend_place = new puaComboBox ( 310, 70, 400, 90, vert_place_entries, FALSE ) ;
  object_vert_legend_place->setLabel ( "Place" ) ;
  object_vert_legend_place->setLabelPlace ( PUPLACE_LEFT ) ;
  object_vert_legend_place->setCallback ( legend_place_cb ) ;

  object_horz_legend_place = new puaComboBox ( 400, 70, 490, 90, horz_place_entries, FALSE ) ;
  object_horz_legend_place->setCallback ( legend_place_cb ) ;

  object_name = new puInput ( 65, 195, 260, 215 ) ;
  object_name->setLabel ( "Name:" ) ;
  object_name->setValidData("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-_0123456789");
  object_name->setLabelPlace ( PUPLACE_CENTERED_LEFT ) ;
  object_name->setCallback ( name_cb ) ;
  object_name->setDownCallback ( name_cb ) ;

  static char *callback_entries [] = { "Up", "Active", "Down", NULL } ;
  object_callbacks = new puButtonBox ( 120, 100, 260, 170, callback_entries, FALSE ) ;
  object_callbacks->setLabel ( "Widget Callbacks:" ) ;
  object_callbacks->setLabelPlace ( PUPLACE_TOP_LEFT ) ;
  object_callbacks->setCallback ( callback_cb ) ;

  static char *visible_entries [] = { "Visible", NULL } ;
  object_visible = new puButtonBox ( 10, 140, 115, 170, visible_entries, FALSE ) ;
  object_visible->setCallback ( visible_cb ) ;

  object_layer = new puaSpinBox ( 65, 100, 115, 120) ;
  object_layer->setMinValue(0.0);
  object_layer->setMaxValue(999.0);
  object_layer->setStepSize(1.00f);
  object_layer->setValue(0.0f);
  object_layer->setCallback ( layer_cb ) ;
  object_layer->setDownCallback ( layer_cb ) ;

  object_layer_text = new puText( 65, 112 );
  object_layer_text->setLabel ( "Layer:" ) ;
  object_layer_text->setLabelPlace ( PUPLACE_CENTERED_LEFT ) ;

  static char *autolock_entries [] = { "Autolock", NULL } ;
  autolock_toggle = new puButtonBox ( 370, 252, 490, 282, autolock_entries, FALSE ) ;
  autolock_toggle->setCallback ( autolock_cb ) ;

  layer_manipulation_frame = new puFrame (300, 120, 495, 215) ;
  layer_manipulation_frame->setLegend ( "Layer Manipulation" );
  layer_manipulation_frame->setLegendFont ( PUFONT_HELVETICA_12 ) ;
  layer_manipulation_frame->setLegendPlace ( PUPLACE_TOP_CENTERED );

  layer_to_act_on_frame = new puFrame ( 300, 145, 495, 190 );
  layer_to_act_on_frame->setLegend ( "Affect Single Layer:" ) ;
  layer_to_act_on_frame->setLegendPlace ( PUPLACE_TOP_CENTERED ) ;

  layer_all_frame = new puFrame ( 300, 121, 495, 148 );
      
  hide_all_objects = new puOneShot ( 360, 124, 420, 144 ) ;
  hide_all_objects->setLegend ( "Hide" ) ;
  hide_all_objects->setLabel ( "All:" ) ;
  hide_all_objects->setLabelPlace ( PUPLACE_CENTERED_LEFT ) ;  
  hide_all_objects->setCallback ( hide_all_cb ) ;  
  
  reveal_all_objects = new puOneShot ( 425, 124, 485, 144 ) ;
  reveal_all_objects->setLegend ( "Reveal" ) ;
  reveal_all_objects->setCallback ( reveal_all_cb ) ;

  layer_to_act_on = new puaSpinBox ( 310, 150, 355, 170 ) ;
  layer_to_act_on->setMinValue(0.0);
  layer_to_act_on->setMaxValue(999.0);
  layer_to_act_on->setStepSize(1.00f);
  layer_to_act_on->setValue(0.0f);

  hide_all_layer = new puOneShot ( 360, 150, 420, 170 ) ;
  hide_all_layer->setLegend ( "Hide" ) ;
  hide_all_layer->setCallback ( hide_all_layer_cb ) ;

  reveal_all_layer = new puOneShot ( 425, 150, 485, 170 ) ;
  reveal_all_layer->setLegend ( "Reveal" ) ;
  reveal_all_layer->setCallback ( reveal_all_layer_cb ) ;
  
  window_name = new puInput ( 130, 340, 430, 360 ) ;
  window_name->setValue( main_window_name );
  window_name->setLabel ( "Window Name:" ) ;
  window_name->setLabelPlace ( PUPLACE_CENTERED_LEFT ) ;
  window_name->setCallback ( window_name_cb ) ;
  window_name->setDownCallback ( window_name_cb ) ;

  window_size_x = new puInput ( 130, 310, 180, 330 ) ;
  window_size_x->setLabel ( "Window Size:" ) ;
  window_size_x->setLabelPlace ( PUPLACE_CENTERED_LEFT ) ;
  window_size_x->setCallback ( window_size_cb ) ;
  window_size_x->setDownCallback ( window_size_cb ) ;

  window_size_y = new puInput ( 180, 310, 230, 330 ) ;
  window_size_y->setCallback ( window_size_cb ) ;
  window_size_y->setDownCallback ( window_size_cb ) ;

  extern int main_window_x, main_window_y ;

  window_position_x = new puInput ( 130, 285, 180 , 305 ) ;
  window_position_x->setLabel ( "Position:" ) ;
  window_position_x->setLabelPlace ( PUPLACE_CENTERED_LEFT ) ;
  window_position_x->setValuator ( &main_window_x ) ;

  window_position_y = new puInput ( 180, 285, 230, 305 ) ;
  window_position_y->setValuator ( &main_window_y ) ;

  extern float main_window_color_r, main_window_color_g,
               main_window_color_b, main_window_color_a ;

  window_color_label = new puText (340, 320) ;
  window_color_label->setLabel ( "Color:" ) ;

  window_color_r = new puaSpinBox ( 270, 285, 320, 305 ) ;
  window_color_r->setMinValue(0.0);
  window_color_r->setMaxValue(1.0);
  window_color_r->setStepSize(0.05f);
  window_color_r->setLabel ( "Red" ) ;
  window_color_r->setLabelPlace ( PUPLACE_TOP_CENTERED ) ;
  window_color_r->setValuator ( &main_window_color_r ) ;
  window_color_r->setValue(1.0f);

  window_color_g = new puaSpinBox ( 320, 285, 370, 305 ) ;
  window_color_g->setMinValue(0.0);
  window_color_g->setMaxValue(1.0);
  window_color_g->setStepSize(0.05f);
  window_color_g->setLabel ( "Green" ) ;
  window_color_g->setLabelPlace ( PUPLACE_TOP_CENTERED ) ;
  window_color_g->setValuator ( &main_window_color_g ) ;
  window_color_g->setValue(1.0f);
  
  window_color_b = new puaSpinBox ( 370, 285, 420, 305 ) ;
  window_color_b->setMinValue(0.0);
  window_color_b->setMaxValue(1.0);
  window_color_b->setStepSize(0.05f);
  window_color_b->setLabel ( "Blue" ) ;
  window_color_b->setLabelPlace ( PUPLACE_TOP_CENTERED ) ;
  window_color_b->setValuator ( &main_window_color_b ) ;
  window_color_b->setValue(1.0f);

  window_color_a = new puaSpinBox ( 420, 285, 470, 305 ) ;
  window_color_a->setMinValue(0.0);
  window_color_a->setMaxValue(1.0);
  window_color_a->setStepSize(0.05f);
  window_color_a->setLabel ( "Alpha" ) ;
  window_color_a->setLabelPlace ( PUPLACE_TOP_CENTERED ) ;
  window_color_a->setValuator ( &main_window_color_a ) ;
  window_color_a->setValue(1.0f);

  status_group->close () ;

  return 0 ;
}

