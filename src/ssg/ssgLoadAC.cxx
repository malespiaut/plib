
#include "ssgLocal.h"

#define MAX_TEXTURES 1000    /* This *ought* to be enough! */

static FILE *loader_fd ;

static char *texture_fnames [ MAX_TEXTURES ] ;

struct _ssgMaterial
{
  sgVec4 spec ;
  sgVec4 emis ;
  sgVec4 rgb  ;
  float  shi  ;
} ;

static int num_materials = 0 ;
static int num_states    = 0 ;
static int num_textures  = 0 ;
static sgVec3 *vtab = NULL ;

static ssgHookFunc      current_hookFunc = NULL ;
static _ssgMaterial    *current_material = NULL ;
static sgVec4          *current_colour   = NULL ;
static ssgTexture      *current_texture  = NULL ;
static ssgBranch       *current_branch   = NULL ;
static char            *current_tfname   = NULL ;
static char            *current_data     = NULL ;

static _ssgMaterial   *mlist    [ MAX_TEXTURES ] ;
static ssgSimpleState *slist    [ MAX_TEXTURES ] ;
static sgVec4         *clist    [ MAX_TEXTURES ] ;
static ssgTexture     *tex_list [ MAX_TEXTURES ] ;

static sgMat4 current_matrix ;
static sgVec2 texrep ;
static sgVec2 texoff ;

int do_material ( char *s ) ;
int do_object   ( char *s ) ;
int do_name     ( char *s ) ;
int do_data     ( char *s ) ;
int do_texture  ( char *s ) ;
int do_texrep   ( char *s ) ;
int do_texoff   ( char *s ) ;
int do_rot      ( char *s ) ;
int do_loc      ( char *s ) ;
int do_url      ( char *s ) ;
int do_numvert  ( char *s ) ;
int do_numsurf  ( char *s ) ;
int do_surf     ( char *s ) ;
int do_mat      ( char *s ) ;
int do_refs     ( char *s ) ;
int do_kids     ( char *s ) ;

int do_obj_world ( char *s ) ;
int do_obj_poly  ( char *s ) ;
int do_obj_group ( char *s ) ;
int do_obj_light ( char *s ) ;

#define PARSE_CONT   0
#define PARSE_POP    1

struct Tag
{
  char *token ;
  int (*func) ( char *s ) ;
} ;

 
void skip_spaces ( char **s )
{
  while ( **s == ' ' || **s == '\t' )
    (*s)++ ;
}


void skip_quotes ( char **s )
{
  skip_spaces ( s ) ;

  if ( **s == '\"' )
  {
    (*s)++ ;

    char *t = *s ;

    while ( *t != '\0' && *t != '\"' )
      t++ ;

    if ( *t != '\"' )
      ulSetError ( UL_WARNING, "ac_to_gl: Mismatched double-quote ('\"') in '%s'", *s ) ;

    *t = '\0' ;
  }
  else
    ulSetError ( UL_WARNING, "ac_to_gl: Expected double-quote ('\"') in '%s'", *s ) ;
}



int search ( Tag *tags, char *s )
{
  skip_spaces ( & s ) ;

  for ( int i = 0 ; tags[i].token != NULL ; i++ )
    if ( _ssgStrNEqual ( tags[i].token, s, strlen(tags[i].token) ) )
    {
      s += strlen ( tags[i].token ) ;

      skip_spaces ( & s ) ;

      return (*(tags[i].func))( s ) ;
    }

  ulSetError ( UL_FATAL, "ac_to_gl: Unrecognised token '%s'", s ) ;

  return 0 ;  /* Should never get here */
}

Tag top_tags [] =
{
  { "MATERIAL", do_material },
  { "OBJECT"  , do_object   },
} ;


Tag object_tags [] =
{
  { "name"    , do_name     },
  { "data"    , do_data     },
  { "texture" , do_texture  },
  { "texrep"  , do_texrep   },
  { "texoff"  , do_texoff   },
  { "rot"     , do_rot      },
  { "loc"     , do_loc      },
  { "url"     , do_url      },
  { "numvert" , do_numvert  },
  { "numsurf" , do_numsurf  },
  { "kids"    , do_kids     },
  { NULL, NULL }
} ;

Tag surf_tag [] =
{
  { "SURF"    , do_surf     },
  { NULL, NULL }
} ;

Tag surface_tags [] =
{
  { "mat"     , do_mat      },
  { "refs"    , do_refs     },
  { NULL, NULL }
} ;

