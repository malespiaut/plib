
#include "ssgLocal.h" 

static int  read_error = FALSE ;
static int write_error = FALSE ;

int _ssgReadError  (void) { return  read_error ; }
int _ssgWriteError (void) { return write_error ; }

void _ssgReadFloat ( FILE *fd, float *var )
{
  if ( fread ( var, sizeof(float), 1, fd ) == 1 ) return ;
  read_error = TRUE ;
}


void _ssgWriteFloat ( FILE *fd, const float var )
{
  if ( fwrite ( & var, sizeof(float), 1, fd ) == 1 ) return ;
  write_error = TRUE ;
}


void _ssgReadUInt ( FILE *fd, unsigned int *var )
{
  if ( fread ( var, sizeof(unsigned int), 1, fd ) == 1 ) return ;
  read_error = TRUE ;
}


void _ssgWriteUInt ( FILE *fd, const unsigned int var )
{
  if ( fwrite ( & var, sizeof(unsigned int), 1, fd ) == 1 ) return ;
  write_error = TRUE ;
}


void _ssgReadInt ( FILE *fd, int *var )
{
  if ( fread ( var, sizeof(int), 1, fd ) == 1 ) return ;
  read_error = TRUE ;
}


void _ssgWriteInt ( FILE *fd, const int var )
{
  if ( fwrite ( & var, sizeof(int), 1, fd ) == 1 ) return ;
  write_error = TRUE ;
}


void _ssgReadUShort ( FILE *fd, unsigned short *var )
{
  if ( fread ( var, sizeof(unsigned short), 1, fd ) == 1 ) return ;
  read_error = TRUE ;
}


void _ssgWriteUShort ( FILE *fd, const unsigned short var )
{
  if ( fwrite ( & var, sizeof(unsigned short), 1, fd ) == 1 ) return ;
  write_error = TRUE ;
}


void _ssgReadShort ( FILE *fd, short *var )
{
  if ( fread ( var, sizeof(short), 1, fd ) == 1 ) return ;
  read_error = TRUE ;
}


void _ssgWriteShort ( FILE *fd, const short var )
{
  if ( fwrite ( & var, sizeof(short), 1, fd ) == 1 ) return ;
  write_error = TRUE ;
}


void _ssgReadFloat ( FILE *fd, const unsigned int n, float *var )
{
  if ( fread ( var, sizeof(float), n, fd ) == n ) return ;
  read_error = TRUE ;
}


void _ssgWriteFloat ( FILE *fd, const unsigned int n, const float *var )
{
  if ( fwrite ( var, sizeof(float), n, fd ) == n ) return ;
  write_error = TRUE ;
}


void _ssgReadUShort ( FILE *fd, const unsigned int n, unsigned short *var )
{
  if ( fread ( var, sizeof(unsigned short), n, fd ) == n ) return ;
  read_error = TRUE ;
}


void _ssgWriteUShort ( FILE *fd, const unsigned int n, const unsigned short *var )
{
  if ( fwrite ( var, sizeof(unsigned short), n, fd ) == n ) return ;
  write_error = TRUE ;
}



void _ssgReadShort ( FILE *fd, const unsigned int n, short *var )
{
  if ( fread ( var, sizeof(short), n, fd ) == n ) return ;
  read_error = TRUE ;
}


void _ssgWriteShort ( FILE *fd, const unsigned int n, const short *var )
{
  if ( fwrite ( var, sizeof(short), n, fd ) == n ) return ;
  write_error = TRUE ;
}


void _ssgReadUInt ( FILE *fd, const unsigned int n, unsigned int *var )
{
  if ( fread ( var, sizeof(unsigned int), n, fd ) == n ) return ;
  read_error = TRUE ;
}


void _ssgWriteUInt ( FILE *fd, const unsigned int n, const unsigned int *var )
{
  if ( fwrite ( var, sizeof(unsigned int), n, fd ) == n ) return ;
  write_error = TRUE ;
}



