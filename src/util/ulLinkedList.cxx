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
*/


#include "ul.h"

class ulListNode
{
protected:

  ulListNode *next ;
  void *data ;

public:

  ulListNode ( void *dt, ulListNode *next_node )
  {
    data = dt ;
    next = next_node ;
  }

  void * getData ( void ) const { return data ; }
  void   setData ( void *d )    { data = d    ; }

  ulListNode * getNext ( void ) const { return next ; }
  void setNext ( ulListNode *n )      { next = n    ; }
} ;


void ulLinkedList::unlinkNode ( ulListNode *prev, ulListNode *node )
{
  /* Is this the first node ? */
  if ( prev == NULL )
    head = node -> getNext () ;
  else
    prev -> setNext ( node -> getNext () ) ;

  /* Is this the last node ? */
  if ( node -> getNext () == NULL )
    tail = prev ;
}


int ulLinkedList::getNodePosition ( void *data ) const
{
  ulListNode *curr = head ;
  int pos = 0 ;

  while ( curr != NULL )
  {
    if ( curr -> getData () == data )
      return pos ;

    pos++ ;
    curr = curr -> getNext () ;
  }

  ulSetError ( UL_WARNING, "ulLInkedList::getNodePosition: No such node" ) ;
  return 0 ;
}


void ulLinkedList::appendNode ( void *data )
{
  ulListNode *new_node = new ulListNode ( data, NULL ) ;

  if ( head == NULL )
    head = new_node ;
  else
    tail -> setNext ( new_node ) ;

  tail = new_node ;

  if ( ++nnodes > 1 )
    sorted = false ;
}

void ulLinkedList::insertNode ( void *data, int pos )
{
  if ( pos == 0 )
  {
    head = new ulListNode ( data, head ) ;

    if ( tail == NULL )
      tail = head ;

    if ( ++nnodes > 1 )
      sorted = false ;
  }
  else
  {
    if ( isValidPosition ( pos ) )
    {
      ulListNode *prev = head ;

      while ( --pos > 0 )
        prev = prev -> getNext () ;

      prev -> setNext ( new ulListNode ( data, prev -> getNext () ) ) ;

      if ( ++nnodes > 1 )
        sorted = false ;
    }
  }
}


int ulLinkedList::insertSorted ( void *data, ulCompareFunc comparefn )
{
  if ( !sorted )
  {
    ulSetError ( UL_WARNING,
                 "ulLinkedList::insertSorted: This is not a sorted list !" ) ;
    return 0 ;
  }

  int pos = 0 ;

  if ( head == NULL )
    head = tail = new ulListNode ( data, NULL ) ;
  else
  {
    ulListNode *curr = head, *prev = NULL ;

    while ( (*comparefn)( curr -> getData (), data ) < 0 )
    {
      prev = curr ;
      curr = curr -> getNext () ;

      pos++ ;

      if ( curr == NULL )
      {
        tail = new ulListNode ( data, curr ) ;
        prev -> setNext ( tail ) ;

        nnodes++ ;
        return pos ;
      }
    }

    if ( prev == NULL )
      head = new ulListNode ( data, head ) ;
    else
      prev -> setNext ( new ulListNode ( data, curr ) ) ;
  }

  nnodes++ ;
  return pos ;
}


void ulLinkedList::removeNode ( void *data )
{
  ulListNode *curr = head, *prev = NULL ;

  while ( curr != NULL )
  {
    if ( curr -> getData () == data )
    {
      unlinkNode ( prev, curr ) ;

      delete curr ;

      if ( --nnodes <= 0 )
        sorted = true ;

      return ;
    }

    prev = curr ;
    curr = curr -> getNext () ;
  }

  ulSetError ( UL_WARNING, "ulHashTable::removeNode: No such node" ) ;
}

void * ulLinkedList::removeNode ( int pos )
{
  if ( ! isValidPosition ( pos ) )
    return NULL ;

  ulListNode *node = head, *prev = NULL ;

  while ( pos-- > 0 )
  {
    prev = node ;
    node = node -> getNext () ;
  }

  unlinkNode ( prev, node ) ;

  void *datap = node -> getData () ;

  delete node ;

  if ( --nnodes <= 1 )
    sorted = true ;

  return datap ;
}


void * ulLinkedList::getNodeData ( int pos ) const
{
  if ( ! isValidPosition ( pos ) )
    return NULL ;

  ulListNode *node ;

  if ( pos == nnodes - 1 )
    node = tail ;
  else
  {
    node = head ;

    while ( pos-- > 0 )
      node = node -> getNext () ;
  }

  return node -> getData () ;
}


void ulLinkedList::forEach ( ulIterateFunc fn ) const
{
  ulListNode *node ;

  for ( node = head ; node != NULL ; node = node -> getNext () )
  {
    if ( (*fn)( node -> getData () ) == false )
      break ;
  }
}


void ulLinkedList::empty ( ulIterateFunc destroyfn )
{
  ulListNode *node = head ;

  while ( node != NULL )
  {
    ulListNode *next = node -> getNext () ;

    if ( destroyfn != NULL )
      (*destroyfn) ( node -> getData () ) ;

    delete node ;

    node = next ;
  }

  head = tail = NULL ;
  nnodes = 0 ;
  sorted = true ;
}

