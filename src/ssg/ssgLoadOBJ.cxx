//
// Wavefront OBJ import for SSG/PLIB
// Ported from IVCON by Dave McClurg (dpm@efn.org) in March-2000
//

#include "ssgLocal.h"

#define MAX_LINE_LEN 1024
#define MAX_LINE 100000
#define MAX_FACE 100000
#define MAX_VERT 100000
#define MAX_ORDER 3

static ssgHookFunc      current_hookFunc = NULL ;
static ssgBranch       *current_branch   = NULL ;


struct faceData
{
   int vlist [ MAX_ORDER ] ;
   int nlist [ MAX_ORDER ] ;
} ;


/******************************************************************************/

static int leqi ( char* string1, char* string2 )

/******************************************************************************/

/*
  Purpose:

    LEQI compares two strings for equality, disregarding case.

  Modified:

    15 September 1998

  Author:
 
    John Burkardt
*/
{
  int i;
  int nchar;
  int nchar1;
  int nchar2;

  nchar1 = strlen ( string1 );
  nchar2 = strlen ( string2 );

  if ( nchar1 < nchar2 ) {
    nchar = nchar1;
  }
  else {
    nchar = nchar2;
  }
/*
  The strings are not equal if they differ over their common length.
*/
  for ( i = 0; i < nchar; i++ ) {

    if ( toupper ( string1[i] ) != toupper ( string2[i] ) ) {
      return FALSE;
    }
  }
/*
  The strings are not equal if the longer one includes nonblanks
  in the tail.
*/
  if ( nchar1 > nchar ) {
    for ( i = nchar; i < nchar1; i++ ) {
      if ( string1[i] != ' ' ) {
        return FALSE;
      }
    } 
  }
  else if ( nchar2 > nchar ) {
    for ( i = nchar; i < nchar2; i++ ) {
      if ( string2[i] != ' ' ) {
        return FALSE;
      }
    } 
  }
  return TRUE;
}
/******************************************************************************/

static int obj_read ( FILE *filein )

/******************************************************************************/

