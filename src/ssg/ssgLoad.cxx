
#include "ssgLocal.h" 


static char _ssgAPOM[16*1024]=""; // APOM = actual path of model (!=ModelPath)
static char _ssgANOM[256]=""; // actual name of model

char * ssgGetANOM()
{ return _ssgANOM;
}

char * ssgGetAPOM()
{ return _ssgAPOM;
}



enum { MAX_SHARED_STATES = 1000 };
static ssgSimpleState* shared_states [ MAX_SHARED_STATES ] ;
static int num_shared_states = 0 ;
static ssgTextureArray shared_textures ;

static void _ssgShareReset ()
{
  int i;

  shared_textures.removeAll () ;

  //~~ T.G. we ref() all new states (see below) so we need to deref here
  for ( i = 0; i < num_shared_states; i++)
    ssgDeRefDelete( shared_states[i] );

  num_shared_states = 0 ;
}

static ssgSimpleState* _ssgShareState ( ssgSimpleState* st )
{
  if ( st == NULL )
     return NULL ;

  for ( int i = 0 ; i < num_shared_states ; i++ )
  {
    ssgSimpleState *st2 = shared_states [ i ] ;

    if ( st == st2 )
      return NULL ; //same pointer -- don't change state

    if ( st->isEnabled ( GL_TEXTURE_2D ) != st2->isEnabled ( GL_TEXTURE_2D ) )
      continue ;

    if ( st->isEnabled ( GL_TEXTURE_2D ) &&
       st -> getTextureHandle () != st2 -> getTextureHandle () )
      continue ;

    if ( st->getCareAbout (SSG_GL_SPECULAR) != st2->getCareAbout (SSG_GL_SPECULAR) ||
      st->getCareAbout (SSG_GL_EMISSION) != st2->getCareAbout (SSG_GL_EMISSION) ||
      st->getCareAbout (SSG_GL_AMBIENT) != st2->getCareAbout (SSG_GL_AMBIENT) ||
      st->getCareAbout (SSG_GL_DIFFUSE) != st2->getCareAbout (SSG_GL_DIFFUSE) )
      continue ;

    if ( ! st->getCareAbout (SSG_GL_SPECULAR) &&
       ! sgEqualVec4 ( st->getMaterial (GL_SPECULAR), st2->getMaterial (GL_SPECULAR) ) )
      continue ;

    if ( ! st->getCareAbout (SSG_GL_EMISSION) &&
       ! sgEqualVec4 ( st->getMaterial (GL_EMISSION), st2->getMaterial (GL_EMISSION) ) )
      continue ;

    if ( ! st->getCareAbout (SSG_GL_AMBIENT) &&
       ! sgEqualVec4 ( st->getMaterial (GL_AMBIENT), st2->getMaterial (GL_AMBIENT) ) )
      continue ;

    if ( ! st->getCareAbout (SSG_GL_DIFFUSE) &&
       ! sgEqualVec4 ( st->getMaterial (GL_DIFFUSE), st2->getMaterial (GL_DIFFUSE) ) )
      continue ;

    if ( st -> isTranslucent () != st2 -> isTranslucent () ||
         st -> getShininess () != st2 -> getShininess () )
      continue ;

    return st2 ;//switch to this state
  }

  //we have a state we've never seen before
  if ( num_shared_states < MAX_SHARED_STATES )
  {
    st -> ref();  // deref'ed in _ssgShareReset()
    shared_states [ num_shared_states++ ] = st ;
  }
  return NULL ; //don't change state
}

ssgLeaf* ssgLoaderOptions::defaultCreateLeaf ( ssgLeaf* leaf,
                                              const char* parent_name ) const
{
  /* is this just a sharing 'reset' */
  if ( leaf == NULL )
  {
    _ssgShareReset () ;
    return NULL ;
  }
  
  /* do we have a texture filename? */
  ssgState* st = leaf -> getState () ;
  if ( st != NULL )
  {
    assert ( st -> isAKindOf ( SSG_TYPE_SIMPLESTATE ) ) ;
    ssgSimpleState *ss = (ssgSimpleState*) st ;
    
    if ( ss -> getTextureFilename() != NULL ) {
      st = createState( ss -> getTextureFilename() ) ;
      if ( st != NULL ) {
        leaf -> setState( st ) ;
        ss = NULL;
      }
    }
    
    if (ss != NULL) {
      ss = _ssgShareState ( ss ) ;
      if ( ss != NULL )
        leaf -> setState ( ss ) ;
    }
  }
  
  return leaf ;
}


