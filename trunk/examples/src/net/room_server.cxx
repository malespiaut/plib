#include "netRoomServer.h"
#include "netRoomMaster.h"
#include "netMonitor.h"


static netRoomServer* room = 0 ;


void processCommand ( cchar* _line, netMonitorChannel* channel )
{
  if ( strlen(_line) > 255 )
    return;
  char line [ 256 ] ;
  strcpy ( line, _line ) ;

  /*
  parse the arguments
  */
  enum { MAX_ARGS = 10 } ;
  char* argv [ MAX_ARGS+1 ] ;
  int argc = 0 ;
  char* argp = strtok ( line, " \t" ) ;
  while ( argp != NULL && argc < MAX_ARGS )
  {
    argv [ argc ++ ] = argp ;
    argp = strtok ( NULL, " \t" ) ;
  }
  argv [ argc ] = NULL ;
  if ( argc == 0 )
    return;

  cchar* command = argv [ 0 ] ;
  if (strcmp(command,"list_games") == 0)
  {
    const netRoomGameInfo* g = room -> getGame ( 0 ) ;
    for ( ; g != NULL ; g = room -> getNextGame () )
      channel -> push ( avar("%d %s\r\n",g->id,g->name) );
    channel -> push (".\r\n");
  }
  else if (strcmp(command,"list_players") == 0)
  {
    const netRoomPlayerInfo* p = room -> getPlayer ( 0 ) ;
    for ( ; p != NULL ; p = room -> getNextPlayer () )
      channel -> push ( avar("%d %s\r\n",p->id,p->name) );
    channel -> push (".\r\n");
  }
  else if (strcmp(command,"print_game") == 0)
  {
    if ( argc > 1 )
    {
      int game_id = atoi(argv[1]);
      const netRoomGameInfo* g = room -> findGame ( game_id ) ;
      if ( g != NULL )
        g -> push ( channel ) ;
    }
  }
  else if (strcmp(command,"print_player") == 0)
  {
    if ( argc > 1 )
    {
      int player_id = atoi(argv[1]);
      const netRoomPlayerInfo* p = room -> findPlayer ( player_id ) ;
      if ( p != NULL )
        p -> push ( channel ) ;
    }
  }
  else if (strcmp(command,"gag") == 0)
  {
    if ( argc > 1 )
    {
      int player_id = atoi(argv[1]);
      room -> gagPlayer ( player_id, true ) ;
    }
  }
  else if (strcmp(command,"ungag") == 0)
  {
    if ( argc > 1 )
    {
      int player_id = atoi(argv[1]);
      room -> gagPlayer ( player_id, false ) ;
    }
  }
  else if (strcmp(command,"kick") == 0)
  {
    if ( argc > 1 )
    {
      int player_id = atoi(argv[1]);
      room -> kickPlayer ( player_id ) ;
    }
  }
  else if (strcmp(command,"ban") == 0)
  {
    if ( argc > 1 )
    {
      int player_id = atoi(argv[1]);
      room -> banPlayer ( player_id, true ) ;
    }
  }
  else if (strcmp(command,"unban") == 0)
  {
    if ( argc > 1 )
    {
      int player_id = atoi(argv[1]);
      room -> banPlayer ( player_id, false ) ;
    }
  }
  else if (strcmp(command,"help") == 0)
  {
    channel -> push("help\r\n");
    channel -> push("exit\r\n");
    channel -> push("list_games\r\n");
    channel -> push("list_players\r\n");
    channel -> push("print_game <player_id>\r\n");
    channel -> push("print_player <player_id>\r\n");
    channel -> push("gag <player_id>\r\n");
    channel -> push("ungag <player_id>\r\n");
    channel -> push("kick <player_id>\r\n");
    channel -> push("ban <player_id>\r\n");
    channel -> push("unban <player_id>\r\n");
    channel -> push (".\r\n");
  }
  else
  {
    printf("echo: %s\n",command);
  
    channel -> push(command);
    channel -> push("\r\n");
  }
}


// {78173AC0-D754-11d4-8748-00D0B796C186}
static const netGuid tic_guid (
  0x78173ac0, 0xd754, 0x11d4, 0x87, 0x48, 0x0, 0xd0, 0xb7, 0x96, 0xc1, 0x86 ) ;


//----------------------------------------------------------------------
int main (int argc, char * argv[])
{
  cchar* server_name = "Test" ;
  if (argc >= 2)
    server_name = argv[1];

  int server_port = 8888 ;
  if (argc >= 3)
    server_port = atoi (argv[2]);

  netInit(&argc,argv);

  netRoomServerInfo server_info ( "localhost", server_port, tic_guid ) ;
  netCopyName ( server_info.name, server_name ) ;
  new netRoomAdvertiser ( "localhost", 8787, server_info ) ;

  room = new netRoomServer ( server_info ) ;
  //room -> setPassword ( "foo" ) ;

  netMonitorServer* monitor ;
  monitor = new netMonitorServer ( server_name, server_port+1 ) ;
  monitor -> setPassword ( "foobar" ) ;
  monitor -> setCommandFunc ( processCommand ) ;

  netChannel::loop (0) ;
  return 0;
}
