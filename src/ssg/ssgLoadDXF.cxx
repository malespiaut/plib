//
// DXF import for SSG/PLIB
// Ported from IVCON by Dave McClurg (dpm@efn.org) in March-2000
// dxf_read() by John Burkardt, 23 May 1999
//

#include "ssgLocal.h"


#define MAX_LINE_LEN 1024
#define MAX_VERT 100000

enum Modes { MODE_NONE, MODE_LINE, MODE_FACE, MODE_POLYLINE };

static ssgHookFunc      current_hookFunc = NULL ;
static ssgBranch       *current_branch   = NULL ;

static int   num_line;
static int   num_face;
static int   num_linevert;
static int   num_facevert;
static sgVec3* linevert;
static sgVec3* facevert;

static int   mode;
static int   num_vert;


static void dxf_flush ( void )
{
  if ( num_vert > 0 ) {
    if ( mode == MODE_LINE ) {
      if ( num_vert >= 2 ) {
        num_line ++;
        num_linevert += 2;
      }
      num_vert = 0;
    }
    else if ( mode == MODE_FACE ) {
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
      num_vert = 0;
    }
  }
}


static int dxf_read ( FILE *filein )
{
  int   code;
  int   count;
  char  input1[MAX_LINE_LEN];
  char  input2[MAX_LINE_LEN];
  float rval;
  int   width;
  int   cpos;
  sgVec3 cvec;

  mode = MODE_NONE;
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

      dxf_flush () ;
      
      if ( strncmp( input2, "LINE", 4 ) == 0 ) {
        mode = MODE_LINE ;
        num_vert = 0 ;
      }
      else if ( strncmp( input2, "3DFACE", 6 ) == 0 ) {
        mode = MODE_FACE ;
        num_vert = 0 ;
      }
      else if ( strncmp( input2, "POLYLINE", 8 ) == 0 ) {
        mode = MODE_POLYLINE ;
        num_vert = 0 ;
      }
      else if ( strncmp( input2, "VERTEX", 6 ) == 0 ) {
        if ( mode != MODE_POLYLINE ) {
          mode = MODE_NONE;
          num_vert = 0 ;
        }
      }
      else if ( strncmp( input2, "SEQEND", 6 ) == 0 ) {
        if ( mode == MODE_POLYLINE ) {
          num_vert = ( num_vert / 2 ) * 2 ;  //make even
          if ( num_vert > 1 ) {
            num_line += ( num_vert / 2 ) ;
            num_linevert += num_vert ;
          }
          num_vert = 0;
        }
        mode = MODE_NONE;
        num_vert = 0;
      }
      else {
        mode = MODE_NONE;
        num_vert = 0;
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

            if ( mode == MODE_LINE ) {
              if ( num_linevert + num_vert < MAX_VERT ) {
                sgCopyVec3( linevert[num_linevert + num_vert], cvec );
                num_vert ++ ;
              }
            }
            else if ( mode == MODE_FACE ) {
              if ( num_facevert + num_vert < MAX_VERT ) {
                sgCopyVec3( facevert[num_facevert + num_vert], cvec );
                num_vert ++ ;
              }
            }
            else if ( mode == MODE_POLYLINE ) {
              if ( num_linevert + num_vert < MAX_VERT ) {
                sgCopyVec3( linevert[num_linevert + num_vert], cvec );
                num_vert ++ ;
              }
              if ( num_vert >= 2 && num_linevert + num_vert < MAX_VERT ) {
                //duplicate vertex if not first
                sgCopyVec3( linevert[num_linevert + num_vert], cvec );
                num_vert ++ ;
              }
            }

            break;
        
          default:
            break;
        }
      }
    }
  }

  dxf_flush () ;

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