void _ssgReadInt ( FILE *fd, const unsigned int n, int *var )
{
  if ( fread ( var, sizeof(int), n, fd ) == n ) return ;
  read_error = TRUE ;
}


void _ssgWriteInt ( FILE *fd, const unsigned int n, const int *var )
{
  if ( fwrite ( var, sizeof(int), n, fd ) == n ) return ;
  write_error = TRUE ;
}



#define MAX_ENTITY_NAME_LENGTH 1024

void _ssgReadString ( FILE *fd, char **var )
{
  int i ;
  char s [ MAX_ENTITY_NAME_LENGTH ] ;

  for ( i = 0 ; i < MAX_ENTITY_NAME_LENGTH ; i++ )
  {
    int c = fgetc ( fd ) ;
    s [ i ] = c ;

    if ( c == '\0' )
      break ;
  }

  if ( i >= MAX_ENTITY_NAME_LENGTH-1 )
    s [ MAX_ENTITY_NAME_LENGTH-1 ] = '\0' ;


  if ( s[0] == '\0' )
    *var = NULL ;
  else
  {
    *var = new char [ strlen(s)+1 ] ;
    strcpy ( *var, s ) ;
  }
}


void _ssgWriteString ( FILE *fd, const char *var )
{
  if ( var != NULL )
    fputs ( var, fd ) ;

  fputc ( '\0', fd ) ;
}


void _ssgReadVec2  ( FILE *fd, sgVec2 var ) { _ssgReadFloat  ( fd, 2, var ) ; }
void _ssgWriteVec2 ( FILE *fd, const sgVec2 var ) { _ssgWriteFloat ( fd, 2, var ) ; }

void _ssgReadVec3  ( FILE *fd, sgVec3 var ) { _ssgReadFloat  ( fd, 3, var ) ; }
void _ssgWriteVec3 ( FILE *fd, const sgVec3 var ) { _ssgWriteFloat ( fd, 3, var ) ; }

void _ssgReadVec4  ( FILE *fd, sgVec4 var ) { _ssgReadFloat  ( fd, 4, var ) ; }
void _ssgWriteVec4 ( FILE *fd, const sgVec4 var ) { _ssgWriteFloat ( fd, 4, var ) ; }

void _ssgReadMat4  ( FILE *fd, sgMat4 var ) { _ssgReadFloat  ( fd, 16, (float *)var ) ; }
void _ssgWriteMat4 ( FILE *fd, const sgMat4 var ) { _ssgWriteFloat ( fd, 16, (float *)var ) ; }


/*
  I'm sick of half the machines on the planet supporting
  strncasecmp and the other half strnicmp - so here is my own
  offering:
*/

int _ssgStrNEqual ( char *s1, char *s2, int len )
{
  int l1 = (s1==NULL) ? 0 : strlen ( s1 ) ;
  int l2 = (s2==NULL) ? 0 : strlen ( s2 ) ;

  if ( l1 > len ) l1 = len ;

  if ( l2 < l1 )
    return FALSE ;

  for ( int i = 0 ; i < l1 ; i++ )
  {
    char c1 = s1[i] ;
    char c2 = s2[i] ;

    if ( c1 == c2 )
     continue ;

    if ( c1 >= 'a' && c1 <= 'z' )
    if ( c1 >= 'a' && c1 <= 'z' )
      c1 = c1 - ('a'-'A') ;

    if ( c2 >= 'a' && c2 <= 'z' )
      c2 = c2 - ('a'-'A') ;

    if ( c1 != c2 )
     return FALSE ;
  }

  return TRUE ;
}

ssgState *( *_ssgGetAppState)( char *) = NULL ;

void ssgSetAppStateCallback ( ssgState *(*cb)(char *) )
{
  _ssgGetAppState = cb ;
}


