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


#include <plib/pw.h>
#include <plib/fnt.h>

int main ( int argc, char **argv )
{
  int w, h ;

  pwInit ( 100, 100, 230, 50, false, "FNT Hello World Demo", true, 0 ) ;
  pwSetCallbacks ( NULL, NULL, NULL, NULL, NULL ) ;
  pwGetSize ( &w, &h ) ;

  fntRenderer *text = new fntRenderer () ;

  text -> setFont ( new fntTexFont ( "data/Helvetica.txf" ) ) ;
  text -> setPointSize ( 30.0f ) ;

  glEnable       ( GL_ALPHA_TEST ) ;
  glEnable       ( GL_BLEND ) ;
  glAlphaFunc    ( GL_GREATER, 0.1f ) ;
  glBlendFunc    ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ) ;
  glMatrixMode   ( GL_PROJECTION ) ;
  glLoadIdentity () ;
  glOrtho        ( 0, w, 0, h, -1, 1 ) ;

  while ( 1 )
  {
    glClearColor ( 0.1f, 0.4f, 0.1f, 1.0f ) ;
    glClear      ( GL_COLOR_BUFFER_BIT ) ;

    text -> begin () ;
      glColor3f ( 1.0f, 1.0f, 0.0f ) ;
      text -> start2f ( 10.0f, 10.0f ) ;
      text -> puts ( "Hello World." ) ;
    text -> end () ;

    pwSwapBuffers () ;
  }

  return 0 ;
}

