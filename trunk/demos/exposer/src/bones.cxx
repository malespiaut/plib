#include "exposer.h"

sgVec3 curr_translate = { 0.0f, 0.0f, 0.0f } ;

ssgSimpleState *boneState = NULL ;

int rootBone = -1  ;
int nextBone =  0  ;
Bone *bone = NULL ;

int nextVertex = 0 ;

puSlider *XtranslateSlider ;
puSlider *YtranslateSlider ;
puSlider *ZtranslateSlider ;
puInput  *XtranslateInput  ;
puInput  *YtranslateInput  ;
puInput  *ZtranslateInput  ;

struct Vertex
{
  int     boneID ;
  sgVec3  rel_vx ;
  float  *vx     ;
} ;

#define MAX_VERTICES  (64*1024)
Vertex vertex [ MAX_VERTICES ] ;

int getNumBones () { return nextBone ; }
Bone *getBone ( int i ) { return & ( bone [ i ] ) ; }


void jointHeadingCB ( puObject *ob )
{
  Bone *bone = (Bone *) (ob->getUserData()) ;
  float a ; ob -> getValue ( & a ) ;
 
  bone->setAngle ( 0, a * 360.0f - 180.0f ) ;
  setShowAngle ( a * 360.0f - 180.0f ) ;
}
 
 
void jointPitchCB ( puObject *ob )
{
  Bone *bone = (Bone *) (ob->getUserData()) ;
  float a ; ob -> getValue ( & a ) ;
 
  bone->setAngle ( 1, a * 360.0f - 180.0f ) ;
  setShowAngle ( a * 360.0f - 180.0f ) ;
}
 
 
void jointRollCB ( puObject *ob )
{
  Bone *bone = (Bone *) (ob->getUserData()) ;
  float a ; ob -> getValue ( & a ) ;
 
  bone->setAngle ( 2, a * 360.0f - 180.0f ) ;
  setShowAngle ( a * 360.0f - 180.0f ) ;
}
 
 
void hide_headingCB ( puObject *ob )
{
  Bone *bone = (Bone *) (ob->getUserData()) ;
 
  if ( ob -> getValue () )
    bone -> sh -> hide () ;
  else
    bone -> sh -> reveal () ;
}
 
 
void hide_pitchCB ( puObject *ob )
{
  Bone *bone = (Bone *) (ob->getUserData()) ;
 
  if ( ob -> getValue () )
    bone -> sp -> hide () ;
  else
    bone -> sp -> reveal () ;
}
                                                                                 
void hide_rollCB ( puObject *ob )
{
  Bone *bone = (Bone *) (ob->getUserData()) ;
 
  if ( ob -> getValue () )
    bone -> sr -> hide () ;
  else
    bone -> sr -> reveal () ;
}
 
 
void resetCB ( puObject *ob )
{
  Bone *bone = (Bone *) (ob->getUserData()) ;
 
  bone -> setAngles ( 0, 0, 0 ) ;
}


Bone::Bone ()
{
  parent = -1 ;
}


 
void Bone::read ( FILE *fd )
{
  char name [ PUSTRING_MAX ] ;
  char shb, spb, srb ;
 
  fscanf ( fd, "BONE \"%s %c%c%c\n",
    name, & shb, & spb, & srb ) ;
 
  if ( name[strlen(name)-1] == '\"' )
    name[strlen(name)-1] = '\0' ;
 
  na -> setValue ( name ) ;
  hb -> setValue ( (shb == 'H') ? 0 : 1 ) ;
  pb -> setValue ( (spb == 'P') ? 0 : 1 ) ;
  rb -> setValue ( (srb == 'R') ? 0 : 1 ) ;
 
  hide_headingCB ( hb ) ;
  hide_pitchCB   ( pb ) ;
  hide_rollCB    ( rb ) ;
}
 
 
void Bone::write ( FILE *fd )
{
  fprintf ( fd, "BONE \"%s\" %c%c%c\n",
    na->getStringValue(),
    hb->getValue() ? '.' : 'H',
    pb->getValue() ? '.' : 'P',
    rb->getValue() ? '.' : 'R' ) ;
}



