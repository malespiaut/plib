//===========================================================================
// ssgLoadMDL.cxx
// This is a loader for Microsoft Flight Simulator / Flight Shop models
// (MDL-files) to SSG. 
//
// Original code by Thomas Engh Sevaldrud, extended and adapted to SSG by
// Per Liedman.
//===========================================================================

/* 
   This loader is intended for loading MDL models constructed with 
   Flight Shop (a model editor for MSFS). Models not created with 
   Flight Shop or hand tweaked afterwards might not work without problems.
*/

#include <iostream.h>
#include "ssgLocal.h"
#include "ssgLoadMDL.h"

#define DEF_SHININESS 50

// Define DEBUG if you want some debug info
/*#define DEBUG 1*/

#ifdef DEBUG
#include <iostream>
#define DEBUGPRINT(x) cerr << x
#else
#define DEBUGPRINT(x)
#endif

struct _MDLPart {
  GLenum type;
  ssgVertexArray   *vtx;
  ssgNormalArray   *nrm;
  ssgIndexArray    *idx;
  ssgTexCoordArray *crd;

  void removeUnnecessaryVertices(void);
};

static const int PART_GEAR     = 0;
static const int PART_FLAPS    = 1;
static const int PART_LIGHTS   = 2;
static const int PART_STROBE   = 3;
static const int PART_SPOILERS = 4;
static const int PART_PROP     = 5;
static const int NUM_MOVING_PARTS = 6;

static char *PART_NAME[] = {"GEAR",     "FLAPS", "LIGHTS", "STROBE",
			    "SPOILERS", "PROP"};

// Temporary vertex arrays
static ssgVertexArray 		*curr_vtx_;
static ssgNormalArray 		*curr_norm_;
   
// Arrays for geometry information
static ssgVertexArray 		*vertex_array_;
static ssgNormalArray 		*normal_array_;
static ssgTexCoordArray 	*tex_coords_;

// Current part (index array)
static _MDLPart 		*curr_part_;
   
static ssgBranch		*model_;
static ssgBranch                *curr_branch_;
static ssgSelector              *moving_parts_[NUM_MOVING_PARTS];
static char                     curr_tex_name_[15];
static int			num_tex_states_, start_idx_, last_idx_;
static int                      curr_color_, curr_pal_id_;
static float                    curr_alpha_;
static bool                     curr_cull_face_;

static bool			has_normals_, vtx_dirty_, join_children_;
static FILE*                    model_file_;

static const ssgLoaderOptions* current_options = NULL ;

static unsigned char get_byte() {
  unsigned char b;
  fread( &b, 1, 1, model_file_ );
  return b;
}

//===========================================================================

void _MDLPart::removeUnnecessaryVertices(void) {
  int i;
  short j;
  ssgVertexArray *vtxnew;
  ssgNormalArray *nrmnew;
  ssgTexCoordArray *crdnew = NULL;
  ssgIndexArray *idxnew;
  
  if ( type == GL_LINES ) 
    return;
  
  assert(vtx!=NULL);
  assert(nrm!=NULL);
  assert(idx!=NULL);  
  assert(vtx->getNum()==nrm->getNum());
  
  if(crd!=NULL) {
       assert(vtx->getNum()==crd->getNum());
  }

  vtxnew = new ssgVertexArray();
  nrmnew = new ssgNormalArray();

  if(crd != NULL) {
    crdnew = new ssgTexCoordArray();
  }

  idxnew = new ssgIndexArray();
  
  // This may generate "double vertices" and can therefore be 
  // optimized even more.
  for ( i=0; i<idx->getNum(); i++) {
    j=*idx->get(i);
    vtxnew->add(vtx->get(j));
    nrmnew->add(nrm->get(j));
    if ( crd != NULL )
      crdnew->add(crd->get(j));
    
    idxnew->add(i);
  }
  assert(idx->getNum()==idxnew->getNum());
  assert(idx->getNum()==vtxnew->getNum());
  vtx = vtxnew;
  nrm = nrmnew;
  idx = idxnew;
  if ( crd != NULL ) {
       crd = crdnew;
  }
}

