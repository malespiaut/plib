
#ifndef _SSGASHAPES_H_
#define _SSGASHAPES_H_  1



class ssgaShape : public ssgBranch
{
  int corrupted ;

protected:
  virtual void copy_from ( ssgaShape *src, int clone_flags ) ;

  sgVec3 center ;
  sgVec3 size   ;

  int ntriangles ;

  void init () ;

public:
  virtual ssgBase *clone ( int clone_flags = 0 ) ;
  ssgaShape (void) ;
  ssgaShape ( int numtris ) ;
  virtual ~ssgaShape (void) ;
  virtual char *getTypeName(void) ;

  void makeCorrupt () { corrupted = TRUE ; }
  int  isCorrupt   () { return corrupted ; }

  void setCenter  ( sgVec3 c ) { sgCopyVec3 ( center, c ) ; regenerate () ; }
  void setSize    ( sgVec3 s ) { sgCopyVec3 ( size  , s ) ; regenerate () ; }
  void setSize    ( float  s ) { sgSetVec3  ( size,s,s,s) ; regenerate () ; }
  void setNumTris ( int ntri ) { ntriangles = ntri ; regenerate () ; }

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
protected:
  virtual void copy_from ( ssgaSphere *src, int clone_flags ) ;
public:
  virtual ssgBase *clone ( int clone_flags = 0 ) ;
  ssgaSphere (void) ;
  ssgaSphere ( int numtris ) ;
  virtual ~ssgaSphere (void) ;
  virtual char *getTypeName(void) ;
  virtual void regenerate () ;
} ;



class ssgaCylinder : public ssgaShape
{
protected:
  virtual void copy_from ( ssgaCylinder *src, int clone_flags ) ;
public:
  virtual ssgBase *clone ( int clone_flags = 0 ) ;
  ssgaCylinder (void) ;
  ssgaCylinder ( int numtris ) ;
  virtual ~ssgaCylinder (void) ;
  virtual char *getTypeName(void) ;
  virtual void regenerate () ;
} ;

#endif

