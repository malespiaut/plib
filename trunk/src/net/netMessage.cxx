#include "netMessage.h"


void
netMessageChannel::handleBufferRead (netBuffer& in_buffer)
{
  int n = in_buffer.getLength () ;
  while ( n >= 2 )
  {
    u16 msg_len = *( (u16*)in_buffer.getData() ) ;
    if ( n >= msg_len )
    {
      //we have a complete message; handle it
      netMessage msg(in_buffer.getData(),msg_len);
      in_buffer.remove(0,msg_len);
      handleMessage ( msg );

      //fprintf ( stderr, "netMessageChannel: %d read\n", msg_len ) ;
      n -= msg_len ;
    }
    else
    {
      //fprintf ( stderr, "netMessageChannel: %d waiting\n", n ) ;
      break ;
    }
  }
}
