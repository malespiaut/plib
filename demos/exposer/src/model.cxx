#include "exposer.h"

void findChildBones ( int root )
{
  float *v0 = getBone ( root ) -> vx[0] ;
  float *v1 = getBone ( root ) -> vx[1] ;

  /*
    Compute parametric equation.
  */

  sgVec3 vec ;
  float  len ;
  int i ;

  sgSubVec3 ( vec, v1, v0 ) ;
  len = sgLengthVec3 ( vec ) ;
  sgScaleVec3 ( vec, 1.0f/len ) ;

  for ( i = 0 ; i < getNumBones() ; i++ )
  {
    /*******************************
     If this bone already has a parent
     then we are done.
    ********************************/

    if ( getBone(i)->parent >= 0 )
      continue ;

    /*******************************
     Test vertex zero of the daughter
     bone against the parent bone.
    ********************************/

    float *vi0 = getBone(i)-> vx[0] ;
    float *vi1 = getBone(i)-> vx[1] ;
    sgVec3 vec_i ;
    float  len_i ;

    sgSubVec3 ( vec_i, vi0, v0 ) ;
    len_i = sgLengthVec3 ( vec_i ) ;

    if ( len_i <= 0.01 )   /* Close to vertex 0 of parent? */
    {
      getBone(i)-> parent = root ;
      continue ;
    }

    sgScaleVec3 ( vec_i, 1.0f/len_i ) ;

    /* Somewhere between vertex 0 and vertex 1 of parent? */

    if ( sgDistanceSquaredVec3 ( vec, vec_i ) <= 0.001 && len_i <= len * 1.01 )
    {
      getBone(i)-> parent = root ;
      continue ;
    }

    /*******************************
     Test vertex one of the daughter
     bone against the parent bone.
    ********************************/

    sgSubVec3 ( vec_i, vi1, v0 ) ;
    len_i = sgLengthVec3 ( vec_i ) ;

    if ( len_i <= 0.01 )   /* Close to vertex 0 of parent? */
    {
      getBone(i)->parent = root ;
      getBone(i)->swapEnds () ;
      continue ;
    }

    sgScaleVec3 ( vec_i, 1.0f/len_i ) ;

    /* Somewhere between vertex 0 and vertex 1 of parent? */

    if ( sgDistanceSquaredVec3 ( vec, vec_i ) <= 0.001 && len_i <= len * 1.01 )
    {
      getBone(i)->parent = root ;
      getBone(i)->swapEnds () ;
      continue ;
    }
  }

  /****************************************************
    Recursively search any new child nodes we found.
  ****************************************************/
 
  for ( i = 0 ; i < getNumBones() ; i++ )
    if ( i != root && getBone(i)->parent == root ) /* If Parented by this root node */
      findChildBones ( i ) ;  /* Recurse downwards. */
}


void addTweenBank ( ssgBranch *root )
{
  if ( root == NULL )
    return ;
 
  for ( int i = 0 ; i < root -> getNumKids () ; i++ )
  {
    ssgEntity *e =  root -> getKid ( i ) ;

    if ( e -> isAKindOf ( ssgTypeBranch() ) )
      addTweenBank ( (ssgBranch *)e ) ;
    else
    if ( e -> isAKindOf ( ssgTypeTween() ) )
    {
      ssgTween *tween = (ssgTween *) e ; 

      ssgVertexArray   *v1 = new ssgVertexArray   ;
      ssgNormalArray   *n1 = new ssgNormalArray   ;
      ssgTexCoordArray *t1 = new ssgTexCoordArray ;
      ssgColourArray   *c1 = new ssgColourArray   ;

      for ( int ii = 0 ; ii < tween -> getNumVertices () ; ii++ )
      {
	v1 -> add ( tween -> getVertex   ( ii ) ) ;
	n1 -> add ( tween -> getNormal   ( ii ) ) ;
	t1 -> add ( tween -> getTexCoord ( ii ) ) ;
	c1 -> add ( tween -> getColour   ( ii ) ) ;
      }

      tween -> newBank ( v1, n1, t1, c1 ) ;
    }
  }
}



