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


netRoomClient::netRoomClient ( const netGuid& _guid, int _version )
{
  memcpy ( &guid, &_guid, sizeof(guid) ) ;
  version = _version ;
  state = STATE_DISCONNECT;
  last_error = 0 ;
}


netRoomClient::~netRoomClient ()
{
}


void netRoomClient::enterRoom ( cchar* player_name, const netRoom& room,
  cchar* room_password )
{
  local_id = 0;
  state = STATE_DISCONNECT;
  last_error = 0 ;

  open () ;
	connect ( room.host, room.port );

  if ( room_password == NULL )
    room_password = "";

  netMessage msg ( netRoom::SYSMSG_ENTER_ROOM, 0 ) ;
  msg.puti ( version ) ;
  msg.puta ( &guid, sizeof(guid) ) ;
  msg.puts ( player_name ) ;
  msg.puts ( room_password ) ;
  sendMessage ( msg ) ;

  state = STATE_LOGIN_REPLY;
}


void netRoomClient::leaveRoom ()
{
  if ( state != STATE_DISCONNECT )
  {
    netMessage msg ( netRoom::SYSMSG_LEAVE_ROOM, 0 ) ;
    sendMessage ( msg ) ;
    closeWhenDone() ;

    state = STATE_DISCONNECT;
    players.removeAll () ;
    games.removeAll () ;
  }
}
  

void netRoomClient::processUpdatePlayer ( const netMessage& msg )
{
  netRoomPlayer player ;
  player.get ( msg ) ;

  netRoomPlayer* p = getPlayer ( player.getID () ) ;
  if ( p == NULL )
    p = (netRoomPlayer*) players.add ( handleNewPlayer () ) ;
  if ( p != NULL )
  {
    int old_game = p -> game_id ;
    p -> copy ( &player ) ;
    
    //get local player
    netRoomPlayer* localp = getPlayer ( local_id ) ;

    //is game changing?
    if ( localp && old_game != p -> game_id )
    {
      //leave?
      if ( old_game )
      {
        //did player leave our game?
        if ( localp -> getID () == p -> getID () ||
          localp -> game_id == old_game )
          handleLeaveGame ( p -> getID () ) ;
      }
      else //join
      {
        //did player join our game?
        if ( localp -> game_id == p -> game_id )
          handleJoinGame ( p -> getID () ) ;
      }
    }
  }
}


void netRoomClient::processUpdateGame ( const netMessage& msg )
{
  netRoomGame game ;
  game.get ( msg ) ;
  
  netRoomGame* g = getGame ( game.getID () ) ;
  if ( g == NULL )
    g = (netRoomGame*) games.add ( handleNewGame () ) ;
  if ( g != NULL )
  {
    g -> copy ( &game ) ;

#if 0
    int old_master = g -> master_id ;
    //is master changing?
    if ( old_master != g -> master_id )
    {
      //get local player
      netRoomPlayer* localp = getPlayer ( local_id ) ;
      if ( localp && g -> getID () == localp -> game_id )
        handleSetGameMaster () ;
    }
#endif
  }
}

void netRoomClient::handleMessage ( const netMessage& msg )
{
  if ( state == STATE_DISCONNECT )
    return;

  switch ( msg.getType() )
  {
    case netRoom::SYSMSG_ERROR_REPLY:
      {
        last_error = msg.geti () ;
        switch (state)
        {
        case STATE_LOGIN_REPLY:
          if ( last_error == 0 )
          {
            local_id = msg.getToID();
            state = STATE_READY;
          }
          break;
        case STATE_JOIN_REPLY:
          if ( last_error == 0 )
          {
            state = STATE_READY;
          }
          break;
        }
        break;
      }
    case netRoom::SYSMSG_REMOVE_ALL_GAMES:
      {
        games.removeAll () ;
        break;
      }
    case netRoom::SYSMSG_REMOVE_GAME:
      {
        int id = msg.geti () ;
        games.remove ( id ) ;
        break;
      }
    case netRoom::SYSMSG_UPDATE_GAME:
      {
        processUpdateGame ( msg ) ;
        break;
      }
    case netRoom::SYSMSG_REMOVE_ALL_PLAYERS:
      {
        players.removeAll () ;
        break;
      }
    case netRoom::SYSMSG_REMOVE_PLAYER:
      {
        int id = msg.geti () ;
        players.remove ( id ) ;
        break;
      }
    case netRoom::SYSMSG_UPDATE_PLAYER:
      {
        processUpdatePlayer ( msg ) ;
        break;
      }
    default:
      {
        netMessageChannel::handleMessage ( msg ) ;
        break;
      }
  }
}


