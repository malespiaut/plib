/*******************************************************
 **  ssgLoad3ds.cxx
 **  
 **  Written by Per Liedman (liedman@home.se)
 **  Last updated: 2000-09-08
 **
 **  This was written to be a part of Stephen J Bakers
 **  PLIB (http://plib.sourceforge.net)
 *******************************************************/

/* KNOWN ISSUES:

  * Some models (one of the test cases) gets turned "inside out" - obviously 
    the triangle winding has been reversed so that all triangles that should be
    culled is show and vice versa. I don't know where this information can
    be found in the file.

  * Models that uses double-sided materials sometimes look strange. My approach
    to double sided materials is to add a copy of each triangle with the 
    winding reversed and also negate the normals...this seems to *almost* work.

  * The transformation matrix in the 3DS files is still a mystery to me -
    whatever I do with it seems to make things worse than ignoring it, although
    that doesn't work perfect either. I've taken a look at other loaders, and
    it seems that no one really knows what to do with them.

*/

#include "ssgLocal.h"

#define MAX_MATERIALS 512

#define PARSE_OK 1
#define PARSE_ERROR 0
#define CHUNK_HEADER_SIZE (2 + 4)

#define IS_DOUBLESIDED 1

/* Define DEBUG if you want debug output
   (this might be a nice way of looking at the
   structure of a 3DS file). */
// #define DEBUG 1


#ifdef DEBUG
#define DEBUGPRINT(f, x, y, z) fprintf(stderr, f, debug_indent, x, y, z)
#else
#define DEBUGPRINT(f, x, y, z)
#endif

#ifdef DEBUG
char debug_indent[256];
#endif

/* this is the minimum value of the dot product for
   to faces if their normals should be smoothed, if
   they don't use smooth groups. */
float _ssg_smooth_threshold = 0.8f;

// 3ds chunk identifiers
enum {
  CHUNK_VERSION         = 0x0002,
  CHUNK_RGB1            = 0x0010,  // 3 floats of RGB
  CHUNK_RGB2            = 0x0011,  // 3 bytes of RGB
  CHUNK_RGB3            = 0x0012,  // 3 bytes of RGB (gamma corrected)
  CHUNK_AMOUNT          = 0x0030,
  CHUNK_MAIN            = 0x4D4D,
  CHUNK_OBJMESH         = 0x3D3D,
  CHUNK_ONEUNIT         = 0x0100,
  CHUNK_BKGCOLOR        = 0x1200,
  CHUNK_AMBCOLOR        = 0x2100,
  CHUNK_OBJBLOCK        = 0x4000,
  CHUNK_TRIMESH         = 0x4100,
  CHUNK_VERTLIST        = 0x4110,
  CHUNK_FACELIST        = 0x4120,
  CHUNK_FACEMAT         = 0x4130,
  CHUNK_MAPLIST         = 0x4140,
  CHUNK_SMOOLIST        = 0x4150,
  CHUNK_TRMATRIX        = 0x4160,
  CHUNK_LIGHT           = 0x4600,
  CHUNK_SPOTLIGHT       = 0x4610,
  CHUNK_CAMERA          = 0x4700,
  CHUNK_MATERIAL        = 0xAFFF,
  CHUNK_MATNAME         = 0xA000,
  CHUNK_AMBIENT         = 0xA010,
  CHUNK_DIFFUSE         = 0xA020,
  CHUNK_SPECULAR        = 0xA030,
  CHUNK_SHININESS       = 0xA040,
  CHUNK_SHINE_STRENGTH  = 0xA041,
  CHUNK_TRANSPARENCY    = 0xA050,
  CHUNK_TRANSP_FALLOFF  = 0xA052,
  CHUNK_DOUBLESIDED     = 0xA081,
  CHUNK_TEXTURE         = 0xA200,
  CHUNK_BUMPMAP         = 0xA230,
  CHUNK_MAPFILENAME     = 0xA300,
  CHUNK_MAPOPTIONS      = 0xA351,
  CHUNK_MAP_VSCALE      = 0xA354,
  CHUNK_MAP_USCALE      = 0xA356,
  CHUNK_MAP_UOFFST      = 0xA358,
  CHUNK_MAP_VOFFST      = 0xA35A,
  CHUNK_KEYFRAMER       = 0xB000,
  CHUNK_FRAMES          = 0xB008
} _3dsChunkIds;