Tag obj_type_tags [] =
{
  { "world", do_obj_world },
  { "poly" , do_obj_poly  },
  { "group", do_obj_group },
  { "light", do_obj_light },
  { NULL, NULL }
} ;


#define OBJ_WORLD  0
#define OBJ_POLY   1
#define OBJ_GROUP  2
#define OBJ_LIGHT  3

int do_obj_world ( char * ) { return OBJ_WORLD ; } 
int do_obj_poly  ( char * ) { return OBJ_POLY  ; }
int do_obj_group ( char * ) { return OBJ_GROUP ; }
int do_obj_light ( char * ) { return OBJ_LIGHT ; }

int last_num_kids    = -1 ;
int current_flags    = -1 ;

static ssgSimpleState *find_state ( _ssgMaterial *mat,
                                     ssgTexture *tex, char * /*tfname*/ )
{
  for ( int i = 0 ; i < num_states ; i++ )
  {
    ssgSimpleState *st2 = slist [ i ] ;

    if ( tex == NULL && st2->isEnabled ( GL_TEXTURE_2D ) )
      continue ;

    if ( tex != NULL && ! st2->isEnabled ( GL_TEXTURE_2D ) )
      continue ;

    if ( tex != NULL && tex -> getHandle() != st2 -> getTextureHandle () )
      continue ;

    if ( ! sgEqualVec4 ( mat->emis, st2->getMaterial ( GL_EMISSION ) ) ||
         ! sgEqualVec4 ( mat->spec, st2->getMaterial ( GL_SPECULAR ) ) ||
         (int)( mat->rgb[3] < 0.99 ) != st2 -> isTranslucent () ||
         mat -> shi != st2->getShininess () )
      continue ;

    return st2 ;
  }

  ssgSimpleState *st = new ssgSimpleState () ;

  slist [ num_states++ ] = st ;

  if ( tex != NULL )
  {
    st -> setTexture ( tex ) ;
    st -> enable     ( GL_TEXTURE_2D ) ;
  }
  else
    st -> disable    ( GL_TEXTURE_2D ) ;

  st -> setMaterial ( GL_SPECULAR, mat -> spec ) ;
  st -> setMaterial ( GL_EMISSION, mat -> emis ) ;
  st -> setShininess ( mat -> shi ) ;

  st -> enable ( GL_COLOR_MATERIAL ) ;
  st -> setColourMaterial ( GL_AMBIENT_AND_DIFFUSE ) ;

  st -> enable  ( GL_LIGHTING       ) ;
  st -> setShadeModel ( GL_SMOOTH ) ;

  if ( mat -> rgb[3] < 0.99 )
  {
    st -> disable ( GL_ALPHA_TEST ) ;
    st -> enable  ( GL_BLEND ) ;
    st -> setTranslucent () ;
  }
  else
  {
    st -> disable ( GL_BLEND ) ;
    st -> setOpaque () ;
  }

  return st ;
}


int do_material ( char *s )
{
  char name [ 1024 ] ;
  sgVec4 rgb  ;
  sgVec4 amb  ;
  sgVec4 emis ;
  sgVec4 spec ;
  int   shi ;
  float trans ;

  if ( sscanf ( s,
  "%s rgb %f %f %f amb %f %f %f emis %f %f %f spec %f %f %f shi %d trans %f",
    name,
    &rgb [0], &rgb [1], &rgb [2],
    &amb [0], &amb [1], &amb [2],
    &emis[0], &emis[1], &emis[2],
    &spec[0], &spec[1], &spec[2],
    &shi,
    &trans ) != 15 )
  {
    ulSetError ( UL_WARNING, "ac_to_gl: Can't parse this MATERIAL:" ) ;
    ulSetError ( UL_WARNING, "ac_to_gl: MATERIAL %s", s ) ;
  }
  else
  {
    char *nm = name ;

    skip_quotes ( &nm ) ;

    amb [ 3 ] = emis [ 3 ] = spec [ 3 ] = 1.0f ;
    rgb [ 3 ] = 1.0f - trans ;

    mlist [ num_materials ] = new _ssgMaterial ;
    clist [ num_materials ] = new sgVec4 [ 1 ] ;

    sgCopyVec4 ( clist [ num_materials ][ 0 ], rgb ) ;

    current_material = mlist [ num_materials ] ;
    sgCopyVec4 ( current_material -> spec, spec ) ;
    sgCopyVec4 ( current_material -> emis, emis ) ;
    sgCopyVec4 ( current_material -> rgb , rgb  ) ;
    current_material -> shi = (float) shi ;
  }

  num_materials++ ;
  return PARSE_CONT ;
}


