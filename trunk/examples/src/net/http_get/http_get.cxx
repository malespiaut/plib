/*
     PLIB - A Suite of Portable Game Libraries
     Copyright (C) 2001  Steve Baker

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

     For further information visit http://plib.sourceforge.net

     $Id$
*/

#include <plib/net.h>


class HTTPClient : public netBufferChannel
{
public:

  HTTPClient ( cchar* host, cchar* path )
  {
		open ();
		connect (host, 80);

    cchar* s = netFormat ( "GET %s HTTP/1.0\r\n\r\n", path );
    bufferSend( s, strlen(s) ) ;
  }

  virtual void handleBufferRead (netBuffer& buffer)
  {
    const char* s = buffer.getData();
    while (*s)
      fputc(*s++,stdout);

    buffer.remove();
  }
} ;


int
main (int argc, char * argv[])
{
  netInit(&argc,argv);

  new HTTPClient( "www.opengl.org", "/index.html" );

  netChannel::loop(0);
  return 0;
}
