
#include "ssgLocal.h"

void ssgStateSelector::copy_from ( ssgStateSelector *src, int clone_flags )
{
  ssgSimpleState::copy_from ( src, clone_flags ) ;

  nstates   = src -> getNumSteps   () ;
  selection = src -> getSelectStep () ;
  statelist = new ssgSimpleState * [ nstates ] ;

  for ( int i = 0 ; i < nstates ; i++ )
  {
    ssgSimpleState *s = src -> getStep ( i ) ;

    if ( s != NULL && (clone_flags & SSG_CLONE_STATE_RECURSIVE) )
      statelist [ i ] = (ssgSimpleState *)( s -> clone ( clone_flags )) ;
    else
      statelist [ i ] = s ;

	//~~ T.G. needs ref count incremented
	if (statelist [ i ] != NULL )      
	   statelist [ i ] -> ref();   

  }
}


ssgBase *ssgStateSelector::clone ( int clone_flags )
{
  ssgStateSelector *b = new ssgStateSelector ;
  b -> copy_from ( this, clone_flags ) ;
  return b ;
}


ssgStateSelector::ssgStateSelector () 
{ 
  nstates = 0 ;
  selection = -1 ; 
  statelist = NULL ;
}

ssgStateSelector::ssgStateSelector ( int ns ) 
{ 
  nstates   = ns ;
  selection = -1 ; 
  statelist = new ssgSimpleState * [ nstates ] ;

  for ( int i = 0 ; i < nstates ; i++ )
    statelist [ i ] = NULL ;
}

ssgStateSelector::~ssgStateSelector (void)
{
  //~~ T.G. deref states before deleting list
  for ( int i = 0 ; i < nstates ; i++ )    
	  ssgDeRefDelete( statelist [ i ] );  
  delete [] statelist ;
}

void ssgStateSelector::selectStep ( unsigned int s )
{
  selection = s ;
}

unsigned int ssgStateSelector::getSelectStep (void)
{
  return selection ;
}

ssgSimpleState *ssgStateSelector::getCurrentStep  (void)
{
  return ( selection < 0 ||
           selection >= nstates ||
           statelist [ selection ] == NULL ) ? this : statelist[selection] ;
}

ssgSimpleState *ssgStateSelector::getStep ( int i )
{
  return ( i < 0 ||
           i >= nstates ||
           statelist [ i ] == NULL ) ? this : statelist[i] ;
}




void ssgStateSelector::setStep  (int i, ssgSimpleState *step)
{
  if ( i < 0 || i >= nstates ) return ;

  //~~ T.G. removed null test -- not necessary
  ssgDeRefDelete ( statelist[i] ) ;

  statelist [ i ] = step ;

  if ( step != NULL )
    step -> ref () ;
}


void ssgStateSelector::force (void)
{
  ssgStateSelector * s = (ssgStateSelector *) getCurrentStep() ;

  if ( s == this )
    ssgSimpleState::force() ;
  else
    s -> force() ;
}

void ssgStateSelector::apply (void)
{
  ssgStateSelector * s = (ssgStateSelector *) getCurrentStep() ;

  if ( s == this )
    ssgSimpleState::apply() ;
  else
    s -> apply() ;
}


void ssgStateSelector:: care_about ( int mode )
{
  ssgStateSelector * s = (ssgStateSelector *) getCurrentStep() ;

  if ( s == this )
    ssgSimpleState::care_about (mode) ;
  else
    s -> care_about (mode);
}


void ssgStateSelector::dont_care_about ( int mode )
{
  ssgStateSelector * s = (ssgStateSelector *) getCurrentStep() ;

  if ( s == this )
    ssgSimpleState::dont_care_about(mode) ;
  else
    s -> dont_care_about(mode);
}


int  ssgStateSelector::isEnabled ( GLenum mode )
{
  ssgStateSelector * s = (ssgStateSelector *) getCurrentStep() ;

  if ( s == this )
    return ssgSimpleState::isEnabled(mode) ;
  else
    return s -> isEnabled(mode) ;
}

