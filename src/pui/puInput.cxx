
#include "puLocal.h"

void puInput::normalize_cursors ( void )
{
  char val [ PUSTRING_MAX ] ;
  getValue ( val ) ;
  int sl = strlen ( val ) ;

  /* Clamp the positions to the limits of the text.  */

  if ( cursor_position       <  0 ) cursor_position       =  0 ;
  if ( select_start_position <  0 ) select_start_position =  0 ;
  if ( select_end_position   <  0 ) select_end_position   =  0 ;
  if ( cursor_position       > sl ) cursor_position       = sl ;
  if ( select_start_position > sl ) select_start_position = sl ;
  if ( select_end_position   > sl ) select_end_position   = sl ;

  /* Swap the ends of the select window if they get crossed over */

  if ( select_end_position < select_start_position )
  {
    int tmp = select_end_position ;     
    select_end_position = select_start_position ;     
    select_start_position = tmp ;
  }
}

void puInput::draw ( int dx, int dy )
{
  normalize_cursors () ;

  if ( !visible || ( window != puGetWindow () ) ) return ;

  /* 3D Input boxes look nicest if they are always in inverse style. */

  abox . draw ( dx, dy, ( (style==PUSTYLE_SMALL_BEVELLED ||
                           style==PUSTYLE_SMALL_SHADED) ) ? -style :
                        (accepting ? -style : style ), colour, FALSE ) ;

  int xx = puGetStringWidth ( legendFont, " " ) ;
  int yy = ( abox.max[1] - abox.min[1] - puGetStringHeight(legendFont) ) / 2 ;

  if ( accepting )
  {
    char val [ PUSTRING_MAX ] ;
    getValue ( val ) ;

    /* Highlight the select area */

    if ( select_end_position > 0 &&
         select_end_position != select_start_position )    
    {
      val [ select_end_position ] = '\0' ;
      int cpos2 = puGetStringWidth ( legendFont, val ) + xx + dx + abox.min[0] ;
      val [ select_start_position ] = '\0' ;
      int cpos1 = puGetStringWidth ( legendFont, val ) + xx + dx + abox.min[0] ;

      glColor3f ( 1.0f, 1.0f, 0.7f ) ;
      glRecti ( cpos1, dy + abox.min[1] + 6 ,
                cpos2, dy + abox.max[1] - 6 ) ;
    }
  }

  /* Draw the text */

  {
    /* If greyed out then halve the opacity when drawing the label and legend */

    if ( active )
      glColor4fv ( colour [ PUCOL_LEGEND ] ) ;
    else
      glColor4f ( colour [ PUCOL_LEGEND ][0],
		  colour [ PUCOL_LEGEND ][1],
		  colour [ PUCOL_LEGEND ][2],
		  colour [ PUCOL_LEGEND ][3] / 2.0f ) ; /* 50% more transparent */

    char val [ PUSTRING_MAX ] ;
    getValue ( val ) ;

    puDrawString ( legendFont, val,
                  dx + abox.min[0] + xx,
                  dy + abox.min[1] + yy ) ;

    draw_label ( dx, dy ) ;
  }

  if ( accepting )
  { 
    char val [ PUSTRING_MAX ] ;
    getValue ( val ) ;

    /* Draw the 'I' bar cursor. */

    if ( cursor_position >= 0 )
    {
      val [ cursor_position ] = '\0' ;

      int cpos = puGetStringWidth ( legendFont, val ) + xx + dx + abox.min[0] ;
      int top = yy + puGetStringHeight ( legendFont ) ;
      int bot = yy - puGetStringDescender ( legendFont ) ;

      glColor3f ( 0.1f, 0.1f, 1.0f ) ;
      glBegin   ( GL_LINES ) ;
      glVertex2i ( cpos    , dy + abox.min[1] + bot ) ;
      glVertex2i ( cpos    , dy + abox.min[1] + top ) ;
      glVertex2i ( cpos - 1, dy + abox.min[1] + bot ) ;
      glVertex2i ( cpos - 1, dy + abox.min[1] + top ) ;
      glVertex2i ( cpos - 4, dy + abox.min[1] + bot ) ;
      glVertex2i ( cpos + 3, dy + abox.min[1] + bot ) ;
      glVertex2i ( cpos - 4, dy + abox.min[1] + top ) ;
      glVertex2i ( cpos + 3, dy + abox.min[1] + top ) ;
      glEnd      () ;
    }
  }
}


