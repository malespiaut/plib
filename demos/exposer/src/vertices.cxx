#include "exposer.h"


class Vertex
{
public:
 
  int     boneID ;
  sgVec3  rel_vx ;
  float  *vx     ;
 
  int getBoneID () { return boneID ; }
} ;
                                                                                

static int nextVertex = 0 ;

#define MAX_VERTICES  (64*1024)                                                 

static Vertex vertex [ MAX_VERTICES ] ;



static int getNumVertices () { return nextVertex ; }


static void addVertex ( sgVec3 v )
{
  vertex [ nextVertex ] . vx = v ;
  sgCopyVec3 ( vertex [ nextVertex ] . rel_vx, v ) ;

  nextVertex++ ;
}


static void initVertices ()
{
  nextVertex = 0 ;
}


static void walkVertices ( ssgBranch *root, sgMat4 mat )
{
  if ( root == NULL )
    return ;
 
  sgMat4 newmat ;

  if ( root -> isAKindOf ( ssgTypeTransform() ) )
  {
    sgMat4 tmp ;

    ((ssgTransform *)root)-> getTransform ( tmp ) ;
    sgMultMat4 ( newmat, mat, tmp ) ;

    sgMakeIdentMat4 ( tmp ) ;
    ((ssgTransform *)root)-> setTransform ( tmp ) ;
  }
  else
    sgCopyMat4 ( newmat, mat ) ;

  for ( int i = 0 ; i < root -> getNumKids () ; i++ )
  {
    ssgEntity *e =  root -> getKid ( i ) ;

    if ( e -> isAKindOf ( ssgTypeBranch() ) )
      walkVertices ( (ssgBranch *)e, newmat ) ;
    else
    {
      ssgLeaf *l = (ssgLeaf *) e ;

      for ( int ll = 0 ; ll < l->getNumVertices() ; ll++ )
      {
	float *v = l -> getVertex ( ll ) ;
        int vv = -1 ;

        for ( int j = 0 ; j < getNumVertices() ; j++ )
          if ( v == vertex [ j ] . vx )
          {
            vv = j ;
            break ;
          }

        if ( vv < 0 )
        {
          sgXformPnt3 ( v, mat ) ;
          addVertex ( v ) ;
        }
      }
    }
  }
}


void extractVertices ( ssgBranch *root )
{
  sgMat4 mat ;

  initVertices () ;

  /*
    Walk the model - making a list of all of the
    unique vertices - flattening the model's transforms
    as we go.
  */

  sgMakeIdentMat4 ( mat ) ;
  walkVertices ( root, mat ) ;

  /*
    Now find the nearest bone to each vertex and
    compute it's position relative to the root of
    that bone.
  */

  for ( int i = 0 ; i < getNumVertices() ; i++ )
  {
    float min = FLT_MAX ;

    for ( int j = 0 ; j < getNumBones() ; j++ )
    {
      sgLineSegment3 ls ;
      sgCopyVec3 ( ls.a, getBone(j)->orig_vx[0] ) ;
      sgCopyVec3 ( ls.b, getBone(j)->orig_vx[1] ) ;

      float d = sgDistSquaredToLineSegmentVec3 ( ls, vertex[i].vx ) ;

      if ( d < min )
      {
        min = d ;
        vertex[i].boneID = j ;
      }
    }

    sgSubVec3 ( vertex[i].rel_vx,
                vertex[i].vx,
                getBone(vertex[i].boneID)->orig_vx[0] ) ;
  }
}


float getLowestVertexZ ()
{
  float lowest = FLT_MAX ;

  for ( int i = 0 ; i < getNumVertices() ; i++ )
  {
    float v = vertex[i].vx [ 2 ] ;

    if ( v < lowest )
      lowest = v ;
  }

  return lowest ;
}


void  transformVertices ()
{
  for ( int i = 0 ; i < getNumVertices () ; i++ )
    getBone ( vertex[i].getBoneID() ) -> transform ( vertex[i].vx,
                                                     vertex[i].rel_vx ) ;
}


