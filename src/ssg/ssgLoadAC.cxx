/*
     PLIB - A Suite of Portable Game Libraries
     Copyright (C) 1998,2002  Steve Baker
 
     This library is free software; you can redistribute it and/or
     modify it under the terms of the GNU Library General Public
     License as published by the Free Software Foundation; either
     version 2 of the License, or (at your option) any later version.
 
     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Library General Public License for more details.
 
     You should have received a copy of the GNU Library General Public
     License along with this library; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 
     For further information visit http://plib.sourceforge.net

     $Id$
*/


#include "ssgLocal.h"
#include "ssgVertSplitter.h"

static FILE *loader_fd ;

struct _ssgMaterial
{
  sgVec4 rgba ;
  sgVec4 spec ;
  sgVec4 emis ;
  sgVec4 amb  ;
  float  shi  ;
} ;

static int num_materials = 0 ;

static ssgLoaderOptions *current_options        = NULL ;
static int               current_materialind    = 0 ;
static ssgBranch        *current_branch         = NULL ;
static ssgVertexArray   *current_vertexarray    = NULL ;
static ssgTexCoordArray *current_texcoordarray  = NULL ;
static ssgIndexArray    *current_triindexarray  = NULL ;
static ssgIndexArray    *current_matindexarray  = NULL ;
static ssgIndexArray    *current_flagsarray     = NULL ;
static char             *current_tfname         = NULL ;
static char             *current_data           = NULL ;
static float             current_crease         = 61.0 ;

#define MAX_MATERIALS 1000    /* This *ought* to be enough! */
static _ssgMaterial   *mlist    [ MAX_MATERIALS ] ;

static sgMat4 current_matrix ;
static sgVec2 texrep ;
static sgVec2 texoff ;

static sgVec2 invalidTexture = { 1e30f, } ;
static sgVec3 zero = { 0.0, } ;

static int do_material ( char *s ) ;
static int do_object   ( char *s ) ;
static int do_name     ( char *s ) ;
static int do_data     ( char *s ) ;
static int do_texture  ( char *s ) ;
static int do_texrep   ( char *s ) ;
static int do_texoff   ( char *s ) ;
static int do_crease   ( char *s ) ;
static int do_rot      ( char *s ) ;
static int do_loc      ( char *s ) ;
static int do_url      ( char *s ) ;
static int do_numvert  ( char *s ) ;
static int do_numsurf  ( char *s ) ;
static int do_surf     ( char *s ) ;
static int do_mat      ( char *s ) ;
static int do_refs     ( char *s ) ;
static int do_kids     ( char *s ) ;

/*static int do_obj_world ( char *s ) ;
static int do_obj_poly  ( char *s ) ;
static int do_obj_group ( char *s ) ;
static int do_obj_light ( char *s ) ;*/

#define PARSE_CONT   0
#define PARSE_POP    1

struct Tag
{
  const char *token ;
  int (*func) ( char *s ) ;
} ;

 
static void skip_spaces ( char **s )
{
  while ( **s == ' ' || **s == '\t' )
    (*s)++ ;
}


static void skip_quotes ( char **s )
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



static int search ( Tag *tags, char *s )
{
  skip_spaces ( & s ) ;

  for ( int i = 0 ; tags[i].token != NULL ; i++ )
    if ( ulStrNEqual ( tags[i].token, s, strlen(tags[i].token) ) )
    {
      s += strlen ( tags[i].token ) ;

      skip_spaces ( & s ) ;

      return (*(tags[i].func))( s ) ;
    }

  ulSetError ( UL_FATAL, "ac_to_gl: Unrecognised token '%s'", s ) ;

  return 0 ;  /* Should never get here */
}

static Tag top_tags [] =
{
  { "MATERIAL", do_material },
  { "OBJECT"  , do_object   },
  { NULL, NULL }
} ;


