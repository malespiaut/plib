
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <math.h>
#include <plib/sg.h>



void printMat4 ( char *s, sgMat4 m )
{
  for ( int i = 0 ; i < 4 ; i++ )
  {
    printf ( "%s: ", s ) ;

    for ( int j = 0 ; j < 4 ; j++ )
      printf ( "%1.4f ", m[i][j] ) ;

    printf ( "\n" ) ;
  }
  printf ( "\n" ) ;
}


int main ( int, char ** )
{
/*
  sgVec3 hpr = { 0, 0, 123.456 } ;
  sgVec3 xyz ;
*/
  sgVec3 hpr ;
  sgSetVec3 ( hpr, 0.0, 0.0, 123.0 ) ;


  sgMat4 m ;
  sgQuat q ;

  sgMakeRotMat4 ( m, hpr[0], hpr[1], hpr[2] ) ;
  printMat4 ( "Mat4:", m ) ;

  sgMakeQuat ( &q, hpr ) ;
  sgMakeRotMat4 ( m, &q ) ;
  printMat4 ( "Quat:", m ) ;

  return 0 ;
}

