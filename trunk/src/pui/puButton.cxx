

#include "puLocal.h"

void puButton::draw ( int dx, int dy )
{
  if ( !visible || ( window != puGetWindow () ) ) return ;

  /* If button is pushed or highlighted - use inverse style for button itself */

  int tempStyle;

  if ( parent && ( ( parent->getType() & PUCLASS_POPUPMENU ) ||
                   ( parent->getType() & PUCLASS_MENUBAR   ) ) )
    tempStyle =  ( getValue() ^ highlighted ) ? PUSTYLE_SMALL_SHADED : style ;
  else
    tempStyle =  ( getValue() ^ highlighted ) ? -style : style ;

  abox . draw ( dx, dy, tempStyle, colour, isReturnDefault() ) ;

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
    int xx, yy ;
    switch ( getLegendPlace() )
    {
    case PUPLACE_LEFT :
      xx = PUSTR_LGAP ;
      break ;

    case PUPLACE_RIGHT :
      xx = abox.max[0] - abox.min[0] - puGetStringWidth ( legendFont, legend ) - PUSTR_LGAP ;
      break ;

    case PUPLACE_CENTERED :
    default :
      xx = ( abox.max[0] -
               abox.min[0] - puGetStringWidth(legendFont,legend) ) / 2 ;
      break ;
    }

      yy = ( abox.max[1] -
               abox.min[1] - puGetStringHeight(legendFont) ) / 2 ;

    puDrawString ( legendFont, legend,
                  dx + abox.min[0] + xx,
                  dy + abox.min[1] + yy ) ;
  }

  draw_label ( dx, dy ) ;
}


void puButton::doHit ( int button, int updown, int, int )
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
      puSetActiveWidget ( this ) ;
      invokeCallback () ;
    }
    else
      highlight () ;
  }
  else
    lowlight () ;
}


