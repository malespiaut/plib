#include <stdio.h>
#include "ssgLocal.h"

static const ssgLoaderOptions* current_options;

ssgEntity* ssgLoadM( const char* fname, 
		     const ssgLoaderOptions* options ) {
  current_options = options? options: &_ssgDefaultOptions ;
  current_options -> begin () ;
  
  char filename [ 1024 ] ;
  if ( fname [ 0 ] != '/' &&
       _ssgModelPath != NULL &&
       _ssgModelPath [ 0 ] != '\0' ) {
    strcpy ( filename, _ssgModelPath ) ;
    strcat ( filename, "/" ) ;
    strcat ( filename, fname ) ;
  }
  else
    strcpy ( filename, fname ) ;
  
  FILE* model_file = fopen(filename, "r");
  if(!model_file) {
    ulSetError(UL_WARNING, "ssgLoadM: Couldn't open file '%s'.",
	       filename);
    return NULL;
  }

  ssgVertexArray* vertices = new ssgVertexArray;
  ssgNormalArray* normals  = new ssgNormalArray;
  ssgIndexArray*  indices  = new ssgIndexArray;

  int i, index;
  char line[256];
  sgVec3 zero = {0.0f, 0.0f, 0.0f};

  fgets(line, 256, model_file);
  while ( !feof(model_file) ) {
    char* token;

    switch (line[0]) {
    case '#':                    // comment, skip this line
      break;
    case 'V':
      sgVec3 vtx;
      token = strtok(line, " ");  // token should now be "Vertex"
      token = strtok(NULL, " ");  // token is vertex index now

      index = atoi(token) - 1;

      // fill out non-declared vertices with zero vectors
      while (index > vertices->getNum()) {
	vertices->add(zero);
	normals ->add(zero);
      }

      // get vertex coordinate
      for (i = 0; i < 3; i++) {
	token = strtok(NULL, " ");
	vtx[i] = atof(token);
      }

      vertices->add(vtx) ;
      normals ->add(zero);

      break;

    case 'F':                   // face
      token = strtok(line, " ");  // token should now be "Face"
      token = strtok(NULL, " ");  // face index, ignored

      for (i = 0; i < 3; i++) {
	token = strtok(NULL, " ");
	indices->add( atoi(token)-1 );
      }

      break;

    case 'E':                  // Edge, ignored
      break;

    default:
      ulSetError(UL_WARNING, "ssgLoadM: Syntax error on line \"%s\".", line);
    }

    fgets(line, 256, model_file);
  }

  ssgSimpleState* state = new ssgSimpleState();
  state->setOpaque();
  state->disable(GL_BLEND);
  state->disable(GL_ALPHA_TEST);
  state->disable(GL_TEXTURE_2D);
  state->enable(GL_COLOR_MATERIAL);
  state->enable(GL_LIGHTING);
  state->setShadeModel(GL_SMOOTH);
  state->setMaterial(GL_AMBIENT , 0.7f, 0.7f, 0.0f, 1.0f);
  state->setMaterial(GL_DIFFUSE , 0.7f, 0.7f, 0.0f, 1.0f);
  state->setMaterial(GL_SPECULAR, 1.0f, 1.0f, 1.0f, 1.0f);
  state->setMaterial(GL_EMISSION, 0.0f, 0.0f, 0.0f, 1.0f);
  state->setShininess(50);

  // now calculate smooth normals (this code belongs elsewhere)
  for (i = 0; i < indices->getNum(); i += 3) {
    short idx0 = *indices->get(i    );
    short idx1 = *indices->get(i + 1);
    short idx2 = *indices->get(i + 2);

    sgVec3 normal;
    sgMakeNormal( normal,
		  vertices->get(idx0),
		  vertices->get(idx1),
		  vertices->get(idx2) );

    sgAddVec3( normals->get(idx0), normal );
    sgAddVec3( normals->get(idx1), normal );
    sgAddVec3( normals->get(idx2), normal );
  }

  for (i = 0; i < vertices->getNum(); i++) {
    sgNormaliseVec3( normals->get(i) );
  }

  ssgVtxArray* leaf = new ssgVtxArray( GL_TRIANGLES,
				       vertices,
				       normals,
				       NULL,
				       NULL,
				       indices );

  leaf->setCullFace( TRUE );
  leaf->setState( state );

  ssgLeaf* model = current_options -> createLeaf( leaf, NULL );

  current_options -> end () ; 
  return model;
}