int do_object   ( char *  /* s */ )
{
/*
  int obj_type = search ( obj_type_tags, s ) ;  
*/

  char buffer [ 1024 ] ;

  sgSetVec2 ( texrep, 1.0f, 1.0f ) ;
  sgSetVec2 ( texoff, 0.0f, 0.0f ) ;

  sgMakeIdentMat4 ( current_matrix ) ;

  ssgEntity *old_cb = current_branch ;

  ssgTransform *tr = new ssgTransform () ;

  tr -> setTransform ( current_matrix ) ;

  current_branch -> addKid ( tr ) ;
  current_branch = tr ;

  while ( fgets ( buffer, 1024, loader_fd ) != NULL )
    if ( search ( object_tags, buffer ) == PARSE_POP )
      break ;

  int num_kids = last_num_kids ;

  for ( int i = 0 ; i < num_kids ; i++ )
  {
    fgets ( buffer, 1024, loader_fd ) ;
    search ( top_tags, buffer ) ;
  }

  current_branch = (ssgBranch *) old_cb ;
  return PARSE_CONT ;
}


int do_name ( char *s )
{
  skip_quotes ( &s ) ;

  current_branch -> setName ( s ) ;

  return PARSE_CONT ;
}


int do_data     ( char *s )
{
  int len = strtol ( s, NULL, 0 ) ;

  current_data = new char [ len + 1 ] ;

  for ( int i = 0 ; i < len ; i++ )
    current_data [ i ] = fgetc ( loader_fd ) ;

  current_data [ len ] = '\0' ;

  fgetc ( loader_fd ) ;  /* Final RETURN */

  if ( current_hookFunc != NULL )
  {
    ssgBranch *br = (*current_hookFunc) ( current_data ) ;

    if ( br != NULL )
    {
      current_branch -> addKid ( br ) ;
      current_branch = br ;
    }

    current_data = NULL ;
  }

  return PARSE_CONT ;
}


int get_texture ( char *s )
{
  char filename [ 1024 ] ;

  if ( s [ 0 ] != '/' && _ssgTexturePath != NULL &&
                         _ssgTexturePath[0] != '\0' )
  {
    strcpy ( filename, _ssgTexturePath ) ;
    strcat ( filename, "/" ) ;
    strcat ( filename, s ) ;
  }
  else
    strcpy ( filename, s ) ;

  for ( int i = 0 ; i < num_textures ; i++ )
    if ( strcmp ( filename, texture_fnames [ i ] ) == 0 )
    {
      current_tfname = texture_fnames [ i ] ;
      return i ;
    }

  texture_fnames [ num_textures ] = new char [ strlen ( filename ) + 1 ] ;
  strcpy ( texture_fnames [ num_textures ], filename ) ;
  current_tfname = texture_fnames [ num_textures ] ;

/*
  if ( _ssgGetAppState == NULL )
    tex_list [ num_textures ] = NULL ;
  else
*/
    tex_list [ num_textures ] = new ssgTexture ( filename ) ;

  return num_textures++ ;
}


int do_texture  ( char *s )
{
  skip_quotes ( &s ) ;

  current_texture = tex_list [ get_texture ( s ) ] ;

  return PARSE_CONT ;
}


int do_texrep ( char *s )
{
  if ( sscanf ( s, "%f %f", & texrep [ 0 ], & texrep [ 1 ] ) != 2 )
    ulSetError ( UL_WARNING, "ac_to_gl: Illegal texrep record." ) ;

  return PARSE_CONT ;
}


int do_texoff ( char *s )
{
  if ( sscanf ( s, "%f %f", & texoff [ 0 ], & texoff [ 1 ] ) != 2 )
    ulSetError ( UL_WARNING, "ac_to_gl: Illegal texoff record." ) ;

  return PARSE_CONT ;
}

