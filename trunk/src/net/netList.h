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
*   netList - list class
*
* DESCRIPTION
*   simple list class to manage things like players, games, and servers.
*   list is a small sparse array designed to give fast 'id' lookup
*
* AUTHOR
*   Dave McClurg <dpm@efn.org>
*
* CREATION DATE
*   Jan-2001
****/

#ifndef __NET_LIST__
#define __NET_LIST__


#include "netMessage.h"


class netThing
{
  int id ;

  friend class netList ;

public:

  char name [ NET_MAX_NAME+1 ] ;

  netThing ()
  {
    id = 0 ;
    name [ 0 ] = 0 ;
  }

  virtual ~netThing ()
  {}

  int getID () const { return id ; }
  cchar* getName () const { return name ; }

  virtual void get ( const netMessage& msg )
  {
    id = msg.geti () ;
    msg.gets( name, sizeof(name) ) ;
  }

  virtual void put ( netMessage& msg ) const
  {
    msg.puti ( id ) ;
    msg.puts ( name ) ;
  }

  virtual void copy ( const netThing* src )
  {
    /* don't copy id */
    netCopyName ( name, src -> name ) ;
  }
} ;


class netList
{
  enum { MAX_IDNUM = 255 } ; /* id must fit in a byte for messages */
  netThing* list [ MAX_IDNUM ] ; /* The list. */

public:

  netList ()
  {
    memset ( list, 0, sizeof ( list ) ) ;
  }
  
  netThing* get ( int id )
  {
    if ( id > 0 && id <= MAX_IDNUM )
      return list [ id-1 ] ;
    return NULL ;
  }

  netThing* findByName ( cchar* name )
  {
    for ( int id=1; id<=MAX_IDNUM; id++ )
    {
      netThing* p = list [ id-1 ] ;
      if ( p && strcasecmp ( p->name, name ) == 0 )
        return p ;
    }
    return NULL ;
  }

  netThing* add ( netThing* thing )
  {
    /* find an unused slot */
    for ( int id=1; id<=MAX_IDNUM; id++ )
    {
      if ( list [ id-1 ] == NULL )
      {
        thing -> id = id ;  /* force correct id */
        list [ id-1 ] = thing ;
        return thing ;
      }
    }
    /* list is full */
    delete thing ;
    return NULL ;
  }

  void remove ( int id )
  {
    if ( id > 0 && id <= MAX_IDNUM )
    {   
      netThing* thing = list [ id-1 ] ;
      if ( thing )
      {
        thing -> id = 0 ;
        delete thing ;
        list [ id-1 ] = NULL ;
      }
    }
  }

  void removeAll ()
  {
    for ( int id=1; id<=MAX_IDNUM; id++ )
      remove ( id ) ;
  }

  virtual ~netList (void)
  {
    removeAll () ;
  } ;

  int getNum () const { return MAX_IDNUM ; }
} ;


#endif //__NET_LIST__
