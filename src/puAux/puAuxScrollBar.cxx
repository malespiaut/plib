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


#include "puAuxLocal.h"

UL_RTTI_DEF1(puaScrollBar,puSlider)


void puaScrollBar::draw ( int dx, int dy )
{
  extern void puDrawArrowButtonTriangle ( int pos_x, int pos_y, int size_x, int size_y,
                                          puColour colour, int arrow_type, int active ) ;

  if ( !visible || ( window != puGetWindow () ) ) return ;

  if ( r_cb )
    r_cb ( this, dx, dy, render_data ) ;
  else
  {
    puBox box = abox ;

    int width = isVertical () ? abox.max[0] - abox.min[0] : abox.max[1] - abox.min[1] ;

    /* Draw the arrow buttons */

    if ( arrow_count == 2 )  /* Double-arrow buttons */
    {
      if ( isVertical () )
      {
        box.min[1] = abox.max[1] - width ;
        box.draw ( dx, dy, style, colour, FALSE, border_thickness ) ;
        puDrawArrowButtonTriangle ( dx+box.min[0]+width/2, dy+box.min[1]+width/2, width, width, colour [ PUCOL_MISC ],
                                    PUARROW_FASTUP, active_arrow & FASTUP ) ;
        box.min[1] = abox.min[1] ;
        box.max[1] = box.min[1] + width ;
        box.draw ( dx, dy, style, colour, FALSE, border_thickness ) ;
        puDrawArrowButtonTriangle ( dx+box.min[0]+width/2, dy+box.min[1]+width/2, width, width, colour [ PUCOL_MISC ],
                                    PUARROW_FASTDOWN, active_arrow & FASTDOWN ) ;
      }
      else
      {
        box.min[0] = abox.max[0] - width ;
        box.draw ( dx, dy, style, colour, FALSE, border_thickness ) ;
        puDrawArrowButtonTriangle ( dx+box.min[0]+width/2, dy+box.min[1]+width/2, width, width, colour [ PUCOL_MISC ],
                                    PUARROW_FASTRIGHT, active_arrow & FASTUP ) ;
        box.min[0] = abox.min[0] ;
        box.max[0] = box.min[0] + width ;
        box.draw ( dx, dy, style, colour, FALSE, border_thickness ) ;
        puDrawArrowButtonTriangle ( dx+box.min[0]+width/2, dy+box.min[1]+width/2, width, width, colour [ PUCOL_MISC ],
                                    PUARROW_FASTLEFT, active_arrow & FASTDOWN ) ;
      }
    }

    if ( arrow_count > 0 )  /* Single-arrow buttons */
    {
      if ( isVertical () )
      {
        box.min[1] = abox.max[1] - arrow_count * width ;
        box.max[1] = box.min[1] + width ;
        box.draw ( dx, dy, style, colour, FALSE, border_thickness ) ;
        puDrawArrowButtonTriangle ( dx+box.min[0]+width/2, dy+box.min[1]+width/2, width, width, colour [ PUCOL_MISC ],
                                    PUARROW_UP, active_arrow & UP ) ;
        box.max[1] = abox.min[1] + arrow_count * width ;
        box.min[1] = box.max[1] - width ;
        box.draw ( dx, dy, style, colour, FALSE, border_thickness ) ;
        puDrawArrowButtonTriangle ( dx+box.min[0]+width/2, dy+box.min[1]+width/2, width, width, colour [ PUCOL_MISC ],
                                    PUARROW_DOWN, active_arrow & DOWN ) ;
      }
      else
      {
        box.min[0] = abox.max[0] - arrow_count * width ;
        box.max[0] = box.min[0] + width ;
        box.draw ( dx, dy, style, colour, FALSE, border_thickness ) ;
        puDrawArrowButtonTriangle ( dx+box.min[0]+width/2, dy+box.min[1]+width/2, width, width, colour [ PUCOL_MISC ],
                                    PUARROW_RIGHT, active_arrow & UP ) ;
        box.max[0] = abox.min[0] + arrow_count * width ;
        box.min[0] = box.max[0] - width ;
        box.draw ( dx, dy, style, colour, FALSE, border_thickness ) ;
        puDrawArrowButtonTriangle ( dx+box.min[0]+width/2, dy+box.min[1]+width/2, width, width, colour [ PUCOL_MISC ],
                                    PUARROW_LEFT, active_arrow & DOWN ) ;
      }
    }

    /* Draw the surrounding box */

    box.min[0] = abox.min[0] + ( isVertical () ? 0 : arrow_count * width ) ;
    box.max[0] = abox.max[0] - ( isVertical () ? 0 : arrow_count * width ) ;
    box.min[1] = abox.min[1] + ( isVertical () ? arrow_count * width : 0 ) ;
    box.max[1] = abox.max[1] - ( isVertical () ? arrow_count * width : 0 ) ;

    if ( ( style == PUSTYLE_BEVELLED ) ||
         ( style == PUSTYLE_SHADED ) )
      box.draw ( dx, dy, -PUSTYLE_BOXED, colour, FALSE, 2 ) ;
    else
      box.draw ( dx, dy, -style, colour, FALSE, border_thickness ) ;

    /* Draw the slider box */

    float val = getFloatValue () ;

    draw_slider_box ( dx, dy, box, ( val - getMinValue () ) / ( getMaxValue () - getMinValue () ) ) ;

    draw_legend ( dx, dy ) ;
  }

  draw_label ( dx, dy ) ;
}

