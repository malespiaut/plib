/*******************************************************
 **  ssgLoad3ds.cxx
 **  
 **  Written by Per Liedman (liedman@home.se)
 **  Last updated: 2000-04-06
 **
 **  This was written to be a part of Stephen J Bakers
 **  PLIB (http://www.woodsoup.org/projs/plib)
 *******************************************************/

#include "ssgLocal.h"

#define MAX_MATERIALS 512

#define PARSE_OK 1
#define PARSE_ERROR 0
#define CHUNK_HEADER_SIZE (2 + 4)

#define IS_DOUBLESIDED 1
#define HAS_TEXTURE    2

/* this is the minimum value of the dot product for
   to faces if their normals should be smoothed, if
   they don't use smooth groups. */
float _ssg_smooth_threshold = 0.8f;

// 3ds chunk identifiers
enum {
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
static int parse_material( char **p, unsigned int length);
static int parse_objblock( char **p, unsigned int length);
static int parse_rgb1( char **p, unsigned int length);
static int parse_rgb2( char **p, unsigned int length);
static int parse_material_name( char **p, unsigned int length);
static int parse_ambient( char **p, unsigned int length);
static int parse_diffuse( char **p, unsigned int length);
static int parse_specular( char **p, unsigned int length);
static int parse_shininess( char **p, unsigned int length);
static int parse_transparency( char **p, unsigned int length);
static int parse_doublesided( char **p, unsigned int length);
static int parse_vert_list( char **p, unsigned int length);
static int parse_face_list( char **p, unsigned int length);
static int parse_map_list( char **p, unsigned int length);
static int parse_tra_matrix( char **p, unsigned int length);
static int parse_trimesh( char **p, unsigned int length);
static int parse_smooth_list( char **p, unsigned int length);
static int parse_face_materials( char **p, unsigned int length);
static int parse_mapname( char **p, unsigned int length);
static int parse_mapoptions( char **p, unsigned int length);
static int parse_uscale( char **p, unsigned int length);
static int parse_vscale( char **p, unsigned int length);
static int parse_uoffst( char **p, unsigned int length);
static int parse_voffst( char **p, unsigned int length);

struct _ssg3dsChunk {
  unsigned short id;
  _ssg3dsChunk *subchunks;
  int (*parse_func) (char** , unsigned int );
};

static _ssg3dsChunk FaceListDataChunks[] =
{ { CHUNK_SMOOLIST, NULL, parse_smooth_list             },
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
  { 0, NULL, NULL }
};

static _ssg3dsChunk MainChunks[] =
{ { CHUNK_OBJMESH, ObjMeshChunks, NULL                  },
  { 0, NULL, NULL }
};

static _ssg3dsChunk TopChunk[] = 
{ { CHUNK_MAIN, MainChunks, NULL                        },
  { 0, NULL, NULL }
};

static int parse_chunks(char *p, char *chunk_end, _ssg3dsChunk *chunk_list);

static int is_little_endian;
static int num_objects, num_materials, num_textures;
static int double_sided;     // is there some double sided material?

static ssgBranch *top_object;

static ssgSimpleState **materials, *current_material;
static char **material_names;
static char **mat_tfnames;
static int *mat_flag;
static sgVec2 *texture_scale, *texture_offst;

static unsigned short *vertex_index, *normal_index, num_vertices, num_faces;
static unsigned int  *smooth_list;

static ssgTransform *current_transform;


static sgVec3 *vertex_list;
static sgVec3 *face_normals, *vertex_normals;
static sgVec2 *texcrd_list;
static int smooth_found;

static GLenum colour_mode;
static float  current_alpha;

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

static float get(float *f) {
  if (is_little_endian)
    return *f;
  else {
    unsigned int p = (unsigned int)*f;
    endian_swap(&p);
    return p;
  }
}

static unsigned int get(unsigned int *f) {
  if (is_little_endian)
    return *f;
  else {
    unsigned int p = *f;
    endian_swap(&p);
    return p;
  }
}

static unsigned short get(unsigned short *f) {
  if (is_little_endian)
    return *f;
  else {
    unsigned short p = *f;
    endian_swap(&p);
    return p;
  }
}

//==========================================================
// MATERIAL PARSERS
static int parse_mapname( char **p, unsigned int length )
{
  int dbgtex=0;
  char filename[1024];

  if (*p[0] != '/') {
    strcpy(filename, _ssgTexturePath);
    strcat(filename, "/");
    strcat(filename, *p);
  }

  int mat_num;
  ssgSimpleState *sametex = NULL;
  char *texname = *p;

  // find a material with the same texture
  // since texture path is in common, just store (and compare) tex file name

  for (mat_num = 0; mat_num < num_materials-1; mat_num++)
    if ( mat_tfnames[mat_num] != (char *)NULL &&
         strcmp( texname, mat_tfnames[mat_num] ) == 0 )
    {
      sametex = materials[mat_num];
      break;
    }

  if (dbgtex)
    printf("%s\t mat #%03d (%s)", texname,num_materials,
                                   material_names[num_materials-1]);

  if ( sametex == (ssgSimpleState *)NULL )
  {
    current_material -> setTexture( filename );

    if ( texname == NULL )
      mat_tfnames[num_materials-1] = NULL ;
    else
    {
      mat_tfnames[num_materials-1] = new char [ strlen(texname) + 1 ] ;
      strcpy( mat_tfnames[num_materials-1], texname );
    }

    char *tf = mat_tfnames[num_materials-1] ;

    if ( tf == (char *)NULL )
      perror("problems with malloc - can't store old texnames\n");
    else
    {
      if (dbgtex) printf(" new!\n");
    }
  }
  else
  {
    current_material -> setTexture( sametex->getTextureHandle() );

    if (dbgtex) printf(" found at mat #%03d (%s)\n", mat_num+1,material_names[mat_num]);
  }

  current_material -> enable(GL_TEXTURE_2D);

  mat_flag[num_materials-1] |= HAS_TEXTURE;

  return PARSE_OK;
}

static int parse_mapoptions( char **p, unsigned int length )
{
  unsigned short value = get((unsigned short *)*p);
  // bit 4: 0=tile (default), 1=do not tile (a single bit for both u and v)
  unsigned int tile = !(value & 0x10);

  GLuint handle = materials[num_materials-1]->getTextureHandle();
  GLfloat *diff_rgb = materials[num_materials-1]->getMaterial( GL_DIFFUSE );
#ifdef GL_VERSION_1_1
  glBindTexture ( GL_TEXTURE_2D, handle ) ;
#else
  glBindTextureEXT ( GL_TEXTURE_2D, handle ) ;
#endif
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, tile ? GL_REPEAT : GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, tile ? GL_REPEAT : GL_CLAMP);
  if ( diff_rgb != (GLfloat *)NULL )
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, diff_rgb);
#ifdef GL_VERSION_1_1
  glBindTexture ( GL_TEXTURE_2D, 0 ) ;
