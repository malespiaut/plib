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

// Callbacks from the internal widgets

static void puLargeInputHandleRightSlider ( puObject * slider )
{
  float val ;
  slider->getValue ( &val ) ;
  val = 1.0f - val ;

  puLargeInput* text = (puLargeInput*) slider->getUserData () ;
  int index = int ( text->getNumLines () * val ) ;
  text->setTopLineInWindow ( index ) ;
}

static void puLargeInputHandleArrow ( puObject *arrow )
{
  puSlider *slider = (puSlider *) arrow->getUserData () ;
  puLargeInput* text = (puLargeInput*) slider->getUserData () ;

  int type = ((puArrowButton *)arrow)->getArrowType() ;
  int inc = ( type == PUARROW_DOWN     ) ?   1 :
            ( type == PUARROW_UP       ) ?  -1 :
            ( type == PUARROW_FASTDOWN ) ?  10 :
            ( type == PUARROW_FASTUP   ) ? -10 : 0 ;

  float val ;
  slider->getValue ( &val ) ;
  val = 1.0f - val ;
  int num_lines = text->getNumLines () ;
  if ( num_lines > 0 )
  {
    int index = int ( num_lines * val + 0.5 ) + inc ;
    if ( index > num_lines ) index = num_lines ;
    if ( index < 0 ) index = 0 ;

    slider->setValue ( 1.0f - (float)index / num_lines ) ;
    text->setTopLineInWindow ( index ) ;
  }
}

// Private function from the widget itself

void puLargeInput::normalize_cursors ( void )
{
  char *val = getText () ;
  int sl = strlen ( val ) ;

  // Clamp the positions to the limits of the text.

  if ( cursor_position       <  0 ) cursor_position       =  0 ;
  if ( select_start_position <  0 ) select_start_position =  0 ;
  if ( select_end_position   <  0 ) select_end_position   =  0 ;
  if ( cursor_position       > sl ) cursor_position       = sl ;
  if ( select_start_position > sl ) select_start_position = sl ;
  if ( select_end_position   > sl ) select_end_position   = sl ;

  // Swap the ends of the select window if they get crossed over

  if ( select_end_position < select_start_position )
  {
    int tmp = select_end_position ;     
    select_end_position = select_start_position ;     
    select_start_position = tmp ;
  }

  // Set the top line in the window so that the last line is at the bottom of the window

  if ( top_line_in_window > num_lines - lines_in_window + 2 )
    top_line_in_window = num_lines - lines_in_window + 2 ;

  if ( top_line_in_window < 0 ) top_line_in_window = 0 ;
}

void puLargeInput::removeSelectRegion ( void )
{
  char *p = new char [ strlen ( text ) + 1 -
                           ( select_end_position - select_start_position ) ] ;
  strncpy ( p, text, select_start_position ) ;
  p [ select_start_position ] = '\0' ;
  strcat ( p, (text + select_end_position ) ) ;
  strcpy ( text, p ) ;
  delete p ;

  cursor_position = select_start_position ;
  select_end_position = select_start_position ;
}


// Public functions from the widget itself

puLargeInput::puLargeInput ( int x, int y, int w, int h, int arrows, int sl_width ) :
                           puGroup ( x, y )
{
  setLegendFont ( PUFONT_8_BY_13 ) ;  // Constant pitch font

  // Set the variables

  type |= PUCLASS_LARGEINPUT ;
  num_lines = 1 ;
  slider_width = sl_width ;
  lines_in_window = ( h - slider_width ) /
                    ( getLegendFont().getStringHeight() + getLegendFont().getStringDescender() + 1 ) ;
  top_line_in_window = 0 ;
  max_width = 0 ;

  accepting = FALSE ;
  cursor_position = 0 ;
  select_start_position = 0 ;
  select_end_position = -1 ;
  valid_data = NULL;

  if ( arrows > 2 ) arrows = 2 ;
  if ( arrows < 0 ) arrows = 0 ;
  arrow_count = arrows ;

  // Set up the widgets

  frame = new puFrame ( 0, 0, w, h );

  bottom_slider = new puSlider ( 0, 0, w - slider_width, 0, slider_width ) ,
  bottom_slider->setValue ( 0.0f ) ;   // All the way to the left
  bottom_slider->setDelta(0.1f);
  bottom_slider->setSliderFraction (1.0f) ;
  bottom_slider->setCBMode( PUSLIDER_DELTA );

  right_slider = new puSlider ( w - slider_width, slider_width*(1+arrows),
                                h - slider_width * ( 1 + 2 * arrows ), 1, slider_width ) ,
  right_slider->setValue ( 1.0f ) ;    // All the way to the top
  right_slider->setDelta(0.1f);
  right_slider->setSliderFraction (1.0f) ;
  right_slider->setCBMode( PUSLIDER_DELTA );
  right_slider->setUserData ( this ) ;
  right_slider->setCallback ( puLargeInputHandleRightSlider ) ;

  down_arrow = (puArrowButton *)NULL; fastdown_arrow = (puArrowButton *)NULL;
  up_arrow = (puArrowButton *)NULL; fastup_arrow = (puArrowButton *)NULL;

  if ( arrows > 0 )
  {
    down_arrow = new puArrowButton ( w-slider_width, slider_width*arrows, w, slider_width*(1+arrows), PUARROW_DOWN ) ;
    down_arrow->setUserData ( right_slider ) ;
    down_arrow->setCallback ( puLargeInputHandleArrow ) ;

    up_arrow = new puArrowButton ( w-slider_width, h-slider_width*arrows, w, h-slider_width*(arrows-1), PUARROW_UP ) ;
    up_arrow->setUserData ( right_slider ) ;
    up_arrow->setCallback ( puLargeInputHandleArrow ) ;
  }

  if ( arrows == 2 )
  {
    fastdown_arrow = new puArrowButton ( w-slider_width, slider_width, w, slider_width*2, PUARROW_FASTDOWN ) ;
    fastdown_arrow->setUserData ( right_slider ) ;
    fastdown_arrow->setCallback ( puLargeInputHandleArrow ) ;

    fastup_arrow = new puArrowButton ( w-slider_width, h-slider_width, w, h, PUARROW_FASTUP ) ;
    fastup_arrow->setUserData ( right_slider ) ;
    fastup_arrow->setCallback ( puLargeInputHandleArrow ) ;
  }

  input_disabled = FALSE ;

  text = NULL ;
  setText ( "\n" ) ;

  close  () ;
  reveal () ;
}