void Bone::createJoint ()
{
  widget = new puGroup ( 0, 0 ) ;
  rs = new puOneShot ( 0, 0, "x" ) ; 
  hb = new puButton  (20, 0, "H" ) ; 
  pb = new puButton  (40, 0, "P" ) ; 
  rb = new puButton  (60, 0, "R" ) ; 
  sh = new puDial  (  80, 0, 40 ) ;
  sp = new puDial  ( 120, 0, 40 ) ;
  sr = new puDial  ( 160, 0, 40 ) ;
  na = new puInput ( 0,20,80,40 ) ;

  na->setUserData ( this ) ;
  na->setColourScheme ( colour[0], colour[1], colour[2], 0.5f ) ;

  rs->setUserData ( this ) ;
  rs->setCallback ( resetCB ) ;
  rs->setColourScheme ( colour[0], colour[1], colour[2], 0.5f ) ;

  hb->setUserData ( this ) ;
  hb->setCallback ( hide_headingCB ) ;
  hb->setColourScheme ( colour[0], colour[1], colour[2], 0.5f ) ;

  pb->setUserData ( this ) ;
  pb->setCallback ( hide_pitchCB ) ;
  pb->setColourScheme ( colour[0], colour[1], colour[2], 0.5f ) ;

  rb->setUserData ( this ) ;
  rb->setCallback ( hide_rollCB ) ;
  rb->setColourScheme ( colour[0], colour[1], colour[2], 0.5f ) ;

  sh->setUserData ( this ) ;
  sh->setValue ( 0.5f ) ;
  sh->setCallback ( jointHeadingCB ) ;
  sh->setColourScheme ( colour[0], colour[1], colour[2], 0.5f ) ;

  sp->setUserData ( this ) ;
  sp->setValue ( 0.5f ) ;
  sp->setCallback ( jointPitchCB ) ;
  sp->setColourScheme ( colour[0], colour[1], colour[2], 0.5f ) ;

  sr->setUserData ( this ) ;
  sr->setValue ( 0.5f ) ;
  sr->setCallback ( jointRollCB ) ;
  sr->setColourScheme ( colour[0], colour[1], colour[2], 0.5f ) ;

  widget -> close () ;
  widget -> hide  () ;
}



void Bone::setAngles ( float h, float p, float r )
{
  sgVec3  hpr ;
  sgSetVec3 ( hpr, h, p, r ) ;
  setAngles ( hpr ) ;
}


void Bone::setAngle ( int which, float a )
{
  getXForm() -> hpr [ which ] = a ;
  setAngles ( getXForm() -> hpr ) ;
}

void Bone::setAngles ( sgVec3 src )
{
  sgCopyVec3 ( getXForm() -> hpr, src ) ;
}


float *Bone::getDialAngles ()
{
  static sgVec3 dst ;
  dst[0] = sh -> getFloatValue () * 360.0f - 180.0f ;
  dst[1] = sp -> getFloatValue () * 360.0f - 180.0f ;
  dst[2] = sr -> getFloatValue () * 360.0f - 180.0f ;
  return dst ;
}


sgCoord *Bone::getXForm ( Event *prev, Event *next, float lerptime )
{
  static sgCoord c ;

  sgCoord *coord0 = prev->getBoneCoord ( id ) ;
  sgCoord *coord1 = next->getBoneCoord ( id ) ;

  sgCopyVec3 ( c.xyz, xlate ) ;

  sgLerpAnglesVec3 ( c.hpr, coord0->hpr, coord1->hpr, lerptime ) ;

  sh -> setValue ( (c.hpr[0] + 180.0f) / 360.0f ) ;
  sp -> setValue ( (c.hpr[1] + 180.0f) / 360.0f ) ;
  sr -> setValue ( (c.hpr[2] + 180.0f) / 360.0f ) ;

  return & c ;
}


