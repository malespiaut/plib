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

//
// IV ( Open Inventor ASCII ) export for SSG/PLIB
// Written by Bram Stolk (bram at sara.nl)
//
// Keep in mind that:
// ssg's Transform's are non-leaf nodes, which correspond to Inventor's
// Transform nodes which are leaf nodes.
// Inventor's Material nodes are scene graph leaf nodes.
// The corresponding State objects from ssg however, are not scene graph nodes.
//
// DONE:
// scene graph hierarchy
// transformation
// states
// colour per vertex
// normal per vertex
// textures
// texture coordinates
// alpha transparency
//
// TODO:
// lights
// rendermodes
//


#include <assert.h>

#include "ssgLocal.h"


// auxiliary stuff for indenting the inventor file.
static int indentLevel ;
static void indent ( FILE *f )
{
  for ( int i=0; i<indentLevel; i++ )
    fprintf( f, "  " ) ;
}


static void writeTransform ( ssgTransform *transform, FILE *f )
{
  indent( f ) ;
  sgMat4 m;
  transform->getTransform( m ) ;
  fprintf
  (
    f,
    "MatrixTransform { matrix %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f }\n",
    m[0][0], m[0][1], m[0][2], m[0][3],
    m[1][0], m[1][1], m[1][2], m[1][3],
    m[2][0], m[2][1], m[2][2], m[2][3],
    m[3][0], m[3][1], m[3][2], m[3][3]
  ) ;
}


// ssg leaf nodes contain the geometry, which we will convert to
// Inventor's IndexedFaceSet.
static void writeLeaf ( ssgLeaf *leaf, FILE *f )
{
  int cnt = leaf->getNumTriangles() ;
  int nv  = leaf->getNumVertices() ;
  int nn  = leaf->getNumNormals() ;
  int nc  = leaf->getNumColours() ;
  int nt  = leaf->getNumTexCoords() ;

  ssgState *st = leaf->getState() ;
  ssgSimpleState *state = 0 ;
  char *textureName = 0 ;
  int i;

  indent( f );
  fprintf( f, "# %d triangles, %d verts, %d normals, %d colours, %d texturecoords\n", cnt, nv,nn,nc,nt ) ;

  if ( st )
    state = dynamic_cast<ssgSimpleState*>( st ) ;

  if ( state )
    textureName = state->getTextureFilename();

  if ( state || nc )
  {
    indent( f ) ;
    fprintf( f, "Material {\n" ) ;
    indentLevel++ ;

    indent( f ) ;
    fprintf( f, "diffuseColor " ) ;
    if ( nc )
    {
      // multiple colour values: colour-per-vertex
      fprintf( f, "[ " ) ;
      for ( i=0; i<nc; i++ )
      {
        float *col = leaf->getColour( i ) ;
        fprintf( f,"%f %f %f, ", col[0],col[1],col[2] ) ;
      }
      fprintf( f, "]\n" ) ;
    }
    else
    {
      // single colour value
      float r=0, g=0, b=0;
      if ( state )
      {
        r = state->diffuse_colour[ 0 ] ;
        g = state->diffuse_colour[ 1 ] ;
        b = state->diffuse_colour[ 2 ] ;
      }
      fprintf( f, "%f %f %f\n", r,g,b ) ;
    }
    if ( state )
    {
      indent( f );
      fprintf( f,"ambientColor %f %f %f\n", state->ambient_colour[0], state->ambient_colour[1], state->ambient_colour[2] ) ;
      indent( f );
      fprintf( f,"specularColor %f %f %f\n", state->specular_colour[0], state->specular_colour[1], state->specular_colour[2] ) ;
      indent( f );
      fprintf( f,"emissiveColor %f %f %f\n", state->emission_colour[0], state->emission_colour[1], state->emission_colour[2] ) ;
      indent( f );
      fprintf( f, "shininess %f\n", state->shininess ) ;
      if ( state->isEnabled( GL_BLEND ) )
      {
        float a = state->diffuse_colour[ 3 ] ;
        indent( f ) ;
        fprintf( f, "transparency %f\n", 1.0-a );
      }
    }

    indentLevel-- ;
    indent( f ) ;
    fprintf( f, "}\n" ) ;
  }

  if ( textureName )
  {
    indent( f ) ;
    fprintf( f, "Texture2 { filename %c%s%c }\n", '"', textureName, '"' ) ;
  }

  if ( nn )
  {
    indent( f ) ;
    fprintf( f, "Normal {\n") ;
    indentLevel++ ;

    indent( f ) ;
    fprintf( f, "vector [ " ) ;
    for ( i=0; i<nn; i++ )
    {
      float *nrm = leaf->getNormal( i ) ;
      fprintf( f, "%f %f %f, ", nrm[ 0 ], nrm[ 1 ], nrm[ 2 ] );
    }
    fprintf( f, " ]\n" );
    indentLevel-- ;

    indent( f ) ;
    fprintf( f, "}\n" ) ;
  }

  if ( nt && textureName )
  {
    indent( f ) ;
    fprintf( f, "SoTextureCoordinate2 { point [ " ) ;
    for ( i=0; i<nt; i++ )
    {
      float *t = leaf->getTexCoord( i ) ;
      fprintf( f, "%f %f, ", t[ 0 ], t[ 1 ] ) ;
    }
    fprintf( f, " ] }\n" ) ;
  }

  indent( f ) ;
  fprintf( f, "Coordinate3 { point [ " ) ;
  for ( i=0; i<nv; i++ )
  {
    float *v = leaf->getVertex( i ) ;
    assert( v ) ;
    fprintf( f, "%f %f %f, ", v[ 0 ], v[ 1 ], v[ 2 ] ) ;
  }
  fprintf( f, " ] }\n" ) ;

  indent( f ) ;
  fprintf( f, "IndexedFaceSet {\n" ) ;

  indent( f ) ;
  fprintf( f, "  coordIndex [ " ) ;
  for ( i=0; i<cnt; i++ )
  {
    short idx0, idx1, idx2 ;
    leaf->getTriangle( i, &idx0, &idx1, &idx2 ) ;
    fprintf( f, "%d,%d,%d,-1,", idx0, idx1, idx2 ) ;
  }

  fprintf( f, " ]\n" ) ;
  indent( f ) ;
  fprintf( f, "}\n" ) ;
}


