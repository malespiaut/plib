/*
     PLIB - A Suite of Portable Game Libraries
     Copyright (C) 2001  Steve Baker
 
     This library is free software; you can redistribute it and/or
     modify it under the terms of the GNU Library General Public
     License as published by the Free Software Foundation; either
     version 2 of the License, or (at your option) any later version.
 
     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Library General Public License for more details.
 
     You should have received a copy of the GNU Library General Public
     License along with this library; if not, write to the Free
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 
     For further information visit http://plib.sourceforge.net

     $Id$
*/

#include "ssgLocal.h"

#ifdef GLU_VERSION_1_1

/* Some weirdness with Windows and callbacks makes the following
   necessary for non-win platforms: */
#ifndef CALLBACK
#define CALLBACK
#endif

static GLUtriangulatorObj *tesselator     = NULL;
static ssgVtxTable        *source         = NULL;
static ssgBranch          *poly_branch    = NULL;

static GLenum             curr_gltype;
static ssgVertexArray     *curr_vertices  = NULL;
static ssgNormalArray     *curr_normals   = NULL;
static ssgColourArray     *curr_colours   = NULL;
static ssgTexCoordArray   *curr_texcoords = NULL;

static void CALLBACK vertexCallback( GLvoid *vertex_data ) {
  int index = *(short*)vertex_data;

  curr_vertices -> add( source -> getVertex(index) );

  if ( source->getNumColours()   > 1 ) 
    curr_colours   -> add ( source -> getColour   (index) );
  if ( source->getNumNormals()   > 1 ) 
    curr_normals   -> add ( source -> getNormal   (index) );
  if ( source->getNumTexCoords() > 1 ) 
    curr_texcoords -> add ( source -> getTexCoord (index) );
}

static void CALLBACK beginCallback( GLenum type ) {
  curr_gltype    = type;
  curr_vertices  = new ssgVertexArray;
  curr_normals   = new ssgNormalArray;
  curr_colours   = new ssgColourArray;
  curr_texcoords = new ssgTexCoordArray;

  if ( source->getNumColours() == 1 ) 
    curr_colours -> add ( source -> getColour (0) );
  if ( source->getNumNormals() == 1 ) 
    curr_normals -> add ( source -> getNormal (0) );
}

static void CALLBACK endCallback( void ) {
  ssgVtxTable *leaf = new ssgVtxTable( curr_gltype,
				       curr_vertices,
				       curr_normals,
				       curr_texcoords,
				       curr_colours );
  leaf -> setName     ( source -> getName    () );
  leaf -> setState    ( source -> getState   () );
  leaf -> setCullFace ( source -> getCullFace() );

  poly_branch -> addKid( leaf );
}

ssgEntity *ssgVtxTable::makeConvex() {
  int i;

  if ( gltype != GL_TRIANGLE_FAN && gltype != GL_POLYGON ) {
    // lines, points, triangles and quads are already convex
    return NULL;
  }

  tesselator = gluNewTess();
  if (tesselator == NULL) {
    ulSetError( UL_FATAL, "ssgVtxTable::makeConvex: Unable to create new " \
		"tesselator." );
    return NULL;
  }

  gluTessCallback( tesselator, (GLenum) GLU_BEGIN , 
		   (void (CALLBACK *)())beginCallback  );
  gluTessCallback( tesselator, (GLenum) GLU_END   , 
		   (void (CALLBACK *)())endCallback    );
  gluTessCallback( tesselator, (GLenum) GLU_VERTEX, 
		   (void (CALLBACK *)())vertexCallback );

  source      = this;
  poly_branch = new ssgBranch;

  /*
    We now have to copy all vertices in the right order into an
    array of sgdVec3, since that's how GLU takes its vertex data.
    
    The way to get the vertices is a bit intricate, since this might
    be an ssgVtxArray, where the vertices aren't ordered in the
    ssgVertexArray. We have to get the vertices in counter clockwise order
    by extracting the triangles.
  */
  sgdVec3 *dvertices    = new sgdVec3[ getNumTriangles() + 2 ];
  short   *vert_indices = new short  [ getNumTriangles() + 2 ];
  int c = 0;
  for ( i = 0; i < getNumTriangles(); i++) {
    short v1, v2, v3;
    getTriangle( i, &v1, &v2, &v3 );
    if ( i == 0 ) {
      sgdSetVec3( dvertices[c    ], 
		  getVertex(v1)[0], getVertex(v1)[1], getVertex(v1)[2] );
      sgdSetVec3( dvertices[c + 1], 
		  getVertex(v2)[0], getVertex(v2)[1], getVertex(v2)[2] );
      vert_indices[c    ] = v1;
      vert_indices[c + 1] = v2;
      c += 2;
    }

    sgdSetVec3( dvertices[c], 
		getVertex(v3)[0], getVertex(v3)[1], getVertex(v3)[2] );    
    vert_indices[c] = v3;
    c++;
  }

  /* Finally, lets feed GLU with the vertices */
  gluBeginPolygon( tesselator );
  for ( i = 0; i < getNumTriangles() + 2; i++) {
    gluTessVertex( tesselator, dvertices[i], & vert_indices[i] );
  }
  gluEndPolygon( tesselator );
  gluDeleteTess( tesselator );

  delete [] dvertices;
  delete vert_indices;

  if ( poly_branch -> getNumKids() > 1 ) {
    return poly_branch;
  } else {
    // since one or less kids were created, this leaf is already
    // convex.
    delete poly_branch;
    return NULL;
  }
}

#else

ssgEntity *ssgVtxTable::makeConvex() {
    ulSetError( UL_FATAL, "ssgVtxTable::makeConvex: GLU version 1.1 was not" \
		" present when compiling PLIB." );
    return NULL;
}

#endif

//  int ssgVtxTable::isConvex() {
//    return TRUE;
//  }
