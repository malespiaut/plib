/****
* NAME
*   netRoom - game room classes
*
* DESCRIPTION
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

#ifndef __NET_ROOM__
#define __NET_ROOM__


#include "netList.h"


#define TINY_INTERNAL
class netRoom : public netThing
{
public:
    
  enum MessageType
  {
    //Client2Client
    MSG_SAY=0,

    /* App message types (0-199) */
    APPMSG_BASE=1,

    /* Sys message types (200-255) */
    SYSMSG_BASE=200,

    //Client2Server
    SYSMSG_ENTER_ROOM=SYSMSG_BASE,
    SYSMSG_LEAVE_ROOM,
    SYSMSG_CREATE_GAME,
    SYSMSG_JOIN_GAME,
    SYSMSG_LEAVE_GAME,
    SYSMSG_START_GAME,
    SYSMSG_SET_GAME_MASTER,
    SYSMSG_UNUSED1,

    //Server2Client
    SYSMSG_ERROR_REPLY,
    SYSMSG_REMOVE_ALL_GAMES,
    SYSMSG_REMOVE_GAME,
    SYSMSG_UPDATE_GAME,
    SYSMSG_REMOVE_ALL_PLAYERS,
    SYSMSG_REMOVE_PLAYER,
    SYSMSG_UPDATE_PLAYER,

    //Client2Master
    SYSMSG_BROWSE_ROOMS,
    SYSMSG_ADVERTISE_ROOM,

    //Master2Client
    SYSMSG_REMOVE_ALL_ROOMS,
    SYSMSG_UPDATE_ROOM
  };

  enum ErrorType
  {
    ERROR_NONE=0,
    ERROR_TOO_MANY_GAMES=100, /* > last system errno */
    ERROR_TOO_MANY_ROOMS,
    ERROR_INVALID_GAME,
    ERROR_GAME_IS_FULL,
    ERROR_WRONG_GAME_PASSWORD,
    ERROR_WRONG_SERVER_PASSWORD,
    ERROR_WRONG_VERSION,
    ERROR_WRONG_GUID
  };

  bool has_password ;

  //address
  char host [ NET_MAX_NAME+1 ] ;
  int port ;

  int max_games ;
  int max_players ;
  int num_games ;
  int num_players ;
  int num_openings ;

  netRoom () ;
  netRoom ( cchar* _host, int _port ) ;
  virtual void get ( const netMessage& msg ) ;
  virtual void put ( netMessage& msg ) const ;
  virtual void copy ( const netRoom* src ) ;
};


class netRoomPlayer : public netThing
{
public:

  int game_id ;
  bool watching ;

  netRoomPlayer () ;
  virtual void get ( const netMessage& msg ) ;
  virtual void put ( netMessage& msg ) const ;
  virtual void copy ( const netRoomPlayer* src ) ;
  bool inGame () const { return game_id != 0 ; }
};


class netRoomGame : public netThing
{
public:

  int master_id ;
  int max_players ;
  bool in_progress ;
  bool has_password ;

  netRoomGame () ;
  virtual void get ( const netMessage& msg ) ;
  virtual void put ( netMessage& msg ) const ;
  virtual void copy ( const netRoomGame* src ) ;
  int getMaxPlayers () const { return max_players ; }
  bool inProgress () const { return in_progress ; }
  bool hasPassword () const { return has_password ; }
};


class netRoomList : public netList
{
public:

  netRoom* get ( int id )
    { return (netRoom *) netList::get ( id ) ; }
  netRoom* add ( netRoom* thing )
    { return (netRoom *) netList::add ( thing ) ; }
  netRoom* findByName ( cchar* name )
    { return (netRoom *) netList::findByName ( name ) ; }

  netRoom* findByAddr ( cchar* host, int port )
  {
    for ( int id = 1; id <= getNum (); id ++ )
    {
      netRoom* p = get ( id ) ;
      if ( p && strcasecmp ( p -> host, host ) == 0 && p -> port == port )
        return p ;
    }
    return NULL ;
  }
} ;


/*
**  game room master
**
**  a game room master server keeps track of which
**  game room servers are running and information
**  about each one.  game room servers send updates
**  and game room clients download the list
**  of game room servers.
*/

class netRoomMasterServer : private netChannel
{
  virtual bool writable (void) { return false ; }
  virtual void handleAccept (void) ;

public:

  netGuid guid ;
  int version ;
  netRoomList rooms ;

  netRoomMasterServer ( int port, const netGuid& _guid, int _version )
  {
    memcpy ( &guid, &_guid, sizeof(guid) ) ;
    version = _version ;
		open () ;
		bind ("", port);
		listen (5);
    printf("Master Server started on port %d\n",port);
  }
} ;


class netRoomBrowser : private netMessageChannel
{
  netGuid guid ;
  int version ;
  netRoomList* rooms ;
  int last_error ;

