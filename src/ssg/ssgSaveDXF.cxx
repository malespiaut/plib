//
// DXF export for SSG/PLIB
// Ported from IVCON by Dave McClurg (dpm@efn.org) in March-2000
//

#include "ssgLocal.h"

static FILE *fileout ;

static void save_vtx_table ( ssgVtxTable *vt )
{
  GLenum mode = vt -> getGLtype () ;
  if (( mode == GL_LINES ) || ( mode == GL_LINE_LOOP) || ( mode == GL_LINE_STRIP))
  {
    int num_vert = vt -> getNumVertices () ;
    num_vert = num_vert - ( num_vert & 1 ) ; //discard odd vertex
		int local_num_lines = vt -> getNumLines();

    for ( int j = 0; j < local_num_lines ; j ++ )
    {
      sgVec3 vert1, vert2;
			short iv1, iv2;
			vt -> getLine (j, &iv1, &iv2);

      sgCopyVec3 ( vert1, vt->getVertex ( iv1 ) ) ;
      sgCopyVec3 ( vert2, vt->getVertex ( iv2 ) ) ;

      fprintf ( fileout, "  0\n" );
      fprintf ( fileout, "LINE\n" );
      fprintf ( fileout, "  8\n" );
      fprintf ( fileout, "  0\n" );
      fprintf ( fileout, " 10\n" );
      fprintf ( fileout, "%f\n", vert1[0] );
      fprintf ( fileout, " 20\n" );
      fprintf ( fileout, "%f\n", vert1[1] );
      fprintf ( fileout, " 30\n" );
      fprintf ( fileout, "%f\n", vert1[2] );
      fprintf ( fileout, " 11\n" );
      fprintf ( fileout, "%f\n", vert2[0] );
      fprintf ( fileout, " 21\n" );
      fprintf ( fileout, "%f\n", vert2[1] );
      fprintf ( fileout, " 31\n" );
      fprintf ( fileout, "%f\n", vert2[2] );
    }
  }
  else if ( mode == GL_TRIANGLES ||
    mode == GL_TRIANGLE_FAN ||
    mode == GL_TRIANGLE_STRIP )
  {
    int num_face = vt -> getNumTriangles () ;
    for ( int j = 0; j < num_face; j++ )
    {
      short face[3];
      vt -> getTriangle ( j, &face[0], &face[1], &face[2] ) ;

      fprintf ( fileout, "  0\n" );
      fprintf ( fileout, "3DFACE\n" );
      fprintf ( fileout, "  8\n" );
      fprintf ( fileout, "  Cube\n" );
    
      for ( int ivert = 0; ivert < 3; ivert++ ) {

        sgVec3 vert;
        sgCopyVec3 ( vert, vt->getVertex ( face[ivert] ) ) ;
   
        fprintf ( fileout, "1%d\n", ivert );
        fprintf ( fileout, "%f\n", vert[0] );
        fprintf ( fileout, "2%d\n", ivert );
        fprintf ( fileout, "%f\n", vert[1] );
        fprintf ( fileout, "3%d\n", ivert );
        fprintf ( fileout, "%f\n", vert[2] );
      }
    }
  }
	else
		ulSetError ( UL_WARNING, "ssgSaveDXF: OpenGL mode %d not implmented yet. Parts or all of the model are ignored!' for writing", (int)mode ) ;
}


static void save_entities ( ssgEntity *e )
{
  if ( e -> isAKindOf ( SSG_TYPE_BRANCH ) )
  {
    ssgBranch *br = (ssgBranch *) e ;

    for ( int i = 0 ; i < br -> getNumKids () ; i++ )
      save_entities ( br -> getKid ( i ) ) ;
  }
  else
  if ( e -> isAKindOf ( SSG_TYPE_VTXTABLE ) )
  {
    ssgVtxTable *vt = (ssgVtxTable *) e ;
    save_vtx_table ( vt ) ;
  }
}


/******************************************************************************/

int ssgSaveDXF ( const char *filename, ssgEntity *ent )

/******************************************************************************/

/*
  Purpose:
   
    DXF_WRITE writes graphics information to an AutoCAD DXF file.

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

    16 May 1999

  Author:
 
    John Burkardt
*/
{
  fileout = fopen ( filename, "wa" ) ;

  if ( fileout == NULL )
  {
    ulSetError ( UL_WARNING, "ssgSaveDXF: Failed to open '%s' for writing", filename ) ;
    return FALSE ;
  }

/* 
  Initialize. 
*/
  fprintf ( fileout, "  0\n" );
  fprintf ( fileout, "SECTION\n" );
  fprintf ( fileout, "  2\n" );
  fprintf ( fileout, "HEADER\n" );
  fprintf ( fileout, "999\n" );
  fprintf ( fileout, "%s created by SSG.\n", filename );
  fprintf ( fileout, "999\n" );
  fprintf ( fileout, "Original data in null.\n" );
  fprintf ( fileout, "  0\n" );
  fprintf ( fileout, "ENDSEC\n" );

  fprintf ( fileout, "  0\n" );
  fprintf ( fileout, "SECTION\n" );
  fprintf ( fileout, "  2\n" );
  fprintf ( fileout, "TABLES\n" );
  fprintf ( fileout, "  0\n" );
  fprintf ( fileout, "ENDSEC\n" );

  fprintf ( fileout, "  0\n" );
  fprintf ( fileout, "SECTION\n" );
  fprintf ( fileout, "  2\n" );
  fprintf ( fileout, "BLOCKS\n" );
  fprintf ( fileout, "  0\n" );
  fprintf ( fileout, "ENDSEC\n" );

  fprintf ( fileout, "  0\n" );
  fprintf ( fileout, "SECTION\n" );
  fprintf ( fileout, "  2\n" );
  fprintf ( fileout, "ENTITIES\n" );

  save_entities ( ent ) ;

  fprintf ( fileout, "  0\n" );
  fprintf ( fileout, "ENDSEC\n" );
  fprintf ( fileout, "  0\n" );
  fprintf ( fileout, "EOF\n" );
/*
  Close.
*/
  fclose ( fileout ) ;
  return TRUE;
}
