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


/*
 *  Define private classes
 */

class Game : public netRoomGame
{
public:

  char password [ NET_MAX_NAME+1 ] ;
};


class Player : public netRoomPlayer
{
public:

  bool gagged ;
  class PlayerChannel* channel ;
};


class PlayerList : public netList
{
public:

  Player* get ( int id )
    { return (Player *) netList::get ( id ) ; }
  Player* add ( Player* thing )
    { return (Player *) netList::add ( thing ) ; }
  Player* findByName ( cchar* name )
    { return (Player *) netList::findByName ( name ) ; }
} ;


class GameList : public netList
{
public:

  Game* get ( int id )
    { return (Game *) netList::get ( id ) ; }
  Game* add ( Game* thing )
    { return (Game *) netList::add ( thing ) ; }
  Game* findByName ( cchar* name )
    { return (Game *) netList::findByName ( name ) ; }
} ;


class PlayerChannel : private netMessageChannel
{
  Player* player ;

public:
  PlayerChannel ( Player* _player, int handle ) ;

  void leaveRoom() { closeWhenDone() ; }
  void leaveGame();

  void processEnterRoom( const netMessage& msg );
  void processStartGame( const netMessage& msg );
  void processCreateGame( const netMessage& msg );
  void processJoinGame( const netMessage& msg );
  void processSetGameMaster( const netMessage& msg );

  void sendReply( int error );

  bool sendMessage ( const netMessage& msg )
  {
    return bufferSend ( msg.getData(), msg.getLength() ) ;
  }

  virtual void handleMessage ( const netMessage& msg ) ;
  virtual void handleClose (void) ;
} ;


static int room_version ;
static netGuid room_guid ;
static netRoom room ;
static char room_password [ NET_MAX_NAME+1 ] ;
static int num_rooms = 0 ;

static GameList games ;
static PlayerList players ;

static void makeUnique( char* name ) ;
static void updatePlayer( Player* p ) ;
static void updateGame( Game* g ) ;
static void removePlayer( Player* p ) ;
static void removeGame( Game* g ) ;


PlayerChannel::PlayerChannel ( Player* _player, int handle )
{
  setHandle ( handle ) ;

  player = _player ;
  player -> gagged = false ;
  player -> channel = this ;
}


void PlayerChannel::sendReply( int error )
{
  netMessage msg( netRoom::SYSMSG_ERROR_REPLY, player -> getID (), 0 );
  msg.puti ( error ) ;
  sendMessage( msg );
}


void PlayerChannel::handleClose (void)
{
  if ( player )
  {
    leaveGame () ;
    removePlayer ( player ) ;
    player = NULL ;
  }
  shouldDelete () ;
  netMessageChannel::handleClose () ;
}


void PlayerChannel::leaveGame()
{
  Game* g = games . get ( player -> game_id ) ;
  if ( g )
  {
    player -> game_id = 0 ;
    updatePlayer ( player ) ;
    printf("Player %s left game %s.\n", player -> name, g -> name ) ;

    // how many players remain?
    int num = 0 ;
    for ( int id = 1; id <= players . getNum (); id ++ )
    {
      Player* p = players . get ( id ) ;
      if ( p && p -> game_id == g -> getID () )
        num ++ ;
    }

    // is game empty ?
    if ( num == 0 )
    {
      removeGame ( g ) ;
    }
    else if ( g -> master_id == player -> getID () )
    {
      //eek! the master left
      //client should handle the event via handleNowMaster()
      g -> master_id = 0 ;
      updateGame ( g ) ;
    }
  }
}


