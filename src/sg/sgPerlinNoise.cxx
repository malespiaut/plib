

/*
  sgPerlinNoise
 
  This is a class to implement coherent noise over 1, 2, or 3
  dimensions.  The implementation is based on the Ken Perlin's
  noise function and borrows heavily from Matt Zucker's implementation.
*/


#include <stdio.h>
#include <time.h>
#include <math.h>
#include "sg.h"

static unsigned int *permTable = NULL ;


static void initPermTable ()
{
  int i ;

  if ( permTable != NULL )
    return ;

  permTable = new unsigned int [ SG_PERLIN_NOISE_WRAP_INDEX * 2 + 2 ] ;

  for ( i = 0 ; i < SG_PERLIN_NOISE_WRAP_INDEX ; i++ )
    permTable[i] = i ;

  /* Shuffle permutation table up to SG_PERLIN_NOISE_WRAP_INDEX */

  for ( i = 0 ; i < SG_PERLIN_NOISE_WRAP_INDEX ; i++ )
  {
    int j = rand () & SG_PERLIN_NOISE_MOD_MASK ;

    int temp = permTable [ i ] ;
    permTable [ i ] = permTable [ j ] ;
    permTable [ j ] = temp ;
  }

  /*
    Add the rest of the table entries in, duplicating 
    indices and entries so that they can effectively be indexed
    by unsigned chars.  I think.  Ask Perlin what this is really doing.
  */
  
  for ( i = 0 ; i < SG_PERLIN_NOISE_WRAP_INDEX + 2 ; i++ )
    permTable [ SG_PERLIN_NOISE_WRAP_INDEX + i ] = permTable [ i ] ;
}






/*
  S curve is (3x^2 - 2x^3) because it's quick to calculate
  though -cos(x * PI) * 0.5 + 0.5 would work too
*/

inline SGfloat easeCurve ( SGfloat t ) { return t * t * (3.0 - 2.0 * t) ; } 

inline SGfloat dot2 ( SGfloat rx,  SGfloat ry, sgVec2 q )
{
  return rx * q[0] + ry * q[1] ;
}

inline SGfloat dot3 ( SGfloat rx,  SGfloat ry,  SGfloat rz, sgVec3 q )
{
  return rx * q[0] + ry * q[1] + rz * q[2] ;
}


inline SGfloat randFloat ()
{ 
  return (SGfloat)(( rand() % ( SG_PERLIN_NOISE_WRAP_INDEX +
                              SG_PERLIN_NOISE_WRAP_INDEX ) ) - 
                              SG_PERLIN_NOISE_WRAP_INDEX ) /
                              SG_PERLIN_NOISE_WRAP_INDEX ;
}


inline void setupValues ( int axis, int *g0, int *g1,
                          SGfloat *d0, SGfloat *d1, SGfloat *pos )
{
  SGfloat t = pos[axis] + SG_PERLIN_NOISE_LARGE_PWR2 ;
  int  it = (int) t ;

  *g0 = it & SG_PERLIN_NOISE_MOD_MASK ;
  *g1 = (*g0 + 1) & SG_PERLIN_NOISE_MOD_MASK ;
  *d0 = t - it ;
  *d1 = *d0 - 1.0 ;
}



SGfloat sgPerlinNoise_1D::getNoise ( SGfloat pos )
{
  int   gridL, gridR ;
  SGfloat distL, distR ;

  /*
    Find out neighboring grid points to pos and
    signed distances from pos to them.
  */ 

  setupValues ( 0, &gridL, &gridR,
                   &distL , &distR , &pos ) ;

  return sgLerp ( distL * gradTable [ permTable [ gridL ] ],
                  distR * gradTable [ permTable [ gridR ] ],
                  easeCurve ( distL ) ) ;
}



SGfloat sgPerlinNoise_2D::getNoise ( sgVec2 pos )
{
  int   gridL, gridR,
        gridD, gridU ;

  SGfloat distL, distR,
        distD, distU ;

  /*
    Find out neighboring grid points to pos and
    signed distances from pos to them.
  */

  setupValues ( 0, &gridL, &gridR, &distL, &distR, pos ) ;
  setupValues ( 1, &gridD, &gridU, &distD, &distU, pos ) ;

  /*
    Generate some temporary indexes associated with the left
    and right grid values
  */

  int indexL = permTable [ gridL ] ;
  int indexR = permTable [ gridR ] ;

  /* Generate indexes in the permutation table for all 4 corners */

  int indexLD = permTable [ indexL + gridD ] ;
  int indexRD = permTable [ indexR + gridD ] ;
  int indexLU = permTable [ indexL + gridU ] ;
  int indexRU = permTable [ indexR + gridU ] ;

  /* Get the s curves at the proper values */

  SGfloat sX = easeCurve ( distL ) ;
  SGfloat sY = easeCurve ( distD ) ;

  return sgLerp ( 
           sgLerp (
             dot2 ( distL, distD, gradTable [ indexLD ] ),
             dot2 ( distR, distD, gradTable [ indexRD ] ), sX ),
           sgLerp (
             dot2 ( distL, distU, gradTable [ indexLU ] ),
             dot2 ( distR, distU, gradTable [ indexRU ] ), sX ),
           sY ) ;
}


