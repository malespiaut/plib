/****
* NAME
*   netRoom - game room classes
*
* DESCRIPTION
*   game room player, game, and server info classes
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

#ifndef __NET_ROOM__
#define __NET_ROOM__


#include "netMessage.h"


class netRoom
{
public:

  enum MessageType
  {
    //App message types start at 0
    
    //Client2Client
    CHAT=200,

    //Client2App
    PLAYER_LEFT,        //player left the game you are in
    PLAYER_JOINED,      //player joined the game you are in
    YOU_ARE_HOST,       //you are now the host of the game

    //Client2Server
    LOGIN,
    LOGOUT,
    CREATE_GAME,
    JOIN_GAME,
    LEAVE_GAME,
    START_GAME,

    //Server2Client
    ERROR_REPLY,
    INIT_GAME_LIST,
    UPDATE_GAME_LIST,
    INIT_PLAYER_LIST,
    UPDATE_PLAYER_LIST,
    
    //Client2Master
    REQUEST_SERVER_LIST,  //browser
    //UPDATE_SERVER_LIST, //advertiser

    //Master2Client
    INIT_SERVER_LIST,
    UPDATE_SERVER_LIST,
  };

  enum Error
  {
    ERROR_NONE,
    ERROR_TOO_MANY_GAMES,
    ERROR_INVALID_GAME,
    ERROR_GAME_IS_FULL,
    ERROR_WRONG_GAME_PASSWORD,
    ERROR_WRONG_SERVER_PASSWORD,
    ERROR_WRONG_NET_VERSION,
    ERROR_WRONG_GAME_GUID,
    NUM_ERRORS
  };

} ;


class netRoomPlayerInfo
{
public:

  int id ;
  char name [ NET_MAX_NAME+1 ] ;
  int game_id ;
  bool watching ;

  void get ( const netMessage& msg )
  {
    id = msg.geti () ;
    msg.gets( name, sizeof(name) ) ;
    game_id = msg.geti () ;
    watching = msg.getb () ;
  }

  void put ( netMessage& msg ) const
  {
    msg.puti ( id ) ;
    msg.puts ( name ) ;
    msg.puti ( game_id ) ;
    msg.putb ( watching ) ;
  }

  void push ( netChat* channel ) const
  {
    channel -> push ( "netRoomPlayerInfo:\r\n" ) ;
    channel -> push ( netFormat ( "  id = %d\r\n", id ) ) ;
    channel -> push ( netFormat ( "  name = %s\r\n", name ) ) ;
    channel -> push ( netFormat ( "  game_id = %d\r\n", game_id ) ) ;
    channel -> push ( netFormat ( "  watching = %d\r\n", watching ) ) ;
  }
};


class netRoomGameInfo
{
public:

  int id ;
  char name [ NET_MAX_NAME+1 ] ;
  int host_id ;
  int max_players ;
  bool in_progress ;
  bool has_password ;

  void get ( const netMessage& msg )
  {
    id = msg.geti () ;
    msg.gets( name, sizeof(name) ) ;
    host_id = msg.geti () ;
    max_players = msg.geti () ;
    in_progress = msg.getb () ;
    has_password = msg.getb () ;
  }

  void put ( netMessage& msg ) const
  {
    msg.puti ( id ) ;
    msg.puts ( name ) ;
    msg.puti ( host_id ) ;
    msg.puti ( max_players ) ;
    msg.putb ( in_progress ) ;
    msg.putb ( has_password ) ;
  }

  void push ( netChat* channel ) const
  {
    channel -> push ( "netRoomGameInfo:\r\n" ) ;
    channel -> push ( netFormat ( "  id = %d\r\n", id ) ) ;
    channel -> push ( netFormat ( "  name = %s\r\n", name ) ) ;
    channel -> push ( netFormat ( "  host_id = %d\r\n", host_id ) ) ;
    channel -> push ( netFormat ( "  max_players = %d\r\n", max_players ) ) ;
    channel -> push ( netFormat ( "  in_progress = %d\r\n", in_progress ) ) ;
    channel -> push ( netFormat ( "  has_password = %d\r\n", has_password ) ) ;
  }
};


class netRoomServerInfo
{
public:

  char name [ NET_MAX_NAME+1 ] ;
  int net_version ; //network library version
  bool has_password ;

  //address
  char host [ NET_MAX_NAME+1 ] ;
  int port ;
  netGuid game_guid ;

  int max_games ;
  int max_players ;
  int num_games ;
  int num_players ;
  int num_openings ;

  netRoomServerInfo () {}

  netRoomServerInfo ( cchar* _host, int _port, const netGuid& _game_guid )
  {
    memset ( this, 0, sizeof(netRoomServerInfo) ) ;
    net_version = NET_VERSION ;
    has_password = false ;

    netCopyName ( host, _host ) ;
    port = _port ;
    memcpy ( &game_guid, &_game_guid, sizeof(game_guid) ) ;

    max_games = 64 ;
    max_players = 64 ;
  }

  void get ( const netMessage& msg )
  {
    msg.gets ( name, sizeof(name) ) ;
    net_version = msg.geti () ;
    has_password = msg.getb () ;

    msg.gets ( host, sizeof(host) ) ;
    port = msg.geti () ;
    msg.geta ( &game_guid, sizeof(game_guid) ) ;

    max_games = msg.geti () ;
    max_players = msg.geti () ;
    num_games = msg.geti () ;
    num_players = msg.geti () ;
    num_openings = msg.geti () ;
  }

  void put ( netMessage& msg ) const
  {
    msg.puts ( name ) ;
    msg.puti ( net_version ) ;
    msg.putb ( has_password ) ;

    msg.puts ( host ) ;
    msg.puti ( port ) ;
    msg.puta ( &game_guid, sizeof(game_guid) ) ;

    msg.puti ( max_games ) ;
    msg.puti ( max_players ) ;
    msg.puti ( num_games ) ;
    msg.puti ( num_players ) ;
    msg.puti ( num_openings ) ;
  }

  void push ( netChat* channel, char *indent = "" ) const
  {
    channel -> push ( "netRoomServerInfo:\r\n" ) ;

    channel -> push ( netFormat ( "  name = %s\r\n", name ) ) ;
    channel -> push ( netFormat ( "  net_version = %d\r\n", net_version ) ) ;
    channel -> push ( netFormat ( "  has_password = %d\r\n", has_password ) ) ;

    channel -> push ( netFormat ( "  host = %s\r\n", host ) ) ;
    channel -> push ( netFormat ( "  port = %d\r\n", port ) ) ;
    channel -> push ( "  game_guid = " ) ;
    game_guid.push ( channel ) ;

    channel -> push ( netFormat ( "  max_games = %d\r\n", max_games ) ) ;
    channel -> push ( netFormat ( "  max_players = %d\r\n", max_players ) ) ;
    channel -> push ( netFormat ( "  num_games = %d\r\n", num_games ) ) ;
    channel -> push ( netFormat ( "  num_players = %d\r\n", num_players ) ) ;
    channel -> push ( netFormat ( "  num_openings = %d\r\n", num_openings ) ) ;
  }
};


#endif //__NET_ROOM__
