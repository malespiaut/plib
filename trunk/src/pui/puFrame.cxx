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

void puFrame::draw ( int dx, int dy )
{
  if ( !visible || ( window != puGetWindow () ) ) return ;

  abox.draw ( dx, dy, style, colour, FALSE, border_thickness ) ;

  /* If greyed out then halve the opacity when drawing the label and legend */

  if ( active )
    glColor4fv ( colour [ PUCOL_LEGEND ] ) ;
  else
    glColor4f ( colour [ PUCOL_LEGEND ][0],
                colour [ PUCOL_LEGEND ][1],
                colour [ PUCOL_LEGEND ][2],
                colour [ PUCOL_LEGEND ][3] / 2.0f ) ; /* 50% more transparent */

  if ( r_cb )
    r_cb ( this, dx, dy, render_data ) ;
  else
  {
    int xx ;
    switch ( getLegendPlace() )
    {
    case PUPLACE_LEFT :
      xx = PUSTR_LGAP ;
      break ;

    case PUPLACE_RIGHT :
      xx = abox.max[0] - abox.min[0] - legendFont.getStringWidth (legend) - PUSTR_LGAP ;
      break ;

    case PUPLACE_CENTERED :
    default :
      xx = ( abox.max[0] -
               abox.min[0] - legendFont.getStringWidth (legend) ) / 2 ;
      break ;
    }

    legendFont.drawString ( legend,
                  dx + abox.min[0] + xx,
                  dy + abox.min[1] + legendFont.getStringDescender() + PUSTR_BGAP ) ;
  }

  draw_label ( dx, dy ) ;
}


