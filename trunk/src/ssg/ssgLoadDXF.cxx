//
// DXF import for SSG/PLIB
// Ported from IVCON by Dave McClurg (dpm@efn.org) in March-2000
//

#include "ssgLocal.h"

#define MAX_LINE_LEN 1024
#define MAX_VERT 100000

static ssgHookFunc      current_hookFunc = NULL ;
static ssgBranch       *current_branch   = NULL ;

/******************************************************************************/

static int dxf_read ( FILE *filein )

/******************************************************************************/

/*
  Purpose:
   
    DXF_READ reads an AutoCAD DXF file.

  Examples:

      0
    SECTION
      2
    HEADER
    999
    diamond.dxf created by IVREAD.
    999
    Original data in diamond.obj.
      0
    ENDSEC
      0
    SECTION
      2
    TABLES
      0
    ENDSEC
      0
    SECTION
      2
    BLOCKS
      0
    ENDSEC
      0
    SECTION
      2
    ENTITIES
      0
    LINE
      8
    0
     10
      0.00  (X coordinate of beginning of line.)
     20
      0.00  (Y coordinate of beginning of line.)
     30
      0.00  (Z coordinate of beginning of line.)
     11
      1.32  (X coordinate of end of line.)
     21
      1.73  (Y coordinate of end of line.)
     31
      2.25  (Z coordinate of end of line.)
      0
    3DFACE
      8
     Cube
    10
    -0.50  (X coordinate of vertex 1)
    20
     0.50  (Y coordinate of vertex 1)   
    30
      1.0  (Z coordinate of vertex 1)  
    11
     0.50  (X coordinate of vertex 2)  
    21
     0.50  (Y coordinate of vertex 2)
    31
      1.0  (Z coordinate of vertex 2)
    12
     0.50  (X coordinate of vertex 3) 
    22
     0.50  (Y coordinate of vertex 3)
    32
     0.00  (Z coordinate of vertex 3)
      0
    ENDSEC
      0
    EOF

  Modified:

    23 May 1999

  Author:
 
    John Burkardt
*/
{
  int   code;
  int   count;
  char  input1[MAX_LINE_LEN];
  char  input2[MAX_LINE_LEN];
  float rval;
  int   width;
  int   cpos;
  sgVec3 cvec;

  int   linemode;
  int   num_line;
  int   num_face;
  int   num_linevert;
  int   num_facevert;
  sgVec3* linevert;
  sgVec3* facevert;
  int   num_vert;

  linemode = 0;
  num_line = 0;
  num_face = 0;
  num_linevert = 0;
  num_facevert = 0;
  linevert = new sgVec3[MAX_VERT];
  facevert = new sgVec3[MAX_VERT];
  num_vert = 0;

/* 
  Read the next two lines of the file into INPUT1 and INPUT2. 
*/

  for ( ;; ) {

/* 
  INPUT1 should contain a single integer, which tells what INPUT2
  will contain.
*/
    if ( fgets ( input1, MAX_LINE_LEN, filein ) == NULL ) {
      break;
    }

    count = sscanf ( input1, "%d%n", &code, &width );
    if ( count <= 0 ) {
      break;
    }
/*
  Read the second line, and interpret it according to the code.
*/
    if ( fgets ( input2, MAX_LINE_LEN, filein ) == NULL ) {
      break;
    }

    if ( code == 0 ) {

      //flush verts
      if ( num_vert > 0 ) {
        if ( linemode ) {
          if ( num_vert >= 2 ) {
            num_line ++;
            num_linevert += 2;
          }
        }
        else {
          if ( num_vert >= 3 ) {
            //quad?
            if ( num_vert >= 4 && num_facevert + 6 < MAX_VERT ) {
              sgCopyVec3( facevert[num_facevert + 4], facevert[num_facevert + 1] );
              sgCopyVec3( facevert[num_facevert + 5], facevert[num_facevert + 2] );
              sgCopyVec3( facevert[num_facevert + 2], facevert[num_facevert + 3] );
              num_face += 2;
              num_facevert += 6;
            }
            else {
              //triangle
              num_face += 1;
              num_facevert += 3;
            }
          }
        }
        num_vert = 0;
      }
      
      if ( strncmp( input2, "LINE", 4 ) == 0 ) {
        linemode = 1;
      }
      else if ( strncmp( input2, "3DFACE", 6 ) == 0 ) {
        linemode = 0;
      }
    }
    else {
      
      for (cpos = 0; input1[cpos] == ' '; cpos++)
        ;
      
      if ( input1[cpos] == '1' || input1[cpos] == '2' || input1[cpos] == '3' ) {

        count = sscanf ( input2, "%e%n", &rval, &width );

        switch ( input1[cpos] )
        {
          case '1':
            cvec[0] = rval;
            break;

          case '2':
            cvec[1] = rval;
            break;

          case '3':
            cvec[2] = rval;

            if ( linemode ) {
              if ( num_linevert + num_vert < MAX_VERT ) {
                sgCopyVec3( linevert[num_linevert + num_vert], cvec );
              }
            }
            else {
              if ( num_facevert + num_vert < MAX_VERT ) {
                sgCopyVec3( facevert[num_facevert + num_vert], cvec );
              }
            }

            num_vert = num_vert + 1;
            break;
        
          default:
            break;
        }
      }
    }
  }

  //flush verts
  if ( num_vert > 0 ) {
    if ( linemode ) {
      if ( num_vert == 2 ) {
        num_line ++;
        num_linevert += 2;
      }
    }
    else {
      if ( num_vert == 3 ) {
        num_face ++;
        num_facevert += 3;
      }
    }
    num_vert = 0;
  }

  //create ssg nodes
  if ( num_face )
  {
    ssgVertexArray* vlist = new ssgVertexArray ( num_facevert ) ;
    for ( int i=0; i<num_facevert; i++ )
      vlist -> add ( facevert[i] ) ;
    ssgVtxTable *vtab = new ssgVtxTable ( GL_TRIANGLES, vlist, 0, 0, 0 );
    current_branch -> addKid ( vtab ) ;
  }

  if ( num_line )
  {
    ssgVertexArray* vlist = new ssgVertexArray ( num_linevert ) ;
    for ( int i=0; i<num_linevert; i++ )
      vlist -> add ( linevert[i] ) ;
    ssgVtxTable *vtab = new ssgVtxTable ( GL_LINES, vlist, 0, 0, 0 );
    current_branch -> addKid ( vtab ) ;
  }

  delete[] linevert;
  delete[] facevert;

  return TRUE;
}

ssgEntity *ssgLoadDXF ( char *fname, ssgHookFunc hookfunc )
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
    fprintf ( stderr, "ssgLoadDXF: Failed to open '%s' for reading\n", filename ) ;
    return NULL ;
  }

  current_branch = new ssgTransform () ;

  dxf_read ( loader_fd ) ;

  fclose ( loader_fd ) ;

  return current_branch ;
}
