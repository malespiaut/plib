
#include "puLocal.h"

puListBox::puListBox  ( int minx, int miny, int maxx, int maxy, char** _list ) :
               puButton ( minx, miny, maxx, maxy )
{
  type |= PUCLASS_LISTBOX ;

  list = _list ;
  for ( num = 0 ; list [ num ] != NULL ; num++ )
    /* Count number of items */ ;
  top = 0 ;

  /* Set index of selected item */
  setValue ( -1 ) ;
}


void puListBox::setTopItem( int item_index )
{
  top = item_index ;
  if ( top < 0 )
    top = 0 ;
  else if ( top > num-1 )
    top = num-1;
}


void puListBox::draw ( int dx, int dy )
{
  if ( !visible || ( window != puGetWindow () ) ) return ;

  abox . draw ( dx, dy, style, colour, isReturnDefault() ) ;

  /* If greyed out then halve the opacity when drawing the label and legend */

  if ( active )
    glColor4fv ( colour [ PUCOL_LEGEND ] ) ;
  else
    glColor4f ( colour [ PUCOL_LEGEND ][0],
                colour [ PUCOL_LEGEND ][1],
                colour [ PUCOL_LEGEND ][2],
                colour [ PUCOL_LEGEND ][3] / 2.0f ) ; /* 50% more transparent */

  if ( r_cb )
    r_cb ( this, render_data ) ;
  else
  {
    int ysize = abox.max[1] - abox.min[1] + 1 ;
    int yinc = puGetStringHeight(legendFont) + PUSTR_BGAP ;
    int num_vis = (ysize - PUSTR_BGAP) / yinc ;

    int selected ;
    getValue ( &selected ) ;

    for ( int i=top; i<num && i<top+num_vis; i++ )
    {
      if ( i == selected )
        glColor4f ( 1.0, 1.0, 1.0, 1.0 ) ;
      else
        glColor4f ( 0.0, 0.0, 0.0, 1.0 ) ;

      int x = PUSTR_LGAP ;
      int y = PUSTR_BGAP + yinc * ((i-top)+1) ;

      int xx = dx + abox.min[0] + x ;
      int yy = dy + abox.max[1] - y ;

      const char* str = list [ i ] ;
      puDrawString ( legendFont, (char*)str, xx, yy ) ;
    }
  }

  draw_label ( dx, dy ) ;
}


void puListBox::doHit ( int button, int updown, int x, int y )
{
  if ( puActiveWidget() && ( this != puActiveWidget() ) )
  {
    puActiveWidget() -> invokeDownCallback () ;
    puDeactivateWidget () ;
  }

  if ( button == PU_LEFT_BUTTON )
  {
    lowlight () ;
    
    int yinc = puGetStringHeight(legendFont) + PUSTR_BGAP ;
    int index = top + ( abox.max[1] - PUSTR_BGAP - y ) / yinc;
    if ( index < 0 )
      index = 0;
    else if ( index >= num )
      index = num-1;
    
    setValue ( index ) ;
    
    puSetActiveWidget ( this ) ;
    invokeCallback () ;
  }
  else
    lowlight () ;
}
