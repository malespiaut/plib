#include "ssgLocal.h"

static const ssgLoaderOptions* current_options;

ssgEntity* ssgLoadStrip( const char* fname, const ssgLoaderOptions* options ) {
  ssgSetCurrentOptions ( (ssgLoaderOptions*)options ) ;
  current_options = ssgGetCurrentOptions () ;

  char filename [ 1024 ] ;
  current_options -> makeModelPath ( filename, fname ) ;
  
  FILE* model_file = fopen(filename, "r");

  if (model_file == NULL) {
    ulSetError(UL_WARNING, "ssgLoadStrip: Couldn't open file '%s'.", filename);
    return NULL;
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

  int i;
  int num_vertices = ulEndianReadLittle32(model_file);
  ssgBranch* model = new ssgBranch();
  ssgVertexArray* vertices = new ssgVertexArray();
  ssgNormalArray* normals  = new ssgNormalArray();

  for (i = 0; i < num_vertices; i++) {
    sgVec3 new_vec;
    int j;

    for (j = 0; j < 3; j++) {
      new_vec[j] = ulEndianReadLittleFloat(model_file);
    }
    vertices->add(new_vec);

    for (j = 0; j < 3; j++) {
      new_vec[j] = ulEndianReadLittleFloat(model_file);
    }
    normals->add(new_vec);
  }

  int num_strips = ulEndianReadLittle32(model_file);
  for (i = 0; i < num_strips; i++) {
    int num_indices = ulEndianReadLittle32(model_file);

    ssgIndexArray* strip_indices = new ssgIndexArray(num_indices);

    for (int j = 0; j < num_indices; j++) {
      strip_indices->add( ulEndianReadLittle16(model_file) );
    }

    ssgVtxArray* varr = new ssgVtxArray( GL_TRIANGLE_STRIP,
					 vertices,
					 normals,
					 NULL,
					 NULL,
					 strip_indices );
    varr->setState( state );
    varr->setCullFace( TRUE );

    ssgLeaf* leaf = current_options->createLeaf(varr, NULL);
    model->addKid(leaf);
  }

  int num_indices = ulEndianReadLittle32(model_file);
  ssgIndexArray* indices = new ssgIndexArray(num_indices);
  for (i = 0; i < num_indices; i++) {
    indices->add( ulEndianReadLittle16(model_file) );
  }

  ssgVtxArray* varr = new ssgVtxArray( GL_TRIANGLES,
				       vertices,
				       normals,
				       NULL,
				       NULL,
				       indices );
  varr->setState( state );
  varr->setCullFace( TRUE );
  
  ssgLeaf* leaf = current_options->createLeaf(varr, NULL);
  model->addKid(leaf);

  return model;
}