// Handles pre-order traversal of the ssg hierarchy.
static void preHandle ( ssgEntity *ent, FILE *f )
{
  // Put some comments in the inventor output file.
  indent( f ) ;
  const char *name = ent->getName() ;
  fprintf( f, "# %s (%s)\n", ent->getTypeName(), (name)?name:"unnamed") ;

  ssgBranch *branch = dynamic_cast<ssgBranch*>( ent ) ;
  if ( branch )
  {
    // ssg's branch nodes are translated to inventor's separator node.
    indent( f ) ;
    fprintf( f,"Separator {\n" ) ;
    indentLevel++ ;
    ssgTransform *transform = dynamic_cast<ssgTransform*>( branch ) ;
    // If this branch is a transform, we need to convert it.
    if ( transform )
      writeTransform( transform, f ) ;
  }

  ssgLeaf *leaf = dynamic_cast<ssgLeaf*>( ent ) ;
  if ( leaf )
    writeLeaf( leaf, f ) ;
}


// Handles post-order traversal of the ssg hierarchy.
static void postHandle ( ssgEntity *ent, FILE *f )
{
  ssgBranch *branch = dynamic_cast<ssgBranch*>( ent ) ;
  if ( branch )
  {
    // For branches, we use IV's Seperator, which we need to close now.
    indentLevel-- ;
    indent( f ) ;
    fprintf( f, "}\n" ) ;
  }
}


// Traverses ssg hierarchy.
static void walkTree ( ssgEntity *ent, FILE *f )
{
  preHandle( ent, f ) ;
  ssgBranch *branch = dynamic_cast<ssgBranch*>( ent ) ;
  if ( branch )
  {
    for ( int i=0; i<branch->getNumKids(); i++ )
    {
      ssgEntity *kid = branch->getKid( i );
      assert( kid ) ;
      walkTree( kid, f ) ;
    }
  }
  postHandle( ent, f ) ;
}


// Entry point: save ssg hierarchy as an OpenInventor file.
int ssgSaveIV ( const char *filename, ssgEntity *ent )
{
  FILE *f = fopen( filename, "w" ) ;
  if ( !f )
    return FALSE ;
  fprintf( f, "#Inventor V2.1 ascii\n" ) ;
  fprintf
  (
    f,
    "#Export from plib version %d.%d.%d\n\n",
    PLIB_MAJOR_VERSION, PLIB_MINOR_VERSION, PLIB_TINY_VERSION
  ) ;
  indentLevel = 0 ;
  walkTree( ent, f ) ;
  return TRUE ;
}

