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

/****
* NAME
*   netMonitor - network monitor server
*
* DESCRIPTION
*   netMonitor is a telnet command port with
*   password authorization.  It can be paired
*   with and used to remotely admin another server.
*
* AUTHORS
*   Sam Rushing <rushing@nightmare.com> - original version for Medusa
*   Dave McClurg <dpm@efn.org> - modified for use in Pegasus
*
* CREATION DATE
*   Dec-2000
*
****/

#ifndef NET_MONITOR_H
#define NET_MONITOR_H

#include "netChat.h"


class netMonitorServer : private netChannel
{
  char* name ;
  char* password ;
  char* prompt ;
  void (*cmdfunc)(cchar*) ;
  class netMonitorChannel* active ;

  friend class netMonitorChannel ;

  virtual bool writable (void) { return false ; }
  virtual void handleAccept (void) ;
  
public:

  netMonitorServer( cchar* _name, int port )
  {
    name = strdup(_name);
    password = strdup("") ;
		prompt = strdup(">>> ");
    cmdfunc = 0 ;
    active = 0 ;

		open () ;
		bind ("", port);
		listen (1);

    printf("Monitor \"%s\" started on port %d\n",name,port);
  }

  ~netMonitorServer()
  {
    ::free(name);
    ::free(password) ;
    ::free(prompt) ;
  }

  cchar* getPassword () const { return password; }
  void setPassword ( cchar* string )
  {
    ::free(password) ;
    password = strdup ( string?string:"" ) ;
  }

  void setPrompt ( cchar* string )
  {
    ::free(prompt) ;
    prompt = strdup ( string?string:"" ) ;
  }

  void setCommandFunc ( void (*func)(cchar*) )
  {
    cmdfunc = func ;
  }

  bool push (const char* s) ;
} ;

#endif // NET_MONITOR_H
