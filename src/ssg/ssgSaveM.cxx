#include <stdio.h>
#include "ssgLocal.h"

ssgVertexArray* vertices;
ssgIndexArray*  indices;

static void traverse( ssgEntity* node, sgMat4 transform ) {
  if ( node->isAKindOf( ssgTypeTransform() ) ) {
    sgMat4 local_transform;
    ssgTransform *t_node = (ssgTransform*)node;
    
    t_node->getTransform(local_transform);
    sgPostMultMat4( local_transform, transform );

    for (ssgEntity* kid = t_node->getKid(0); kid != NULL; 
	 kid = t_node->getNextKid()) {
      traverse( kid, local_transform );
    }
  } else if ( node->isAKindOf( ssgTypeBranch() ) ) {
    ssgBranch *b_node = (ssgBranch*)node;
    for (ssgEntity* kid = b_node->getKid(0); kid != NULL; 
	 kid = b_node->getNextKid()) {
      traverse( kid, transform );
    }    
  } else if ( node->isAKindOf( ssgTypeLeaf() ) ) {
    ssgLeaf* l_node = (ssgLeaf*)node;
    int i, vert_low = vertices->getNum();
    for (i = 0; i < l_node->getNumVertices(); i++) {
      sgVec3 new_vertex;
      sgXformVec3(new_vertex, l_node->getVertex(i), transform);
      vertices->add(new_vertex);
    }

    for (i = 0; i < l_node->getNumTriangles(); i++) {
      short v1, v2, v3;
      l_node->getTriangle(i, &v1, &v2, &v3);
      indices->add( vert_low + v1 );
      indices->add( vert_low + v2 );
      indices->add( vert_low + v3 );
    }
  }
}

int ssgSaveM( const char* fname, ssgEntity *ent ) {
  FILE *fd = fopen ( fname, "w" ) ;
	int i;
 
  if ( fd == NULL ) {
    ulSetError ( UL_WARNING, "ssgSaveM: Failed to open '%s' for writing", 
		 fname ); 
    return FALSE ;
  }

  vertices = new ssgVertexArray();
  indices  = new ssgIndexArray ();

  fprintf(fd, "# Model output by ssgSaveM. Original graph structure was:\n");
  ent->print(fd, "#", 0);

  sgMat4 ident;
  sgMakeIdentMat4( ident );
  traverse( ent, ident );

  for (i = 0; i < vertices->getNum(); i++) {
    fprintf(fd, "Vertex %d  %f %f %f\n", i+1,
	    vertices->get(i)[0],
	    vertices->get(i)[1],
	    vertices->get(i)[2]);
  }

  for (i = 0; i < indices->getNum(); i += 3) {
    fprintf(fd, "Face %d  %d %d %d\n", (i/3)+1,
	    *indices->get(i    ) + 1,
	    *indices->get(i + 1) + 1,
	    *indices->get(i + 2) + 1);
  }

  fclose( fd ) ;

  delete vertices;
  delete indices ;
 
  return TRUE;
}