void PlayerChannel::processEnterRoom ( const netMessage &msg )
{
  int version ;
  netGuid guid ;
  char password[NET_MAX_NAME+1];

  version = msg.geti () ;
  msg.geta ( &guid, sizeof(guid) ) ;
  msg.gets ( player -> name, sizeof(player -> name) ) ;
  msg.gets ( password, sizeof(password) ) ;

  makeUnique( player -> name );

  printf("Player %s connected.\n", player -> name);

  int error = netRoom::ERROR_NONE ;
  if ( version != room_version )
  {
    error = netRoom::ERROR_WRONG_VERSION ;
  }
  else if ( guid != room_guid )
  {
    error = netRoom::ERROR_WRONG_GUID ;
  }
  else if ( room_password[0] != 0 && strcasecmp(password,room_password) )
  {
    error = netRoom::ERROR_WRONG_SERVER_PASSWORD ;
  }

  //reply to login with player ID
  sendReply( error ) ;

  if ( error != netRoom::ERROR_NONE )
  {
    leaveRoom () ;
    return ;
  }

  {
    netMessage msg( netRoom::SYSMSG_REMOVE_ALL_PLAYERS, player -> getID (), 0 );
    sendMessage( msg );

    for ( int id = 1; id <= players . getNum (); id ++ )
    {
      Player* p = players . get ( id ) ;
      if ( p )
      {
        netMessage msg( netRoom::SYSMSG_UPDATE_PLAYER, player -> getID (), 0 );
        p -> put ( msg ) ;
        sendMessage( msg );
      }
    }
  }

  {
    netMessage msg( netRoom::SYSMSG_REMOVE_ALL_GAMES, player -> getID (), 0 );
    sendMessage( msg );

    for ( int id = 1; id <= games . getNum (); id ++ )
    {
      Game* g = games . get ( id ) ;
      if ( g )
      {
        netMessage msg( netRoom::SYSMSG_UPDATE_GAME, player -> getID (), 0 );
        g -> put ( msg ) ;
        sendMessage( msg );
      }
    }
  }

  /*
  **  tell everyone else i'm here!
  */
  updatePlayer ( player );
}


void PlayerChannel::processStartGame( const netMessage &msg )
{
  Game* g = games . get ( player -> game_id ) ;
  if ( g )
  {
    if ( g -> master_id == player -> getID () )
    {
      g -> in_progress = true;
      updateGame (g);
  
      printf( "Game %s has started.\n", g->name );
    }
  }
}


void PlayerChannel::processSetGameMaster( const netMessage &msg )
{
  int	_master_id = msg.geti () ;

  Game* g = games . get ( player -> game_id ) ;
  if ( g )
  {
    if ( g -> master_id == 0 || g -> master_id == player -> getID () )
    {
      Player* p = players . get ( _master_id ) ;
      if ( p && p -> game_id == player -> game_id && p -> getID () != g -> master_id )
      {
        g -> master_id = _master_id ;
        updateGame (g);
  
        printf( "Game %s master is now %s.\n", g->name, p->name );
      }
    }
  }
}


void PlayerChannel::processCreateGame( const netMessage& msg )
{
  int error = netRoom::ERROR_NONE ;

  Game* g = games.add ( new Game ) ;
  if ( g )
  {
    leaveGame();

    g->max_players = msg.geti () ;
    msg.gets( g->name, sizeof(g->name) );
    msg.gets( g->password, sizeof(g->password) );

    makeUnique( g->name );
    g->has_password = (g->password[0] != 0);
    g->master_id = player -> getID () ;
    g->in_progress = false;

    player->game_id = g -> getID () ;
    player->watching = false ;

    updateGame ( g ) ;
    updatePlayer ( player ) ;

    printf("Player %s created game %s.\n", player->name, g->name);
  }
  else
    error = netRoom::ERROR_TOO_MANY_GAMES;
  
  sendReply( error );
}


void PlayerChannel::processJoinGame( const netMessage& msg )
{
  int error = netRoom::ERROR_INVALID_GAME;
  int _game_id;
  bool _watching;
  char _password[NET_MAX_NAME+1];

  _game_id = msg.geti () ;
  _watching = msg.getb () ;
  msg.gets( _password, sizeof(_password) ) ;

  Game* g = games . get ( _game_id ) ;
  if ( g )
  {
    //are there any other players in this game?
    int count = 0;
    for ( int id = 1; id <= players . getNum (); id ++ )
    {
      Player* p = players . get ( id ) ;
      if ( p && p != player && p->game_id == _game_id )
        count++;
    }

    if ( g->password[0] != 0 && strcasecmp(_password,g->password) )
    {
      error = netRoom::ERROR_WRONG_GAME_PASSWORD;
    }
    else if ( !_watching && (count >= g->max_players || g->in_progress) )
    {
      error = netRoom::ERROR_GAME_IS_FULL;
    }
    else
    {
      leaveGame();

      player->game_id = _game_id;
      player->watching = _watching;
      updatePlayer ( player ) ;

      printf("Player %s joined game %s.\n", player->name, g->name);
      error = netRoom::ERROR_NONE ;
    }
  }
  sendReply( error );
}


