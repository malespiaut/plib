#include "netRoomServer.h"


#if !defined (WIN32)
#define stricmp strcasecmp
#define strnicmp strncasecmp
#endif

//game_id and player_id must fit into message header bytes
#define MAX_IDNUM 256


void netRoomPlayerState::sendReply( int error )
{
  netMessage msg( netRoom::ERROR_REPLY, id, 0 );
  msg.puti ( error ) ;
  sendMessage( msg );
}


void netRoomPlayerState::sendMessage ( const netMessage& msg )
{
  if ( channel )
  {
    channel -> sendMessage ( msg ) ;
  }
}


void netRoomPlayerState::init ( int fd )
{
  channel = new netRoomPlayerChannel ( this, fd ) ;
  gagged = false ;
}


void netRoomPlayerState::disconnect (void)
{
  leaveGame();
  printf("Player %s disconnected.\n", name);

  if ( channel )
    channel -> disconnect () ;

  server -> delPlayer (this);
}


bool netRoomPlayerState::isGameHost() const
{
  netRoomGameState* g = server -> findGame( game_id );
  if ( g )
  {
    if ( g->host_id == id )
      return(true);
  }
  return(false);
}


void netRoomPlayerState::leaveGame()
{
  netRoomGameState* g = server -> findGame( game_id );
  if ( g )
  {
    //are there any other players in this game?
    netRoomPlayerState* found = 0;
    netRoomPlayerState* p = server -> getPlayer ( 0 ) ;
    for ( ; !found && p != NULL ; p = server -> getNextPlayer () )
      if ( p != this && p->game_id == game_id )
        found = p;

    game_id = 0;

    server -> updatePlayer ( this ) ;

    printf("Player %s left game %s.\n", name, g->name);

    if ( found )
    {
      if ( g->host_id == id )
      {
        //migrate host
        g->host_id = found -> id ;
        server -> updateGame ( g ) ;
        printf("Player %s is now host of game %s\n", found->name, g->name);
      }
    }
    else
    {
      printf("Game %s was disbanded.\n", g->name);
      server -> delGame ( g ) ;
    }
  }
}


void netRoomPlayerState::processLogin ( const netMessage &msg )
{
  int net_version ;
  netGuid game_guid ;
  char password[NET_MAX_NAME+1];

  net_version = msg.geti () ;
  msg.geta ( &game_guid, sizeof(game_guid) ) ;
  msg.gets ( name, sizeof(name) ) ;
  msg.gets ( password, sizeof(password) ) ;

  server -> makeUnique( name );

  printf("Player %s connected.\n", name);

  int error = netRoom::ERROR_NONE ;
  if ( net_version != NET_VERSION )
  {
    error = netRoom::ERROR_WRONG_NET_VERSION ;
  }
  else if ( game_guid != server -> info.game_guid )
  {
    error = netRoom::ERROR_WRONG_GAME_GUID ;
  }
  else if ( server -> password[0] != 0 && stricmp(password,server -> password) )
  {
    error = netRoom::ERROR_WRONG_SERVER_PASSWORD ;
  }

  //reply to login with player ID
  sendReply( error ) ;

  if ( error != netRoom::ERROR_NONE )
  {
    disconnect () ;
    return ;
  }

  {
    netMessage msg( netRoom::INIT_PLAYER_LIST, id, 0 );
    msg.puti ( server -> max_players ) ;
    sendMessage( msg );

    netRoomPlayerState* p = server -> getPlayer ( 0 ) ;
    for ( ; p != NULL ; p = server -> getNextPlayer () )
    {
      netMessage msg( netRoom::UPDATE_PLAYER_LIST, id, 0 );
      p -> put ( msg ) ;
      sendMessage( msg );
    }
  }

  {
    netMessage msg( netRoom::INIT_GAME_LIST, id, 0 );
    msg.puti ( server -> max_games ) ;
    sendMessage( msg );

    netRoomGameState* g = server -> getGame ( 0 ) ;
    for ( ; g != NULL ; g = server -> getNextGame () )
    {
      netMessage msg( netRoom::UPDATE_GAME_LIST, id, 0 );
      g -> put ( msg ) ;
      sendMessage( msg );
    }
  }

  /*
  **  tell everyone else i'm here!
  */
  server -> updatePlayer (this);
}


void netRoomPlayerState::processGameStart( const netMessage &msg )
{
  int error = netRoom::ERROR_NONE ;
  int	_game_id = msg.geti () ;

  netRoomGameState* g = server -> findGame( _game_id );
  if ( g )
  {
    g -> in_progress = true;
    server -> updateGame (g);

    printf( "Game %s has started.\n", g->name );
  }

  sendReply( error );
}