// parsing functions for chunks that need separate treatment.
static int parse_material( unsigned int length);
static int parse_objblock( unsigned int length);
static int parse_rgb1( unsigned int length);
static int parse_rgb2( unsigned int length);
static int parse_material_name( unsigned int length);
static int parse_ambient( unsigned int length);
static int parse_diffuse( unsigned int length);
static int parse_specular( unsigned int length);
static int parse_shininess( unsigned int length);
static int parse_transparency( unsigned int length);
static int parse_doublesided( unsigned int length);
static int parse_vert_list( unsigned int length);
static int parse_face_list( unsigned int length);
static int parse_map_list( unsigned int length);
static int parse_tra_matrix( unsigned int length);
static int parse_trimesh( unsigned int length);
static int parse_smooth_list( unsigned int length);
static int parse_face_materials( unsigned int length);
static int parse_mapname( unsigned int length);
static int parse_mapoptions( unsigned int length);
static int parse_uscale( unsigned int length);
static int parse_vscale( unsigned int length);
static int parse_uoffst( unsigned int length);
static int parse_voffst( unsigned int length);
static int parse_oneunit( unsigned int length);
static int parse_version( unsigned int length);
static int identify_face_materials( unsigned int length );

struct _ssg3dsChunk {
  unsigned short id;
  _ssg3dsChunk *subchunks;
  int (*parse_func) ( unsigned int );
};

static _ssg3dsChunk FaceListDataChunks[] =
{ { CHUNK_SMOOLIST, NULL, parse_smooth_list             },
  { CHUNK_FACEMAT, NULL,  identify_face_materials       },
  { 0, NULL, NULL }
};

static _ssg3dsChunk FaceListChunks[] =
{ { CHUNK_FACEMAT, NULL, parse_face_materials           },
  { 0, NULL, NULL }
};

static _ssg3dsChunk TriMeshDataChunks[] =
{ { CHUNK_VERTLIST, NULL, parse_vert_list               },
  { CHUNK_MAPLIST, NULL, parse_map_list                 },
  { 0, NULL, NULL }
};  /* these chunks have to be known *before* we call parse_face_list
       (see parse_trimesh for more info) */

static _ssg3dsChunk TriMeshChunks[] =
{ { CHUNK_FACELIST, FaceListChunks, parse_face_list     },
  { CHUNK_TRMATRIX, NULL, parse_tra_matrix              },
  { 0, NULL, NULL }
};

static _ssg3dsChunk ObjBlockChunks[] =
{ { CHUNK_TRIMESH, TriMeshChunks, parse_trimesh         },
  { 0, NULL, NULL }
};

static _ssg3dsChunk TextureChunks[] =
{ { CHUNK_MAPFILENAME, NULL, parse_mapname              },
  { CHUNK_MAP_USCALE, NULL, parse_uscale                },
  { CHUNK_MAP_VSCALE, NULL, parse_vscale                },
  { CHUNK_MAP_UOFFST, NULL, parse_uoffst                },
  { CHUNK_MAP_VOFFST, NULL, parse_voffst                },
  { CHUNK_MAPOPTIONS, NULL, parse_mapoptions            },
  { 0, NULL, NULL }
};

static _ssg3dsChunk MaterialColourChunks[] =
{ { CHUNK_RGB1, NULL, parse_rgb1                        },
  { CHUNK_RGB2, NULL, parse_rgb2                        },
  { CHUNK_RGB3, NULL, parse_rgb2                        },
  { 0, NULL, NULL }
};

static _ssg3dsChunk MaterialChunks[] =
{ { CHUNK_MATNAME, NULL, parse_material_name            },
  { CHUNK_AMBIENT, MaterialColourChunks, parse_ambient  },
  { CHUNK_DIFFUSE, MaterialColourChunks, parse_diffuse  },
  { CHUNK_SPECULAR, MaterialColourChunks, parse_specular},
  { CHUNK_SHINE_STRENGTH, NULL, parse_shininess         },
  { CHUNK_TRANSPARENCY, NULL, parse_transparency        },
  { CHUNK_TEXTURE, TextureChunks, NULL                  },
  { CHUNK_DOUBLESIDED, NULL, parse_doublesided          },
  { 0, NULL, NULL }
};

static _ssg3dsChunk ObjMeshChunks[] =
{ { CHUNK_MATERIAL, MaterialChunks, parse_material      },
  { CHUNK_OBJBLOCK, ObjBlockChunks, parse_objblock      },
  { CHUNK_ONEUNIT,  NULL,           parse_oneunit       },
  { 0, NULL, NULL }
};

static _ssg3dsChunk MainChunks[] =
{ { CHUNK_OBJMESH, ObjMeshChunks, NULL                  },
  { CHUNK_VERSION, NULL,    parse_version               },
  { 0, NULL, NULL }
};

