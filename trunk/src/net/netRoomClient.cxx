#include "netRoomClient.h"


int netRoomClient::last_error = 0 ;


netRoomClient::netRoomClient ( cchar* host, int port )
{
  last_error = 0 ;

  local_id = 0;
  state = STATE_DISCONNECT;

  num_games = 0 ;
  num_players = 0 ;
  next_game = 0 ;
  next_player = 0 ;
  max_games = 0 ;
  max_players = 0 ;
  games = 0 ;
  players = 0 ;

  open ();
	connect (host, port);
}


void netRoomClient::login ( cchar* player_name,
  const netGuid& game_guid, cchar* server_password )
{
  if ( server_password == NULL )
    server_password = "";

  netMessage msg ( netRoom::LOGIN, 0, local_id ) ;
  msg.puti ( NET_VERSION ) ;
  msg.puta ( &game_guid, sizeof(game_guid) ) ;
  msg.puts ( player_name ) ;
  msg.puts ( server_password ) ;
  sendMessage ( msg ) ;

  state = STATE_LOGIN_REPLY;
}


netRoomClient::~netRoomClient ()
{
  if ( games ) delete [] games ;
  if ( players ) delete [] players ;
  games = 0 ;
  players = 0 ;
}


void netRoomClient::logout ()
{
  if ( state != STATE_DISCONNECT )
  {
    netMessage msg ( netRoom::LOGOUT, 0, local_id ) ;
    sendMessage ( msg ) ;
    closeWhenDone() ;
  }
}
  

void netRoomClient::processUpdatePlayer ( const netMessage& msg )
{
  netRoomPlayerInfo player ;
  player.get ( msg ) ;

  if ( player.id < 0 )
  {
    //delete player
    const netRoomPlayerInfo* p = findPlayer ( -player.id ) ;
    if ( p != NULL )
    {
      u32 i = p - players ;
      num_players -- ;
      memmove ( &players[i], &players[i+1],
        sizeof(netRoomPlayerInfo) * (num_players-i) ) ;
    }
  }
  else  //update
  {
    netRoomPlayerInfo* p = (netRoomPlayerInfo*)findPlayer ( player.id ) ;
    if ( p == NULL && num_players < max_players )
    {
      //add
      p = &players[num_players++];
      memset( p, 0, sizeof(netRoomPlayerInfo) );
    }

    if ( p != NULL )
    {
      int old_game = p -> game_id ;
      memcpy ( p, &player, sizeof(netRoomPlayerInfo) ) ;

      //is game changing?
      if ( old_game != p -> game_id )
      {
        //get local player
        int player_id = getPlayerID();
        
        //was this player in our game?
        if ( old_game && getGameID() == old_game )
        {
          netMessage msg ( netRoom::PLAYER_LEFT, player_id, p -> id );
          processMessage ( msg ) ;
        }
        
        //did player join our game?
        if ( p -> id != player_id && getGameID() == p -> game_id )
        {
          netMessage msg ( netRoom::PLAYER_JOINED, player_id, p -> id );
          processMessage ( msg ) ;
        }
      }
    }
  }
}


void netRoomClient::processUpdateGame ( const netMessage& msg )
{
  netRoomGameInfo game ;
  game.get ( msg ) ;

  if ( game.id < 0 )
  {
    //delete game
    const netRoomGameInfo* g = findGame ( -game.id ) ;
    if ( g != NULL )
    {
      u32 i = g - games ;
      num_games -- ;
      memmove ( &games[i], &games[i+1],
        sizeof(netRoomGameInfo) * (num_games-i) ) ;
    }
  }
  else  //update
  {
    netRoomGameInfo* g = (netRoomGameInfo*)findGame ( game.id ) ;
    if ( g == NULL && num_games < max_games )
    {
      //add
      g = &games[num_games++];
      memset( g, 0, sizeof(netRoomGameInfo) );
    }

    if ( g != NULL )
    {
      int old_host = g -> host_id ;
      memcpy ( g, &game, sizeof(netRoomGameInfo) ) ;
      
      //is local game changing?
      if ( g -> id == getGameID() )
      {
        //am i the host now?
        int player_id = getPlayerID();
        if ( player_id == g -> host_id && old_host != g -> host_id )
        {
          netMessage msg ( netRoom::YOU_ARE_HOST, player_id, 0 );
          processMessage ( msg ) ;
        }
      }
    }
  }
}