void netRoomPlayerState::processCreateGame( const netMessage& msg )
{
  int error = netRoom::ERROR_NONE ;

  netRoomGameState* found = server -> addGame () ;
  if (found)
  {
    leaveGame();

    found->max_players = msg.geti () ;
    msg.gets( found->name, sizeof(found->name) );
    msg.gets( found->password, sizeof(found->password) );

    server -> makeUnique( found->name );
    found->has_password = (found->password[0] != 0);
    found->host_id = id ;
    found->in_progress = false;

    game_id = found -> id ;
    watching = false ;

    server -> updateGame ( found ) ;
    server -> updatePlayer ( this ) ;

    printf("Player %s created game %s.\n", name, found->name);
  }
  else
    error = netRoom::ERROR_TOO_MANY_GAMES;
  
  sendReply( error );
}


void netRoomPlayerState::processJoinGame( const netMessage& msg )
{
  int error = netRoom::ERROR_INVALID_GAME;
  int _game_id;
  bool _watching;
  char _password[NET_MAX_NAME+1];

  _game_id = msg.geti () ;
  _watching = msg.getb () ;
  msg.gets( _password, sizeof(_password) ) ;

  netRoomGameState* g = server -> findGame( _game_id );
  if ( g )
  {
    //are there any other players in this game?
    int count = 0;
    netRoomPlayerState* p = server -> getPlayer ( 0 ) ;
    for ( ; p != NULL ; p = server -> getNextPlayer () )
      if ( p != this && p->game_id == _game_id )
        count++;

    if ( g->password[0] != 0 && stricmp(_password,g->password) )
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

      game_id = _game_id;
      watching = _watching;
      server -> updatePlayer ( this ) ;

      printf("Player %s joined game %s.\n", name, g->name);
      error = netRoom::ERROR_NONE ;
    }
  }
  sendReply( error );
}


