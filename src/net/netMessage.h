/****
* NAME
*   netMessage - message buffer and channel classes
*
* DESCRIPTION
*   messages are a binary format for sending buffers over a channel.
*   message headers contain a type field and length.
*
* AUTHOR
*   Dave McClurg <dpm@efn.org>
*
* CREATION DATE
*   Dec-2000
****/

#ifndef __NET_MESSAGE__
#define __NET_MESSAGE__


#include "netChat.h"


/*
 * These SWAP macros are inserted into the message class
 * methods to deal with host machine endianness.
 *
 * We'll transmit everything over the wire as <little endian>.
 * On <big endian> machines such as the MAC, we'll SWAP bytes
 * around.  If you run on a PC, these macros will compile out.
 */

inline u16 _NET_SWAP16(u16 D)
   { return((D<<8)|(D>>8)); }

inline u32 _NET_SWAP32(u32 D)
   { return((D<<24)|((D<<8)&0x00FF0000)|((D>>8)&0x0000FF00)|(D>>24)); }

/* macintosh is big-endian or high to low */
#ifdef _MACOS
#  define NET_SWAP16(D) _NET_SWAP16(D)
#  define NET_SWAP32(D) _NET_SWAP32(D)
#else
#  define NET_SWAP16(D) (D)
#  define NET_SWAP32(D) (D)
#endif


class netGuid //Globally Unique IDentifier
{
public:
  u8 data [ 16 ] ;

  netGuid () {}

  netGuid ( u32 l, u16 w1, u16 w2,
    u8 b1, u8 b2, u8 b3, u8 b4, u8 b5, u8 b6, u8 b7, u8 b8 )
  {
    //store in network format (big-endian)
    data[0] = u8(l>>24);
    data[1] = u8(l>>16);
    data[2] = u8(l>>8);
    data[3] = u8(l);
    data[4] = u8(w1>>8);
    data[5] = u8(w1);
    data[6] = u8(w2>>8);
    data[7] = u8(w2);
    data[8] = b1;
    data[9] = b2;
    data[10] = b3;
    data[11] = b4;
    data[12] = b5;
    data[13] = b6;
    data[14] = b7;
    data[15] = b8;
  }

  bool operator ==( const netGuid& guid ) const
  { 
    return memcmp ( data, guid.data, sizeof(data) ) == 0 ;
  }
  bool operator !=( const netGuid& guid ) const
  { 
    return memcmp ( data, guid.data, sizeof(data) ) != 0 ;
  }

  void push ( netChat* channel ) const
  {
    // eg. {2F5B6220-DB7D-11d4-8748-00D0B796C186}
    channel -> push ( "{" ) ;
    for ( int i=0; i<16; i++ )
    {
      if ( i==4 || i==6 || i==8 || i==10 )
        channel -> push ( "-" ) ;
      channel -> push ( netFormat ( "%02x", data[i] ) ) ;
    }
    channel -> push ( "}\r\n" ) ;
  }
} ;


class netMessage : public netBuffer
{
  int pos ;

  void seek ( int new_pos ) const
  {
    if ( new_pos < 0 )
      new_pos = 0 ;
    else if ( new_pos > length )
      new_pos = length ;

    //logical const-ness
    ((netMessage*)this) -> pos = new_pos ;
  }
  void skip ( int off ) const
  {
    seek(pos+off);
  }

public:

  netMessage ( const char* s, int n ) : netBuffer(n)
  {
    assert ( n >= 4 && n < 256 ) ;
    append(s,n);
    pos = 4 ;
  }

  netMessage ( int type, int to_id, int from_id ) : netBuffer(250)
  {
    putch ( 0 ) ;  //msg_len
    putch ( type ) ;
    putch ( to_id ) ;
    putch ( from_id ) ;
  }

  int getType () const { return ( (u8*)data )[ 1 ] ; }
  int getToID () const { return ( (u8*)data )[ 2 ] ; }
  int getFromID () const { return ( (u8*)data )[ 3 ] ; }

  void geta ( void* a, int n ) const
  {
    assert (pos>=0 && pos<length && (pos+n)<=length) ;
    //if (pos>=0 && pos<length && (pos+n)<=length)
    {
      memcpy(a,&data[pos],n) ;
      seek(pos+n);
    }
  }
  void puta ( const void* a, int n )
  {
    append((const char*)a,n);
    pos = length;

    //update msg_len
    assert(length<256);
    ((u8*)data)[0] = u8(length);
  }

  int getch () const
  {
    u8 temp ;
    geta(&temp,sizeof(temp)) ;
    return temp ;
  }
  void putch ( int c )
  {
    u8 temp = c ;
    puta(&temp,sizeof(temp)) ;
  }

  bool getb () const
  {
    u8 temp ;
    geta(&temp,sizeof(temp)) ;
    return temp != 0 ;
  }
  void putb ( bool b )
  {
    u8 temp = b? 1: 0 ;
    puta(&temp,sizeof(temp)) ;
  }

  int getw () const
  {
    u16 temp ;
    geta ( &temp, sizeof(temp) ) ;
    return int ( NET_SWAP16 ( temp ) ) ;
  }
  void putw ( int i )
  {
    u16 temp = NET_SWAP16 ( u16(i) ) ;
    puta ( &temp, sizeof(temp) ) ;
  }

  int geti () const
  {
    u32 temp ;
    geta ( &temp, sizeof(temp) ) ;
    return int ( NET_SWAP32 ( temp ) ) ;
  }
  void puti ( int i )
  {
    u32 temp = NET_SWAP32 ( u32(i) ) ;
    puta ( &temp, sizeof(temp) ) ;
  }

  void gets ( char* s, int n ) const
  {
    char* src = &data[pos];
    char* dst = s;
    while (pos<length)
    {
      char ch = *src++;
      if ((dst-s)<(n-1))
        *dst++ = ch ;
      ((netMessage*)this)->pos++;
      if (ch==0)
        break;
    }
    *dst = 0 ;
  }
  void puts ( const char* s )
  {
    puta(s,strlen(s)+1);
  }

  void print ( FILE *fd = stderr ) const
  {
    fprintf ( fd, "netMessage: %p, length=%d\n", this, length ) ;
    fprintf ( fd, "  header (type,to,from) = (%d,%d,%d)\n",
      getType(), getToID(), getFromID() ) ;
    fprintf ( fd, "  data = " ) ;
    for ( int i=0; i<length; i++ )
      fprintf ( fd, "%02x ", data[i] ) ;
    fprintf ( fd, "\n" ) ;
  }
};


class netMessageChannel : public netBufferChannel
{
  void (*processCB)(const netMessage& msg) ;

  virtual void handleBufferRead (netBuffer& buffer) ;

public:

  netMessageChannel ()
  {
    processCB = 0 ;
  }

  bool sendMessage ( const netMessage& msg )
  {
    return bufferSend ( msg.getData(), msg.getLength() ) ;
  }

  virtual void processMessage ( const netMessage& msg )
  {
    if ( processCB ) processCB ( msg ) ;
  }

  void setCallback ( void (*callback)(const netMessage& msg) )
  {
    processCB = callback ;
  }
};


#endif //__NET_MESSAGE__