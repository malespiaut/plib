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
     License along with this library; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 
     For further information visit http://plib.sourceforge.net

     $Id$
*/



#include "puLocal.h"

void puButton::draw ( int dx, int dy )
{
  if ( !visible || ( window != puGetWindow () ) ) return ;

  /* If button is pushed or highlighted - use inverse style for button itself */

  if ( getValue() ^ highlighted )
  {
    if ( parent && ( ( parent->getType() & PUCLASS_POPUPMENU ) ||
                     ( parent->getType() & PUCLASS_MENUBAR   ) ) )
      abox.draw ( dx, dy, PUSTYLE_SMALL_SHADED, colour, isReturnDefault(), 2 ) ;
    else
      abox.draw ( dx, dy, -style, colour, isReturnDefault(), border_thickness ) ;
  }
  else
    abox.draw ( dx, dy, style, colour, isReturnDefault(), border_thickness ) ;

  if ( r_cb )
    r_cb ( this, dx, dy, render_data ) ;
  else
    draw_legend ( dx, dy ) ;

  draw_label ( dx, dy ) ;
}


void puButton::doHit ( int button, int updown, int x, int y )
{
  if ( puActiveWidget() && ( this != puActiveWidget() ) )
  {
    puActiveWidget() -> invokeDownCallback () ;
    puDeactivateWidget () ;
  }

  if ( updown != PU_DRAG )
    puMoveToLast ( this );

  if ( button == PU_LEFT_BUTTON )
  {
    if ( updown == active_mouse_edge || active_mouse_edge == PU_UP_AND_DOWN )
    {
      lowlight () ;
      setValue ( (int) ! getValue () ) ;
      puSetActiveWidget ( this, x, y ) ;
      invokeCallback () ;
    }
    else
      highlight () ;
  }
  else
    lowlight () ;
}