void netRoomPlayerState::processMessage ( const netMessage& msg )
{
  switch ( msg.getType() )
  {
  case netRoom::LOGIN:
    processLogin( msg );
    break;
  case netRoom::LOGOUT:
    disconnect();
    break;
  case netRoom::CREATE_GAME:
    processCreateGame( msg );
    break;
  case netRoom::JOIN_GAME:
    processJoinGame( msg );
    break;
  case netRoom::LEAVE_GAME:
    leaveGame();
    break;
  case netRoom::START_GAME:
    processGameStart( msg );
    break;
  default:
    /*
    **  Echo chat messages
    */
    if ( msg.getType() == netRoom::CHAT )
    {
      const netRoomPlayerState	*from = server -> findPlayer ( msg.getFromID() );
      if ( from -> gagged )
        break;

      char text[256];
      msg.gets( text, sizeof(text) );
      
      if ( msg.getToID() )
      {
        const netRoomPlayerInfo	*to = server -> findPlayer ( msg.getToID() );
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
      netRoomPlayerState	*p = server -> findPlayer( msg.getToID() );
      if ( p )
        p->sendMessage( msg );
    }
    else //broadcast
    {
      netRoomPlayerState* p = server -> getPlayer ( 0 ) ;
      for ( ; p != NULL ; p = server -> getNextPlayer () )
      {
        if ( p != this && p->game_id == game_id )
          p->sendMessage( msg );
      }
    }
    break;
  }
}


const netRoomServerInfo* netRoomServer::getInfo ()
{
  //set dynamic info

  info.num_games = num_games ;
  info.num_players = num_players ;
  info.num_openings = 0 ;
  int	num_occupied = 0;

  const netRoomGameState* g = getGame ( 0 ) ;
  for ( ; g != NULL ; g = getNextGame () )
  {
    if ( !g->in_progress )
      info.num_openings += g->max_players;
  }

  const netRoomPlayerState* p = getPlayer ( 0 ) ;
  for ( ; p != NULL ; p = getNextPlayer () )
  {
    const netRoomGameState* g = findGame ( p->game_id ) ;
    if ( p->game_id && !g->in_progress )
      info.num_openings -- ;
  }

  return & info ;
}


netRoomServer::netRoomServer ( const netRoomServerInfo& _info )
{
  memset(password,0,sizeof(password));
  memcpy ( &info, &_info, sizeof(info) ) ;

  num_games = 0 ;
  num_players = 0 ;
  next_game = 0 ;
  next_player = 0 ;
  max_games = info.max_games ;
  max_players = info.max_players ;

  assert ( max_games && max_players ) ;

  games = new netRoomGameState [ max_games ] ;
  players = new netRoomPlayerState [ max_players ] ;
  memset ( games, 0, sizeof(netRoomGameState) * max_games ) ;
  memset ( players, 0, sizeof(netRoomPlayerState) * max_players ) ;

	create ();
	bind ("localhost", info.port);
	listen (5);

  printf("Room server \"%s\" started on port %d\n",info.name,info.port);
}


netRoomServer::~netRoomServer ()
{
  delete [] games ;
  delete [] players ;
  games = 0 ;
  players = 0 ;
}


netRoomGameState* netRoomServer::addGame ()
{
  if ( num_games < max_games )
  {
    /* get a unique network id for the player */
    bool id_used [MAX_IDNUM];
    memset ( id_used, 0, sizeof(id_used) ) ;
    const netRoomGameState* g = getGame ( 0 ) ;
    for ( ; g != NULL ; g = getNextGame () )
    {
      assert ( g->id > 0 && g->id < MAX_IDNUM ) ;
      id_used [g->id] = true;
    }
    /* pick lowest unused id */
    int id = 0 ;
    for ( int i=1; i<MAX_IDNUM; i++ )
    {
      if ( !id_used[i] )
      {
        id = i;
        break;
      }
    }
    assert( id ) ;

    netRoomGameState* found = &games[num_games++];
    memset( found, 0, sizeof(netRoomGameState) );
    found->id = id ;
    found->server = this;
    return found ;
  }
  return NULL ;
}


void netRoomServer::delGame ( netRoomGameState* g )
{
  g->id = - g->id;
  updateGame ( g ) ;

  u32 i = g - games ;
  num_games -- ;
  memmove ( &games[i], &games[i+1], sizeof(netRoomGameState) * (num_games-i) ) ;
}


netRoomPlayerState* netRoomServer::addPlayer ()
{
  if ( num_players < max_players )
  {
    /* get a unique network id for the player */
    bool id_used [MAX_IDNUM];
    memset ( id_used, 0, sizeof(id_used) ) ;
    const netRoomPlayerState* p = getPlayer ( 0 ) ;
    for ( ; p != NULL ; p = getNextPlayer () )
    {
      assert ( p->id > 0 && p->id < MAX_IDNUM ) ;
      id_used [p->id] = true;
    }
    /* pick lowest unused id */
    int id = 0 ;
    for ( int i=1; i<MAX_IDNUM; i++ )
    {
      if ( !id_used[i] )
      {
        id = i;
        break;
      }
    }
    assert( id ) ;

    netRoomPlayerState* found = &players[num_players++];
    memset( found, 0, sizeof(netRoomPlayerState) );
    found->id = id ;
    found->server = this ;
    return found ;
  }
  return NULL ;
}


void netRoomServer::delPlayer ( netRoomPlayerState* p )
{
  p->id = - p->id;
  updatePlayer ( p ) ;

  u32 i = p - players ;
  num_players -- ;
  memmove ( &players[i], &players[i+1], sizeof(netRoomPlayerState) * (num_players-i) ) ;
}


void netRoomServer::updatePlayer( netRoomPlayerState* player )
{
  netMessage msg( netRoom::UPDATE_PLAYER_LIST, 0, 0 );
  player -> put ( msg ) ;

  netRoomPlayerState* p = getPlayer ( 0 ) ;
  for ( ; p != NULL ; p = getNextPlayer () )
    p->sendMessage( msg );
}


void netRoomServer::updateGame( netRoomGameState* game )
{
  netMessage msg( netRoom::UPDATE_GAME_LIST, 0, 0 );
  game -> put ( msg ) ;

  netRoomPlayerState* p = getPlayer ( 0 ) ;
  for ( ; p != NULL ; p = getNextPlayer () )
    p->sendMessage( msg );
}


void netRoomServer::handleAccept (void)
{
  netRoomPlayerState* found = addPlayer () ;
  if (found)
  {
    netAddress addr ;
    int fd = accept (&addr) ;
  	printf("%d: Client %s:%d connected\n",fd,addr.getHost(),addr.getPort());
  
    found -> init ( fd ) ;
  }
}


void netRoomServer::makeUnique( char* name ) const
{
  char	filtered_name[ 250 ];
  strcpy( filtered_name, name );
  int		append = 0;

  u32 len = strlen( filtered_name );
  u32 i ;

  const netRoomPlayerState* p = players ;
  for ( i=0; i<num_players; i++, p++ )
  {
    if ( p->name != name &&
         (strnicmp(p->name, filtered_name, len) == 0) && 
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

  const netRoomGameState* g = games ;
  for ( i=0; i<num_games; i++, g++ )
  {
    if ( g->name != name && 
         (strnicmp( g->name, filtered_name, len ) == 0) && 
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


void netRoomServer::gagPlayer( int player_id, bool flag )
{
  netRoomPlayerState* p = findPlayer ( player_id ) ;
  if ( p )
  {
    p -> gagged = flag ;
  }
}


void netRoomServer::kickPlayer( int player_id )
{
  netRoomPlayerState* p = findPlayer ( player_id ) ;
  if ( p )
  {
    p -> disconnect () ;
  }
}


void netRoomServer::banPlayer( int player_id, bool flag )
{
  netRoomPlayerState* p = findPlayer ( player_id ) ;
  if ( p )
  {
    if ( flag )
      p -> disconnect () ;
    //NEED: permanent banning and unbanning
  }
}