static Tag object_tags [] =
{
  { "name"    , do_name     },
  { "data"    , do_data     },
  { "texture" , do_texture  },
  { "texrep"  , do_texrep   },
  { "texoff"  , do_texoff   },
  { "crease"  , do_crease   },
  { "rot"     , do_rot      },
  { "loc"     , do_loc      },
  { "url"     , do_url      },
  { "numvert" , do_numvert  },
  { "numsurf" , do_numsurf  },
  { "kids"    , do_kids     },
  { NULL, NULL }
} ;

static Tag surf_tag [] =
{
  { "SURF"    , do_surf     },
  { NULL, NULL }
} ;

static Tag surface_tags [] =
{
  { "mat"     , do_mat      },
  { "refs"    , do_refs     },
  { NULL, NULL }
} ;

/*static Tag obj_type_tags [] =
{
  { "world", do_obj_world },
  { "poly" , do_obj_poly  },
  { "group", do_obj_group },
  { "light", do_obj_light },
  { NULL, NULL }
} ;*/


#define OBJ_WORLD  0
#define OBJ_POLY   1
#define OBJ_GROUP  2
#define OBJ_LIGHT  3

/*static int do_obj_world ( char * ) { return OBJ_WORLD ; } 
static int do_obj_poly  ( char * ) { return OBJ_POLY  ; }
static int do_obj_group ( char * ) { return OBJ_GROUP ; }
static int do_obj_light ( char * ) { return OBJ_LIGHT ; }*/

static int last_num_kids    = -1 ;
static int current_flags    = -1 ;

static ssgState *get_state ( _ssgMaterial *mat )
{
  if (current_tfname != NULL) {
    ssgState *st = current_options -> createState ( current_tfname ) ;
    if ( st != NULL )
      return st ;
  }

  ssgSimpleState *st = new ssgSimpleState () ;

  st -> setMaterial ( GL_DIFFUSE,  mat -> rgba ) ;
  st -> setMaterial ( GL_AMBIENT,  mat -> amb ) ;
  st -> setMaterial ( GL_EMISSION, mat -> emis ) ;
  st -> setMaterial ( GL_SPECULAR, mat -> spec ) ;
  st -> setShininess ( mat -> shi ) ;

  st -> enable ( GL_COLOR_MATERIAL ) ;
  st -> setColourMaterial ( GL_AMBIENT_AND_DIFFUSE ) ;

  st -> enable  ( GL_LIGHTING ) ;
  st -> setShadeModel ( GL_SMOOTH ) ;

  bool has_alpha = false ;

  if ( current_tfname != NULL )
  {
    ssgTexture *tex = current_options -> createTexture ( current_tfname ) ;
    has_alpha = tex -> hasAlpha () ;
    st -> setTexture( tex ) ;
    st -> enable( GL_TEXTURE_2D ) ;
  }
  else
  {
    st -> disable( GL_TEXTURE_2D ) ;
  }

  if ( mat -> rgba[3] < 0.99 || has_alpha )
  {
    st -> disable ( GL_ALPHA_TEST ) ;
    st -> enable  ( GL_BLEND ) ;
    st -> setTranslucent () ;
  }
  else
  {
    st -> disable ( GL_ALPHA_TEST ) ;
    st -> disable ( GL_BLEND ) ;
    st -> setOpaque () ;
  }

  return st ;
}


