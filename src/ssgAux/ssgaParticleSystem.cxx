
#include "ssgAux.h"

void ssgaParticle::init ()
{
  sgSetVec4 ( col, 1, 1, 1, 1 ) ;
  sgZeroVec3 ( pos ) ;
  sgZeroVec3 ( vel ) ;
  sgZeroVec3 ( acc ) ;
  time_to_live = 0 ;
}

void ssgaParticle::update ( float dt )
{
  sgAddScaledVec3 ( vel, acc, dt ) ;
  sgAddScaledVec3 ( pos, vel, dt ) ;
  time_to_live -= dt ;
}


ssgaParticleSystem::ssgaParticleSystem ( int num, int initial_num,
                                 float _create_rate, int _ttf,
                                 float sz, float bsphere_size,
                                 ssgaParticleFunc _particle_create,
                                 ssgaParticleFunc _particle_update ) :
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

  size = sz ;

  num_particles = num ;
  num_verts     = num * 4 ;

  particle = new ssgaParticle [ num ] ;

  for ( int i = 0 ; i < num_verts ; i++ )
  {
    sgSetVec3 ( getNormal ( i ), 0, -1, 0 ) ;
    sgSetVec4 ( getColour ( i ), 1, 1, 1, 1 ) ;
    *(getIndex( i )) = i ;
  }

  for ( int i = 0 ; i < num_particles ; i++ )
  {
    particle [ i ] . init () ;

    sgSetVec3 ( getVertex ( i*4+0 ),  0  , 0,  0   ) ;
    sgSetVec3 ( getVertex ( i*4+1 ),  0  , 0, size ) ;
    sgSetVec3 ( getVertex ( i*4+2 ), size, 0, size ) ;
    sgSetVec3 ( getVertex ( i*4+3 ), size, 0,  0   ) ;
    sgSetVec2 ( getTexCoord ( i*4+0 ), 0, 0 ) ;
    sgSetVec2 ( getTexCoord ( i*4+1 ), 1, 0 ) ;
    sgSetVec2 ( getTexCoord ( i*4+2 ), 1, 1 ) ;
    sgSetVec2 ( getTexCoord ( i*4+3 ), 0, 1 ) ;

    if ( i < initial_num && particle_create )
      (*particle_create) ( & particle [ i ] ) ;
  }

  getBSphere () -> setRadius ( bsphere_size ) ;
  getBSphere () -> setCenter ( 0, 0, 0 ) ;
}


void ssgaParticleSystem::draw_geometry ()
{
  sgVec3 xx, xxyy, yy ;

  if ( turn_to_face )
  {
    sgMat4 mat ;

    glGetFloatv ( GL_MODELVIEW_MATRIX, (float *) mat ) ;

    sgSetVec3 ( xx, mat[0][0] * size, mat[1][0] * size, mat[2][0] * size ) ;
    sgSetVec3 ( yy, mat[0][1] * size, mat[1][1] * size, mat[2][1] * size ) ;
    sgAddVec3 ( xxyy, xx, yy ) ;
  }
  else
  {
    sgSetVec3 (  xx , size, 0,  0   ) ;
    sgSetVec3 (  yy ,  0  , 0, size ) ;
    sgSetVec3 ( xxyy, size, 0, size ) ;
  }

  int j = 0 ;

  for ( int i = 0 ; i < num_particles ; i++, j += 4 )
  {
    if ( particle [ i ] . time_to_live < 0.0 )
    {
      getVertex ( j + 0 ) [ 2 ] = -1000000.0f ;
      getVertex ( j + 1 ) [ 2 ] = -1000000.0f ;
      getVertex ( j + 2 ) [ 2 ] = -1000000.0f ;
      getVertex ( j + 3 ) [ 2 ] = -1000000.0f ;
    }
    else  
    {
      sgCopyVec4  ( getColour ( j + 0 ), particle[i].col ) ;
      sgCopyVec4  ( getColour ( j + 1 ), particle[i].col ) ;
      sgCopyVec4  ( getColour ( j + 2 ), particle[i].col ) ;
      sgCopyVec4  ( getColour ( j + 3 ), particle[i].col ) ;

      sgCopyVec3 ( getVertex ( j + 0 ), particle[i].pos ) ;
      sgAddVec3  ( getVertex ( j + 1 ), particle[i].pos, xx ) ;
      sgAddVec3  ( getVertex ( j + 2 ), particle[i].pos, xxyy ) ;
      sgAddVec3  ( getVertex ( j + 3 ), particle[i].pos, yy ) ;
    }
  }

  glDisable   ( GL_CULL_FACE ) ;
  glDepthMask ( 0 ) ;
 
  ssgVtxArray::draw_geometry () ;

  glDepthMask ( 1 ) ;
  glEnable ( GL_CULL_FACE ) ;
}


void ssgaParticleSystem::update ( float t )
{
  create_error += create_rate * t ;

  for ( int i = 0 ; i < num_particles ; i++ )
  {
    particle [ i ] . update ( t ) ;

    if ( particle_update )
      (*particle_update) ( & particle [ i ] ) ;

    if ( particle [ i ] . time_to_live < 0.0 &&
         create_error >= 1.0f && particle_create )
    {
      (*particle_create) ( & particle [ i ] ) ;
      create_error -= 1.0f ;
    }
  }
}