int do_rot ( char *s )
{
  current_matrix [ 0 ][ 3 ] = current_matrix [ 1 ][ 3 ] = current_matrix [ 2 ][ 3 ] =
    current_matrix [ 3 ][ 0 ] = current_matrix [ 3 ][ 1 ] = current_matrix [ 3 ][ 2 ] = 0.0f ;
  current_matrix [ 3 ][ 3 ] = 1.0f ; 

  if ( sscanf ( s, "%f %f %f %f %f %f %f %f %f",
        & current_matrix [ 0 ] [ 0 ], & current_matrix [ 0 ] [ 1 ], & current_matrix [ 0 ] [ 2 ],
        & current_matrix [ 1 ] [ 0 ], & current_matrix [ 1 ] [ 1 ], & current_matrix [ 1 ] [ 2 ],
        & current_matrix [ 2 ] [ 0 ], & current_matrix [ 2 ] [ 1 ], & current_matrix [ 2 ] [ 2 ] ) != 9 )
    ulSetError ( UL_WARNING, "ac_to_gl: Illegal rot record." ) ;

  ((ssgTransform *)current_branch) -> setTransform ( current_matrix ) ;
  return PARSE_CONT ;
}

int do_loc      ( char *s )
{
  if ( sscanf ( s, "%f %f %f", & current_matrix [ 3 ][ 0 ], & current_matrix [ 3 ][ 2 ], & current_matrix [ 3 ][ 1 ] ) != 3 )
    ulSetError ( UL_WARNING, "ac_to_gl: Illegal loc record." ) ;

  current_matrix [ 3 ][ 1 ] = - current_matrix [ 3 ][ 1 ] ;
  current_matrix [ 3 ][ 3 ] = 1.0f ;
  ((ssgTransform *)current_branch) -> setTransform ( current_matrix ) ;

  return PARSE_CONT ;
}

int do_url      ( char *s )
{
  skip_quotes ( & s ) ;

#ifdef PRINT_URLS
  printf ( "/* URL: \"%s\" */\n", s ) ;
#endif

  return PARSE_CONT ;
}

int do_numvert  ( char *s )
{
  char buffer [ 1024 ] ;

  int nv = strtol ( s, NULL, 0 ) ;
 
  delete [] vtab ;

  vtab = new sgVec3 [ nv ] ;

  for ( int i = 0 ; i < nv ; i++ )
  {
    fgets ( buffer, 1024, loader_fd ) ;

    if ( sscanf ( buffer, "%f %f %f",
                          &vtab[i][0], &vtab[i][1], &vtab[i][2] ) != 3 )
    {
      ulSetError ( UL_FATAL, "ac_to_gl: Illegal vertex record." ) ;
    }

    float tmp  =  vtab[i][1] ;
    vtab[i][1] = -vtab[i][2] ;
    vtab[i][2] = tmp ;
  }

  return PARSE_CONT ;
}

int do_numsurf  ( char *s )
{
  int ns = strtol ( s, NULL, 0 ) ;

  for ( int i = 0 ; i < ns ; i++ )
  {
    char buffer [ 1024 ] ;

    fgets ( buffer, 1024, loader_fd ) ;
    search ( surf_tag, buffer ) ;
  }

  return PARSE_CONT ;
}

int do_surf     ( char *s )
{
  current_flags = strtol ( s, NULL, 0 ) ;

  char buffer [ 1024 ] ;

  while ( fgets ( buffer, 1024, loader_fd ) != NULL )
    if ( search ( surface_tags, buffer ) == PARSE_POP )
      break ;

  return PARSE_CONT ;
}


int do_mat ( char *s )
{
  int mat = strtol ( s, NULL, 0 ) ;

  current_material = mlist [ mat ] ;
  current_colour   = clist [ mat ] ;

  return PARSE_CONT ;
}