void walkBones ( ssgBranch *root, sgMat4 mat )
{
  if ( root == NULL )
    return ;
 
  sgMat4 newmat ;

  if ( root -> isAKindOf ( ssgTypeTransform() ) )
  {
    sgMat4 tmp ;

    ((ssgTransform *)root)-> getTransform ( tmp ) ;
    sgMultMat4 ( newmat, mat, tmp ) ;
  }
  else
    sgCopyMat4 ( newmat, mat ) ;

  for ( int i = 0 ; i < root -> getNumKids () ; i++ )
  {
    ssgEntity *e =  root -> getKid ( i ) ;

    if ( e -> isAKindOf ( ssgTypeBranch() ) )
      walkBones ( (ssgBranch *)e, newmat ) ;
    else
    {
      ssgLeaf *l = (ssgLeaf *) e ;

      if ( l -> getPrimitiveType () == GL_LINES ||
           l -> getPrimitiveType () == GL_LINE_LOOP ||
           l -> getPrimitiveType () == GL_LINE_STRIP )
      {
        for ( int ll = 0 ; ll < l->getNumLines() ; ll++ )
        {
          short vv [ 2 ] ;

          l -> getLine ( ll, & vv[0], & vv[1] ) ;

          getBone ( getNumBones() ) -> init ( l, newmat, vv,
                                              getNumBones() ) ;

          /*
            Is this line red?
            If so, it's the 'root' line.
          */

          float *col = l -> getColour ( vv[0] ) ;

          if ( l -> getState () != NULL &&
               l -> getState () -> isAKindOf ( ssgTypeSimpleState () ) )
          {
            ssgSimpleState *ss = (ssgSimpleState *)( l -> getState () ) ;

//            if ( ! ss -> getColourMaterial () == GL_DIFFUSE &&
//                 ! ss -> getColourMaterial () == GL_AMBIENT_AND_DIFFUSE )
            if ( ss -> getColourMaterial () != GL_DIFFUSE &&
                 ss -> getColourMaterial () != GL_AMBIENT_AND_DIFFUSE )
            {
              col = ss -> getMaterial ( GL_DIFFUSE ) ;
            }
          }

          if ( col [ 0 ] > 0.75 && col [ 1 ] < 0.25 && col [ 2 ] < 0.25 )
          {
            if ( rootBone != -1 )
              rootBone = -2 ;
            else
              rootBone = getNumBones() ;
          }

          nextBone++ ;
        }

        root -> removeKid ( e ) ;
        i-- ;
      }
      else
      {
        ssgVertexArray   *v0 = new ssgVertexArray   ;
        ssgNormalArray   *n0 = new ssgNormalArray   ;
        ssgTexCoordArray *t0 = new ssgTexCoordArray ;
        ssgColourArray   *c0 = new ssgColourArray   ;

        for ( int ii = 0 ; ii < l -> getNumVertices () ; ii++ )
        {
          v0 -> add ( l -> getVertex   ( ii ) ) ;
          n0 -> add ( l -> getNormal   ( ii ) ) ;
          t0 -> add ( l -> getTexCoord ( ii ) ) ;
          c0 -> add ( l -> getColour   ( ii ) ) ;
        }

        ssgTween *tween = new ssgTween ( l -> getPrimitiveType () ) ;

        tween -> setState ( l -> getState () ) ;
        tween -> newBank ( v0, n0, t0, c0 ) ;

        root -> replaceKid ( l, tween ) ;
      }
    }
  }
}



void offsetChildBones ( int root, sgVec3 v )
{
  for ( int i = 0 ; i < getNumBones() ; i++ )
    if ( getBone(i)->parent == root )
    {
      sgSubVec3 ( getBone(i)->vx[0], v ) ;
      sgSubVec3 ( getBone(i)->vx[1], v ) ;
      offsetChildBones ( i, v ) ;
    }
}


ssgBranch *extractBones ( ssgBranch *root )
{
  sgMat4 mat ;

  sgMakeIdentMat4 ( mat ) ;
  walkBones ( root, mat ) ;

  if ( rootBone == -1 )
  {
    fprintf ( stderr, "exposer: ERROR - No 'root' bone found?!?\n" ) ;
    fprintf ( stderr, "exposer:   (Exactly one 'bone' line should be RED).\n" ) ;
    exit ( 1 ) ;
  }

  if ( rootBone == -2 )
  {
    fprintf ( stderr, "exposer: ERROR - Multiple 'root' bones found?!?\n" ) ;
    fprintf ( stderr, "exposer:   (Only one 'bone' line should be RED).\n" ) ;
    exit ( 1 ) ;
  }

  fprintf ( stderr, "exposer: %d bones found, rootBone is number %d.\n",
                                  getNumBones(), rootBone ) ;

  getBone(rootBone)->parent = ROOT_BONE ;

  findChildBones ( rootBone ) ;

  int disconnected = 0 ;

  for ( int i = 0 ; i < getNumBones() ; i++ )
    if ( getBone(i)->parent < 0 )
      disconnected++ ;

  if ( disconnected > 0 )
  {
    fprintf ( stderr,
         "exposer: ERROR - %d bones are not connected to the 'root' bone.\n",
                                 disconnected ) ;
    fprintf ( stderr,
         "exposer:   (The root bone line should be RED.  All other bones\n" ) ;
    fprintf ( stderr,
         "exposer:   should either touch the root bone - or touch some\n" ) ;
    fprintf ( stderr,
         "exposer:   other bone that is connected to the root bone so that\n");
    fprintf ( stderr,
         "exposer:   all the bones form one continuously connected\n" ) ;
    fprintf ( stderr,
         "exposer:   skeleton.)\n" ) ;
    exit ( 1 ) ;
  }


  ssgBranch *res = getBone ( rootBone ) -> generateGeometry ( rootBone ) ;

  return res ;
}