static int do_material ( char *s )
{
  char name [ 1024 ] ;
  sgVec4 rgba ;
  sgVec4 amb  ;
  sgVec4 emis ;
  sgVec4 spec ;
  int   shi ;
  float trans ;

  if ( sscanf ( s,
  "%s rgb %f %f %f amb %f %f %f emis %f %f %f spec %f %f %f shi %d trans %f",
    name,
    &rgba [0], &rgba [1], &rgba [2],
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
    rgba [ 3 ] = 1.0f - trans ;

    mlist [ num_materials ] = new _ssgMaterial ;
    _ssgMaterial  *current_material ;
    current_material = mlist [ num_materials ] ;
    sgCopyVec4 ( current_material -> rgba , rgba  ) ;
    sgCopyVec4 ( current_material -> amb  , amb   ) ;
    sgCopyVec4 ( current_material -> emis , emis  ) ;
    sgCopyVec4 ( current_material -> spec , spec  ) ;
    current_material -> shi = (float) shi ;
  }

  num_materials++ ;
  return PARSE_CONT ;
}

static int do_object   ( char *  /* s */ )
{
/*
  int obj_type = search ( obj_type_tags, s ) ;  
*/

  delete [] current_tfname ;
  current_tfname = NULL ;

  char buffer [ 1024 ] ;

  sgSetVec2 ( texrep, 1.0f, 1.0f ) ;
  sgSetVec2 ( texoff, 0.0f, 0.0f ) ;

  sgMakeIdentMat4 ( current_matrix ) ;

  ssgEntity *old_cb = current_branch ;

  ssgTransform *tr = new ssgTransform () ;

  tr -> setTransform ( current_matrix ) ;

  current_branch -> addKid ( tr ) ;
  current_branch = tr ;

  current_matindexarray = new ssgIndexArray ;
  current_flagsarray    = new ssgIndexArray ;
  current_texcoordarray = new ssgTexCoordArray ;
  current_vertexarray   = new ssgVertexArray ;
  current_triindexarray = new ssgIndexArray ;

  while ( fgets ( buffer, 1024, loader_fd ) != NULL )
    if ( search ( object_tags, buffer ) == PARSE_POP )
      break ;

  // If he have read some surfaces belonging to that object,
  // compute normals and distribute the triangels across the materials.
  int ntris = current_triindexarray -> getNum () / 3;
  if ( 0 < ntris ) {
    int i;
    int nvert = current_vertexarray -> getNum () ;

    // Put the triangles and the current crease angle into the vertex splitter ...
    ssgVertSplitter split( nvert, ntris );
    split.setSharpAngle ( current_crease );
    
    for ( i = 0 ; i < nvert ; i++ )
      sgCopyVec3 ( split.vert ( i ), current_vertexarray -> get ( i ) ) ;
    for ( i = 0 ; i < ntris ; i++ )
      split.setTri ( i,
                     * ( current_triindexarray -> get ( 3*i ) ),
                     * ( current_triindexarray -> get ( 3*i+1 ) ),
                     * ( current_triindexarray -> get ( 3*i+2 ) ) );

    // ... and let it compute the normals.
    split.splitAndCalcNormals () ;

    // Cycle through all ssgState/colour combinations and emit leafs from them.
    for ( int cullface = 0 ; cullface < 2 ; cullface++ ) {
      for ( int material = 0 ; material < num_materials ; material++ ) {
        ssgVertexArray* vertexarray = (ssgVertexArray *) current_vertexarray -> clone () ;
        vertexarray -> ref () ;
        ssgNormalArray* normalarray = new ssgNormalArray ( nvert ) ;
        normalarray -> ref () ;
        ssgTexCoordArray* texcoordarray = (ssgTexCoordArray *) current_texcoordarray -> clone () ;
        texcoordarray -> ref () ;
        ssgIndexArray* triindexarray = new ssgIndexArray ( nvert ) ;
        triindexarray -> ref () ;
        // Mark all normals as unset using a zero vector. 
        for ( i = 0 ; i < nvert ; i++ )
          normalarray -> add ( zero ) ;
        
        bool has_texture = current_tfname != NULL ;

        // For every material/state cycle through all triangles and pick
        // out those ones with that maternial/state.
        for ( int tri = 0 ; tri < ntris ; tri++ ) {
          int triMatindex = * ( current_matindexarray -> get ( tri ) ) ;
          int triFlags = * ( current_flagsarray -> get ( tri ) ) ;
          int triCullface = ! ( triFlags & 0x20 ) ;

          if ( triMatindex == material && triCullface == cullface ) {
            // get the original indices
            int* triind = split.getTri ( tri ) ;
            int origtriind[3];
            for ( int k = 0 ; k < 3 ; k++ )
              origtriind[k] = triind[k] < nvert ? triind[k] : split.origVert( triind[k] - nvert ) ;

            // Note that copying those values prevents us from a race condion
            // which occurs when doing something like:
            // texcoordarray -> add ( texcoordarray -> get ( origtriind[i] ) ) ;
          
            // make a local copy of texcoords ...
            sgVec2 texcoords[3];
            for ( i = 0 ; i < 3 ; i++ )
              sgCopyVec2 ( texcoords[i], texcoordarray -> get ( origtriind[i] ) ) ;

            // ... of vertices ...
            sgVec3 vertices[3];
            for ( i = 0 ; i < 3 ; i++ )
              sgCopyVec3 ( vertices[i], split.vert ( origtriind[i] ) ) ;

            int triSmooth = ( triFlags & 0x10 ) ;
            
            // ... and of normals.
            sgVec3 normals[3];
            if ( triSmooth ) {
              for ( i = 0 ; i < 3 ; i++ )
                sgCopyVec3 ( normals[i], split.norm ( triind[i] ) ) ;
            } else {
              // If we have flat shading, compute one normal for all.
              sgSubVec3 ( normals[1], vertices[1], vertices[0] ) ;
              sgSubVec3 ( normals[2], vertices[2], vertices[0] ) ;
              sgVectorProductVec3 ( normals[0], normals[1], normals[2] ) ;
              sgNormaliseVec3 ( normals[0] ) ;
              sgCopyVec3 ( normals[1], normals[0] ) ;
              sgCopyVec3 ( normals[2], normals[0] ) ;
            }
                      

            for ( i = 0 ; i < 3 ; i++ ) {
              
              if ( sgEqualVec3 ( normalarray -> get ( origtriind[i] ), zero ) ) {
                // Case: not yet initialized. 
                
                sgCopyVec3 ( normalarray -> get ( origtriind[i] ), normals[i] ) ;
                
                triindexarray -> add ( origtriind[i] ) ;
                
              } else if ( sgEqualVec3 ( normalarray -> get ( origtriind[i] ), normals[i] ) ) {
                // Case: initialized and the same as before. 
                
                triindexarray -> add ( origtriind[i] ) ;
                
              } else {
                // Case: initialized and different.
                
                // Scan for a vertex/normal/texcoord triple matching the required one.
                // If there exist one, use that.
                int num = vertexarray -> getNum () ;
                int replind = -1 ;
                for ( int l = nvert ; l < num ; l++ ) {
                  if ( sgEqualVec3( vertices[i], vertexarray -> get ( l ) ) &&
                       ( !has_texture || sgEqualVec2( texcoordarray -> get ( origtriind[i] ), texcoordarray -> get ( l ) ) ) &&
                       sgEqualVec3( normals[i], normalarray -> get ( l ) ) ) {
                    replind = l ;
                  }
                }
                
                // If we have not yet the required triple in our dataset, add it.
                if ( replind < 0 ) {
                  vertexarray -> add ( vertices[i] ) ;
                  normalarray -> add ( normals[i] ) ;
                  texcoordarray -> add ( texcoords[i] ) ;
                  replind = num ;
                }
                triindexarray -> add ( replind ) ;
              }
            }
          }
        }
        
        if ( 0 < triindexarray -> getNum () ) {
          ssgColourArray *colour = new ssgColourArray ( 1 ) ;
          colour -> add ( mlist [ material ] -> rgba ) ;
          ssgVtxArray* v = new ssgVtxArray ( GL_TRIANGLES,
                                             vertexarray,
                                             normalarray,
                                             has_texture ? texcoordarray : 0,
                                             colour,
                                             triindexarray ) ;
          v -> removeUnusedVertices();
          v -> setState ( get_state ( mlist [ material ] ) ) ;
          v -> setCullFace ( cullface ) ;
          ssgLeaf* leaf = current_options -> createLeaf ( v, 0 ) ;
          if ( leaf )
            tr -> addKid ( leaf ) ;
        }
        
        ssgDeRefDelete ( vertexarray ) ;
        ssgDeRefDelete ( normalarray ) ;
        ssgDeRefDelete ( texcoordarray ) ;
        ssgDeRefDelete ( triindexarray ) ;
      }
    }
  }

  // Cleanup
  delete current_matindexarray ;
  current_matindexarray = NULL ;
  delete current_flagsarray ;
  current_flagsarray    = NULL ;
  delete current_vertexarray ;
  current_vertexarray   = NULL ;
  delete current_texcoordarray ;
  current_texcoordarray = NULL ;
  delete current_triindexarray ;
  current_triindexarray = NULL ;

  // Process child nodes

  int num_kids = last_num_kids ;

  for ( int i = 0 ; i < num_kids ; i++ )
  {
    fgets ( buffer, 1024, loader_fd ) ;
    search ( top_tags, buffer ) ;
  }

  current_branch = (ssgBranch *) old_cb ;

  return PARSE_CONT ;
}


static int do_name ( char *s )
{
  skip_quotes ( &s ) ;

  current_branch -> setName ( s ) ;

  return PARSE_CONT ;
}


static int do_data ( char *s )
{
  int len = strtol ( s, NULL, 0 ) ;

  current_data = new char [ len + 1 ] ;

  for ( int i = 0 ; i < len ; i++ )
    current_data [ i ] = getc ( loader_fd ) ;

  current_data [ len ] = '\0' ;

  int c;
  while ( ( c = getc( loader_fd ) ) != EOF )  /* Final RETURN */
    if ( c != '\r' && c != '\n' ) {
      ungetc ( c, loader_fd ) ;
      break ;
    }

  ssgBranch *br = current_options -> createBranch ( current_data ) ;

  if ( br != NULL )
  {
    current_branch -> addKid ( br ) ;
    current_branch = br ;
  }

  /* delete [] current_data ; */
  current_data = NULL ;

  return PARSE_CONT ;
}


static int do_texture  ( char *s )
{
  skip_quotes ( &s ) ;

  delete [] current_tfname ;

  if ( s == NULL || s[0] == '\0' )
    current_tfname = NULL ;
  else
    current_tfname = ulStrDup ( s ) ;

  return PARSE_CONT ;
}


static int do_texrep ( char *s )
{
  if ( sscanf ( s, "%f %f", & texrep [ 0 ], & texrep [ 1 ] ) != 2 )
    ulSetError ( UL_WARNING, "ac_to_gl: Illegal texrep record." ) ;

  return PARSE_CONT ;
}


static int do_texoff ( char *s )
{
  if ( sscanf ( s, "%f %f", & texoff [ 0 ], & texoff [ 1 ] ) != 2 )
    ulSetError ( UL_WARNING, "ac_to_gl: Illegal texoff record." ) ;

  return PARSE_CONT ;
}

static int do_crease ( char *s )
{
  // the crease angle is not yet used. However, reading the crease line correctly means 
  // *.ac lines with "crease" can now be read.
  if ( sscanf ( s, "%f", & current_crease ) != 1 )
    ulSetError ( UL_WARNING, "ac_to_gl: Illegal crease angle." ) ;

  return PARSE_CONT ;
}

static int do_rot ( char *s )
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

static int do_loc      ( char *s )
{
  if ( sscanf ( s, "%f %f %f", & current_matrix [ 3 ][ 0 ], & current_matrix [ 3 ][ 2 ], & current_matrix [ 3 ][ 1 ] ) != 3 )
    ulSetError ( UL_WARNING, "ac_to_gl: Illegal loc record." ) ;

  current_matrix [ 3 ][ 1 ] = - current_matrix [ 3 ][ 1 ] ;
  current_matrix [ 3 ][ 3 ] = 1.0f ;
  ((ssgTransform *)current_branch) -> setTransform ( current_matrix ) ;

  return PARSE_CONT ;
}

static int do_url      ( char *s )
{
  skip_quotes ( & s ) ;

#ifdef PRINT_URLS
  ulSetError ( UL_DEBUG, "/* URL: \"%s\" */\n", s ) ;
#endif

  return PARSE_CONT ;
}

static int do_numvert  ( char *s )
{
  char buffer [ 1024 ] ;

  int nv = strtol ( s, NULL, 0 ) ;
 
  for ( int i = 0 ; i < nv ; i++ )
  {
    sgVec3 v;
    fgets ( buffer, 1024, loader_fd ) ;

    if ( sscanf ( buffer, "%f %f %f", &v[0], &v[1], &v[2] ) != 3 )
    {
      ulSetError ( UL_FATAL, "ac_to_gl: Illegal vertex record." ) ;
    }

    float tmp  =  v[1] ;
    v[1] = -v[2] ;
    v[2] = tmp ;

    current_vertexarray -> add ( v ) ;
    current_texcoordarray -> add ( invalidTexture ) ;
  }

  return PARSE_CONT ;
}

static int do_numsurf  ( char *s )
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

static int do_surf     ( char *s )
{
  current_flags = strtol ( s, NULL, 0 ) ;

  char buffer [ 1024 ] ;

  while ( fgets ( buffer, 1024, loader_fd ) != NULL )
    if ( search ( surface_tags, buffer ) == PARSE_POP )
      break ;

  return PARSE_CONT ;
}


static int do_mat ( char *s )
{
  int mat = strtol ( s, NULL, 0 ) ;

  current_materialind = mat ;

  return PARSE_CONT ;
}

static void add_textured_vertex_edge ( short ind, sgVec2 tex )
{
  // Add a new triangle edge. For that, check for a vertex/texcoord
  // pair already in the current dataset. Use the already present index if present.

  bool has_texture = current_tfname != NULL ;
  if ( sgEqualVec2( tex, current_texcoordarray -> get ( ind ) ) || !has_texture ) {
    // In this case, we have that vertex/texcoord pair already at the index 
    // within the ac file.

    current_triindexarray -> add ( ind ) ;

  } else if ( sgEqualVec2( invalidTexture, current_texcoordarray -> get ( ind ) ) ) {
    // In this case, we have not yet stored a valid texture coordinate
    // for the vertex at the given index. Just copy the texturecoordinate
    // value into that place.

    sgCopyVec2( current_texcoordarray -> get ( ind ), tex ) ;

    current_triindexarray -> add ( ind ) ;

  } else {
    // Texture coordinate do not match the prevous texcoords stored for this vertex.
    // Search for a vertex/texcoord pair matching the requested one, if not
    // yet present, add a new one.
  
    int num = current_vertexarray -> getNum () ;
    for ( int i = 0 ; i < num ; i++ ) {
      if ( ( !has_texture || sgEqualVec2( tex, current_texcoordarray -> get ( i ) ) ) &&
           sgEqualVec3( current_vertexarray -> get ( ind ), current_vertexarray -> get ( i ) ) ) {
        current_triindexarray -> add ( i ) ;
        return ;
      }
    }

    // Need to copy that before, else we run into a racecondition where
    // we copy from an location which is already freed.
    sgVec3 vertex ;
    sgCopyVec3 ( vertex, current_vertexarray -> get ( ind ) ) ;
    current_vertexarray -> add ( vertex ) ;
    current_texcoordarray -> add ( tex ) ;

    current_triindexarray -> add ( num ) ;
  }
}

static int do_refs     ( char *s )
{
  int nrefs = strtol ( s, NULL, 0 ) ;
  char buffer [ 1024 ] ;

  if ( nrefs == 0 )
    return PARSE_POP ;
  
  int type = ( current_flags & 0x0F ) ;

  // Handle line type objects by creating a single leaf for each line segment.
  if ( type == 1 || type == 2 ) {
    ssgIndexArray *ind = new ssgIndexArray ;
    for ( int i = 0 ; i < nrefs ; i++ )
    {
      fgets ( buffer, 1024, loader_fd ) ;

      int vtx ;
      float dummy ;

      if ( sscanf ( buffer, "%d %f %f", &vtx, &dummy, &dummy ) != 3 )
      {
        ulSetError ( UL_FATAL, "ac_to_gl: Illegal ref record." ) ;
      }

      ind -> add ( vtx ) ;
    }    

    ssgColourArray *col = new ssgColourArray ( 1 ) ;
    col -> add ( mlist [ current_materialind ] -> rgba ) ;

    GLenum gltype = ( type == 1 ) ? GL_LINE_LOOP : GL_LINE_STRIP ;
    ssgVtxArray *va = new ssgVtxArray ( gltype, (ssgVertexArray *)current_vertexarray -> clone (),
                                        0, 0, col, ind );
    va -> removeUnusedVertices();
    va -> setState ( get_state ( mlist [ current_materialind ] ) ) ;

    ssgLeaf *leaf = current_options -> createLeaf ( va, 0 ) ;
    if ( leaf )
      current_branch -> addKid ( leaf ) ;
  }

  if ( type == 0 ) {
    // Handle surface datatypes.
    // Each surface is triangulated and each triangle is put
    // into the index array current_triindexarray.
    int first_vertind = -1 ;
    int prev_vertind  = 0 ;
    sgVec2 first_texcoord ;
    sgVec2 prev_texcoord ;
    for ( int i = 0 ; i < nrefs ; i++ )
      {
        fgets ( buffer, 1024, loader_fd ) ;
        
        int vertind ;
        sgVec2 texcoord ;
        
        if ( sscanf ( buffer, "%d %f %f", &vertind,
                      &texcoord[0],
                      &texcoord[1] ) != 3 )
          {
            ulSetError ( UL_FATAL, "ac_to_gl: Illegal ref record." ) ;
          }
        
        texcoord[0] *= texrep[0] ;
        texcoord[1] *= texrep[1] ;
        texcoord[0] += texoff[0] ;
        texcoord[1] += texoff[1] ;
        
        // Store the first index texcoord pair.
        // This one is referenced for every triangle.
        if ( first_vertind < 0 ) {
          first_vertind = vertind ;
          sgCopyVec2( first_texcoord, texcoord );
        }
        
        // When we have read the third vertex index we can emit the first triangle.
        if ( 2 <= i ) {
          // Store the edges of the triangle
          add_textured_vertex_edge ( first_vertind, first_texcoord ) ;
          add_textured_vertex_edge ( prev_vertind, prev_texcoord ) ;
          add_textured_vertex_edge ( vertind, texcoord ) ;
          
          // Store the material and flags for this surface. 
          current_matindexarray -> add ( current_materialind );
          current_flagsarray -> add ( current_flags );
        }
        
        // Copy current -> previous
        prev_vertind = vertind ;
        sgCopyVec2( prev_texcoord, texcoord );
      }
  }
  
  return PARSE_POP ;
}

static int do_kids ( char *s )
{
  last_num_kids = strtol ( s, NULL, 0 ) ;

  return PARSE_POP ;
}


ssgEntity *ssgLoadAC3D ( const char *fname, const ssgLoaderOptions* options )
{
  ssgEntity *obj = ssgLoadAC ( fname, options ) ;

  if ( obj == NULL )
    return NULL ;

  /* Do some simple optimisations */

  ssgBranch *model = new ssgBranch () ;
  model -> addKid ( obj ) ;
  ssgFlatten      ( obj ) ;
  return model ;
}

/*
  Original function for backwards compatibility...
*/

ssgEntity *ssgLoadAC ( const char *fname, const ssgLoaderOptions* options )
{
  ssgSetCurrentOptions ( (ssgLoaderOptions*)options ) ;
  current_options = ssgGetCurrentOptions () ;

  char filename [ 1024 ] ;
  current_options -> makeModelPath ( filename, fname ) ;

  num_materials = 0 ;

  current_tfname   = NULL ;
  current_branch   = NULL ;

  current_crease = 61.0 ;

  sgSetVec2 ( texrep, 1.0, 1.0 ) ;
  sgSetVec2 ( texoff, 0.0, 0.0 ) ;

  loader_fd = fopen ( filename, "r" ) ;

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

      if ( ! ulStrNEqual ( s, "AC3D", 4 ) )
      {
        fclose ( loader_fd ) ;
        ulSetError ( UL_WARNING, "ssgLoadAC: '%s' is not in AC3D format.", filename ) ;
        return NULL ;
      }
    }
    else
      search ( top_tags, s ) ;
  }

  delete [] current_tfname ;
  current_tfname = NULL ;
  fclose ( loader_fd ) ;

  return current_branch ;
}

