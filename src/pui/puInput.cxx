
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

      glColor3f ( 0.1f, 0.1f, 1.0f ) ;
      glBegin   ( GL_LINES ) ;
      glVertex2i ( cpos    , dy + abox.min[1] + 7 ) ;
      glVertex2i ( cpos    , dy + abox.max[1] - 7 ) ;
      glVertex2i ( cpos - 1, dy + abox.min[1] + 7 ) ;
      glVertex2i ( cpos - 1, dy + abox.max[1] - 7 ) ;
      glVertex2i ( cpos - 4, dy + abox.min[1] + 7 ) ;
      glVertex2i ( cpos + 3, dy + abox.min[1] + 7 ) ;
      glVertex2i ( cpos - 4, dy + abox.max[1] - 7 ) ;
      glVertex2i ( cpos + 3, dy + abox.max[1] - 7 ) ;
      glEnd      () ;
    }
  }
}


void puInput::doHit ( int button, int updown, int x, int /* y */ )
{
  if ( puActiveWidget() && ( this != puActiveWidget() ) )
  {
    puActiveWidget() -> invokeDownCallback () ;
    puDeactivateWidget () ;
  }

  if ( button == PU_LEFT_BUTTON )
  {
    /* Most GUI's activate a button on button-UP not button-DOWN. */

    if ( updown == active_mouse_edge || active_mouse_edge == PU_UP_AND_DOWN )
    {
      lowlight () ;

      char *strval ;
      getValue ( & strval ) ;
      char *tmpval = new char [ strlen(strval) + 1 ] ;
      strcpy ( tmpval, strval ) ;

      int i = strlen ( tmpval ) ;

      while ( x <= puGetStringWidth ( legendFont, tmpval ) + abox.min[0] &&
              i >= 0 )
        tmpval[--i] = '\0' ;
    
      accepting = TRUE ;
      cursor_position = i ;
      normalize_cursors () ;
      puSetActiveWidget ( this ) ;
      invokeCallback () ;
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
      if ( cursor_position > 0 ) 
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

        select_end_position = select_start_position ;
      }
      else
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

      if ( strlen ( getStringValue() ) >= PUSTRING_MAX - 1 )
        return FALSE ;

/*
  This code was:

      for ( p = & (getStringValue() [ strlen(getStringValue()) ]) ;
               p != &(getStringValue()[cursor_position]) ; p-- )
        *(p+1) = *p ;

  ...until Eero Pajarre <epajarre@koti.tpo.fi> said:

        EERO: The next loop is not too pretty,
        but I think the original code did not work properly.
        Especially it did not always move the trailing 0 
        of  the string (if insertion was to the end of string)
        losing the terminating 0 had nasty effects elsewhere

  ...thanks Eero!
*/
      p = & string [ strlen(string) ] ;
      do{
        *(p+1)=*p;
        p--;
      }while(p >= &string[cursor_position]);
      p++;
/* END */
      *p = key ;
      cursor_position++ ;
      break ;
  }

  setValue ( getStringValue() ) ;
  normalize_cursors () ;
  return TRUE ;
}


