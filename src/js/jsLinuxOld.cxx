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

#include "js.h"

#if defined(UL_LINUX) && !defined(JS_NEW)

#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/joystick.h>

struct os_specific_s {
  JS_DATA_TYPE js ;
  char         fname [ 128 ] ;
  int          fd ;
};

void jsJoystick::open ()
{
  name [0] = '\0' ;

  num_axes    =  2 ;   /* Default for older Linux systems. */
  num_buttons = 32 ;

  os->fd = ::open ( os->fname, O_RDONLY ) ;

  error = ( os->fd < 0 ) ;

  if ( error )
    return ;

  /*
    The Linux driver seems to return 512 for all axes
    when no stick is present - but there is a chance
    that could happen by accident - so it's gotta happen
    on both axes for at least 100 attempts.
  */

  int counter = 0 ;

  do
  {
    rawRead ( NULL, center ) ;
    counter++ ;
  } while ( ! error &&
              counter < 100 &&
              center[0] == 512.0f &&
              center[1] == 512.0f ) ;

  if ( counter >= 100 )
    setError() ;


  for ( int i = 0 ; i < _JS_MAX_AXES ; i++ )
  {
    max       [ i ] = center [ i ] * 2.0f ;
    min       [ i ] = 0.0f ;
    dead_band [ i ] = 0.0f ;
    saturate  [ i ] = 1.0f ;
  }
}



void jsJoystick::close ()
{
  if ( ! error )
    ::close ( os->fd ) ;
  delete os;
}


jsJoystick::jsJoystick ( int ident )
{
  id = ident ;
  os = new struct os_specific_s;
  
  sprintf ( os->fname, "/dev/input/js%d", ident ) ;

  if ( access ( os->fname, F_OK ) != 0 )
    sprintf ( os->fname, "/dev/js%d", ident ) ;

  open () ;
}


void jsJoystick::rawRead ( int *buttons, float *axes )
{
  if ( error )
  {
    if ( buttons )
      *buttons = 0 ;

    if ( axes )
      for ( int i = 0 ; i < num_axes ; i++ )
        axes[i] = 1500.0f ;

    return ;
  }

  int status = ::read ( os->fd, &(os->js), JS_RETURN ) ;

  if ( status != JS_RETURN )
  {
    perror ( os->fname ) ;
    setError () ;
    return ;
  }

  if ( buttons != NULL )
    *buttons = os->js.buttons ;

  if ( axes != NULL )
  {
    axes[0] = (float) os->js.x ;
    axes[1] = (float) os->js.y ;
  }
}

void jsInit() {}

#endif