puLargeInput::~puLargeInput ()
{
  delete text ;
  if ( valid_data ) delete valid_data ;

  if ( puActiveWidget() == this )
    puDeactivateWidget () ;
}

void puLargeInput::setSize ( int w, int h )
{
  // Resize the frame:
  frame->setSize ( w, h ) ;

  // Resize and reposition the sliders
  if ( bottom_slider ) bottom_slider->setSize ( w - slider_width, slider_width ) ;

  right_slider->setPosition ( w-slider_width, slider_width*(1+arrow_count) ) ;
  right_slider->setSize ( slider_width, h-slider_width*(1+2*arrow_count) ) ;

  // Reposition the arrow buttons
  if ( down_arrow ) down_arrow->setPosition ( w-slider_width, slider_width*arrow_count ) ;
  if ( fastdown_arrow ) fastdown_arrow->setPosition ( w-slider_width, slider_width ) ;
  if ( up_arrow ) up_arrow->setPosition ( w-slider_width, h-slider_width*arrow_count ) ;
  if ( fastup_arrow ) fastup_arrow->setPosition ( w-slider_width, h-slider_width ) ;

  lines_in_window = ( h - slider_width ) /
                    ( getLegendFont().getStringHeight() + getLegendFont().getStringDescender() + 1 ) ;
}

void puLargeInput::setSelectRegion ( int s, int e )
{
  select_start_position = s ;
  select_end_position   = e ;
  char *lin_ptr = text ;
  int line_count = 0 ;
  while ( lin_ptr && ( lin_ptr <= text + select_start_position ) )  // Count the lines
  {
    line_count ++ ;
    lin_ptr = strchr ( lin_ptr+1, '\n' ) ;
  }

  if ( num_lines > lines_in_window )
  {
    if ( line_count < num_lines - lines_in_window )
    {
      top_line_in_window = line_count ;
      right_slider->setValue ( 1.0f - (float)line_count / (float)( num_lines - lines_in_window ) ) ;
    }
    else
    {
      top_line_in_window = num_lines - lines_in_window ;
      right_slider->setValue ( 0.0f ) ;
    }
  }
}

void  puLargeInput::selectEntireLine ( void )
{
  while ( ( select_start_position > 0 ) && ( *(text + select_start_position) != '\n' ) )
    select_start_position -- ;


  if ( select_start_position > 0 )
    select_start_position ++ ;

  select_end_position = int ( strchr ( text + select_end_position, '\n' ) + 1 - text ) ;
  if ( select_end_position == 1 ) select_end_position = strlen ( text ) ;
}

void  puLargeInput::addNewLine ( char *l )
{
  if ( cursor_position > 0 )  // If not at start of line, go to start of next line
    cursor_position = int (strchr ( text + cursor_position - 1, '\n' ) - text + 1) ;

  select_end_position = select_start_position = cursor_position ;
  addText ( l ) ;
}