static _ssg3dsChunk TopChunk[] = 
{ { CHUNK_MAIN, MainChunks, NULL                        },
  { 0, NULL, NULL }
};

struct _3dsMat {
  char *name ;
  int flags;
  sgVec3 colour[4];
  float shi, alpha;
  
  char *tex_name;
  sgVec2 tex_scale, tex_offset;
  bool wrap_s, wrap_t;
};

#define _3DSMAT_AMB 0
#define _3DSMAT_DIF 1
#define _3DSMAT_EMI 2
#define _3DSMAT_SPE 3

static int parse_chunks( _ssg3dsChunk *chunk_list, unsigned int length);
static void add_leaf( _3dsMat *material, int listed_faces, 
		      unsigned short *face_indices );

FILE *model;

static int is_little_endian;
static int num_objects, num_materials, num_textures;
static int double_sided;     // is there some double sided material?

static ssgBranch *top_object;

static _3dsMat **materials, *current_material;

static unsigned short *vertex_index, *normal_index, num_vertices, num_faces;
static unsigned int  *smooth_list;

static ssgTransform *current_transform;

static sgVec3 *vertex_list;
static sgVec3 *face_normals, *vertex_normals;
static sgVec2 *texcrd_list;
static int smooth_found, facemat_found;

static int colour_mode;

//==========================================================
// ENDIAN ISSUES
static inline void endian_swap(unsigned int *x) {
  *x = (( *x >> 24 ) & 0x000000FF ) | 
    (( *x >>  8 ) & 0x0000FF00 ) | 
    (( *x <<  8 ) & 0x00FF0000 ) | 
    (( *x << 24 ) & 0xFF000000 ) ;
}

static inline void endian_swap(unsigned short *x) {
  *x = (( *x >>  8 ) & 0x00FF ) | 
    (( *x <<  8 ) & 0xFF00 ) ;
}

static float get_float() {
  float f;
  fread( &f, 4, 1, model );

  if (is_little_endian)
    return f;
  else {
    endian_swap((unsigned int*)&f);
    return f;
  }
}

static unsigned int get_dword() {
  unsigned int d;
  fread( &d, 4, 1, model );

  if (is_little_endian)
    return d;
  else {
    endian_swap(&d);
    return d;
  }
}

static unsigned short get_word() {
  unsigned short w;
  fread( &w, 2, 1, model );
  
  if (is_little_endian)
    return w;
  else {
    endian_swap(&w);
    return w;
  }
}

static unsigned char get_byte() {
  unsigned char b;
  fread( &b, 1, 1, model );
  return b;
}

/* NOTE: This string has to be freed by the caller
   Also note: You can't fetch strings longer than 256
   characters! */
static char* get_string() {
  char *s = new char[256], read;
  int c = 0;

  while ( (read = fgetc(model)) != 0 ) {
    if (c < 255)
      s[c++] = read;
  }
  s[c] = 0;

  return s;
}

//==========================================================
// MATERIAL PARSERS
static int parse_mapname( unsigned int length )
{
  current_material->tex_name = get_string();
  DEBUGPRINT("%sMap name: %s %s%s\n", current_material->tex_name, "", "");
  return PARSE_OK;
}

static int parse_mapoptions( unsigned int length )
{
  unsigned short value = get_word();
  // bit 4: 0=tile (default), 1=do not tile (a single bit for both u and v)
  current_material->wrap_s = current_material->wrap_t = ((value & 0x10) == 0);
  DEBUGPRINT("%sMap options (wrap): %c %s%s\n", 
	     (current_material->wrap_s)?'Y':'N', "", "");

  return PARSE_OK;
}

static int parse_uscale( unsigned int length )
{
  current_material->tex_scale[1] = get_float();
  DEBUGPRINT("%sU-scale: %.3f %s%s\n", current_material->tex_scale[1], "", "");
  return PARSE_OK;
}

static int parse_vscale( unsigned int length )
{
  current_material->tex_scale[0] = get_float();
  DEBUGPRINT("%sV-scale: %.3f %s%s\n", current_material->tex_scale[0], "", "");
  return PARSE_OK;
}

static int parse_uoffst( unsigned int length )
{
  current_material->tex_offset[1] = get_float();
  DEBUGPRINT("%sU-offset: %.3f %s%s\n",
	     current_material->tex_offset[1], "", "");
  return PARSE_OK;
}

