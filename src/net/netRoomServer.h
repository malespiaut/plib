/****
* NAME
*   netRoomServer - game room server
*
* DESCRIPTION
*   a game room server keeps track of who is in what game and routes
*   or broadcasts messages for the players.
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

#ifndef __NET_ROOM_SERVER__
#define __NET_ROOM_SERVER__


#include "netRoom.h"


class netRoomPlayerChannel ;
class netRoomServer ;


class netRoomGameState : public netRoomGameInfo
{
public:

  netRoomServer* server ;
  char password [ NET_MAX_NAME+1 ] ;
};


class netRoomPlayerState : public netRoomPlayerInfo
{
  netRoomPlayerChannel* channel ;

  void processLogin( const netMessage& msg );
  void processGameStart( const netMessage& msg );
  void processCreateGame( const netMessage& msg );
  void processJoinGame( const netMessage& msg );

  void sendReply( int error );

public:

  netRoomServer* server ;
  bool gagged ;

  void init ( int fd ) ;
  void disconnect (void) ;

  bool isGameHost() const;
  void leaveGame();
  
  void sendMessage ( const netMessage& msg );
  void processMessage ( const netMessage& msg ) ;
};


class netRoomPlayerChannel : private netMessageChannel
{
  netRoomPlayerState* player ;

  virtual void processMessage ( const netMessage& msg )
  {
    player -> processMessage ( msg ) ;
  }

  virtual void handleClose (void)
  {
    shouldDelete () ;
    player -> disconnect () ;
  }

public:
  netRoomPlayerChannel ( netRoomPlayerState* _player, int s )
    : player ( _player )
  {
    setHandle ( s ) ;
  }

  bool sendMessage ( const netMessage& msg )
  {
    return bufferSend ( msg.getData(), msg.getLength() ) ;
  }

  void disconnect ()
  {
    closeWhenDone () ;
  }
};


class netRoomServer : private netChannel
{
  netRoomServerInfo info ;
  char password [ NET_MAX_NAME+1 ] ;

  u32 num_games, num_players ;
  u32 max_games, max_players ;
  u32 next_game, next_player ;
  netRoomGameState* games ;
  netRoomPlayerState* players ;

  virtual bool writable (void)
  {
    return false ;
  }

  virtual void handleAccept (void) ;

  friend class netRoomGameState ;
  friend class netRoomPlayerState ;

  void makeUnique( char* name ) const ;

  netRoomGameState* addGame () ;
  netRoomPlayerState* addPlayer () ;

  void delGame ( netRoomGameState* g ) ;
  void delPlayer ( netRoomPlayerState* p ) ;

  void updatePlayer( netRoomPlayerState* player );
  void updateGame( netRoomGameState* game );

public:

  netRoomServer ( const netRoomServerInfo& info ) ;
  ~netRoomServer () ;

  const netRoomServerInfo* getInfo () ;

  void setPassword ( cchar* _password )
  {
    netCopyName ( password, _password ) ;
    info.has_password = ( password[0] != 0 ) ;
  }

  /* games */

  u32 getNumGames () const { return num_games ; }

  netRoomGameState* getGame ( u32 i )
  {
    next_game = i ;
    return ( i >= num_games ) ? NULL : &games [ i ] ;
  }

  netRoomGameState* getNextGame ()
  {
    return getGame ( next_game+1 ) ;
  }

  netRoomGameState* findGame ( int id )
  {
    if ( id ) {
      netRoomGameState* g = games ;
      for ( u32 i=0; i<num_games; i++, g++ ) {
        if ( g->id == id )
          return g;
      }
    }
    return NULL ;
  }

  /* players */

  u32 getNumPlayers () const { return num_players ; }

  netRoomPlayerState* getPlayer ( u32 i )
  {
    next_player = i ;
    return ( i >= num_players ) ? NULL : &players [ i ] ;
  }

  netRoomPlayerState* getNextPlayer ()
  {
    return getPlayer ( next_player+1 ) ;
  }

  netRoomPlayerState* findPlayer ( int id )
  {
    if ( id ) {
      netRoomPlayerState* p = players ;
      for ( u32 i=0; i<num_players; i++, p++ ) {
        if ( p->id == id )
          return p;
      }
    }
    return NULL ;
  }

  void gagPlayer ( int player_id, bool flag=true ) ;
  void kickPlayer ( int player_id ) ;
  void banPlayer ( int player_id, bool flag=true ) ;
};


#endif //__NET_ROOM_SERVER__