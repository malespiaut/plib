

#include "p3d.h"

static int size[3] ;


Puzzle::Puzzle ()
{ 
  size[0] = 4 ;
  size[1] = 4 ;
  size[2] = 4 ;
  
  sgSetCoord ( &pos, 0, 0, 0, 0, 0, 0 ) ;

  ground   = makeGround       () ;
  selector = makeBlueWireCube () ;

  spin = new ssgTransform ( &pos ) ;
  spin -> addKid ( ground ) ;
  spin -> addKid ( selector ) ;

  sel [ 0 ] = 0 ;
  sel [ 1 ] = 0 ;
  sel [ 2 ] = 0 ;

  for ( int x = 0 ; x < size [ 0 ] ; x++ ) 
    for ( int y = 0 ; y < size [ 1 ] ; y++ ) 
      for ( int z = 0 ; z < size [ 2 ] ; z++ )
      {
        cell[x][y][z] = new Cell ( x, y, z, size[0] ) ;
        spin -> addKid ( cell[x][y][z]->getSSG() ) ;
      }

  reset () ;
}


void Puzzle::reset ()
{
  cursor ( 0, 0, 0 ) ;
  sgSetCoord ( &pos, 0, 0, 0, 0, 0, 0 ) ;

  for ( int x = 0 ; x < size [ 0 ] ; x++ ) 
    for ( int y = 0 ; y < size [ 1 ] ; y++ ) 
      for ( int z = 0 ; z < size [ 2 ] ; z++ )
        cell[x][y][z]->set(WIRE_CELL) ; 

  setGameState ( STILL_PLAYING ) ;
  game_init () ;
}


void Puzzle::update ()
{
  static int flasher = 0 ;
  sgCoord spos ;
  sgSetVec3 ( spos.hpr, 0, 0, 0 ) ;

  flasher++ ;
  if ( flasher > 5 )
    sgSetVec3 ( spos.xyz, (float)sel[0] - (float)size[0]/2.0f + 0.51f,
                          (float)sel[1] - (float)size[1]/2.0f + 0.51f,
                          (float)sel[2] - (float)size[2]/2.0f + 0.51f ) ;
  else
    sgSetVec3 ( spos.xyz, 1000000, 1000000, 1000000 ) ;

  if ( flasher > 60 )
    flasher = 0 ;

  selector -> setTransform ( & spos ) ;
}


Puzzle::~Puzzle ()
{
  delete spin ;
}


