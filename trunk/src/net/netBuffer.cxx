#include "netBuffer.h"

void
netBufferChannel::handleRead (void)
{
  int max_read = in_buffer.getMaxLength() - in_buffer.getLength() ;
  if (max_read)
  {
    char* data = in_buffer.getData() + in_buffer.getLength() ;
    int num_read = recv (data, max_read) ;
    if (num_read)
    {
      in_buffer.append (num_read) ;
      //fprintf ( stderr, "netBufferChannel: %d read\n", num_read ) ;
    }
  }
  if (in_buffer.getLength())
  {
    handleBufferRead (in_buffer);
  }
}

void
netBufferChannel::handleWrite (void)
{
  if (out_buffer.getLength())
  {
    if (isConnected())
    {
      int length = out_buffer.getLength() ;
      if (length>512)
        length=512;
      int num_sent = netChannel::send (
        out_buffer.getData(), length);
      if (num_sent)
      {
        out_buffer.remove (0, num_sent);
        //fprintf ( stderr, "netBufferChannel: %d sent\n", num_sent ) ;
      }
    }
  }
  else if (should_close)
  {
    close();
  }
}