enum { MAX_SHARED_TEXTURES = 100, MAX_SHARED_STATES = 1000 };
static ssgTexture* shared_textures [ MAX_SHARED_TEXTURES ] ;
static ssgSimpleState* shared_states [ MAX_SHARED_STATES ] ;
static int num_shared_textures = 0 ;
static int num_shared_states = 0 ;

void _ssgShareReset ()
{
   num_shared_textures = 0 ;
   num_shared_states = 0 ;
}

GLuint _ssgShareTexture ( char* tfname )
{
  if ( tfname == NULL || tfname[0] == 0 )
     return 0 ;

  for ( int i = 0 ; i < num_shared_textures ; i++ )
  {
    ssgTexture *tex = shared_textures [ i ] ;
    if ( _ssgStrEqual ( tfname, tex->getFilename () ) )
      return tex -> getHandle () ;
  }

  ssgTexture* tex = new ssgTexture ( tfname ) ;
  if ( tex && num_shared_textures < MAX_SHARED_TEXTURES )
     shared_textures [ num_shared_textures++ ] = tex ;
  if ( tex )
    return tex -> getHandle () ;
  return 0 ;
}

ssgSimpleState* _ssgShareState ( ssgSimpleState* st )
{
  if ( st == NULL )
     return NULL ;

  for ( int i = 0 ; i < num_shared_states ; i++ )
  {
    ssgSimpleState *st2 = shared_states [ i ] ;

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

    delete st ;
    return st2 ;
  }

  if ( num_shared_states < MAX_SHARED_STATES )
     shared_states [ num_shared_states++ ] = st ;
  return st ;
}


ssgCreateData::ssgCreateData ()
{
  parentName = NULL ;

  gltype = GL_TRIANGLES ;
  vl = NULL ;
  nl = NULL ;
  tl = NULL ;
  cl = NULL ;
  il = NULL ;

  st = NULL ;
  tfname = NULL ;
  cull_face = TRUE ;
}

ssgCreateData::~ssgCreateData ()
{
  delete parentName ;
  delete vl ;
  delete nl ;
  delete tl ;
  delete cl ;
  delete il ;
  delete st ;
  delete tfname ;
}