void  puLargeInput::addText ( char *l )
{
  if ( !l ) return ;

  int length = strlen ( l ) + strlen ( text )  /* Length of the final string */
               + select_start_position - select_end_position + 2 ;
  if ( *(l+strlen(l)-1) == '\n' ) length -- ;           // Decrement "length" for each final
  if ( text[select_end_position] == '\n' ) length -- ;  // carriage return already there

  char *temp_text = new char [ length ] ;

  strncpy ( temp_text, text, select_start_position ) ;
  *(temp_text+select_start_position) = '\0' ;

  strcat ( temp_text, l ) ;
  if ( ( *(l+strlen(l)-1) == '\n' ) && ( text[select_end_position] == '\n' ) )
    temp_text[strlen(temp_text)-1] = '\0' ;  /* Erase the duplicate carriage return */
  else if ( ( *(l+strlen(l)-1) != '\n' ) && ( text[select_end_position] != '\n' ) )
    strcat ( temp_text, "\n" ) ;  /* Add a carriage return */

  strcat ( temp_text, (text+select_end_position) ) ;
  int temp_select_start = select_start_position ;
  setText ( temp_text ) ;
  delete temp_text ;
  setSelectRegion ( temp_select_start,
                    temp_select_start + strlen(l) ) ;
  setCursor ( select_end_position ) ;
}

void  puLargeInput::appendText ( char *l )
{
  if ( !l ) return ;

  int oldlen = strlen ( text ) ;
  if ( oldlen == 1 ) oldlen = 0 ;  /* Don't want null line at the beginning */
  int length = oldlen + strlen ( l ) + 2 ;
  if ( *(l+strlen(l)-1) == '\n' ) length -- ;  /* Already have a trailing carriage return, decrement the length */

  char *temp_text = new char [ length ] ;

  if ( oldlen > 0 )  /* More than just the empty carriage return */
    strcpy ( temp_text, text ) ;
  else
    temp_text[0] = '\0' ;

  strcat ( temp_text, l ) ;
  if ( *(l+strlen(l)-1) != '\n' )
    strcat ( temp_text, "\n" ) ;

  setText ( temp_text ) ;
  setSelectRegion ( oldlen, strlen(temp_text) ) ;
  setCursor ( oldlen ) ;
  delete temp_text ;
}

void  puLargeInput::removeText ( int start, int end )
{
  char *temp_text = new char [ strlen(text) + start - end + 1 ] ;
  strncpy ( temp_text, text, start ) ;
  temp_text[start] = '\0' ;
  strcat ( temp_text, text+end ) ;
  setText ( temp_text ) ;
  setCursor ( start ) ;
  delete temp_text ;
}

void  puLargeInput::setText ( char *l )
{
  if ( text )
  {
    delete text ;
    text = NULL ;
  }

  cursor_position = 0 ;
  select_start_position = select_end_position = 0 ;

  bottom_slider->setSliderFraction ( 0.0 ) ;
  right_slider->setSliderFraction ( 0.0 ) ;

  puRefresh = TRUE ;

  if ( !l )
  {
    text = new char [ 2 ] ;
    strcpy ( text, "\n" ) ;
    return ;
  }

  int length = strlen ( l ) + 2 ;
  if ( ( strlen(l) > 0 ) && ( *(l+strlen(l)-1) == '\n' ) )
    length -- ;  /* Already have a trailing carriage return, don't need to add one */

  text = new char [ length ] ;
  strcpy ( text, l ) ;
  if ( ( strlen(l) == 0 ) || ( *(l+strlen(l)-1) != '\n' ) )
    strcat ( text, "\n" ) ;

  // Find the greatest width of a line
  max_width = 0 ;

  int line_width = 0 ;       // Width of current line
  char *this_char = text ;   // Pointer to character in text
  num_lines = 0 ;

  while ( *this_char != '\0' )
  {
    char *line_end = strchr ( this_char, '\n' ) ;
    if ( line_end )  // Found an end-of-line
    {
      *line_end = '\0' ;  // Temporary break in line
      line_width = legendFont.getStringWidth ( this_char ) ;
      *line_end = '\n' ;  // Reset the carriage return

      if ( max_width < line_width )
        max_width = line_width ;

      num_lines++ ;                 // Increment line counter

      this_char = line_end + 1 ;
    }
    else  // No carriage return.  Since a carriage return should be the last character,
      this_char++ ;        // we should not get here.
  }

  if ( max_width < line_width )
    max_width = line_width ;

  // Set slider fractions

  int line_size = legendFont.getStringHeight () +     // Height of a line
                  legendFont.getStringDescender() ;  // of text, in pixels

  int box_width = abox.max[0] - abox.min[0] - slider_width ;   // Input box width, in pixels
  int box_height = ( abox.max[1] - abox.min[1] - slider_width ) / line_size ;
                                                // Input box height, in lines

  bottom_slider->setSliderFraction ( (float)box_width / (float)max_width ) ;
  right_slider->setSliderFraction ( (float)box_height / (float)num_lines ) ;
}