/*
  Purpose:
   
    OBJ_READ reads a Wavefront OBJ file.

  Example:

    #  magnolia.obj

    mtllib ./vp.mtl

    g
    v -3.269770 -39.572201 0.876128
    v -3.263720 -39.507999 2.160890
    ...
    v 0.000000 -9.988540 0.000000
    g stem
    s 1
    usemtl brownskn
    f 8 9 11 10
    f 12 13 15 14
    ...
    f 788 806 774

  Modified:

    20 October 1998

  Author:
 
    John Burkardt
*/
{
  int   count;
  int   ivert;
  char *next;
  char *next2;
  char *next3;
  int   node;
  float r1;
  float r2;
  float r3;
  char  input[MAX_LINE_LEN];
  char  token[MAX_LINE_LEN];
  char  token2[MAX_LINE_LEN];
  int   width;
/* 
  Initialize. 
*/
  int num_line = 0 ;
  int num_face = 0 ;
  int num_vert = 0 ;
  int num_vert_normal = 0 ;
  int* line_dex = new int[ MAX_LINE ] ;
  faceData* face = new faceData[ MAX_FACE ] ;
  sgVec3* vert = new sgVec3[ MAX_VERT ] ;
  sgVec3* vert_normal = new sgVec3[ MAX_VERT ] ;

/* 
  Read the next line of the file into INPUT. 
*/
  while ( fgets ( input, MAX_LINE_LEN, filein ) != NULL ) {

/* 
  Advance to the first nonspace character in INPUT. 
*/
    for ( next = input; *next != '\0' && isspace(*next); next++ ) {
    }
/* 
  Skip blank lines and comments. 
*/

    if ( *next == '\0' ) {
      continue;
    }

    if ( *next == '#' || *next == '$' ) {
      continue;
    }
/* 
  Extract the first word in this line. 
*/
    sscanf ( next, "%s%n", token, &width );
/* 
  Set NEXT to point to just after this token. 
*/

    next = next + width;
/*
  BEVEL
  Bevel interpolation.
*/
    if ( leqi ( token, "BEVEL" ) == TRUE ) {
      continue;
    }
/*
  BMAT
  Basis matrix.
*/
    else if ( leqi ( token, "BMAT" ) == TRUE ) {
      continue;
    }
/*
  C_INTERP
  Color interpolation.
*/
    else if ( leqi ( token, "C_INTERP" ) == TRUE ) {
      continue;
    }
/*
  CON
  Connectivity between free form surfaces.
*/
    else if ( leqi ( token, "CON" ) == TRUE ) {
      continue;
    }
/*
  CSTYPE
  Curve or surface type.
*/
    else if ( leqi ( token, "CSTYPE" ) == TRUE ) {
      continue;
    }
/*
  CTECH
  Curve approximation technique.
*/
    else if ( leqi ( token, "CTECH" ) == TRUE ) {
      continue;
    }
/*
  CURV
  Curve.
*/
    else if ( leqi ( token, "CURV" ) == TRUE ) {
      continue;
    }
/*
  CURV2
  2D curve.
*/
    else if ( leqi ( token, "CURV2" ) == TRUE ) {
      continue;
    }
/*
  D_INTERP
  Dissolve interpolation.
*/
    else if ( leqi ( token, "D_INTERP" ) == TRUE ) {
      continue;
    }
/*
  DEG
  Degree.
*/
    else if ( leqi ( token, "DEG" ) == TRUE ) {
      continue;
    }
/*
  END
  End statement.
*/
    else if ( leqi ( token, "END" ) == TRUE ) {
      continue;
    }
/*  
  F V1 V2 V3
    or
  F V1/VT1/VN1 V2/VT2/VN2 ...
    or
  F V1//VN1 V2//VN2 ...

  Face.
  A face is defined by the vertices.
  Optionally, slashes may be used to include the texture vertex
  and vertex normal indices.

  OBJ line node indices are 1 based rather than 0 based.
  So we have to decrement them before loading them into FACE.
*/

    else if ( leqi ( token, "F" ) == TRUE ) {

      ivert = 0;
/*
  Read each item in the F definition as a token, and then
  take it apart.
*/
      for ( ;; ) {

        count = sscanf ( next, "%s%n", token2, &width );
        next = next + width;
 
        if ( count != 1 ) {
          break;
        }
 
        count = sscanf ( token2, "%d%n", &node, &width );
        next2 = token2 + width;

        if ( count != 1 ) {
          break;
        }

        face[num_face].vlist[ivert] = 0 ;
        face[num_face].nlist[ivert] = 0 ;

        if ( ivert < MAX_ORDER && num_face < MAX_FACE ) {
          face[num_face].vlist[ivert] = node-1;
        } 
/*
  If there's a slash, skip to the next slash, and extract the
  index of the normal vector.
*/
        if ( *next2 == '/' ) {

          for ( next3 = next2 + 1; next3 < token2 + MAX_LINE_LEN; next3++ ) {

            if ( *next3 == '/' ) {
              next3 = next3 + 1;
              count = sscanf ( next3, "%d%n", &node, &width );

              if ( 0 < node && node <= num_vert_normal ) {
                face[num_face].nlist[ivert] = node-1;
              }
              break;
            }
          }
        }
        ivert = ivert + 1;
      }

      if ( ivert == MAX_ORDER ) {
         num_face = num_face + 1;
      }
    }

/*  
  G  
  Group name.
*/

    else if ( leqi ( token, "G" ) == TRUE ) {
      continue;
    }
/*
  HOLE
  Inner trimming hole.
*/
    else if ( leqi ( token, "HOLE" ) == TRUE ) {
      continue;
    }
/*  
  L  
  I believe OBJ line node indices are 1 based rather than 0 based.
  So we have to decrement them before loading them into LINE_DEX.
*/

    else if ( leqi ( token, "L" ) == TRUE ) {

      int lastnode = -1 ;

      for ( ;; ) {

        count = sscanf ( next, "%d%n", &node, &width );
        next = next + width;

        if ( count != 1 ) {
          break;
        }

        if ( lastnode != -1 && num_line + 2 <= MAX_LINE  ) {

          line_dex[num_line] = lastnode-1;
          num_line = num_line + 1;
          line_dex[num_line] = node-1;
          num_line = num_line + 1;
        }

        lastnode = node ;
      } 
    }

/*
  LOD
  Level of detail.
*/
    else if ( leqi ( token, "LOD" ) == TRUE ) {
      continue;
    }
/*
  MG
  Merging group.
*/
    else if ( leqi ( token, "MG" ) == TRUE ) {
      continue;
    }
/*
  MTLLIB
  Material library.
*/

    else if ( leqi ( token, "MTLLIB" ) == TRUE ) {
      continue;
    }
/*
  O
  Object name.
*/
    else if ( leqi ( token, "O" ) == TRUE ) {
      continue;
    }
/*
  P
  Point.
*/
    else if ( leqi ( token, "P" ) == TRUE ) {
      continue;
    }
/*
  PARM
  Parameter values.
*/
    else if ( leqi ( token, "PARM" ) == TRUE ) {
      continue;
    }
/*
  S  
  Smoothing group
*/
    else if ( leqi ( token, "S" ) == TRUE ) {
      continue;
    }
/*
  SCRV
  Special curve.
*/
    else if ( leqi ( token, "SCRV" ) == TRUE ) {
      continue;
    }
/*
  SHADOW_OBJ
  Shadow casting.
*/
    else if ( leqi ( token, "SHADOW_OBJ" ) == TRUE ) {
      continue;
    }
/*
  SP
  Special point.
*/
    else if ( leqi ( token, "SP" ) == TRUE ) {
      continue;
    }
/*
  STECH
  Surface approximation technique.
*/
    else if ( leqi ( token, "STECH" ) == TRUE ) {
      continue;
    }
/*
  STEP
  Stepsize.
*/
    else if ( leqi ( token, "CURV" ) == TRUE ) {
      continue;
    }
/*
  SURF
  Surface.
*/
    else if ( leqi ( token, "SURF" ) == TRUE ) {
      continue;
    }
/*
  TRACE_OBJ
  Ray tracing.
*/
    else if ( leqi ( token, "TRACE_OBJ" ) == TRUE ) {
      continue;
    }
/*
  TRIM
  Outer trimming loop.
*/
    else if ( leqi ( token, "TRIM" ) == TRUE ) {
      continue;
    }
/*
  USEMTL  
  Material name.
*/
    else if ( leqi ( token, "USEMTL" ) == TRUE ) {
      continue;
    }

/*
  V X Y Z W
  Geometric vertex.
  W is optional, a weight for rational curves and surfaces.
  The default for W is 1.
*/

    else if ( leqi ( token, "V" ) == TRUE ) {

      sscanf ( next, "%e %e %e", &r1, &r2, &r3 );

      if ( num_vert < MAX_VERT ) {
        vert[num_vert][0] = r1;
        vert[num_vert][1] = r2;
        vert[num_vert][2] = r3;

        num_vert = num_vert + 1;
      }
    }
/*
  VN
  Vertex normals.
*/

    else if ( leqi ( token, "VN" ) == TRUE ) {

      sscanf ( next, "%e %e %e", &r1, &r2, &r3 );

      if ( num_vert_normal < MAX_VERT ) {
        vert_normal[num_vert_normal][0] = r1;
        vert_normal[num_vert_normal][1] = r2;
        vert_normal[num_vert_normal][2] = r3;

        num_vert_normal = num_vert_normal + 1;
      }
    }
/*
  VT
  Vertex texture.
*/
    else if ( leqi ( token, "VT" ) == TRUE ) {
      continue;
    }
/*
  VP
  Parameter space vertices.
*/
    else if ( leqi ( token, "VP" ) == TRUE ) {
      continue;
    }
  }

  //create ssg nodes
  if ( num_face )
  {
    ssgVertexArray* vlist = new ssgVertexArray ( num_face * 3 ) ;
    for ( int i=0; i<num_face; i++ )
    for ( int j=0; j<3; j++ )
       vlist -> add ( vert[ face[i].vlist[j] ] ) ;
    ssgVtxTable *vtab = new ssgVtxTable ( GL_TRIANGLES, vlist, 0, 0, 0 );
    current_branch -> addKid ( vtab ) ;
  }

  if ( num_line )
  {
    ssgVertexArray* vlist = new ssgVertexArray ( num_line ) ;
    for ( int i=0; i<num_line; i++ )
      vlist -> add ( vert[ line_dex[i] ] ) ;
    ssgVtxTable *vtab = new ssgVtxTable ( GL_LINES, vlist, 0, 0, 0 );
    current_branch -> addKid ( vtab ) ;
  }

  delete[] line_dex ;
  delete[] face ;
  delete[] vert ;
  delete[] vert_normal ;

  return TRUE;
}

ssgEntity *ssgLoadOBJ ( char *fname, ssgHookFunc hookfunc )
{
  current_hookFunc = hookfunc ;
  current_branch   = NULL ;

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

  FILE *loader_fd = fopen ( filename, "ra" ) ;

  if ( loader_fd == NULL )
  {
    perror ( filename ) ;
    fprintf ( stderr, "ssgLoadOBJ: Failed to open '%s' for reading\n", filename ) ;
    return NULL ;
  }

  current_branch = new ssgTransform () ;

  obj_read ( loader_fd ) ;

  fclose ( loader_fd ) ;

  return current_branch ;
}
