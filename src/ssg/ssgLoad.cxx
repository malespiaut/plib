
#include "ssgLocal.h" 


static ssgLoaderOptions default_options ;

ssgLoaderOptions *_ssgCurrentOptions = &default_options ;


#ifdef FOR_PPE



static char _ssgAPOM[16*1024]=""; // APOM = actual path of model (!=ModelPath)

char * ssgGetAPOM() // get actual path of (last loaded) model
{ return _ssgAPOM;
}


#if !defined(WIN32) || defined(__CYGWIN__)

static void appendPath(char *a, char *b)
// appends b to a. a has to be long enough to hold the result.
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
  
static void savePathInAPOM(const char * pathAndFileName )
{
	// save path in _ssgAPOM (actual path of model):

  // two different algorithms are used:
	// On Windo$, _fullpath is used to get the absolute path.
	// This is not possible under Unix.
#if defined(WIN32) && !defined(__CYGWIN__)

	char *path_only = new char [ strlen(pathAndFileName) + 1 ];
	strcpy( path_only, pathAndFileName );
	char *s_ptr;
	s_ptr = &(path_only[strlen(path_only)-1]);
	while ((s_ptr > path_only) && (*s_ptr != '/') && (*s_ptr != '\\')) 
		s_ptr--;
	if ( s_ptr >= path_only ) *s_ptr = 0;
	char buffer[ _MAX_PATH ];
  if ( NULL != _fullpath( buffer, path_only, _MAX_PATH ))
		strncpy( _ssgAPOM, buffer, 1024);

#else

	// This code accumulates all the relative pathes that make up
	// _ssgAPOM. This code may not be very elegant, but should be 
	// portable and avoids the problems with creating absolute paths on Unix.
	// For more, see mailing list.
	
	char buffer[16*1024];
	strncpy( buffer, pathAndFileName, 16*1024);
	char *s_ptr;
	s_ptr = &(buffer[strlen(buffer)-1]);
	while ((s_ptr > buffer) && (*s_ptr != '/') && (*s_ptr != '\\')) 
		s_ptr--;
	if ( s_ptr >= buffer ) *s_ptr = 0;
	appendPath(_ssgAPOM, buffer);
#endif
}

void appLoaderOptions::makeModelPath ( char *path, const char *fname ) const
{
  savePathInAPOM( fname );
  make_path ( path, model_dir, fname ) ;
}


void appLoaderOptions::makeTexturePath ( char *path, const char *fname ) const
{
	ulFindFile( path, texture_dir, fname, ssgGetAPOM() ) ;
}

// end of PPE stuff
#endif


char* ssgLoaderOptions::make_path ( char* path,
      const char* dir, const char* fname ) const
{
  if ( fname != NULL && fname [ 0 ] != '\0' )
  {
    if ( fname [ 0 ] != '/' &&
       dir != NULL && dir[0] != '\0' )
    {
      strcpy ( path, dir ) ;
      strcat ( path, "/" ) ;
      strcat ( path, fname ) ;
    }
    else
      strcpy ( path, fname ) ;

    //convert backward slashes to forward slashes
    for ( char* ptr = path ; *ptr ; ptr ++ )
    {
      if ( *ptr == '\\' )
        *ptr = '/' ;
    }
  }
  else
     path [0] = 0 ;
  return( path );
}


void ssgLoaderOptions::makeModelPath ( char *path, const char *fname ) const
{
  make_path ( path, model_dir, fname ) ;
}


void ssgLoaderOptions::makeTexturePath ( char *path, const char *fname ) const
{
  /* Remove all leading path information. */
  const char* seps = "\\/" ;
  const char* fn = & fname [ strlen ( fname ) - 1 ] ;
  for ( ; fn != fname && strchr(seps,*fn) == NULL ; fn-- )
    /* Search back for a seperator */ ;
  if ( strchr(seps,*fn) != NULL )
    fn++ ;
  fname = fn ;

  make_path ( path, texture_dir, fname ) ;
}


ssgLeaf* ssgLoaderOptions::createLeaf ( ssgLeaf* leaf,
                                        const char* parent_name )
{
  
  /* is this just a sharing 'reset' */
  if ( leaf == NULL )
  {
    shared_textures.removeAll () ;
    shared_states.removeAll () ;
    return NULL ;
  }
  
  /* try to do some state sharing */
  ssgState* st = leaf -> getState () ;
  if ( st != NULL && st -> isAKindOf ( SSG_TYPE_SIMPLESTATE ) )
  {
    ssgSimpleState *ss = (ssgSimpleState*) st ;
    ssgSimpleState *match = shared_states.findMatch ( ss ) ;
    if ( match != NULL )
      leaf -> setState ( match ) ;
    else
      shared_states.add ( ss ) ;
  }

  return leaf ;
}

ssgTexture* ssgLoaderOptions::createTexture ( char* tfname,
						     int wrapu,
						     int wrapv,
						     int mipmap ) 
{
  
  char filename [ 1024 ] ;
  makeTexturePath ( filename, tfname ) ;

  ssgTexture *tex = shared_textures.findByFilename ( filename ) ;
  if ( tex )
    return tex ;
  
  tex = new ssgTexture ( filename, wrapu, wrapv, mipmap ) ;
  if ( tex )
    shared_textures.add ( tex ) ;
  return tex ;
}

ssgTransform* ssgLoaderOptions::createTransform ( ssgTransform* tr,
      ssgTransformArray* ta ) const
{
  if ( ta != NULL )
    tr -> setUserData ( ta ) ;
  return tr ;
}


void ssgLoaderOptions::setModelDir ( const char *s )
{
  delete model_dir ;
  model_dir = new char [ strlen ( s ) + 1 ] ;
  strcpy ( model_dir, s ) ;
}

void ssgLoaderOptions::setTextureDir ( const char *s )
{
  delete texture_dir ;
  texture_dir = new char [ strlen ( s ) + 1 ] ;
  strcpy ( texture_dir, s ) ;
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



ssgEntity *ssgLoad ( const char *fname, const ssgLoaderOptions* options )
{
  if ( fname == NULL || *fname == '\0' )
    return NULL ;

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
	       ulStrEqual ( extn, f->extension ) )
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
         ulStrEqual ( extn, f->extension ) )
      return f->savefunc( fname, ent ) ;
  }

  ulSetError ( UL_WARNING, "ssgSave: Unrecognised file type '%s'", extn ) ;
  return FALSE ;
}