#else
  glBindTextureEXT ( GL_TEXTURE_2D, 0 ) ;
#endif

  return PARSE_OK;
}

static int parse_uscale( char **p, unsigned int length )
{
  texture_scale[ num_materials - 1 ][1] = get((float*)*p);
  return PARSE_OK;
}

static int parse_vscale( char **p, unsigned int length )
{
  texture_scale[ num_materials - 1 ][0] = get((float*)*p);
  return PARSE_OK;
}

static int parse_uoffst( char **p, unsigned int length )
{
  texture_offst[ num_materials - 1 ][0] = get((float*)*p);
  return PARSE_OK;
}

static int parse_voffst( char **p, unsigned int length )
{
  texture_offst[ num_materials - 1 ][1] = get((float*)*p);
  return PARSE_OK;
}

static int parse_material( char **p, unsigned int length ) {
  materials[num_materials] = current_material = new ssgSimpleState();
  mat_flag[num_materials] = 0;
  num_materials++;

  current_material -> enable        ( GL_LIGHTING       ) ;
  current_material -> disable       ( GL_TEXTURE_2D     ) ;
  current_material -> disable       ( GL_COLOR_MATERIAL ) ;
  current_material -> disable       ( GL_BLEND          ) ;
  current_material -> setShadeModel ( GL_SMOOTH         ) ;
  current_material -> setOpaque () ;

  current_material -> enable  ( GL_CULL_FACE ) ;

  current_alpha = 1.0f;
  
  return PARSE_OK;
}