ssgTexture* ssgLoaderOptions::defaultCreateTexture ( char* tfname,
						     int wrapu,
						     int wrapv,
						     int mipmap ) const
{
  char filename [ 1024 ] ;
	ulFindFile( filename, _ssgTexturePath, tfname, _ssgAPOM ) ;

  ssgTexture *tex = shared_textures.find ( filename ) ;
  if ( tex )
    return tex ;
  
  tex = new ssgTexture ( filename, wrapu, wrapv, mipmap ) ;
  if ( tex )
    shared_textures.add ( tex ) ;
  return tex ;
}

ssgTransform* ssgLoaderOptions::defaultCreateTransform ( ssgTransform* tr,
      ssgTransformArray* ta ) const
{
  if ( ta != NULL )
    tr -> setUserData ( ta ) ;
  return tr ;
}

ssgLoaderOptions _ssgDefaultOptions ( NULL, NULL, NULL, NULL, NULL ) ;

void ssgModelPath ( const char *s )
{
  delete _ssgModelPath ;
  _ssgModelPath = new char [ strlen ( s ) + 1 ] ;
  strcpy ( _ssgModelPath, s ) ;
}

void ssgTexturePath ( const char *s )
{
  delete _ssgTexturePath ;
  _ssgTexturePath = new char [ strlen ( s ) + 1 ] ;
  strcpy ( _ssgTexturePath, s ) ;
}


static const char *file_extension ( const char *fname )
{
  const char *p = & ( fname [ strlen(fname) ] ) ;

  while ( p != fname && *p != '/' && *p != '.' )
    p-- ;

  return p ;
}


typedef ssgEntity *_ssgLoader ( const char *, const ssgLoaderOptions * ) ;
typedef int         _ssgSaver ( const char *, ssgEntity * ) ;

struct _ssgFileFormat
{
  const char *extension ;
  _ssgLoader *loadfunc ;
  _ssgSaver  *savefunc ;
} ;


static _ssgFileFormat formats[] =
{
  { ".3ds",   ssgLoad3ds  , ssgSave3ds },
  { ".ac" ,   ssgLoadAC3D , ssgSaveAC  },
  { ".ase",   ssgLoadASE  , ssgSaveASE },
  { ".dxf",   ssgLoadDXF  , ssgSaveDXF },
  { ".obj",   ssgLoadOBJ  , ssgSaveOBJ },
  { ".ssg",   ssgLoadSSG  , ssgSaveSSG },
  { ".tri",   ssgLoadTRI  , ssgSaveTRI },
  { ".wrl",   ssgLoadVRML , NULL       },
  { ".md2",   ssgLoadMD2  , NULL       },
  { ".mdl",   ssgLoadMDL  , NULL       },
  { ".x"  ,   ssgLoadX    , ssgSaveX   },
  { ".flt",   ssgLoadFLT  , NULL       },
  { ".strip", ssgLoadStrip, NULL       },
  { ".m"  ,   ssgLoadM    , ssgSaveM   },
  { ".off"  , ssgLoadOFF  , ssgSaveOFF },
	{ ".qhi"  , NULL        , ssgSaveQHI },
  { NULL  , NULL       , NULL       }
} ;


	// Changes 14.Feb.2001, Wolfram Kuss:
	// I need this functionality *now* so that I can at last publish my homepage.
	// This code *may* not be very elegant, but should be 
	// portable and works.
	// For more, see mailing list.
	// I/we will decide on a final solution shortly.

#if defined(WIN32) && !defined(__CYGWIN__)
  #include "Shlwapi.h"
	#include "assert.h"
  #define appendPath(a, b) assert( PathAppend(a, b));
#else
//wk stop1

