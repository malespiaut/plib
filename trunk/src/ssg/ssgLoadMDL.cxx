//===========================================================================
// ssgLoadMDL.cxx
// This is a loader for Microsoft Flight Simulator / Flight Shop models
// (MDL-files) to SSG. 
//
// Original code by Thomas Engh Sevaldrud, adapted to SSG by
// Per Liedman.

#include <iostream.h>
#include "ssgLocal.h"
#include "ssgLoadMDL.h"

#define DEF_SHININESS 50
#define MSFS_MAX_STATES 256

// Define DEBUG if you want some debug info
#define DEBUG 1

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

static bool			has_normals_, vtx_dirty_, join_children_;
static FILE*                    model_file_;
static int                      is_little_endian_;


//static void joinChildren(ssgEntity*);

//==========================================================
// ENDIAN ISSUES
static inline void endian_swap(unsigned int *x) {
  *x = (( *x >> 24 ) & 0x000000FF ) | 
    (( *x >>  8 ) & 0x0000FF00 ) | 
    (( *x <<  8 ) & 0x00FF0000 ) | 
    (( *x << 24 ) & 0xFF000000 ) ;
}

static inline void endian_swap(unsigned short *x) {
  *x = (( *x >>  8 ) & 0x00FF ) | 
    (( *x <<  8 ) & 0xFF00 ) ;
}

/*
static float get_float() {
  float f;
  fread( &f, 4, 1, model_file_ );

  if (is_little_endian_)
    return f;
  else {
    endian_swap((unsigned int*)&f);
    return f;
  }
}
*/

static unsigned int get_dword() {
  unsigned int d;
  fread( &d, 4, 1, model_file_ );

  if (is_little_endian_)
    return d;
  else {
    endian_swap(&d);
    return d;
  }
}

static unsigned short get_word() {
  unsigned short w;
  fread( &w, 2, 1, model_file_ );
  
  if (is_little_endian_)
    return w;
  else {
    endian_swap(&w);
    return w;
  }
}

static unsigned char get_byte() {
  unsigned char b;
  fread( &b, 1, 1, model_file_ );
  return b;
}

//===========================================================================

