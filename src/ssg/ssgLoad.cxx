
#include "ssgLocal.h" 


static char _ssgAPOM[16*1024]=""; // APOM = actual path of model (!=ModelPath)
static char _ssgANOM[256]=""; // actual name of model

char * ssgGetANOM()
{ return _ssgANOM;
}

char * ssgGetAPOM()
{ return _ssgAPOM;
}


static ssgSimpleStateArray shared_states ;
static ssgTextureArray shared_textures ;


ssgLeaf* ssgLoaderOptions::defaultCreateLeaf ( ssgLeaf* leaf,
                                              const char* parent_name ) const
{
  /* is this just a sharing 'reset' */
  if ( leaf == NULL )
  {
    shared_textures.removeAll () ;
    shared_states.removeAll () ;
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
    
    if (ss != NULL)
    {
      ssgSimpleState *match = shared_states.findMatch ( ss ) ;
      if ( match )
        leaf -> setState ( match ) ;
      else
        shared_states.add ( ss ) ;
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

  ssgTexture *tex = shared_textures.findByFilename ( filename ) ;
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


struct _ssgModelFormat
{
  const char *extension ;
  ssgLoadFunc *loadfunc ;
  ssgSaveFunc *savefunc ;
} ;


enum { MAX_FORMATS = 100 } ;

static _ssgModelFormat formats [ MAX_FORMATS ] ;
static int num_formats = 0 ;


void ssgAddModelFormat ( const char* extension,
                        ssgLoadFunc *loadfunc , ssgSaveFunc  *savefunc )
{
  if ( num_formats < MAX_FORMATS )
  {
    formats [ num_formats ] . extension = extension ;
    formats [ num_formats ] . loadfunc = loadfunc ;
    formats [ num_formats ] . savefunc = savefunc ;
    num_formats ++ ;
  }
  else
  {
    ulSetError ( UL_WARNING, "ssgAddModelFormat: too many formats" );
  }
}


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

  _ssgModelFormat *f = formats ;
  for ( int i=0; i<num_formats; i++, f++ )
  {
    if ( f->loadfunc != NULL &&
	       _ssgStrEqual ( extn, f->extension ) )
      return f->loadfunc( fname, options ) ;
  }

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

  _ssgModelFormat *f = formats ;
  for ( int i=0; i<num_formats; i++, f++ )
  {
    if ( f->savefunc != NULL &&
         _ssgStrEqual ( extn, f->extension ) )
      return f->savefunc( fname, ent ) ;
  }

  ulSetError ( UL_WARNING, "ssgSave: Unrecognised file type '%s'", extn ) ;
  return FALSE ;
}


char* _ssgMakePath( char* path, const char* dir, const char* fname )
{ // MakePath was originally in ssg, so we dont want to remove this function from ssg
	// But we need it in ul also
	return ulMakePath( path, dir, fname );
}