  virtual void handleMessage ( const netMessage& msg ) ;
  virtual void handleError (int error)
  {
    if ( errno )
      last_error = errno ;
  }

public:

  netRoomBrowser ( const netGuid& _guid, int _version )
  {
    memcpy ( &guid, &_guid, sizeof(guid) ) ;
    version = _version ;
    rooms = 0 ;
    last_error = 0 ;
  }

  int getError ()
  {
    int error = last_error ;
    last_error = 0 ;
    return error ;
  }

  void browse ( cchar* host, int port, netRoomList* _rooms ) ;
  void close ( void ) { netMessageChannel::close () ; }
  virtual netRoom* handleNewRoom () { return new netRoom ; }
} ;


class netRoomAdvertiser : private netMessageChannel
{
  netGuid guid ;
  int version ;

public:

  netRoomAdvertiser ( const netGuid& _guid, int _version )
  {
    memcpy ( &guid, &_guid, sizeof(guid) ) ;
    version = _version ;
  }

  void advertise ( cchar* host, int port, const netRoom& room ) ;
  void update ( const netRoom& room ) ;
} ;


/*
**  Game Room Server
**
**  a game room server keeps track of who is in what game and routes
**  or broadcasts messages for the players.
**
**  Game rooms are a client-server design pattern where
**  you select a game to play, choose a game room, and
**  then enter the room lobby where you can chat,
**  create a game, or join/watch an existing game.
*/


class netRoomServer : private netChannel
{
  virtual bool writable (void) { return false ; }
  virtual void handleAccept (void) ;

public:

  netRoomServer ( const netRoom& room, const netGuid& _guid, int _version ) ;
  ~netRoomServer () ;

  const netRoom* getInfo () ;
  const netRoomPlayer* getPlayer ( int id ) ;
  const netRoomGame* getGame ( int id ) ;
  int getNumPlayers () const ;
  int getNumGames () const ;

  void setPassword ( cchar* _password ) ;
  void gagPlayer ( int player_id, bool flag=true ) ;
  void kickPlayer ( int player_id ) ;
  void banPlayer ( int player_id, bool flag=true ) ;
} ;


/*
**  game room client
**
**  a game room client connects to a game room server and
**  create/join/leave games through this interface.
**
**  Game rooms are a client-server design pattern where
**  you select a game to play, choose a game room, and
**  then enter the room lobby where you can chat,
**  create a game, or join/watch an existing game.
*/

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
  
  netGuid guid ;
  int version ;
  int local_id ;
  int state;
  int last_error ;

  void processUpdateGame ( const netMessage& msg ) ;
  void processUpdatePlayer ( const netMessage& msg ) ;

  virtual void handleError (int error)
  {
    if ( errno )
      last_error = errno ;
  }

protected:

  netList games ;
  netList players ;

public:

  netRoomClient ( const netGuid& _guid, int _version ) ;
  ~netRoomClient () ;

  int getError ()
  {
    int error = last_error ;
    last_error = 0 ;
    return error ;
  }

  /* room */

  void enterRoom ( cchar* player_name, const netRoom& room, cchar* room_password=0 ) ;
  void leaveRoom () ;
  bool inRoom () const
  {
    return( isConnected() && state > STATE_LOGIN_REPLY );
  }
  
  /* games */

  void createGame ( cchar* game_name, cchar* password, int max_players ) ;
  void joinGame ( cchar* game_name, cchar* password ) ;
  void watchGame ( cchar* game_name, cchar* password ) ;
  void leaveGame () ;
  void startGame () ;
  bool inGame () const ;
  bool isGameMaster () const ;
  int getGameMaster () const ;
  void setGameMaster ( int player_id ) ;

  virtual void handleSay ( cchar* text ) {}
  virtual void handleJoinGame ( int player_id ) {}
  virtual void handleLeaveGame ( int player_id ) {}

  /* players */

  int getLocalID () const { return local_id ; }

  /* messages */

  bool sendMessage ( netMessage& msg )
  {
    msg.setFromID ( local_id ) ;
    return netMessageChannel::sendMessage ( msg ) ;
  }

  virtual void handleMessage ( const netMessage& msg ) ;

  /* list methods */

  virtual netRoomGame* handleNewGame () { return new netRoomGame ; }
  virtual netRoomPlayer* handleNewPlayer () { return new netRoomPlayer ; }

  int getNumGames () const { return games . getNum () ; }
  netRoomGame* getGame ( int id )
    { return (netRoomGame*) games . get ( id ) ; }
  netRoomGame* findGame ( cchar* name )
    { return (netRoomGame*) games . findByName ( name ) ; }

  int getNumPlayers () const { return players . getNum () ; }
  netRoomPlayer* getPlayer ( int id )
    { return (netRoomPlayer*) players . get ( id ) ; }
  netRoomPlayer* findPlayer ( cchar* name )
    { return (netRoomPlayer*) players . findByName ( name ) ; }
} ;


#endif //__NET_ROOM__