//===========================================================================
static void readIfIn1() {
  int i;
  unsigned short var;
  short offset, high, low, next_op;
  offset  = ulEndianReadLittle16(model_file_);
  var     = ulEndianReadLittle16(model_file_);
  low     = ulEndianReadLittle16(model_file_);
  high    = ulEndianReadLittle16(model_file_);
  next_op = ulEndianReadLittle16(model_file_);

  int part_idx, kid_idx;

  // for moving parts except propeller, this seems
  // to work
  kid_idx = ( next_op == 0x000d ) ? 0 : 1;

  switch (var) {
  case 0x0052:
  case 0x006c:
    part_idx = PART_FLAPS; break;
  case 0x0054:
  case 0x006e:
    part_idx = PART_GEAR; break;
  case 0x005a:
  case 0x0074:
    part_idx = PART_PROP; break;
  case 0x005c:
  case 0x0076:
    part_idx = PART_LIGHTS; break;
  case 0x005e:
  case 0x0078:
    part_idx = PART_STROBE; break;
  case 0x0062:
  case 0x007c:
    part_idx = PART_SPOILERS; break;
  default:
    part_idx = -1; break;
  }

  /*
    DEBUGPRINT( "IfVarRange(" << std::hex << offset << " " << var
    << " " << low << " " << high << ")"
    << "   (next op " << next_op << ", kid_idx "
    << kid_idx << ")" << 
    std::dec << std::endl );
  */

  char nodename[64];

  if (part_idx == -1) {
    curr_branch_ = model_;
  } else {
    if (moving_parts_[part_idx] == NULL) {
      moving_parts_[part_idx] = new ssgSelector;
      model_->addKid(moving_parts_[part_idx]);	      	   
      moving_parts_[part_idx]->setName( PART_NAME[part_idx] );
      //moving_parts_[part_idx]->select(1);
    }

    // nasty special case for prop
    if (part_idx == PART_PROP) {
      sprintf(nodename, "PROP_%d_%d", low, high);
      i = 0;
      for (ssgEntity* propkid=moving_parts_[PART_PROP]->getKid(0);
	   propkid != NULL; 
	   propkid = moving_parts_[PART_PROP]->getNextKid(), i++) {
	if ( propkid->getName() != NULL ) {
	  if ( strcmp(nodename, propkid->getName()) == 0 ) {
	    break;
	  }
	}
      }

      kid_idx = i;
    } else {
      sprintf(nodename, "%s_%s", 
	      moving_parts_[part_idx]->getName(),
	      (kid_idx == 0)?"TRUE":"FALSE");		      
    }

    //DEBUGPRINT( nodename << " " << kid_idx << std::endl );

    while (moving_parts_[part_idx]->getKid(kid_idx) == NULL)
      moving_parts_[part_idx]->addKid( new ssgBranch() );

    curr_branch_ = (ssgBranch*)moving_parts_[part_idx]->
      getKid(kid_idx);
    curr_branch_->setName(nodename);
  }

  // now move back the file pointer 16 bits (we have peeked at the next op)
  fseek( model_file_, -2, SEEK_CUR );
}

static bool findPart(FILE* fp)
{
  int i;

  curr_branch_ = model_;

  unsigned char pattern1[4] = { 0x1a, 0x00, 0x01, 0x00 };
  unsigned char pattern2[4] = { 0x29, 0x00, 0x01, 0x00 };
 
  unsigned char pattern[4];
  fread(pattern, 1, 4, fp);

  while(!feof(fp))
    {
      int match1 = 0;
      int match2 = 0;
      int matchpos = -1;

      for(i = 0; i < 4; i++)
	{
	  if(pattern[i] == pattern1[i]) match1++;
	  if(pattern[i] == pattern2[i]) match2++;
	  if((pattern[i] == 0x24 || pattern[i] == 0x1c) && matchpos < 0) 
	    matchpos = i;
	}

      if(match1 == 4)
	{
	  fseek(fp, -4, SEEK_CUR);
	  //DEBUGPRINT( "found vertices at " << std::hex << ftell(fp) 
	  //	      << std::dec << std::endl);
	  return true;
	}

      else if(match2 == 4)
	{
	  fseek(fp, -4, SEEK_CUR);
	  //DEBUGPRINT( "found vertices at " << std::hex << ftell(fp) 
	  //      << std::dec << std::endl);
	  return true;
	}
      else if(matchpos >= 0) 
	{
	  /* this section of the code takes care of moving parts. 
	     (all moving parts are currently handled by the BGL_IFIN1
	     op-code) */

	  long pos = ftell(fp);

	  fseek(fp, -2+matchpos, SEEK_CUR);
	  readIfIn1();
	  fseek(fp, pos, SEEK_SET);
	}
      
      pattern[0] = pattern[1];
      pattern[1] = pattern[2];
      pattern[2] = pattern[3];
      fread(&pattern[3], 1, 1, fp);
    }

  return false;
}




//===========================================================================

static void readPoint(FILE* fp, sgVec3 p)
{
  short x_int, y_int, z_int;
  y_int = ulEndianReadLittle16(model_file_);
  z_int = ulEndianReadLittle16(model_file_);
  x_int = ulEndianReadLittle16(model_file_);
 
  // Convert from .MDL units (ca 2mm) to meters
  p[0] =  -(float)x_int/512.0;
  p[1] =  (float)y_int/512.0;
  p[2] =  (float)z_int/512.0;
}