static int parse_voffst( unsigned int length )
{
  current_material->tex_offset[0] = get_float();
  DEBUGPRINT("%sV-offset: %.3f %s%s\n",
	     current_material->tex_offset[0], "", "");
  return PARSE_OK;
}

static int parse_material( unsigned int length ) {
  materials[num_materials] = current_material = new _3dsMat;
  current_material->flags = 0;
  current_material->tex_name = NULL;
  num_materials++;

  /* set default value for material colours (taken from glMaterial man page) */
  sgSetVec3(current_material -> colour[_3DSMAT_AMB], 0.2f, 0.2f, 0.2f );
  sgSetVec3(current_material -> colour[_3DSMAT_DIF], 0.8f, 0.8f, 0.8f );
  sgSetVec3(current_material -> colour[_3DSMAT_SPE], 0.0f, 0.0f, 0.0f );
  sgSetVec3(current_material -> colour[_3DSMAT_EMI], 0.0f, 0.0f, 0.0f );
  current_material -> shi = 0.0f;
  current_material -> alpha = 1.0f;

  /* set up texture info */
  sgSetVec2(current_material -> tex_scale , 1.0f, 1.0f);
  sgSetVec2(current_material -> tex_offset, 0.0f, 0.0f);
  current_material -> wrap_s = current_material -> wrap_t = true;
  
  DEBUGPRINT("%sNew material found.%s%s%s\n", "", "", "");
  return PARSE_OK;
}

static int parse_material_name( unsigned int length ) {
  current_material -> name = get_string();
  DEBUGPRINT("%sMaterial name:%s%s%s\n", current_material->name, "", "");
  
  return PARSE_OK;
}

static int parse_rgb1( unsigned int length ) {
  float r, g, b;

  r = get_float();
  g = get_float();
  b = get_float();
  DEBUGPRINT("%sColour: R:%.2f, G:%.2f, B:%.2f\n", r, g, b);

  sgSetVec3(current_material->colour[colour_mode], r, g, b);

  return PARSE_OK;
}

static int parse_rgb2( unsigned int length ) {
  float r, g, b;

  r = (float)get_byte() / 255.0f;
  g = (float)get_byte() / 255.0f;
  b = (float)get_byte() / 255.0f;
  DEBUGPRINT("%sColour: R:%.2f, G:%.2f, B:%.2f\n", r, g, b);

  sgSetVec3(current_material->colour[colour_mode], r, g, b);
  return PARSE_OK;
}

static int parse_ambient( unsigned int length ) {
  colour_mode = _3DSMAT_AMB;
  DEBUGPRINT("%sAmbient colour%s%s%s\n", "", "", "");
  return PARSE_OK;
}

static int parse_diffuse( unsigned int length ) {
  colour_mode = _3DSMAT_DIF;
  DEBUGPRINT("%sDiffuse colour%s%s%s\n", "", "", "");
  return PARSE_OK;
}

static int parse_specular( unsigned int length ) {
  colour_mode = _3DSMAT_SPE;
  DEBUGPRINT("%sSpecular colour%s%s%s\n", "", "", "");
  return PARSE_OK;
}

static int parse_shininess( unsigned int length ) {
  // this chunk contains a percentage chunk,
  // so just read that chunks header
  get_word(); get_dword();
  current_material -> shi = (float)get_word() * 128.0f / 100.0f;
  DEBUGPRINT("%sShininess:%.1f%s%s\n", current_material->shi, "", "");
  return PARSE_OK;
}

static int parse_transparency( unsigned int length ) {
  // this chunk contains a percentage chunk,
  // so just read that chunks header
  get_word(); get_dword();
  current_material->alpha = 1.0f - (float)get_word() / 100.0f;
  DEBUGPRINT("%sAlpha:%.1f%s%s\n", current_material->alpha, "", "");
  return PARSE_OK;
}

static int parse_doublesided( unsigned int length ) {
  double_sided = current_material->flags |= IS_DOUBLESIDED;

  DEBUGPRINT("%sMaterial is double sided.%s%s%s\n", "", "", "");

  return PARSE_OK;
}

