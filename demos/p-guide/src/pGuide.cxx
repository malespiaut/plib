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

#ifdef VERSION
#undef VERSION
#endif

#define VERSION = "0.1a"

// Program to allow the user to build a PUI Graphical User Interface

#include "WidgetList.h"

WidgetList *widgets = (WidgetList *)NULL ;
int max_layer = 1 ;

puObject *active_object = (puObject *)NULL ;
WidgetList *active_widget = (WidgetList *)NULL ;
static short activity_flag = 0 ;  // 0 - inactive; 1 - moving; 2 - resizing xmin, 3 - resizing ymin, 4 - resizing xmax, 5 - resizing ymax
static int resize_symmetric = 1 ;
static int resize_corner = 0 ;

static int mouse_x_position_in_object = 0 ;
static int mouse_y_position_in_object = 0 ;

static int pguide_last_buttons = 0 ; // Because puMouse isn't called, we have to do our own last_buttons checking

// Main window parameters
int main_window = 0 ;  // Main window handle
int main_window_width  = 600 ;
int main_window_height = 600 ;
char main_window_name [ PUSTRING_MAX ] ;
int main_window_x = 0, main_window_y = 0 ;

float main_window_color_r = 1.0, main_window_color_g = 1.0,
      main_window_color_b = 1.0, main_window_color_a = 1.0 ;

bool main_window_changed = false ;
static bool done_first_setup = false ;
bool currently_loading = false ;

static int ctrl_key_down = 0 ;

static int mouse_x = 0 ;
static int mouse_y = 0 ;

char pguide_current_directory [ PUSTRING_MAX ] ;

// Widget count
int widget_number = 0 ;

// Upon creation of new widgets, should they automatically be made "locked"?
bool autolock = false ;

// Properties popup
static puPopupMenu *context_menu;

// From the status window:
extern void setStatusWidgets ( WidgetList *wid ) ;

// From the properties window:
extern int properties_window;

// Definitions
#define RESIZING_BORDER_WIDTH   5
#define RESIZING_CORNER_BORDER_WIDTH   4
  /** 4 seems to be the best number for this define... this makes sure  *
   *  that the default widget size allows you to still resize along the *
   *  X axis without difficulty -  23 Jan 03 JCJ                       **/


// Properties Callback

static void cb_edit_properties ( puObject *ob )
{
    if (properties_window)
    {
        extern puGroup *properties_group; 
        /* Delete the widgets */
        puDeleteObject( properties_group );
        glutDestroyWindow( properties_window );
        properties_window = 0;
        glutSetWindow( main_window );
    }
    extern int define_properties_window () ;
    // Open the properties menu
    define_properties_window();
}

static void cb_lock_toggle ( puObject *ob )
{
  WidgetList *wid = widgets ;
  while ( wid )
  {
    if ( wid->obj == active_object )
    {
        if (wid->locked == false)
            wid->locked = true ;
        else
            wid->locked = false ;
    }
    wid = wid->next ;
  }
}

static void cb_popup_delete ( puObject *ob )
{
  WidgetList *wid = widgets ;
  WidgetList *prv = (WidgetList *)NULL ;
  while ( wid )
  {
    if ( wid->obj == active_object )
    {
      if ( prv )  // Remove the widget from the linked list
        prv->next = wid->next ;
      else
        widgets = wid->next ;

      delete wid->object_type_name ;  // Delete the widget
      delete wid->legend_text ;
      delete wid->label_text ;
      delete wid->obj ;
      delete wid->allowed ;
      delete wid->items ;
      delete wid ;
      active_widget = (WidgetList *)NULL ;
      active_object = (puObject *)NULL ;
      wid = (WidgetList *)NULL ;
      if (properties_window)
      {
        extern puGroup *properties_group; 
        /* Delete the widgets */
        puDeleteObject( properties_group );
        glutDestroyWindow( properties_window );
        properties_window = 0;
        glutSetWindow( main_window );
      }
    }
    else
    {
      prv = wid ;
      wid = wid->next ;
    }
  }
}


// GLUT Main Window Callbacks