void netRoomClient::createGame ( cchar* game_name, cchar* _password, 
    int max_players )
{
  leaveGame () ;

  if ( state == STATE_READY )
  {
    printf ( "Creating game %s...\n", game_name ) ;

    char name[ NET_MAX_NAME+1 ];
    netCopyName ( name, game_name ) ;
  
    char password[ NET_MAX_NAME+1 ];
    netCopyName ( password, _password ) ;

    netMessage msg ( netRoom::SYSMSG_CREATE_GAME, 0 );
    msg.puti( max_players ) ;
    msg.puts( name );
    msg.puts( password );
    sendMessage ( msg ) ;

    //wait for reply
    state = STATE_JOIN_REPLY ;
    last_error = 0 ;
  }
}


void netRoomClient::joinGame ( cchar* game_name, cchar* _password )
{
  leaveGame () ;

  if ( state == STATE_READY )
  {
    const netRoomGame* found = NULL ;
    for ( int id = 1; id <= games . getNum (); id ++ )
    {
      const netRoomGame* g = getGame ( id ) ;
      if ( g && strcmp ( g -> name, game_name ) == 0 )
      {
        found = g ;
        break ;
      }
    }
  
    if ( found != NULL )
    {
      printf ( "Joining game %s...\n", found -> name ) ;

      char password[ NET_MAX_NAME+1 ];
      netCopyName ( password, _password ) ;

      bool watching = false;
  
      netMessage msg( netRoom::SYSMSG_JOIN_GAME, 0 );
      msg.puti ( found -> getID () ) ;
      msg.putb ( watching ) ;
      msg.puts( password );
      sendMessage ( msg ) ;
  
      //wait for reply
      state = STATE_JOIN_REPLY ;
      last_error = 0 ;
    }
  }
}


void netRoomClient::watchGame ( cchar* game_name, cchar* _password )
{
  leaveGame () ;

  if ( state == STATE_READY )
  {
    const netRoomGame* found = NULL ;
    for ( int id = 1; id <= games . getNum (); id ++ )
    {
      const netRoomGame* g = getGame ( id ) ;
      if ( g && strcmp ( g -> name, game_name ) == 0 )
      {
        found = g ;
        break ;
      }
    }
  
    if ( found != NULL )
    {
      printf ( "Watching game %s...\n", found -> name ) ;

      char password[ NET_MAX_NAME+1 ];
      netCopyName ( password, _password ) ;
  
      bool watching = true;
  
      netMessage msg( netRoom::SYSMSG_JOIN_GAME, 0 );
      msg.puti ( found -> getID () ) ;
      msg.putb ( watching ) ;
      msg.puts ( password ) ;
      sendMessage ( msg ) ;
  
      //wait for reply
      state = STATE_JOIN_REPLY ;
      last_error = 0 ;
    }
  }
}


void netRoomClient::leaveGame ()
{
  if ( inGame () && state == STATE_READY )
  {
    printf ( "Leaving game...\n" ) ;

    netMessage msg( netRoom::SYSMSG_LEAVE_GAME, 0 );
    sendMessage ( msg ) ;
  }
}


void netRoomClient::startGame ()
{
  netRoomPlayer* p = getPlayer ( local_id ) ;
  if ( p )
  {
    const netRoomGame* g = getGame ( p -> game_id ) ;
    if ( g && g -> master_id == local_id )
    {
      printf ( "Starting game...\n" ) ;

      netMessage	msg( netRoom::SYSMSG_START_GAME, 0 );
      sendMessage ( msg ) ;
    }
  }
}


bool netRoomClient::inGame () const
{
  const netRoomPlayer* p = ((netRoomClient*)this) -> getPlayer ( local_id ) ;
  return p && p -> game_id != 0 ;
}


bool netRoomClient::isGameMaster () const
{
  const netRoomPlayer* p = ((netRoomClient*)this) -> getPlayer ( local_id ) ;
  if ( p )
  {
    const netRoomGame* g = ((netRoomClient*)this) -> getGame ( p -> game_id ) ;
    return g && g -> master_id == local_id ;
  }
  return false ;
}


int netRoomClient::getGameMaster () const
{
  const netRoomPlayer* p = ((netRoomClient*)this) -> getPlayer ( local_id ) ;
  if ( p )
  {
    const netRoomGame* g = ((netRoomClient*)this) -> getGame ( p -> game_id ) ;
    if ( g )
      return g -> master_id ;
  }
  return 0 ;
}


void netRoomClient::setGameMaster ( int player_id )
{
  netRoomPlayer* p = getPlayer ( local_id ) ;
  if ( p )
  {
    const netRoomGame* g = getGame ( p -> game_id ) ;
    if ( g && ( g -> master_id == 0 || g -> master_id == local_id ) )
    {
      netRoomPlayer* m = getPlayer ( player_id ) ;
      if ( m && m -> game_id == p -> game_id && m -> getID () != g -> master_id )
      {
        printf ( "Setting game master to %s...\n", m -> name ) ;
  
        netMessage	msg( netRoom::SYSMSG_SET_GAME_MASTER, 0 );
        msg.puti ( player_id ) ;
        sendMessage ( msg ) ;
      }
    }
  }
}
