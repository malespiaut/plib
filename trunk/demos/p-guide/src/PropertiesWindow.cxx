/*
     This file is part of P-GUIDE -
     PUI-based Graphical User Interface Designer.
     Copyright (C) 2002  John F. Fay

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

#include <plib/pu.h>

#include "WidgetList.h"

// From the Main Window:

extern int max_layer ;

extern WidgetList *active_widget ;
extern puObject *active_object ;

extern char pguide_current_directory [ PUSTRING_MAX ] ;

extern bool main_window_changed ;

// Status window parameters
int properties_window = 0 ;  // Window handle
extern int main_window ;

// Status window widgets
puGroup *properties_group ;

// Function to set the widgets from the active object


// GLUT Status Window Callbacks

static void properties_window_specialfn ( int key, int, int )
{
  puKeyboard ( key + PU_KEY_GLUT_SPECIAL_OFFSET, PU_DOWN ) ;

  glutPostRedisplay () ;
}

static void properties_window_keyfn ( unsigned char key, int, int )
{
  puKeyboard ( key, PU_DOWN ) ;

  glutPostRedisplay () ;
}

static void properties_window_motionfn ( int x, int y )
{
  puMouse ( x, y ) ;

  glutPostRedisplay () ;
}

static void properties_window_mousefn ( int button, int updown, int x, int y )
{
  puMouse ( button, updown, x, y ) ;

  glutPostRedisplay () ;
}

static void properties_window_displayfn ( void )
{
  glutSetWindow ( properties_window ) ;

  /* Clear the screen */

  glClearColor ( 0.1, 0.1, 0.1, 1.0 ) ;
  glClear      ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;

  puDisplay () ;

  /* Update GLUT */

  glutSwapBuffers   () ;
  glutPostRedisplay () ;
}

// PUI Callback Functions:

static void cb_properties_close (puObject *obj)
{
    /* Save the changes ... Needed? */
    if ( puActiveWidget() )
        puActiveWidget() -> invokeDownCallback () ; /* Just make sure the last widget saves ... */
    /* Delete the widgets */
    puDeleteObject( properties_group );
    /* Destroy the window */
    glutDestroyWindow( properties_window );
    properties_window = 0;
    glutSetWindow( main_window );
}

// Data setting standard callbacks

static void cb_items (puObject *obj)
{
    /*Set the Items*/
    if (active_widget)
    {
        delete active_widget->items;
        active_widget->items = new char [strlen(obj->getStringValue())+1] ;
        strcpy(active_widget->items, obj->getStringValue());
    }
        
}

static void cb_allowed (puObject *obj)
{
    /*Set the Allowed string*/
    if (active_widget)
    {
        delete active_widget->allowed;
        active_widget->allowed = new char [strlen(obj->getStringValue())+1] ;
        strcpy(active_widget->allowed, obj->getStringValue());
    }
}
static void cb_int1 (puObject *obj)
{
    /*Set the first integer storage value */
    if (active_widget)
        active_widget->intval1 = obj->getIntegerValue();
}
static void cb_int2 (puObject *obj)
{
    /*Set the second integer storage value */
    if (active_widget)
        active_widget->intval2 = obj->getIntegerValue();
}
static void cb_bool1 (puObject *obj)
{
        /*Set the first boolean storage value */
    if (active_widget)
    {
        if (obj->getIntegerValue() == 1)
            active_widget->boolval1 =  true ;
        else
            active_widget->boolval1 =  false ;
    }
}
static void cb_bool2 (puObject *obj)
{
            /*Set the second boolean storage value */
    if (active_widget)
    {
        if (obj->getIntegerValue() == 1)
            active_widget->boolval2 =  true ;
        else
            active_widget->boolval2 =  false ;
    }
}
static void cb_bool3 (puObject *obj)
{
            /*Set the second boolean storage value */
    if (active_widget)
    {
        if (obj->getIntegerValue() == 1)
            active_widget->boolval3 =  true ;
        else
            active_widget->boolval3 =  false ;
    }
}
static void cb_float1 (puObject *obj)
{
    /*Set the first floating storage value */
    if (active_widget)
        active_widget->floatval1 = obj->getFloatValue();
}
static void cb_float2 (puObject *obj)
{
    /*Set the first floating storage value */
    if (active_widget)
        active_widget->floatval2 = obj->getFloatValue();
}
static void cb_float3 (puObject *obj)
{
    /*Set the first floating storage value */
    if (active_widget)
        active_widget->floatval3 = obj->getFloatValue();
}
static void cb_float4 (puObject *obj)
{
    /*Set the first floating storage value */
    if (active_widget)
        active_widget->floatval4 = obj->getFloatValue();
}
static void cb_float5 (puObject *obj)
{
    /*Set the first floating storage value */
    if (active_widget)
        active_widget->floatval5 = obj->getFloatValue();
}
static void cb_float6 (puObject *obj)
{
    /*Set the first floating storage value */
    if (active_widget)
        active_widget->floatval6 = obj->getFloatValue();
}

