// This is an example of a UDP server that accepts messages coming from a
// client.
//
// UDP is a "fire and forget" protocal so the client and server can be
// started up in any order.  The client doesn't care if the server
// receives the messages and the server doesn't care if the client is
// sending messages, so either side can be killed and restarted at any
// time.

#include <stdio.h>

#include <plib/netSocket.h>

int main( int argc, char **argv ) {
    int port = 5501;
    char host[256] = "";	// accept messages from anyone

    // Must call this before any other net stuff
    netInit( &argc,argv );

    netSocket s;

    if ( ! s.open( false ) ) {	// open a UDP socket
	printf("error opening socket\n");
	return -1;
    }

    s.setBlocking( false );

    if ( s.bind( host, port ) == -1 ) {
	printf("error binding to port %d\n", port);
	return -1;
    }

    char msg[256];
    int maxlen = 256;
    int len;

    while ( true ) {
	if ( (len = s.recv(msg, maxlen, 0)) >= 0 ) {
	    msg[len] = '\0';
	    printf("msg received = %s\n", msg);
	    
	}
    }
	
    return 0;
}