sgCoord *Bone::getXForm ()
{
  if ( curr_event != NULL )
  {
    sgCoord *coord = curr_event->getBoneCoord ( id ) ;
    sgCopyVec3 ( coord->xyz, xlate ) ;
    return coord ;
  }

  static sgCoord coord ;
  sgZeroCoord ( & coord ) ;
  sgCopyVec3 ( coord.xyz, xlate ) ;
  return &coord ;
}


void Bone::computeTransform ( Event *prev, Event *next, float t )
{
  effector -> setTransform ( getXForm( prev, next, t ) ) ;
}

void Bone::transform ( sgVec3 dst, sgVec3 src )
{
  sgXformPnt3 ( dst, src, netMatrix ) ;
  sgAddVec3 ( dst, curr_translate ) ;
}



void Bone::swapEnds()
{
  sgVec3 tmp ;

  /* Swap vertices so that vx0 is always the root. */

  sgCopyVec3 ( tmp, vx[0] ) ;
  sgCopyVec3 ( vx[0], vx[1] ) ;
  sgCopyVec3 ( vx[1], tmp ) ;

  sgCopyVec3 ( tmp, orig_vx[0] ) ;
  sgCopyVec3 ( orig_vx[0], orig_vx[1] ) ;
  sgCopyVec3 ( orig_vx[1], tmp ) ;
}



void Bone::init ( ssgLeaf *l, sgMat4 newmat, short vv[2], int ident )
{
  parent = -1 ;
  sgCopyVec3  ( vx [ 0 ], l -> getVertex ( vv[0] ) ) ;
  sgCopyVec3  ( vx [ 1 ], l -> getVertex ( vv[1] ) ) ;
  sgXformPnt3 ( vx [ 0 ], newmat ) ;
  sgXformPnt3 ( vx [ 1 ], newmat ) ;

  sgCopyVec3 ( orig_vx [ 0 ], vx [ 0 ] ) ;
  sgCopyVec3 ( orig_vx [ 1 ], vx [ 1 ] ) ;

  id = ident ;
  if ( sgEqualVec3 ( vx [ 0 ], vx [ 1 ] ) )
    fprintf ( stderr, "exposer: Zero length bone found.\n" ) ;
}



void Bone::print ( FILE *fd, int which )
{
  fprintf ( fd, "Bone %d: vx  (%f,%f,%f) -> (%f,%f,%f)  Parent = %d\n",
             which, 
             vx[0][0],vx[0][1],vx[0][2],
             vx[1][0],vx[1][1],vx[1][2],
             parent ) ;
}



void printBones ()
{
  fprintf ( stderr, "BONE TREE.\n" ) ;
  fprintf ( stderr, "~~~~~~~~~~\n" ) ;

  for ( int i = 0 ; i < getNumBones() ; i++ )
    getBone(i)->print(stderr, i) ;

  fprintf ( stderr, "\n" ) ;
}




void opaqueBones ()
{
  if ( boneState != NULL )
    boneState -> disable ( GL_BLEND ) ;
}


void blendBones ()
{
  if ( boneState != NULL )
    boneState -> enable ( GL_BLEND ) ;
}


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


#define NUM_COLOURS 13

sgVec4 colourTable [] =
{
  { 1, 0, 0, 0.3f }, { 0, 1, 0, 0.3f }, { 0, 0, 1, 0.3f },
  { 1, 1, 0, 0.3f }, { 1, 0, 1, 0.3f }, { 0, 1, 1, 0.3f },

  { 0.5f, 0.5f, 0.5f, 0.3f },

  { 1, 0.5f, 0.5f, 0.3f }, { 0.5f,  1  , 0.5f, 0.3f }, { 0.5f, 0.5f, 1, 0.3f },
  { 1,  1  , 0.5f, 0.3f }, {  1  , 0.5f,  1  , 0.3f }, { 0.5f, 1, 1, 0.3f } 
} ;




