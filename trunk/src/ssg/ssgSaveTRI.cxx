//
// TRI ( AC3D triangle file ) export for SSG/PLIB
// Written by Dave McClurg (dpm@efn.org) in March-2000
//

#include "ssgLocal.h"

static FILE *fileout ;


static void save_vtx_table ( ssgVtxTable *vt )
{
  GLenum mode = vt -> getGLtype () ;
  if ( mode == GL_TRIANGLES ||
    mode == GL_TRIANGLE_FAN ||
    mode == GL_TRIANGLE_STRIP )
  {
    int num_tri = vt -> getNumTriangles () ;
    for ( int i = 0; i < num_tri; i++ )
    {
      short tri[3];
      vt -> getTriangle ( i, &tri[0], &tri[1], &tri[2] ) ;

      for ( int j = 0; j < 3; j ++ )
      {
        sgVec3 vert;
        sgCopyVec3 ( vert, vt->getVertex ( tri[j] ) ) ;
        fprintf ( fileout, "%f %f %f ", vert[0], vert[1], vert[2] ) ;
      }

      fprintf ( fileout, "0xFFFFFF\n" ) ;
    }
  }
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

int ssgSaveTRI ( const char *filename, ssgEntity *ent )

/******************************************************************************/

/*
  Purpose:
   
    writes an AC3D triangle file.

  Example:

    Each line contains 9 floating point values and a 1 hex value for color.
    the 9 floating point values represent 3 vertices of a triangle
    the color format is 0xRRGGBB (eg 0xffffff is white)

    0.0 0.0 0.0 1.0 0.0 0.0 1.0 1.0 0.0 0xffffff

*/
{
  fileout = fopen ( filename, "wa" ) ;

  if ( fileout == NULL )
  {
    ulSetError ( UL_WARNING, "ssgSaveTRI: Failed to open '%s' for writing", filename ) ;
    return FALSE ;
  }

  save_entities ( ent ) ;

  fclose ( fileout ) ;
  return TRUE;
}