void PlayerChannel::handleMessage ( const netMessage& msg )
{
  switch ( msg.getType() )
  {
  case netRoom::SYSMSG_ENTER_ROOM:
    processEnterRoom( msg );
    break;
  case netRoom::SYSMSG_LEAVE_ROOM:
    leaveRoom();
    break;
  case netRoom::SYSMSG_CREATE_GAME:
    processCreateGame( msg );
    break;
  case netRoom::SYSMSG_JOIN_GAME:
    processJoinGame( msg );
    break;
  case netRoom::SYSMSG_LEAVE_GAME:
    leaveGame();
    break;
  case netRoom::SYSMSG_START_GAME:
    processStartGame( msg );
    break;
  case netRoom::SYSMSG_SET_GAME_MASTER:
    processSetGameMaster( msg );
    break;
  default:
    /*
    **  Echo chat messages
    */
    if ( msg.getType() == netRoom::MSG_SAY )
    {
      const Player* from = players . get ( msg.getFromID() );
      if ( from && from -> gagged )
        break;

      char text[256];
      msg.gets( text, sizeof(text) );
      
      if ( msg.getToID() )
      {
        const Player* to = players . get ( msg.getToID() );
        if ( from && to )
          printf("whisper from %s to %s> %s\n", from->name, to->name, text);
      }
      else
      {
        if ( from )
          printf("%s> %s\n", from->name, text);
      }
    }
    /*
    **  Route messages
    */
    if ( msg.getToID() )
    {
      Player* q = players . get ( msg.getToID() );
      if ( q )
        q -> channel -> sendMessage ( msg ) ;
    }
    else //broadcast
    {
      for ( int id = 1; id <= players . getNum (); id ++ )
      {
        Player* q = players . get ( id ) ;
        if ( q && q != player && q->game_id == player->game_id )
          q -> channel -> sendMessage ( msg ) ;
      }
    }
    break;
  }
}


const netRoom* netRoomServer::getInfo ()
{
  //set dynamic room

  room.num_games = games . getNum () ;
  room.num_players = players . getNum () ;
  room.num_openings = 0 ;

  int id ;

  for ( id = 1; id <= games . getNum (); id ++ )
  {
    Game* g = games . get ( id ) ;
    if ( g && !g->in_progress )
      room.num_openings += g->max_players;
  }

  for ( id = 1; id <= players . getNum (); id ++ )
  {
    Player* p = players . get ( id ) ;
    if ( p && p->game_id )
    {
      const Game* g = games. get ( p->game_id ) ;
      if ( g && !g->in_progress )
        room.num_openings -- ;
    }
  }

  return & room ;
}


static void updatePlayer( Player* player )
{
  netMessage msg( netRoom::SYSMSG_UPDATE_PLAYER, 0, 0 );
  player -> put ( msg ) ;

  for ( int id = 1; id <= players . getNum (); id ++ )
  {
    Player* p = players . get ( id ) ;
    if ( p )
      p -> channel -> sendMessage ( msg ) ;
  }
}


static void updateGame( Game* game )
{
  netMessage msg( netRoom::SYSMSG_UPDATE_GAME, 0, 0 );
  game -> put ( msg ) ;

  for ( int id = 1; id <= players . getNum (); id ++ )
  {
    Player* p = players . get ( id ) ;
    if ( p )
      p -> channel -> sendMessage ( msg ) ;
  }
}


static void removePlayer( Player* player )
{
  printf("Player %s left room.\n", player -> name );

  netMessage msg( netRoom::SYSMSG_REMOVE_PLAYER, 0, 0 );
  msg.puti ( player -> getID () ) ;

  for ( int id = 1; id <= players . getNum (); id ++ )
  {
    Player* p = players . get ( id ) ;
    if ( p )
      p -> channel -> sendMessage ( msg ) ;
  }

  players . remove ( player -> getID () ) ;
}


