/*
     PLIB - A Suite of Portable Game Libraries
     Copyright (C) 2001  Steve Baker
 
     This library is free software; you can redistribute it and/or
     modify it under the terms of the GNU Library General Public
     License as published by the Free Software Foundation; either
     version 2 of the License, or (at your option) any later version.
 
     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Library General Public License for more details.
 
     You should have received a copy of the GNU Library General Public
     License along with this library; if not, write to the Free
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 
     For further information visit http://plib.sourceforge.net

     $Id$
*/


#include "puLocal.h"

void puDial::draw ( int dx, int dy )
{
  if ( !visible || ( window != puGetWindow () ) ) return ;

  // Draw the active box.

  abox . draw ( dx, dy,
                (style==PUSTYLE_BEVELLED||
                 style==PUSTYLE_SHADED) ? -PUSTYLE_BOXED : -style,
                colour, FALSE ) ;

  // If greyed out then halve the opacity when drawing the label and legend

  if ( active )
    glColor4fv ( colour [ PUCOL_LEGEND ] ) ;
  else
    glColor4f ( colour [ PUCOL_LEGEND ][0],
                colour [ PUCOL_LEGEND ][1],
                colour [ PUCOL_LEGEND ][2],
                colour [ PUCOL_LEGEND ][3] / 2.0f ) ; // 50% more transparent

  // Draw the surrounding circle.

  float rad = (float)( abox.max [0] - abox.min [0] ) / 2.0f - 3.0f ;
  int x_cen = dx + ( abox.max [0] + abox.min [0] ) / 2 ;
  int y_cen = dy + ( abox.max [1] + abox.min [1] ) / 2 ;

  float dtheta = 3.0f / rad ;   // three pixels per segment

  float old_line_width ;

  glGetFloatv ( GL_LINE_WIDTH, &old_line_width ) ;
  
  glLineWidth ( 2.0f ) ;   // set line width to two pixels

  glBegin ( GL_LINE_LOOP ) ;

  float theta ;
  for ( theta = -3.1415927f; theta < 3.1415927f; theta+= dtheta )
  {
    float x = (float)x_cen + rad * cos ( (double)theta ) ;
    float y = (float)y_cen + rad * sin ( (double)theta ) ;

    glVertex2f ( x, y ) ;
  }

  glEnd () ;

  // Draw the line from the center.

  glLineWidth ( 4.0f ) ;  // four pixels wide

  float val ;
  getValue ( &val ) ;

  if ( val < min_value ) val = min_value ;
  if ( val > max_value ) val = max_value ;

  val = ( 2.0f * ( val - min_value ) / ( max_value - min_value ) - 1.0f ) * 3.1415927f ;

  glBegin ( GL_LINES ) ;

  glVertex2f ( x_cen, y_cen ) ;
  glVertex2f ( x_cen + rad * sin ( (double)val ), y_cen + rad * cos ( (double)val ) ) ;

  glEnd () ;

  glLineWidth ( old_line_width ) ;  // restore the old width

  int xx = ( abox.max[0] - abox.min[0] - puGetStringWidth(legendFont,legend) ) / 2 ;
  int yy = ( abox.max[1] - abox.min[1] - puGetStringHeight(legendFont) ) / 2 ;

  puDrawString ( legendFont, legend,
                  dx + abox.min[0] + xx,
                  dy + abox.min[1] + yy ) ;

  draw_label ( dx, dy ) ;
}


void puDial::doHit ( int button, int updown, int x, int y )
{
  if ( puActiveWidget() && ( this != puActiveWidget() ) )
  {
    puActiveWidget() -> invokeDownCallback () ;
    puDeactivateWidget () ;
  }

  if ( updown != PU_DRAG )
    puMoveToLast ( this );

  if ( button == PU_LEFT_BUTTON && updown != PU_UP )
  {
    int x_cen = ( abox.max [0] + abox.min [0] ) / 2 ;
    int y_cen = ( abox.max [1] + abox.min [1] ) / 2 ;
    float angle = atan2 ( (double)(x-x_cen), (double)(y-y_cen) ) *  // Up is zero degrees
                  180.0f / 3.1415927f ;
    // Move to within the (min, max) interval

    if ( angle < -180.0f )
      angle += 360.0 ;
    else if (angle > 180.0f )
      angle -= 360.0 ;

    angle = min_value + ( max_value - min_value ) * ( angle +180.0f ) / 360.0f ;

    setValue ( angle ) ;

    puSetActiveWidget ( this ) ;
    invokeCallback () ;
  }
}


