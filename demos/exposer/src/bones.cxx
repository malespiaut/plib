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
  if ( getCurrentEvent() != NULL )
  {
    sgCoord *coord = getCurrentEvent()->getBoneCoord ( id ) ;
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




void syncTranslators ( sgVec3 trans )
{
  XtranslateSlider -> setValue ( trans [ 0 ] / 5.0f + 0.5f ) ;
  YtranslateSlider -> setValue ( trans [ 1 ] / 5.0f + 0.5f ) ;
  ZtranslateSlider -> setValue ( trans [ 2 ] / 5.0f + 0.5f ) ;

  if ( ! XtranslateInput -> isAcceptingInput () )
    XtranslateInput  -> setValue ( trans [ 0 ] ) ;
  if ( ! YtranslateInput -> isAcceptingInput () )
    YtranslateInput  -> setValue ( trans [ 1 ] ) ;
  if ( ! ZtranslateInput -> isAcceptingInput () )
    ZtranslateInput  -> setValue ( trans [ 2 ] ) ;

}



float *getCurrTranslate ()
{
  return curr_translate ;
}                                                                               



void currTranslateTxtXCB ( puObject *sl )
{
  float v = sl -> getFloatValue () ;

  if ( getCurrentEvent() == NULL ) return ;

  sgVec3 xyz ;

  getCurrentEvent() -> getTranslate ( xyz ) ;
  xyz [ 0 ] = v ;
  getCurrentEvent() -> setTranslate ( xyz ) ;

  syncTranslators ( xyz ) ;
}



void currTranslateTxtYCB ( puObject *sl )
{
  float v = sl -> getFloatValue () ;

  if ( getCurrentEvent() == NULL ) return ;

  sgVec3 xyz ;

  getCurrentEvent() -> getTranslate ( xyz ) ;
  xyz [ 1 ] = v ;
  getCurrentEvent() -> setTranslate ( xyz ) ;

  syncTranslators ( xyz ) ;
}



void currTranslateTxtZCB ( puObject *sl )
{
  float v = sl -> getFloatValue () ;

  if ( getCurrentEvent() == NULL ) return ;

  sgVec3 xyz ;

  getCurrentEvent() -> getTranslate ( xyz ) ;
  xyz [ 2 ] = v ;
  getCurrentEvent() -> setTranslate ( xyz ) ;

  syncTranslators ( xyz ) ;
}




void currTranslateXCB ( puObject *sl )
{
  float v = (((puSlider *)sl) -> getFloatValue () - 0.5 ) * 5.0f ;

  if ( getCurrentEvent() == NULL ) return ;

  sgVec3 xyz ;

  getCurrentEvent() -> getTranslate ( xyz ) ;
  xyz [ 0 ] = v ;
  getCurrentEvent() -> setTranslate ( xyz ) ;

  syncTranslators ( xyz ) ;
}



void currTranslateYCB ( puObject *sl )
{
  float v = (((puSlider *)sl) -> getFloatValue () - 0.5 ) * 5.0f ;

  if ( getCurrentEvent() == NULL ) return ;

  sgVec3 xyz ;

  getCurrentEvent() -> getTranslate ( xyz ) ;
  xyz [ 1 ] = v ;
  getCurrentEvent() -> setTranslate ( xyz ) ;

  syncTranslators ( xyz ) ;
}



void currTranslateZCB ( puObject *sl )
{
  float v = (((puSlider *)sl) -> getFloatValue () - 0.5 ) * 5.0f ;

  if ( getCurrentEvent() == NULL ) return ;

  sgVec3 xyz ;

  getCurrentEvent() -> getTranslate ( xyz ) ;
  xyz [ 2 ] = v ;
  getCurrentEvent() -> setTranslate ( xyz ) ;

  syncTranslators ( xyz ) ;
}


void init_bones ()
{
  nextBone =  0 ;
  rootBone = -1 ;
 
  if ( bone == NULL )
    bone = new Bone [ 1000 ] ;

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


