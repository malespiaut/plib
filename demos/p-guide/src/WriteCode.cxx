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

// Function to write the actual code

#include <stdio.h>

#include <plib/pu.h>

#include "WidgetList.h"

// From the Main Window:

extern WidgetList *widgets ;
extern int max_layer ;

extern int main_window_width  ;
extern int main_window_height ;

extern char main_window_name [ PUSTRING_MAX ] ;

extern int main_window_x ;
extern int main_window_y ;

extern bool main_window_changed ;

extern float main_window_color_r, main_window_color_g,
             main_window_color_b, main_window_color_a ;

// From the Status Window

extern int status_window ;
extern puFileSelector *file_selector ;

// The function itself

void write_code ( puObject *ob )
{
  // Get the file name and open the file
  char* filename ;
  file_selector -> getValue ( &filename ) ;

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

  fprintf ( out, "#include <plib/pu.h>\n\n" ) ;

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

  fprintf ( out, "void main ( int argc, char *argv[] )\n" ) ;
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
  fprintf ( out, "  puSetDefaultColourScheme ( 0.3, 0.4, 0.6, 1.0 ) ;\n" ) ;
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

        // TO DO:  Customize this on a widget-type-by-widget-type basis.
        fprintf ( out, "  %s = new %s ( %d, %d, %d, %d ) ;\n", wid->object_name,
                  wid->object_type_name, x, y, x+w, y+h ) ;
        if ( wid->callbacks & 0x01 )  // Up-callback defined
          fprintf ( out, "  %s->setCallback ( %s_cb ) ;\n", wid->object_name, wid->object_name ) ;

        if ( wid->callbacks & 0x02 )  // Active callback defined
          fprintf ( out, "  %s->setActiveCallback ( %s_active_cb ) ;\n", wid->object_name, wid->object_name ) ;

        if ( wid->callbacks & 0x04 )  // Down-callback defined
          fprintf ( out, "  %s->setDownCallback ( %s_down_cb ) ;\n", wid->object_name, wid->object_name ) ;

        // TO DO:  Allow the user to customize colour and style

        char *place_name [] = { "PUPLACE_TOP_LEFT", "PUPLACE_TOP_CENTERED", "PUPLACE_TOP_RIGHT",
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

        wid = wid->next ;
      }
    }
  }

  fprintf ( out, "\n\n" ) ;

  fprintf ( out, "  window_group->close () ;\n\n\n" ) ;

  fprintf ( out, "  // GLUT Main Loop\n" ) ;
  fprintf ( out, "  glutMainLoop () ;\n" ) ;
  fprintf ( out, "  return ;\n" ) ;
  fprintf ( out, "}\n\n" ) ;

  // Close up shop
  fclose ( out ) ;
  main_window_changed = false ;
}