void puLargeInput::draw ( int dx, int dy )
{
  if ( !visible || ( window != puGetWindow () ) ) return ;
  normalize_cursors () ;

  // 3D Input boxes look nicest if they are always in inverse style.

  abox.draw ( dx, dy, ( (style==PUSTYLE_SMALL_BEVELLED ||
                         style==PUSTYLE_SMALL_SHADED) ) ? -style :
                        (accepting ? -style : style ), colour, FALSE, border_thickness ) ;

  if ( r_cb )
    r_cb ( this, dx, dy, render_data ) ;
  else
  {
    // Draw the frame widget

    int xwidget = abox.min[0] + dx ;
    int ywidget = abox.min[1] + dy ;
    frame->draw ( xwidget, ywidget ) ;

    // Calculate window parameters:

    int line_size = legendFont.getStringHeight () +         // Height of a line
                    legendFont.getStringDescender() + 1 ;  // of text, in pixels

    int box_width = abox.max[0] - abox.min[0] - slider_width ;   // Input box width, in pixels
    int box_height = ( abox.max[1] - abox.min[1] - slider_width ) / line_size ;
                                                  // Input box height, in lines

    float bottom_value ;
    bottom_slider->getValue ( &bottom_value ) ;
    float right_value ;
    right_slider->getValue ( &right_value ) ;

    int beg_pos      // Position in window of start of line, in pixels
                = (int)(( box_width - max_width ) * bottom_value ) ;
////  int end_pos      // Position in window of end of line, in pixels
////              = beg_pos + max_width - 1 ;
    if ( top_line_in_window < 0 ) top_line_in_window = 0 ;
    int end_lin      // Position on line count of bottom of window, in lines
                = top_line_in_window + box_height - 1 ;

    int xx = legendFont.getStringWidth ( " " ) ;
    int yy = (int)( abox.max[1] - abox.min[1] - legendFont.getStringHeight () * 1.5 ) ;

    if ( accepting )
    {
      char *val = getText () ;

      // Highlight the select area

      if ( select_end_position > 0 &&
           select_end_position != select_start_position )    
      {
        // First:  find the positions on the window of the selection start and end

        char temp_char = val[ select_start_position ] ;
        val [ select_start_position ] = '\0' ;

        xx = dx + abox.min[0] + legendFont.getStringWidth ( " " ) ;
        yy = (int)( abox.max[1] - abox.min[1] - legendFont.getStringHeight () * 1.5 
                + top_line_in_window * line_size ) ;   // Offset y-coord for unprinted lines

        char *end_of_line = strchr ( val, '\n' ) ;
        char *start_of_line = val;

        // Step down the lines until you reach the line with the selection start

        int select_start_line = 0 ;

        while ( end_of_line )
        {
          select_start_line++ ;
          start_of_line = end_of_line + 1 ;
          yy -= line_size ;
          end_of_line = strchr ( start_of_line, '\n' ) ;
        }

        int start_pos = legendFont.getStringWidth ( start_of_line ) + xx +
                        beg_pos ;   // Start of selection

        val [ select_start_position ] = temp_char ;

        // Now repeat the process for the end of the selection.

        temp_char = val[ select_end_position ] ;
        val [ select_end_position ] = '\0' ;

        end_of_line = strchr ( start_of_line, '\n' ) ;

        // Step down the lines until you reach the line with the selection end

        int select_end_line = select_start_line ;

        while ( end_of_line )
        {
          select_end_line++ ;
          start_of_line = end_of_line + 1 ;
          end_of_line = strchr ( start_of_line, '\n' ) ;
        }

        int end_pos = legendFont.getStringWidth ( start_of_line ) + xx +
                      beg_pos ;   // End of selection

        val [ select_end_position ] = temp_char ;

        // Now draw the selection area.

        for ( int line_count = select_start_line ; line_count <= select_end_line ;
                  line_count++ )
        {
          if ( line_count >= top_line_in_window )
          {
            int x_start, x_end ;

            if ( line_count == select_start_line )
              x_start = ( start_pos > xx ) ? start_pos : xx ;
            else
              x_start = xx ;

            x_start = ( x_start < abox.max[0] + dx ) ? x_start : abox.max[0] + dx ;

            if ( line_count == select_end_line )
              x_end = ( end_pos < abox.max[0] + dx ) ? end_pos : abox.max[0] + dx ;
            else
              x_end = abox.max[0] + dx ;

            x_end = ( x_end > xx ) ? x_end : xx ;

            int top = dy + abox.min[1] + yy + legendFont.getStringHeight () ;
            int bot = dy + abox.min[1] + yy - legendFont.getStringDescender() ;

            glColor3f ( 1.0f, 1.0f, 0.7f ) ;
            glRecti ( x_start, bot, x_end, top ) ;

            if ( line_count == end_lin ) break ;
          }

          yy -= line_size ;
        }
      }
    }

    // Draw the text

    {
      // If greyed out then halve the opacity when drawing the label and legend

      if ( active )
        glColor4fv ( colour [ PUCOL_LEGEND ] ) ;
      else
        glColor4f ( colour [ PUCOL_LEGEND ][0],
                    colour [ PUCOL_LEGEND ][1],
                    colour [ PUCOL_LEGEND ][2],
                    colour [ PUCOL_LEGEND ][3] / 2.0f ) ; // 50% more transparent

      char *val ;                   // Pointer to the actual text in the box
      val = getText () ;

      if ( val )
      {
        char *end_of_line = strchr (val, '\n') ;
        int line_count = 0;

        xx = legendFont.getStringWidth ( " " ) ;
        yy = (int)( abox.max[1] - abox.min[1] - legendFont.getStringHeight () * 1.5 ) ;

        while (end_of_line)  // While there is a carriage return in the string
        {
          if ( line_count < top_line_in_window )
          {                                        // Before the start of the window
            val = end_of_line + 1 ;
            end_of_line = strchr (val, '\n') ;     // Just go to the next line
          }
          else if ( line_count <= end_lin )        // Within the window, draw it
          {
            char temp_char = *end_of_line ;   // Temporary holder for last char on line

            *end_of_line = '\0' ;     // Make end-of-line be an end-of-string

            int beg_pos      // Position in window of start of line, in pixels
                    = (int)( ( box_width - max_width ) * bottom_value ) ;
            int end_pos      // Position in window of end of line, in pixels
                    = (int)( beg_pos + legendFont.getStringWidth ( val ) ) ;

            while ( ( beg_pos < 0 ) && ( val < end_of_line ) )   // Step down line
            {                                                    // until it is in the window
              char chr = *(val+1) ;
              *(val+1) = '\0' ;
              beg_pos += legendFont.getStringWidth ( val ) ;
              *(val+1) = chr ;
              val++ ;
            }

            while ( end_pos > box_width )  // Step up the line until it is in the window
            {
              *end_of_line = temp_char ;
              end_of_line--;
              temp_char = *end_of_line ;
              *end_of_line = '\0' ;
              end_pos = beg_pos + legendFont.getStringWidth ( val ) ;
            }

            if ( val < end_of_line )                 // If any text shows in the window,
              legendFont.drawString ( val,           // draw it.
                                      dx + abox.min[0] + xx + beg_pos,
                                      dy + abox.min[1] + yy ) ;

            *end_of_line = temp_char ;     // Restore the end-of-line character

            if ( temp_char != '\n' )               // If we had to step up from the end of
              end_of_line = strchr (val, '\n') ;   // the line, go back to the actual end

            yy -= line_size ;
            val = end_of_line + 1 ;
            end_of_line = strchr (val, '\n') ;     // On to the next line
          }
          else if ( line_count > end_lin )        // Have gone beyond window, end process
            end_of_line = NULL ;

          line_count ++ ;

        }     // while ( end_of_line )
      }     // if ( val )
    }

    if ( accepting )
    { 
      char *val ;                   // Pointer to the actual text in the box
      val = getText () ;

      // Draw the 'I' bar cursor.

      if ( val && ( cursor_position >= 0 ) )
      {
        char temp_char = val[ cursor_position ] ;
        val [ cursor_position ] = '\0' ;

        xx = legendFont.getStringWidth ( " " ) ;
        yy = (int)( abox.max[1] - abox.min[1] - legendFont.getStringHeight () * 1.5 
                + top_line_in_window * line_size ) ;   // Offset y-coord for unprinted lines

        char *end_of_line = strchr ( val, '\n' ) ;
        char *start_of_line = val;

        // Step down the lines until you reach the line with the cursor

        int line_count = 1 ;

        while ( end_of_line )
        {
          line_count++ ;
          start_of_line = end_of_line + 1 ;
          yy -= line_size ;
          end_of_line = strchr ( start_of_line, '\n' ) ;
        }

        if ( ( line_count > top_line_in_window ) && ( line_count <= end_lin+1 ) )
        {
          int beg_pos      // Position in window of start of line, in pixels
                    = (int)( ( box_width - max_width ) * bottom_value ) ;

          int cpos = (int)( legendFont.getStringWidth ( start_of_line ) + xx +
                     abox.min[0] + beg_pos ) ;
          int top = (int)( abox.min[1] + yy + legendFont.getStringHeight () ) ;
          int bot = (int)( abox.min[1] + yy - legendFont.getStringDescender () ) ;

          if ( ( cpos > abox.min[0] ) && ( cpos < abox.max[0] ) )
          {
            glColor3f ( 0.1f, 0.1f, 1.0f ) ;
            glBegin   ( GL_LINES ) ;
            glVertex2i ( dx + cpos    , dy + bot ) ;
            glVertex2i ( dx + cpos    , dy + top ) ;
            glVertex2i ( dx + cpos - 1, dy + bot ) ;
            glVertex2i ( dx + cpos - 1, dy + top ) ;
            glVertex2i ( dx + cpos - 4, dy + bot ) ;
            glVertex2i ( dx + cpos + 3, dy + bot ) ;
            glVertex2i ( dx + cpos - 4, dy + top ) ;
            glVertex2i ( dx + cpos + 3, dy + top ) ;
            glEnd      () ;
          }
        }

        val[ cursor_position ] = temp_char ;
      }
    }

    // Draw the other widgets in the large input box

    if ( bottom_slider ) bottom_slider->draw ( xwidget, ywidget ) ;
    right_slider->draw ( xwidget, ywidget ) ;
    if ( up_arrow ) up_arrow->draw ( xwidget, ywidget ) ;
    if ( down_arrow ) down_arrow->draw ( xwidget, ywidget ) ;
    if ( fastup_arrow ) fastup_arrow->draw ( xwidget, ywidget ) ;
    if ( fastdown_arrow ) fastdown_arrow->draw ( xwidget, ywidget ) ;
  }

  draw_label ( dx, dy ) ;
}


