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

void puInput::removeSelectRegion ( void )
{
  char *p1 = getStringValue () + select_start_position ;
  char *p2 = getStringValue () + select_end_position   ;

  while ( *p1 != '\0' )
    *p1++ = *p2++ ;

  *p2 = '\0' ;

  cursor_position = select_start_position ;
  select_end_position = select_start_position ;
}


static char *chop_to_width ( puFont fnt, const char *s, int width, int *ncut )
{
  static char res [ PUSTRING_MAX ] ;
  int n = 0 ;
  int w ;

  do
  {
    strcpy ( res, & s[n] ) ;
    n++ ;
    w = fnt.getStringWidth ( res ) + 2 * PUSTR_RGAP + PUSTR_LGAP ;
  } while ( w >= width ) ;

  if ( ncut != NULL ) *ncut = n-1 ;

  return res ;
}

void puInput::draw ( int dx, int dy )
{
  normalize_cursors () ;

  if ( !visible || ( window != puGetWindow () ) ) return ;

  /* 3D Input boxes look nicest if they are always in inverse style. */

  abox.draw ( dx, dy, ( (style==PUSTYLE_SMALL_BEVELLED ||
                         style==PUSTYLE_SMALL_SHADED) ) ? -style :
                        (accepting ? -style : style ), colour, FALSE, border_thickness ) ;

  if ( r_cb )
    r_cb ( this, dx, dy, render_data ) ;
  else
  {
    int xx = legendFont.getStringWidth ( " " ) ;
    int yy = ( abox.max[1] - abox.min[1] - legendFont.getStringHeight () ) / 2 ;

    int ncut ;
    char *s2 ;

    if ( accepting )
    {
      /* Highlight the select area */

      if ( select_end_position > 0 &&
           select_end_position != select_start_position )    
      {
        s2 = chop_to_width ( legendFont, getStringValue(), abox.max[0]-abox.min[0], &ncut ) ;

        int sep = select_end_position   - ncut ;
        int ssp = select_start_position - ncut ;

        if ( sep < 0 ) sep = 0 ;
        if ( ssp < sep )
        {
          s2 [ sep ] = '\0' ;
          int cpos2 = legendFont.getStringWidth ( s2 ) +
                                                  xx + dx + abox.min[0] ;
          s2 [ ssp ] = '\0' ;
          int cpos1 = legendFont.getStringWidth ( s2 ) +
                                                  xx + dx + abox.min[0] ;

          glColor3f ( 1.0f, 1.0f, 0.7f ) ;
          glRecti ( cpos1, dy + abox.min[1] + 2 ,
                    cpos2, dy + abox.max[1] - 2 ) ;
        }

      }
    }

    /* Draw the text */

    /* If greyed out then halve the opacity when drawing the text */

    if ( active )
      glColor4fv ( colour [ PUCOL_LEGEND ] ) ;
    else
      glColor4f ( colour [ PUCOL_LEGEND ][0],
                  colour [ PUCOL_LEGEND ][1],
                  colour [ PUCOL_LEGEND ][2],
                  colour [ PUCOL_LEGEND ][3] / 2.0f ) ; /* 50% more transp */

    s2 = chop_to_width ( legendFont, getStringValue(),
                         abox.max[0]-abox.min[0], &ncut ) ;

    legendFont.drawString ( s2,
                  dx + abox.min[0] + xx,
                  dy + abox.min[1] + yy ) ;


    if ( accepting )
    { 
      /* Draw the 'I' bar cursor. */

      if ( cursor_position - ncut >= 0 )
      {
        s2 [ cursor_position-ncut ] = '\0' ;

        int cpos = legendFont.getStringWidth ( s2 ) + xx + dx + abox.min[0] ;
        int top = yy + legendFont.getStringHeight () ;
        int bot = yy - legendFont.getStringDescender() ;

        glColor4fv ( colour [ PUCOL_MISC ] ) ;
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

  draw_label ( dx, dy ) ;
}


void puInput::doHit ( int button, int updown, int x, int y )
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

    int ncut ;
    char *s2 = chop_to_width ( legendFont, getStringValue(),
                         abox.max[0]-abox.min[0], &ncut ) ;
    int i = strlen ( s2 ) ;

    int length, prev_length ;
    length = legendFont.getStringWidth ( s2 ) + abox.min[0] ;
    prev_length = length ;

    while ( ( x <= prev_length ) && ( i >= 0 ) )
    {
      prev_length = length ;
      s2[--i] = '\0' ;
      length = legendFont.getStringWidth ( s2 ) + abox.min[0] ;
    }

    if ( ( x - length ) > ( prev_length - x ) )
      i++ ;   /* Mouse is closer to next character than previous character */

    i += ncut ;

    /* Process the mouse click. */

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
  extern void puSetPasteBuffer  ( char *ch ) ;
  extern char *puGetPasteBuffer ( void ) ;

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

    case PU_KEY_HOME   : cursor_position = 0 ; break ;
    case PU_KEY_END    : cursor_position = PUSTRING_MAX ; break ;
    case PU_KEY_LEFT   : cursor_position-- ; break ;
    case PU_KEY_RIGHT  : cursor_position++ ; break ;
  }

  if ( ! input_disabled )
  {
    switch ( key )
    {
      case '\b' : /* Backspace */
        if ( select_start_position != select_end_position )
          removeSelectRegion () ;
        else if ( cursor_position > 0 ) 
          for ( p = & (getStringValue() [ --cursor_position ]) ; *p != '\0' ; p++ )
            *p = *(p+1) ;
        break ;

      case 0x7F : /* DEL */
        if ( select_start_position != select_end_position )
          removeSelectRegion () ;
        else if ( cursor_position != (int)strlen ( getStringValue() ) )
          for ( p = & (getStringValue() [ cursor_position ]) ; *p != '\0' ; p++ )
            *p = *(p+1) ;
        break ;

      case 0x15 /* ^U */ : (getStringValue() [ 0 ]) = '\0' ; break ;
      case 0x03 /* ^C */ :
      case 0x18 /* ^X */ :  /* Cut or copy selected text */
        if ( select_start_position != select_end_position )
        {
          p = getStringValue () ;
          char ch = p[select_end_position] ;
          p[select_end_position] = '\0' ;
          puSetPasteBuffer ( p + select_start_position ) ;
          p[select_end_position] = ch ;

          if ( key == 0x18 )  /* Cut, remove text from string */
            removeSelectRegion () ;
        }

        break ;

      case 0x16 /* ^V */ : /* Paste buffer into text */
        if ( ( strlen ( getStringValue () ) + strlen ( puGetPasteBuffer () ) ) < PUSTRING_MAX - 1 )
        {
          if ( select_start_position != select_end_position )
            removeSelectRegion () ;

          p = new char [ PUSTRING_MAX ] ;
          strcpy ( p, getStringValue() + cursor_position ) ;
          *(getStringValue() + cursor_position) = '\0' ;
          strcat ( getStringValue(), puGetPasteBuffer () ) ;
          strcat ( getStringValue(), p ) ;
          cursor_position += strlen ( puGetPasteBuffer () ) ;
          delete p ;
        }

        break ;

      default:
        if ( key < ' ' || key > 127 ) return FALSE ;
        if ( valid_data )
        {
          if ( !strchr ( valid_data, key ) ) return TRUE ;
        }

        if ( select_start_position != select_end_position ) // remove selected text
          removeSelectRegion () ;

        if ( strlen ( getStringValue() ) >= PUSTRING_MAX - 1 )
          return FALSE ;

        for ( p = & (getStringValue() [ strlen(getStringValue()) ]) ;
                 p != &(getStringValue()[cursor_position-1]) ; p-- )
          *(p+1) = *p ;

        *(p+1) = key ;
        cursor_position++ ;
        break ;
    }

    setValue ( getStringValue () ) ;
  }

  normalize_cursors () ;
  return TRUE ;
}


