#include <stdio.h>
#include "ssgLocal.h"
#include "ssgLoaderWriterStuff.h"



int ssgSaveM( const char* fname, ssgEntity *ent ) {
  FILE *fd = fopen ( fname, "w" ) ;
	int i;
 
  if ( fd == NULL ) {
    ulSetError ( UL_WARNING, "ssgSaveM: Failed to open '%s' for writing", 
		 fname ); 
    return FALSE ;
  }

  ssgVertexArray* vertices = new ssgVertexArray();
  ssgIndexArray*  indices  = new ssgIndexArray ();

  fprintf(fd, "# Model output by ssgSaveM. Original graph structure was:\n");
  ent->print(fd, "#", 0);

  sgMat4 ident;
  sgMakeIdentMat4( ident );
  ssgAccumVerticesAndFaces( ent, ident, vertices, indices, -1 );

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
