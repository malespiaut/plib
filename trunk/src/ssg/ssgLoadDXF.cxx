//
// DXF import for SSG/PLIB
// Ported from IVCON by Dave McClurg (dpm@efn.org) in March-2000
// dxf_read() by John Burkardt, 23 May 1999
//

#include "ssgLocal.h"


#define MAX_LINE_LEN 1024
#define MAX_VERT 100000

enum Modes { MODE_NONE, MODE_LINE, MODE_FACE, MODE_POLYLINE, MODE_VERTEX };

static const ssgLoaderOptions* current_options = NULL ;
static ssgBranch       *current_branch   = NULL ;

static int num_line;
static int num_linevert;
static sgVec3* linevert;

static int num_face;
static int num_facevert;
static sgVec3* facevert;

static int num_meshvert;
static sgVec3* meshvert;
static int meshface[4];
static int meshflags;
static int meshsize[2];

static int mode;
static int num_vert;
static int cflags;
static sgVec3 cvec;

static void dxf_flush ( void )
{
  if ( mode == MODE_LINE ) {
    if ( num_vert >= 2 ) {
      num_line ++;
      num_linevert += 2;
    }
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
  }
  else if ( mode == MODE_POLYLINE ) {
    meshflags = cflags;
    num_meshvert = 0;
    meshsize[0] = meshface[0];
    meshsize[1] = meshface[1];
  }
  else if ( mode == MODE_VERTEX ) {

    if ( (meshflags & 8) != 0 ) {

      //This is a 3D Polyline
      if ( (cflags & 32) != 0 ) {

        //This is a 3D Polyline vertex
        if ( num_meshvert < MAX_VERT ) {
          sgCopyVec3( meshvert[num_meshvert], cvec );
          num_meshvert ++ ;
        }
      }
    }
    else if ( (meshflags & 16) != 0 ) {

      //This is a 3D polygon MxN mesh. (uniform grid)
      if ( (cflags & 64) != 0 ) {

        //This is a 3D polygon mesh vertex
        if ( num_meshvert < MAX_VERT ) {
          sgCopyVec3( meshvert[num_meshvert], cvec );
          num_meshvert ++ ;
        }
      }
    }
    else if ( (meshflags & 64) != 0 ) {

      //This Polyline is a polyface mesh.
      if ( (cflags & 128) != 0 ) {

        if ( (cflags & 64) != 0 ) {

          //This is a 3D polygon mesh vertex
          if ( num_meshvert < MAX_VERT ) {
            sgCopyVec3( meshvert[num_meshvert], cvec );
            num_meshvert ++ ;
          }
        }
        else if ( num_vert >= 3 && num_facevert + num_vert < MAX_VERT ) {

          //copy each vertex where the first is numbered 1
          int error = 0;
          for ( int i=0; i<num_vert; i++ )
          {
            int ival = meshface[i];
            if ( ival < 0 )
              ival = -ival;  //invisible vertex whatever that means :)
            if ( ival > 0 && ival <= num_meshvert )
              sgCopyVec3( facevert[num_facevert + i], meshvert[ival-1] );
            else
              error = 1;
          }

          if ( error == 0 ) {
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
      }
    }
  }

  num_vert = 0;
  cflags = 0;
  cvec[0] = 0.0f;
  cvec[1] = 0.0f;
  cvec[2] = 0.0f;
  meshface[0] = 0;
  meshface[1] = 0;
  meshface[2] = 0;
  meshface[3] = 0;
}


static int dxf_read ( FILE *filein )
{
/*
initialize lists
*/
  num_line = 0;
  num_linevert = 0;
  linevert = new sgVec3[MAX_VERT];

  num_face = 0;
  num_facevert = 0;
  facevert = new sgVec3[MAX_VERT];

  num_meshvert = 0;
  meshvert = new sgVec3[MAX_VERT];

  mode = MODE_NONE;
  num_vert = 0;

/* 
  Read the next two lines of the file into INPUT1 and INPUT2. 
*/
  for ( ;; ) {

    int   count;
    int   code;
    int   width;
    char  input1[MAX_LINE_LEN];
    char  input2[MAX_LINE_LEN];

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

      //set mode
      if ( strncmp( input2, "LINE", 4 ) == 0 )
        mode = MODE_LINE ;
      else if ( strncmp( input2, "3DFACE", 6 ) == 0 )
        mode = MODE_FACE ;
      else if ( strncmp( input2, "POLYLINE", 8 ) == 0 )
        mode = MODE_POLYLINE ;
      else if ( strncmp( input2, "VERTEX", 6 ) == 0 )
        mode = MODE_VERTEX;
      else if ( strncmp( input2, "SEQEND", 6 ) == 0 ) {

        if ( (meshflags & 8) != 0 ) {

          //This is a 3D Polyline
          int last = 0;
          int i = 1;

          if ( (meshflags & 33) != 0 ) {

            //Polyline is closed
            last = num_meshvert - 1;
            i = 0;
          }

          for ( ; i<num_meshvert; i++ ) {

            if ( num_linevert + 2 < MAX_VERT ) {

              sgCopyVec3( linevert[num_linevert], meshvert[last] );
              sgCopyVec3( linevert[num_linevert+1], meshvert[i] );
              num_line ++;
              num_linevert += 2;
            }
          }
        }
        else if ( (meshflags & 16) != 0 ) {

          //This is a 3D polygon MxN mesh. (uniform grid)
          if ( num_meshvert >= ( meshsize[0] * meshsize[1] ) ) {

            for ( int i=1; i<meshsize[0]; i++ )
            for ( int j=1; j<meshsize[1]; j++ ) {

              if ( num_facevert + 3 < MAX_VERT ) {
                sgCopyVec3( facevert[num_facevert], meshvert[(i-1)+(j-1)*meshsize[0]] );
                sgCopyVec3( facevert[num_facevert+1], meshvert[i+(j-1)*meshsize[0]] );
                sgCopyVec3( facevert[num_facevert+2], meshvert[i+j*meshsize[0]] );
                num_face ++;
                num_facevert += 3;
              }

              if ( num_facevert + 3 < MAX_VERT ) {
                sgCopyVec3( facevert[num_facevert], meshvert[(i-1)+(j-1)*meshsize[0]] );
                sgCopyVec3( facevert[num_facevert+1], meshvert[i+j*meshsize[0]] );
                sgCopyVec3( facevert[num_facevert+2], meshvert[(i-1)+j*meshsize[0]] );
                num_face ++;
                num_facevert += 3;
              }
            }

            if ( (meshflags & 1) != 0 ) {

              //The polygon mesh is closed in the M direction
              for ( int j=1; j<meshsize[1]; j++ ) {
  
                int i1 = meshsize[0]-1;
                int i2 = 0;

                if ( num_facevert + 3 < MAX_VERT ) {
                  sgCopyVec3( facevert[num_facevert], meshvert[i1+(j-1)*meshsize[0]] );
                  sgCopyVec3( facevert[num_facevert+1], meshvert[i2+(j-1)*meshsize[0]] );
                  sgCopyVec3( facevert[num_facevert+2], meshvert[i2+j*meshsize[0]] );
                  num_face ++;
                  num_facevert += 3;
                }
   
                if ( num_facevert + 3 < MAX_VERT ) {
                  sgCopyVec3( facevert[num_facevert], meshvert[i1+(j-1)*meshsize[0]] );
                  sgCopyVec3( facevert[num_facevert+1], meshvert[i2+j*meshsize[0]] );
                  sgCopyVec3( facevert[num_facevert+2], meshvert[i1+j*meshsize[0]] );
                  num_face ++;
                  num_facevert += 3;
                }
              }
            }

            if ( (meshflags & 32) != 0 ) {

              //The polygon mesh is closed in the N direction
              for ( int i=1; i<meshsize[0]; i++ ) {

                int j1 = meshsize[1]-1;
                int j2 = 0;

                if ( num_facevert + 3 < MAX_VERT ) {
                  sgCopyVec3( facevert[num_facevert], meshvert[(i-1)+j1*meshsize[0]] );
                  sgCopyVec3( facevert[num_facevert+1], meshvert[i+j1*meshsize[0]] );
                  sgCopyVec3( facevert[num_facevert+2], meshvert[i+j2*meshsize[0]] );
                  num_face ++;
                  num_facevert += 3;
                }

                if ( num_facevert + 3 < MAX_VERT ) {
                  sgCopyVec3( facevert[num_facevert], meshvert[(i-1)+j1*meshsize[0]] );
                  sgCopyVec3( facevert[num_facevert+1], meshvert[i+j2*meshsize[0]] );
                  sgCopyVec3( facevert[num_facevert+2], meshvert[(i-1)+j2*meshsize[0]] );
                  num_face ++;
                  num_facevert += 3;
                }
              }
            }
          }
        }

        meshflags = 0;
        mode = MODE_NONE;
      }
      else
        mode = MODE_NONE;
    }
    else {
      
      int cpos;
      for (cpos = 0; input1[cpos] == ' '; cpos++)
        ;
      
      if ( input1[cpos] == '1' || input1[cpos] == '2' || input1[cpos] == '3' ) {

        float rval;
        count = sscanf ( input2, "%e%n", &rval, &width );

        char ch = input1[cpos+1];
        if ( ch == '0' || ch == '1' || ch == '2' || ch == '3' )
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
            break;
        }
      }
      else if ( input1[cpos] == '7' ) {

        int ival ;
        count = sscanf ( input2, "%d%n", &ival, &width );

        switch ( input1[cpos+1] )
        {
          case '0':
            cflags = ival;
            break;

          case '1':
            meshface[0] = ival;
            num_vert = 1;
            break;

          case '2':
            meshface[1] = ival;
            num_vert = 2;
            break;

          case '3':
            meshface[2] = ival;
            num_vert = 3;
            break;

          case '4':
            meshface[3] = ival;
            num_vert = 4;
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
  delete[] meshvert;

  return TRUE;
}

ssgEntity *ssgLoadDXF ( const char *fname, const ssgLoaderOptions* options )
{
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
    ulSetError ( UL_WARNING, "ssgLoadDXF: Failed to open '%s' for reading", filename ) ;
    return NULL ;
  }

  current_options = options? options: &_ssgDefaultOptions ;
  current_options -> begin () ;

  current_branch = new ssgTransform () ;

  dxf_read ( loader_fd ) ;

  fclose ( loader_fd ) ;

  current_options -> end () ;

  return current_branch ;
}
