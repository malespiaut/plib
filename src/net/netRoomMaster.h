/****
* NAME
*   netRoomMaster - game room master
*
* DESCRIPTION
*   a game room master server keeps track of which
*   game room servers are running and information
*   about each one.  game room servers update their
*   info and game room clients download the list
*   of game room servers.
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

#ifndef __NET_ROOM_MASTER__
#define __NET_ROOM_MASTER__


#include "netRoom.h"


class netRoomServerList
{
  u32 num, max ;
  netRoomServerInfo* list ;
  u32 next ;  /* The next i when we are doing getNext ops */

  netRoomServerInfo* match ( const netRoomServerInfo* s )
  {
    //compare host,port
    netRoomServerInfo* p = list ;
    for ( u32 i = 0; i < num; i ++, p ++ )
    {
      if ( stricmp ( p -> host, s -> host ) == 0 &&
        p -> port == s -> port )
      {
        return p ;
      }
    }
    return 0 ;
  }

public:

  netRoomServerList ( u32 max=0 )
  {
    list = 0 ;
    init(max);
  }

  ~netRoomServerList ()
  {
    if ( list )
      delete [] list ;
    list = 0 ;
  }

  void clear ()
  {
    if ( list )
      memset ( list, 0, sizeof(netRoomServerInfo) * max ) ;
    num = 0 ;
  }

  void init ( u32 _max )
  {
    if ( list )
      delete [] list ;
    list = 0 ;

    max = _max ;
    if ( max )
      list = new netRoomServerInfo [ max ] ;

    clear () ;
  }

  u32 getNum () const { return num ; }

  netRoomServerInfo* get ( u32 i )
  {
    next = i ;
    return ( i >= num ) ? NULL : &list [ i ] ;
  }

  netRoomServerInfo* getNext ()
  {
    return get ( next+1 ) ;
  }

  void remove ( const netRoomServerInfo* s )
  {
    u32 i = s - list ;
    if ( i < num )
    {
      num -- ;
      memmove ( &list[i], &list[i+1], sizeof(netRoomServerInfo) * (num-i) ) ;
    }
  }

  void add ( const netRoomServerInfo* s )
  {
    netRoomServerInfo* p = match ( s ) ;
    if ( p == NULL && num < max )
      p = &list[ num++ ] ;

    if ( p != NULL )
      memcpy ( p, s, sizeof(netRoomServerInfo) ) ;
  }
} ;


class netRoomMasterChannel : public netMessageChannel
{
  char host [ NET_MAX_NAME+1 ] ;
  netRoomServerList* servers ;

  void sendReply( int error )
  {
    netMessage msg( netRoom::ERROR_REPLY, 0, 0 );
    msg.puti ( error ) ;
    sendMessage( msg );
  }

  void processRequest ( const netMessage &msg ) ;
  void processUpdate ( const netMessage &msg ) ;

  virtual void handleClose (void)
  {
    shouldDelete () ;
  }

public:

  netRoomMasterChannel ( cchar* _host, int s, netRoomServerList* _servers )
  {
    netCopyName ( host, _host ) ;
    setHandle ( s ) ;
    servers = _servers ;
  }

  virtual void processMessage ( const netMessage& msg ) ;
} ;


class netRoomMasterServer : private netChannel
{
  virtual bool writable (void)
  {
    return false ;
  }

  virtual void handleAccept (void)
  {
    netAddress addr ;
    int fd = accept ( &addr ) ;
    cchar* host = addr.getHost() ;
		printf("%d: Client %s:%d connected\n",fd,host,addr.getPort());

    new netRoomMasterChannel ( host, fd, &servers ) ;
  }

public:

  netRoomServerList servers ;

  netRoomMasterServer ( int port ) : servers ( 256 )
  {
		create ();
		bind ("localhost", port);
		listen (5);
    printf("Master Server started on port %d\n",port);
  }
} ;


class netRoomBrowser : private netMessageChannel
{
  netRoomServerList* servers ;

public:

  netRoomBrowser ( cchar* host, int port,
    const netGuid& game_guid, netRoomServerList* _servers )
  {
    servers = _servers ;

    create ();
  	connect (host, port);

    netMessage msg ( netRoom::REQUEST_SERVER_LIST, 0, 0 ) ;

    msg.puti ( NET_VERSION ) ;
    msg.puta ( &game_guid, sizeof(game_guid) ) ;

    sendMessage ( msg ) ;
  }

  virtual void processMessage ( const netMessage& msg )
  {
    switch ( msg.getType() )
    {
    case netRoom::INIT_SERVER_LIST:
      {
        int max_servers = msg.geti () ;
        servers -> init ( max_servers ) ;
      }
      break;
    case netRoom::UPDATE_SERVER_LIST:
      {
        netRoomServerInfo server ;
        server.get ( msg ) ;
        servers -> add ( &server ) ;
      }
      break;
    default:
      close () ;
      break;
    }
  }

  virtual void handleClose (void)
  {
    shouldDelete () ;
  }
} ;


class netRoomAdvertiser : private netMessageChannel
{
public:

  netRoomAdvertiser ( cchar* host, int port,
    const netRoomServerInfo& server )
  {
    create ();
  	connect (host, port) ;

    netMessage msg ( netRoom::UPDATE_SERVER_LIST, 0, 0 ) ;

    msg.puti ( NET_VERSION ) ;
    server.put ( msg ) ;

    sendMessage ( msg ) ;
    closeWhenDone () ;
  }

  virtual void handleClose (void)
  {
    shouldDelete () ;
  }
} ;


#endif //__NET_ROOM_MASTER__