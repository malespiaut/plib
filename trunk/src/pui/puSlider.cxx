/*
     PLIB - A Suite of Portable Game Libraries
     Copyright (C) 1998,2002  Steve Baker

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

UL_RTTI_DEF2(puSlider,puRange,puObject)


void puSlider::draw_slider_box ( int dx, int dy, const puBox &box, float val, const char *box_label )
{
  int sd, od ;
  if ( isVertical() ) { sd = 1 ; od = 0 ; } else { sd = 0 ; od = 1 ; }

  int sz = box.max [sd] - box.min [sd] ;  // Size of slider box, in pixels

  if ( val < 0.0f ) val = 0.0f ;
  if ( val > 1.0f ) val = 1.0f ;

  val *= (float) sz * (1.0f - slider_fraction) ;

  puBox bx ;

  bx.min [ sd ] = box.min [ sd ] + (int) val ;
  bx.max [ sd ] = (int) ( (float) bx.min [ sd ] + (float) sz * slider_fraction ) ;
  bx.min [ od ] = box.min [ od ] + 2 ;
  bx.max [ od ] = box.max [ od ] - 2 ;

  bx.draw ( dx, dy, PUSTYLE_SMALL_SHADED, colour, FALSE, 2 ) ;

  if ( box_label )
  {
    int xx ;
    int yy ;
    if ( isVertical () )  // Vertical slider, text goes to the right of it
    {
      xx = bx.max[0] + PUSTR_LGAP ;
      yy = ( bx.max[1] + bx.min[1] - legendFont.getStringHeight ( box_label ) ) / 2 ;
    }
    else  // Horizontal slider, text goes above it
    {
      xx = ( bx.max[0] + bx.min[0] - legendFont.getStringWidth ( box_label ) ) / 2 ;
      yy = bx.max[1] + PUSTR_BGAP ;
    }

    /* If greyed out then halve the opacity when drawing the label */

    if ( active )
      glColor4fv ( colour [ PUCOL_LABEL ] ) ;
    else
      glColor4f ( colour [ PUCOL_LABEL ][0],
                  colour [ PUCOL_LABEL ][1],
                  colour [ PUCOL_LABEL ][2],
                  colour [ PUCOL_LABEL ][3] / 2.0f ) ; /* 50% more transparent */

    legendFont.drawString ( box_label, dx + xx, dy + yy ) ;
  }
}

void puSlider::draw ( int dx, int dy )
{
  if ( !visible || ( window != puGetWindow () ) ) return ;

  if ( ( style == PUSTYLE_BEVELLED ) ||
       ( style == PUSTYLE_SHADED ) )
    abox.draw ( dx, dy, -PUSTYLE_BOXED, colour, FALSE, 2 ) ;
  else
    abox.draw ( dx, dy, -style, colour, FALSE, border_thickness ) ;

  if ( r_cb )
    r_cb ( this, dx, dy, render_data ) ;
  else
  {
    float val = getFloatValue () ;

    draw_slider_box ( dx, dy, abox, ( val - minimum_value ) / ( maximum_value - minimum_value ) ) ;

    draw_legend ( dx, dy ) ;
  }

  draw_label ( dx, dy ) ;
}


void puSlider::doHit ( int button, int updown, int x, int y )
{
  if ( puActiveWidget() && ( this != puActiveWidget() ) )
  {
    puActiveWidget() -> invokeDownCallback () ;
    puDeactivateWidget () ;
  }

  if ( updown != PU_DRAG )
    puMoveToLast ( this );

  if ( button != active_mouse_button )
    return ;

  if ( updown == PU_UP )
  {
    puDeactivateWidget () ;
    return ;
  }

  int sd = isVertical() ;
  int coord = sd ? y : x ;
  float range = maximum_value - minimum_value ;
  float last_value = clamp ( getFloatValue () );
  float next_value = last_value ;
  float norm_value = ( next_value - minimum_value ) / range ;
  int box_len = abox.max [sd] - abox.min [sd] ;

  if ( updown == PU_DOWN ) {
    int lower = abox.min [sd] + int(box_len * (1.0 - slider_fraction) * norm_value) ;
    int upper = lower + int(box_len * slider_fraction) ;  // upper/lower slider margin in pixels
    float page_step = getPageStepSize () ;

    start_offset = -1 ;
    if ( page_step_size <= 0.0f ) {      // old slider behavior
      start_offset = int(box_len * slider_fraction * 0.5f) ;
      updown = PU_DRAG ;

    } else if ( coord < lower ) {
      next_value -= page_step ;

    } else if ( coord > upper ) {
      next_value += page_step ;

    } else {                            // new slider behavior
      start_offset = coord - abox.min [sd] - int(box_len * (1.0 - slider_fraction) * norm_value) ;
      puSetActiveWidget ( this, x, y ) ;
      return;
    }
  }

  if ( updown == PU_DRAG && start_offset >= 0 ) {
    if ( box_len == 0 ) {
      norm_value = 0.5f ;
    } else if ( slider_fraction >= 1.0f ) {
      norm_value = 0.0f ;
    } else {
      norm_value = ( coord - abox.min[sd] - start_offset ) / ( box_len * (1.0f - slider_fraction) ) ;
    }
    next_value = norm_value * range + minimum_value ;
  }

  setValue ( checkStep ( clamp ( next_value ) ) ) ;

  if ( next_value == last_value ) return ;

  switch ( cb_mode )
  {
    case PUSLIDER_CLICK :
      if ( updown == active_mouse_edge )
      {
        last_cb_value = next_value ;
        puSetActiveWidget ( this, x, y ) ;
        invokeCallback () ;
      }
      break ;

    case PUSLIDER_DELTA : /* Deprecated! */
      if ( fabs ( last_cb_value - next_value ) >= cb_delta )
      {
        last_cb_value = next_value ;
        puSetActiveWidget ( this, x, y ) ;
        invokeCallback () ;
      }
      break ;

    case PUSLIDER_ALWAYS :
    default :
      last_cb_value = next_value ;
      puSetActiveWidget ( this, x, y ) ;
      invokeCallback () ;
      break ;
  }
}


void puSlider::setSliderFraction ( float f )
{
  int i = isVertical() ? 1 : 0 ;
  int sz = abox.max [i] - abox.min [i] ;  // Size of slider box, in pixels
  float minf = 8.0f / sz ;                // fraction that makes a 8px handle

  slider_fraction = (f<minf) ? minf : (f>1.0f) ? 1.0f : f ;
  puPostRefresh () ;
}