void ssgStateSelector::disable ( GLenum mode )
{
  ssgStateSelector * s = (ssgStateSelector *) getCurrentStep() ;

  if ( s == this )
    ssgSimpleState::disable(mode) ;
  else
    s  -> disable(mode) ;
}

void ssgStateSelector::enable ( GLenum mode )
{
  ssgStateSelector * s = (ssgStateSelector *) getCurrentStep() ;

  if ( s == this )
    ssgSimpleState::enable(mode) ;
  else
    s -> enable(mode) ;
}


void ssgStateSelector::setTexture ( char *fname, int _wrapu, int _wrapv,
				    int _mipmap )
{
  ssgStateSelector * s = (ssgStateSelector *) getCurrentStep() ;

  if ( s == this )
    ssgSimpleState::setTexture ( fname, _wrapu, _wrapv, _mipmap ) ;
  else
    s  -> setTexture ( fname, _wrapu, _wrapv, _mipmap ) ;
}

char *ssgStateSelector::getTextureFilename (void)  
{
  ssgStateSelector * s = (ssgStateSelector *) getCurrentStep() ;

  if ( s == this )
    return ssgSimpleState::getTextureFilename() ;
  else
    return s -> getTextureFilename();
}

void ssgStateSelector::setTextureFilename ( char *fname )  
{
  ssgStateSelector * s = (ssgStateSelector *) getCurrentStep() ;

  if ( s == this )
    ssgSimpleState::setTextureFilename ( fname ) ;
  else
    s -> setTextureFilename ( fname ) ;
}

GLuint ssgStateSelector::getTextureHandle (void)  
{
  ssgStateSelector * s = (ssgStateSelector *) getCurrentStep() ;

  if ( s == this )
    return ssgSimpleState::getTextureHandle() ;
  else
    return s -> getTextureHandle() ;
}

void ssgStateSelector::setTexture ( ssgTexture *tex )
{
  ssgStateSelector * s = (ssgStateSelector *) getCurrentStep() ;

  if ( s == this )
    ssgSimpleState::setTexture(tex) ;
  else
    s -> setTexture(tex) ;
}

void ssgStateSelector::setTexture ( GLuint tex )
{
  ssgStateSelector * s = (ssgStateSelector *) getCurrentStep() ;

  if ( s == this )
    ssgSimpleState::setTexture(tex) ;
  else
    s -> setTexture(tex) ;
}

void ssgStateSelector::setColourMaterial(GLenum which)
{
  ssgStateSelector * s = (ssgStateSelector *) getCurrentStep() ;

  if ( s == this )
    ssgSimpleState::setColourMaterial(which) ;
  else
    s -> setColourMaterial(which) ;
}

void ssgStateSelector::setMaterial ( GLenum which, float r, float g,
                                                   float b, float a )
{
  ssgStateSelector * s = (ssgStateSelector *) getCurrentStep() ;

  if ( s == this )
    ssgSimpleState::setMaterial(which,r,g,b,a) ;
  else
    s -> setMaterial(which,r,g,b,a) ;
}


void ssgStateSelector::setMaterial ( GLenum which, sgVec4 rgba )
{
  ssgStateSelector * s = (ssgStateSelector *) getCurrentStep() ;

  if ( s == this )
    ssgSimpleState::setMaterial(which,rgba) ;
  else
    s -> setMaterial(which,rgba) ;
}

float *ssgStateSelector::getMaterial ( GLenum which )
{
  ssgStateSelector * s = (ssgStateSelector *) getCurrentStep() ;

  if ( s == this )
    return ssgSimpleState::getMaterial(which) ;
  else
    return s -> getMaterial(which) ;
}

float ssgStateSelector::getShininess (void)
{
  ssgStateSelector * s = (ssgStateSelector *) getCurrentStep() ;

  if ( s == this )
    return ssgSimpleState::getShininess() ;
  else
    return s -> getShininess() ;
}

void ssgStateSelector::setShininess ( float sh )
{
  ssgStateSelector * s = (ssgStateSelector *) getCurrentStep() ;

  if ( s == this )
    ssgSimpleState::setShininess(sh) ;
  else
    s -> setShininess(sh) ;
}

