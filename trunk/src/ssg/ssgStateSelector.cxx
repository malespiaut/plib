
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
      statelist [ i ] = s -> clone ( clone_flags ) ;
    else
      statelist [ i ] = s ;
  }
}


ssgStateSelector *ssgStateSelector::clone ( int clone_flags )
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

  statelist [ i ] = step ;
}


void ssgStateSelector::force (void)
{
  getCurrentStep()->force() ;
}

void ssgStateSelector::apply (void)
{
  getCurrentStep()->apply() ;
}


void ssgStateSelector:: care_about ( int mode )
{
  getCurrentStep()->care_about (mode);
}


void ssgStateSelector::dont_care_about ( int mode )
{
  getCurrentStep()->dont_care_about(mode);
}


int  ssgStateSelector::isEnabled ( GLenum mode )
{
  return getCurrentStep()->isEnabled(mode);
}

void ssgStateSelector::disable ( GLenum mode )
{
  getCurrentStep()->disable(mode);
}

void ssgStateSelector::enable ( GLenum mode )
{
  getCurrentStep()->enable(mode);
}


void ssgStateSelector::setTexture ( char *fname, int _wrapu, int _wrapv )
{
  getCurrentStep()->setTexture ( fname, _wrapu, _wrapv ) ;
}

char *ssgStateSelector::getTextureFilename (void)  
{
  return getCurrentStep()->getTextureFilename();
}

void ssgStateSelector::setTextureFilename ( char *fname )  
{
  getCurrentStep()->setTextureFilename ( fname ) ;
}

GLuint ssgStateSelector::getTextureHandle (void)  
{
  return getCurrentStep()->getTextureHandle();
}

void ssgStateSelector::setTexture ( ssgTexture *tex )
{
  getCurrentStep()->setTexture(tex) ;
}

void ssgStateSelector::setTexture ( GLuint tex )
{
  getCurrentStep()->setTexture(tex) ;
}

void ssgStateSelector::setColourMaterial(GLenum which)
{
  getCurrentStep()->setColourMaterial(which) ;
}

void ssgStateSelector::setMaterial ( GLenum which, float r, float g,
                                                   float b, float a )
{
  getCurrentStep()->setMaterial(which,r,g,b,a) ;
}


void ssgStateSelector::setMaterial ( GLenum which, sgVec4 rgba )
{
  getCurrentStep()->setMaterial(which,rgba) ;
}

float *ssgStateSelector::getMaterial ( GLenum which )
{
  return getCurrentStep()->getMaterial(which) ;
}

float ssgStateSelector::getShininess (void)
{
  return getCurrentStep()->getShininess() ;
}

void ssgStateSelector::setShininess ( float sh )
{
  getCurrentStep()->setShininess(sh) ;
}

void ssgStateSelector::setShadeModel ( GLenum model )
{
  getCurrentStep()->setShadeModel(model) ;
}

void ssgStateSelector::setAlphaClamp ( float clamp )
{
  getCurrentStep()->setAlphaClamp(clamp) ;
}

void ssgStateSelector::print ( FILE *fd, char *indent )
{
  getCurrentStep()->print(fd,indent);
}


int ssgStateSelector::load ( FILE *fd )
{
  int key, t, i ;

  _ssgReadInt ( fd, & nstates   ) ;
  _ssgReadInt ( fd, & selection ) ;
  
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
        statelist[i] = (ssgSimpleState *) _ssgGetFromList ( key ) ;
    }
    else
    if ( t == ssgTypeSimpleState() )
    {
      statelist[i] = new ssgSimpleState ;

      if ( ! statelist[i] -> load ( fd ) )
        return FALSE ;
    }
    else
    if ( t == ssgTypeStateSelector() )
    {
      statelist[i] = new ssgStateSelector ;

      if ( ! statelist[i] -> load ( fd ) )
        return FALSE ;
    }
    else
    {
      fprintf ( stderr, "ssgStateSelector::load - Unrecognised ssgState type 0x%08x\n", t ) ;
      statelist[i] = NULL ;
    }
  }

  return ssgSimpleState::load(fd) ;
}


int ssgStateSelector::save ( FILE *fd )
{
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
    if ( statelist[i] -> getSpare () > 0 )
    {
      _ssgWriteInt ( fd, SSG_BACKWARDS_REFERENCE ) ;
      _ssgWriteInt ( fd, statelist[i] -> getSpare () ) ;
    }
    else
    {
      _ssgWriteInt ( fd, statelist[i]->getType() ) ;
  
      if ( ! statelist[i] -> save ( fd ) )
        return FALSE ;
    }
  }

  return ssgSimpleState::save(fd) ;
}

