//
// TRI ( AC3D triangle file ) import for SSG/PLIB
// Written by Dave McClurg (dpm@efn.org) in March-2000
//

#include "ssgLocal.h"

#define MAX_TRI 100000

struct triData
{
  sgVec3 v[3] ;
  int color ;
} ;

/******************************************************************************/

ssgEntity *ssgLoadTRI ( const char *fname, ssgHookFunc hookfunc )

/******************************************************************************/

/*
  Purpose:
   
    reads an AC3D triangle file.

  Example:

    Each line contains 9 floating point values and a 1 hex value for color.
    the 9 floating point values represent 3 vertices of a triangle
    the color format is 0xRRGGBB (eg 0xffffff is white)

    0.0 0.0 0.0 1.0 0.0 0.0 1.0 1.0 0.0 0xffffff

*/
{
  //open the file
  char filename [ 1024 ] ;

  if ( fname [ 0 ] != '/' &&
       _ssgModelPath != NULL &&
       _ssgModelPath [ 0 ] != '\0' )
  {
    strcpy ( filename, _ssgModelPath ) ;
    strcat ( filename, "/" ) ;
    strcat ( filename, fname ) ;
  }
  else
    strcpy ( filename, fname ) ;

  FILE *loader_fd = fopen ( filename, "ra" ) ;

  if ( loader_fd == NULL )
  {
    ulSetError ( UL_WARNING, "ssgLoadTRI: Failed to open '%s' for reading", filename ) ;
    return NULL ;
  }

  //read the data
  triData* tri = new triData [ MAX_TRI ] ;
  int num_tri = 0 ;

  char buffer [ 1024 ] ;

  while ( fgets ( buffer, 1024, loader_fd ) != NULL )
  {
    float coord [9] ;
    int color ;

    if ( sscanf ( buffer, "%e %e %e %e %e %e %e %e %e %d",
      &coord[0], &coord[1], &coord[2],
      &coord[3], &coord[4], &coord[5],
      &coord[6], &coord[7], &coord[8],
      &color ) != 10 )
    {
      ulSetError ( UL_WARNING, "ssgLoadTRI: Can't parse triangle: %s", buffer ) ;
    }
    else if ( num_tri < MAX_TRI )
    {
      float* cp = coord;
      for ( int i=0; i<3; i++ )
      for ( int j=0; j<3; j++ )
        tri[ num_tri ].v[ i ][ j ] = *cp++;

      tri[ num_tri ].color = color ;
      num_tri ++ ;
    }
    else
    {
      break;
    }
  }

  fclose ( loader_fd ) ;

  ssgBranch* current_branch = NULL ;

  if ( num_tri )
  {
    ssgVertexArray* vlist = new ssgVertexArray ( num_tri * 3 ) ;
    for ( int i=0; i<num_tri; i++ )
    for ( int j=0; j<3; j++ )
      vlist -> add ( tri[ i ].v[ j ] ) ;
    ssgVtxTable *vtab = new ssgVtxTable ( GL_TRIANGLES, vlist, 0, 0, 0 );
    current_branch = new ssgTransform () ;
    current_branch -> addKid ( vtab ) ;
  }

  delete[] tri ;

  return current_branch ;
}
