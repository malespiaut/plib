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

/*
  FreeBSD port - courtesy of Stephen Montgomery-Smith
  <stephen@math.missouri.edu>

  NetBSD mods - courtesy Rene Hexel.
*/

#ifdef UL_BSD

#include <machine/joystick.h>
#define JS_DATA_TYPE joystick
#define JS_RETURN_SIZE (sizeof(struct JS_DATA_TYPE))
#define _JS_MAX_AXES 2

void jsJoystick::open ()
{
  name [0] = '\0' ;

  num_axes    =  2 ;
  num_buttons = 32 ;

  fd = ::open ( fname, O_RDONLY ) ;

  error = ( fd < 0 ) ;

  if ( error )
    return ;

  /*
    FIXME: get joystick name for BSD
  */

  FILE *joyfile ;
  char joyfname [ 1024 ] ;
  int noargs, in_no_axes ;

  float axes  [ _JS_MAX_AXES ] ;
  int buttons [ _JS_MAX_AXES ] ;

  rawRead ( buttons, axes ) ;
  error = axes[0] < -1000000000.0f ;
  if ( error )
    return ;

  sprintf( joyfname, "%s/.joy%drc", ::getenv ( "HOME" ), id ) ;

  joyfile = fopen ( joyfname, "r" ) ;
  error = joyfile == NULL ;
  if ( error )
    return ;

  noargs = fscanf ( joyfile, "%d%f%f%f%f%f%f", &in_no_axes,
                    &min [ 0 ], &center [ 0 ], &max [ 0 ],
                    &min [ 1 ], &center [ 1 ], &max [ 1 ] ) ;
  error = noargs != 7 || in_no_axes != _JS_MAX_AXES ;
  fclose ( joyfile ) ;
  if ( error )
    return ;

  for ( int i = 0 ; i < _JS_MAX_AXES ; i++ )
  {
    dead_band [ i ] = 0.0f ;
    saturate  [ i ] = 1.0f ;
  }
}


void jsJoystick::close ()
{
  if ( ! error )
    ::close ( fd ) ;
}


jsJoystick::jsJoystick ( int ident )
{
  id = ident ;
  sprintf ( fname, "/dev/joy%d", ident ) ;
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

  int status = ::read ( fd, &js, JS_RETURN_SIZE ) ;

  if ( status != JS_RETURN_SIZE )
  {
    perror ( fname ) ;
    setError () ;
    return ;
  }

  if ( buttons != NULL )
    *buttons = ( js.b1 ? 1 : 0 ) | ( js.b2 ? 2 : 0 ) ;

  if ( axes != NULL )
  {
    axes[0] = (float) js.x ;
    axes[1] = (float) js.y ;
  }
}

#endif