static int parse_material_name( char **p, unsigned int length ) {
  material_names[num_materials - 1] = *p;
  
  return PARSE_OK;
}

static int parse_rgb1( char **p, unsigned int length ) {
  float *w = (float*)*p, r, g, b;

  r = get(w++);
  g = get(w++);
  b = get(w++);

  current_material->setMaterial(colour_mode, r, g, b, current_alpha);

  *p = (char*)w;

  return PARSE_OK;
}

static int parse_rgb2( char **p, unsigned int length ) {
  unsigned char *w = (unsigned char*)*p;
  float r, g, b;

  r = (float)*w++ / 255.0;
  g = (float)*w++ / 255.0;
  b = (float)*w++ / 255.0;

  current_material->setMaterial(colour_mode, r, g, b, current_alpha);

  *p = (char*)w;

  return PARSE_OK;
}

static int parse_ambient( char **p, unsigned int length ) {
  colour_mode = GL_AMBIENT;
  return PARSE_OK;
}

static int parse_diffuse( char **p, unsigned int length ) {
  colour_mode = GL_DIFFUSE;
  return PARSE_OK;
}

static int parse_specular( char **p, unsigned int length ) {
  colour_mode = GL_SPECULAR;
  return PARSE_OK;
}

static int parse_shininess( char **p, unsigned int length ) {
  unsigned short *w = (unsigned short*)(*p + CHUNK_HEADER_SIZE);
  current_material -> setShininess( get(w) * 128.0 / 100.0 );

  return PARSE_OK;
}

static int parse_transparency( char **p, unsigned int length ) {
  float *col;
  unsigned short *w = (unsigned short*)(*p + CHUNK_HEADER_SIZE);
  current_alpha = 1.0f - (float)(get(w)) / 100.0f;

  if (current_alpha <= 0.99f) {
    current_material -> disable(GL_ALPHA_TEST);
    current_material -> enable(GL_BLEND);
    current_material -> setTranslucent();

    // diffuse may already have been set, change alpha in that case
    col = current_material -> getMaterial( GL_DIFFUSE );
    current_material -> setMaterial( GL_DIFFUSE, col[0], col[1], col[2], current_alpha );
  }
 
  return PARSE_OK;
}

static int parse_doublesided( char **p, unsigned int length ) {
  double_sided = mat_flag[num_materials - 1] |= IS_DOUBLESIDED;

  fprintf( stderr, "Warning: '%s' is doublesided.\n", material_names[num_materials-1] );

  return PARSE_OK;
}


//==========================================================
// TRIMESH PARSERS

