#include "netMessage.h"


void
netMessageChannel::handleBufferRead (netBuffer& in_buffer)
{
  int n = in_buffer.getLength () ;
  while ( n > 0 )
  {
    u8 msg_len = *( (u8*)in_buffer.getData() ) ;
    if ( n >= msg_len )
    {
      //we have a complete message; process it
      netMessage msg(in_buffer.getData(),msg_len);
      in_buffer.remove(0,msg_len);
      processMessage ( msg );

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
