
#include "ssgLocal.h"

#define MAX_HITS  100
static ssgHit hitlist [ MAX_HITS ] ;
static int next_hit = 0 ;
static ssgEntity *pathlist [ SSG_MAXPATH ] ;
static int next_path = 0 ;

int _ssgIsHotTest = FALSE ;

void _ssgPushPath ( ssgEntity *e )
{
  if ( next_path + 1 >= SSG_MAXPATH )
  {
    next_path++ ;  /* So pop works! */
    return ;
  }

  pathlist [ next_path++ ] = e ;
}


void _ssgPopPath ()
{
  next_path-- ;
}


void _ssgAddHit ( ssgLeaf *l, int trinum, sgMat4 mat, sgVec4 pl )
{
  if ( next_hit + 1 >= MAX_HITS )
    return ;

  ssgHit *h = & hitlist [ next_hit++ ] ;

  h -> leaf = l ;
  h -> triangle = trinum ;

  h -> num_entries = (next_path>=SSG_MAXPATH) ? SSG_MAXPATH : next_path ;
  memcpy ( h -> path, pathlist, h->num_entries * sizeof ( ssgEntity * ) ) ;
  
  sgCopyMat4 ( h -> matrix, mat ) ;
  sgCopyVec4 ( h -> plane, pl ) ;
}


int ssgIsect ( ssgRoot *root, sgSphere *s, sgMat4 mat, ssgHit **results )
{
  _ssgIsHotTest = FALSE ;
  next_hit  = 0 ;
  next_path = 0 ;
  root -> isect ( s, mat, TRUE ) ;
  *results = & hitlist [ 0 ] ;
  return next_hit ;
}


int ssgHOT ( ssgRoot *root, sgVec3 s, sgMat4 mat, ssgHit **results )
{
  _ssgIsHotTest = TRUE ;
  next_hit  = 0 ;
  next_path = 0 ;
  root -> hot ( s, mat, TRUE ) ;
  *results = & hitlist [ 0 ] ;
  return next_hit ;
}


int ssgLOS ( ssgRoot *root, sgVec3 s, sgMat4 mat, ssgHit **results )
{
  _ssgIsHotTest = FALSE ;
  next_hit  = 0 ;
  next_path = 0 ;
  root -> hot ( s, mat, TRUE ) ;
  *results = & hitlist [ 0 ] ;
  return next_hit ;
}