static void process_key ( int key )
{
  extern int selected_object_type ;
  extern puButton *active_button ;

  ctrl_key_down = ( glutGetModifiers () & GLUT_ACTIVE_CTRL ) ;
#define NUDGE_DISTANCE    1
#define LARGE_NUDGE_DISTANCE    10

  if ( active_object )  // Active object; check the keys
  {
    main_window_changed = true ;

    int xo, yo ;
    active_object->getPosition ( &xo, &yo ) ;
    int dist = ctrl_key_down ? LARGE_NUDGE_DISTANCE : NUDGE_DISTANCE ;

    if ( key == GLUT_KEY_LEFT )
      active_object->setPosition ( xo-dist, yo ) ;
    else if ( key == GLUT_KEY_RIGHT )
      active_object->setPosition ( xo+dist, yo ) ;
    else if ( key == GLUT_KEY_UP )
      active_object->setPosition ( xo, yo+dist ) ;
    else if ( key == GLUT_KEY_DOWN )
      active_object->setPosition ( xo, yo-dist ) ;
    else if ( ( key == 127 ) || ( key == 8 ) )  // Delete or Backspace
    {
      WidgetList *wid = widgets ;
      WidgetList *prv = (WidgetList *)NULL ;
      while ( wid )
      {
        if ( wid->obj == active_object )
        {
          if ( prv )  // Remove the widget from the linked list
            prv->next = wid->next ;
          else
            widgets = wid->next ;

          delete wid->object_type_name ;  // Delete the widget
          delete wid->legend_text ;
          delete wid->label_text ;
          delete wid->obj ;
          delete wid->allowed ;
          delete wid->items ;
          delete wid ;
          active_widget = (WidgetList *)NULL ;
          active_object = (puObject *)NULL ;
          wid = (WidgetList *)NULL ;
        }
        else
        {
          prv = wid ;
          wid = wid->next ;
        }
      }
    }
    else if ( key == 27 )  // Escape key, deactivate the object
    {
      active_widget = (WidgetList *)NULL ;
      active_object = (puObject *)NULL ;
    }

    setStatusWidgets ( active_widget ) ;
  }

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

static void main_window_specialfn ( int key, int, int )
{
  process_key ( key ) ;
}

static void main_window_keyfn ( unsigned char key, int, int )
{
  process_key ( key ) ;
}

static void main_window_motionfn ( int x, int yy )
{
  int y = main_window_height - yy ;

  mouse_x = x ;
  mouse_y = y ;
      
  // Dragging the mouse:  If there is an active object, slide it around
  if ( ( active_object ) && ( pguide_last_buttons != 4 ) )
  {
    WidgetList *wid = widgets ;
    while ( wid )
    {
        if ( wid->obj == active_object )
        {
            if (wid->locked == false)
            {
                extern puInput *object_position_x ;
                extern puInput *object_position_y ;
                extern puInput *object_size_x ;
                extern puInput *object_size_y ;

                main_window_changed = true ;

                int dist ;
                int xo, yo ;
                active_object->getPosition ( &xo, &yo ) ;
                int w, h ;
                active_object->getSize ( &w, &h ) ;

                if (resize_corner == 1)
                {
                    switch ( activity_flag )
                    {
                    case 2 : // Resize top-right
                    case 3 :
                        activity_flag = 6;
                        break ;

                    case 4 : // Resize bottom_left
                    case 5 :
                        activity_flag = 7;
                        break ;
                    }
                }

                switch ( activity_flag )
                {
                case 1 :  // Moving the object
                  active_object->setPosition ( x - mouse_x_position_in_object,
                                               y - mouse_y_position_in_object ) ;
                  break ;

                case 2 :  // Resizing on x-min
                  dist = x - xo ;
                  if (w - dist < 5) break ;
                  if ( resize_symmetric * dist > w ) dist = w / resize_symmetric - 1 ;
                  active_object->setPosition ( x, yo ) ;
                  active_object->setSize ( w - resize_symmetric * dist, h ) ;
                  break ;

                case 3 :  // Resizing on y-min
                  dist = y - yo ;
                  if (h - dist < 5) break ;
                  if ( resize_symmetric * dist > h ) dist = h / resize_symmetric - 1 ;
                  active_object->setPosition ( xo, y ) ;
                  active_object->setSize ( w, h - resize_symmetric * dist ) ;
                  break ;

                case 4 :  // Resizing on x-max
                  dist = x - xo - w ;
                  if (w + dist < 5) break ;
                  if ( resize_symmetric * dist < -w ) dist = 1 - w / resize_symmetric ;
                  active_object->setSize ( w + resize_symmetric * dist, h ) ;
                  if ( resize_symmetric == 2 ) active_object->setPosition ( xo - dist, yo ) ;
                  break ;

                case 5 :  // Resizing on y-max
                  dist = y - yo - h ;
                  if (h + dist < 5) break ;
                  if ( resize_symmetric * dist < -h ) dist = 1 - h / resize_symmetric ;
                  active_object->setSize ( w, h + resize_symmetric * dist ) ;
                  if ( resize_symmetric == 2 ) active_object->setPosition ( xo, yo - dist ) ;
                  break ;

                case 6 :  // Resizing bottom-left
                  if ( yo - y > xo - x )
                    dist = yo - y;
                  else
                    dist = xo - x;
                  if ( (w + dist < 5) || (h + dist < 5) ) break ;
                  active_object->setPosition ( xo - dist, yo - dist ) ;
                  active_object->setSize ( w + dist, h + dist ) ;
                  break ;

                case 7 :  // Resizing top-right
                  if (y-yo-h>x-xo-w)
                    dist = y - yo - h ;
                  else
                    dist = x - xo - w ;
                  if ( (w + dist < 5) || (h + dist < 5) ) break ;
                  active_object->setSize ( w + dist, h + dist ) ;
                  break ;

                case 8 :  // Resizing top-left
                  if (y-yo-h>x-xo-w)
                    dist = y - yo - h ;
                  else
                    dist = xo - x ;
                  if ( (w + dist < 5) || (h + dist < 5) ) break ;
		  active_object->setPosition ( xo - dist, yo ) ;
                  active_object->setSize ( w + dist, h + dist ) ;
                  break ;

                case 9 :  // Resizing bottom-right
                  if (y-yo-h>x-xo-w)
                    dist = yo - y ;
                  else
                    dist = x - xo - w ;
                  if ( (w + dist < 5) || (h + dist < 5) ) break ;
                  active_object->setPosition ( xo , yo - dist ) ;
                  active_object->setSize ( w + dist, h + dist ) ;
                  break ;
                }

                int a, b ;
                active_object->getPosition ( &a, &b ) ;
                object_position_x->setValue ( a ) ;
                object_position_y->setValue ( b ) ;

                active_object->getSize ( &a, &b ) ;
                object_size_x->setValue ( a ) ;
                object_size_y->setValue ( b ) ;
              }
        }
    wid = wid->next ;
    }
  }
  glutPostRedisplay () ;
}

static void main_window_passivefn ( int x, int yy )
{
  int y = main_window_height - yy ;

  // Dragging the mouse without a button down:  save the mouse coordinates
  mouse_x = x ;
  mouse_y = y ;
  /*if (context_menu->isVisible() == 1)
  {
      puBox *cont_box = context_menu->getABox();
      if ( ( x >= cont_box->min[0] ) && ( x <= cont_box->max[0] ) &&
           ( y >= cont_box->min[1] ) && ( y <= cont_box->max[1] ) )
      {
          context_menu->checkHit(-1, 0, mouse_x, mouse_y);
      }
  }*/
}

static void main_window_mousefn ( int button, int updown, int x, int yy )
{
  int y = main_window_height - yy ;

  mouse_x = x ;
  mouse_y = y ;

  if ( updown == PU_DOWN )
    pguide_last_buttons |=  ( 1 << button ) ;
  else
    pguide_last_buttons &= ~( 1 << button ) ;

  if ( button == PU_RIGHT_BUTTON )
  {
  /* Pressed the right mouse button -- popup the manipulation window. */
      WidgetList *wid = widgets ;
      while ( wid )
      {
        puBox *box = wid->obj->getABox () ;
        if ( wid->visible && ( x >= box->min[0] ) && ( x <= box->max[0] ) &&
                             ( y >= box->min[1] ) && ( y <= box->max[1] ) )
        {

          active_widget = wid ;
          active_object = wid->obj ;
          /* We can modify mouse_x and mouse_y, because theyre not used again if we're in the right mouse function */
          /* If we're real close to the right of the screen, modify the X position ...*/
          if (mouse_x > main_window_width - 110 ) mouse_x = main_window_width - 110 ;
          /* If we're real close to the top of the screen, modify the Y position...*/
          if (mouse_y > main_window_height - 140) mouse_y = main_window_height - 140 ;

          context_menu->setPosition(mouse_x, mouse_y) ;
          context_menu->reveal() ;
          //puSetActiveWidget(context_menu, mouse_x, mouse_y) ;
          break ;
        }
        wid = wid->next ;
      }

  } else {
      if ( puActiveWidget() && ( active_object != puActiveWidget() ) )
      {
        puActiveWidget() -> invokeDownCallback () ;
        puDeactivateWidget () ;
      }

      ctrl_key_down = ( glutGetModifiers () & GLUT_ACTIVE_CTRL ) ;

      if ( ( context_menu->isVisible() == 1) && ( active_widget ) )
      {
          puBox *cont_box = context_menu->getABox();
          if ( ( x >= cont_box->min[0] ) && ( x <= cont_box->max[0] ) &&
               ( y >= cont_box->min[1] ) && ( y <= cont_box->max[1] ) )
          {
              context_menu->checkHit(button, updown, mouse_x, mouse_y);
          } else {
              context_menu->hide();
          }
      } else /*if ( context_menu->isVisible() == 1) { context_menu->hide(); } * The user did something funny, so hide the menu since they've been naughty */
      {
      // Downclick:  Place a new widget, activate an existing widget, deactivate widget, or select from menu.
      if ( updown == GLUT_DOWN )
      {
        // If there is a selected object, create a new one of that type.
    //    extern puObject *createWidget ( int type ) ;
        extern int selected_object_type ;

        if ( selected_object_type )
        {
          extern bool selected_object_sticky ;
          extern char *selected_type_string ;
          extern puButton *active_button ;

          main_window_changed = true ;

    //      puObject *new_obj = createWidget ( selected_object->getType () ) ;
          puObject *new_obj = new puFrame ( 0, 0, 90, 20 ) ;
          char *object_type_name = new char [ PUSTRING_MAX ] ;
          strcpy ( object_type_name, selected_type_string ) ;
          new_obj->setLegend ( object_type_name ) ;

          // Add the new object to the list of widgets
          WidgetList *new_wid = new WidgetList ;
          new_wid->obj = new_obj ;
          new_wid->label_text = (char *)NULL ;
          new_wid->legend_text = (char *)NULL ;
          new_wid->allowed = (char *)NULL ;
          new_wid->items = (char *)NULL ;
          new_wid->object_type_name = object_type_name ;
          new_wid->object_type = selected_object_type ;
          new_wid->callbacks = 1 ;  // Default:  up callback enabled, others disabled
          sprintf ( new_wid->object_name, "widget%d", widget_number++ ) ;
          new_wid->visible = true ;
          new_wid->locked = autolock ; /* Set it to the autolock var (true or false ) */
          new_wid->layer = max_layer - 1 ;
          new_wid->next = widgets ;
          widgets = new_wid ;

          // Set the new widget's position
          new_obj->setPosition ( x, y ) ;

          mouse_x_position_in_object = 0 ;
          mouse_y_position_in_object = 0 ;


          /* Set up the widget's default data values */
          new_wid->intval1 = 0;
          new_wid->intval2 = 0;
          new_wid->boolval1 = false;
          new_wid->boolval2 = false;
          new_wid->boolval3 = false;
          new_wid->floatval1 = 0.0f;
          new_wid->floatval2 = 0.0f;
          new_wid->floatval3 = 0.0f;
          new_wid->floatval4 = 0.0f;
          new_wid->floatval5 = 0.0f;
          new_wid->floatval6 = 0.0f;

          if (new_wid->object_type == PUCLASS_TEXT)
          {
               new_obj->setSize(5,20);
          }
          if (new_wid->object_type == PUCLASS_COMBOBOX)
          {
               new_wid->intval1 = 1;
               new_wid->boolval1 = true;
          }

          if (new_wid->object_type == PUCLASS_SELECTBOX)
          {
               new_wid->intval1 = 1;
          }

          if (new_wid->object_type == PUCLASS_BUTTONBOX)
          {
               new_wid->boolval1 = true;
          }

          if (new_wid->object_type == PUCLASS_COMBOBOX)
          {
               new_wid->intval1 = 1;
               new_wid->boolval1 = true;
          }

          if ( (new_wid->object_type == PUCLASS_SLIDER )       || 
               (new_wid->object_type == PUCLASS_BISLIDER )     || 
               (new_wid->object_type == PUCLASS_TRISLIDER )    || 
               (new_wid->object_type == PUCLASS_DIAL )         || 
               (new_wid->object_type == PUCLASS_SPINBOX )      || 
               (new_wid->object_type == PUCLASS_SCROLLBAR )    )
          {
                new_wid->floatval1 = 1.0f;
                new_wid->floatval2 = 0.0f;
                new_wid->floatval3 = 0.0f;
                new_wid->boolval1 = true;
          }

         if ( (new_wid->object_type == PUCLASS_SLIDER )       || 
              (new_wid->object_type == PUCLASS_BISLIDER )     || 
              (new_wid->object_type == PUCLASS_TRISLIDER )     )  
         {
                new_wid->boolval2 = true;
         }

         if (new_wid->object_type == PUCLASS_SLIDER)
         {
            new_wid->floatval4 = 0.5f;
         }

         if (new_wid->object_type == PUCLASS_BISLIDER)
         {
            new_wid->floatval4 = 1.0f;
            new_wid->floatval5 = 0.0f;
         }
                  
         if (new_wid->object_type == PUCLASS_TRISLIDER)
         {
            new_wid->floatval4 = 1.0f;
            new_wid->floatval5 = 0.0f;
            new_wid->floatval6 = 0.5f;
            new_wid->boolval3 = false;
         }

         if (new_wid->object_type == PUCLASS_DIAL)
         {
            new_obj->setSize(50,50);
            new_wid->boolval2 = true;
         }

         if (new_wid->object_type == PUCLASS_SPINBOX)
         {
            new_wid->boolval2 = true;
            new_wid->floatval4 = 0.5f;
         }
         
         if (new_wid->object_type == PUCLASS_SCROLLBAR)
         {
           /* Not Yet Implemented */
         }

         if (new_wid->object_type == PUCLASS_SPINBOX)
         {
            new_wid->boolval2 = true;
            new_wid->floatval4 = 0.5f;
         }
         
         if (new_wid->object_type == PUCLASS_INPUT)
         {
            new_wid->boolval1 = true;
         }

         if (new_wid->object_type == PUCLASS_LARGEINPUT)
         {
            new_wid->boolval1 = true;
            new_wid->intval1 = 1;
            new_wid->intval2 = 5;
         }

         if (new_wid->object_type == PUCLASS_MENUBAR)
         {
            new_obj->setSize( 120, 30) ;
            new_obj->setPosition( 0, puGetWindowHeight()-30) ;
            new_wid->locked = true ;
         }

         if (new_wid->object_type == PUCLASS_VERTMENU)
         {
            new_obj->setSize( 45, 120) ;
            new_obj->setPosition( 0, puGetWindowHeight()-120) ;
            new_obj->setLegend("");
            new_obj->setLabelPlace(PUPLACE_BOTTOM_LEFT);
            new_obj->setLabel("puVerticalMenu");
            new_wid->locked = true ;
         }
             
          // Make the new widget the active widget
          active_object = new_obj ;
          active_widget = new_wid ;

          // If the "sticky" flag is not set, reset the selected object
          if ( !selected_object_sticky )
          {
            selected_object_type = 0 ;
            active_button->setValue ( 0 ) ;
          }
        }
        else
        {
          // Clicking on a widget in the main widget list activates it
          WidgetList *wid = widgets ;
          while ( wid )
          {
            puBox *box = wid->obj->getABox () ;
            if ( wid->visible && ( x >= box->min[0] ) && ( x <= box->max[0] ) &&
                                 ( y >= box->min[1] ) && ( y <= box->max[1] ) )
            {
              active_widget = wid ;
              active_object = wid->obj ;
              if ( abs ( x - box->min[0] ) < RESIZING_BORDER_WIDTH )
                activity_flag = 2 ;  // Resizing on x-min
              else if ( abs ( x - box->max[0] ) < RESIZING_BORDER_WIDTH )
                activity_flag = 4 ;  // Resizing on x-max
              else if ( abs ( y - box->min[1] ) < RESIZING_BORDER_WIDTH )
                activity_flag = 3 ;  // Resizing on y-min
              else if ( abs ( y - box->max[1] ) < RESIZING_BORDER_WIDTH )
                activity_flag = 5 ;  // Resizing on y-max
              else
                activity_flag = 1 ;  // Away from edges, we're moving it

       /* Now we check and see if we're clicking on a corner - the sensitivity for
       the determiniation of whether we're clicking a corner or not is held by the
       definition of "RESIZING_CORNER_BORDER_WIDTH" which is at the top of this file */
                
	      int corner_resize_width = RESIZING_CORNER_BORDER_WIDTH + (((box->max[0] - box->min[0]) + (box->max[1] - box->min[1]) / 2 ) / 50 ) * 2 ;

              if ( activity_flag != 1 )
	      {
                  // Bottom Right Corner
                  if ( ( abs( y - box->min[1] ) < corner_resize_width  ) && (abs ( x - box->max[0] ) < corner_resize_width ) )
		    activity_flag = 9;

                  // Bottom Left Corner
                  if ( ( abs( y - box->min[1] ) < corner_resize_width ) && (abs ( x - box->min[0] ) < corner_resize_width ) )
		    activity_flag = 6;

                  // Top Right Corner
                  if ( ( abs( y - box->max[1] ) < corner_resize_width ) && (abs ( x - box->max[0] ) < corner_resize_width ) )
		    activity_flag = 7;

                  // Top Left Corner
                  if ( ( abs( y - box->max[1] ) < corner_resize_width ) && (abs ( x - box->min[0] ) < corner_resize_width ) )
		    activity_flag = 8;
	      }

              /* If CTRL is held down, symmetric resize is forced. */

              resize_symmetric = ctrl_key_down ? 2 : 1 ;

              if (active_widget->object_type == PUCLASS_DIAL)
                  resize_corner = 1 ;
	      else
                  resize_corner = 0;

              int object_x, object_y ;
              active_object->getPosition ( &object_x, &object_y ) ;
              mouse_x_position_in_object = x - object_x ;
              mouse_y_position_in_object = y - object_y ;

              break ;
            }

            wid = wid->next ;
          }

          if ( ( !wid ) && ( context_menu->isVisible()==0 ) )  // Ran through the entire list, deactivate any active widget
          {
            active_widget = (WidgetList *)NULL ;
            active_object = (puObject *)NULL ;
          }
        }
}

        setStatusWidgets ( active_widget ) ;
      }
    }
  glutPostRedisplay () ;
}


static void main_window_reshapefn ( int w, int h )
{
  extern puInput *window_size_x ;
  extern puInput *window_size_y ;

  if (done_first_setup)
    main_window_changed = true ;
  else
    done_first_setup = true ;

  if ( ( !currently_loading ) && (
       ( mouse_x < main_window_width/2 ) ||  // Grabbed the left edge ...
       ( mouse_y < main_window_height/2 ) ) )  // or the bottom edge, move the widgets
  {
    WidgetList *wid = widgets ;
    int deltax = 0 ;
    int deltay = 0 ;

    if ( mouse_x < main_window_width/2  ) deltax = w - main_window_width ;
    if ( mouse_y < main_window_height/2 ) deltay = h - main_window_height ;

    while ( wid )
    {
      int x, y ;
      wid->obj->getPosition ( &x, &y ) ;
      wid->obj->setPosition ( x + deltax, y + deltay ) ;

      wid = wid->next ;
    }
  }

  main_window_width = w ;
  main_window_height = h ;

  window_size_x->setValue ( w ) ;
  window_size_y->setValue ( h ) ;

  currently_loading = false ;
}

static void main_window_displayfn ( void )
{
  /* Clear the screen */
  glutSetWindow ( main_window ) ;

  glClearColor ( main_window_color_r, main_window_color_g, main_window_color_b, main_window_color_a ) ;
  glClear      ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;

  // Set the OpenGL to draw PUI widgets

  int w = puGetWindowWidth  () ;
  int h = puGetWindowHeight () ;

  glPushAttrib   ( GL_ENABLE_BIT | GL_TRANSFORM_BIT | GL_LIGHTING_BIT ) ;

  glDisable      ( GL_LIGHTING   ) ;
  glDisable      ( GL_FOG        ) ;
  glDisable      ( GL_TEXTURE_2D ) ;
  glDisable      ( GL_DEPTH_TEST ) ;
  glDisable      ( GL_CULL_FACE  ) ;
 
  glViewport ( 0, 0, w, h ) ;
 
  glMatrixMode   ( GL_PROJECTION ) ;
  glPushMatrix   () ;
  glLoadIdentity () ;
  gluOrtho2D     ( 0, w, 0, h ) ;
  glMatrixMode   ( GL_MODELVIEW ) ;
  glPushMatrix   () ;
  glLoadIdentity () ;

  // Draw the widgets
  int layer ;
  for ( layer = 0; layer < max_layer; layer++ )
  {
    WidgetList *wid = widgets ;
    while ( wid )
    {
      if ( wid->visible && ( wid->layer == layer ) ) wid->obj->draw ( 0, 0 ) ;
      wid = wid->next ;
    }
  }

  if ( active_object )
  {
    puBox *box = active_object->getBBox () ;
    glColor4f ( 0.0f, 0.0f, 0.0f, 1.0f ) ;
    glLineWidth ( 3.0f ) ;
    glBegin ( GL_LINES ) ;
    glVertex2i ( box->min[0]-1, box->min[1]-1 ) ;
    glVertex2i ( box->max[0]+1, box->min[1]-1 ) ;
    glVertex2i ( box->max[0]+1, box->max[1]+1 ) ;
    glVertex2i ( box->min[0]-1, box->max[1]+1 ) ;
    glVertex2i ( box->min[0]-1, box->min[1]-1 ) ;
    glEnd () ;
  }

  context_menu->draw(0,0);

  /* Update GLUT */

  glutSwapBuffers   () ;
  glutPostRedisplay () ;

}

int main ( int argc, char **argv )
{
  extern int define_widget_window () ;
  extern int define_status_window () ;
  int i = 0;

  strcpy ( main_window_name, "PUI GUI Builder" ) ;

  glutInitWindowPosition( 100,   0 ) ;
  glutInitWindowSize    ( main_window_width, main_window_height ) ;
  glutInit              ( &argc, argv ) ;
  glutInitDisplayMode   ( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH ) ;
  main_window = glutCreateWindow      ( main_window_name ) ;
  glutDisplayFunc       ( main_window_displayfn ) ;
  glutKeyboardFunc      ( main_window_keyfn     ) ;
  glutSpecialFunc       ( main_window_specialfn ) ;
  glutMouseFunc         ( main_window_mousefn   ) ;
  glutMotionFunc        ( main_window_motionfn  ) ;
  glutPassiveMotionFunc ( main_window_passivefn ) ;
  glutReshapeFunc       ( main_window_reshapefn ) ;
  glutIdleFunc          ( main_window_displayfn ) ;

// If we're using windows, ignore the C: or whatnot 

#ifdef WIN32
  #define PATH_SEPARATOR   '\\'
  strcpy (pguide_current_directory, argv[0]+2);
#else
  #define PATH_SEPARATOR   '/'
  strcpy (pguide_current_directory, argv[0]);
#endif
  i = strlen(pguide_current_directory);


  while (pguide_current_directory[i] != PATH_SEPARATOR) {
      if (i>0) i-- ;
      else break ;
  }
  pguide_current_directory[i+1] = '\0';
  
  puInit () ;

#ifdef VOODOO
  puShowCursor () ;
#endif

  puSetDefaultStyle        ( PUSTYLE_SMALL_SHADED ) ;
  puSetDefaultColourScheme ( 0.8f, 0.8f, 0.8f, 1.0f ) ;

  // Add the properties context menu
  context_menu = new puPopupMenu( 0, 0 ) ;
  context_menu->add_item("Properties", cb_edit_properties) ;
  context_menu->add_item("Lock/Unlock", cb_lock_toggle) ;
  context_menu->add_item("Delete", cb_popup_delete) ;
  context_menu->add_item("Cut", NULL) ;
  context_menu->add_item("Copy", NULL) ;
  context_menu->add_item("Change Class", NULL) ;
  context_menu->close() ;

  // Set up the other windows
  define_widget_window () ;
  define_status_window () ;

  glutMainLoop () ;
  return 0 ;
}

