
#ifndef _SSGASHAPES_H_
#define _SSGASHAPES_H_  1



class ssgaShape : public ssgBranch
{
  int corrupted ;

protected:
  virtual void copy_from ( ssgaShape *src, int clone_flags ) ;

  sgVec4 colour ;
  sgVec3 center ;
  sgVec3 size   ;

  int ntriangles ;

  ssgState   *kidState      ;
  ssgCallback kidPreDrawCB  ;
  ssgCallback kidPostDrawCB ;

  void init () ;

protected:

  ssgState    *getKidState      () { return kidState      ; }
  ssgCallback  getKidPreDrawCB  () { return kidPreDrawCB  ; }
  ssgCallback  getKidPostDrawCB () { return kidPostDrawCB ; }

public:
  virtual ssgBase *clone ( int clone_flags = 0 ) ;
  ssgaShape (void) ;
  ssgaShape ( int numtris ) ;
  virtual ~ssgaShape (void) ;
  virtual char *getTypeName(void) ;

  void makeCorrupt () { corrupted = TRUE ; }
  int  isCorrupt   () { return corrupted ; }

  float *getCenter () { return center ; }
  float *getSize   () { return size   ; }

  void setColour  ( sgVec4 c ) { sgCopyVec4 ( colour, c ) ; regenerate () ; }
  void setCenter  ( sgVec3 c ) { sgCopyVec3 ( center, c ) ; regenerate () ; }
  void setSize    ( sgVec3 s ) { sgCopyVec3 ( size  , s ) ; regenerate () ; }
  void setSize    ( float  s ) { sgSetVec3  ( size,s,s,s) ; regenerate () ; }
  void setNumTris ( int ntri ) { ntriangles = ntri ; regenerate () ; }

  void setKidState    ( ssgState *s )
  {
    kidState = s ;

    for ( int i = 0 ; i < getNumKids() ; i++ )
      ((ssgLeaf *)getKid(i)) -> setState ( s ) ;
  }

  void setKidCallback ( int cb_type, ssgCallback cb )
  {
    if ( cb_type == SSG_CALLBACK_PREDRAW )
      kidPreDrawCB = cb ;
    else
      kidPostDrawCB = cb ;

    for ( int i = 0 ; i < getNumKids() ; i++ )
      ((ssgLeaf *)getKid(i)) -> setCallback ( cb_type, cb ) ;
  }

  virtual void regenerate () = 0 ;
} ;



class ssgaCube : public ssgaShape
{
protected:
  virtual void copy_from ( ssgaCube *src, int clone_flags ) ;
public:
  virtual ssgBase *clone ( int clone_flags = 0 ) ;
  ssgaCube (void) ;
  ssgaCube ( int numtris ) ;
  virtual ~ssgaCube (void) ;
  virtual char *getTypeName(void) ;
  virtual void regenerate () ;
} ;



class ssgaSphere : public ssgaShape
{
  int latlong_style ;

  void regenerateLatLong () ;
  void regenerateTessellatedIcosahedron () ;
protected:
  virtual void copy_from ( ssgaSphere *src, int clone_flags ) ;
public:
  virtual ssgBase *clone ( int clone_flags = 0 ) ;
  ssgaSphere (void) ;
  ssgaSphere ( int numtris ) ;
  virtual ~ssgaSphere (void) ;
  virtual char *getTypeName(void) ;
  virtual void regenerate () ;

  void setLatLongStyle ( int ll ) { latlong_style = ll ; regenerate () ; }
  int  isLatLongStyle  ()         { return latlong_style ; }
} ;



class ssgaCylinder : public ssgaShape
{
  int capped ;

protected:
  virtual void copy_from ( ssgaCylinder *src, int clone_flags ) ;
public:
  virtual ssgBase *clone ( int clone_flags = 0 ) ;
  ssgaCylinder (void) ;
  ssgaCylinder ( int numtris ) ;
  virtual ~ssgaCylinder (void) ;
  virtual char *getTypeName(void) ;
  virtual void regenerate () ;

  void makeCapped ( int c ) { capped = c ; regenerate () ; }
  int  isCapped   ()        { return capped ; }
} ;

#endif

