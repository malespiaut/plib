
#include "ssgAux.h"

ssgaParticleSystem::ssgaParticleSystem ( int num, int initial_num,
                                 float _create_rate, int _ttf,
                                 float sz, float bsphere_size,
                                 ssgaParticleCreateFunc _particle_create,
                                 ssgaParticleUpdateFunc _particle_update,
                                 ssgaParticleDeleteFunc _particle_delete ) :
         ssgVtxArray ( GL_QUADS, 
              new ssgVertexArray   ( num * 4, new sgVec3 [ num * 4 ] ), 
              new ssgNormalArray   ( num * 4, new sgVec3 [ num * 4 ] ), 
              new ssgTexCoordArray ( num * 4, new sgVec2 [ num * 4 ] ), 
              new ssgColourArray   ( num * 4, new sgVec4 [ num * 4 ] ), 
              new ssgIndexArray    ( num * 4, new short  [ num * 4 ] ) )
{
  turn_to_face = _ttf ;
  create_error = 0 ;
  create_rate = _create_rate ;
  particle_create = _particle_create ;
  particle_update = _particle_update ;
  particle_delete = _particle_delete ;

  size = sz ;

  num_particles = num ;
  num_verts     = num * 4 ;

  getBSphere () -> setRadius ( bsphere_size ) ;
  getBSphere () -> setCenter ( 0, 0, 0 ) ;

  particle = new ssgaParticle [ num ] ;

  int i ;

  for ( i = 0 ; i < num_verts ; i++ )
  {
    sgSetVec3  ( getNormal ( i ), 0, -1, 0 ) ;
    sgSetVec4  ( getColour ( i ), 1, 1, 1, 1 ) ;
    sgZeroVec3 ( getVertex ( i ) ) ;
    *(getIndex( i )) = i ;
  }

  for ( i = 0 ; i < num_particles ; i++ )
  {
    sgSetVec2 ( getTexCoord ( i*4+0 ), 0, 0 ) ;
    sgSetVec2 ( getTexCoord ( i*4+1 ), 1, 0 ) ;
    sgSetVec2 ( getTexCoord ( i*4+2 ), 1, 1 ) ;
    sgSetVec2 ( getTexCoord ( i*4+3 ), 0, 1 ) ;
  }

  if ( particle_create )
    for ( i = 0 ; i < initial_num ; i++ )
      (*particle_create) ( this, i, & particle [ i ] ) ;

  update ( 0.1f ) ;
}


void ssgaParticleSystem::draw_geometry ()
{
  sgVec3 nxny, xxny, xxyy, nxyy ;

  float sz = size / 2.0f ;

  if ( turn_to_face )
  {
    sgMat4 mat ;

    glGetFloatv ( GL_MODELVIEW_MATRIX, (float *) mat ) ;

    sgVec3 xx, yy ;

    sgSetVec3 ( xx, mat[0][0] * sz, mat[1][0] * sz, mat[2][0] * sz ) ;
    sgSetVec3 ( yy, mat[0][1] * sz, mat[1][1] * sz, mat[2][1] * sz ) ;

    sgSetVec3 ( nxny, -xx[0]-yy[0], -xx[1]-yy[1], -xx[2]-yy[2] ) ;
    sgSetVec3 ( nxyy, -xx[0]+yy[0], -xx[1]+yy[1], -xx[2]+yy[2] ) ;
    sgSetVec3 ( xxny,  xx[0]-yy[0],  xx[1]-yy[1],  xx[2]-yy[2] ) ;
    sgSetVec3 ( xxyy,  xx[0]+yy[0],  xx[1]+yy[1],  xx[2]+yy[2] ) ;
  }
  else
  {
    sgSetVec3 ( xxny ,  sz, 0, -sz ) ;
    sgSetVec3 ( nxny , -sz, 0, -sz ) ;
    sgSetVec3 ( nxyy , -sz, 0,  sz ) ;
    sgSetVec3 ( xxyy,   sz, 0,  sz ) ;
  }

  int j = 0 ;

  for ( int i = 0 ; i < num_particles ; i++, j += 4 )
  {
    sgCopyVec4 ( getColour ( j + 0 ), particle[i].col ) ;
    sgCopyVec4 ( getColour ( j + 1 ), particle[i].col ) ;
    sgCopyVec4 ( getColour ( j + 2 ), particle[i].col ) ;
    sgCopyVec4 ( getColour ( j + 3 ), particle[i].col ) ;

    sgAddScaledVec3 ( getVertex ( j + 0 ), particle[i].pos,
                                     nxny, particle[i].size ) ;
    sgAddScaledVec3 ( getVertex ( j + 1 ), particle[i].pos,
                                     xxny, particle[i].size ) ;
    sgAddScaledVec3 ( getVertex ( j + 2 ), particle[i].pos,
                                     xxyy, particle[i].size ) ;
    sgAddScaledVec3 ( getVertex ( j + 3 ), particle[i].pos,
                                     nxyy, particle[i].size ) ;
  }

  glDisable   ( GL_CULL_FACE ) ;
  glDepthMask ( 0 ) ;
 
  ssgVtxArray::draw_geometry () ;

  glDepthMask ( 1 ) ;
  glEnable ( GL_CULL_FACE ) ;
}


ssgaParticleSystem::~ssgaParticleSystem ()
{
  if ( particle_delete )
    for ( int i = 0 ; i < num_particles ; i++ )
      if ( particle [ i ] . time_to_live >= 0.0 )
  	(*particle_delete) ( this, i, & particle [ i ] ) ;

  delete [] particle ;
}


void ssgaParticleSystem::update ( float t )
{
  int i ;

  create_error += create_rate * t ;

  num_active = 0 ;

  /* Update all the particles */

  for ( i = 0 ; i < num_particles ; i++ )
    particle [ i ] . update ( t ) ;

  /* Call the update routine for all the particles */

  if ( particle_update )
    for ( i = 0 ; i < num_particles ; i++ )
      (*particle_update) ( t, this, i, & particle [ i ] ) ;

  /* Check for death of particles */

  for ( i = 0 ; i < num_particles ; i++ )
    if ( particle [ i ] . time_to_live < 0.0 )
    {
      if ( particle_delete )
	(*particle_delete) ( this, i, & particle [ i ] ) ;

      particle [ i ] . pos [ 2 ] = -1000000.0f ;

      if ( create_error >= 1.0f && particle_create )
      {
	(*particle_create) ( this, i, & particle [ i ] ) ;
	create_error -= 1.0f ;
      }
    }
    else
      num_active++ ;
}


