#include "netRoomMaster.h"


void netRoomMasterChannel::processRequest ( const netMessage &msg )
{
  int net_version = msg.geti () ;

  if ( net_version != NET_VERSION )
  {
    sendReply( netRoom::ERROR_WRONG_NET_VERSION ) ;
    closeWhenDone () ;
    return ;
  }

  netGuid game_guid ;
  msg.geta ( &game_guid, sizeof(game_guid) ) ;

  {
    netRoomServerInfo* s ;
    int num = 0 ;
    for ( s = servers -> get ( 0 ) ; s != NULL ; s = servers -> getNext () )
    {
      if ( s -> game_guid == game_guid )
      {
        num ++ ;
      }
    }

    netMessage msg( netRoom::INIT_SERVER_LIST, 0, 0 );
    msg.puti ( num ) ;
    sendMessage( msg );

    for ( s = servers -> get ( 0 ) ; s != NULL ; s = servers -> getNext () )
    {
      if ( s -> game_guid == game_guid )
      {
        netMessage msg( netRoom::UPDATE_SERVER_LIST, 0, 0 );
        s -> put ( msg ) ;

        sendMessage( msg );
      }
    }
  }

  sendReply( netRoom::ERROR_NONE ) ;
  closeWhenDone () ;
}


void netRoomMasterChannel::processUpdate ( const netMessage &msg )
{
  int net_version = msg.geti () ;

  if ( net_version != NET_VERSION )
  {
    sendReply( netRoom::ERROR_WRONG_NET_VERSION ) ;
    closeWhenDone () ;
    return ;
  }

  netRoomServerInfo server ;
  server.get ( msg ) ;

  //force correct host address
  netCopyName ( server.host, host ) ;

  servers -> add ( &server ) ;

  printf ( "Server %s added (%s,%d)\n", server.name, server.host, server.port ) ;

  sendReply( netRoom::ERROR_NONE ) ;
  closeWhenDone () ;
}


void netRoomMasterChannel::processMessage ( const netMessage& msg )
{
  switch ( msg.getType() )
  {
  case netRoom::REQUEST_SERVER_LIST:
    processRequest( msg ) ;
    break;
  case netRoom::UPDATE_SERVER_LIST:
    processUpdate( msg ) ;
    break;
  default:
    closeWhenDone () ;
    break;
  }
}