ssgBranch *Bone::generateGeometry ( int root )
{
  static int nextColIndex = 0 ;

  ssgTransform *b = new ssgTransform ;

  b -> setUserData ( this ) ;
  effector = b ;
  sgZeroVec3 ( getXForm()->hpr ) ;
  sgCopyVec3 ( xlate, vx[0] ) ;
  sgZeroVec3 ( vx[0] ) ;
  sgSubVec3  ( vx[1], xlate ) ;

  sgMat4 mat ;
  sgMakeCoordMat4 ( mat, getXForm() ) ;
  b -> setTransform ( mat ) ;

  offsetChildBones ( root, xlate ) ;

  sgCopyVec4 ( colour, colourTable[nextColIndex] ) ;

  createJoint () ;

  ssgaCube *shape = new ssgaCube () ;
  sgVec3 org ; sgCopyVec3  ( org, vx[1] ) ;
  sgVec3 siz ; sgCopyVec3  ( siz, vx[1] ) ;

  sgScaleVec3 ( org, 0.5 ) ;

  siz[0] = fabs ( siz[0] ) ; if ( siz[0] <= 0.1f ) siz [ 0 ] = 0.1f ;
  siz[1] = fabs ( siz[1] ) ; if ( siz[1] <= 0.1f ) siz [ 1 ] = 0.1f ;
  siz[2] = fabs ( siz[2] ) ; if ( siz[2] <= 0.1f ) siz [ 2 ] = 0.1f ;

  if ( boneState == NULL )
  {
    boneState = new ssgSimpleState () ;
    boneState -> setColourMaterial ( GL_AMBIENT_AND_DIFFUSE ) ;
    boneState -> setMaterial ( GL_SPECULAR, 0, 0, 0, 1 ) ;
    boneState -> setMaterial ( GL_EMISSION, 0, 0, 0, 1 ) ;
    boneState -> enable ( GL_BLEND ) ;
  }


  shape -> setCenter ( org ) ;
  shape -> setSize   ( siz ) ;
  shape -> setColour ( colourTable[nextColIndex] ) ;
  shape -> setKidState ( boneState ) ;

  nextColIndex = ( nextColIndex + 1 ) % NUM_COLOURS ;

  b -> addKid ( shape ) ;

  for ( int i = 0 ; i < getNumBones() ; i++ )
    if ( i != root && getBone(i)->parent == root )
      b -> addKid ( getBone(i)->generateGeometry ( i ) ) ;

  return b ;
}


ssgBranch *extractBones ( ssgBranch *root )
{
  sgMat4 mat ;

  nextBone =  0 ;
  rootBone = -1 ;

  if ( bone == NULL )
    bone = new Bone [ 1000 ] ;

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

    walkTransforms ( (ssgRoot *) e, newmat ) ;
  }
}


