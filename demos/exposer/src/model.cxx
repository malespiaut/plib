#include "exposer.h"

int rootBone = -1  ;
int nextBone =  0  ;

int getNumBones () { return nextBone ; }                                  

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

      for ( int ii = 0 ; ii < tween -> getNumVertices () ; ii++ )
	v1 -> add ( tween -> getVertex   ( ii ) ) ;

      tween -> newBank ( v1, NULL, NULL, NULL ) ;
    }
  }
}


void makeTweenCopy ( ssgBranch *dst, ssgBranch *src )
{
  for ( int i = 0 ; i < src -> getNumKids () ; i++ )
  {
    ssgEntity *s =  src -> getKid ( i ) ;
    ssgEntity *d =  dst -> getKid ( i ) ;

    if ( s -> isAKindOf ( ssgTypeBranch() ) )
      makeTweenCopy ( (ssgBranch *)d, (ssgBranch *)s ) ;
    else
    {
      ssgLeaf *ls = (ssgLeaf *) s ;
      ssgLeaf *ld = (ssgLeaf *) d ;

      for ( int ii = 0 ; ii < ls -> getNumVertices () ; ii++ )
        sgCopyVec3 ( ld->getVertex(ii), ls->getVertex(ii) ) ;
    }
  }
}


void tweenify ( ssgBranch *root )
{
  for ( int i = 0 ; i < root -> getNumKids () ; i++ )
  {
    ssgEntity *e =  root -> getKid ( i ) ;

    if ( e -> isAKindOf ( ssgTypeBranch() ) )
      tweenify ( (ssgBranch *)e ) ;
    else
    {
      ssgLeaf *l = (ssgLeaf *) e ;

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



ssgEntity *makeTweenCopy ( ssgEntity *root )
{
  if ( root == NULL )
    return NULL ;
 
  ssgEntity *res = (ssgEntity *) ( root -> clone ( SSG_CLONE_RECURSIVE |
                                                   SSG_CLONE_GEOMETRY ) ) ;
  tweenify ( (ssgBranch *) res ) ;

  return res ;
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

  nextBone =  0 ;
  rootBone = -1 ;                                                              

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

    if ( e -> isAKindOf ( ssgTypeBranch () ) )
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

  sgLerpVec3 ( getCurrTranslate(), ptra, ntra, lerptime ) ;

  syncTranslators ( getCurrTranslate() ) ;

  int i ;

  for ( i = 0 ; i < getNumBones () ; i++ )
    getBone ( i ) -> computeTransform ( prev, next, lerptime ) ;

  sgMat4 mat ;

  sgMakeIdentMat4 ( mat ) ;

  walkTransforms ( (ssgBranch *) boneRoot, mat ) ; 

  transformVertices () ;
}