int puLargeInput::checkHit ( int button, int updown, int x, int y )
{
  int xwidget = x - abox.min[0] ;
  int ywidget = y - abox.min[1] ;

  if ( bottom_slider )
  {
    if ( bottom_slider->checkHit ( button, updown, xwidget, ywidget ) ) return TRUE ;
  }

  if ( right_slider->checkHit ( button, updown, xwidget, ywidget ) ) return TRUE ;
  if ( up_arrow )
  {
    if ( up_arrow->checkHit ( button, updown, xwidget, ywidget ) ) return TRUE ;
    if ( down_arrow->checkHit ( button, updown, xwidget, ywidget ) ) return TRUE ;
  }

  if ( fastup_arrow )
  {
    if ( fastup_arrow->checkHit ( button, updown, xwidget, ywidget ) ) return TRUE ;
    if ( fastdown_arrow->checkHit ( button, updown, xwidget, ywidget ) ) return TRUE ;
  }

  if ( input_disabled ) return FALSE ;

  // If the user has clicked within the bottom slider or to its right, don't activate.

  if ( y < slider_width ) return FALSE ;

  if ( puObject::checkHit ( button, updown, x, y ) )
    return TRUE ;

  return FALSE ;
}


void puLargeInput::doHit ( int button, int updown, int x, int y )
{
  if ( puActiveWidget() && ( this != puActiveWidget() ) )
  {
    // Active widget exists and is not this one; call its down callback if it exists

    puActiveWidget()->invokeDownCallback () ;
    puDeactivateWidget () ;
  }

  if ( updown != PU_DRAG )
    puMoveToLast ( this );

  if ( button == PU_LEFT_BUTTON )
  {
    // Most GUI's activate a button on button-UP not button-DOWN.

    // Text and window parameters:

    int line_size = legendFont.getStringHeight () +         // Height of a line
                    legendFont.getStringDescender() + 1 ;  // of text, in pixels

    int box_width = abox.max[0] - abox.min[0] - slider_width ;   // Input box width, in pixels
//  int box_height = ( abox.max[1] - abox.min[1] ) / line_size ;
                                               // Input box height, in lines

    float bottom_value ;
    bottom_slider->getValue ( &bottom_value ) ;
    float right_value ;
    right_slider->getValue ( &right_value ) ;

    int beg_pos      // Position in window of start of line, in pixels
                = (int)( ( box_width - max_width ) * bottom_value ) ;
//  int end_pos      // Position in window of end of line, in pixels
//              = (int)( beg_pos + max_width - 1 ) ;
    if ( top_line_in_window < 0 ) top_line_in_window = 0 ;
//  int end_lin      // Position on line count of bottom of window, in lines
//              = top_line_in_window + box_height - 1 ;

//  int xx = legendFont.getStringWidth ( " " ) ;
    int yy = (int)( abox.max[1] - legendFont.getStringHeight () * 1.5 
            + top_line_in_window * line_size ) ;   // Offset y-coord for unprinted lines

    // Get the line number and position on the line of the mouse

    char *strval = getText () ;
    char *tmpval = new char [ strlen(strval) + 1 ] ;
    strcpy ( tmpval, strval ) ;

    int i = strlen ( tmpval ) ;

    char *end_of_line = strchr ( tmpval, '\n' ) ;
    char *start_of_line = tmpval;

    // Step down the lines until the y-coordinate is less than the mouse

    int line_count = 0 ;

    while ( ( yy > y ) && end_of_line )
    {
      line_count++ ;
      start_of_line = end_of_line + 1 ;
      yy -= line_size ;
      end_of_line = strchr ( start_of_line, '\n' ) ;
    }

    if ( end_of_line )
    {
      *end_of_line = '\0' ;

      i = strlen ( tmpval ) ;

      int length, prev_length = 0 ;
      while ( x <= (length = legendFont.getStringWidth ( start_of_line )
                    + abox.min[0] + beg_pos ) &&
              i >= 0 )
      {
        prev_length = length ;
        tmpval[--i] = '\0' ;
      }

      if ( ( x - length ) < ( prev_length - x ) )
        i-- ;   // Mouse is closer to previous character than next character

    }

    // Now process the mouse click itself.

    if ( updown == active_mouse_edge || active_mouse_edge == PU_UP_AND_DOWN )
    {
      lowlight () ;

      accepting = TRUE ;
      cursor_position = i ;
      normalize_cursors () ;
      puSetActiveWidget ( this, x, y ) ;
      invokeCallback () ;
    }
    else if ( updown == PU_DOWN )
    {
      // We get here if the active edge is not down but the mouse button has
      // been pressed.  Start a selection.

      select_start_position = i ;
      select_end_position = i ;
    }
    else if ( updown == PU_DRAG )
    {
      // Drag -- extend the selection.

      if ( (select_end_position - i) > (i - select_start_position) )
        select_start_position = i ;   // Cursor closer to start than to end
      else
        select_end_position = i ;     // Cursor closer to end than to start

      if (select_start_position > select_end_position)
      {
        i = select_end_position ;
        select_end_position = select_start_position ;
        select_start_position = i ;
      }

    }
    else
      highlight () ;
  }
  else
    lowlight () ;
}

