#include "netRoom.h"


class netRoomMasterChannel : public netMessageChannel
{
  netRoomMasterServer* server ;
  netRoom* aroom ;

  void sendReply( int error )
  {
    netMessage msg( netRoom::SYSMSG_ERROR_REPLY, 0, 0 );
    msg.puti ( error ) ;
    sendMessage( msg );
  }

  void processBrowse ( const netMessage &msg ) ;
  void processAdvertise ( const netMessage &msg ) ;

  virtual void handleClose (void) ;

public:

  netRoomMasterChannel ( int handle, netRoomMasterServer* _server )
  {
    setHandle ( handle ) ;
    server = _server ;
    aroom = 0 ;
  }

  virtual void handleMessage ( const netMessage& msg ) ;
} ;


void netRoomMasterChannel::processBrowse ( const netMessage &msg )
{
  int version = msg.geti () ;

  if ( server -> version != version )
  {
    sendReply( netRoom::ERROR_WRONG_VERSION ) ;
    closeWhenDone () ;
    return ;
  }

  netGuid guid ;
  msg.geta ( &guid, sizeof(guid) ) ;
  
  if ( server -> guid != guid )
  {
    sendReply( netRoom::ERROR_WRONG_GUID ) ;
    closeWhenDone () ;
    return ;
  }

  {
    netMessage msg( netRoom::SYSMSG_REMOVE_ALL_ROOMS, 0, 0 );
    sendMessage( msg );

    for ( int id = 1; id <= server -> rooms . getNum (); id ++ )
    {
      netRoom* room = server -> rooms . get ( id ) ;
      if ( room )
      {
        netMessage msg( netRoom::SYSMSG_UPDATE_ROOM, 0, 0 );
        room -> put ( msg ) ;

        sendMessage( msg );
      }
    }
  }

  sendReply( netRoom::ERROR_NONE ) ;
  closeWhenDone () ;
}


void netRoomMasterChannel::processAdvertise ( const netMessage &msg )
{
  int version = msg.geti () ;

  if ( server -> version != version )
  {
    sendReply( netRoom::ERROR_WRONG_VERSION ) ;
    closeWhenDone () ;
    return ;
  }

  netGuid guid ;
  msg.geta ( &guid, sizeof(guid) ) ;
  
  if ( server -> guid != guid )
  {
    sendReply( netRoom::ERROR_WRONG_GUID ) ;
    closeWhenDone () ;
    return ;
  }

  netRoom room ;
  room.get ( msg ) ;

  netRoom* r = server -> rooms . findByAddr ( room.host, room.port ) ;
  if ( r == NULL )
  {
    r = server -> rooms . add ( new netRoom ) ;
    if ( r == NULL )
    {
      sendReply( netRoom::ERROR_TOO_MANY_ROOMS ) ;
      closeWhenDone () ;
      return ;
    }

    printf ( "add room \"%s\" (%s,%d)\n",
      room.name, room.host, room.port ) ;
  }
  else
  {
    printf ( "update room \"%s\" (%s,%d)\n",
      room.name, room.host, room.port ) ;
  }
  if ( r != NULL )
    r -> copy ( &room ) ;

  sendReply( netRoom::ERROR_NONE ) ;
  aroom = r ;
//  closeWhenDone () ;
}


void netRoomMasterChannel::handleClose (void)
{
  if ( aroom )
  {
    printf ( "remove room \"%s\" (%s,%d)\n",
      aroom->name, aroom->host, aroom->port ) ;
    server -> rooms . remove ( aroom -> getID () ) ;
    aroom = 0 ;
  }
  shouldDelete () ;
  netMessageChannel::handleClose () ;
}


void netRoomMasterChannel::handleMessage ( const netMessage& msg )
{
  switch ( msg.getType() )
  {
  case netRoom::SYSMSG_BROWSE_ROOMS:
    processBrowse( msg ) ;
    break;
  case netRoom::SYSMSG_ADVERTISE_ROOM:
    processAdvertise( msg ) ;
    break;
  default:
    closeWhenDone () ;
    break;
  }
}


void netRoomMasterServer::handleAccept (void)
{
  netAddress addr ;
  int handle = accept ( &addr ) ;
  new netRoomMasterChannel ( handle, this ) ;
}


void netRoomBrowser::handleMessage ( const netMessage& msg )
{
  switch ( msg.getType() )
  {
  case netRoom::SYSMSG_ERROR_REPLY:
    {
      last_error = msg.geti () ;
      break;
    }
  case netRoom::SYSMSG_REMOVE_ALL_ROOMS:
    {
      rooms -> removeAll () ;
    }
    break;
  case netRoom::SYSMSG_UPDATE_ROOM:
    {
      netRoom room ;
      room.get ( msg ) ;

      netRoom* r = rooms -> findByAddr ( room.host, room.port ) ;
      if ( r == NULL )
        r = rooms -> add ( handleNewRoom () ) ;
      if ( r != NULL )
        r -> copy ( &room ) ;
    }
    break;
  default:
    close () ;
    break;
  }
}


void netRoomBrowser::browse ( cchar* host, int port, netRoomList* _rooms )
{
  rooms = _rooms ;

  open ();
  connect ( host, port ) ;

  netMessage msg ( netRoom::SYSMSG_BROWSE_ROOMS, 0, 0 ) ;

  msg.puti ( version ) ;
  msg.puta ( &guid, sizeof(guid) ) ;

  sendMessage ( msg ) ;
}


void netRoomAdvertiser::advertise ( cchar* host, int port, const netRoom& room )
{
  open ();
  connect ( host, port ) ;
  update ( room ) ;
  //closeWhenDone () ;
}


void netRoomAdvertiser::update ( const netRoom& room )
{
  netMessage msg ( netRoom::SYSMSG_ADVERTISE_ROOM, 0, 0 ) ;

  msg.puti ( version ) ;
  msg.puta ( &guid, sizeof(guid) ) ;
  room.put ( msg ) ;

  sendMessage ( msg ) ;
}