static void free_trimesh()
{
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

static int parse_trimesh( char **p, unsigned int length ) {
  current_transform = new ssgTransform();

  free_trimesh();

  top_object -> addKid( current_transform );

  /* Before we parse CHUNK_FACEMAT, we have to know vertices and texture
     coordinates. To ensure this, we make a special pass of the Trimesh
     chunks, just extracting this information.
     This is kind of a kludge, but it was the easiest way to solve this problem */

  return parse_chunks( *p, *p + length - CHUNK_HEADER_SIZE, TriMeshDataChunks );
}

static int parse_vert_list( char **p, unsigned int length ) {
  num_vertices = get((unsigned short*)(*p));
  float *l = (float*)(*p + 2);

  vertex_list = new sgVec3[num_vertices];

  for (int i = 0; i < num_vertices; i++) {
    vertex_list[i][0] = get(l++);
    vertex_list[i][1] = get(l++);
    vertex_list[i][2] = get(l++);
  }

  *p = (char*)l;
  return PARSE_OK;
}

static int parse_smooth_list( char **p, unsigned int length )
{
  int i;
  unsigned int *w = (unsigned int*)*p;
  smooth_found = TRUE;

  smooth_list = new unsigned int[num_faces];

  for (i = 0; i < num_faces; i++)
    smooth_list[i] = get(w++);

  *p = (char*)w;

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

    if (double_sided) {
      int n = num_faces * 3;
      sgNegateVec3( vertex_normals[n + v1], vertex_normals[v1] );
      sgNegateVec3( vertex_normals[n + v2], vertex_normals[v2] );
      sgNegateVec3( vertex_normals[n + v3], vertex_normals[v3] );
    }

  }
}


