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

void puBiSlider::draw ( int dx, int dy )
{
  if ( !visible || ( window != puGetWindow () ) ) return ;

  // Draw the slider box itself

  abox . draw ( dx, dy,
                (style==PUSTYLE_BEVELLED||
                 style==PUSTYLE_SHADED) ? -PUSTYLE_BOXED : -style,
                colour, FALSE ) ;

  int sd, od ;

  if ( isVertical() ) { sd = 1 ; od = 0 ; } else { sd = 0 ; od = 1 ; }

//  int sz = abox.max [sd] - abox.min [sd] ;  // Size of slider box, in pixels

  // Draw the current_max slider and label it

  float val ;

  if ( getMaxValue() > getMinValue() )
    val = (float)(getCurrentMax() - getMinValue()) / (float)(getMaxValue() - getMinValue()) ;
  else
    val = 1.0f ;

  char str_value[10] ;
  sprintf (str_value, "%d", getCurrentMax() ) ;

  draw_slider_box ( dx, dy, val, str_value ) ;

  // Draw the current_min slider and label it

  if ( getMaxValue() > getMinValue() )
    val = (float)(getCurrentMin() - getMinValue()) / (float)(getMaxValue() - getMinValue()) ;
  else
    val = 0.0f ;

  sprintf (str_value, "%d", getCurrentMin() ) ;

  draw_slider_box ( dx, dy, val, str_value ) ;

  // If greyed out then halve the opacity when drawing the label and legend

  if ( active )
    glColor4fv ( colour [ PUCOL_LEGEND ] ) ;
  else
    glColor4f ( colour [ PUCOL_LEGEND ][0],
                colour [ PUCOL_LEGEND ][1],
                colour [ PUCOL_LEGEND ][2],
                colour [ PUCOL_LEGEND ][3] / 2.0f ) ; // 50% more transparent

  int xx = ( abox.max[0] - abox.min[0] - puGetStringWidth(legendFont,legend) ) / 2 ;
  int yy = ( abox.max[1] - abox.min[1] - puGetStringHeight(legendFont) ) / 2 ;

  puDrawString ( legendFont, legend,
                  dx + abox.min[0] + xx,
                  dy + abox.min[1] + yy ) ;

  draw_label ( dx, dy ) ;
}


void puBiSlider::doHit ( int button, int updown, int x, int y )
{
  if ( puActiveWidget() && ( this != puActiveWidget() ) )
  {
    puActiveWidget() -> invokeDownCallback () ;
    puDeactivateWidget () ;
  }

  if ( updown != PU_DRAG )
    puMoveToLast ( this );

  if ( button == PU_LEFT_BUTTON && updown == PU_UP )
  {
    setActiveButton ( 0 ) ;
    puDeactivateWidget () ;
    return ;
  }                                                                             

  if ( button == PU_LEFT_BUTTON )
  {
    int sd = isVertical() ;
    int sz = abox.max [sd] - abox.min [sd] ;
    int coord = isVertical() ? y : x ;

    float next_value ;

    if ( sz == 0 )
      next_value = 0.5f ;
    else
    {
      next_value = ( (float)coord - (float)abox.min[sd] - (float)sz * slider_fraction / 2.0f ) /
                   ( (float) sz * (1.0f - slider_fraction) ) ;
    }

    next_value = (next_value < 0.0f) ? 0.0f : (next_value > 1.0) ? 1.0f : next_value ;

    int new_value = getMinValue() + (int)( next_value * ( getMaxValue() - getMinValue() ) + 0.5 ) ;

    if ( ( getActiveButton() == 0 ) || ( updown == PU_DOWN ) )  // No currently-active slider, set whichever is closest
    {
      if ( (new_value-getCurrentMin()) < (getCurrentMax()-new_value) ) // Closest to current_min
      {
        setCurrentMin ( new_value ) ;
        setActiveButton ( 1 ) ;
      }
      else  // Closest to current_max
      {
        setCurrentMax ( new_value ) ;
        setActiveButton ( 2 ) ;
      }
    }
    else if ( getActiveButton() == 1 )  // Currently moving current_min
    {
      setCurrentMin ( new_value ) ;
      if ( getCurrentMax() < getCurrentMin() ) setCurrentMax ( getCurrentMin() ) ;
    }
    else if ( getActiveButton() == 2 )  // Currently moving current_max
    {
      setCurrentMax ( new_value ) ;
      if ( getCurrentMax() < getCurrentMin() ) setCurrentMin ( getCurrentMax() ) ;
    }


    switch ( cb_mode )
    {
      case PUSLIDER_CLICK :
        if ( updown == active_mouse_edge )
        {
          last_cb_value = next_value ;
          puSetActiveWidget ( this ) ;
          invokeCallback () ;
        }
        break ;

      case PUSLIDER_DELTA :
        if ( fabs ( last_cb_value - next_value ) >= cb_delta )
        {
          last_cb_value = next_value ;
          puSetActiveWidget ( this ) ;
          invokeCallback () ;
        }
        break ;

      case PUSLIDER_ALWAYS :
      default :
        last_cb_value = next_value ;
        puSetActiveWidget ( this ) ;
        invokeCallback () ;
        break ;
    }
  }
}