void puInput::doHit ( int button, int updown, int x, int /* y */ )
{
  if ( puActiveWidget() && ( this != puActiveWidget() ) )
  {
    /* Active widget exists and is not this one; call its down callback if it exists */

    puActiveWidget() -> invokeDownCallback () ;
    puDeactivateWidget () ;
  }

  if ( updown != PU_DRAG )
    puMoveToLast ( this );

  if ( button == PU_LEFT_BUTTON )
  {
    /* Most GUI's activate a button on button-UP not button-DOWN. */

    /* Find the position of the mouse on the line of text */

    char *strval = getStringValue () ;
    char *tmpval = new char [ strlen(strval) + 1 ] ;
    strcpy ( tmpval, strval ) ;

    int i = strlen ( tmpval ) ;

    int length, prev_length ;
    length = puGetStringWidth ( legendFont, tmpval ) + abox.min[0] ;
    prev_length = length ;
    while ( ( x <= prev_length ) && ( i >= 0 ) )
    {
      prev_length = length ;
      tmpval[--i] = '\0' ;
      length = puGetStringWidth ( legendFont, tmpval ) + abox.min[0] ;
    }

    if ( ( x - length ) > ( prev_length - x ) )
      i++ ;   /* Mouse is closer to next character than previous character */

    /* Process the mouse click. */

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
      /*
       * We get here if the active edge is not down but the mouse button has
       * been pressed.  Start a selection if this isn't the initial activation
       * of the widget.
       */

      if ( this == puActiveWidget() )
      {
        select_start_position = i ;
        select_end_position = i ;
      }
    }
    else if ( updown == PU_DRAG )
    {

      if ( (select_end_position - i) > (i - select_start_position) )
        select_start_position = i ;   /* Cursor closer to start than to end */
      else
        select_end_position = i ;     /* Cursor closer to end than to start */

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

int puInput::checkKey ( int key, int /* updown */ )
{
  if ( ! isAcceptingInput() || ! isActive () || ! isVisible () || ( window != puGetWindow () ) )
    return FALSE ;

  if ( puActiveWidget() && ( this != puActiveWidget() ) )
  {
    /* Active widget exists and is not this one; call its down callback if it exists */

    puActiveWidget() -> invokeDownCallback () ;
    puDeactivateWidget () ;
  }

  normalize_cursors () ;

  char *p ;

  switch ( key )
  {
    case PU_KEY_PAGE_UP   :
    case PU_KEY_PAGE_DOWN :
    case PU_KEY_INSERT    : return FALSE ; 

    case PU_KEY_UP   :
    case PU_KEY_DOWN :
    case 0x1B /* ESC */ :
    case '\t' :
    case '\r' :
    case '\n' : /* Carriage return/Line Feed/TAB  -- End of input */
      rejectInput () ;
      normalize_cursors () ;
      invokeCallback () ;
      puDeactivateWidget () ;
      break ;

    case '\b' : /* Backspace */
      if ( select_start_position != select_end_position )
      {
        char *p1 = & (getStringValue() [ select_start_position ]) ;
        char *p2 = & (getStringValue() [ select_end_position   ]) ;

        while ( *p1 != '\0' )
          *p1++ = *p2++ ;

        *p2 = '\0' ;
        cursor_position = select_start_position ;
        select_end_position = select_start_position ;
      }
      else if ( cursor_position > 0 ) 
        for ( p = & (getStringValue() [ --cursor_position ]) ; *p != '\0' ; p++ )
          *p = *(p+1) ;
      break ;

    case 0x7F : /* DEL */
      if ( select_start_position != select_end_position )
      {
        char *p1 = & (getStringValue() [ select_start_position ]) ;
        char *p2 = & (getStringValue() [ select_end_position   ]) ;

        while ( *p1 != '\0' )
          *p1++ = *p2++ ;

        *p2 = '\0' ;
        cursor_position = select_start_position ;
        select_end_position = select_start_position ;
      }
      else if ( cursor_position != (int)strlen ( getStringValue() ) )
        for ( p = & (getStringValue() [ cursor_position ]) ; *p != '\0' ; p++ )
	        *p = *(p+1) ;
      break ;

    case 0x15 /* ^U */ : (getStringValue() [ 0 ]) = '\0' ; break ;
    case PU_KEY_HOME   : cursor_position = 0 ; break ;
    case PU_KEY_END    : cursor_position = PUSTRING_MAX ; break ;
    case PU_KEY_LEFT   : cursor_position-- ; break ;
    case PU_KEY_RIGHT  : cursor_position++ ; break ;

    default:
      if ( key < ' ' || key > 127 ) return FALSE ;
      if ( valid_data )
      {
        if ( !strchr ( valid_data, key ) ) return TRUE ;
      }

      if ( select_start_position != select_end_position ) // remove selected text
      {
        char *p1 = & (getStringValue() [ select_start_position ]) ;
        char *p2 = & (getStringValue() [ select_end_position   ]) ;

        while ( *p1 != '\0' )
          *p1++ = *p2++ ;

        *p2 = '\0' ;
        cursor_position = select_start_position ;
        select_end_position = select_start_position ;
      }

      if ( strlen ( getStringValue() ) >= PUSTRING_MAX - 1 )
        return FALSE ;

      for ( p = & (getStringValue() [ strlen(getStringValue()) ]) ;
               p != &(getStringValue()[cursor_position-1]) ; p-- )
        *(p+1) = *p ;

      *(p+1) = key ;
      cursor_position++ ;
      break ;
  }

  setValue ( getStringValue() ) ;
  normalize_cursors () ;
  return TRUE ;
}


