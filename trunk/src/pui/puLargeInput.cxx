
#include "puLocal.h"

// Callbacks from the internal widgets

static void puLargeInputHandleRightSlider ( puObject * slider )
{
  float val ;
  slider -> getValue ( &val ) ;
  val = 1.0f - val ;

  puLargeInput* text = (puLargeInput*) slider -> getUserData () ;
  int index = int ( text -> getNumLines () * val ) ;
  text -> setTopLineInWindow ( index ) ;
}

static void puLargeInputHandleArrow ( puObject *arrow )
{
  puSlider *slider = (puSlider *) arrow->getUserData () ;
  puLargeInput* text = (puLargeInput*) slider -> getUserData () ;

  int type = ((puArrowButton *)arrow)->getArrowType() ;
  int inc = ( type == PUARROW_DOWN     ) ?   1 :
            ( type == PUARROW_UP       ) ?  -1 :
            ( type == PUARROW_FASTDOWN ) ?  10 :
            ( type == PUARROW_FASTUP   ) ? -10 : 0 ;

  float val ;
  slider -> getValue ( &val ) ;
  val = 1.0f - val ;
  int num_lines = text->getNumLines () ;
  if ( num_lines > 0 )
  {
    int index = int ( num_lines * val + 0.5 ) + inc ;
    if ( index > num_lines ) index = num_lines ;
    if ( index < 0 ) index = 0 ;

    slider -> setValue ( 1.0f - (float)index / num_lines ) ;
    text -> setTopLineInWindow ( index ) ;
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
}

// Public functions from the widget itself

puLargeInput::puLargeInput ( int x, int y, int w, int h, int arrows, int sl_width ) :
                           puGroup ( x, y )
{
  setLegendFont ( PUFONT_8_BY_13 ) ;  // Constant pitch font

  // Set the variables

  type |= PUCLASS_LARGEINPUT ;
  num_lines = 1 ;
  lines_in_window = ( h - slider_width ) /
                    ( getLegendFont().getStringHeight() + getLegendFont().getStringDescender() + 1 ) ;
  top_line_in_window = 0 ;
  max_width = 0 ;
  slider_width = sl_width ;

  accepting = FALSE ;
  cursor_position = 0 ;
  select_start_position = 0 ;
  select_end_position = -1 ;
  valid_data = NULL;

  if ( arrows > 2 ) arrows = 2 ;
  if ( arrows < 0 ) arrows = 0 ;
  arrow_count = arrows ;

  // Set up the widgets

//  new puFrame ( 0, 0, w, h );

  bottom_slider = new puSlider ( 0, 0, w - slider_width, 0, slider_width ) ,
  bottom_slider -> setValue ( 0.0f ) ;   // All the way to the left
  bottom_slider->setDelta(0.1f);
  bottom_slider->setSliderFraction (1.0f) ;
  bottom_slider->setCBMode( PUSLIDER_DELTA );

  right_slider = new puSlider ( w - slider_width, slider_width*(1+arrows),
                                h - slider_width * ( 1 + 2 * arrows ), 1, slider_width ) ,
  right_slider -> setValue ( 1.0f ) ;    // All the way to the top
  right_slider->setDelta(0.1f);
  right_slider->setSliderFraction (1.0f) ;
  right_slider->setCBMode( PUSLIDER_DELTA );
  right_slider -> setUserData ( this ) ;
  right_slider -> setCallback ( puLargeInputHandleRightSlider ) ;

  if ( arrows > 0 )
  {
    puArrowButton *down_arrow = new puArrowButton ( w-slider_width, slider_width*arrows, w, slider_width*(1+arrows), PUARROW_DOWN ) ;
    down_arrow->setUserData ( right_slider ) ;
    down_arrow->setCallback ( puLargeInputHandleArrow ) ;

    puArrowButton *up_arrow = new puArrowButton ( w-slider_width, h-slider_width*arrows, w, h-slider_width*(arrows-1), PUARROW_UP ) ;
    up_arrow->setUserData ( right_slider ) ;
    up_arrow->setCallback ( puLargeInputHandleArrow ) ;
  }

  if ( arrows == 2 )
  {
    puArrowButton *down_arrow = new puArrowButton ( w-slider_width, slider_width, w, slider_width*2, PUARROW_FASTDOWN ) ;
    down_arrow->setUserData ( right_slider ) ;
    down_arrow->setCallback ( puLargeInputHandleArrow ) ;

    puArrowButton *up_arrow = new puArrowButton ( w-slider_width, h-slider_width, w, h, PUARROW_FASTUP ) ;
    up_arrow->setUserData ( right_slider ) ;
    up_arrow->setCallback ( puLargeInputHandleArrow ) ;
  }

  text = NULL ;
  setText ( "\n" ) ;

  close  () ;
  reveal () ;
}

puLargeInput::~puLargeInput ()
{
  free ( text ) ;
  if ( valid_data ) free ( valid_data ) ;

  if ( puActiveWidget() == this )
    puDeactivateWidget () ;
}

void puLargeInput::setSize ( int w, int h )
{
  puObject *ob ;
  for ( ob = dlist; ob != NULL; ob = ob->next )
  {
/*     if ( ob->getType() & PUCLASS_FRAME )  // Resize the frame
      ob->setSize ( w, h ) ;
    else */ if ( ob->getType() & PUCLASS_SLIDER )  /* Resize and position the slider */
    {
      if ( ob == bottom_slider )
        ob->setSize ( w - slider_width, slider_width ) ;
      else  // Right slider
      {
        ob->setPosition ( w-slider_width, slider_width*(1+arrow_count) ) ;
        ob->setSize ( slider_width, h-slider_width*(1+2*arrow_count) ) ;
      }
    }
    else if ( ob->getType() & PUCLASS_ARROW )  /* Position the arrow buttons */
    {
      int type = ((puArrowButton *)ob)->getArrowType () ;
      if ( type == PUARROW_DOWN )
        ob->setPosition ( w-slider_width, slider_width*arrow_count ) ;
      else if ( type == PUARROW_FASTDOWN )
        ob->setPosition ( w-slider_width, slider_width ) ;
      else if ( type == PUARROW_UP )
        ob->setPosition ( w-slider_width, h-slider_width*arrow_count ) ;
      else  /* fast up */
        ob->setPosition ( w-slider_width, h-slider_width ) ;
    }
  }
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

void  puLargeInput::addNewLine ( char *l )
{
  select_end_position = cursor_position ;
  if ( select_end_position > 0 )
  {
    select_end_position -- ;
    while ( *(text+select_end_position) != '\n' )
      select_end_position ++ ;

    select_end_position ++ ;  // Step to start of next line
  }

  select_start_position = select_end_position ;
  addText ( l ) ;
}

void  puLargeInput::addText ( char *l )
{
  char *temp_text;

  if ( !l ) return ;

  if ( ( *(l+strlen(l)-1) == '\n' ) && ( text[select_end_position] == '\n' ) )
  {                 // Two carriage returns, only keep one
    temp_text = (char *)malloc ( sizeof (char) * ( strlen(text) + strlen(l)
                                    + select_start_position - select_end_position ) ) ;
    strncpy ( temp_text, text, select_start_position ) ;
    *(temp_text+select_start_position) = '\0' ;
    strcat ( temp_text, l ) ;
    temp_text[strlen(temp_text)-1] = '\0' ;  // Erase the carriage return
  }
  else if ( ( *(l+strlen(l)-1) == '\n' ) || ( text[select_end_position] == '\n' ) )
  {                 // Already have a trailing carriage return or positioned at carriage return
    temp_text = (char *)malloc ( sizeof (char) * ( strlen(text) + strlen(l) + 1
                                    + select_start_position - select_end_position ) ) ;
    strncpy ( temp_text, text, select_start_position ) ;
    *(temp_text+select_start_position) = '\0' ;
    strcat ( temp_text, l ) ;
  }
  else
  {                 // Add a carriage return to the end of the string
    temp_text = (char *)malloc ( sizeof (char) * ( strlen(text) + strlen(l) + 2
                                    + select_start_position - select_end_position ) ) ;
    strncpy ( temp_text, text, select_start_position ) ;
    *(temp_text+select_start_position) = '\0' ;
    strcat ( temp_text, l ) ;
    strcat ( temp_text, "\n" ) ;  // Add a carriage return
  }

  strcat ( temp_text, (text+select_end_position) ) ;  // All branches have the following code in common
  setText ( temp_text ) ;
  setSelectRegion ( select_start_position,
                    select_start_position + strlen(l) ) ;
  setCursor ( select_end_position ) ;
}

void  puLargeInput::appendText ( char *l )
{
  char *temp_text;

  if ( !l ) return ;

  int oldlen = strlen ( text ) ;
  if ( *(l+strlen(l)-1) == '\n' )
  {                 // Already have a trailing carriage return
    temp_text = (char *)malloc ( sizeof (char) * ( oldlen + strlen(l) + 1 ) ) ;
    if ( oldlen > 1 )  // More than just the empty carriage return
      strcpy ( temp_text, text ) ;
    else
      temp_text[0] = '\0' ;

    strcat ( temp_text, l ) ;
  }
  else
  {                 // Add a carriage return to the end of the string
    temp_text = (char *)malloc ( sizeof (char) * ( oldlen + strlen(l) + 2 ) ) ;
    if ( oldlen > 1 )  // More than just the empty carriage return
      strcpy ( temp_text, text ) ;
    else
      temp_text[0] = '\0' ;

    strcat ( temp_text, l ) ;
    strcat ( temp_text, "\n" ) ;
  }

  setText ( temp_text ) ;
  setSelectRegion ( oldlen, strlen(temp_text) ) ;
  free ( temp_text ) ;
}

void  puLargeInput::setText ( char *l )
{
  if ( text )
  {
    free ( text ) ;
    text = NULL ;
  }

  bottom_slider->setSliderFraction ( 0.0 ) ;
  right_slider->setSliderFraction ( 0.0 ) ;

  puRefresh = TRUE ;

  if ( !l )
  {
    text = (char *)malloc ( sizeof (char) * 2 ) ;
    strcpy ( text, "\n" ) ;
    return ;
  }

  if ( ( strlen(l) > 0 ) && ( *(l+strlen(l)-1) == '\n' ) )
  {                 // Already have a trailing carriage return
    text = (char *)malloc ( sizeof (char) * ( strlen(l) + 1 ) ) ;
    strcpy ( text, l ) ;
    strcat ( text, "\0" ) ;
  }
  else
  {                 // Add a carriage return to the end of the string
    text = (char *)malloc ( sizeof (char) * ( strlen(l) + 2 ) ) ;
    strcpy ( text, l ) ;
    strcat ( text, "\n" ) ;
    strcat ( text, "\0" ) ;
  }

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

  int line_size = puGetStringHeight ( legendFont ) +     // Height of a line
                  puGetStringDescender ( legendFont ) ;  // of text, in pixels

  int box_width = abox.max[0] - abox.min[0] - slider_width ;   // Input box width, in pixels
  int box_height = ( abox.max[1] - abox.min[1] ) / line_size ;
                                                // Input box height, in lines

  bottom_slider->setSliderFraction ( (float)box_width / (float)max_width ) ;
  right_slider->setSliderFraction ( (float)box_height / (float)num_lines ) ;
}


void puLargeInput::draw ( int dx, int dy )
{
  if ( !visible || ( window != puGetWindow () ) ) return ;
  normalize_cursors () ;

  // 3D Input boxes look nicest if they are always in inverse style.

  abox . draw ( dx, dy, ( (style==PUSTYLE_SMALL_BEVELLED ||
                           style==PUSTYLE_SMALL_SHADED) ) ? -style :
                        (accepting ? -style : style ), colour, FALSE ) ;

  // Calculate window parameters:

  int line_size = puGetStringHeight ( legendFont ) +         // Height of a line
                  puGetStringDescender ( legendFont ) + 1 ;  // of text, in pixels

  int box_width = abox.max[0] - abox.min[0] - slider_width ;   // Input box width, in pixels
  int box_height = ( abox.max[1] - abox.min[1] - slider_width ) / line_size ;
                                                // Input box height, in lines

  float bottom_value ;
  bottom_slider -> getValue ( &bottom_value ) ;
  float right_value ;
  right_slider -> getValue ( &right_value ) ;

  int beg_pos      // Position in window of start of line, in pixels
              = (int)(( box_width - max_width ) * bottom_value ) ;
//  int end_pos      // Position in window of end of line, in pixels
//              = beg_pos + max_width - 1 ;
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

          int top = dy + abox.min[1] + yy + puGetStringHeight ( legendFont ) ;
          int bot = dy + abox.min[1] + yy - puGetStringDescender ( legendFont ) ;

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
#ifdef PU_NOT_USING_GLUT
            beg_pos += fontSize [ *val ] ;
#else
            char chr = *(val+1) ;
            *(val+1) = '\0' ;
            beg_pos += legendFont.getStringWidth ( val ) ;
            *(val+1) = chr ;
#endif
            val++ ;
          }

          while ( end_pos >= box_width )  // Step up the line until it is in the window
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

    draw_label ( dx, dy ) ;
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

  puGroup::draw ( dx, dy ) ;
}


int puLargeInput::checkHit ( int button, int updown, int x, int y )
{
  if ( puGroup::checkHit ( button, updown, x, y ) )
    return TRUE ;

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

    puActiveWidget() -> invokeDownCallback () ;
    puDeactivateWidget () ;
  }

  if ( updown != PU_DRAG )
    puMoveToLast ( this );

  if ( button == PU_LEFT_BUTTON )
  {
    // Most GUI's activate a button on button-UP not button-DOWN.

    // Text and window parameters:

    int line_size = puGetStringHeight ( legendFont ) +         // Height of a line
                    puGetStringDescender ( legendFont ) + 1 ;  // of text, in pixels

    int box_width = abox.max[0] - abox.min[0] - slider_width ;   // Input box width, in pixels
//  int box_height = ( abox.max[1] - abox.min[1] ) / line_size ;
                                               // Input box height, in lines

    float bottom_value ;
    bottom_slider -> getValue ( &bottom_value ) ;
    float right_value ;
    right_slider -> getValue ( &right_value ) ;

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
      puSetActiveWidget ( this ) ;
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
  if ( !isAcceptingInput () || !isActive () || !isVisible () || ( window != puGetWindow () ) )
    return FALSE ;

  if ( puActiveWidget() && ( this != puActiveWidget() ) )
  {
    // Active widget exists and is not this one; call its down callback if it exists

    puActiveWidget() -> invokeDownCallback () ;
    puDeactivateWidget () ;
  }

  if ( ( key == '\t' ) &&  // Tab character -- change widgets
       isReturnDefault () )
  {
    // Tab character.  If just tab, make the next widget the default return;
    // if shift-tab, make the previous widget the default return.

//    if ( glutGetModifiers () & GLUT_ACTIVE_SHIFT )  // shift-tab
//    {
//      if ( prev_default )
//      {
//        prev_default -> makeReturnDefault ( TRUE ) ;
//        makeReturnDefault ( FALSE ) ;
//      }
//    }
//    else
//    {
//      if ( next_default )
//      {
//        next_default -> makeReturnDefault ( TRUE ) ;
//        makeReturnDefault ( FALSE ) ;
//      }
//    }

    return TRUE ;
  }

  if ( ! isAcceptingInput() )
    return FALSE ;

  normalize_cursors () ;

  char *old_text = getText () ;
  char *p = NULL ;
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
      {
        p = (char *)malloc ( sizeof (char) * ( strlen ( old_text ) + 1 -
                                 ( select_end_position - select_start_position ) ) ) ;
        strncpy ( p, old_text, select_start_position ) ;
        p [ select_start_position ] = '\0' ;
        strcat ( p, (old_text + select_end_position ) ) ;

        cursor_position = select_start_position ;
        select_end_position = select_start_position ;
      }
      else if ( cursor_position > 0 )
      {
        p = (char *)malloc ( sizeof (char) * ( strlen(old_text) ) ) ;
        strncpy ( p, old_text, cursor_position ) ;
        p [ --cursor_position ] = '\0' ;
        strcat ( p, ( old_text + cursor_position + 1 ) ) ;
      }

      setText ( p ) ;
      break ;

    case 0x7F :  // DEL
      if ( select_start_position != select_end_position )
      {
        p = (char *)malloc ( sizeof (char) * ( strlen ( old_text ) + 1 -
                                 ( select_end_position - select_start_position ) ) ) ;
        strncpy ( p, old_text, select_start_position ) ;
        p [ select_start_position ] = '\0' ;
        strcat ( p, (old_text + select_end_position ) ) ;

        cursor_position = select_start_position ;
        select_end_position = select_start_position ;
      }
      else if (cursor_position != (int)strlen ( old_text ) )
      {
        p = (char *)malloc ( sizeof (char) * ( strlen(old_text) ) ) ;
        strncpy ( p, old_text, cursor_position ) ;
        p [ cursor_position ] = '\0' ;
        strcat ( p, ( old_text + cursor_position + 1 ) ) ;
      }

      setText ( p ) ;
      break ;

    case 0x15          : string [ 0 ] = '\0' ; break ;  // ^U
    case PU_KEY_HOME   : cursor_position = 0 ; break ;
    case PU_KEY_END    : cursor_position = PUSTRING_MAX ; break ;
    case PU_KEY_LEFT   : cursor_position-- ; break ;
    case PU_KEY_RIGHT  : cursor_position++ ; break ;

    default:
      if ( ( key < ' ' || key > 127 ) && ( key != '\n' )
                                      && ( key != '\r' ) ) return FALSE ;

      if ( key == '\r' ) key = '\n' ;

      if ( select_start_position != select_end_position ) // remove selected text
      {
        p = (char *)malloc ( sizeof (char) * ( strlen ( old_text ) + 2 -
                                 ( select_end_position - select_start_position ) ) ) ;
        strncpy ( p, old_text, select_start_position ) ;
        p [ select_start_position ] = key ;
        p [ select_start_position + 1 ] = '\0' ;
        strcat ( p, (old_text + select_end_position ) ) ;
        setText ( p ) ;

        cursor_position = ++select_start_position ;
        select_end_position = select_start_position ;
      }
      else
      {
        p = (char *)malloc ( sizeof (char) * ( strlen(old_text) + 2 ) ) ;

        strncpy ( p, old_text, cursor_position ) ;

        p [ cursor_position ] = key ;
        p [ cursor_position + 1 ] = '\0' ;

        strcat (p, ( old_text + cursor_position ) ) ;

        setText ( p ) ;

        cursor_position++ ;
      }

      break ;
  }

  normalize_cursors () ;
  return TRUE ;
}


