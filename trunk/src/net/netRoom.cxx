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

#include "netRoom.h"

netRoom::netRoom ()
{
  has_password = 0 ;
  
  host [ 0 ] = 0 ;
  port = 0 ;

  max_games = 0 ;
  max_players = 0 ;
  num_games = 0 ;
  num_players = 0 ;
  num_openings = 0 ;
}

netRoom::netRoom ( cchar* _host, int _port )
{
  has_password = false ;

  netCopyName ( host, _host ) ;
  port = _port ;

  max_games = 64 ;
  max_players = 64 ;
  num_games = 0 ;
  num_players = 0 ;
  num_openings = 0 ;
}

void netRoom::get ( const netMessage& msg )
{
  netThing::get ( msg ) ;
  has_password = msg.getb () ;

  msg.gets ( host, sizeof(host) ) ;
  port = msg.geti () ;

  max_games = msg.geti () ;
  max_players = msg.geti () ;
  num_games = msg.geti () ;
  num_players = msg.geti () ;
  num_openings = msg.geti () ;
}

void netRoom::put ( netMessage& msg ) const
{
  netThing::put ( msg ) ;
  msg.putb ( has_password ) ;

  msg.puts ( host ) ;
  msg.puti ( port ) ;

  msg.puti ( max_games ) ;
  msg.puti ( max_players ) ;
  msg.puti ( num_games ) ;
  msg.puti ( num_players ) ;
  msg.puti ( num_openings ) ;
}

void netRoom::copy ( const netRoom* src )
{
  netThing::copy ( src ) ;
  has_password = src -> has_password ;

  netCopyName ( host, src -> host ) ;
  port = src -> port ;

  max_games = src -> max_games ;
  max_players = src -> max_players ;
  num_games = src -> num_games ;
  num_players = src -> num_players ;
  num_openings = src -> num_openings ;
}

netRoomPlayer::netRoomPlayer ()
{
  game_id = 0 ;
  watching = false ;
}

void netRoomPlayer::get ( const netMessage& msg )
{
  netThing::get ( msg ) ;
  game_id = msg.geti () ;
  watching = msg.getb () ;
}

void netRoomPlayer::put ( netMessage& msg ) const
{
  netThing::put ( msg ) ;
  msg.puti ( game_id ) ;
  msg.putb ( watching ) ;
}

void netRoomPlayer::copy ( const netRoomPlayer* src )
{
  netThing::copy ( src ) ;
  game_id = src -> game_id ;
  watching = src -> watching ;
}

netRoomGame::netRoomGame ()
{
  master_id = 0 ;
  max_players = 0 ;
  in_progress = false ;
  has_password = false ;
}

void netRoomGame::get ( const netMessage& msg )
{
  netThing::get ( msg ) ;
  master_id = msg.geti () ;
  max_players = msg.geti () ;
  in_progress = msg.getb () ;
  has_password = msg.getb () ;
}

void netRoomGame::put ( netMessage& msg ) const
{
  netThing::put ( msg ) ;
  msg.puti ( master_id ) ;
  msg.puti ( max_players ) ;
  msg.putb ( in_progress ) ;
  msg.putb ( has_password ) ;
}

void netRoomGame::copy ( const netRoomGame* src )
{
  netThing::copy ( src ) ;
  master_id = src -> master_id ;
  max_players = src -> max_players ;
  in_progress = src -> in_progress ;
  has_password = src -> has_password ;
}