//===========================================================================

static void readVector(FILE* fp, sgVec3 v)
{
  short x_int, y_int, z_int;
  y_int = ulEndianReadLittle16(model_file_);
  z_int = ulEndianReadLittle16(model_file_);
  x_int = ulEndianReadLittle16(model_file_);

  v[0] = -(float)x_int;
  v[1] = (float)y_int;
  v[2] = (float)z_int;

  sgNormaliseVec3( v );
}

//===========================================================================

static void recalcNormals( _MDLPart *part ) {
  //DEBUGPRINT( "Calculating normals." << std::endl);
  sgVec3 n;

  for (int i = 0; i < part->idx->getNum() / 3; i++) {
    short ix0 = *part->idx->get(i*3    );
    short ix1 = *part->idx->get(i*3 + 1);
    short ix2 = *part->idx->get(i*3 + 2);

    sgMakeNormal( n, 
		  curr_part_->vtx->get(ix0),
		  curr_part_->vtx->get(ix1),
		  curr_part_->vtx->get(ix2) );
    
    sgCopyVec3( part->nrm->get(ix0), n );
    sgCopyVec3( part->nrm->get(ix1), n );
    sgCopyVec3( part->nrm->get(ix2), n );
  }
}

//===========================================================================

static void createTriangIndices(ssgIndexArray *ixarr,
				int numverts, const sgVec3 s_norm)
{
  sgVec3 v1, v2, cross;

  // triangulate polygons
  if(numverts == 1)
    {
      unsigned short ix0 = *ixarr->get(0);
      if ( ix0 >= curr_part_->vtx->getNum() ) {
	ulSetError(UL_WARNING, "ssgLoadMDL: Index out of bounds.");
	return;
      }

      curr_part_->idx->add(ix0);
      curr_part_->idx->add(ix0);
      curr_part_->idx->add(ix0);
    }

  else if(numverts == 2)
    {
      unsigned short ix0 = *ixarr->get(0);
      unsigned short ix1 = *ixarr->get(1);
      if ( ix0 >= curr_part_->vtx->getNum() ||
	   ix1 >= curr_part_->vtx->getNum() ) {
	ulSetError(UL_WARNING, "ssgLoadMDL: Index out of bounds.");
	return;
      }

      curr_part_->idx->add(ix0);
      curr_part_->idx->add(ix1);
      curr_part_->idx->add(ix0);
    }

  else if(numverts == 3)
    {
      unsigned short ix0 = *ixarr->get(0);
      unsigned short ix1 = *ixarr->get(1);
      unsigned short ix2 = *ixarr->get(2);
      if ( ix0 >= curr_part_->vtx->getNum() ||
	   ix1 >= curr_part_->vtx->getNum() ||
	   ix2 >= curr_part_->vtx->getNum() ) {
	ulSetError(UL_WARNING, "ssgLoadMDL: Index out of bounds.");
	return;
      }

      sgSubVec3(v1, 
		curr_part_->vtx->get(ix1), 
		curr_part_->vtx->get(ix0));
      sgSubVec3(v2, 
		curr_part_->vtx->get(ix2),
		curr_part_->vtx->get(ix0));
    
      sgVectorProductVec3(cross, v1, v2);

      if(sgScalarProductVec3(cross, s_norm) > 0.0f)
	{
	  curr_part_->idx->add(ix0);
	  curr_part_->idx->add(ix1);
	  curr_part_->idx->add(ix2);
	}
      else
	{
	  curr_part_->idx->add(ix0);
	  curr_part_->idx->add(ix2);
	  curr_part_->idx->add(ix1);
	}
    }

  else
    {
      unsigned short ix0 = *ixarr->get(0);
      unsigned short ix1 = *ixarr->get(1);
      unsigned short ix2 = *ixarr->get(2);
      if ( ix0 >= curr_part_->vtx->getNum() ||
	   ix1 >= curr_part_->vtx->getNum() ||
	   ix2 >= curr_part_->vtx->getNum() ) {
	ulSetError(UL_WARNING, "ssgLoadMDL: Index out of bounds.");
	return;
      }

      // Ensure counter-clockwise ordering
      sgMakeNormal(cross, 
		   curr_part_->vtx->get(ix0), 
		   curr_part_->vtx->get(ix1), 
		   curr_part_->vtx->get(ix2));
      bool flip = (sgScalarProductVec3(cross, s_norm) < 0.0);

      curr_part_->idx->add(ix0);
      for(int i = 1; i < numverts; i++)
	{
	  ix1 = *ixarr->get( flip ? numverts-i : i);

	  if ( ix1 >= curr_part_->vtx->getNum() ) {
	    ulSetError(UL_WARNING, "ssgLoadMDL: Index out of bounds.");
	    continue;
	  }

	  curr_part_->idx->add(ix1);
	}
       
    }
}

