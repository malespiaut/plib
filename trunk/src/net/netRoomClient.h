/****
* NAME
*   netRoomClient - game room client
*
* DESCRIPTION
*   a game room client connects to a game room server and
*   create/join/leave games through this interface.
*
*   Game rooms are a client-server design pattern where
*   you select a game to play, choose a game room, and
*   then enter the room lobby where you can chat,
*   create a game, or join/watch an existing game.
*
* AUTHOR
*   Dave McClurg <dpm@efn.org>
*
* CREATION DATE
*   Dec-2000
****/

#ifndef __NET_ROOM_CLIENT__
#define __NET_ROOM_CLIENT__


#include "netRoom.h"


class netRoomClient : private netMessageChannel
{
  enum States
  {
    STATE_DISCONNECT,
    STATE_LOGIN_REPLY,
    STATE_JOIN_REPLY,
    STATE_READY,
    NUM_STATES
  };
  
  int local_id ;
  int state;

  u32 num_games, num_players ;
  u32 max_games, max_players ;
  u32 next_game, next_player ;
  netRoomGameInfo* games ;
  netRoomPlayerInfo* players ;

  void processUpdateGame ( const netMessage& msg ) ;
  void processUpdatePlayer ( const netMessage& msg ) ;
  virtual void processMessage ( const netMessage& msg ) ;

  static int last_error ;

public:

  static int getLastError () { return last_error ; }

  netRoomClient ( cchar* host, int port ) ;
  ~netRoomClient () ;

  void login ( cchar* player_name,
    const netGuid& game_guid, cchar* server_password=0 ) ;
  void logout () ;

  bool isLoggedIn () const
  {
    return(state > STATE_LOGIN_REPLY);
  }
  
  /* games */

  u32 getNumGames () const { return num_games ; }

  const netRoomGameInfo* getGame ( u32 i ) const
  {
    ((netRoomClient*)this)->next_game = i ;
    return ( i >= num_games ) ? NULL : &games [ i ] ;
  }

  const netRoomGameInfo* getNextGame () const
  {
    return getGame ( next_game+1 ) ;
  }

  const netRoomGameInfo* findGame ( int id ) const
  {
    if ( id ) {
      netRoomGameInfo* g = games ;
      for ( u32 i=0; i<num_games; i++, g++ ) {
        if ( g->id == id )
          return g;
      }
    }
    return NULL ;
  }

  bool createGame ( cchar* name, cchar* password, int max_players ) ;
  bool joinGame ( int game_id, cchar* password ) ;
  bool watchGame ( int game_id, cchar* password ) ;
  void startGame ( int game_id ) ;
  void leaveGame () ;

  /* players */

  u32 getNumPlayers () const { return num_players ; }

  const netRoomPlayerInfo* getPlayer ( u32 i ) const
  {
    ((netRoomClient*)this)->next_player = i ;
    return ( i >= num_players ) ? NULL : &players [ i ] ;
  }

  const netRoomPlayerInfo* getNextPlayer () const
  {
    return getPlayer ( next_player+1 ) ;
  }

  const netRoomPlayerInfo* findPlayer ( int id ) const
  {
    if ( id ) {
      const netRoomPlayerInfo* p = players ;
      for ( u32 i=0; i<num_players; i++, p++ ) {
        if ( p->id == id )
          return p;
      }
    }
    return NULL ;
  }

  int getPlayerID () const
  {
    return local_id ;
  }

  int getGameID () const
  {
    const netRoomPlayerInfo* p = findPlayer ( local_id ) ;
    if ( p )
      return p -> game_id ;
    return(0);
  }

  bool isGameHost () const
  {
    const netRoomGameInfo* g = findGame ( getGameID () ) ;
    if ( g )
      return ( local_id == g -> host_id ) ;
    return false ;
  }

  /* messages */

  bool sendMessage ( const netMessage& msg )
  {
    return netMessageChannel::sendMessage ( msg ) ;
  }

  void setCallback ( void (*callback)(const netMessage& msg) )
  {
    netMessageChannel::setCallback ( callback ) ;
  }
} ;


#endif //__NET_ROOM_CLIENT__