static bool findPart(FILE* fp)
{
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

      for(int i = 0; i < 4; i++)
	{
	  if(pattern[i] == pattern1[i]) match1++;
	  if(pattern[i] == pattern2[i]) match2++;
	  if((pattern[i] == 0x24 || pattern[i] == 0x1c /*||
				   pattern[i] == 0x7C ||
				   pattern[i] == 0xB3*/
	      ) && matchpos < 0) matchpos = i;
	}

      if(match1 == 4)
	{
	  fseek(fp, -4, SEEK_CUR);
	  DEBUGPRINT( "found vertices at " << hex << ftell(fp) << dec << endl);
	  return true;
	}

      else if(match2 == 4)
	{
	  fseek(fp, -4, SEEK_CUR);
	  DEBUGPRINT( "found vertices at " << hex << ftell(fp) << dec << endl);
	  return true;
	}
      else if(matchpos >= 0) 
	{
	  long pos = ftell(fp);

	  unsigned short var;
	  short offset, high, low;
	  fseek(fp, -3+matchpos, SEEK_CUR);
	  offset = get_word();
	  var    = get_word();
	  low    = get_word();
	  high   = get_word();
	  DEBUGPRINT( "JumpOnVar (" << hex << (int)pattern[matchpos] << 
		      "): var " << var << dec << ", offset: " << offset <<
		      ", value: " << low << " < " << high << endl );

	  int part_idx;
	  switch (var) {
	  case 0x5200:
	  case 0x6c00:
	    part_idx = PART_FLAPS; break;
	  case 0x5400:
	  case 0x6e00:
	    part_idx = PART_GEAR; break;
	  case 0x5a00:
	  case 0x7400:
	    part_idx = PART_PROP; break;
	  case 0x5c00:
	  case 0x7600:
	    part_idx = PART_LIGHTS; break;
	  case 0x5e00:
	  case 0x7800:
	    part_idx = PART_STROBE; break;
	  case 0x6200:
	  case 0x7c00:
	    part_idx = PART_SPOILERS; break;
	  default:
	    part_idx = -1; break;
	  }

	  if (part_idx == -1) {
	    curr_branch_ = model_;
	  } else {
	    if (moving_parts_[part_idx] == NULL) {
	      moving_parts_[part_idx] = new ssgSelector;
	      model_->addKid(moving_parts_[part_idx]);	      	   
	      moving_parts_[part_idx]->setName( PART_NAME[part_idx] );
	      moving_parts_[part_idx]->addKid( new ssgBranch() );
	    }

	    curr_branch_ = (ssgBranch*)moving_parts_[part_idx]->getKid(0);
	  }

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
  y_int = get_word();
  z_int = get_word();
  x_int = get_word();
 
  // Convert from .MDL units (ca 2mm) to meters
  p[0] =  -(float)x_int/512.0;
  p[1] =  (float)y_int/512.0;
  p[2] =  (float)z_int/512.0;
}


//===========================================================================

static void readVector(FILE* fp, sgVec3 v)
{
  short x_int, y_int, z_int;
  y_int = get_word();
  z_int = get_word();
  x_int = get_word();

  v[0] = -(float)x_int;
  v[1] = (float)y_int;
  v[2] = (float)z_int;

  sgNormaliseVec3( v );
}

//===========================================================================

static void recalcNormals( _MDLPart *part ) {
  DEBUGPRINT( "Calculating normals." << endl);
  sgVec3 v1, v2, n;

  for (int i = 0; i < part->idx->getNum() / 3; i++) {
    short ix0 = *part->idx->get(i*3    );
    short ix1 = *part->idx->get(i*3 + 1);
    short ix2 = *part->idx->get(i*3 + 2);

    sgSubVec3(v1, 
	      curr_part_->vtx->get(ix1),
	      curr_part_->vtx->get(ix0));
    sgSubVec3(v2,
	      curr_part_->vtx->get(ix2),
	      curr_part_->vtx->get(ix0));
    
    sgVectorProductVec3(n, v1, v2);

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
      curr_part_->idx->add(*ixarr->get(0));
      curr_part_->idx->add(*ixarr->get(0));
      curr_part_->idx->add(*ixarr->get(0));
    }

  else if(numverts == 2)
    {
      curr_part_->idx->add(*ixarr->get(0));
      curr_part_->idx->add(*ixarr->get(1));
      curr_part_->idx->add(*ixarr->get(0));
    }

  else if(numverts == 3)
    {
      sgSubVec3(v1, 
		curr_part_->vtx->get(*ixarr->get(1)), 
		curr_part_->vtx->get(*ixarr->get(0)));
      sgSubVec3(v2, 
		curr_part_->vtx->get(*ixarr->get(2)),
		curr_part_->vtx->get(*ixarr->get(0)));
    
      sgVectorProductVec3(cross, v1, v2);

      if(sgScalarProductVec3(cross, s_norm) > 0.0f)
	{
	  curr_part_->idx->add(*ixarr->get(0));
	  curr_part_->idx->add(*ixarr->get(1));
	  curr_part_->idx->add(*ixarr->get(2));
	}
      else
	{
	  curr_part_->idx->add(*ixarr->get(0));
	  curr_part_->idx->add(*ixarr->get(2));
	  curr_part_->idx->add(*ixarr->get(1));
	}
    }

  else
    {
      unsigned short ix0 = *ixarr->get(0);
      for(int i = 2; i < numverts; i++)
	{
	  unsigned short ix1 = *ixarr->get(i-1);
	  unsigned short ix2 = *ixarr->get(i);

	  // Ensure counter-clockwise ordering
	  sgSubVec3(v1, 
		    curr_part_->vtx->get(ix1),
		    curr_part_->vtx->get(ix0));
	  sgSubVec3(v2,
		    curr_part_->vtx->get(ix2),
		    curr_part_->vtx->get(ix0));
       
	  sgVectorProductVec3(cross, v1, v2);

	  if(sgScalarProductVec3(cross, s_norm) > 0.0)
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
       
    }
}

//===========================================================================

static bool readTexIndices(FILE* fp, int numverts, const sgVec3 s_norm)
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

      ix     = get_word();
      tx_int = get_word();
      ty_int = get_word();

      int tex_idx = ix - start_idx_ + last_idx_;

      sgVec2 tc;
      sgSetVec2(tc, double(tx_int)/255.0, double(ty_int)/255.0);

      sgVec2 curr_tc;
      sgCopyVec2(curr_tc, tex_coords_->get(tex_idx));

      double dist = sgDistanceVec2(curr_tc, tc);
    
      if((curr_tc[0] >= FLT_MAX - 1 && curr_tc[1] >= FLT_MAX - 1))
	{
	  DEBUGPRINT( "." );
	  sgCopyVec2(tex_coords_->get(tex_idx), tc);
	}

      else if(dist > 0.0001)
	{
	  // We have a different texture coordinate for an existing vertex,
	  // so we have to copy this vertex and create a new index for it
	  // to get the correct texture mapping.

	  DEBUGPRINT( "duplicating texture coordinate!\n");
       
	  int idx = ix - start_idx_ + last_idx_;
	  tex_idx = curr_part_->vtx->getNum();

	  ssgVertexArray* vtx_arr  = curr_part_->vtx;
	  ssgNormalArray* norm_arr = curr_part_->nrm;
       
	  vtx_arr ->add(vtx_arr ->get(idx));
	  norm_arr->add(norm_arr->get(idx));
       
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

bool readIndices(FILE* fp, int numverts, const sgVec3 s_norm)
{
  if(numverts <= 0)
    return false;
	  
  // Read index values
  ssgIndexArray ixarr(numverts);
  for(int v = 0; v < numverts; v++)
    {
      unsigned short ix;
      ix = get_word();
      ixarr.add(ix - start_idx_ + last_idx_);
      DEBUGPRINT( "ix[" << v << "] = " << *ixarr.get(v) << endl);
      //ixarr.insert(v, ix - start_idx_);
    }

  createTriangIndices(&ixarr, numverts, s_norm);
 
  return true;
}

//===========================================================================

ssgSimpleState* createMaterialState(int color, int pal_id)
{ 
  ssgSimpleState* state = new ssgSimpleState();
    
  float r, g, b, a;
  if(pal_id == 0x68) 
    {
      state->setTranslucent();
      state->enable(GL_BLEND);
      state->enable(GL_ALPHA_TEST);
      r = (float)(fsAltPalette[color].r)/255.0;
      g = (float)(fsAltPalette[color].g)/255.0;
      b = (float)(fsAltPalette[color].b)/255.0;
      a = 0.3f;
    }
  else 
    {
      state->disable(GL_BLEND);
      state->disable(GL_ALPHA_TEST);
      r = (float)(fsAcPalette[color].r)/255.0;
      g = (float)(fsAcPalette[color].g)/255.0;
      b = (float)(fsAcPalette[color].b)/255.0;
      a = 1.0;
    }
    
  DEBUGPRINT( "  Creating non-textured state: color = (" << r << ", " << g <<
	      ", " << b << ")" << endl);
 
  state->setMaterial(GL_AMBIENT , r   , g   , b   , a   );
  state->setMaterial(GL_DIFFUSE , r   , g   , b   , a   );
  state->setMaterial(GL_SPECULAR, 0.0f, 0.0f, 0.0f, 1.0f);
  state->setMaterial(GL_EMISSION, 0.0f, 0.0f, 0.0f, 1.0f);
  state->setShininess(DEF_SHININESS);
  state->disable(GL_TEXTURE_2D);
  state->enable(GL_LIGHTING);
  state->setShadeModel(GL_SMOOTH);
  state->disable(GL_COLOR_MATERIAL);

  return state;
}


//===========================================================================

ssgSimpleState* createTextureState(char *name)
{
  ssgSimpleState* state = new ssgSimpleState();

  strcpy(curr_tex_name_, name);
 
  state->setMaterial(GL_AMBIENT , 1.0f, 1.0f, 1.0f, 1.0f);
  state->setMaterial(GL_DIFFUSE , 1.0f, 1.0f, 1.0f, 1.0f);
  state->setMaterial(GL_SPECULAR, 0.0f, 0.0f, 0.0f, 1.0f);
  state->setMaterial(GL_EMISSION, 0.0f, 0.0f, 0.0f, 1.0f);
  state->setShadeModel(GL_SMOOTH);
  state->setShininess(DEF_SHININESS);
  state->enable(GL_LIGHTING);
  state->disable(GL_COLOR_MATERIAL);
  state->enable(GL_TEXTURE_2D);

  DEBUGPRINT( "  Creating texture state: name = " << name << endl);

  return state;
}
//===========================================================================

ssgEntity *ssgLoadMDL( const char* fname, ssgHookFunc hook )
{
  int endiantest = 1 ;
  is_little_endian_ = *((char *) &endiantest );

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
 
  //vertex_array_tex_ = new ssgVertexArray();
  //normal_array_tex_ = new ssgNormalArray();
  tex_coords_ = new ssgTexCoordArray();
 
  unsigned int code_len;
  fseek(model_file_, 0x11fc, SEEK_SET);
  code_len = get_dword();
  code_len -= 0x12;
  
  DEBUGPRINT( "code length = " << code_len << " bytes\n");
  
  start_idx_ = 0;
  last_idx_  = 0;
 
  findPart(model_file_);
  
  // Parse opcodes
  bool done = false;
  while(!feof(model_file_) && !done) 
    {
      unsigned short opcode;

      opcode = get_word();

      switch(opcode)
	{
	case 0x02:      // NOOP
	  break;

	case 0x1a: 	// RESLIST (point list with no normals)
	  {
	    start_idx_ = get_word();

	    has_normals_ = false;
	    vtx_dirty_   = true;

	    unsigned short numpoints;
	    numpoints = get_word();

	    DEBUGPRINT( "New group (unlit): start_idx = " << start_idx_ 
		 << ", num vertices = " << numpoints << endl);

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
	
	case 0x29: 	// GORAUD RESLIST (point list with normals)
	  {
	    start_idx_ = get_word();

	    has_normals_ = true;
	    vtx_dirty_   = true;

	    unsigned short numpoints;
	    numpoints = get_word();

	    DEBUGPRINT( "New group (goraud): start_idx = " << start_idx_
		 << ", num vertices = " << numpoints << endl);

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
	
	case 0x0f:	// STRRES: Start line definition
	  {
	    unsigned short idx;
	    idx = get_word();
	    DEBUGPRINT( "Start line: idx = " << idx << endl);
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

      ssgVtxArray* vtab = new ssgVtxArray ( curr_part_->type,
        curr_part_->vtx,
        curr_part_->nrm,
        NULL,
        NULL,
        curr_part_->idx ) ;

      ssgSimpleState* st = createMaterialState(curr_color_, curr_pal_id_) ;

      vtab -> setCullFace ( TRUE ) ;
      vtab -> setState ( st ) ;

      ssgLeaf* leaf = (*_ssgCreateFunc) ( vtab, NULL, NULL ) ;
	    curr_branch_->addKid(leaf);
	  }
	  break;
	
	case 0x10:	// CNTRES: Continue line definition
	  {
	    unsigned short idx;
	    idx = get_word();
	    DEBUGPRINT( "Cont. line: idx = " << idx << endl);
	    curr_part_->idx->add(idx - start_idx_ + last_idx_);
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
	    curr_part_->type = GL_TRIANGLES;
	    curr_part_->vtx = vertex_array_;
	    curr_part_->nrm = normal_array_;
	    curr_part_->crd = tex_coords_;
	    curr_part_->idx  = new ssgIndexArray;

	    unsigned short numverts;
	    numverts = get_word();
	    DEBUGPRINT( "New part: (goraud/texture), num indices = " << numverts << endl);

	    // Unused data
	    sgVec3 v;
	    readVector(model_file_, v);
	    //	  v *= -1.0;
	    unsigned int d;
	    fread(&d, 4, 1, model_file_);  // we don't care about endian here

	    // Read vertex inidices and texture coordinates
	    readTexIndices(model_file_, numverts, v);

	    if(!has_normals_)
	      {
		for(int i = 0; i < curr_part_->idx->getNum(); i++)
		  sgCopyVec3(normal_array_->get(*curr_part_->idx->get(i)),
			     v);
		recalcNormals(curr_part_);
	      }
	    
      ssgVtxArray* vtab = new ssgVtxArray ( curr_part_->type,
        curr_part_->vtx,
        curr_part_->nrm,
        curr_part_->crd,
        NULL,
        curr_part_->idx ) ;

      ssgSimpleState* st = createTextureState(curr_tex_name_) ;

      vtab -> setCullFace ( TRUE ) ;
      vtab -> setState ( st ) ;

      ssgLeaf* leaf = (*_ssgCreateFunc) ( vtab, curr_tex_name_, NULL ) ;
	    curr_branch_->addKid(leaf);
	  }
	  break;
       
	case 0x3e:	// FACETN (no texture)
	case 0x2a:	// Goraud shaded ABCD Facet
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
	    curr_part_->type = GL_TRIANGLES ;
	    curr_part_->vtx  = vertex_array_;
	    curr_part_->nrm  = normal_array_;
	    curr_part_->crd  = NULL;
	    curr_part_->idx  = new ssgIndexArray;

	    unsigned short numverts;
	    numverts = get_word();
	    DEBUGPRINT( "New part: (no tex), num indices = " << numverts << endl);

	    // Surface normal
	    sgVec3 v;
	    readVector(model_file_, v);

	    get_dword();  // dummy data

	    // Read vertex indices
	    readIndices(model_file_, numverts, v);

	    if(!has_normals_)
	      {
		for(int i = 0; i < curr_part_->idx->getNum(); i++)
		  sgCopyVec3(normal_array_->get(*curr_part_->idx->get(i)), v);
		recalcNormals(curr_part_);
	      }

      ssgVtxArray* vtab = new ssgVtxArray ( curr_part_->type,
        curr_part_->vtx,
        curr_part_->nrm,
        NULL,
        NULL,
        curr_part_->idx ) ;

      ssgSimpleState* st = createMaterialState(curr_color_, curr_pal_id_) ;

      vtab -> setCullFace ( TRUE ) ;
      vtab -> setState ( st ) ;

      ssgLeaf* leaf = (*_ssgCreateFunc) ( vtab, NULL, NULL ) ;
	    curr_branch_->addKid(leaf);
	  }
	  break;
	
	case 0x18: 	// Set texture
	  {
	    unsigned short id, dx, scale, dy;
	    id    = get_word();
	    dx    = get_word();
	    scale = get_word();
	    dy    = get_word();
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
	    DEBUGPRINT( "Set texture: name = " << curr_tex_name_ << 
			", id = " << id << ", dx = " << dx << ", dy = " << 
			dy << ", scale = " << scale << endl);
	  }
	  break;
	
	case 0x50: 	// GCOLOR (Goraud shaded color)
	case 0x51:	// LCOLOR (Line color)
	case 0x52:     	// SCOLOR (Light source shaded surface color)
	  {
	    curr_color_  = get_byte();
	    curr_pal_id_ = get_byte();
	    DEBUGPRINT( "Set color = " << (int)curr_color_ << " (" << hex << 
			curr_pal_id_ << dec << ")\n");
	  }
	  break;

	case 0x08: 	// CLOSURE
	  {
	    DEBUGPRINT( "CLOSURE\n\n");

	    findPart(model_file_);
	    continue;
	  }
	  break;
	  
	case 0x0:	// EOF
	case 0x22: 	// BGL return
	  {
	    DEBUGPRINT( "BGL return\n\n");

	    findPart(model_file_);
	  }
	  break;

	  //-------------------------------------------
	  // The rest of the codes are either ignored
	  // or for experimental use..
	  //-------------------------------------------

	  // These have no endian handling, since they are ignored
	
	case 0x23:	// BGL Call subroutine
	  {
	    unsigned short rel_addr;
	    fread(&rel_addr, 2, 1, model_file_);
	  }
	  break;
       
	case 0x39:      	// Relative Jump 
	  {
	    unsigned short rel_addr, var, var_offset, mask;
	    fread(&rel_addr, 2, 1, model_file_);
	    fread(&var_offset, 2, 1, model_file_);
	    fread(&mask, 2, 1, model_file_);
	    long pos = ftell(model_file_);
	    fseek(model_file_, var_offset-5, SEEK_CUR);
	    fread(&var, 2, 1, model_file_);
	    DEBUGPRINT( "JumpOnVar = " << hex << var_offset << dec << endl);
	    fseek(model_file_, pos, SEEK_SET);
	    /*
	      if((var & mask) == 0)
	      {
	      DEBUGPRINT( "JUMP " << rel_addr << " bytes\n");
	      long pos = ftell(model_file_);
	      addr_stack_.insertFirst(pos);
	      fseek(model_file_, rel_addr, SEEK_CUR);
	      }
	    */
	  }
	  break;
	
	case 0x1c:	// IfVarRange2
	  {
	    unsigned short offset, var1, minval1, maxval1, var2, minval2, maxval2;
	    fread(&offset,  2, 1, model_file_);
	    fread(&var1,    2, 1, model_file_);
	    fread(&minval1, 2, 1, model_file_);
	    fread(&maxval1, 2, 1, model_file_);
	    fread(&var2,    2, 1, model_file_);
	    fread(&minval2, 2, 1, model_file_);
	    fread(&maxval2, 2, 1, model_file_);
	  
	    DEBUGPRINT( "offset: " << offset << ", var1 = " << var1
		 << ", range1 = [ " << minval1 << ", " << maxval1 << " ], var2 = " << var2 
		 << ", range2 = [ " << minval2 << ", " << maxval2 << " ]\n");
	  }
	  break;
	
	case 0x41:	// SHADOW VINSTANCE -- ignored
	  {
	    unsigned char param[4];
	    fread(param, 1, 4, model_file_);
	  }
	  break;
	
	
	case 0x68: 	// TMAP LIGHT SOURCE SHADE - ignored
	  {
	    sgVec3 v;
	    readVector(model_file_, v);
	  }
	  break;
	
	case 0x38:	// CONCAVE override -- ignored
	  {
	    ;
	  }
	  break;

	case 0x35:	// PNTROW -- ignored
	  {
	    sgVec3 p;
	    readPoint(model_file_, p);
	    readPoint(model_file_, p);
	    unsigned short reserve;
	    fread(&reserve, 2, 1, model_file_);
	  }
	  break;
	
	case 0x34:	// SUPER_SCALE -- not used 
	  {
	    unsigned short offset, v1, v2, sx;
	    fread(&offset, 2, 1, model_file_);
	    fread(&v1, 2, 1, model_file_);
	    fread(&v2, 2, 1, model_file_);
	    fread(&sx, 2, 1, model_file_);
	    DEBUGPRINT( "offset = " << offset << endl);
	    DEBUGPRINT( "v1 = " << v1 << ", v2 = " << v2 << ", sx = " << sx << endl);
	    DEBUGPRINT( "scale = " << (double)(1 << sx)/65536.0 << endl);
	  }
	  break;
	
	case 0x3a:	// BGL VPOSITION -- ignored
	  {
	    unsigned char param[10];
	    fread(param, 1, 10, model_file_);
	    //	  unsigned short addr;
	    //	  fread(&addr, 2, 1, model_file_);
	    //	  DEBUGPRINT( "vpoint addr = " << addr << endl);
	  }
	  break;

	case 0x3b:	// VINSTANCE Call --ignored
	  {
	    unsigned char param[4];
	    fread(param, 1, 4, model_file_);
	  }
	  break;
	
	case 0x12:	// GSTRRES -- Ignored
	  {
	    unsigned short var;
	    fread(&var, 2, 1, model_file_);
	  }
	  break;

	case 0x7d: 	// PERSPECTIVE OVERRIDE
	  {
	    ;
	  }
	  break;
	
	case 0x04:	// DEBUG
	  {
	    ;
	  }
	  break;

	case 0x81:	// ANTI-ALIAS on/off
	  {
	    unsigned short val;
	    fread(&val, 2, 1, model_file_);
	  }
	  break;
	
	case 0x82:	// SHADOW Position -- ignored
	  {
	    char pos[18];
	    fread(pos, 1, 18, model_file_);
	  }
	  break;
	
	case 0x8A:	// BGL Call subroutine
	  {
	    unsigned short rel_addr;
	    fread(&rel_addr, 4, 1, model_file_);
	  }
	  break;
       
	default: // Unknown opcode, bail out
	  {
	    done = true;
	    ulSetError( UL_WARNING, "ssgLoadMDL: Unknown opcode = %X. " \
			"Aborting.", opcode);
	  }
	  break;
	}       
    }
  
  fclose(model_file_);
  
  delete curr_vtx_;
  delete curr_norm_;

  //joinChildren( model_ );
  
  return model_;
}


//===========================================================================

/*static void joinChildren(ssgEntity* grp)
{
  if ( grp->isAKindOf(ssgTypeBranch()) ) {
    ssgBranch* b = (ssgBranch*)grp;
    ssgIndexArray* idx = NULL;

    for (int i = 0; i < b->getNumKids(); i++) {
      ssgEntity* k = b->getKid(i);

      if ( k->isA(ssgTypeVtxArray()) ) {
	ssgVtxArray *vtx = (ssgVtxArray*)k;
	if (vtx->getGLtype() == GL_TRIANGLES) {
	  if (idx == NULL) {
	    idx = new ssgIndexArray();
	  }

	  for (int j = 0; j < vtx->getNumIndices(); j++) {
	    idx->add(*vtx->getIndex(j));
	  }

	  b->removeKid(i--);
	}
      } else {
	joinChildren( k );
      }
    }

    if (idx != NULL) {
      ssgVtxArray *v = new ssgVtxArray( GL_TRIANGLES,
					vertex_array_,
					normal_array_,
					tex_coords_,
					NULL,
					idx );
      b->addKid(v);
    }
  }
} 
*/