// Function to define the window

int define_properties_window ()
{
  properties_window = glutCreateWindow      ( "Properties" ) ;
  
  glutPositionWindow    ( 400, 200 ) ;
  glutReshapeWindow     ( 500, 250 ) ;
  glutDisplayFunc       ( properties_window_displayfn ) ;
  glutKeyboardFunc      ( properties_window_keyfn     ) ;
  glutSpecialFunc       ( properties_window_specialfn ) ;
  glutMotionFunc        ( properties_window_motionfn  ) ;
  glutMouseFunc         ( properties_window_mousefn   ) ;

  // Set up the widgets

  properties_group = new puGroup ( 0, 0 ) ;
  new puFrame ( 0, 0, 500, 250 ) ;

  puText *properties_instructions = new puText ( 250, 230 );
  properties_instructions->setLabelPlace(PUPLACE_TOP_CENTERED);
  properties_instructions->setLabel("Here are the extended options for your");

  puText *properties_typelabel = new puText ( 250, 215 );
  properties_typelabel->setLabelPlace(PUPLACE_TOP_CENTERED);
  properties_typelabel->setLabel(active_widget->object_type_name);

  puOneShot *properties_close = new puOneShot ( 370, 10, 490, 30 ) ;
  properties_close->setLegend("Accept");
  properties_close->setCallback(cb_properties_close);

  /* puStyle stuff here*/

  /* Now customize the display based on the widget currently selected */
  if (active_widget->object_type == PUCLASS_FRAME)
  {
    puText *properties_nooptions = new puText (25, 100 );
    properties_nooptions->setLabelPlace(PUPLACE_CENTERED_RIGHT);
    properties_nooptions->setLabel("There are no options for a puFrame.");
  } if (active_widget->object_type == PUCLASS_TEXT)
  {
      /* Add in font properties? */
    puText *properties_nooptions = new puText (25, 100 );
    properties_nooptions->setLabelPlace(PUPLACE_CENTERED_RIGHT);
    properties_nooptions->setLabel("There are no options for puText.");
  } if (active_widget->object_type == PUCLASS_BUTTON)
  {
    puText *properties_nooptions = new puText (25, 100 );
    properties_nooptions->setLabelPlace(PUPLACE_CENTERED_RIGHT);
    properties_nooptions->setLabel("There are no options for puButton.");
  } if (active_widget->object_type == PUCLASS_ONESHOT)
  {
    puText *properties_nooptions = new puText (25, 100 );
    properties_nooptions->setLabelPlace(PUPLACE_CENTERED_RIGHT);
    properties_nooptions->setLabel("There are no options for puOneShot.");
  } if ( (active_widget->object_type == PUCLASS_POPUPMENU)  || 
         (active_widget->object_type == PUCLASS_MENUBAR)    ||
         (active_widget->object_type == PUCLASS_VERTMENU)   ||
         (active_widget->object_type == PUCLASS_LISTBOX)    ||
         (active_widget->object_type == PUCLASS_COMBOBOX)   ||
         (active_widget->object_type == PUCLASS_SELECTBOX)  ||
         (active_widget->object_type == PUCLASS_BUTTONBOX)  )
  {
  /* List */
    puLargeInput *properties_list_items = new puLargeInput(10,10,300,195,0,5);
    properties_list_items->setLabelPlace(PUPLACE_TOP_CENTERED);
    properties_list_items->setLabel("Enter items, each on a new line.");
    properties_list_items->setValidData("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-_012345679 :;.,\n");
    properties_list_items->setCallback(cb_items);
    properties_list_items->setDownCallback(cb_items);
    properties_list_items->setValue(active_widget->items);

    puText *properties_list_note = new puText(400,190);
    properties_list_note->setLabelPlace(PUPLACE_TOP_CENTERED);
    properties_list_note->setLabel("Each line will become");
 
    puText *properties_list_note2 = new puText(400,175);
    properties_list_note2->setLabelPlace(PUPLACE_TOP_CENTERED);
    properties_list_note2->setLabel("a selection option.");

  } if (active_widget->object_type == PUCLASS_POPUPMENU)
  {
    /* Allow a list of menu items, and remember to point out these are hidden when created, and must be reveal()ed. */
    /* NOT CURRENTLY IMPLEMENTED */
    puText *properties_popupmenu_warning = new puText(400,150);
    properties_popupmenu_warning->setLabelPlace(PUPLACE_TOP_CENTERED);
    properties_popupmenu_warning->setLabel("WARNING: Popup Menus are hidden by \
                                            default when created. You must use \
                                            reveal() to make them appear.");

  } if (active_widget->object_type == PUCLASS_MENUBAR)
  {
    /* Allow a list of menu items */

  } if (active_widget->object_type == PUCLASS_VERTMENU)
  {
    /* Allow a list of menu items */

  } if (active_widget->object_type == PUCLASS_LISTBOX)
  {
    /* Allow a list of items */

  } if (active_widget->object_type == PUCLASS_COMBOBOX)
  {
    /* List and a setCurrentItem, and if editable  */
    puSpinBox *properties_combobox_currentitem = new puSpinBox(320,135,480,155);
    properties_combobox_currentitem->setLabel("Initial Selection:");
    properties_combobox_currentitem->setLabelPlace(PUPLACE_TOP_CENTERED);
    properties_combobox_currentitem->setMinValue(0.0f);
    properties_combobox_currentitem->setMaxValue(30.0f);
    properties_combobox_currentitem->setCallback(cb_int1);
    properties_combobox_currentitem->setValue(active_widget->intval1);

    static char *properties_combobox_editable_callback_entries [] = { "No", "Yes", NULL } ;
    puButtonBox *properties_combobox_editable = new puButtonBox(320, 50, 480, 120, properties_combobox_editable_callback_entries, TRUE);
    properties_combobox_editable->setLabel("Editable?");
    properties_combobox_editable->setLabelPlace(PUPLACE_TOP_CENTERED);
    properties_combobox_editable->setCallback(cb_bool1);
    properties_combobox_editable->setValue(active_widget->boolval1);

  } if (active_widget->object_type == PUCLASS_SELECTBOX)
  {
    /* List and a setCurrentItem  */
    puSpinBox *properties_selectbox_currentitem = new puSpinBox(320,135,480,155);
    properties_selectbox_currentitem->setLabel("Initial Selection:");
    properties_selectbox_currentitem->setLabelPlace(PUPLACE_TOP_CENTERED);
    properties_selectbox_currentitem->setMinValue(0.0f);
    properties_selectbox_currentitem->setMaxValue(30.0f);
    properties_selectbox_currentitem->setCallback(cb_int1);
    properties_selectbox_currentitem->setValue(active_widget->intval1);

  } if (active_widget->object_type == PUCLASS_BUTTONBOX)
  {
    /* Allow a list of menu items, and if multiple can be selected at once */
    puText *properties_buttonbox_label = new puText(400,135);
    properties_buttonbox_label->setLabelPlace(PUPLACE_TOP_CENTERED); 
    properties_buttonbox_label->setLabel("Allow multiple");

    static char *properties_buttonbox_multiselect_callback_entries [] = { "No", "Yes", NULL } ;
    puButtonBox *properties_buttonbox_multiselect = new puButtonBox(320, 50, 480, 120, properties_buttonbox_multiselect_callback_entries, TRUE);
    properties_buttonbox_multiselect->setLabel("selections?");
    properties_buttonbox_multiselect->setLabelPlace(PUPLACE_TOP_CENTERED);
    properties_buttonbox_multiselect->setCallback(cb_bool1);
    properties_buttonbox_multiselect->setValue(active_widget->boolval1);

  } if ( (active_widget->object_type == PUCLASS_SLIDER )       || 
              (active_widget->object_type == PUCLASS_BISLIDER )     || 
              (active_widget->object_type == PUCLASS_TRISLIDER )    || 
              (active_widget->object_type == PUCLASS_DIAL )         || 
              (active_widget->object_type == PUCLASS_SPINBOX )      || 
              (active_widget->object_type == PUCLASS_SCROLLBAR )    )
  {
    /* int minx, int miny, int sz, {TRUE|FALSE}, int width */
    puSpinBox *properties_range_setmaxvalue = new puSpinBox(30,150,110,170);
    properties_range_setmaxvalue->setLabel("Maximum Value:");
    properties_range_setmaxvalue->setLabelPlace(PUPLACE_TOP_CENTERED);
    properties_range_setmaxvalue->setMinValue(-5000.0f);
    properties_range_setmaxvalue->setMaxValue(5000.0f);
    properties_range_setmaxvalue->setCallback(cb_float1);
    properties_range_setmaxvalue->setValue(active_widget->floatval1);

    puSpinBox *properties_range_setminvalue = new puSpinBox(230,150,310,170);
    properties_range_setminvalue->setLabel("Minimum Value:");
    properties_range_setminvalue->setLabelPlace(PUPLACE_TOP_CENTERED);
    properties_range_setminvalue->setMinValue(-5000.0f);
    properties_range_setminvalue->setMaxValue(5000.0f);
    properties_range_setminvalue->setCallback(cb_float2);
    properties_range_setminvalue->setValue(active_widget->floatval2);

    puSpinBox *properties_range_setstepvalue = new puSpinBox(400,150,480,170);
    properties_range_setstepvalue->setLabel("Step Size:");
    properties_range_setstepvalue->setLabelPlace(PUPLACE_TOP_CENTERED);
    properties_range_setstepvalue->setMinValue(0.0f);
    properties_range_setstepvalue->setMaxValue(5000.0f);
    properties_range_setstepvalue->setCallback(cb_float3);
    properties_range_setstepvalue->setValue(active_widget->floatval3);

    static char *properties_range_cbmode_callback_entries [] = { "On Click", "Always", NULL } ;
    puButtonBox *properties_range_cbmode = new puButtonBox(20, 80, 170, 130, properties_range_cbmode_callback_entries, TRUE);
    properties_range_cbmode->setLabel("Callback Mode:");
    properties_range_cbmode->setLabelPlace(PUPLACE_TOP_CENTERED);
    properties_range_cbmode->setCallback(cb_bool1);
    properties_range_cbmode->setValue(active_widget->boolval1);
   
  } if ( (active_widget->object_type == PUCLASS_SLIDER )       || 
         (active_widget->object_type == PUCLASS_BISLIDER )     || 
         (active_widget->object_type == PUCLASS_TRISLIDER )     )  
  {
    static char *properties_range_vertical_callback_entries [] = { "Horizontal", "Vertical", NULL } ;
    puButtonBox *properties_range_vertical = new puButtonBox(20, 10, 170, 60, properties_range_vertical_callback_entries, TRUE);
    properties_range_vertical->setLabel("Orientation:");
    properties_range_vertical->setLabelPlace(PUPLACE_TOP_CENTERED);
    properties_range_vertical->setValue(active_widget->boolval2);
    properties_range_vertical->setCallback(cb_bool2);

  } if (active_widget->object_type == PUCLASS_SLIDER)
  {
    puSpinBox *properties_slider_value = new puSpinBox(230,110,310,130);
    properties_slider_value->setLabel("Starting Value:");
    properties_slider_value->setLabelPlace(PUPLACE_TOP_CENTERED);
    properties_slider_value->setMinValue(-5000.0f);
    properties_slider_value->setMaxValue(5000.0f);
    properties_slider_value->setValue(active_widget->floatval4);
    properties_slider_value->setStepSize(0.1f);
    properties_slider_value->setCallback(cb_float4);

  } if (active_widget->object_type == PUCLASS_BISLIDER)
  {
    puSpinBox *properties_bislider_topvalue = new puSpinBox(230,110,310,130);
    properties_bislider_topvalue->setLabel("Top Slider Value:");
    properties_bislider_topvalue->setLabelPlace(PUPLACE_TOP_CENTERED);
    properties_bislider_topvalue->setMinValue(-5000.0f);
    properties_bislider_topvalue->setMaxValue(5000.0f);
    properties_bislider_topvalue->setValue(active_widget->floatval4);
    properties_bislider_topvalue->setStepSize(0.1f);
    properties_bislider_topvalue->setCallback(cb_float4);

    puSpinBox *properties_bislider_botvalue = new puSpinBox(230,40,310,60);
    properties_bislider_botvalue->setLabel("Bottom Slider Value:");
    properties_bislider_botvalue->setLabelPlace(PUPLACE_TOP_CENTERED);
    properties_bislider_botvalue->setMinValue(-5000.0f);
    properties_bislider_botvalue->setMaxValue(5000.0f);
    properties_bislider_botvalue->setValue(active_widget->floatval5);
    properties_bislider_botvalue->setStepSize(0.1f);
    properties_bislider_botvalue->setCallback(cb_float5);

  } if (active_widget->object_type == PUCLASS_TRISLIDER)
  {
    puSpinBox *properties_trislider_topvalue = new puSpinBox(230,110,310,130);
    properties_trislider_topvalue->setLabel("Top Slider Value:");
    properties_trislider_topvalue->setLabelPlace(PUPLACE_TOP_CENTERED);
    properties_trislider_topvalue->setMinValue(-5000.0f);
    properties_trislider_topvalue->setMaxValue(5000.0f);
    properties_trislider_topvalue->setValue(active_widget->floatval4);
    properties_trislider_topvalue->setStepSize(0.1f);
    properties_trislider_topvalue->setCallback(cb_float4);

    puSpinBox *properties_trislider_centvalue = new puSpinBox(230,60,310,80);
    properties_trislider_centvalue->setLabel("Center Slider Value:");
    properties_trislider_centvalue->setLabelPlace(PUPLACE_TOP_CENTERED);
    properties_trislider_centvalue->setMinValue(-5000.0f);
    properties_trislider_centvalue->setMaxValue(5000.0f);
    properties_trislider_centvalue->setValue(active_widget->floatval6);
    properties_trislider_centvalue->setStepSize(0.1f);
    properties_trislider_centvalue->setCallback(cb_float6);

    puSpinBox *properties_trislider_botvalue = new puSpinBox(230,10,310,30);
    properties_trislider_botvalue->setLabel("Bottom Slider Value:");
    properties_trislider_botvalue->setLabelPlace(PUPLACE_TOP_CENTERED);
    properties_trislider_botvalue->setMinValue(-5000.0f);
    properties_trislider_botvalue->setMaxValue(5000.0f);
    properties_trislider_botvalue->setValue(active_widget->floatval5);
    properties_trislider_botvalue->setStepSize(0.1f);
    properties_trislider_botvalue->setCallback(cb_float5);

    static char *properties_trislider_lock_callback_entries [] = { "No", "Yes", NULL } ;
    puButtonBox *properties_trislider_lock = new puButtonBox(380, 80, 480, 130, properties_trislider_lock_callback_entries, TRUE);
    properties_trislider_lock->setLabel("Lock End Sliders:");
    properties_trislider_lock->setLabelPlace(PUPLACE_TOP_CENTERED);
    properties_trislider_lock->setValue(active_widget->boolval3);
    properties_trislider_lock->setCallback(cb_bool3);

  } if (active_widget->object_type == PUCLASS_DIAL)
  {
    /* Wrap Mode */
    static char *properties_dial_wrap_callback_entries [] = { "No", "Yes", NULL } ;
    puButtonBox *properties_dial_wrap = new puButtonBox(20, 10, 170, 60, properties_dial_wrap_callback_entries, TRUE);
    properties_dial_wrap->setLabel("Allow Dial Wrapping:");
    properties_dial_wrap->setLabelPlace(PUPLACE_TOP_CENTERED);
    properties_dial_wrap->setValue(active_widget->boolval2);
    properties_dial_wrap->setCallback(cb_bool2);

  } if (active_widget->object_type == PUCLASS_SPINBOX)
  {
    /* Arrow position and height */
    static char *properties_spinbox_arrow_callback_entries [] = { "Left", "Right", NULL } ;
    puButtonBox *properties_spinbox_arrow = new puButtonBox(20, 10, 170, 60, properties_spinbox_arrow_callback_entries, TRUE);
    properties_spinbox_arrow->setLabel("Arrow Position");
    properties_spinbox_arrow->setLabelPlace(PUPLACE_TOP_CENTERED);
    properties_spinbox_arrow->setValue(active_widget->boolval2);
    properties_spinbox_arrow->setCallback(cb_bool2);

    puSpinBox *properties_spinbox_arrowheight = new puSpinBox(230,110,310,130);
    properties_spinbox_arrowheight->setLabel("Arrow Height:");
    properties_spinbox_arrowheight->setLabelPlace(PUPLACE_TOP_CENTERED);
    properties_spinbox_arrowheight->setMinValue(0.0f);
    properties_spinbox_arrowheight->setMaxValue(20.0f);
    properties_spinbox_arrowheight->setValue(active_widget->floatval4);
    properties_spinbox_arrowheight->setStepSize(0.05f);
    properties_spinbox_arrowheight->setCallback(cb_float4);

    puText *properties_spinbox_arrowextra = new puText ( 310, 90 );
    properties_spinbox_arrowextra->setLabelPlace(PUPLACE_TOP_CENTERED);
    properties_spinbox_arrowextra->setLabel("(as porportion of spinbox height)");

  } if (active_widget->object_type == PUCLASS_SCROLLBAR)
  {
    /* Not yet implemented as a class */

  } if (active_widget->object_type == PUCLASS_DIALOGBOX)
  {
    /* Not yet implemented as a class */

  } if (active_widget->object_type == PUCLASS_ARROW)
  {
    static char *properties_arrow_arrowtype_options [] = { "Up", "Down", "Left", "Right",
                                                   "Double-Up", "Double-Down", "Double-Left", 
                                                   "Double-Right", NULL } ;

    puComboBox *properties_arrow_arrowtype = new puComboBox ( 120,10,310,30, properties_arrow_arrowtype_options, FALSE ) ;
    properties_arrow_arrowtype->setLabel ( "Arrow Type:" ) ;
    properties_arrow_arrowtype->setLabelPlace ( PUPLACE_LOWER_LEFT ) ;
    properties_arrow_arrowtype->setCallback(cb_items);

    int num = 0;

    if (active_widget->items)
    {
        if (strstr(active_widget->items,"Double-Up"))
            num = 4;
        else if (strstr(active_widget->items,"Double-Down"))
            num = 5;
        else if (strstr(active_widget->items,"Double-Left"))
            num = 6;
        else if (strstr(active_widget->items,"Double-Right"))
            num = 7;
        else if (strstr(active_widget->items,"Up"))
            num = 0;
        else if (strstr(active_widget->items,"Down"))
            num = 1;
        else if (strstr(active_widget->items,"Left"))
            num = 2;
        else if (strstr(active_widget->items,"Right"))
            num = 3;
    }

    properties_arrow_arrowtype->setCurrentItem(num);

  } if ( (active_widget->object_type == PUCLASS_INPUT ) || (active_widget->object_type == PUCLASS_LARGEINPUT ) )
  {
    /*int minx, int miny, int maxx, int maxy */
    /* Disabled, and Valid Data list */
    puInput *properties_input_validdata = new puInput(20, 150, 260, 170);
    properties_input_validdata->setLabel("Allowed Characters for Input:");
    properties_input_validdata->setLabelPlace(PUPLACE_TOP_CENTERED);
    properties_input_validdata->setCallback(cb_allowed);
    properties_input_validdata->setDownCallback(cb_allowed);
    properties_input_validdata->setValue(active_widget->allowed);
    
    static char *properties_input_enabled_callback_entries [] = { "Yes", NULL } ;
    puButtonBox *properties_input_enabled = new puButtonBox(300, 140, 450, 170, properties_input_enabled_callback_entries, FALSE);
    properties_input_enabled->setLabel("Input Allowed? (enabled)");
    properties_input_enabled->setLabelPlace(PUPLACE_TOP_CENTERED);
    properties_input_enabled->setValue(active_widget->boolval1);
    properties_input_enabled->setCallback(cb_bool1);

  } if (active_widget->object_type == PUCLASS_LARGEINPUT)
  {
    /*Arrows and Slider Size*/
    static char *properties_largeinput_arrows_callback_entries [] = { "None", "Normal", "Normal and Fast", NULL } ;
    puButtonBox *properties_largeinput_arrows = new puButtonBox(20, 10, 200, 130, properties_largeinput_arrows_callback_entries, TRUE);
    properties_largeinput_arrows->setLabel("Which Arrow Buttons:");
    properties_largeinput_arrows->setLabelPlace(PUPLACE_TOP_CENTERED);
    properties_largeinput_arrows->setValue(active_widget->intval1);
    properties_largeinput_arrows->setCallback(cb_int1);

    puSpinBox *properties_largeinput_sliderwidth = new puSpinBox(300,90,400,110);
    properties_largeinput_sliderwidth->setLabel("Slider Width:");
    properties_largeinput_sliderwidth->setLabelPlace(PUPLACE_TOP_CENTERED);
    properties_largeinput_sliderwidth->setMinValue(0.0f);
    properties_largeinput_sliderwidth->setMaxValue(150.0f);
    properties_largeinput_sliderwidth->setValue(active_widget->intval2);
    properties_largeinput_sliderwidth->setStepSize(1.0f);
    properties_largeinput_sliderwidth->setCallback(cb_int2);

  }
  
  properties_group->close () ;

  return 0 ;
}