//===========================================================================

static bool readTexIndices(int numverts, const sgVec3 s_norm, bool flip_y)
{
  if(numverts <= 0)
    return false;

  if(tex_coords_->getNum() <
     vertex_array_->getNum())
    {
      sgVec2 dummy_pt;
      sgSetVec2(dummy_pt, FLT_MAX, FLT_MAX);
      for(int i = tex_coords_->getNum();
	  i < vertex_array_->getNum(); i++)
	tex_coords_->add(dummy_pt);
    }

  // Read index values and texture coordinates
  ssgIndexArray ixarr(numverts);
  for(int v = 0; v < numverts; v++) 
    {
      unsigned short ix;
      short tx_int, ty_int;

      ix     = ulEndianReadLittle16(model_file_);
      tx_int = ulEndianReadLittle16(model_file_);
      ty_int = ulEndianReadLittle16(model_file_);

      if (flip_y) {
	ty_int = 255 - ty_int;
      }

      int tex_idx = ix - start_idx_ + last_idx_;

      sgVec2 tc;
      sgSetVec2(tc, double(tx_int)/255.0, double(ty_int)/255.0);

      sgVec2 curr_tc;
      sgCopyVec2(curr_tc, tex_coords_->get(tex_idx));

      double dist = sgDistanceVec2(curr_tc, tc);
    
      if((curr_tc[0] >= FLT_MAX - 1 && curr_tc[1] >= FLT_MAX - 1))
	{
	  //DEBUGPRINT( "." );
	  sgCopyVec2(tex_coords_->get(tex_idx), tc);
	}

      else if(dist > 0.0001)
	{
	  // We have a different texture coordinate for an existing vertex,
	  // so we have to copy this vertex and create a new index for it
	  // to get the correct texture mapping.

	  //DEBUGPRINT( "duplicating texture coordinate!\n");
       
	  int idx = ix - start_idx_ + last_idx_;
	  tex_idx = curr_part_->vtx->getNum();

	  ssgVertexArray* vtx_arr  = curr_part_->vtx;
	  ssgNormalArray* norm_arr = curr_part_->nrm;

	  sgVec3 vtx, nrm;
	  sgCopyVec3( vtx, vtx_arr ->get(idx) );
	  sgCopyVec3( nrm, norm_arr->get(idx) );
	  vtx_arr ->add(vtx);
	  norm_arr->add(nrm);
       
	  tex_coords_->add(tc);
	}

      /* ssgIndexArray doesn't let us assign arbitrary elements, so
	 this hack is needed */
      short *ixp = ixarr.get(v);
      if (ixp != NULL) {
	*ixp = tex_idx;
      } else {
	ixarr.add(tex_idx);
      }
    }

  createTriangIndices(&ixarr, numverts, s_norm);
 
  return true;
}

//===========================================================================

static bool readIndices(FILE* fp, int numverts, const sgVec3 s_norm)
{
  if(numverts <= 0)
    return false;
	  
  // Read index values
  ssgIndexArray ixarr(numverts);
  for(int v = 0; v < numverts; v++)
    {
      unsigned short ix;
      ix = ulEndianReadLittle16(model_file_);
      ixarr.add(ix - start_idx_ + last_idx_);
      //DEBUGPRINT( "ix[" << v << "] = " << *ixarr.get(v) << std::endl);
    }

  createTriangIndices(&ixarr, numverts, s_norm);
 
  return true;
}

//===========================================================================

static ssgSimpleState* createMaterialState(int color, int pal_id)
{ 
  ssgSimpleState* state = new ssgSimpleState();
    
  float r, g, b;
  if (curr_alpha_ < 1.0f) 
    {
      state->setTranslucent();

      state->enable    (GL_BLEND);
      state->enable    (GL_ALPHA_TEST);

      r = (float)(fsAltPalette[color].r)/255.0;
      g = (float)(fsAltPalette[color].g)/255.0;
      b = (float)(fsAltPalette[color].b)/255.0;
    }
  else 
    {
      state->setOpaque();

      state->disable   (GL_BLEND);
      state->disable   (GL_ALPHA_TEST);

      r = (float)(fsAcPalette[color].r)/255.0;
      g = (float)(fsAcPalette[color].g)/255.0;
      b = (float)(fsAcPalette[color].b)/255.0;
    }
     
  state->setShadeModel (GL_SMOOTH);

  state->enable        (GL_LIGHTING);

  state->disable       (GL_TEXTURE_2D);
  state->disable       (GL_COLOR_MATERIAL);

  state->setShininess  (DEF_SHININESS);
  state->setMaterial   (GL_AMBIENT , r   , g   , b   , curr_alpha_ );
  state->setMaterial   (GL_DIFFUSE , r   , g   , b   , curr_alpha_ );
  state->setMaterial   (GL_SPECULAR, 1.0f, 1.0f, 1.0f, curr_alpha_ );
  state->setMaterial   (GL_EMISSION, 0.0f, 0.0f, 0.0f, curr_alpha_ );

  //DEBUGPRINT( "  Creating non-textured state: color = (" << r << ", " << g <<
  //      ", " << b << ")" << std::endl);

  return state;
}