int puLargeInput::checkKey ( int key, int /* updown */ )
{
  extern void puSetPasteBuffer ( char *ch ) ;
  extern char *puGetPasteBuffer () ;

  if ( input_disabled || !isAcceptingInput () || !isActive () || !isVisible () || ( window != puGetWindow () ) )
    return FALSE ;

  if ( puActiveWidget() && ( this != puActiveWidget() ) )
  {
    // Active widget exists and is not this one; call its down callback if it exists

    puActiveWidget()->invokeDownCallback () ;
    puDeactivateWidget () ;
  }

  if ( ! isAcceptingInput() )
    return FALSE ;

  normalize_cursors () ;

  char *old_text = getText () ;
  char *p = NULL ;
  int temp_cursor = cursor_position ;
  int i ;

  switch ( key )
  {
    case PU_KEY_PAGE_UP   :
    case PU_KEY_PAGE_DOWN :
    case PU_KEY_INSERT    : return FALSE ; 

    case PU_KEY_UP   :
    case PU_KEY_DOWN :
      i = 1 ;
      while ( ( old_text [ cursor_position - i ] != '\n' ) &&
              ( i < cursor_position ) )
        i++ ;            // Step back to the beginning of the line
      if ( i < cursor_position ) i-- ;

      if ( key == PU_KEY_UP )
      {
        // Step backwards to the beginning of the previous line
        cursor_position -= (i + 2) ;
        while ( ( old_text [ cursor_position ] != '\n' ) &&
                ( cursor_position > 0 ) )
          cursor_position-- ;
        if ( cursor_position > 0 ) cursor_position++ ;

        // Step down the line "i" spaces or to the end of the line
        while ( ( old_text [ cursor_position ] != '\n' ) &&
                ( i > 0 ) )
        {
          cursor_position++ ;
          i-- ;
        }
      }
      else   // Down-arrow key
      {
        // Skip to beginning of next line
        while ( old_text [ cursor_position ] != '\n' )
          cursor_position++ ;
        cursor_position++ ;

        // Step down the line "i" spaces or to the end of the line
        // or to the end of the text

        while ( ( old_text [ cursor_position ] != '\n' ) &&
                ( cursor_position < (int)strlen ( old_text ) ) &&
                ( i > 0 ) )
        {
          cursor_position++ ;
          i-- ;
        }
      }

      select_start_position = select_end_position = cursor_position ;
      break ;

    case 0x1B :  // ESC
    case '\t' :  // TAB  -- End of input
      rejectInput () ;
      normalize_cursors () ;
      invokeCallback () ;
      puDeactivateWidget () ;
      break ;

    case '\b' :  // Backspace
      if ( select_start_position != select_end_position )
        removeSelectRegion () ;
      else if ( cursor_position > 0 )
      {
        p = new char [ strlen(old_text) ] ;
        strncpy ( p, old_text, cursor_position ) ;
        p [ --cursor_position ] = '\0' ;
        strcat ( p, ( old_text + cursor_position + 1 ) ) ;
        setText ( p ) ;
        setCursor ( temp_cursor - 1 ) ;
        delete p ;
      }

      break ;

    case 0x7F :  // DEL
      if ( select_start_position != select_end_position )
        removeSelectRegion () ;
      else if (cursor_position != (int)strlen ( old_text ) )
      {
        p = new char [ strlen(old_text) ] ;
        strncpy ( p, old_text, cursor_position ) ;
        p [ cursor_position ] = '\0' ;
        strcat ( p, ( old_text + cursor_position + 1 ) ) ;
        setText ( p ) ;
        setCursor ( temp_cursor ) ;
        delete p ;
      }

      break ;

    case 0x15 /* ^U */ : string [ 0 ] = '\0' ; break ;
    case 0x03 /* ^C */ :
    case 0x18 /* ^X */ :  /* Cut or copy selected text */
      if ( select_start_position != select_end_position )
      {
        p = getText () ;
        char ch = p[select_end_position] ;
        p[select_end_position] = '\0' ;
        puSetPasteBuffer ( p + select_start_position ) ;
        p[select_end_position] = ch ;

        if ( key == 0x18 )  /* Cut, remove text from string */
          removeSelectRegion () ;
      }

      break ;

    case 0x16 /* ^V */ : /* Paste buffer into text */
      if ( select_start_position != select_end_position )
        removeSelectRegion () ;

      p = puGetPasteBuffer () ;
      if ( p )  // Make sure something has been cut previously!
      {
        p = new char [ strlen ( getText () ) + strlen ( p ) + 1 ] ;
        strncpy ( p, getText (), cursor_position ) ;
        p[cursor_position] = '\0' ;
        strcat ( p, puGetPasteBuffer () ) ;
        strcat ( p, getText() + cursor_position ) ;
        temp_cursor += strlen ( puGetPasteBuffer () ) ;
        setText ( p ) ;
        setCursor ( temp_cursor ) ;
        delete p ;
      }

      break ;

    case PU_KEY_HOME   :
      cursor_position = 0 ;
      select_start_position = select_end_position = cursor_position ;
      break ;
    case PU_KEY_END    :
      cursor_position = PUSTRING_MAX ;
      select_start_position = select_end_position = cursor_position ;
      break ;
    case PU_KEY_LEFT   :
      cursor_position-- ;
      select_start_position = select_end_position = cursor_position ;
      break ;
    case PU_KEY_RIGHT  :
      cursor_position++ ;
      select_start_position = select_end_position = cursor_position ;
      break ;

    default:
      if ( ( key < ' ' || key > 127 ) && ( key != '\n' )
                                      && ( key != '\r' ) ) return FALSE ;

      if ( key == '\r' ) key = '\n' ;

      if ( select_start_position != select_end_position ) // remove selected text
      {
        temp_cursor -= ( select_end_position - select_start_position ) ;
        removeSelectRegion () ;
      }

      p = new char [ strlen(old_text) + 2 ] ;

      strncpy ( p, old_text, cursor_position ) ;

      p [ cursor_position ] = key ;
      p [ cursor_position + 1 ] = '\0' ;

      strcat (p, ( old_text + cursor_position ) ) ;

      setText ( p ) ;
      setCursor ( temp_cursor + 1 ) ;
      delete p ;

      break ;
  }

  normalize_cursors () ;
  return TRUE ;
}

