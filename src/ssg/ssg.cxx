
#include "ssgLocal.h"

sgMat4 _ssgOpenGLAxisSwapMatrix =
{
  {  1.0f,  0.0f,  0.0f,  0.0f },
  {  0.0f,  0.0f, -1.0f,  0.0f },
  {  0.0f,  1.0f,  0.0f,  0.0f },
  {  0.0f,  0.0f,  0.0f,  1.0f }
} ;

sgVec3 _ssgVertex000   = { 0.0f, 0.0f, 0.0f } ;
sgVec4 _ssgColourWhite = { 1.0f, 1.0f, 1.0f, 1.0f } ;
sgVec3 _ssgNormalUp    = { 0.0f, 0.0f, 1.0f } ;
sgVec2 _ssgTexCoord00  = { 0.0f, 0.0f } ;

char *_ssgModelPath   = NULL ;
char *_ssgTexturePath = NULL ;

ssgLight _ssgLights [ 8 ] ;
int      _ssgFrameCounter = 0 ;

int  ssgGetFrameCounter () { return _ssgFrameCounter ; }
void ssgSetFrameCounter ( int fc ) { _ssgFrameCounter = fc ; }

char *ssgGetVersion ()
{
#ifdef VERSION
  return VERSION ;
#else
  return "Unknown" ;
#endif
}

void ssgDeRefDelete ( ssgBase *s )
{
  if ( s == NULL ) return ;

  s -> deRef () ;

  if ( s -> getRef () <= 0 )
    delete s ;
}

void ssgDelete ( ssgBranch *br )
{
  if ( br == NULL )
    return ;

  br -> removeAllKids () ;
  delete br ;
}

ssgLight *ssgGetLight ( int i )
{
  return &_ssgLights [ i ] ;
}

void ssgInit ()
{
#ifdef WIN32
  if ( wglGetCurrentContext () == NULL )
#else
#if defined(macintosh)
  if ( aglGetCurrentContext () == NULL )
#else
  if ( glXGetCurrentContext () == NULL )
#endif
#endif
  {
    fprintf ( stderr,
    "FATAL: ssgInit called without a valid OpenGL context.\n");
    exit ( 1 ) ;
  }

  ssgTexturePath ( "." ) ;
  ssgModelPath   ( "." ) ;

  _ssgLights [ 0 ] . setID ( 0 ) ;
  _ssgLights [ 0 ] . on    () ;

  for ( int i = 1 ; i < 8 ; i++ )
  {
    _ssgLights [ i ] . setID ( i ) ;
    _ssgLights [ i ] . off   () ;
  }

  new ssgContext ;  /* Sets the current context with defaults */
}


void ssgCullAndPick ( ssgRoot *r, sgVec2 botleft, sgVec2 topright )
{
  if ( _ssgCurrentContext == NULL )
  {
    fprintf ( stderr,
             "ssg: No Current Context: Did you forgot to call ssgInit()?\n" ) ;
    exit ( 1 ) ;
  }

  ssgForceBasicState () ;

  int w = (int)(topright[0] - botleft[0]) ;
  int h = (int)(topright[1] - botleft[1]) ;

  int x = (int)(botleft[0] + topright[0]) / 2 ;
  int y = (int)(botleft[1] + topright[1]) / 2 ;

  GLint viewport [ 4 ] ;
  glGetIntegerv ( GL_VIEWPORT, viewport ) ;
  glMatrixMode ( GL_PROJECTION ) ;
  glLoadIdentity () ;
  gluPickMatrix ( x, y, w, h, viewport ) ;
  _ssgCurrentContext->pushProjectionMatrix () ;

  glMatrixMode ( GL_MODELVIEW ) ;
  glLoadIdentity () ;

  int i ;

  for ( i = 0 ; i < 8 ; i++ )
    if ( _ssgLights [ i ] . isHeadlight () )
      _ssgLights [ i ] . setup () ;

  _ssgCurrentContext->loadModelviewMatrix () ;

  for ( i = 0 ; i < 8 ; i++ )
    if ( ! _ssgLights [ i ] . isHeadlight () )
      _ssgLights [ i ] . setup () ;

  _ssgCurrentContext->cull(r) ;
  _ssgDrawDList () ;

  glMatrixMode ( GL_MODELVIEW ) ;
  glLoadIdentity () ;
}


void ssgCullAndDraw ( ssgRoot *r )
{
  if ( _ssgCurrentContext == NULL )
  {
    fprintf ( stderr,
             "ssg: No Current Context: Did you forgot to call ssgInit()?\n" ) ;
    exit ( 1 ) ;
  }

  _ssgFrameCounter++ ;

  ssgForceBasicState () ;

  glMatrixMode ( GL_PROJECTION ) ;
  _ssgCurrentContext->loadProjectionMatrix () ;

  glMatrixMode ( GL_MODELVIEW ) ;
  glLoadIdentity () ;

  int i ;

  for ( i = 0 ; i < 8 ; i++ )
    if ( _ssgLights [ i ] . isHeadlight () )
      _ssgLights [ i ] . setup () ;

  _ssgCurrentContext->loadModelviewMatrix () ;

  for ( i = 0 ; i < 8 ; i++ )
    if ( ! _ssgLights [ i ] . isHeadlight () )
      _ssgLights [ i ] . setup () ;

  _ssgCurrentContext->cull(r) ;
  _ssgDrawDList () ;

  glMatrixMode ( GL_MODELVIEW ) ;
  glLoadIdentity () ;
}