static int parse_face_list( char **p, unsigned int length ) {
  int i;
  unsigned short *w = (unsigned short*)*p;     // to make less mess with dereferencing
  num_faces = get(w++);

  vertex_index   = new unsigned short[num_faces*3];
  face_normals   = new sgVec3[num_faces];

  if (!double_sided)
    vertex_normals = new sgVec3[num_faces * 3    ];
  else
    vertex_normals = new sgVec3[num_faces * 3 * 2];   // each side must have its normal :-(

  for (i = 0; i < num_faces; i++) {
    vertex_index[i*3    ] = get(w++);
    vertex_index[i*3 + 1] = get(w++);
    vertex_index[i*3 + 2] = get(w++);
    unsigned short flags  = get(w++);

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
  parse_chunks( (char*)w, *p + length - CHUNK_HEADER_SIZE, FaceListDataChunks );

  /* now apply correct smoothing. If smooth list has been found,
     use it, otherwise use threshold value. */
  smooth_normals( smooth_found );

  *p = (char*)w;
  return PARSE_OK;
}

static int parse_map_list( char **p, unsigned int length ) {
  unsigned short num_v = get((unsigned short*)*p);
  float *w = (float*)(*p + 2);

  texcrd_list = new sgVec2[num_v];

  for (int i = 0; i < num_v; i++) {
    texcrd_list[i][0] = get(w++);
    texcrd_list[i][1] = get(w++);
  }

  return PARSE_OK;
}

static int parse_tra_matrix( char **p, unsigned int length ) {
  sgMat4 m;

  sgMakeIdentMat4( m );

  /* Strange things seems to be going on with the
     local coordinate system in 3ds - I have commented
     this out, but things seems to work better without
     it (which is odd).
  
     float *f = (float*)*p;
     int i, j;

     for (i = 0; i < 4; i++) {
     for (j = 0; j < 3; j++) {
     m[i][j] = get(f++);
     }
     }


     for (int a = 0; a < 4; a++) {
     for (int b = 0; b < 4; b++) {
     printf("%6.2f  ", m[a][b]);
     }
     printf("\n");
     }

     printf("\n");*/

  current_transform -> setTransform( m );
  
  return PARSE_OK;
}

static int parse_face_materials( char **p, unsigned int length ) {
  int mat_num;
  char *material = *p;
  ssgSimpleState *state = NULL;

  while (**p) (*p)++;

  // find the material
  for (mat_num = 0; mat_num < num_materials; mat_num++) {
    if ( strcmp( material, material_names[mat_num] ) == 0 ) {
      state = materials[mat_num];
      break;
    }
  }

  unsigned short *w = (unsigned short*)(*p + 1);    // first character after string
  unsigned short listed_faces = get(w++);

  int is_ds       = mat_flag[mat_num] & IS_DOUBLESIDED, 
    has_texture = mat_flag[mat_num] & HAS_TEXTURE, 
    num_allocate;
  
  if (is_ds)
    num_allocate = listed_faces * 3 * 2; // material is double sided
  else
    num_allocate = listed_faces * 3;

  sgVec3 *vertices = new sgVec3[ num_allocate ];
  sgVec3 *normals  = new sgVec3[ num_allocate ];
  sgVec2 *texcrds  = NULL;
  
  if (has_texture) {
    if (texcrd_list == NULL) {
      fprintf(stderr, "ssgLoad3ds: Texture coords seems to be missing.\n");
    } else {
      texcrds = new sgVec2[ num_allocate ];  
    }
  }

  int vertex_count = 0,
    normal_count = 0,
    texcrd_count = 0;

  for (int i = 0; i < listed_faces; i++) {
    unsigned short faceindex = get(w++);
    int v1 = faceindex * 3,
      v2 = faceindex * 3 + 1,
      v3 = faceindex * 3 + 2;

    sgCopyVec3( vertices[vertex_count++], vertex_list[ vertex_index[v1] ] );
    sgCopyVec3( vertices[vertex_count++], vertex_list[ vertex_index[v2] ] );
    sgCopyVec3( vertices[vertex_count++], vertex_list[ vertex_index[v3] ] );

    sgCopyVec3( normals[normal_count++], vertex_normals[ v1 ] );
    sgCopyVec3( normals[normal_count++], vertex_normals[ v2 ] );
    sgCopyVec3( normals[normal_count++], vertex_normals[ v3 ] );

    if (has_texture && texcrd_list != NULL) {
      int num_texcrds = 3;

      sgCopyVec2( texcrds[texcrd_count    ], texcrd_list[ vertex_index[v1] ] );
      sgCopyVec2( texcrds[texcrd_count + 1], texcrd_list[ vertex_index[v2] ] );
      sgCopyVec2( texcrds[texcrd_count + 2], texcrd_list[ vertex_index[v3] ] );

      if (is_ds) {
        num_texcrds = 6;
        sgCopyVec2( texcrds[texcrd_count + 3], texcrd_list[ vertex_index[v1] ] );
        sgCopyVec2( texcrds[texcrd_count + 4], texcrd_list[ vertex_index[v3] ] );
        sgCopyVec2( texcrds[texcrd_count + 5], texcrd_list[ vertex_index[v2] ] );
      }

      for (int j = 0; j < num_texcrds; j++) {
        texcrds[texcrd_count + j][0] *= texture_scale[mat_num][0];
        texcrds[texcrd_count + j][1] *= texture_scale[mat_num][1];
        sgAddVec2( texcrds[texcrd_count + j], texture_offst[mat_num] );
      }

      texcrd_count += num_texcrds;
    }

    if (is_ds) {
      sgCopyVec3( vertices[vertex_count++], vertex_list[ vertex_index[v1] ] );
      sgCopyVec3( vertices[vertex_count++], vertex_list[ vertex_index[v3] ] );
      sgCopyVec3( vertices[vertex_count++], vertex_list[ vertex_index[v2] ] );

      sgCopyVec3( normals[normal_count++], vertex_normals[ v1 + num_faces*3 ] );
      sgCopyVec3( normals[normal_count++], vertex_normals[ v3 + num_faces*3 ] );
      sgCopyVec3( normals[normal_count++], vertex_normals[ v2 + num_faces*3 ] );
    }

  }

  ssgVTable *v = new ssgVTable( GL_TRIANGLES, vertex_count, vertices,
                                normal_count, normals,
                                texcrd_count, texcrds,
                                0, NULL);
  v -> setName( material );
  v -> setState( state );
  current_transform -> addKid( v );

  return PARSE_OK;
}

//==========================================================
// OBJBLOCK PARSER

static int parse_objblock( char **p, unsigned int length ) {
  while (*(*p)++);    // skip object name

  return PARSE_OK;
}


//==========================================================
// GENERAL CHUNK PARSER

static int parse_chunks(char *p, char *chunk_end, _ssg3dsChunk *chunk_list)
{
  int parse_ok = PARSE_OK;
  char *next_chunk;
  unsigned short id;
  unsigned int sub_length;
  _ssg3dsChunk *t;

  while (parse_ok && p < chunk_end) {
    id = get((unsigned short*)p);
    sub_length = get((unsigned int*)(p + 2));

    if (p + sub_length > chunk_end) {
      fprintf(stderr, "Illegal chunk %X of length %i. Chunk is longer than parent chunk.\n", (int)id, (int)sub_length);
      return PARSE_ERROR;
    }

    next_chunk = p + sub_length;    // p might be changed by parsing function
    p += CHUNK_HEADER_SIZE;

    for (t = chunk_list; t->id != 0 && t->id != id; t++);

    if (t->id == id) {
      // this is a known chunk

      // do chunk-specific parsing if available
      if (t->parse_func)
        parse_ok = t->parse_func( &p, sub_length );

      // if chunk can have subchunks, parse these
      if (t->subchunks && parse_ok) {
        parse_ok = parse_chunks( p, next_chunk, t->subchunks );
      }
    }

    // move to next chunk
    p = next_chunk;
  }

  return parse_ok;
}



ssgEntity *ssgLoad3ds( char *filename, ssgHookFunc hookfunc ) {
  int i = 1 ;
  is_little_endian = *((char *) &i );

  char *buffer, *filepath;
  FILE *loader_fd;

  if (filename[0] != '/' && _ssgModelPath[0] != '\0') {
    filepath = new char[strlen(filename) + strlen(_ssgModelPath) + 2];
    strcpy( filepath, _ssgModelPath);
    strcat( filepath, "/" );
    strcat( filepath, filename );
  } else {
    filepath = new char[strlen(filename) + 1];
    strcpy(filepath, filename);
  }
  
  loader_fd = fopen ( filepath, "rb" );

  if ( loader_fd == NULL ) {
    fprintf ( stderr, "ssgLoad3ds: Failed to open '%s' for reading\n", filepath ) ;
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
  materials = new ssgSimpleState*[MAX_MATERIALS];
  material_names = new char*[MAX_MATERIALS];
  mat_tfnames = new char*[MAX_MATERIALS];
  mat_flag = new int[MAX_MATERIALS];
  texture_scale = new sgVec2[MAX_MATERIALS]; 
  int _i;
  for ( _i=0; _i < MAX_MATERIALS; _i++ )
    texture_scale[_i][0] = texture_scale[_i][1] = 1.0f;
  texture_offst = new sgVec2[MAX_MATERIALS]; 
  for ( _i=0; _i < MAX_MATERIALS; _i++ )
    texture_offst[_i][0] = texture_offst[_i][1] = 0.0f;
  double_sided = FALSE;

  // Get file length and load into buffer
  fseek(loader_fd, 0, SEEK_END);
  long size = ftell(loader_fd);
  rewind(loader_fd);

  buffer = new char [ size ] ;

  if ( buffer == NULL )
    {
      fprintf( stderr, "ssgLoad3ds: Failed to allocate memory for file %s.\n",
               filepath);
      fclose ( loader_fd ) ;
      return NULL;
    }

  if ((long)fread(buffer, 1, size, loader_fd) < size)
    {
      fprintf(stderr, "ssgLoad3ds: Could not read file '%s' - size mismatch.\n",
              filepath );
      return NULL;
    }

  fclose ( loader_fd ) ;
  
  parse_chunks( buffer, buffer + size, TopChunk );

  delete [] buffer;
  delete [] filepath;
  
  delete [] material_names;
  delete [] mat_tfnames;
  delete [] mat_flag;
  delete [] texture_scale; 
  delete [] texture_offst; 

  free_trimesh();

  return top_object; 
}