void netRoomClient::processMessage ( const netMessage& msg )
{
  switch ( msg.getType() )
  {
    case netRoom::ERROR_REPLY:
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
    case netRoom::INIT_GAME_LIST:
      {
        max_games = msg.geti () ;
        num_games = 0 ;

        if ( games )
          delete [] games ;
        games = 0 ;

        if ( max_games )
        {
          games = new netRoomGameInfo [ max_games ] ;
          memset ( games, 0, sizeof(netRoomGameInfo) * max_games ) ;
        }
        break;
      }
    case netRoom::UPDATE_GAME_LIST:
      {
        processUpdateGame ( msg ) ;
        break;
      }
    case netRoom::INIT_PLAYER_LIST:
      {
        max_players = msg.geti () ;
        num_players = 0 ;

        if ( players )
          delete [] players ;
        players = 0 ;

        if ( max_players )
        {
          players = new netRoomPlayerInfo [ max_players ] ;
          memset ( players, 0, sizeof(netRoomPlayerInfo) * max_players ) ;
        }
        break;
      }
    case netRoom::UPDATE_PLAYER_LIST:
      {
        processUpdatePlayer ( msg ) ;
        break;
      }
    default:
      {
        netMessageChannel::processMessage ( msg ) ;
        break;
      }
  }
}


bool netRoomClient::createGame ( cchar* _name, cchar* _password, int max_players )
{
  char name[ NET_MAX_NAME+1 ];
  netCopyName ( name, _name ) ;

  char password[ NET_MAX_NAME+1 ];
  netCopyName ( password, _password ) ;

  if ( state == STATE_READY )
  {
    netMessage msg ( netRoom::CREATE_GAME, 0, local_id );
    msg.puti( max_players ) ;
    msg.puts( name );
    msg.puts( password );
    sendMessage ( msg ) ;

    state = STATE_JOIN_REPLY;
    last_error = netRoom::ERROR_NONE; //pending
    return(true);
  }
  return(false);
}


bool netRoomClient::joinGame ( int game_id, cchar* _password )
{
  char password[ NET_MAX_NAME+1 ];
  netCopyName ( password, _password ) ;

  const netRoomGameInfo* g = findGame( game_id );
  if ( g && state == STATE_READY )
  {
    bool watching = false;

    netMessage msg( netRoom::JOIN_GAME, 0, local_id );
    msg.puti ( game_id ) ;
    msg.putb ( watching ) ;
    msg.puts( password );
    sendMessage ( msg ) ;

    state = STATE_JOIN_REPLY;
    last_error = netRoom::ERROR_NONE; //pending
    return(true);
  }
  return(false);
}


bool netRoomClient::watchGame ( int game_id, cchar* _password )
{
  char password[ NET_MAX_NAME+1 ];
  netCopyName ( password, _password ) ;

  const netRoomGameInfo* g = findGame( game_id );
  if ( g && state == STATE_READY )
  {
    bool watching = true;

    netMessage msg( netRoom::JOIN_GAME, 0, local_id );
    msg.puti ( game_id ) ;
    msg.putb ( watching ) ;
    msg.puts ( password ) ;
    sendMessage ( msg ) ;

    state = STATE_JOIN_REPLY;
    last_error = netRoom::ERROR_NONE; //pending
    return(true);
  }
  return(false);
}


void netRoomClient::startGame ( int game_id )
{
  const netRoomGameInfo	* g = findGame( game_id );
  if ( g && isGameHost() )
  {
    netMessage	msg( netRoom::START_GAME, 0, local_id );
    msg.puti ( game_id ) ;
    sendMessage ( msg ) ;
  }
}


void netRoomClient::leaveGame ()
{
  if ( state == STATE_READY && getGameID() )
  {
    netMessage msg( netRoom::LEAVE_GAME, 0, local_id );
    sendMessage ( msg ) ;
  }
}