void transformModel ( ssgRoot *boneRoot, float tim )
{
  if ( getNumEvents () < 1 )
    return ;

  Event *prev = getEvent ( 0 ) ;
  Event *next = prev ;

  if ( curr_event )
  {
    prev = next = curr_event ;
    tim = curr_event -> getTime () ;
  }
  else
  {
    for ( int i = 1 ; i < getNumEvents () ; i++ )
    {
      Event *ev = getEvent ( i ) ;
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

  XtranslateSlider -> setValue ( curr_translate [ 0 ] / 5.0f + 0.5f ) ;
  YtranslateSlider -> setValue ( curr_translate [ 1 ] / 5.0f + 0.5f ) ;
  ZtranslateSlider -> setValue ( curr_translate [ 2 ] / 5.0f + 0.5f ) ;

  if ( ! XtranslateInput -> isAcceptingInput () )
    XtranslateInput  -> setValue ( curr_translate [ 0 ] ) ;
  if ( ! YtranslateInput -> isAcceptingInput () )
    YtranslateInput  -> setValue ( curr_translate [ 1 ] ) ;
  if ( ! ZtranslateInput -> isAcceptingInput () )
    ZtranslateInput  -> setValue ( curr_translate [ 2 ] ) ;

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



float *getCurrTranslate ()
{
  return curr_translate ;
}                                                                               



void currTranslateTxtXCB ( puObject *sl )
{
  float v = sl -> getFloatValue () ;

  if ( curr_event == NULL ) return ;

  sgVec3 xyz ;

  curr_event -> getTranslate ( xyz ) ;
  xyz [ 0 ] = v ;
  curr_event -> setTranslate ( xyz ) ;
}



void currTranslateTxtYCB ( puObject *sl )
{
  float v = sl -> getFloatValue () ;

  if ( curr_event == NULL ) return ;

  sgVec3 xyz ;

  curr_event -> getTranslate ( xyz ) ;
  xyz [ 1 ] = v ;
  curr_event -> setTranslate ( xyz ) ;
}



void currTranslateTxtZCB ( puObject *sl )
{
  float v = sl -> getFloatValue () ;

  if ( curr_event == NULL ) return ;

  sgVec3 xyz ;

  curr_event -> getTranslate ( xyz ) ;
  xyz [ 2 ] = v ;
  curr_event -> setTranslate ( xyz ) ;
}




void currTranslateXCB ( puObject *sl )
{
  float v = (((puSlider *)sl) -> getFloatValue () - 0.5 ) * 5.0f ;

  if ( curr_event == NULL ) return ;

  sgVec3 xyz ;

  curr_event -> getTranslate ( xyz ) ;
  xyz [ 0 ] = v ;
  curr_event -> setTranslate ( xyz ) ;
}



void currTranslateYCB ( puObject *sl )
{
  float v = (((puSlider *)sl) -> getFloatValue () - 0.5 ) * 5.0f ;

  if ( curr_event == NULL ) return ;

  sgVec3 xyz ;

  curr_event -> getTranslate ( xyz ) ;
  xyz [ 1 ] = v ;
  curr_event -> setTranslate ( xyz ) ;
}



void currTranslateZCB ( puObject *sl )
{
  float v = (((puSlider *)sl) -> getFloatValue () - 0.5 ) * 5.0f ;

  if ( curr_event == NULL ) return ;

  sgVec3 xyz ;

  curr_event -> getTranslate ( xyz ) ;
  xyz [ 2 ] = v ;
  curr_event -> setTranslate ( xyz ) ;
}


void init_bones ()
{
  sgZeroVec3 ( curr_translate ) ;

  puText   *message ;

  ZtranslateInput  =  new puInput ( 5, 485, 80, 505 ) ;
  ZtranslateInput  -> setCallback ( currTranslateTxtZCB ) ;

  ZtranslateSlider = new puSlider ( 80, 485, 120, FALSE ) ;
  ZtranslateSlider -> setCBMode   ( PUSLIDER_DELTA ) ;
  ZtranslateSlider -> setDelta    ( 0.01    ) ;
  ZtranslateSlider -> setCallback ( currTranslateZCB ) ;
  message = new puText ( 205,485 ) ; message->setLabel ( "Z" ) ; 

  YtranslateInput  =  new puInput ( 5, 505, 80, 525 ) ;
  YtranslateInput  -> setCallback ( currTranslateTxtYCB ) ;

  YtranslateSlider = new puSlider ( 80, 505, 120, FALSE ) ;
  YtranslateSlider -> setCBMode   ( PUSLIDER_DELTA ) ;
  YtranslateSlider -> setDelta    ( 0.01    ) ;
  YtranslateSlider -> setCallback ( currTranslateYCB ) ;
  message = new puText ( 205,505 ) ; message->setLabel ( "Y" ) ; 

  XtranslateInput  =  new puInput ( 5, 525, 80, 545 ) ;
  XtranslateInput  -> setCallback ( currTranslateTxtZCB ) ;

  XtranslateSlider = new puSlider ( 80, 525, 120, FALSE ) ;
  XtranslateSlider -> setCBMode   ( PUSLIDER_DELTA ) ;
  XtranslateSlider -> setDelta    ( 0.01    ) ;
  XtranslateSlider -> setCallback ( currTranslateXCB ) ;
  message = new puText ( 205,525 ) ; message->setLabel ( "X" ) ; 
}