int do_refs     ( char *s )
{
  int nrefs = strtol ( s, NULL, 0 ) ;
  char buffer [ 1024 ] ;

  if ( nrefs == 0 )
    return PARSE_POP ;

  ssgVertexArray   *vlist = new ssgVertexArray ( nrefs ) ;
  ssgTexCoordArray *tlist = new ssgTexCoordArray ( nrefs ) ;
 
  for ( int i = 0 ; i < nrefs ; i++ )
  {
    fgets ( buffer, 1024, loader_fd ) ;

    int vtx ;
    sgVec2 tc ;

    if ( sscanf ( buffer, "%d %f %f", &vtx,
                                      &tc[0],
                                      &tc[1] ) != 3 )
    {
      ulSetError ( UL_FATAL, "ac_to_gl: Illegal ref record." ) ;
    }

    tc[0] *= texrep[0] ;
    tc[1] *= texrep[1] ;
    tc[0] += texoff[0] ;
    tc[1] += texoff[1] ;

    tlist -> add ( tc ) ;
    vlist -> add ( vtab[vtx] ) ;
  }

  if ( nrefs < 3 )
  {
    delete vlist ;
    delete tlist ;
    return PARSE_POP ;
  }

  ssgNormalArray *nrm = new ssgNormalArray ( 1 ) ;
  ssgColourArray *col = new ssgColourArray ( 1 ) ;

  col -> add ( *current_colour ) ;

  sgVec3 nm ;
  sgMakeNormal ( nm, vlist->get(0), vlist->get(1), vlist->get(2) ) ;

  nrm -> add ( nm ) ;

  ssgVtxTable *vl ;

  switch ( current_flags & 0x0F )
  {
    case 0 : vl = new ssgVtxTable ( GL_TRIANGLE_FAN,
                   vlist, nrm, tlist, col ) ; 
             break ;
    case 1 : vl = new ssgVtxTable ( GL_LINE_LOOP,
                   vlist, nrm, tlist, col ) ; 
             break ;
    case 2 : vl = new ssgVtxTable ( GL_LINE_STRIP,
                   vlist, nrm, tlist, col ) ; 
             break ;
    default : vl = NULL ; break ;
  }

  vl -> setCullFace ( ! ( (current_flags>>4) & 0x02 ) ) ;

  ssgState *st = NULL ;

  if ( _ssgGetAppState != NULL && current_tfname != NULL )
    st =_ssgGetAppState ( current_tfname ) ;

  if ( st != NULL )
    vl -> setState ( st ) ;
  else
    vl -> setState ( find_state ( current_material,
                 current_texture, current_tfname ) ) ;

  current_branch -> addKid ( vl ) ;
  return PARSE_POP ;
}

int do_kids ( char *s )
{
  last_num_kids = strtol ( s, NULL, 0 ) ;

  return PARSE_POP ;
}


ssgEntity *ssgLoadAC3D ( char *fname, ssgHookFunc hookfunc )
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

/*
  Original function for backwards compatibility...
*/

ssgEntity *ssgLoadAC ( char *fname, ssgHookFunc hookfunc )
{
  current_hookFunc = hookfunc ;

  char filename [ 1024 ] ;

  if ( fname [ 0 ] != '/' &&
       _ssgModelPath != NULL &&
       _ssgModelPath [ 0 ] != '\0' )
  {
    strcpy ( filename, _ssgModelPath ) ;
    strcat ( filename, "/" ) ;
    strcat ( filename, fname ) ;
  }
  else
    strcpy ( filename, fname ) ;

  num_textures  = 0 ;
  num_materials = 0 ;
  num_states    = 0 ;
  vtab = NULL ;

  current_material = NULL ;
  current_colour   = NULL ;
  current_texture  = NULL ;
  current_tfname   = NULL ;
  current_branch   = NULL ;

  sgSetVec2 ( texrep, 1.0, 1.0 ) ;
  sgSetVec2 ( texoff, 0.0, 0.0 ) ;

  loader_fd = fopen ( filename, "ra" ) ;

  if ( loader_fd == NULL )
  {
    ulSetError ( UL_WARNING, "ssgLoadAC: Failed to open '%s' for reading", filename ) ;
    return NULL ;
  }

  char buffer [ 1024 ] ;
  int firsttime = TRUE ;

  current_branch = new ssgTransform () ;

  while ( fgets ( buffer, 1024, loader_fd ) != NULL )
  {
    char *s = buffer ;

    /* Skip leading whitespace */

    skip_spaces ( & s ) ;

    /* Skip blank lines and comments */

    if ( *s < ' ' && *s != '\t' ) continue ;
    if ( *s == '#' || *s == ';' ) continue ;

    if ( firsttime )
    {
      firsttime = FALSE ;

      if ( ! _ssgStrNEqual ( s, "AC3D", 4 ) )
      {
        fclose ( loader_fd ) ;
        ulSetError ( UL_WARNING, "ssgLoadAC: '%s' is not in AC3D format.", filename ) ;
        return NULL ;
      }
    }
    else
      search ( top_tags, s ) ;
  }

  for ( int i = 0 ; i < num_textures ; i++ )
    delete texture_fnames [ i ] ;

  delete [] vtab ;
  fclose ( loader_fd ) ;

  return current_branch ;
}