static void appendPath(char *a, char *b)
// appends b to a
{
  if ( b[0] == '/' )
    strcpy ( a, b ) ;  /* b is an absolute path - replace a by b */
	else
		if ( a[0] == 0 )
			strcpy ( a, b ) ;
		else 
			// therefore strlen(a)>0
			if ( a[ strlen(a)-1 ] == '/' )
			{ if ( b[0] == '/' )
					strcat ( a, &(b[1]) );
				else
					strcat ( a, b );
			}
			else
			{ if ( b[0] == '/' )
					strcat ( a, b );
				else
				{ strcat ( a, "/" );
					strcat ( a, b );
				}
			}
}

#endif
  

ssgEntity *ssgLoad ( const char *fname, const ssgLoaderOptions* options )
{
  if ( fname == NULL || *fname == '\0' )
    return NULL ;


	// save path in _ssgAPOM (actual path of model):
#ifdef EXPERIMENTAL_WINDOWS_APOM
	// 14.Feb.2001:
	// This is experimental code by Wolfram Kuss. It runs only on Windows.
	// It will be removed in favour of a final version shortly.

	// start alt + neu1
	strncpy( _ssgAPOM, fname, 1024);
	char *s_ptr;
	s_ptr = &(_ssgAPOM[strlen(_ssgAPOM)-1]);
	while ((s_ptr > _ssgAPOM) && (*s_ptr != '/') && (*s_ptr != '\\')) 
		s_ptr--;
	if ( s_ptr >= _ssgAPOM ) *s_ptr = 0;
	// stop alt + neu1
  // wk start neu1
	char buffer[1024];
  if ( NULL != _fullpath( buffer, _ssgAPOM, 1024 ))
		strncpy( _ssgAPOM, buffer, 1024);
	// stop neu1

#else

	// Changes 14.Feb.2001, Wolfram Kuss:
	// This code accumulates all the relative pathes that make up
	// _ssgAPOM. This code may not be very elegant, but should be 
	// portable and avoid the problems with ssgGetAPOM.
	// For more, see mailing list.
	// I/we will decide on a final solution shortly.

	char buffer[16*1024];
	strncpy( buffer, fname, 16*1024);
	char *s_ptr;
	s_ptr = &(buffer[strlen(buffer)-1]);
	while ((s_ptr > buffer) && (*s_ptr != '/') && (*s_ptr != '\\')) 
		s_ptr--;
// wk: Hint for wk :-): missing in Windows code:

	if ((*s_ptr != '/') && (*s_ptr != '\\'))
	  strcpy(_ssgANOM, s_ptr);
	else
		strcpy(_ssgANOM, &(s_ptr[1]));
	if ( strchr(_ssgANOM, '.') != NULL )
		strchr(_ssgANOM, '.')[0] = 0;
//
	if ( s_ptr >= buffer ) *s_ptr = 0;
	appendPath(_ssgAPOM, buffer);
#endif

	// end of 14.Feb.2001 changes
	


  // find appropiate loader and call its loadfunc
  const char *extn = file_extension ( fname ) ;
  if ( *extn != '.' )
  {
    ulSetError ( UL_WARNING, "ssgLoad: Cannot determine file type for '%s'", fname );
    return NULL ;
  }

  for ( _ssgFileFormat *f = formats; f->extension != NULL; f++ )
    if ( f->loadfunc != NULL &&
	       _ssgStrEqual ( extn, f->extension ) )
      return f->loadfunc( fname, options ) ;

  ulSetError ( UL_WARNING, "ssgLoad: Unrecognised file type '%s'", extn ) ;
  return NULL ;
}


int ssgSave ( const char *fname, ssgEntity *ent )
{
  if ( fname == NULL || ent == NULL || *fname == '\0' )
    return FALSE ;

  const char *extn = file_extension ( fname ) ;

  if ( *extn != '.' )
  {
    ulSetError ( UL_WARNING, "ssgSave: Cannot determine file type for '%s'", fname );
    return FALSE ;
  }

  for ( _ssgFileFormat *f = formats; f->extension != NULL; f++ )
    if ( f->savefunc != NULL &&
         _ssgStrEqual ( extn, f->extension ) )
      return f->savefunc( fname, ent ) ;

  ulSetError ( UL_WARNING, "ssgSave: Unrecognised file type '%s'", extn ) ;
  return FALSE ;
}


char* _ssgMakePath( char* path, const char* dir, const char* fname )
{ // MakePath was originally in ssg, so we dont want to remove this function from ssg
	// But we need it in ul also
	return ulMakePath( path, dir, fname );
}