int puaScrollBar::checkHit ( int button, int updown, int x, int y )
{
  if ( updown == PU_UP && ( button == PU_SCROLL_UP_BUTTON || button == PU_SCROLL_DOWN_BUTTON ) )
  {
    float last_value = clamp ( getFloatValue () ), next_value = last_value ;
    float range = maximum_value - minimum_value ;
    float line_step = line_step_size ;
    if ( line_step <= 0.0f ) line_step = range / 10.0f ;
    if ( button == PU_SCROLL_UP_BUTTON )
      next_value += line_step;
    else
      next_value -= line_step;
    setValue ( checkStep ( clamp ( next_value ) ) ) ;
    if ( next_value != last_value ) invokeCallback () ;
    return TRUE;
  }
  return puSlider::checkHit ( button, updown, x, y ) ;
}

void puaScrollBar::doHit ( int button, int updown, int x, int y )
{
  if ( puActiveWidget() && ( this != puActiveWidget() ) )
  {
    puActiveWidget() -> invokeDownCallback () ;
    puDeactivateWidget () ;
  }

  if ( updown != PU_DRAG )
    puMoveToLast ( this );

  if ( button != active_mouse_button )
    return;

  if ( updown == PU_UP )
  {
    active_arrow = NONE ;
    puDeactivateWidget () ;
    return ;
  }

  int sd = isVertical() ;
  int coord = sd ? y : x ;
  float range = maximum_value - minimum_value ;
  float last_value = clamp ( getFloatValue () ), next_value = last_value ;
  float norm_value = ( last_value - minimum_value ) / range ;
  int width = abox.max[!sd] - abox.min[!sd] ;
  int box_len = abox.max [sd] - abox.min [sd] - arrow_count * width * 2 ;

  if ( updown == PU_DOWN ) {
    int lower = abox.min [sd] + int(float(box_len) * (1.0 - slider_fraction) * norm_value)
                + arrow_count * width ;
    int upper = lower + int(float(box_len) * slider_fraction) ;  // upper/lower slider margin in pixels

    float line_step = line_step_size ;
    float page_step = page_step_size ;
    if ( line_step <= 0.0f ) line_step = range / 10.0f ;
    if ( page_step <= 0.0f ) page_step = range ;

    start_offset = -1 ;
    if ( arrow_count && coord < abox.min[sd] + width ) {                 // lowest button
      if (arrow_count == 2) {
        next_value -= page_step ;
        active_arrow |= FASTDOWN ;
      } else {
        next_value -= line_step ;
        active_arrow |= DOWN ;
      }

    } else if ( arrow_count == 2 && coord < abox.min[sd] + 2 * width ) { // 2nd low button
      next_value -= line_step ;
      active_arrow |= DOWN ;

    } else if ( arrow_count && coord > abox.max[sd] - width ) {          // highest button
      if (arrow_count == 2) {
        next_value += page_step ;
        active_arrow |= FASTUP ;
      } else {
        next_value += line_step ;
        active_arrow |= UP ;
      }

    } else if ( arrow_count == 2 && coord > abox.max[sd] - 2 * width ) { // 2nd high button
      next_value += line_step ;
      active_arrow |= UP ;

    } else if ( page_step_size <= 0.0f ) {                               // old slider behavior (jumping)
      start_offset = int(float(box_len) * slider_fraction * 0.5f) + 2 * arrow_count * width ;
      updown = PU_DRAG ;

    } else if (coord < lower) {                                          // lower background
      next_value -= page_step ;

    } else if (coord > upper) {                                          // upper background
      next_value += page_step ;

    } else {                                                             // slider handle (new behavior)
      start_offset = coord + arrow_count * width - abox.min [sd]
                     - int(float(box_len) * (1.0f - slider_fraction) * norm_value) ;
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
      norm_value = ( coord + arrow_count * width - abox.min[sd] - start_offset )
                   / ( box_len * (1.0f - slider_fraction) ) ;
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