void ssgStateSelector::setShadeModel ( GLenum model )
{
  ssgStateSelector * s = (ssgStateSelector *) getCurrentStep() ;

  if ( s == this )
    ssgSimpleState::setShadeModel(model) ;
  else
    s -> setShadeModel(model) ;
}

void ssgStateSelector::setAlphaClamp ( float clamp )
{
  ssgStateSelector * s = (ssgStateSelector *) getCurrentStep() ;

  if ( s == this )
    ssgSimpleState::setAlphaClamp(clamp) ;
  else
    s -> setAlphaClamp(clamp) ;
}

void ssgStateSelector::print ( FILE *fd, char *indent, int how_much )
{
  ssgStateSelector * s = (ssgStateSelector *) getCurrentStep() ;

  if ( s == this )
    ssgSimpleState::print(fd, indent, how_much) ;
  else
    s -> print(fd, indent, how_much) ;
}


int ssgStateSelector::load ( FILE *fd )
{
  int key, t, i ;

  _ssgReadInt ( fd, & nstates   ) ;
  _ssgReadInt ( fd, & selection ) ;

  //~~ T.G. clear state list if already existing
  //   or create new list
  if (statelist != NULL)
  {
     for ( int i = 0 ; i < nstates ; i++ )    
	    ssgDeRefDelete( statelist [ i ] );  
  } else
     statelist = new ssgSimpleState * [ nstates ] ;

  for ( i = 0 ; i < nstates ; i++ )
    statelist [ i ] = NULL ;

  for ( i = 0 ; i < nstates ; i++ )
  {
    _ssgReadInt ( fd, & t ) ;

    if ( t == SSG_BACKWARDS_REFERENCE )
    {
      _ssgReadInt ( fd, & key ) ;

      if ( key == 0 )
        statelist[i] = NULL ;
      else
      {
        statelist[i] = (ssgSimpleState *) _ssgGetFromList ( key ) ;
		//~~ T.G. 
		if (statelist[i]) statelist[i]->ref();
      } 
    }
    else
    if ( t == ssgTypeSimpleState() )
    {
      statelist[i] = new ssgSimpleState ;
	  //~~ T.G. 
	  statelist[i]->ref(); 

      if ( ! statelist[i] -> load ( fd ) )
        return FALSE ;
    }
    else
    if ( t == ssgTypeStateSelector() )
    {
      statelist[i] = new ssgStateSelector ;
	  //~~ T.G. 
	  statelist[i]->ref(); 

      if ( ! statelist[i] -> load ( fd ) )
        return FALSE ;
    }
    else
    {
      ulSetError ( UL_WARNING,
        "ssgStateSelector::load - Unrecognised ssgState type 0x%08x", t ) ;
      statelist[i] = NULL ;
    }
  }

  return ssgSimpleState::load(fd) ;
}


int ssgStateSelector::save ( FILE *fd )
{
#ifdef WRITE_SSG_VERSION_ZERO
	ulSetError ( UL_WARNING, "I doubt that ssgStateSelectors can be saved in File format 0. Proceed at your own risc" ) ;
#endif  

  _ssgWriteInt ( fd, nstates   ) ;
  _ssgWriteInt ( fd, selection ) ;
  for ( int i = 0 ; i < nstates ; i++ )
  {
    if ( statelist[i] == NULL )
    {
      _ssgWriteInt ( fd, SSG_BACKWARDS_REFERENCE ) ;
      _ssgWriteInt ( fd, 0 ) ;
    }
    else
#ifndef WRITE_SSG_VERSION_ZERO
    if ( statelist[i] -> getSpare () > 0 )
    {
      _ssgWriteInt ( fd, SSG_BACKWARDS_REFERENCE ) ;
      _ssgWriteInt ( fd, statelist[i] -> getSpare () ) ;
    }
    else
#endif
    {
      _ssgWriteInt ( fd, statelist[i]->getType() ) ;
  
      if ( ! statelist[i] -> save ( fd ) )
        return FALSE ;
    }
  }

  return ssgSimpleState::save(fd) ;
}