//===========================================================================

static ssgSimpleState* createTextureState(char *name)
{
  ssgSimpleState* state = new ssgSimpleState();

  strcpy(curr_tex_name_, name);
 
  state->setShadeModel (GL_SMOOTH);

  if (curr_alpha_ < 1.0f) 
    {
      state->setTranslucent();

      state->enable    (GL_BLEND);
      state->enable    (GL_ALPHA_TEST);
    }
  else 
    {
      state->setOpaque();

      state->disable   (GL_BLEND);
      state->disable   (GL_ALPHA_TEST);
    }

  state->enable        (GL_LIGHTING);
  state->enable        (GL_TEXTURE_2D);

  state->disable       (GL_COLOR_MATERIAL);

  state->setShininess  (DEF_SHININESS);
  state->setMaterial   (GL_AMBIENT , 1.0f, 1.0f, 1.0f, curr_alpha_);
  state->setMaterial   (GL_DIFFUSE , 1.0f, 1.0f, 1.0f, curr_alpha_);
  state->setMaterial   (GL_SPECULAR, 1.0f, 1.0f, 1.0f, curr_alpha_);
  state->setMaterial   (GL_EMISSION, 0.0f, 0.0f, 0.0f, curr_alpha_);

  state->setTexture( current_options -> createTexture(name, FALSE, FALSE) ) ;

  //DEBUGPRINT( "  Creating texture state: name = " << name << std::endl);

  return state;
}
//===========================================================================

