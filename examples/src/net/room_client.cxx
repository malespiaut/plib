/****
* NAME
*   room_client
*
* DESCRIPTION
*   simple room client example of the game 'tic-tac-toe'.
*
*   it was a challenge to make this example small and
*   without using a sophisticated user interface.  perhaps
*   one day i'll make something better with PUI/GLUT/OpenGL
*
* AUTHORS
*   Dave McClurg <dpm@efn.org>
*
* CREATION DATE
*   Dec-2000
*
****/

#include "netRoomClient.h"
#include "netRoomMaster.h"
#include "kb.h"


static netRoomClient* client = 0 ;

// {78173AC0-D754-11d4-8748-00D0B796C186}
static const netGuid tic_guid (
  0x78173ac0, 0xd754, 0x11d4, 0x87, 0x48, 0x0, 0xd0, 0xb7, 0x96, 0xc1, 0x86 ) ;


class Game
{
public:
  enum Messages  //application messages start at 0
  {
    MSG_START_GAME,
    MSG_TAKE_TURN
  } ;

  bool is_host;
  bool my_turn;
  int local_id;
  int other_id;
  char board[9]; // X,O, or blank (host=X)

  void init( bool _is_host )
  {
    is_host = _is_host ;
    my_turn = _is_host ;

    local_id = client -> getPlayerID();
    other_id = 0 ;

    memset(&board,' ',sizeof(board));
  }

  void drawBoard () const
  {
    printf( " %c | %c | %c\n", board[0],board[1],board[2] );
    printf( "---+---+---\n" );
    printf( " %c | %c | %c\n", board[3],board[4],board[5] );
    printf( "---+---+---\n" );
    printf( " %c | %c | %c\n", board[6],board[7],board[8] );
  }

  bool haveWinner () const
  {
    bool tie = true;
    int arr[ 9 ];
    for ( int i=0; i<9; i++ )
    {
      switch ( board[i] )
      {
      case 'X': arr[i] = 1; break;
      case 'O': arr[i] = -1; break;
      default:
        arr[i] = 0;
        tie = false;
        break;
      }
    }
    int wins[ 8*3 ] =
    {
      0,1,2,
      3,4,5,
      6,7,8,
      0,3,6,
      1,4,7,
      2,5,8,
      0,4,8,
      2,4,6,
    };
    for ( i=0; i<8; i++ )
    {
      int sum = 0;
      for ( int j=0; j<3; j++ )
        sum += arr[ wins[ i*3 + j ] ];
      if ( abs(sum) == 3 )
        return(true);
    }
    if ( tie )
      return(true);
    return(false);
  }
  
  bool markSpot ( int player_id, int spot )
  {
    if ( spot >= '1' && spot <= '9' )
    {
      int i = spot-'1';
      if ( board[ i ] == ' ' )
      {
        int mark = 'O' ;
        if ( (player_id == local_id) == is_host )
          mark = 'X' ;
        board[ i ] = mark ;
        
        if ( haveWinner() )
        {
          printf("we have a winner! the game is over\n");
          exit(0);
        }
        return true ;
      }
    }
    return false ;
  }
  
  bool takeTurn ( int spot )
  {
    if ( markSpot(local_id,spot) )
    {
      netMessage msg ( MSG_TAKE_TURN, other_id, local_id ) ;
      msg.puti ( spot ) ;
      client -> sendMessage ( msg ) ;
      my_turn = false ;
      return true ;
    }
    return false ;
  }

  void processMessage( const netMessage& msg )
  {
    switch (msg.getType())
    {
    case netRoom::PLAYER_LEFT:
      {
        printf("other player has left! the game is over\n");
        exit(0);
        break;
      }
    case netRoom::PLAYER_JOINED:
      {
        if ( is_host && !other_id )
        {
          other_id = msg.getFromID() ;
          
          netMessage msg ( MSG_START_GAME, other_id, local_id ) ;
          client -> sendMessage ( msg );
        }
        break;
      }
    case MSG_START_GAME:
      {
        if ( !is_host && !other_id )
        {
          other_id = msg.getFromID() ;
        }
        break;
      }
    case MSG_TAKE_TURN:
      {
        if ( !my_turn )
        {
          int spot;
          spot = msg.geti () ;
          markSpot ( msg.getFromID(), spot ) ;
          my_turn = true ;
        }
        break;
      }
    }
  }
};


static Game game ;


static void processfn ( const netMessage& msg )
{
  game.processMessage ( msg ) ;
}


static void idlefn ()
{
  netChannel::poll(0) ;
  int err = client -> getLastError() ;
  if ( err )
  {
    printf ( "network error = %d\n", err ) ;
    exit(1) ;
  }
}


/*
** main - this is where it all starts
*/
int main ( int argc, char **argv )
{
  netInit ( &argc, argv ) ;

  /*
  **  get the list of room servers
  */

  printf ( "Finding room servers...\n" ) ;

  netRoomServerList servers ;
  netRoomBrowser browser ( &servers ) ;
  browser.sendRequest ( "localhost", 8787, tic_guid ) ;

  const netRoomServerInfo* server ;
  while ( !(server = servers.get (0)) )
    idlefn () ;

  /*
  **  login to the room server
  */

  printf ( "Logging in to room %s...\n", server -> name ) ;

  client = new netRoomClient ( server -> host, server -> port ) ;
  client -> login ( "Dave", tic_guid, NULL ) ;

  while ( ! client -> isLoggedIn () )
    idlefn () ;

  /*
  **  login to the room server
  */

  printf ( "Create a new game? (y/n) " ) ;
  while ( !kbHit() )
    idlefn();
  int key = kbGet () ;
  printf ( "%c\n", key ) ;

  if ( key == 'y' || key == 'Y' )
  {
    printf ( "Creating game...\n" ) ;
    game.init( true );
    client -> setCallback ( processfn ) ;
    client -> createGame ( "fun", NULL, 2 ) ;
  }
  else
  {
    printf ( "Finding game to join...\n" ) ;

    const netRoomGameInfo* g ;
    while ( ! (g = client -> getGame (0)) )
      idlefn () ;

    printf ( "Joining game...\n" ) ;
    game.init( false );
    client -> setCallback ( processfn ) ;
    client -> joinGame (g->id,NULL) ;
  }

  /*
  **  Wait until we are in the game
  */

  while ( ! client -> getGameID () )
    idlefn () ;

  printf ("waiting for the other player to join...\n");

  while ( ! game.other_id )
    idlefn () ;

  while (1)
  {
    if ( game.my_turn )
    {
      while ( 1 )
      {
        game.drawBoard();
        printf ( "Pick a spot: (1-9) " ) ;
        while ( !kbHit() )
          idlefn();
        int spot = kbGet () ;
        printf ( "%c\n", spot ) ;
        if ( game.takeTurn(spot) )
          break;
        printf("not that one. try again...\n");
      }
    }
    else
    {
      game.drawBoard();
      printf("waiting for other player to take his turn...\n");
      while ( ! game.my_turn )
        idlefn () ;
    }
  }

  client -> logout () ;
  delete client ;
  client = 0 ;

//  int timeout = clock() + CLOCKS_PER_SEC*3;
//  while ( clock() < timeout && ! link -> isLoggedIn () )
//    netPoll (0);
  exit(0);

  return 0 ;
}