static void removeGame( Game* game )
{
  printf("Game %s was disbanded.\n",  game -> name ) ;

  netMessage msg( netRoom::SYSMSG_REMOVE_GAME, 0, 0 );
  msg.puti ( game -> getID () ) ;

  for ( int id = 1; id <= players . getNum (); id ++ )
  {
    Player* p = players . get ( id ) ;
    if ( p )
      p -> channel -> sendMessage ( msg ) ;
  }

  games . remove ( game -> getID () ) ;
}


static void makeUnique( char* name )
{
  char	filtered_name[ 250 ];
  strcpy( filtered_name, name );
  int		append = 0;

  u32 len = strlen( filtered_name );
  int id ;

  for ( id = 1; id <= players . getNum (); id ++ )
  {
    Player* p = players . get ( id ) ;
    if ( p && p->name != name &&
         (strncasecmp(p->name, filtered_name, len) == 0) && 
         (p->name[ len ] == '.' || len == strlen( p->name )) )
    {
      if ( len != strlen( p->name ) )
      {
        int	num;
        num = strtol( p->name + len + 1, NULL, 10 );
        if ( num >= append )
        {
          append = num + 1;
        }
      }
      else
      {
        if ( !append )
        {
          append = 1;
        }
      }
    }
  }

  for ( id = 1; id <= games . getNum (); id ++ )
  {
    Game* g = games . get ( id ) ;
    if ( g && g->name != name && 
         (strncasecmp( g->name, filtered_name, len ) == 0) && 
         (g->name[ len ] == '.' || len == strlen( g->name )) )
    {
      if ( len != strlen( g->name ) )
      {
        int	num;
        num = strtol( g->name + len + 1, NULL, 10 );
        if ( num >= append )
        {
          append = num + 1;
        }
      }
      else
      {
        if ( !append )
        {
          append = 1;
        }
      }
    }
  }

  strncpy( name, filtered_name, NET_MAX_NAME );
  name[ NET_MAX_NAME ] = 0;

  if ( append )
  {
    //append .<append> to name
    char num[ 10 ];
    sprintf( num, ".%d", append );
    int len = strlen(name);
    if ( len + strlen(num) > NET_MAX_NAME )
    {
      len = NET_MAX_NAME - strlen( num );
    }
    strcpy( name + len, num );
  }
}


netRoomServer::netRoomServer ( const netRoom& _room, const netGuid& _guid, int _version )
{
  assert ( num_rooms == 0 ) ;
  num_rooms = 1 ;

  memset(room_password,0,sizeof(room_password));
  memcpy ( &room, &_room, sizeof(room) ) ;
  memcpy ( &room_guid, &_guid, sizeof(room_guid) ) ;
  room_version = _version ;

  open () ;
	bind ("", room.port);
	listen (5);

  printf("Room server \"%s\" started on port %d\n",room.name,room.port);
}


netRoomServer::~netRoomServer ()
{
  assert ( num_rooms == 1 ) ;
  num_rooms = 0 ;
}


void netRoomServer::handleAccept (void)
{
  Player* p = players.add ( new Player ) ;
  if ( p )
  {
    netAddress addr ;
    int handle = accept (&addr) ;
  	printf("%d: Client %s:%d connected\n",handle,addr.getHost(),addr.getPort());
    new PlayerChannel ( p, handle ) ;
  }
}


const netRoomPlayer* netRoomServer::getPlayer ( int id )
{
  return players . get ( id ) ;
}


const netRoomGame* netRoomServer::getGame ( int id )
{
  return games . get ( id ) ;
}


int netRoomServer::getNumPlayers () const
{
  return players . getNum () ;
}


int netRoomServer::getNumGames () const
{
  return games . getNum () ;
}


void netRoomServer::setPassword ( cchar* _password )
{
  netCopyName ( room_password, _password ) ;
  room.has_password = ( room_password[0] != 0 ) ;
}


void netRoomServer::gagPlayer( int player_id, bool flag )
{
  Player* p = players . get ( player_id ) ;
  if ( p )
    p -> gagged = flag ;
}


void netRoomServer::kickPlayer( int player_id )
{
  Player* p = players . get ( player_id ) ;
  if ( p )
    p -> channel -> leaveRoom () ;
}


void netRoomServer::banPlayer( int player_id, bool flag )
{
  Player* p = players . get ( player_id ) ;
  if ( p )
  {
    if ( flag )
      p -> channel -> leaveRoom () ;
    //NEED: permanent banning and unbanning
  }
}
