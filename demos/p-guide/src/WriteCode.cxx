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

// Function to write the actual code

#include <stdio.h>

#include "WidgetList.h"

#ifdef WIN32
  #define PATH_SEPARATOR   '\\'
#else
  #define PATH_SEPARATOR   '/'
#endif

// From the Main Window:

extern WidgetList *widgets ;
extern int max_layer ;

extern int main_window_width  ;
extern int main_window_height ;

extern char main_window_name [ PUSTRING_MAX ] ;
extern char pguide_current_directory [ PUSTRING_MAX ] ;

extern int main_window_x ;
extern int main_window_y ;

extern bool main_window_changed ;

extern float main_window_color_r, main_window_color_g,
             main_window_color_b, main_window_color_a ;

// From the Status Window

extern int status_window ;
extern puFileSelector *file_selector ;

const char* trueOrFalse(bool tf)
{
   if (tf) return "TRUE";
   else return "FALSE";
}

// The function itself

void write_code ( puObject *ob )
{
  // Get the file name and open the file
  char* filename ;
  file_selector -> getValue ( &filename ) ;
  if (filename[0] == '\0')
  {
      puDeleteObject ( file_selector ) ;
      file_selector = (puFileSelector *)NULL ;
      glutHideWindow () ;
      glutSetWindow ( status_window ) ;
      return ;
  }        

  /* Save the new current directory */
  strcpy(pguide_current_directory, filename) ;
  int i = strlen(pguide_current_directory);
  while (pguide_current_directory[i] != PATH_SEPARATOR) {
      if (i>0) i-- ;
      else break ;
  }
  pguide_current_directory[i+1] = '\0' ;

  /* If they didn't give an extension, then tack ".cxx" onto the end. */
  if(!strstr(filename, "."))
      sprintf(filename, "%s.cxx", filename);

  FILE *out = fopen ( filename, "wt" ) ;
  if ( !out )
    printf ( "ERROR opening file <%s> for writing\n", filename ) ;

  puDeleteObject ( file_selector ) ;
  file_selector = (puFileSelector *)NULL ;
  glutHideWindow () ;
  glutSetWindow ( status_window ) ;

  if ( !out ) return ;

  // Start writing code:

  fprintf ( out, "// TODO:  Initial documentation\n" ) ;
  fprintf ( out, "// TODO:  Configuration Management system flags\n\n" ) ;

  fprintf ( out, "// TODO:  Any additional includes you may need\n\n" ) ;

  fprintf ( out, "#include <GL/freeglut.h>\n\n" ) ;
  fprintf ( out, "#include <plib/pu.h>\n\n" ) ;
  fprintf ( out, "#include <plib/puAux.h>  // TODO:  Decide if you really need this\n\n" ) ;

  fprintf ( out, "// GLUT Window Handle\n" ) ;
  fprintf ( out, "int window_handle ;\n\n" ) ;

  WidgetList *wid = widgets ;
  fprintf ( out, "// PUI Widget Handles:\n" ) ;

  while ( wid )
  {
    fprintf ( out, "%s *%s = (%s *)NULL ;\n", wid->object_type_name, wid->object_name,
                                              wid->object_type_name ) ;
    wid = wid->next ;
  }

  fprintf ( out, "\n" ) ;

  // GLUT Callbacks:

  fprintf ( out, "// GLUT Callbacks:\n" ) ;
  fprintf ( out, "static void specialfn ( int key, int x, int y )\n" ) ;
  fprintf ( out, "{\n" ) ;
  fprintf ( out, "  // TODO:  Put any of your own special-key functionality in here\n" ) ;
  fprintf ( out, "  puKeyboard ( key + PU_KEY_GLUT_SPECIAL_OFFSET, PU_DOWN ) ;\n" ) ;
  fprintf ( out, "  glutPostRedisplay () ;\n" ) ;
  fprintf ( out, "}\n\n" ) ;

  fprintf ( out, "static void keyfn ( unsigned char key, int x, int y )\n" ) ;
  fprintf ( out, "{\n" ) ;
  fprintf ( out, "  // TODO:  Put any of your own keyboard functionality in here\n" ) ;
  fprintf ( out, "  puKeyboard ( key, PU_DOWN ) ;\n" ) ;
  fprintf ( out, "  glutPostRedisplay () ;\n" ) ;
  fprintf ( out, "}\n\n" ) ;

  fprintf ( out, "static void motionfn ( int x, int y )\n" ) ;
  fprintf ( out, "{\n" ) ;
  fprintf ( out, "  // TODO:  Put any of your own mouse motion functionality in here\n" ) ;
  fprintf ( out, "  puMouse ( x, y ) ;\n" ) ;
  fprintf ( out, "  glutPostRedisplay () ;\n" ) ;
  fprintf ( out, "}\n\n" ) ;

  fprintf ( out, "static void mousefn ( int button, int updown, int x, int y )\n" ) ;
  fprintf ( out, "{\n" ) ;
  fprintf ( out, "  // TODO:  Put any of your own mouse click functionality in here\n" ) ;
  fprintf ( out, "  puMouse ( button, updown, x, y ) ;\n" ) ;
  fprintf ( out, "  glutPostRedisplay () ;\n" ) ;
  fprintf ( out, "}\n\n" ) ;

  fprintf ( out, "static void displayfn ( void )\n" ) ;
  fprintf ( out, "{\n" ) ;
  fprintf ( out, "  // Clear the screen\n\n" ) ;
  fprintf ( out, "  glClearColor ( %f, %f, %f, %f ) ;\n", main_window_color_r, main_window_color_g,
                                                          main_window_color_b, main_window_color_a ) ;
  fprintf ( out, "  glClear      ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;\n\n" ) ;

  fprintf ( out, "  // TODO:  Put any of your own window display in here\n\n" ) ;

  fprintf ( out, "  // Make PUI display\n" ) ;
  fprintf ( out, "  puDisplay () ;\n\n" ) ;

  fprintf ( out, "  glutSwapBuffers   () ;\n" ) ;
  fprintf ( out, "  glutPostRedisplay () ;\n" ) ;
  fprintf ( out, "}\n\n" ) ;

  fprintf ( out, "// TODO:  Put any of your own additional GLUT callbacks in here\n\n" ) ;

  // PUI Callbacks

  fprintf ( out, "// PUI Callbacks:\n\n" ) ;

  wid = widgets ;

  while ( wid )
  {
    if ( wid->callbacks & 0x01 )  // Up-callback defined
    {
      fprintf ( out, "void %s_cb ( puObject *ob )\n", wid->object_name ) ;
      fprintf ( out, "{\n" ) ;
      fprintf ( out, "  // TODO:  Put your up-callback functionality in here\n" ) ;
      fprintf ( out, "}\n\n" ) ;
    }

    if ( wid->callbacks & 0x02 )  // Active callback defined
    {
      fprintf ( out, "void %s_active_cb ( puObject *ob )\n", wid->object_name ) ;
      fprintf ( out, "{\n" ) ;
      fprintf ( out, "  // TODO:  Put your active callback functionality in here\n" ) ;
      fprintf ( out, "}\n\n" ) ;
    }

    if ( wid->callbacks & 0x04 )  // Down-callback defined
    {
      fprintf ( out, "void %s_down_cb ( puObject *ob )\n", wid->object_name ) ;
      fprintf ( out, "{\n" ) ;
      fprintf ( out, "  // TODO:  Put your down-callback functionality in here\n" ) ;
      fprintf ( out, "}\n\n" ) ;
    }

    wid = wid->next ;
  }

  fprintf ( out, "\n\n" ) ;

  // The Main Program

  fprintf ( out, "//**********************************************************\n" ) ;
  fprintf ( out, "//*                    The Main Program                    *\n" ) ;
  fprintf ( out, "//**********************************************************\n\n" ) ;

  fprintf ( out, "int main ( int argc, char *argv[] )\n" ) ;
  fprintf ( out, "{\n" ) ;
  fprintf ( out, "  // TODO:  Add any non-GLUT functionality you want to\n\n" ) ;

  fprintf ( out, "  // Create the GLUT window:\n" ) ;
  fprintf ( out, "  glutInitWindowPosition( %d, %d ) ;\n", main_window_x, main_window_y ) ;
  fprintf ( out, "  glutInitWindowSize    ( %d, %d ) ;\n", main_window_width, main_window_height ) ;
  fprintf ( out, "  glutInit              ( &argc, argv ) ;\n" ) ;
  fprintf ( out, "  glutInitDisplayMode   ( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH ) ;\n" ) ;
  fprintf ( out, "  window_handle = glutCreateWindow      ( \"%s\"  ) ;\n\n", main_window_name ) ;

  fprintf ( out, "  // GLUT Windowing Callbacks:\n" ) ;
  fprintf ( out, "  glutDisplayFunc       ( displayfn ) ;\n" ) ;
  fprintf ( out, "  glutKeyboardFunc      ( keyfn     ) ;\n" ) ;
  fprintf ( out, "  glutSpecialFunc       ( specialfn ) ;\n" ) ;
  fprintf ( out, "  glutMouseFunc         ( mousefn   ) ;\n" ) ;
  fprintf ( out, "  glutMotionFunc        ( motionfn  ) ;\n" ) ;
  fprintf ( out, "  glutPassiveMotionFunc ( motionfn  ) ;\n" ) ;
  fprintf ( out, "  glutIdleFunc          ( displayfn ) ;\n" ) ;
  fprintf ( out, "  // TODO:  Add any additional GLUT callbacks that you defined earlier\n\n" ) ;

  fprintf ( out, "  // Initialize PUI:\n" ) ;
  fprintf ( out, "  puInit () ;\n\n" ) ;

  // TO DO:  Allow the user to select his own default fonts, style, and colour scheme

  fprintf ( out, "  // PUI Default Style, and Colors:\n" ) ;
  fprintf ( out, "  puSetDefaultStyle        ( PUSTYLE_SMALL_SHADED ) ;\n" ) ;
  fprintf ( out, "  puSetDefaultColourScheme ( 0.3f, 0.4f, 0.6f, 1.0f ) ;\n" ) ;
  fprintf ( out, "  // TODO:  Customize this as you like\n\n" ) ;

  fprintf ( out, "  // Define the widgets:\n\n" ) ;

  fprintf ( out, "  puGroup *window_group = new puGroup ( 0, 0 ) ;\n\n" ) ;

  int layer ;
  for ( layer = 0; layer < max_layer; layer++ )
  {
    wid = widgets ;
    while ( wid )
    {
      if ( layer == wid->layer )
      {
        puObject *ob = wid->obj ;
        int x, y, w, h ;
        ob->getPosition ( &x, &y ) ;
        ob->getSize ( &w, &h ) ;

        /* Customize widget's constructor and extra details */
        /*         fprintf ( out, "  %s = new %s ( %d, %d, %d, %d ) ;\n", wid->object_name,
                  wid->object_type_name, x, y, x+w, y+h ) ;    */

        /* General minx, miny, maxx, maxy constructor */
          if ( (wid->object_type == PUCLASS_FRAME           ) ||
               (wid->object_type == PUCLASS_BUTTON          ) ||
               (wid->object_type == PUCLASS_INPUT           ) ||
               (wid->object_type == PUCLASS_BISLIDERWITHENDS))
          {
            fprintf ( out, "  %s = new %s (%d, %d, %d, %d ) ;\n", wid->object_name, wid->object_type_name, x, y, x+w, y+h) ;
          } 
  
                /* General minx, miny constructor */
          if (wid->object_type == PUCLASS_TEXT)
          {
              /* Add in font properties? */
            fprintf ( out, "  %s = new %s (%d, %d ) ;\n", wid->object_name, wid->object_type_name, x, y) ;
          } 
  
          if (wid->object_type == PUCLASS_ONESHOT)
          {
              /*
              Add support for both constructors! 
              puButton::puButton ( int minx, int miny, const char *legend ) ;
              puButton::puButton ( int minx, int miny, int maxx, int maxy ) ;

              */
            fprintf ( out, "  %s = new %s (%d, %d, %d, %d ) ;\n", wid->object_name, wid->object_type_name, x, y, x+w, y+h) ;
          } 

          if ( (wid->object_type == PUCLASS_MENUBAR) || 
               (wid->object_type == PUCLASS_VERTMENU) ) {
 
            if (wid->items)
            {
                char *temp_items = wid->items ;
                char *cr = strchr(temp_items, '\n');
                while ( cr != '\0' )
                {
                    char *spc = strchr ( temp_items, ' ' ) ;
                    while ( spc )
                    {
                      *spc = '_' ;
                      spc = strchr ( temp_items, ' ' ) ;
                    }
                    spc = strchr ( temp_items, ':' ) ;
                    while ( spc )
                    {
                      *spc = '_' ;
                      spc = strchr ( temp_items, ':' ) ;
                    }
                    spc = strchr ( temp_items, ';' ) ;
                    while ( spc )
                    {
                      *spc = '_' ;
                      spc = strchr ( temp_items, ';' ) ;
                    }
                    spc = strchr ( temp_items, ',' ) ;
                    while ( spc )
                    {
                      *spc = '_' ;
                      spc = strchr ( temp_items, ',' ) ;
                    }
                    spc = strchr ( temp_items, '.' ) ;
                    while ( spc )
                    {
                      *spc = '_' ;
                      spc = strchr ( temp_items, '.' ) ;
                    }
                    *cr = '\0';
                    fprintf ( out, "  static char *%s_%s_submenu [] = { \"Filler\", NULL } ;\n", wid->object_name, temp_items) ;
                    fprintf ( out, "  /* TODO: You need to create your own callbacks here, such as { \"exit_cb\",\"delete_cb\",NULL } */ \n") ;
                    fprintf ( out, "  puCallback %s_%s_submenu_cb [] = { NULL, NULL } ;\n\n", wid->object_name, temp_items) ;
                    *cr = '\n';
                    temp_items = cr + 1;
                    cr = strchr(temp_items, '\n');
                }
            }

            if (wid->object_type == PUCLASS_MENUBAR) {
                /* TODO: add in "height" support */
                fprintf ( out, "  %s = new %s ( ) ;\n", wid->object_name, wid->object_type_name) ;
            }

            if (wid->object_type == PUCLASS_VERTMENU) {
            /* TODO: add in "lock to corner" support */
                fprintf ( out, "  %s = new %s (%d, %d ) ;\n", wid->object_name, wid->object_type_name, x, y) ;
            }
    
            if (wid->items)
            {
                char *temp_items = wid->items ;
                char unspaced_items[PUSTRING_MAX];
                char *cr = strchr(temp_items, '\n');
                fprintf ( out, "  {\n") ;
                while ( cr != '\0' )
                {
                    *cr = '\0';
                    strcpy(unspaced_items, temp_items);
                    /* Put back the space */
                    char *spc = strchr ( temp_items, '_' ) ;
                    while ( spc )
                    {
                      *spc = ' ' ;
                      spc = strchr ( temp_items, '_' ) ;
                    }
                    /* Now "unspaced_items" writes "I_have_no_spaces" while temp_items writes "I have no spaces" (AND LIES!) */
                    fprintf ( out, "    %s->add_submenu (\"%s\", %s_%s_submenu, %s_%s_submenu_cb ) ;\n", wid->object_name, temp_items, wid->object_name, unspaced_items, wid->object_name, unspaced_items ) ;
                    *cr = '\n';
                    temp_items = cr + 1;
                    cr = strchr(temp_items, '\n');
                }
                fprintf ( out, "  }\n") ;
            }
            fprintf ( out, "  %s->close() ;\n", wid->object_name) ;
          }
  
          if ( (wid->object_type == PUCLASS_LISTBOX)      ||
               (wid->object_type == PUCLASS_COMBOBOX)     ||
               (wid->object_type == PUCLASS_SELECTBOX)    ||
               (wid->object_type == PUCLASS_BUTTONBOX)    ||
               (wid->object_type == PUCLASS_SCROLLINGLIST))
          {
            char data[1024]; 
            char onedata[PUSTRING_MAX]; 

            if (wid->items)
            {
                char *temp_items = wid->items ;
                char *cr = strchr(temp_items, '\n');
                sprintf ( data, " ");
                while ( cr != '\0' )
                {
                    *cr = '\0';
                    sprintf ( onedata, "\"%s\",", temp_items);
                    *cr = '\n';
                    temp_items = cr + 1;
                    cr = strchr(temp_items, '\n');
                    strcat(data, onedata);
                }
            }

            fprintf ( out, "  static char *%s_entries [] = { %s NULL } ;\n", wid->object_name, data) ;
            if ( (wid->object_type == PUCLASS_LISTBOX)       ||
                 (wid->object_type == PUCLASS_SCROLLINGLIST) )
                fprintf ( out, "  %s = new %s (%d, %d, %d, %d, %s_entries ) ;\n", wid->object_name, wid->object_type_name, x, y, x+w, y+h, wid->object_name) ;
            if (wid->object_type == PUCLASS_BUTTONBOX)
                fprintf ( out, "  %s = new %s (%d, %d, %d, %d, %s_entries, %s ) ;\n", wid->object_name, wid->object_type_name, x, y, x+w, y+h, wid->object_name, trueOrFalse(wid->boolval1)) ;
            if (wid->object_type == PUCLASS_COMBOBOX)
            {
                fprintf ( out, "  %s = new %s (%d, %d, %d, %d, %s_entries, %s ) ;\n", wid->object_name, wid->object_type_name, x, y, x+w, y+h, wid->object_name, trueOrFalse(wid->boolval1)) ;
                fprintf ( out, "  %s->setCurrentItem(%d) ;\n", wid->object_name, wid->intval1) ;
            }
            if (wid->object_type == PUCLASS_SELECTBOX)
            {
                fprintf ( out, "  %s = new %s (%d, %d, %d, %d, %s_entries ) ;\n", wid->object_name, wid->object_type_name, x, y, x+w, y+h, wid->object_name) ;
                fprintf ( out, "  %s->setCurrentItem(%d) ;\n", wid->object_name, wid->intval1) ;
            }
          }

          if (wid->object_type == PUCLASS_POPUPMENU)
          {
            /* Allow a list of menu items, and remember to point out these are hidden when created, and must be reveal()ed. */
            /* NOT CURRENTLY IMPLEMENTED */
          } 

          /* puRange */
          if ( (wid->object_type == PUCLASS_SLIDER )       || 
               (wid->object_type == PUCLASS_BISLIDER )     || 
               (wid->object_type == PUCLASS_TRISLIDER )    || 
               (wid->object_type == PUCLASS_DIAL )         || 
               (wid->object_type == PUCLASS_SPINBOX )      || 
               (wid->object_type == PUCLASS_SCROLLBAR )    )
          {

                /* Constructors */
                if (  (wid->object_type == PUCLASS_SLIDER )   || 
                      (wid->object_type == PUCLASS_BISLIDER ) || 
                      (wid->object_type == PUCLASS_TRISLIDER ))
                {
                    /* Sliders */
                    fprintf ( out, "  %s = new %s (%d, %d, %d, %s, %d ) ;\n", wid->object_name, wid->object_type_name, x, y, (wid->boolval2==1)?h:w, trueOrFalse(wid->boolval2), (wid->boolval2==1)?w:h) ;
                    /* Slider value options */
                    if (wid->object_type == PUCLASS_SLIDER )
                        fprintf ( out, "  %s->setValue(%ff) ;\n", wid->object_name, wid->floatval4) ;
                    if ( (wid->object_type == PUCLASS_BISLIDER ) || (wid->object_type == PUCLASS_TRISLIDER ) )
                    {
                        fprintf ( out, "  %s->setCurrentMax(%ff) ;\n", wid->object_name, wid->floatval4) ;
                        fprintf ( out, "  %s->setCurrentMin(%ff) ;\n", wid->object_name, wid->floatval5) ;
                        if (wid->object_type == PUCLASS_TRISLIDER )
                        {
                            if (!wid->boolval3)
                                fprintf ( out, "  %s->setFreezeEnds(FALSE) ;\n", wid->object_name ) ;
                            fprintf ( out, "  %s->setValue(%ff) ;\n", wid->object_name, wid->floatval6) ;
                            fprintf ( out, "  %s->setSliderFraction(0.1f) ;\n", wid->object_name) ;
                            /*Allow setting the slider fraction?*/
                        }
                    }
                } else if (wid->object_type == PUCLASS_DIAL )
                {
                    fprintf ( out, "  %s = new %s (%d, %d, %d ) ;\n", wid->object_name, wid->object_type_name, x, y, (h+w)/2) ;
                    fprintf ( out, "  %s->setWrap(%s) ;\n", wid->object_name, trueOrFalse(wid->boolval2)) ;
             
                } else if (wid->object_type == PUCLASS_SPINBOX )
                {
                    fprintf ( out, "  %s = new %s (%d, %d, %d, %d, %s ) ;\n", wid->object_name, wid->object_type_name, x, y, x+w, y+h, trueOrFalse(wid->boolval2) ) ;
                    fprintf ( out, "  %s->setArrowHeight(%ff) ;\n", wid->object_name, wid->floatval4 ) ;

                } else if (wid->object_type == PUCLASS_SCROLLBAR )
                {
                    fprintf ( out, "  %s = new %s (%d, %d, %d, %d, %d, %s ) ;\n", wid->object_name, wid->object_type_name, x, y, x+w, y+h, wid->intval1, trueOrFalse(wid->boolval2) ) ;
                    fprintf ( out, "  %s->setArrowHeight(%ff) ;\n", wid->object_name, wid->floatval4 ) ;
                }
                /* All puRange options */
                fprintf ( out, "  %s->setMaxValue (%ff) ;\n", wid->object_name, wid->floatval1 ) ;
                fprintf ( out, "  %s->setMinValue (%ff) ;\n", wid->object_name, wid->floatval2 ) ;
                fprintf ( out, "  %s->setStepSize (%ff) ;\n", wid->object_name, wid->floatval3 ) ;
                char cbmodetext[20] = "PUSLIDER_ALWAYS";
                if (wid->boolval1)
                    strcpy(cbmodetext,"PUSLIDER_ALWAYS");
                else
                    strcpy(cbmodetext,"PUSLIDER_CLICK");
                fprintf ( out, "  %s->setCBMode (%s) ;\n", wid->object_name, cbmodetext) ;
          } 

          if (wid->object_type == PUCLASS_DIALOGBOX)
          {
            /* Not yet implemented as a class */
          } 

          if (wid->object_type == PUCLASS_ARROW)
          {
            char arr_name[20] = "PUARROW_UP";
            if (wid->items)
            {
                if (strstr(wid->items,"Double-Up"))
                    strcpy(arr_name, "PUARROW_FASTUP");
                else if (strstr(wid->items,"Double-Down"))
                    strcpy(arr_name, "PUARROW_FASTDOWN");
                else if (strstr(wid->items,"Double-Left"))
                    strcpy(arr_name, "PUARROW_FASTLEFT");
                else if (strstr(wid->items,"Double-Right"))
                    strcpy(arr_name, "PUARROW_FASTRIGHT");
                else if (strstr(wid->items,"Up"))
                    strcpy(arr_name, "PUARROW_UP");
                else if (strstr(wid->items,"Down"))
                    strcpy(arr_name, "PUARROW_DOWN");
                else if (strstr(wid->items,"Left"))
                    strcpy(arr_name, "PUARROW_LEFT");
                else if (strstr(wid->items,"Right"))
                    strcpy(arr_name, "PUARROW_RIGHT");
            }    
            fprintf ( out, "  %s = new puArrowButton (%d, %d, %d, %d, %s ) ;\n", wid->object_name, x, y, x+w, y+h, arr_name) ;
          } 

          if (wid->object_type == PUCLASS_CHOOSER)
          {
            // Just give it a dummy legend; set the legend later on
            fprintf ( out, "  %s = new %s (%d, %d, %d, %d, "" ) ;\n", wid->object_name, wid->object_type_name, x, y, x+w, y+h) ;
          } 

          if (wid->object_type == PUCLASS_SLIDERWITHINPUT)
          {
            // Extra argument for input box being on the bottom of the slider
            fprintf ( out, "  %s = new %s (%d, %d, %d, %d, 0 ) ;\n", wid->object_name, wid->object_type_name, x, y, x+w, y+h) ;
          } 

          if (wid->object_type == PUCLASS_INPUT )
          {
            /*Already had its constructor defined by the general -- look up*/
            if (wid->boolval1 == false)
                fprintf ( out, "  %s->rejectInput() ;\n", wid->object_name ) ;
            if ( wid->allowed)
                fprintf ( out, "  %s->setValidData(\"%s\") ;\n", wid->object_name, wid->allowed ) ;
          }

          if ( wid->object_type == PUCLASS_LARGEINPUT )
          {   
            fprintf ( out, "  %s = new %s (%d, %d, %d, %d, %d, %d ) ;\n", wid->object_name, wid->object_type_name, x, y, w, h, wid->intval1, wid->intval2 ) ;
            if (wid->boolval1 == false)
                fprintf ( out, "  %s->rejectInput() ;\n", wid->object_name ) ;
            if ( wid->allowed)
                fprintf ( out, "  %s->setValidData(\"%s\") ;\n", wid->object_name, wid->allowed ) ;

          }
        /* Done custom constructor and extra details */

        if ( wid->callbacks & 0x01 )  // Up-callback defined
          fprintf ( out, "  %s->setCallback ( %s_cb ) ;\n", wid->object_name, wid->object_name ) ;

        if ( wid->callbacks & 0x02 )  // Active callback defined
          fprintf ( out, "  %s->setActiveCallback ( %s_active_cb ) ;\n", wid->object_name, wid->object_name ) ;

        if ( wid->callbacks & 0x04 )  // Down-callback defined
          fprintf ( out, "  %s->setDownCallback ( %s_down_cb ) ;\n", wid->object_name, wid->object_name ) ;

        // TO DO:  Allow the user to customize colour and style

        const char *place_name [] = { "PUPLACE_TOP_LEFT", "PUPLACE_TOP_CENTERED", "PUPLACE_TOP_RIGHT",
                                      "PUPLACE_CENTERED_LEFT", "PUPLACE_CENTERED_RIGHT",
                                      "PUPLACE_BOTTOM_LEFT", "PUPLACE_BOTTOM_CENTERED", "PUPLACE_BOTTOM_RIGHT",
                                      "PUPLACE_CENTERED_CENTERED",
                                      "PUPLACE_ABOVE_LEFT", "PUPLACE_ABOVE_RIGHT",
                                      "PUPLACE_BELOW_LEFT", "PUPLACE_BELOW_RIGHT",
                                      "PUPLACE_UPPER_LEFT", "PUPLACE_UPPER_RIGHT",
                                      "PUPLACE_LOWER_LEFT", "PUPLACE_LOWER_RIGHT" } ;

        if ( wid->label_text )
        {
          fprintf ( out, "  %s->setLabel ( \"%s\" ) ;\n", wid->object_name, wid->label_text ) ;
          fprintf ( out, "  %s->setLabelPlace ( %s ) ;\n", wid->object_name, place_name [ ob->getLabelPlace () ] ) ;
          // TO DO:  Check for label font and print it
        }

        if ( wid->legend_text )
        {
          fprintf ( out, "  %s->setLegend ( \"%s\" ) ;\n", wid->object_name, wid->legend_text ) ;
          fprintf ( out, "  %s->setLegendPlace ( %s ) ;\n", wid->object_name, place_name [ ob->getLegendPlace () ] ) ;
          // TO DO:  Check for legend font and print it
        }

        if( !wid->visible )
          fprintf ( out, "  %s->hide () ;\n", wid->object_name ) ;

        fprintf ( out, "  \n" ) ;

      }
      wid = wid->next ;
    }
  }

  fprintf ( out, "\n\n" ) ;

  fprintf ( out, "  window_group->close () ;\n\n\n" ) ;

  fprintf ( out, "  // GLUT Main Loop\n" ) ;
  fprintf ( out, "  glutMainLoop () ;\n" ) ;
  fprintf ( out, "  return 0;\n" ) ;
  fprintf ( out, "}\n\n" ) ;

  // Close up shop
  fclose ( out ) ;
  main_window_changed = false ;
}