ssgLeaf* _ssgCreateFunc ( ssgCreateData* data )
{
  ssgState *st = NULL ;
  ssgVtxTable *vtab = 0 ;

  if ( data )
  {
    if ( data -> il != NULL )
      vtab = new ssgVtxArray ( data->gltype,
        data->vl, data->nl, data->tl, data->cl, data->il ) ;
    else
      vtab = new ssgVtxTable ( data->gltype,
        data->vl, data->nl, data->tl, data->cl ) ;

    vtab -> setCullFace ( data -> cull_face ) ;

    /* do we have a global AppState function? */
    if ( _ssgGetAppState != NULL &&
         data -> tfname != NULL && data -> tfname[0] != 0 )
    {
      st = _ssgGetAppState ( data -> tfname ) ;

      if ( st != NULL )
      {
        delete data -> st ;
        data -> st = NULL ;
      }
    }
 
    if ( st == NULL && data -> st != NULL )
    {
      char filename [ 1024 ] ;
      _ssgMakePath ( filename, _ssgTexturePath, data->tfname ) ;

      GLuint texture_handle = _ssgShareTexture ( filename ) ;

      if ( texture_handle )
      {
        /* Don't change the order of these two statements! */
        data -> st -> setTexture         ( texture_handle ) ;
        data -> st -> setTextureFilename ( filename ) ;
 
        data -> st -> enable ( GL_TEXTURE_2D ) ;
      }
      else
        data -> st -> disable ( GL_TEXTURE_2D ) ;
 
/*
This allows the code to run...but
screws up the 'sharing' code.
Dunno what's wrong with it.
*/

      /* Replaced this... */
      /* st = _ssgShareState ( data -> st ) ; */
      /* With this... */
      st = data -> st ;
    }

    vtab -> setState ( st ) ;
  }
  else
  {
     _ssgShareReset () ;
  }

  return vtab ;
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


static char *file_extension ( char *fname )
{
  char *p = & ( fname [ strlen(fname) ] ) ;

  while ( p != fname && *p != '/' && *p != '.' )
    p-- ;

  return p ;
}


typedef ssgEntity *_ssgLoader ( char *, ssgHookFunc, ssgCreateFunc ) ;
typedef int         _ssgSaver ( char *, ssgEntity * ) ;

struct _ssgFileFormat
{
  char *extension ;
  _ssgLoader *loadfunc ;
  _ssgSaver  *savefunc ;
} ;


static _ssgFileFormat formats[] =
{
  { ".3ds", ssgLoad3ds , NULL       },
  { ".ac" , ssgLoadAC3D, ssgSaveAC  },
  { ".ase", ssgLoadASE , ssgSaveASE },
  { ".dxf", ssgLoadDXF , ssgSaveDXF },
  { ".obj", ssgLoadOBJ , ssgSaveOBJ },
  { ".ssg", ssgLoadSSG , ssgSaveSSG },
  { ".tri", ssgLoadTRI , ssgSaveTRI },
  { ".wrl", ssgLoadVRML, NULL       },
  { NULL  , NULL       , NULL       }
} ;

  
ssgEntity *ssgLoad ( char *fname, ssgHookFunc hookfunc, ssgCreateFunc createfunc )
{
  if ( fname == NULL || *fname == '\0' )
    return NULL ;

  char *extn = file_extension ( fname ) ;

  if ( *extn != '.' )
  {
    ulSetError ( UL_WARNING, "ssgLoad: Cannot determine file type for '%s'", fname );
    return NULL ;
  }

  for ( _ssgFileFormat *f = formats; f->extension != NULL; f++ )
    if ( f->loadfunc != NULL &&
         _ssgStrNEqual ( extn, f->extension, strlen(f->extension) ) )
      return f->loadfunc( fname, hookfunc, createfunc ) ;

  ulSetError ( UL_WARNING, "ssgLoad: Unrecognised file type '%s'", extn ) ;
  return NULL ;
}


int ssgSave ( char *fname, ssgEntity *ent )
{
  if ( fname == NULL || ent == NULL || *fname == '\0' )
    return FALSE ;

  char *extn = file_extension ( fname ) ;

  if ( *extn != '.' )
  {
    ulSetError ( UL_WARNING, "ssgSave: Cannot determine file type for '%s'", fname );
    return FALSE ;
  }

  for ( _ssgFileFormat *f = formats; f->extension != NULL; f++ )
    if ( f->savefunc != NULL &&
         _ssgStrNEqual ( extn, f->extension, strlen(f->extension) ) )
      return f->savefunc( fname, ent ) ;

  ulSetError ( UL_WARNING, "ssgSave: Unrecognised file type '%s'", extn ) ;
  return FALSE ;
}


char* _ssgMakePath( char* path, const char* dir, const char* fname )
{
  if ( fname )
  {
    if ( fname [ 0 ] != '\0' && fname [ 0 ] != '/' &&
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


/*
  I'm sick of half the machines on the planet supporting
  strcasecmp and the other half stricmp - so here is my own
  offering:
*/

int _ssgStrEqual ( char *s1, char *s2 )
{
  int l1 = (s1==NULL) ? 0 : strlen ( s1 ) ;
  int l2 = (s2==NULL) ? 0 : strlen ( s2 ) ;

  if ( l1 != l2 ) return FALSE ;

  for ( int i = 0 ; i < l1 ; i++ )
  {
    char c1 = s1[i] ;
    char c2 = s2[i] ;

    if ( c1 == c2 )
     continue ;

    if ( c1 >= 'a' && c1 <= 'z' )
      c1 = c1 - ('a'-'A') ;

    if ( c2 >= 'a' && c2 <= 'z' )
      c2 = c2 - ('a'-'A') ;

    if ( c1 != c2 )
     return FALSE ;
  }

  return TRUE ;
}