void puLargeInput::setStyle( int style )
{
  frame->setStyle ( style ) ;
  bottom_slider->setStyle ( style ) ;
  right_slider->setStyle ( style ) ;

  if ( down_arrow != NULL )
  {
    down_arrow->setStyle ( style ) ;
    up_arrow->setStyle ( style ) ;
    if ( fastdown_arrow != NULL )
    {
      fastdown_arrow->setStyle ( style ) ;
      fastup_arrow->setStyle ( style ) ;
    }
  }

  puObject::setStyle ( style ) ;
}

void puLargeInput::setColour( int which, float r, float g, float b, float  a )
{
  frame->setColour ( which, r, g, b, a ) ;
  bottom_slider->setColour ( which, r, g, b, a ) ;
  right_slider->setColour ( which, r, g, b, a ) ;

  if ( down_arrow != NULL )
  {
    down_arrow->setColour ( which, r, g, b, a ) ;
    up_arrow->setColour ( which, r, g, b, a ) ;
    if ( fastdown_arrow != NULL )
    {
      fastdown_arrow->setColour ( which, r, g, b, a ) ;
      fastup_arrow->setColour ( which, r, g, b, a ) ;
    }
  }

  puObject::setColour ( which, r, g, b, a ) ;
}

void puLargeInput::setBorderThickness( int t )
{
  frame->setBorderThickness ( t ) ;
  bottom_slider->setBorderThickness ( t ) ;
  right_slider->setBorderThickness ( t ) ;

  if ( down_arrow != NULL )
  {
    down_arrow->setBorderThickness ( t ) ;
    up_arrow->setBorderThickness ( t ) ;
    if ( fastdown_arrow != NULL )
    {
      fastdown_arrow->setBorderThickness ( t ) ;
      fastup_arrow->setBorderThickness ( t ) ;
    }
  }

  puObject::setBorderThickness ( t ) ;
}