void walkVertices ( ssgBranch *root, sgMat4 mat )
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

        for ( int j = 0 ; j < nextVertex ; j++ )
          if ( v == vertex [ j ] . vx )
          {
            vv = j ;
            break ;
          }

        if ( vv < 0 )
        {
          sgXformPnt3 ( v, mat ) ;
          vertex [ nextVertex ] . vx = v ;
          sgCopyVec3 ( vertex [ nextVertex ] . rel_vx, v ) ;

          nextVertex++ ;
        }
      }
    }
  }
}


void extractVertices ( ssgBranch *root )
{
  sgMat4 mat ;

  nextVertex = 0 ;

  /*
    Walk the model - making a list of all of the
    unique vertices - flattening the model's transforms
    as we go.
  */

  sgMakeIdentMat4 ( mat ) ;
  walkVertices ( root, mat ) ;

  fprintf ( stderr, "exposer: %d vertices found.\n", nextVertex ) ;

  /*
    Now find the nearest bone to each vertex and
    compute it's position relative to the root of
    that bone.
  */

  for ( int i = 0 ; i < nextVertex ; i++ )
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

  for ( int i = 0 ; i < nextVertex ; i++ )
  {
    float v = vertex[i].vx [ 2 ] ;

    if ( v < lowest )
      lowest = v ;
  }

  return lowest ;
}


void walkTransforms ( ssgBranch *root, sgMat4 mat )
{
  if ( root == NULL )
    return ;
 
  sgMat4 newmat ;

  if ( root -> isAKindOf ( ssgTypeTransform() ) )
  {
    sgMat4 tmp ;

    ((ssgTransform *)root)-> getTransform ( tmp ) ;
    sgMultMat4 ( newmat, mat, tmp ) ;

    Bone *bo = (Bone *)(((ssgTransform *)root)-> getUserData ()) ;
    sgCopyMat4 ( bo -> netMatrix, newmat ) ;
  }
  else
    sgCopyMat4 ( newmat, mat ) ;

  for ( int i = 0 ; i < root -> getNumKids () ; i++ )
  {
    ssgEntity *e =  root -> getKid ( i ) ;

    if ( ! e -> isAKindOf ( ssgTypeBranch () ) )
      continue ;

    walkTransforms ( (ssgBranch *) e, newmat ) ;
  }
}



void transformModel ( ssgBranch *boneRoot, float tim )
{
  if ( eventList->getNumEvents () < 1 )
    return ;

  Event *prev = eventList->getEvent ( 0 ) ;
  Event *next = prev ;

  if ( eventList->getCurrentEvent() != NULL )
  {
    prev = next = eventList->getCurrentEvent() ;
    tim = eventList->getCurrentEvent() -> getTime () ;
  }
  else
  {
    for ( int i = 1 ; i < eventList->getNumEvents () ; i++ )
    {
      Event *ev = eventList->getEvent ( i ) ;
      float t = ev -> getTime () ;

      if ( t < tim )
        next = prev = ev ;
      else
      {
        next = ev ;
        break ;
      }
    }
  }

  float lerptime = 0.0f ;

  if ( next->getTime() - prev->getTime() >= 0.01 )
  {
    lerptime =  tim - prev->getTime() ;
    lerptime /= (next->getTime() - prev->getTime()) ;
  }

  sgVec3 ptra ; prev->getTranslate(ptra) ;
  sgVec3 ntra ; next->getTranslate(ntra) ;

  sgLerpVec3 ( curr_translate, ptra, ntra, lerptime ) ;

  syncTranslators ( curr_translate ) ;

  int i ;

  for ( i = 0 ; i < getNumBones () ; i++ )
    getBone ( i ) -> computeTransform ( prev, next, lerptime ) ;

  ssgBranch *b = boneRoot ;

  sgMat4 mat ;

  sgMakeIdentMat4 ( mat ) ;

  walkTransforms ( b, mat ) ; 

  for ( i = 0 ; i < nextVertex ; i++ )
    getBone ( vertex[i].boneID ) -> transform ( vertex[i].vx,
                                                vertex[i].rel_vx ) ;
}