ssgEntity *ssgLoadMDL( const char* fname, const ssgLoaderOptions* options )
{
  current_options = options? options: &_ssgDefaultOptions ;
  current_options -> begin () ;

  num_tex_states_ = 0;
  start_idx_      = 0;
  join_children_  = true;
  {
    for (int i = 0; i < NUM_MOVING_PARTS; i++) {
      moving_parts_[i] = NULL;
    }
  }

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


  model_file_ = fopen(filename, "rb");
  if(!model_file_) 
    {
      ulSetError(UL_WARNING, "ssgLoadMDL: Couldn't open MDL file '%s'.",
		 filename);
      return NULL;
    }

  // Initialize object graph
  model_ = new ssgBranch();
  curr_branch_ = model_;
  char* model_name = new char[128];
  char *ptr = (char*)&fname[strlen(fname) - 1];
  while(ptr != &fname[0] && *ptr != '/') ptr--;
  if(*ptr == '/') ptr++;
  strcpy(model_name, ptr);
  ptr = &model_name[strlen(model_name)];
  while(*ptr != '.' && ptr != &model_name[0]) ptr--; 
  *ptr = '\0';
  model_->setName(model_name);

  curr_vtx_  = new ssgVertexArray();
  curr_norm_ = new ssgNormalArray();
 
  vertex_array_ = new ssgVertexArray();
  normal_array_ = new ssgNormalArray();
  vertex_array_ -> ref();
  normal_array_ -> ref();

  curr_alpha_    = 1.0f;
 
  tex_coords_ = new ssgTexCoordArray();
 
  /* I have not found any good references on where actual BGL
     data starts in a MDL file, but this seems like a good guess
     from looking at a few MDL files in a hex editor :-O  /PL */
  fseek(model_file_, 0x10a4, SEEK_SET); // 0x11fc in Thomas' original code
  unsigned int code_len;
  code_len = ulEndianReadLittle32(model_file_);
  DEBUGPRINT( "Code length: " << code_len << " bytes\n");
  
  start_idx_ = 0;
  last_idx_  = 0;
 
  findPart(model_file_);
  
  // Parse opcodes
  bool done = false;
  while(!feof(model_file_) && !done) 
    {
      unsigned short opcode;
      unsigned int   skip_offset = 0;

      opcode = ulEndianReadLittle16(model_file_);

      switch(opcode)
	{
	case 0x0:	// EOF
	case 0x22: 	// BGL return
	  {
	    //DEBUGPRINT( "BGL return\n");

	    curr_branch_ = model_;
	    findPart(model_file_);
	  }
	  break;

	case 0x02:      // NOOP
	  break;

	case 0x08: 	// CLOSURE
	  {
	    //DEBUGPRINT( "CLOSURE\n\n");

	    curr_branch_ = model_;
	    findPart(model_file_);
	    continue;
	  }
	  break;

	case 0x0f:	// STRRES: Start line definition
	  {
	    unsigned short idx;
	    idx = ulEndianReadLittle16(model_file_);
	    //DEBUGPRINT( "Start line: idx = " << idx << std::endl);
	    if(vtx_dirty_)
	      {
		last_idx_ = vertex_array_->getNum();
		for(int i = 0; i < curr_vtx_->getNum(); i++)
		  {
		    vertex_array_->add(curr_vtx_ ->get(i));
		    normal_array_->add(curr_norm_->get(i));
		  }
		vtx_dirty_ = false;
	      }
	  
	    curr_part_ = new _MDLPart;
	    curr_part_->type = GL_LINES;
	    curr_part_->vtx  = vertex_array_;
	    curr_part_->nrm  = normal_array_;
	    curr_part_->crd  = NULL;
	    curr_part_->idx  = new ssgIndexArray();

	    curr_part_->idx->add(idx - start_idx_ + last_idx_);

	    curr_part_->removeUnnecessaryVertices();
	    ssgVtxArray* vtab = new ssgVtxArray ( curr_part_->type,
						  curr_part_->vtx,
						  curr_part_->nrm,
						  NULL,
						  NULL,
						  curr_part_->idx ) ;
	    
	    ssgSimpleState* st = createMaterialState(curr_color_, 
						     curr_pal_id_) ;
	    
	    vtab -> setCullFace ( curr_cull_face_ ) ;
	    vtab -> setState ( st ) ;
	    
	    ssgLeaf* leaf = current_options -> createLeaf ( vtab, NULL ) ;
	    char lname[5];
	    sprintf(lname, "%X%X", curr_color_, curr_pal_id_);
	    leaf -> setName(lname);
	    curr_branch_->addKid(leaf);
	  }
	  break;
	
	case 0x10:	// CNTRES: Continue line definition
	  {
	    unsigned short idx;
	    idx = ulEndianReadLittle16(model_file_);
	    //DEBUGPRINT( "Cont. line: idx = " << idx << std::endl);
	    curr_part_->idx->add(idx - start_idx_ + last_idx_);
	  }
	  break;

	case 0x18: 	// Set texture
	  {
	    unsigned short id, dx, scale, dy;
	    id    = ulEndianReadLittle16(model_file_);
	    dx    = ulEndianReadLittle16(model_file_);
	    scale = ulEndianReadLittle16(model_file_);
	    dy    = ulEndianReadLittle16(model_file_);
	    char tex_name[14];
	    fread(tex_name, 1, 14, model_file_);
	    int j = 0;
	    for(int i = 0; i < 14; i++) 
	      {
		if(!isspace(tex_name[i]))
		  curr_tex_name_[j++] = tolower(tex_name[i]);
	      }
	    // for some reason, MSFS likes to store an '_' instead
	    // of the '.' before the file extension (!)
	    //curr_tex_name_[j-4] = '.';
	    curr_tex_name_[j] = '\0';
	    //DEBUGPRINT( "Set texture: name = " << curr_tex_name_ << 
	    //	", id = " << id << ", dx = " << dx << ", dy = " << 
	    //	dy << ", scale = " << scale << std::endl);
	  }
	  break;
	  
	case 0x1a: 	// RESLIST (point list with no normals)
	  {
	    start_idx_ = ulEndianReadLittle16(model_file_);

	    has_normals_ = false;
	    vtx_dirty_   = true;

	    unsigned short numpoints;
	    numpoints = ulEndianReadLittle16(model_file_);

	    //DEBUGPRINT( "New group (unlit): start_idx = " << start_idx_ 
	    // << ", num vertices = " << numpoints << std::endl);

	    sgVec3 null_normal;
	    sgSetVec3(null_normal, 0.0f, 0.0f, 0.0f);

	    delete curr_vtx_ ;
	    delete curr_norm_;
	    curr_vtx_  = new ssgVertexArray();
	    curr_norm_ = new ssgNormalArray();

	    for(int i = 0; i < numpoints; i++) 
	      {
		sgVec3 p;
		readPoint(model_file_, p);
		curr_vtx_ ->add(p);
		curr_norm_->add(null_normal);
	      }
	  }
	  break;

	case 0x24:      // BGL_IFIN1
	  readIfIn1();
	  break;
	
	case 0x29: 	// GORAUD RESLIST (point list with normals)
	  {
	    start_idx_ = ulEndianReadLittle16(model_file_);

	    has_normals_ = true;
	    vtx_dirty_   = true;

	    unsigned short numpoints;
	    numpoints = ulEndianReadLittle16(model_file_);

	    //DEBUGPRINT( "New group (goraud): start_idx = " << start_idx_
	    // << ", num vertices = " << numpoints << std::endl);

	    delete curr_vtx_ ;
	    delete curr_norm_;
	    curr_vtx_  = new ssgVertexArray();
	    curr_norm_ = new ssgNormalArray();

	    for(int i = 0; i < numpoints; i++) 
	      {
		sgVec3 p, v;
		readPoint(model_file_, p);
		readVector(model_file_, v);
		curr_vtx_ ->add(p);
		curr_norm_->add(v);
	      }
	  }
	  break;

	case 0x2a:	// Goraud shaded ABCD Facet
	case 0x3e:	// FACETN (no texture)
	  {
	    if(vtx_dirty_)
	      {
		last_idx_ = vertex_array_->getNum();
		for(int i = 0; i < curr_vtx_->getNum(); i++)
		  {
		    vertex_array_->add(curr_vtx_->get(i));
		    normal_array_->add(curr_norm_->get(i));
		  }
		vtx_dirty_ = false;
	      }
	  
	    curr_part_ = new _MDLPart;
	    curr_part_->type = GL_TRIANGLE_FAN ;
	    curr_part_->vtx  = vertex_array_;
	    curr_part_->nrm  = normal_array_;
	    curr_part_->crd  = NULL;
	    curr_part_->idx  = new ssgIndexArray;

	    unsigned short numverts;
	    numverts = ulEndianReadLittle16(model_file_);
	    //DEBUGPRINT( "New part: (no tex), num indices = " << numverts 
	    //<< std::endl);

	    // Surface normal
	    sgVec3 v;
	    readVector(model_file_, v);

	    curr_cull_face_ = ulEndianReadLittle32(model_file_) >= 0;

	    // Read vertex indices
	    readIndices(model_file_, numverts, v);

	    if(!has_normals_)
	      {
		for(int i = 0; i < curr_part_->idx->getNum(); i++)
		  sgCopyVec3(normal_array_->get(*curr_part_->idx->get(i)), v);
		recalcNormals(curr_part_);
	      }

	    curr_part_->removeUnnecessaryVertices();
	    ssgVtxArray* vtab = new ssgVtxArray ( curr_part_->type,
						  curr_part_->vtx,
						  curr_part_->nrm,
						  NULL,
						  NULL,
						  curr_part_->idx ) ;
	    
	    ssgSimpleState* st = createMaterialState(curr_color_, 
						     curr_pal_id_) ;
	    
	    vtab -> setCullFace ( curr_cull_face_ ) ;
	    vtab -> setState ( st ) ;
	    
	    ssgLeaf* leaf = current_options -> createLeaf ( vtab, NULL ) ;
	    char lname[5];
	    sprintf(lname, "%X%X", curr_color_, curr_pal_id_);
	    leaf -> setName(lname);
	    curr_branch_->addKid(leaf);
	  }
	  break;
		
	case 0x43:      // TEXTURE2
	  {
	    ulEndianReadLittle16(model_file_);  // record length
	    ulEndianReadLittle16(model_file_);  // must be zero
	    get_byte();  // flags, ignored
	    get_byte();  // checksum, must be zero
	    curr_color_  = get_byte();
	    curr_pal_id_ = get_byte();
	    ulEndianReadLittle16(model_file_);  // ??

	    int i;
	    for (i = 0; (curr_tex_name_[i] = get_byte()) != '\0'; i++);
	    
	    if (i % 2 == 0) get_byte();  // padd to even length
	  
	    //DEBUGPRINT( "Set texture: name = " << curr_tex_name_ <<
	    //	"color: " << (int)curr_color_ << " (" << std::hex <<
	    //	curr_pal_id_ << std::dec << ")\n");
	  }
	  break;
	
#ifdef DEBUG
	case 0x46:      // POINT_VICALL (rotate-translate)
	  {
	    DEBUGPRINT( "BGL_POINT_VICALL\t" );
	    DEBUGPRINT( ulEndianReadLittle16( model_file_ ) << "\n" );
	    sgCoord rot;
	    unsigned short hpr_vars[3];
	    readPoint( model_file_, rot.xyz );

	    DEBUGPRINT( rot.xyz[0] << "\t" << rot.xyz[1] << "\t" <<
			rot.xyz[2] << "\n\t" );
	  
	    for (int i = 0; i < 3; i++) {
	      rot.hpr [i] = (float)ulEndianReadLittle16( model_file_ ) / 65536;
	      hpr_vars[i] = ulEndianReadLittle16       ( model_file_ );
	      DEBUGPRINT( rot.hpr[i] << "(" << hpr_vars[i] << ")\t" );
	    }

	    DEBUGPRINT( "\n" );
	  }

	  break;
#endif	
	case 0x50: 	// GCOLOR (Goraud shaded color)
	case 0x51:	// LCOLOR (Line color)
	case 0x52:     	// SCOLOR (Light source shaded surface color)
	  {
	    curr_color_  = get_byte();
	    curr_pal_id_ = get_byte();

	    if(curr_pal_id_ == 0x68) {
	      curr_alpha_ = 0.3f;
	    } else {
	      curr_alpha_ = 1.0f;
	    }
	      
	    //DEBUGPRINT( "Set color = " << (int)curr_color_ << " (" << 
	    //     std::hex << curr_pal_id_ << std::dec << ")\n");
	  }
	  break;

	case 0x7a: 	// Goraud shaded Texture-mapped ABCD Facet
	  {
	    if(vtx_dirty_)
	      {
		last_idx_ = vertex_array_->getNum();
		for(int i = 0; i < curr_vtx_->getNum(); i++)
		  {
		    vertex_array_->add(curr_vtx_ ->get(i));
		    normal_array_->add(curr_norm_->get(i));
		  }
		vtx_dirty_ = false;
	      }
	  
	    curr_part_ = new _MDLPart;
	    curr_part_->type = GL_TRIANGLE_FAN;
	    curr_part_->vtx = vertex_array_;
	    curr_part_->nrm = normal_array_;
	    curr_part_->crd = tex_coords_;
	    curr_part_->idx  = new ssgIndexArray;

	    unsigned short numverts;
	    numverts = ulEndianReadLittle16(model_file_);
	    //DEBUGPRINT( "New part: (goraud/texture), num indices = " << 
	    //	numverts << std::endl);

	    // Unused data
	    sgVec3 v;
	    readVector(model_file_, v);
	    //	  v *= -1.0;
	    curr_cull_face_ = ulEndianReadLittle32(model_file_) >= 0;

	    // Read vertex inidices and texture coordinates
	    char *texture_extension = 
	      curr_tex_name_ + strlen(curr_tex_name_) - 3;
	    bool flip_y = _ssgStrEqual( texture_extension, "BMP" );
	    readTexIndices(numverts, v, flip_y);

	    if(!has_normals_)
	      {
		for(int i = 0; i < curr_part_->idx->getNum(); i++)
		  sgCopyVec3(normal_array_->get(*curr_part_->idx->get(i)),
			     v);
		recalcNormals(curr_part_);
	      }
	    
	    curr_part_->removeUnnecessaryVertices();
	    ssgVtxArray* vtab = new ssgVtxArray ( curr_part_->type,
						  curr_part_->vtx,
						  curr_part_->nrm,
						  curr_part_->crd,
						  NULL,
						  curr_part_->idx ) ;
	    
	    ssgSimpleState* st = createTextureState(curr_tex_name_) ;
	    
	    vtab -> setCullFace ( curr_cull_face_ ) ;
	    vtab -> setState ( st ) ;
	    
	    ssgLeaf* leaf = current_options -> createLeaf ( vtab, NULL ) ;
	    leaf -> setName( curr_tex_name_ );
	    curr_branch_->addKid(leaf);
	  }
	  break;

	case 0x8F:
	  {
	    int value  = ulEndianReadLittle32( model_file_ );
	    if ( value == 0 ) {
	      curr_alpha_ = 1.0f;
	    } else {
	      /* Scenery SDK is a bit fuzzy here, but this works fairly well */
	      curr_alpha_ = 0.3f;
	    }
	  }

	  break;
       
	  //-------------------------------------------
	  // The rest of the codes are either ignored
	  // or for experimental use..
	  //-------------------------------------------
	case 0x03:
	  {
	    //DEBUGPRINT( "BGL_CASE\n" );
	    unsigned short number_cases = ulEndianReadLittle16(model_file_);
	    skip_offset = 6 + 2 * number_cases;
	  }
	  break;

	default: // Unknown opcode
	  {
	    if (opcode < 256) {
	      if ( opcodes[opcode].size != -1) {
		DEBUGPRINT( opcodes[opcode].name << " (size " <<
			    opcodes[opcode].size << ")" << std::endl );
		skip_offset = opcodes[opcode].size - 2; // opcode already read
	      } else {
		DEBUGPRINT( "Unhandled opcode " << opcodes[opcode].name
			    << " (" << std::hex << opcode << std::dec << ")" <<
			    std::endl );
		findPart(model_file_);
	      }
	    } else {
	      DEBUGPRINT( "Op-code out of range: " << std::hex << opcode <<
			  std::dec << std::endl );
	      findPart(model_file_);
	    }
	  }
	  break;
	}
      if (skip_offset > 0) {
	fseek( model_file_, skip_offset, SEEK_CUR );
      }
    }
   
  fclose(model_file_);
  
  delete curr_vtx_;
  delete curr_norm_;

  vertex_array_ -> deRef();
  normal_array_ -> deRef();

  current_options -> end () ;

  return model_;
}