static ssgSimpleState *get_state( _3dsMat *mat ) {
  ssgSimpleState *st = new ssgSimpleState () ;

  /* 
     NOTE: wrap_s & wrap_t is not used. How should this be passed
     before we have a texture!? (Other loaders seem to ignore this problem)
  */
  
  st -> setMaterial ( GL_AMBIENT, 
		      mat->colour[_3DSMAT_AMB][0], 
		      mat->colour[_3DSMAT_AMB][1], 
		      mat->colour[_3DSMAT_AMB][2], mat->alpha ) ;
  st -> setMaterial ( GL_DIFFUSE,
		      mat->colour[_3DSMAT_DIF][0], 
		      mat->colour[_3DSMAT_DIF][1], 
		      mat->colour[_3DSMAT_DIF][2], mat->alpha ) ;
  st -> setMaterial ( GL_SPECULAR, 
		      mat->colour[_3DSMAT_SPE][0], 
		      mat->colour[_3DSMAT_SPE][1], 
		      mat->colour[_3DSMAT_SPE][2], mat->alpha ) ;
  st -> setMaterial ( GL_EMISSION, 
		      mat->colour[_3DSMAT_EMI][0], 
		      mat->colour[_3DSMAT_EMI][1], 
		      mat->colour[_3DSMAT_EMI][2], mat->alpha ) ;
  st -> setShininess( mat -> shi ) ;

  st -> disable ( GL_COLOR_MATERIAL ) ;
  st -> enable  ( GL_LIGHTING       ) ;
  st -> setShadeModel ( GL_SMOOTH ) ;

  if ( mat -> alpha < 0.99f )
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


//==========================================================
// TRIMESH PARSERS

static void free_trimesh()
{
  DEBUGPRINT("%sFreeing trimesh object%s%s%s\n", "","","");

  if (vertex_list)
    delete [] vertex_list;

  if (face_normals)
    delete [] face_normals;

  if (vertex_normals)
    delete [] vertex_normals;

  if (texcrd_list)
    delete [] texcrd_list;

  if (smooth_list)
    delete [] smooth_list;

  if (vertex_index)
    delete [] vertex_index;

  vertex_list = NULL;
  face_normals = NULL;
  vertex_normals = NULL;
  texcrd_list = NULL;
  smooth_list = NULL;
  vertex_index = NULL;
}

static int parse_trimesh( unsigned int length ) {
  current_transform = new ssgTransform();

  free_trimesh();

  top_object -> addKid( current_transform );

  /* Before we parse CHUNK_FACEMAT, we have to know vertices and texture
     coordinates. To ensure this, we make a special pass of the Trimesh
     chunks, just extracting this information.
     This is kind of a kludge, but it was the easiest way to solve this problem
  */
  DEBUGPRINT("%sPrescanning sub-chunks for vertices and texture coords." \
	     "%s%s%s\n", "","","");
#ifdef DEBUG
  strcat(debug_indent, "    ");
#endif

  unsigned long p = ftell(model);
  int parse_ok = parse_chunks( TriMeshDataChunks, length );
  fseek(model, p, SEEK_SET);

#ifdef DEBUG
  debug_indent[strlen(debug_indent)-4] = 0;
#endif
  DEBUGPRINT("%sDone prescanning.%s%s%s\n", "","","");

  return parse_ok;
}

static int parse_vert_list( unsigned int length ) {
  num_vertices = get_word();
  vertex_list = new sgVec3[num_vertices];

  DEBUGPRINT("%sReading %d vertices.%s%s\n", num_vertices, "", "");

  for (int i = 0; i < num_vertices; i++) {
    vertex_list[i][0] = get_float();
    vertex_list[i][1] = get_float();
    vertex_list[i][2] = get_float();
  }

  return PARSE_OK;
}

static int parse_smooth_list( unsigned int length )
{
  int i;
  smooth_found = TRUE;

  smooth_list = new unsigned int[num_faces];
  DEBUGPRINT("%sReading smoothlist%s%s%s\n", "", "", "");

  for (i = 0; i < num_faces; i++)
    smooth_list[i] = get_dword();

  return PARSE_OK;
}

static void smooth_normals( int use_smooth_list )
{
  int i, j;

  for (i = 0; i < num_faces; i++) {
    int v1 = i * 3,
      v2 = i * 3 + 1,
      v3 = i * 3 + 2;

    sgZeroVec3( vertex_normals[v1] );
    sgZeroVec3( vertex_normals[v2] );
    sgZeroVec3( vertex_normals[v3] );

    for (j = 0; j < num_faces; j++) {
      int should_smooth;

      if (use_smooth_list) {
        should_smooth = (smooth_list[i] & smooth_list[j]) && 
          (sgScalarProductVec3( face_normals[i], face_normals[j] ) >= -0.5f);
      } else {
        should_smooth = (sgScalarProductVec3( face_normals[i], face_normals[j] ) > 
                         _ssg_smooth_threshold);
      }

      if (should_smooth) {
        // now we have to know if the faces shares vertices
        for (int a = v1; a < v1 + 3; a++) {
          for (int b = j * 3; b < j * 3 + 3; b++) {
            if (vertex_index[a] == vertex_index[b]) {
              sgAddVec3( vertex_normals[a], face_normals[j] );
            }
          }
        }
      }
    }

    sgNormaliseVec3( vertex_normals[v1] );
    sgNormaliseVec3( vertex_normals[v2] );
    sgNormaliseVec3( vertex_normals[v3] );
  }
}

static int identify_face_materials( unsigned int length ) {
  facemat_found = TRUE;
  DEBUGPRINT("%sFace materials found.%s%s%s\n", "","","");

  fseek( model, length, SEEK_CUR );

  return PARSE_OK;
}

static int parse_face_list( unsigned int length ) {
  int i;
  num_faces = get_word();

  DEBUGPRINT("%sReading %d faces.%s%s\n", num_faces, "", "");

  vertex_index   = new unsigned short[num_faces*3];
  face_normals   = new sgVec3[num_faces];

  vertex_normals = new sgVec3[num_faces * 3];

  for (i = 0; i < num_faces; i++) {
    vertex_index[i*3    ] = get_word();
    vertex_index[i*3 + 1] = get_word();
    vertex_index[i*3 + 2] = get_word();
    unsigned short flags  = get_word();

    if (flags & 7 == 0) {     // Triangle vertices order should be swapped
      unsigned short tmp    = vertex_index[i*3 + 1];
      vertex_index[i*3 + 1] = vertex_index[i*3 + 2];
      vertex_index[i*3 + 2] = tmp;
    }

    sgMakeNormal( face_normals[i], vertex_list[vertex_index[i*3    ]], 
                  vertex_list[vertex_index[i*3 + 1]], 
                  vertex_list[vertex_index[i*3 + 2]] );

  }

  /* this is a special "hack" for the face list chunk
     because we HAVE to know the smooth list (if there is one)
     before building the ssgVtxTable objects, this parsing has to
     be done first...*ugh*/

  smooth_found = FALSE;
  facemat_found = FALSE;
  DEBUGPRINT("%sPrescanning sub-chunks for smooth list...%s%s%s\n", "","","");
#ifdef DEBUG
  strcat(debug_indent, "    ");
#endif

  unsigned long p = ftell(model);
  parse_chunks( FaceListDataChunks, length - (2 + 8*num_faces) );
  fseek(model, p, SEEK_SET);

#ifdef DEBUG
  debug_indent[strlen(debug_indent)-4] = 0;
#endif
  DEBUGPRINT("%sDone prescanning.%s%s%s\n", "","","");

  /* now apply correct smoothing. If smooth list has been found,
     use it, otherwise use threshold value. */
  smooth_normals( smooth_found );

  if (!facemat_found) {
    DEBUGPRINT("%sNo CHUNK_FACEMAT found. Adding default faces of material " \
	       "\"%s\"%s%s.\n", materials[0]->name, "", "");
    unsigned short *face_indices = new unsigned short[num_faces];
    for (i = 0; i < num_faces; i++) {
      face_indices[i] = i;
    }
    add_leaf(materials[0], num_faces, face_indices);
  }

  return PARSE_OK;
}

static int parse_map_list( unsigned int length ) {
  unsigned short num_v = get_word();
  texcrd_list = new sgVec2[num_v];

  DEBUGPRINT("%sReading %d texture coords.%s%s\n", num_v, "", "");

  for (int i = 0; i < num_v; i++) {
    texcrd_list[i][0] = get_float();
    texcrd_list[i][1] = get_float();
  }

  return PARSE_OK;
}

static int parse_tra_matrix( unsigned int length ) {
  sgMat4 m;

  sgMakeIdentMat4( m );

  DEBUGPRINT("%sReading transformation matrix.%s%s%s\n", "","","");

  /* Strange things seems to be going on with the
     local coordinate system in 3ds - I have commented
     this out, but things seems to work better without
     it (which is odd). */
  
  int i, j;
  
  for (i = 0; i < 4; i++) {
    for (j = 0; j < 3; j++) {
      m[j][i] = get_float();
    }
  }

  m[3][3] = 1.0f;
  sgTransposeNegateMat4( m );
  
#ifdef DEBUG
  for (int a = 0; a < 4; a++) {
    DEBUGPRINT("%s%s%s%s", "","","");
    for (int b = 0; b < 4; b++) {
      fprintf(stderr, "%.2f\t", m[b][a]);
    }
    fprintf(stderr, "\n");
  }
#endif  

  current_transform -> setTransform( m );
  
  return PARSE_OK;
}

static void add_leaf( _3dsMat *material, int listed_faces, 
		      unsigned short *face_indices ) {
  int is_ds       = material->flags & IS_DOUBLESIDED;
  int has_texture = material->tex_name != NULL;
  ssgVertexArray   *vertices = new ssgVertexArray();
  ssgNormalArray   *normals  = new ssgNormalArray();
  ssgTexCoordArray *texcrds  = NULL;
  
  if (has_texture) {
    if (texcrd_list == NULL) {
      ulSetError(UL_WARNING, "ssgLoad3ds: Texture coords missing.");
    } else {
      texcrds = new ssgTexCoordArray();
    }
  }

  for (int i = 0; i < listed_faces; i++) {
    unsigned short faceindex = face_indices[i];
    int v1 = faceindex * 3,
      v2 = faceindex * 3 + 1,
      v3 = faceindex * 3 + 2;

    vertices->add( vertex_list[ vertex_index[v1] ] );
    vertices->add( vertex_list[ vertex_index[v2] ] );
    vertices->add( vertex_list[ vertex_index[v3] ] );

    normals ->add( vertex_normals[ v1 ] );
    normals ->add( vertex_normals[ v2 ] );
    normals ->add( vertex_normals[ v3 ] );

    if (has_texture && texcrd_list != NULL) {
      int num_texcrds = 3;
      sgVec2 _texcrds[6];
      sgCopyVec2( _texcrds[0], texcrd_list[ vertex_index[v1] ] );
      sgCopyVec2( _texcrds[1], texcrd_list[ vertex_index[v2] ] );
      sgCopyVec2( _texcrds[2], texcrd_list[ vertex_index[v3] ] );
      if (is_ds) {
        num_texcrds = 6;
	sgCopyVec2( _texcrds[3], texcrd_list[ vertex_index[v1] ] );
	sgCopyVec2( _texcrds[4], texcrd_list[ vertex_index[v3] ] );
	sgCopyVec2( _texcrds[5], texcrd_list[ vertex_index[v2] ] );
      }

      for (int j = 0; j < num_texcrds; j++) {
        _texcrds[j][0] *= material->tex_scale[0];
        _texcrds[j][1] *= material->tex_scale[1];
        sgAddVec2( _texcrds[j], material->tex_offset );
	texcrds->add( _texcrds[j] );
      }
    }

    if (is_ds) {
      sgVec3 n[3];  /* we have to use the *negated* normals for back faces */

      vertices->add( vertex_list[ vertex_index[v1] ] );
      vertices->add( vertex_list[ vertex_index[v3] ] );
      vertices->add( vertex_list[ vertex_index[v2] ] );

      sgCopyVec3( n[0], vertex_normals[v1] );
      sgCopyVec3( n[1], vertex_normals[v3] );
      sgCopyVec3( n[2], vertex_normals[v2] );

      for (int j = 0; j < 3; j++) {
	sgNegateVec3( n[j] );
	normals->add( n[j] );
      }
    }

  }

  ssgVtxTable* vtab = new ssgVtxTable ( GL_TRIANGLES,
    vertices, normals, texcrds, NULL ) ;
  vtab -> setState ( get_state( material ) ) ;
  vtab -> setCullFace ( TRUE ) ;

  ssgLeaf* leaf = (*_ssgCreateFunc) ( vtab, material -> tex_name, 0 ) ;

  if ( leaf )
    current_transform -> addKid( leaf );
}

static int parse_face_materials( unsigned int length ) {
  int mat_num;
  char *mat_name = get_string();
  _3dsMat *material = NULL;

  // find the material
  for (mat_num = 0; mat_num < num_materials; mat_num++) {
    if ( strcmp( mat_name, materials[mat_num]->name ) == 0 ) {
      material = materials[mat_num];
      break;
    }
  }

  if (material == NULL) {
    ulSetError(UL_WARNING, "ssgLoad3ds: Undefined reference to material " \
	    "\"%s\" found.", mat_name);
    return PARSE_ERROR;
  }

  unsigned short listed_faces = get_word();

  DEBUGPRINT("%sFaces of \"%s\" list with %d faces.%s\n", 
	     mat_name, listed_faces, "");

  delete mat_name;  // no longer needed
  
  unsigned short *face_indices = new unsigned short[listed_faces];
  for (int i = 0; i < listed_faces; i++) {
    face_indices[i] = get_word();
  }

  add_leaf(material, listed_faces, face_indices);

  delete face_indices;

  return PARSE_OK;
}

//==========================================================
// OBJBLOCK PARSER

static int parse_objblock( unsigned int length ) {
  char *name = get_string();
  DEBUGPRINT("%sObject block \"%s\"%s%s\n", name, "", "");
  delete name;  // we don't use this for anything

  return PARSE_OK;
}

static int parse_oneunit( unsigned int length ) {
#ifdef DEBUG
  float oneunit = get_float();
  DEBUGPRINT("%sOne unit: %.3f%s%s\n", oneunit, "", "");
#else
  get_float() ;
#endif

  return PARSE_OK;
}

static int parse_version( unsigned int length ) {
#ifdef DEBUG
  unsigned int version = get_dword();
  DEBUGPRINT("%s3DS Version: %d%s%s\n", version, "", "");
#else
  get_dword() ;
#endif

  return PARSE_OK;
}

//==========================================================
// GENERAL CHUNK PARSER

static int parse_chunks( _ssg3dsChunk *chunk_list, unsigned int length )
{
  int parse_ok = PARSE_OK;
  unsigned short id;
  unsigned int sub_length;
  unsigned int p = 0;
  _ssg3dsChunk *t;

  while (parse_ok && p < length) {
    id = get_word();
    sub_length = get_dword();

    if (p + sub_length > length) {
      ulSetError(UL_WARNING, "ssgLoad3ds: Illegal chunk %X of length %i. " \
		 "Chunk is longer than parent chunk.", (int)id, sub_length);
      return PARSE_ERROR;
    }

    p += sub_length;
    sub_length -= CHUNK_HEADER_SIZE;

    for (t = chunk_list; t->id != 0 && t->id != id; t++);

    if (t->id == id) {
      DEBUGPRINT("%sFound chunk %X of length %d%s (known)\n", 
		 id, sub_length,"");
      // this is a known chunk
      // do chunk-specific parsing if available
      unsigned long cp = ftell(model);

      if (t->parse_func)
        parse_ok = t->parse_func( sub_length );

#ifdef DEBUG
      strcat(debug_indent, "    ");
#endif

      // if chunk can have subchunks, parse these
      if (t->subchunks && parse_ok) {
        parse_ok = parse_chunks( t->subchunks, 
				 sub_length - (ftell(model)-cp) );
      }

#ifdef DEBUG
      debug_indent[strlen(debug_indent)-4] = 0;
#endif

    } else {
      DEBUGPRINT("%sFound chunk %X of length %d%s (unknown)\n", 
		 id, sub_length,"");
      fseek( model, sub_length, SEEK_CUR );
    }

  }

  return parse_ok;
}



ssgEntity *ssgLoad3ds( const char *filename, ssgHookFunc hookfunc ) {
  int i = 1 ;
  is_little_endian = *((char *) &i );
  (*_ssgCreateFunc) ( 0, 0, 0 ) ;  //reset

  char *filepath;

  if (filename[0] != '/' && _ssgModelPath[0] != '\0') {
    filepath = new char[strlen(filename) + strlen(_ssgModelPath) + 2];
    strcpy( filepath, _ssgModelPath);
    strcat( filepath, "/" );
    strcat( filepath, filename );
  } else {
    filepath = new char[strlen(filename) + 1];
    strcpy(filepath, filename);
  }
  
  model = fopen ( filepath, "rb" );
  fseek(model, 0, SEEK_END);
  unsigned long size = ftell(model);
  rewind(model);

  if ( model == NULL ) {
    ulSetError(UL_WARNING, "ssgLoad3ds: Failed to open '%s' for reading", 
	       filepath ) ;
    return NULL ;
  }

  num_objects = num_materials = num_textures = 0;
  vertex_list = NULL;
  texcrd_list = NULL;
  face_normals = NULL;
  vertex_index = normal_index = NULL;
  top_object = new ssgBranch();

  // initialize some storage room for materials
  // (ok, could be implemented as linked list, but...well I'm lazy)
  materials = new _3dsMat*[MAX_MATERIALS];
  
  parse_chunks( TopChunk, size );

  fclose(model);

  // clean up the materials array
  for (i = 0; i < num_materials; i++) {
    if (materials[i] -> name != NULL) {
      delete materials[i] -> name;
    }
    if (materials[i] -> tex_name != NULL) {
      delete materials[i] -> tex_name;
    }

    delete materials[i];
  }

  delete [] filepath;
  delete [] materials;

  free_trimesh();

  (*_ssgCreateFunc) ( 0, 0, 0 ) ;  //reset
  return top_object; 
}