SGfloat sgPerlinNoise_3D::getNoise ( sgVec3 pos )
{
  int gridL, gridR,
      gridD, gridU,
      gridB, gridF ;

  SGfloat distL, distR,
        distD, distU,
        distB, distF ;

  /*
    Find out neighboring grid points to pos and signed
    distances from pos to them.
  */

  setupValues ( 0, &gridL, &gridR, &distL, &distR, pos ) ;
  setupValues ( 1, &gridD, &gridU, &distD, &distU, pos ) ;
  setupValues ( 2, &gridB, &gridF, &distB, &distF, pos ) ;
  
  int indexL = permTable [ gridL ] ;
  int indexR = permTable [ gridR ] ;

  int indexLD = permTable [ indexL + gridD ] ;
  int indexRD = permTable [ indexR + gridD ] ;
  int indexLU = permTable [ indexL + gridU ] ;
  int indexRU = permTable [ indexR + gridU ] ;

  SGfloat sX = easeCurve ( distL ) ;
  SGfloat sY = easeCurve ( distD ) ;
  SGfloat sZ = easeCurve ( distB ) ;

  return sgLerp (
      sgLerp (
	sgLerp (
	  dot3 ( distL, distD, distB, gradTable [ indexLD + gridB ] ),
	  dot3 ( distR, distD, distB, gradTable [ indexRD + gridB ] ), sX ),
	sgLerp (
	  dot3 ( distL, distU, distB, gradTable [ indexLU + gridB ] ),
	  dot3 ( distR, distU, distB, gradTable [ indexRU + gridB ] ), sX ),
	sY ),
      sgLerp (
	sgLerp (
	  dot3 ( distL, distD, distF, gradTable [ indexLD + gridF ] ),
	  dot3 ( distR, distD, distF, gradTable [ indexRD + gridF ] ), sX ),
	sgLerp (
	  dot3 ( distL, distU, distF, gradTable [ indexLU + gridF ] ),
	  dot3 ( distR, distU, distF, gradTable [ indexRU + gridF ] ), sX ),
	  sY ),
	sZ ) ;
}




sgPerlinNoise_1D::sgPerlinNoise_1D ()
{
  regenerate () ;
}


void sgPerlinNoise_1D::regenerate ()
{
  int i ;

  for ( i = 0 ; i < SG_PERLIN_NOISE_WRAP_INDEX ; i++ )
    gradTable [ i ] = randFloat () ;

  for ( i = 0 ; i < SG_PERLIN_NOISE_WRAP_INDEX + 2 ; i++ )
    gradTable [ SG_PERLIN_NOISE_WRAP_INDEX + i ] = gradTable [ i ]    ;

  initPermTable () ;
}



sgPerlinNoise_2D::sgPerlinNoise_2D ()
{
  regenerate () ;
}


void sgPerlinNoise_2D::regenerate ()
{
  int i ;

  for ( i = 0 ; i < SG_PERLIN_NOISE_WRAP_INDEX ; i++ )
  {
    sgSetVec2 ( gradTable [ i ], randFloat(), randFloat() ) ;
    sgNormalizeVec2 ( gradTable [ i ] ) ;
  }

  for ( i = 0 ; i < SG_PERLIN_NOISE_WRAP_INDEX + 2 ; i++ )
  {
    gradTable [ SG_PERLIN_NOISE_WRAP_INDEX + i ][ 0 ] = gradTable [ i ][ 0 ] ; 
    gradTable [ SG_PERLIN_NOISE_WRAP_INDEX + i ][ 1 ] = gradTable [ i ][ 1 ] ; 
  }

  initPermTable () ;
}



sgPerlinNoise_3D::sgPerlinNoise_3D ()
{
  regenerate () ;
}


void sgPerlinNoise_3D::regenerate ()
{
  int i ;

  for ( i = 0 ; i < SG_PERLIN_NOISE_WRAP_INDEX ; i++ )
  {
    sgSetVec3 ( gradTable [ i ], randFloat(), randFloat(), randFloat() ) ;
    sgNormalizeVec3 ( gradTable [ i ] ) ;
  }

  for ( i = 0 ; i < SG_PERLIN_NOISE_WRAP_INDEX + 2 ; i++ )
  {
    gradTable [ SG_PERLIN_NOISE_WRAP_INDEX + i ][ 0 ] = gradTable [ i ][ 0 ] ;
    gradTable [ SG_PERLIN_NOISE_WRAP_INDEX + i ][ 1 ] = gradTable [ i ][ 1 ] ;
    gradTable [ SG_PERLIN_NOISE_WRAP_INDEX + i ][ 2 ] = gradTable [ i ][ 2 ] ;
  }

  initPermTable () ;
}