void ssgModelPath ( char *s )
{
  delete _ssgModelPath ;
  _ssgModelPath = new char [ strlen ( s ) + 1 ] ;
  strcpy ( _ssgModelPath, s ) ;
}

void ssgTexturePath ( char *s )
{
  delete _ssgTexturePath ;
  _ssgTexturePath = new char [ strlen ( s ) + 1 ] ;
  strcpy ( _ssgTexturePath, s ) ;
}


char *ssgBase         ::getTypeName (void) { return "ssgBase"          ; }
char *ssgTexture      ::getTypeName (void) { return "ssgTexture"       ; }
char *ssgState        ::getTypeName (void) { return "ssgState"         ; }
char *ssgSimpleState  ::getTypeName (void) { return "ssgSimpleState"   ; }
char *ssgStateSelector::getTypeName (void) { return "ssgStateSelector" ; }
char *ssgEntity       ::getTypeName (void) { return "ssgEntity"        ; }
char *ssgLeaf         ::getTypeName (void) { return "ssgLeaf"          ; }
char *ssgVTable       ::getTypeName (void) { return "ssgVTable"        ; }
char *ssgVtxTable     ::getTypeName (void) { return "ssgVtxTable"      ; }
char *ssgBranch       ::getTypeName (void) { return "ssgBranch"        ; }
char *ssgSelector     ::getTypeName (void) { return "ssgSelector"      ; }
char *ssgRangeSelector::getTypeName (void) { return "ssgRangeSelector" ; }
char *ssgTimedSelector::getTypeName (void) { return "ssgTimedSelector" ; }
char *ssgBaseTransform::getTypeName (void) { return "ssgBaseTransform" ; }
char *ssgTransform    ::getTypeName (void) { return "ssgTransform"     ; }
char *ssgTexTrans     ::getTypeName (void) { return "ssgTexTrans"      ; }
char *ssgCutout       ::getTypeName (void) { return "ssgCutout"        ; }
char *ssgRoot         ::getTypeName (void) { return "ssgRoot"          ; }


static char *file_extension ( char *fname )
{
  char *p = & ( fname [ strlen(fname) ] ) ;

  while ( p != fname && *p != '/' && *p != '.' )
    p-- ;

  return p ;
}


ssgEntity *ssgLoad ( char *fname, ssgHookFunc hookfunc )
{
  if ( fname == NULL || *fname == '\0' )
    return NULL ;

  char *extn = file_extension ( fname ) ;

  if ( *extn != '.' )
    return NULL ;

  if ( _ssgStrNEqual ( extn, ".ac", 3 ) )
  { 
    ssgEntity *obj = ssgLoadAC ( fname, hookfunc ) ;

    if ( obj == NULL )
      return NULL ;

    /* Do some simple optimisations */

    ssgBranch *model = new ssgBranch () ;
    model -> addKid ( obj ) ;
    ssgFlatten      ( obj ) ;
    ssgStripify   ( model ) ;
    return model ;
  }

  if ( _ssgStrNEqual ( extn, ".ase", 4 ) )
    return ssgLoadASE ( fname, hookfunc ) ;

  if ( _ssgStrNEqual ( extn, ".wrl", 4 ) )
    return ssgLoadVRML ( fname, hookfunc ) ;

  if ( _ssgStrNEqual ( extn, ".3ds", 4 ) )
    return ssgLoad3ds  ( fname, hookfunc ) ;

  if ( _ssgStrNEqual ( extn, ".ssg", 4 ) )
    return ssgLoadSSG  ( fname, hookfunc ) ;

  if ( _ssgStrNEqual ( extn, ".dxf", 4 ) )
    return ssgLoadDXF ( fname, hookfunc ) ;

  return NULL ;
}


int ssgSave ( char *fname, ssgEntity *ent )
{
  if ( fname == NULL || ent == NULL || *fname == '\0' )
    return FALSE ;

  char *extn = file_extension ( fname ) ;

  if ( *extn != '.' )
    return FALSE ;

  if ( _ssgStrNEqual ( extn, ".ac", 3 ) )
    return ssgSaveAC ( fname, ent ) ;

  if ( _ssgStrNEqual ( extn, ".ase", 4 ) )
    return ssgSaveASE ( fname, ent ) ;

  if ( _ssgStrNEqual ( extn, ".ssg", 4 ) )
    return ssgSaveSSG ( fname, ent ) ;

  if ( _ssgStrNEqual ( extn, ".dxf", 4 ) )
    return ssgSaveDXF ( fname, ent ) ;

  return FALSE ;
}


