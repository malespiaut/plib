#include <stdio.h>
#include "ssgLocal.h"
#include "ssgLoaderWriterStuff.h"



int ssgSaveQHI( const char* fname, ssgEntity *ent ) {
  FILE *fd = fopen ( fname, "w" ) ;
	int i;
 
  if ( fd == NULL ) {
    ulSetError ( UL_WARNING, "ssgSaveQHI: Failed to open '%s' for writing", 
		 fname ); 
    return FALSE ;
  }

  ssgVertexArray* vertices = new ssgVertexArray();
  
  
  sgMat4 ident;
  sgMakeIdentMat4( ident );
  ssgAccumVerticesAndFaces( ent, ident, vertices, NULL, 0.0001f );

	fprintf(fd, "3\n");  // Dimension
	fprintf(fd, "%d\n", vertices->getNum()); // No of points

	// Points:
  for (i = 0; i < vertices->getNum(); i++) {
    fprintf(fd, "%f %f %f\n", vertices->get(i)[0],
															vertices->get(i)[1],
															vertices->get(i)[2]);
  }


  fclose( fd ) ;

  delete vertices;
 
  return TRUE;
}
