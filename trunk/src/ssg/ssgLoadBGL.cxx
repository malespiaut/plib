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

//===========================================================================
//                                                                           
// File: GngMsfsIO.cpp                                                       
//                                                                           
// Created: Tue Feb 29 22:20:31 2000                                         
//                                                                           
// Author: Thomas Engh Sevaldrud <tse@math.sintef.no>
//                                                                           
// Revision: $Id$
//                                                                           
// Description:
//
// Changes:  by Juergen Marquardt
//           - reference point support 0x2f, 0x77, 0x3c added
//             the first refpoint will be taken as reference point for the scenery
//             all other will be calculated relative to this one
//           - scaling of objects is handled now correctly
//           - current branch will be pushed on stack when calling and restored
//             by return
//           - rotations and and translations are now done by ssgTransform
//           - super scale support (0x34)
//           - support for SCOLOR24 (0x2d) added
//           - bgl building corrected ( center is in the middle of the building)
//           - support for bgl variables added
//           - branch support on IFMSK (0x39), IFIN1 (0x24), IFIN3 (0x21)
//           - color handling corrected
//           - HAZE (0x1e) support
//           - support for transparent and transluctent textures added
//           - support for lines (06,07) added
//           - partial support for 0xa0 and 0x70 added, actally these are ignored
//             at the momnet but at least they are paresed correctly ;)
//           - ssgLoadMDLTexture removed from here. It's now a seperate file.
//
//===========================================================================
// Copyright (c) 2000 Thomas E. Sevaldrud <tse@math.sintef.no>
// Copyright (c) 2002 Jürgen Marquardt <juergen_marquardt@t-online.de>
//===========================================================================

#include "ssgLocal.h"

#ifdef SSG_LOAD_BGL_SUPPORTED

#include "ssgLoadMDL.h"

#define DEF_SHININESS 50

//#define DEBUG
//#define JMDEBUG
//#define EXPERIMENTAL_CULL_FACE_CODE

#ifdef DEBUG
#include <iostream>
#define DEBUGPRINT(x) std::cerr << x
#else
#define DEBUGPRINT(x)
#endif

#include <iostream>

#ifdef JMDEBUG
#define JMPRINT(x,y) cout.flags(x); cout << y << "\n"
#else
#define JMPRINT(x,y)
#endif

#ifdef PRINT_JOHN
#define PRINT_JOHN(x) PRINT_JOHN(x)
#define PRINT_JOHN2(x,y) PRINT_JOHN2(x,y)
#define PRINT_JOHN3(x,y,z) PRINT_JOHN3(x,y,z)
#else
#define PRINT_JOHN(x)
#define PRINT_JOHN2(x,y)
#define PRINT_JOHN3(x,y,z)
#endif

#define EARTH_RADIUS 6367311.808

static ssgLoaderOptions         *current_options;

// Temporary vertex arrays
static ssgIndexArray            *curr_index_;

// Vertex arrays
static ssgVertexArray           *vertex_array_;
static ssgNormalArray           *normal_array_;
static ssgTexCoordArray         *tex_coords_;

// Current part (index array)
static ssgLeaf                  *curr_part_=0;
static ssgBranch                *model_;
static ssgTransform             *curr_branch_=0;

// Moving parts
static ssgBranch                *ailerons_grp_, *elevator_grp_, *rudder_grp_;
static ssgBranch                *gear_grp_, *spoilers_grp_, *flaps_grp_;
static ssgBranch                *prop_grp_;

static sgMat4                   curr_matrix_;
static sgVec3                   curr_rot_pt_;
static sgVec4                   curr_col_;
static char                     *curr_tex_name_;
static int                      curr_tex_type_;
static ssgAxisTransform         *curr_xfm_;

#ifdef EXPERIMENTAL_CULL_FACE_CODE
static bool                     curr_cull_face_;
#endif

// File Address Stack
static const int                MAX_STACK_DEPTH = 64; // wk: 32 is too small
static long                     stack_[MAX_STACK_DEPTH];
static int                      stack_depth_;


static bool                     has_normals_;
static bool                     join_children_, override_normals_;

static char                     *tex_fmt_;
// john ....
bool                            poly_from_line;
unsigned short                  poly_from_line_numverts;

//john ........
ssgTransform                    *building = NULL;
ssgVertexArray                  *vertices_walls = NULL;
ssgNormalArray                  *normals_walls = NULL;
ssgVertexArray                  *vertices_roof = NULL;
ssgNormalArray                  *normals_roof = NULL;
ssgVertexArray                  *vertices_base = NULL;
ssgNormalArray                  *normals_base = NULL;
ssgVtxTable                     *pVtx_walls = NULL;
ssgVtxTable                     *pVtx_roof = NULL;
ssgVtxTable                     *pVtx_base = NULL;
ssgVtxTable                     *pVtx;

long                            object_end;
long                            object_end_offset;
long                            object_position_lat;
long                            object_position_lon;

int                             building_count = 0; // temp test


// jm's variables
long                            scenery_center_lat;
long                            scenery_center_lon;

double                          ref_scale;
double                          ref_delta_lat, ref_delta_lng, ref_delta_alt;
short                           haze_;
char                            tex_filename[128];
struct {
  sgVec3 point;
  sgVec3 norm;
  int    index;
  }                             tmp_vtx[400];
bool                            blending;

// This struct contains variable definitions of MS Flightsimulator
// upt to now only season, complexity and day time are supported
// unfortunately we have to set these varibales befor loading the bgl file
// thus we can't chnage them on the fly
struct {
  int var;
  int val;
  }                             vardef[100] = { {0x346,4},      // complexity: 0 lowest; 4 most
                                                {0x6f8,2},      // season: 0=winter; 1=spring; 
                                                                //         2=summer; 3=autumn;
                                                {0x28c,0x05},   // Day time: 0=Day, 1=Dusk,
                                                                //           2=Night, bit2=light on/off
                                                {0x000,0}       // END of table
                                              };


//===========================================================================

int getVariableValue(int var, int *val)
{
for (int i=0; vardef[i].var != 0; i++){
  if (vardef[i].var == var ) {
    *val = vardef[i].val;
    return (0);
    }
  }
  return(1);
}

//===========================================================================

static void initLoader()
{
  join_children_    = true;
  override_normals_ = true;
  tex_fmt_          = "tif";
  stack_depth_      = 0;
#ifdef EXPERIMENTAL_CULL_FACE_CODE
  curr_cull_face_   = false ;
#endif
}

//===========================================================================

static void newPart()
{
  curr_tex_name_ = NULL;
  sgSetVec4( curr_col_, 1.0f, 1.0f, 1.0f, 1.0f );
}

//===========================================================================

static void push_stack( long entry ) {
  assert( stack_depth_ < MAX_STACK_DEPTH - 1 );
  
  stack_[stack_depth_++] = entry;
}

//===========================================================================

static long pop_stack() {
  assert( stack_depth_ > 0 );
  
  return stack_[--stack_depth_];
}

//===========================================================================

static void recalcNormals() {
  DEBUGPRINT( "Calculating normals." << std::endl);
  sgVec3 n;

  for (int i = 0; i < curr_index_->getNum() - 2; i++) {

    unsigned short ix0 = *curr_index_->get(i    );
    unsigned short ix1 = *curr_index_->get(i + 1);
    unsigned short ix2 = *curr_index_->get(i + 2);

    sgMakeNormal( n,
      vertex_array_->get(ix0),
      vertex_array_->get(ix1),
      vertex_array_->get(ix2) );

    sgCopyVec3( normal_array_->get(ix0), n );
    sgCopyVec3( normal_array_->get(ix1), n );
    sgCopyVec3( normal_array_->get(ix2), n );
  }
}

//===========================================================================

static void readPoint(FILE* fp, sgVec3 p)
{
  short x_int, y_int, z_int;
  y_int = ulEndianReadLittle16(fp) ;
  z_int = ulEndianReadLittle16(fp) ;
  x_int = ulEndianReadLittle16(fp) ;

  p[0] =  - (x_int / ref_scale) ;
  p[1] =  y_int / ref_scale ;
  p[2] =  z_int / ref_scale ;

}

//===========================================================================

static void readVector(FILE* fp, sgVec3 v)
{
  short x_int, y_int, z_int;
  y_int = ulEndianReadLittle16(fp);
  z_int = ulEndianReadLittle16(fp);
  x_int = ulEndianReadLittle16(fp);

  v[0] = -(float)x_int;
  v[1] = (float)y_int;
  v[2] = (float)z_int;

  sgNormaliseVec3( v );
}

//===========================================================================

static void readRefPointTranslation(FILE* fp, sgVec3 v)
{
  unsigned long tmp;
  long long dlat, dlon;
  long long ref_lat, ref_lng, ref_alt;
  double lat_radius;

  tmp = ulEndianReadLittle16(fp);
  ref_lat = (long)ulEndianReadLittle32(fp); // Latitude
  ref_lat = (ref_lat<<16)+tmp;
  tmp = ulEndianReadLittle16(fp);
  ref_lng = (long)ulEndianReadLittle32(fp); // Longitude
  ref_lng = (ref_lng<<16)+tmp;
  tmp = ulEndianReadLittle16(fp);
  ref_alt = (long)ulEndianReadLittle32(fp); // Alitude
  ref_alt = (ref_alt<<16)+tmp;
  dlat = ref_lat - (((long long)scenery_center_lat)<<16);
  v[0] = -(double)dlat / 65536.0;
  lat_radius = cos(scenery_center_lat*M_PI/(2*10001750.0)) * EARTH_RADIUS;
  dlon = ref_lng - (((long long)scenery_center_lon)<<16);
  v[1] = ((double)dlon * 2.2322358e-14 ) * lat_radius;
  v[2] = (double) ref_alt / 65536.0;
}

//===========================================================================

static void createTriangIndices(ssgIndexArray *ixarr,
                                int numverts, const sgVec3 s_norm)
{
  sgVec3 v1, v2, cross;

  if ( numverts > ixarr->getNum() ) {
    ulSetError( UL_WARNING, "ssgLoadBGL: Index array with too few entries." );
    return;
  }

  // triangulate polygons
  if(numverts == 1)
  {
    unsigned short ix0 = *ixarr->get(0);
    if ( ix0 >= vertex_array_->getNum() ) {
      ulSetError(UL_WARNING, "ssgLoadBGL: Index out of bounds (%d/%d).",
        ix0, vertex_array_->getNum() );
      return;
    }

    curr_index_->add(ix0);
    curr_index_->add(ix0);
    curr_index_->add(ix0);
  }

  else if(numverts == 2)
  {
    unsigned short ix0 = *ixarr->get(0);
    unsigned short ix1 = *ixarr->get(1);
    if ( ix0 >= vertex_array_->getNum() ||
      ix1 >= vertex_array_->getNum() ) {
      ulSetError(UL_WARNING, "ssgLoadBGL: Index out of bounds. (%d,%d / %d",
        ix0, ix1, vertex_array_->getNum() );
      return;
    }

    curr_index_->add(ix0);
    curr_index_->add(ix1);
    curr_index_->add(ix0);
  }

  else if(numverts == 3)
  {
    unsigned short ix0 = *ixarr->get(0);
    unsigned short ix1 = *ixarr->get(1);
    unsigned short ix2 = *ixarr->get(2);
    if ( ix0 >= vertex_array_->getNum() ||
      ix1 >= vertex_array_->getNum() ||
      ix2 >= vertex_array_->getNum() ) {
      ulSetError(UL_WARNING, "ssgLoadBGL: Index out of bounds. " \
        "(%d,%d,%d / %d)", ix0, ix1, ix2, vertex_array_->getNum());
      return;
    }

    sgSubVec3(v1,
      vertex_array_->get(ix1),
      vertex_array_->get(ix0));
    sgSubVec3(v2,
      vertex_array_->get(ix2),
      vertex_array_->get(ix0));

    sgVectorProductVec3(cross, v1, v2);

    if(sgScalarProductVec3(cross, s_norm) > 0.0f)
    {
      curr_index_->add(ix0);
      curr_index_->add(ix1);
      curr_index_->add(ix2);
    }
    else
    {
      curr_index_->add(ix0);
      curr_index_->add(ix2);
      curr_index_->add(ix1);
    }
  }

  else
  {
    unsigned short ix0 = *ixarr->get(0);
    unsigned short ix1 = *ixarr->get(1);
    unsigned short ix2 = *ixarr->get(2);
    if ( ix0 >= vertex_array_->getNum() ||
      ix1 >= vertex_array_->getNum() ||
      ix2 >= vertex_array_->getNum() ) {
      ulSetError(UL_WARNING, "ssgLoadBGL: Index out of bounds. " \
        "(%d,%d,%d / %d)", ix0, ix1, ix2, vertex_array_->getNum());
      return;
    }

    // Ensure counter-clockwise ordering
    sgMakeNormal(cross,
      vertex_array_->get(ix0),
      vertex_array_->get(ix1),
      vertex_array_->get(ix2));
    bool flip = (sgScalarProductVec3(cross, s_norm) < 0.0);

    curr_index_->add(ix0);
    for(int i = 1; i < numverts; i++)
    {
      ix1 = *ixarr->get( flip ? numverts-i : i);

      if ( ix1 >= vertex_array_->getNum() ) {
        ulSetError(UL_WARNING, "ssgLoadBGL: Index out of bounds. (%d/%d)",
          ix1, vertex_array_->getNum());
        continue;
      }

      curr_index_->add(ix1);
    }

  }
}

//===========================================================================

static bool readTexIndices(FILE *fp, int numverts, const sgVec3 s_norm, bool flip_y)
{
  if(numverts <= 0)
    return false;

  ssgIndexArray *curr_index_ = new ssgIndexArray();

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
  for(int v = 0; v < numverts; v++)
  {
    unsigned short ix;
    short tx_int, ty_int;

    ix     = ulEndianReadLittle16(fp);
    tx_int = ulEndianReadLittle16(fp);
    ty_int = ulEndianReadLittle16(fp);

    if (flip_y) {
      ty_int = 255 - ty_int;
    }

//JM
    int tex_idx;
    if (tmp_vtx[ix].index == -1) { // add vertex
      sgVec2 dummy_pt;
      tex_idx = vertex_array_->getNum();
      vertex_array_->add(tmp_vtx[ix].point);
      normal_array_->add(tmp_vtx[ix].norm);
      tex_coords_->add(dummy_pt);
      tmp_vtx[ix].index = tex_idx;
    }
    else {
      tex_idx = tmp_vtx[ix].index;
    }

    sgVec2 tc;
    sgSetVec2(tc, tx_int/255.0f, ty_int/255.0f);

    sgVec2 curr_tc;

    if ( tex_idx >= 0 && tex_idx < tex_coords_->getNum() ) {
      sgCopyVec2(curr_tc, tex_coords_->get(tex_idx));
    } else {
      ulSetError( UL_WARNING, "ssgLoadBGL: Texture coord out of range (%d).",
        tex_idx );
      continue;
    }

    double dist = sgDistanceVec2(curr_tc, tc);

    if((curr_tc[0] >= FLT_MAX - 1 && curr_tc[1] >= FLT_MAX - 1))
    {
      sgCopyVec2(tex_coords_->get(tex_idx), tc);
    }

    else if(dist > 0.0001)
    {
      // We have a different texture coordinate for an existing vertex,
      // so we have to copy this vertex and create a new index for it
      // to get the correct texture mapping.
      tex_idx = vertex_array_->getNum();
      vertex_array_->add(tmp_vtx[ix].point);
      normal_array_->add(tmp_vtx[ix].norm);

      tex_coords_->add(tc);

    }

    curr_index_->add(tex_idx);

#ifdef DEBUG
    int check_index = *curr_index_->get(v);
    float *check_tc = tex_coords_->get(check_index);
    DEBUGPRINT( "ix[" << v << "] = " << check_index <<
      " (u=" << check_tc[0] << ", v=" <<
      check_tc[1] << ")" << std::endl);
#endif

  }

  createTriangIndices(curr_index_, numverts, s_norm);

  return true;
}

//===========================================================================

static bool readIndices(FILE* fp, int numverts, const sgVec3 s_norm)
{
  if(numverts <= 0)
    return false;

  ssgIndexArray *curr_index_ = new ssgIndexArray();

  // Read index values
  for(int v = 0; v < numverts; v++)
  {
    unsigned short ix;
    ix = ulEndianReadLittle16(fp);
    if (tmp_vtx[ix].index == -1) { // add vertex
      int last_idx = vertex_array_->getNum();
      vertex_array_->add(tmp_vtx[ix].point);
      normal_array_->add(tmp_vtx[ix].norm);
      tmp_vtx[ix].index = last_idx;
    }
    curr_index_->add(tmp_vtx[ix].index);
    DEBUGPRINT( "ix[" << v << "] = " << *curr_index_->get(v) << std::endl);
    JMPRINT( ios::dec, "ix:" << ix << "idx:" << tmp_vtx[ix].index);
  }

  createTriangIndices(curr_index_, numverts, s_norm);

  return true;
}

//===========================================================================

static void setColor(int color, int pal_id)
{
  if(pal_id == 0x68) 
  {
    curr_col_[0] = fsAltPalette[color].r / 255.0f;
    curr_col_[1] = fsAltPalette[color].g / 255.0f;
    curr_col_[2] = fsAltPalette[color].b / 255.0f;
    curr_col_[3] = 0.2f;
  }
  else
  {
    curr_col_[0] = fsAcPalette[color].r / 255.0f;
    curr_col_[1] = fsAcPalette[color].g / 255.0f;
    curr_col_[2] = fsAcPalette[color].b / 255.0f;
    curr_col_[3] = 1.0f;
  }
}

//===========================================================================

static void setColor(int r, int g, int b, int attr)
{
  switch (attr) {
  
  case 0xf0: setColor( r, 0xf0); break;
  
  case 0xe0:
  case 0xe1:
  case 0xe2:
  case 0xe3:
  case 0xe4:
  case 0xe5:
  case 0xe6:
  case 0xe7:
  case 0xe8:
  case 0xe9:
  case 0xea:
  case 0xeb:
  case 0xec:
  case 0xed:
  case 0xee:
  case 0xef: {
               curr_col_[0] = r / 255.0f;
               curr_col_[1] = g / 255.0f;
               curr_col_[2] = b / 255.0f;
               curr_col_[3] = (attr & 0xf) / 15.0;
               if (attr == 0xef) blending = false; else blending = true;
             } break;
   default: {
               curr_col_[0] = r / 255.0f;
               curr_col_[1] = g / 255.0f;
               curr_col_[2] = b / 255.0f;
               curr_col_[3] = 1.0;
               blending = false;
               JMPRINT( ios::hex, "HINT: Unknown color Attribut: " << (short)attr );
             } break;

  }
}

//===========================================================================

static bool setTexture(char* name, int type)
{
  curr_tex_name_ = name;
  curr_tex_type_ = type;

  return true;
}

//===========================================================================

static ssgSimpleState *createState(bool use_texture)
{
  JMPRINT(ios::dec,"new State: col = " << curr_col_[0] << ", " << curr_col_[1] <<
    ", " << curr_col_[2] << ", " << curr_col_[3]);
  if ( curr_tex_name_  == NULL )
    DEBUGPRINT(", tex = <NULL> " << std::endl);
  else
    DEBUGPRINT(", tex = " << curr_tex_name_ << std::endl);

  ssgSimpleState *state = new ssgSimpleState();

  state->setShininess(DEF_SHININESS);
  state->setShadeModel(GL_SMOOTH);

  state->enable   (GL_LIGHTING);
  state->enable   (GL_CULL_FACE);
  state->disable  (GL_COLOR_MATERIAL);

  if(curr_tex_name_ != NULL && use_texture)
  {
    char tname[128];
    if (haze_ > 0) {
      sprintf(tname,"%s_%d",curr_tex_name_, haze_);
    }
    else {
      sprintf(tname,"%s",curr_tex_name_);
    }
    ssgTexture* tex = current_options ->createTexture(tname, FALSE, FALSE) ;
    state->setTexture( tex ) ;
    state->setOpaque();
    if ( ( (curr_tex_type_ == 0x18)&&(haze_ > 0) ) ||
             ( blending == true ) || 
         ( (curr_tex_type_ == 0x43)&&(tex->hasAlpha() == true) ) ){
      state->enable(GL_BLEND);
      state->enable(GL_ALPHA_TEST);
      state->setAlphaClamp(.2) ;
    } else {
      state->disable(GL_BLEND);
      state->disable(GL_ALPHA_TEST);
    }
    state->setMaterial( GL_AMBIENT, 1.0f, 1.0f, 1.0f, curr_col_[3]);
    state->setMaterial( GL_DIFFUSE, 1.0f, 1.0f, 1.0f, curr_col_[3]);
    state->enable(GL_TEXTURE_2D);
  }
  else
  {
    if(curr_col_[3] < 0.99f)
    {
      state->setTranslucent();
      state->enable(GL_BLEND);
      state->enable(GL_ALPHA_TEST);
    }
    else
    {
      state->setOpaque();
      state->disable(GL_BLEND);
      state->disable(GL_ALPHA_TEST);
    }
    state->setMaterial( GL_AMBIENT, curr_col_ );
    state->setMaterial( GL_DIFFUSE, curr_col_ );
    state->disable(GL_TEXTURE_2D);
  }

  state->setMaterial( GL_SPECULAR, 1.0f, 1.0f, 1.0f, curr_col_[3] );
  state->setMaterial( GL_EMISSION, 0.0f, 0.0f, 0.0f, 1.0f );

  return state;
}

//===========================================================================

static ssgBranch *getCurrGroup() {
  //Find the correct parent for the new group
  if(curr_xfm_)
    return curr_xfm_;
  else if(curr_branch_)
    return curr_branch_;
  else
  {
    return model_;
  }
}

//===========================================================================

static ssgBranch *getMPGroup(int var)
{
  
  switch(var)
  {
  case 0x4c:            // Rudder
    if(!rudder_grp_)
    {
      rudder_grp_ = new ssgBranch();
      rudder_grp_->setName("rudder");
      model_->addKid(rudder_grp_);
    }
    return rudder_grp_;
    break;
    
  case 0x4e:            // Elevator
    if(!elevator_grp_)
    {
      elevator_grp_ = new ssgBranch();
      elevator_grp_->setName("elevator");
      model_->addKid(elevator_grp_);
    }
    return elevator_grp_;
    break;
    
  case 0x6a:            // Ailerons
    if(!ailerons_grp_)
    {
      ailerons_grp_ = new ssgBranch();
      ailerons_grp_->setName("ailerons");
      model_->addKid(ailerons_grp_);
    }
    return ailerons_grp_;
    break;
    
  case 0x6c:            // Flaps
    if(!flaps_grp_)
    {
      flaps_grp_ = new ssgBranch();
      flaps_grp_->setName("flaps");
      model_->addKid(flaps_grp_);
    }
    return flaps_grp_;
    break;
    
  case 0x6e:            // Gear
    if(!gear_grp_)
    {
      gear_grp_ = new ssgBranch();
      gear_grp_->setName("gear");
      model_->addKid(gear_grp_);
    }
    return gear_grp_;
    break;
    
  case 0x7c:            // Spoilers
    if(!spoilers_grp_)
    {
      spoilers_grp_ = new ssgBranch();
      spoilers_grp_->setName("spoilers");
      model_->addKid(spoilers_grp_);
    }
    return spoilers_grp_;
    break;
    
  case 0x58:
  case 0x7a:            // Propeller
    if(!prop_grp_)
    {
      prop_grp_ = new ssgBranch();
      prop_grp_->setName("propeller");
      model_->addKid(prop_grp_);
    }
    return prop_grp_;
    break;
    
  default:
    return model_;
  }
  return NULL;
} 

//===========================================================================

static void getMPLimits(int var, float *min, float *max)
{
  switch(var)
  {
  case 0x4c:            // Rudder
    *min = -30.0;
    *max =  30.0;
    break;
    
  case 0x4e:            // Elevator
    *min = -30.0;
    *max =  30.0;
    break;
    
  case 0x6a:            // Ailerons
    *min = -30.0;
    *max =  30.0;
    break;
    
  case 0x6c:            // Flaps
    *min = 0.0;
    *max = 70.0;
    break;
    
  case 0x6e:            // Gear
    *min = 0.0;
    *max = -90.0;
    break;
    
  case 0x7c:            // Spoilers
    *min = 0.0;
    *max = 90.0;
    break;
    
  case 0x58:
  case 0x7a:            // Propeller
    *min = 0.0;
    *max = 360.0;
    break;
  }
} 

//===========================================================================

ssgEntity *ssgLoadBGL(const char *fname, const ssgLoaderOptions *options)
{
  // john ....
  poly_from_line = false;
  //// 
  ssgSetCurrentOptions ( (ssgLoaderOptions*)options ) ;
  current_options = ssgGetCurrentOptions () ;

  ailerons_grp_ = NULL;
  elevator_grp_ = NULL;
  rudder_grp_ = NULL;
  gear_grp_ = NULL;
  spoilers_grp_ = NULL;
  flaps_grp_ = NULL;
  prop_grp_ = NULL;
  
  char filename [ 1024 ] ;
  current_options -> makeModelPath ( filename, fname ) ;
  
  FILE *fp = fopen(filename, "rb");
  if(!fp) 
  {
    ulSetError( UL_WARNING, "ssgLoadBGL: Couldn't open MDL file '%s'!", 
      filename );
    return NULL;
  }
  
  // Find beginning of BGL Code segment
  if(feof(fp))
  {
    ulSetError( UL_WARNING, "ssgLoadBGL: No BGL Code found in file '%s'!",
      filename );
    return NULL;
  }
  
  // Initialize object graph
  model_ = new ssgBranch();
  char* model_name = new char[128];
  char *ptr = (char*)&fname[strlen(fname) - 1];
  while(ptr != &fname[0] && *ptr != '/') ptr--;
  if(*ptr == '/') ptr++;
  strcpy(model_name, ptr);
  ptr = &model_name[strlen(model_name)];
  while(*ptr != '.' && ptr != &model_name[0]) ptr--; 
  *ptr = '\0';
  model_->setName(model_name);
  
  // Create group nodes for textured and non-textured objects
  vertex_array_ = new ssgVertexArray();
  normal_array_ = new ssgNormalArray();

  tex_coords_ = new ssgTexCoordArray();

  curr_branch_=0;
  stack_depth_ = 0;
  sgMakeIdentMat4(curr_matrix_);

  // Parse opcodes
  bool done = false;
  bool for_scenery_center = true;
  long band_addr;
  long object_position_lat;
  long object_position_lon;
  short object_data;

  fseek(fp, 0x3a, SEEK_SET);
  fread(&object_data, 2, 1, fp);
  PRINT_JOHN2("Ok, now object data is in %x\n", object_data);
  fseek(fp, object_data, SEEK_SET);
  PRINT_JOHN2("Ok, now I am in addr %x\n", ftell(fp));
  fseek(fp, 5, SEEK_CUR);
  PRINT_JOHN("Ok, now in offset\n");
  band_addr = ulEndianReadLittle32(fp);  
  fseek(fp, band_addr-9, SEEK_CUR);
  PRINT_JOHN2("Now I should be in object header, addr is %x\n", ftell(fp));

// john ... we now consider BGL "object header opcode"
  while(!feof(fp)) {
    char header_opcode;
    char image_power;
    fread (&header_opcode, 1, 1, fp);
    PRINT_JOHN2("Header Opcode is %x\n", header_opcode);
    JMPRINT( ios::hex, ftell(fp)-2 << ":  " << (short)header_opcode << "  " << opcodes[header_opcode].name );
    if (header_opcode == 0)
      break;
    object_position_lat = ulEndianReadLittle32(fp);
    object_position_lon = ulEndianReadLittle32(fp);
    if (for_scenery_center == true) {
      scenery_center_lat = object_position_lat;
      scenery_center_lon = object_position_lon;
      JMPRINT(ios::hex, "Reference Lat:    " <<  object_position_lat << "; Reference Lon:" << object_position_lon);
      cout.precision(12);
      cout << ios::fixed << "Reference Lat:    " <<  object_position_lat*90.0/10001750.0 << "; Reference Lon:" << object_position_lon*90.0/(256.0* 4194304.0);
      for_scenery_center = false;
    }
    JMPRINT(ios::hex, ftell(fp)-2 << "obj lat:" << object_position_lat << "  obj lon" << object_position_lon);
    fread (&image_power, 1, 1, fp);
    object_end_offset = ulEndianReadLittle16(fp);
    object_end = object_end_offset + ftell(fp) - 12;
    PRINT_JOHN2("Now I am in %x\n", ftell(fp));
    PRINT_JOHN3("Object end offset, end is %x, %x\n", object_end_offset, object_end);
    done = false;

    while(!feof(fp) && !done && (ftell(fp) < object_end) ) 
    {
      unsigned int   skip_offset = 0;
      unsigned short opcode;
      PRINT_JOHN("Still parsing bgl\n");
      fread(&opcode, 2, 1, fp);
      JMPRINT(ios::hex, ftell(fp)-2 << ":    " << opcode << "  " << (opcode<=255?opcodes[opcode].name:"error: opcode out of range"));
      
      PRINT_JOHN2("Here is a %x\n", opcode);

      DEBUGPRINT( "opcode = " << std::hex << opcode << std::dec << std::endl );

      switch(opcode)
      {
      case 0x23:        // BGL_CALL
        {
          short offset;
          offset = ulEndianReadLittle16(fp);
          long addr = ftell(fp);
          DEBUGPRINT( "BGL_CALL(" << offset << ")\n" );
          ssgBranch* grp = getCurrGroup();
          push_stack((long int)grp);
          push_stack(addr);
          long dst = addr + offset - 4;
          fseek(fp, dst, SEEK_SET);
        }
        break;

      case 0x8a:        // BGL_CALL32
        {
          int offset;
          offset = ulEndianReadLittle32(fp);
          long addr = ftell(fp);
          DEBUGPRINT( "BGL_CALL32(" << offset << ")\n" );
          ssgBranch* grp = getCurrGroup();
          push_stack((long int)grp);
          push_stack(addr);
          long dst = addr + offset - 6;
          fseek(fp, dst, SEEK_SET);
        }
        break;
      
      case 0x0d:        // BGL_JUMP
        {
          short offset;
          offset = ulEndianReadLittle16(fp);
          long addr = ftell(fp);
          long dst = addr + offset - 4;
          fseek(fp, dst, SEEK_SET);
          DEBUGPRINT( "BGL_JUMP(" << offset << ")\n" );
        }
        break;
      
      case 0x88:        // BGL_JUMP32
        {
          int offset;
          offset = ulEndianReadLittle32(fp);
          long addr = ftell(fp);
          DEBUGPRINT( "BGL_JUMP32(" << offset << ")\n" );
          long dst = addr + offset - 6;
          fseek(fp, dst, SEEK_SET);
        }
        break;
      
      case 0x8e:        // BGL_VFILE_MARKER
        {
          short offset;
          offset = ulEndianReadLittle16(fp);
          DEBUGPRINT( "vars: " << offset << std::endl);
          break;
        }

    case 0x39:  // BGL_IFMSK
      {
        short offset, var, mask;
        int val;
        offset = ulEndianReadLittle16(fp);
        var    = ulEndianReadLittle16(fp);
        mask   = ulEndianReadLittle16(fp);

        JMPRINT(ios::dec, "IFMSK: Offset" << offset << ", var: 0x" << std::hex << var <<
          ", mask:" << mask << "\n" );
        int stat = getVariableValue(var, &val);
        if ( (stat == 0) && ((val & mask) == 0) ) { // value out of range: jump
          long addr = ftell(fp);
          long dst = addr + offset - 8;
          fseek(fp, dst, SEEK_SET);
        }
      }
      break;

    case 0x1e:  // BGL_HAZE
      {
        haze_ = ulEndianReadLittle16(fp);
        JMPRINT(ios::hex, "Haze:" << haze_);
      }
      break;

    case 0x21:  // BGL_IFIN3
      {
        short offset, lo, hi;
        int val;
        int stat, s = false;
        unsigned short var;
        offset = ulEndianReadLittle16(fp);
        JMPRINT(ios::dec, "Jump on 3 vars: Offset" << offset);
        long addr = ftell(fp);
        for (int i=0; i<3; i++) {
          var    = ulEndianReadLittle16(fp);
          lo     = ulEndianReadLittle16(fp);
          hi     = ulEndianReadLittle16(fp);
          stat   = getVariableValue(var, &val);
          JMPRINT(ios::dec, "var: 0x" << std::hex << var <<", lo:" << std::dec << lo << ", hi:" << hi );
          if ( (stat == 0) && ((val > hi) || (val < lo)) ) {
            s = true;
            }
          }
        if ( s == true ) { // value out of range: jump
          long dst = addr + offset - 4;
          fseek(fp, dst, SEEK_SET);
        }
      }
      break;

    case 0x24:  // BGL_IFIN1
      {
        short offset, lo, hi;
        int val;
        unsigned short var;
        offset = ulEndianReadLittle16(fp);
        var    = ulEndianReadLittle16(fp);
        lo     = ulEndianReadLittle16(fp);
        hi     = ulEndianReadLittle16(fp);
        JMPRINT(ios::dec, "Jump on var: Offset" << offset << ", var: 0x" << std::hex << var <<
          ", lo:" << std::dec << lo << ", hi:" << hi << ")\n" );
        int stat = getVariableValue(var, &val);
        if ( (stat == 0) && ((val > hi) || (val < lo)) ) { // value out of range: jump
          long addr = ftell(fp);
          long dst = addr + offset - 10;
          fseek(fp, dst, SEEK_SET);
        }
      }
      break;
    
    case 0x26:  // BGL_SETWRD
      {
        int val;
        unsigned short var;
        var    = ulEndianReadLittle16(fp);
        val    = ulEndianReadLittle16(fp);
        JMPRINT(ios::dec, "var: 0x" << std::hex << var <<", val:" << std::dec << val);
      }
      break;

    case 0x46:  // BGL_POINT_VICALL
      {
        short offset, var_rot_x, var_rot_y, var_rot_z;
        unsigned short int_rot_x, int_rot_y, int_rot_z;
        offset = ulEndianReadLittle16(fp);
        sgVec3 ctr;
        readPoint(fp, ctr);
        
        int_rot_x = ulEndianReadLittle16(fp);
        var_rot_x = ulEndianReadLittle16(fp);
        int_rot_z = ulEndianReadLittle16(fp);
        var_rot_z = ulEndianReadLittle16(fp);
        int_rot_y = ulEndianReadLittle16(fp);
        var_rot_y = ulEndianReadLittle16(fp);


        float rx =  360.0f*(float)int_rot_x/0xffff;
        float ry =  360.0f*(float)int_rot_y/0xffff;
        float rz =  360.0f*(float)int_rot_z/0xffff;

        // We build a rotation matrix by adding all constant
        // rotations (int_rot_*) to current_matrix_. As soon as we reach
        // the actual variable rotation, we multiply
        // the axis of the variable rotation with our current matrix.
        // This will be the axis of rotation in the original coordinate
        // system. This can now be inserted into a GngLinearControl
        // transform.
//        if(var_rot_x > 0 || var_rot_y > 0 || var_rot_z > 0)
        ssgAxisTransform* tmp = NULL;
        if(var_rot_x != 0 || var_rot_y != 0 || var_rot_z != 0)
        {
          if(curr_xfm_)
            tmp = curr_xfm_;
          curr_xfm_ = new ssgAxisTransform();
          curr_xfm_->setCenter(curr_rot_pt_);

          int var = 0;
//          if(var_rot_x > 0)
          if(var_rot_x != 0)
            var = var_rot_x;
//          else if(var_rot_y > 0)
          else if(var_rot_y != 0)
            var = var_rot_y;
//          else if(var_rot_z > 0)
          else if(var_rot_z != 0)
            var = var_rot_z;

          float min_limit, max_limit;
          getMPLimits(var, & min_limit, & max_limit);

          sgVec3 axis = { (float)var_rot_y, (float)var_rot_z,
            (float)var_rot_x };
          sgNormaliseVec3( axis ) ;
          sgXformVec3( axis, curr_matrix_ ) ;
          sgNegateVec3(axis);
          curr_xfm_->setAxis(axis);
          curr_xfm_->setRotationLimits(min_limit, max_limit);

          char name[256];
          sprintf(name, "ssgAxisRotation(%x)", var);
          curr_xfm_->setName(name);
          if(tmp)
            tmp->addKid(curr_xfm_);
          else
          {
            ssgBranch* grp = getMPGroup(var);
            grp->addKid(curr_xfm_);
          }
        }

        // Build up the constant rotations
        sgCoord *rottrans = new sgCoord;
        sgSetCoord  ( rottrans, ctr[0], ctr[1], ctr[2], -ry+360, -rz, -rx ) ;
        long addr = ftell(fp);
        long dst = addr + offset - 22;
        fseek(fp, dst, SEEK_SET);
        // push current Branch
        ssgBranch* grp;
        if (tmp !=NULL) {
          grp = tmp;
        }
        else {
          grp = getCurrGroup();
        }
        curr_branch_ = new ssgTransform();
        curr_branch_->setName("RotoCallInd");
        curr_branch_->setTransform(rottrans);

        grp->addKid(curr_branch_);
        push_stack((long int)grp);
        push_stack(addr);

        break;
      }

    case 0x5f:  // BGL_IFSIZEV
      {
        short offset;
        unsigned short real_size, pixels_ref;
        offset     = ulEndianReadLittle16(fp);
        real_size  = ulEndianReadLittle16(fp);
        pixels_ref = ulEndianReadLittle16(fp);
        DEBUGPRINT("BGL_IFSIZEV: jmp = " << offset << ", sz = " <<
          real_size << ", px = " << pixels_ref << std::endl);
       break;
      }

    case 0x3b:  // BGL_VINSTANCE
      {
        short offset, var;
        offset = ulEndianReadLittle16(fp);
        var    = ulEndianReadLittle16(fp);
        
	float p = 360.0f * (float)ulEndianReadLittle32(fp) / 0xffffffff;
        float r = 360.0f * (float)ulEndianReadLittle32(fp) / 0xffffffff;
        float h = 360.0f * (float)ulEndianReadLittle32(fp) / 0xffffffff;
        sgMat4 rot_mat;
//        sgMakeRotMat4(rot_mat, -h+360, r, p);
        sgMakeRotMat4(rot_mat, -h+360, -r, -p);
        sgPostMultMat4(curr_matrix_, rot_mat);
        DEBUGPRINT( "BGL_VINSTANCE(" << offset << ", h=" << h << ", p=" <<
          p << ", r=" << r << ")\n");
       
        long addr = ftell(fp);
        long dst = addr + offset - 18;
        // push current branch
        ssgBranch* grp = getCurrGroup();
        push_stack((long int)grp);
        push_stack(addr);

        fseek(fp, dst, SEEK_SET);
      }
      break;

    case 0x0:   // EOF
      {
        PRINT_JOHN("I got a 0x00\n");
      }
      break;
    case 0x34:  // BGL_SUPER_SCALE
      {
        unsigned short offset, var1, var2, var3;
        offset = ulEndianReadLittle16(fp);
        var1   = ulEndianReadLittle16(fp);
        var2   = ulEndianReadLittle16(fp);
        var3   = ulEndianReadLittle16(fp);

        ref_scale = 65536.0/double(1<<var3);
        
        JMPRINT( ios::dec, "SUPER_SCALE: Offset:" << offset << " var1:" << var1 << " var2:" << var2 << " var3:" << var3  );
        JMPRINT( ios::dec, "-> Scale:" << ref_scale );
      }
      break;
    
    case 0x22:  // BGL return
      {
//        curr_xfm_    = NULL;
        PRINT_JOHN("I got a 0x22\n");
        sgMakeIdentMat4( curr_matrix_ );
        sgZeroVec3( curr_rot_pt_ );
        DEBUGPRINT( "BGL return\n\n");
        if(stack_depth_ == 0) {
          //done = true;
          PRINT_JOHN("It's done\n");
        }
        else
        {
          long addr = pop_stack();
          curr_branch_ = (ssgTransform *)pop_stack();
          fseek(fp, addr, SEEK_SET);
        }
        PRINT_JOHN2("Now addr is %x\n", ftell(fp));
      }
      break;

    case 0x1a:  // RESLIST (point list with no normals)
      {
        newPart();
        has_normals_ = false;

        int start_idx            = ulEndianReadLittle16(fp);
        unsigned short numpoints = ulEndianReadLittle16(fp);

        //john ...
        if ( poly_from_line ) {
          poly_from_line_numverts = numpoints;
        }

        DEBUGPRINT( "New group (unlit): start_idx = " << start_idx
          << ", num vertices = " << numpoints << std::endl);

        sgVec3 null_normal;
        sgZeroVec3( null_normal );
        JMPRINT( ios::dec, "RESLIST: First:" << start_idx << " Nr:" << numpoints);
        for(int i = 0; i < numpoints; i++)
        {
          sgVec3 p;
          readPoint(fp, p);
          JMPRINT( ios::dec, "RESLIST: 0:" << p[0] << " 1:" << p[1] << " 2:" << p[2]);
          tmp_vtx[start_idx+i].point=p;
          tmp_vtx[start_idx+i].norm=null_normal;
          tmp_vtx[start_idx+i].index=-1; // mark as dirty
        }
      }
      break;

    case 0x29:  // GORAUD RESLIST (point list with normals)
      {
        newPart();
        has_normals_ = true;

        int start_idx            = ulEndianReadLittle16(fp);
        unsigned short numpoints = ulEndianReadLittle16(fp);

        DEBUGPRINT( "New group (goraud): start_idx = " << start_idx
          << ", num vertices = " << numpoints << std::endl);
        
        JMPRINT( ios::dec, "GORAUD RESLIST: First:" << start_idx << " Nr:" << numpoints);
        for(int i = 0; i < numpoints; i++) 
        {
          sgVec3 p;
          readPoint(fp, p);
          JMPRINT( ios::dec, "GORAUD RESLIST: 0:" << p[0] << " 1:" << p[1] << " 2:" << p[2]);
          sgVec3 v;
          readVector(fp, v);
          tmp_vtx[start_idx+i].point=p;
          tmp_vtx[start_idx+i].norm=v;
          tmp_vtx[start_idx+i].index=-1; // mark as dirty
        }
      }
      break;
    case 0x06:  // STRRES: Start line definition
      {
        sgVec3 null_normal;
        sgZeroVec3( null_normal );
        sgVec3 p;
        readPoint(fp, p);
        tmp_vtx[0].point=p;
        tmp_vtx[0].norm=null_normal;
        tmp_vtx[0].index=-1; // mark as dirty
       }
    case 0x07:  // STRRES: Ends line definition
      {
        sgVec3 null_normal;
        sgZeroVec3( null_normal );
        sgVec3 p;
        readPoint(fp, p);
        tmp_vtx[1].point=p;
        tmp_vtx[1].norm=null_normal;
        tmp_vtx[1].index=-1; // mark as dirty
        if (tmp_vtx[0].index == -1) { // add vertex
          int ix = vertex_array_->getNum();
          curr_index_ = new ssgIndexArray();
          vertex_array_->add(tmp_vtx[0].point);
          normal_array_->add(tmp_vtx[0].norm);
          vertex_array_->add(tmp_vtx[1].point);
          normal_array_->add(tmp_vtx[1].norm);
          curr_index_->add(ix);
          curr_index_->add(ix+1);
          sgVec3 v;
          createTriangIndices(curr_index_, 2, v);
          curr_part_ = new ssgVtxArray( GL_LINES,
            vertex_array_,
            normal_array_,
            NULL,
            NULL,
            curr_index_ );
          curr_part_->setState( createState(false) );
          ssgBranch* grp = getCurrGroup();
          grp->addKid( current_options -> createLeaf(curr_part_, NULL) );
        }
      }
    case 0x0f:  // STRRES: Start poly line definition
      {
        unsigned short idx = ulEndianReadLittle16(fp);
        DEBUGPRINT( "Start line: idx = " << idx << std::endl);

        curr_index_ = new ssgIndexArray();
        
	if (tmp_vtx[idx].index == -1) { // add vertex
          int ix = vertex_array_->getNum();
          vertex_array_->add(tmp_vtx[idx].point);
          normal_array_->add(tmp_vtx[idx].norm);
          tmp_vtx[idx].index = ix;
        }
        curr_index_->add(tmp_vtx[idx].index);

        // john .....
        if ( !poly_from_line ) {  // don't add this kid right now

          curr_part_ = new ssgVtxArray( GL_LINES,
            vertex_array_,
            normal_array_,
            NULL,
            NULL,
            curr_index_ );
          
          if ( poly_from_line ) {
            curr_part_->setState( createState(false) );
          }
          else {
            curr_part_->setState( createState(true) );
          }
#ifdef EXPERIMENTAL_CULL_FACE_CODE
          curr_part_->setCullFace ( curr_cull_face_ ) ;
#endif

          ssgBranch *grp = getCurrGroup();
          grp->addKid(curr_part_);

        }
      }
      break;

    case 0x10:  // CNTRES: Continue poly line definition
      {
        unsigned short idx = ulEndianReadLittle16(fp);
        DEBUGPRINT( "Cont. line: idx = " << idx << std::endl);
        if (tmp_vtx[idx].index == -1) { // add vertex
          int ix = vertex_array_->getNum();
          vertex_array_->add(tmp_vtx[idx].point);
          normal_array_->add(tmp_vtx[idx].norm);
          tmp_vtx[idx].index = ix;
        }
        curr_index_->add(tmp_vtx[idx].index);
      }
      break;

    case 0x20:
    case 0x7a:  // Goraud shaded Texture-mapped ABCD Facet
      {
        curr_index_ = new ssgIndexArray();

	unsigned short numverts = ulEndianReadLittle16(fp);
        DEBUGPRINT( "New part: (goraud/texture), num indices = " <<
          numverts << std::endl);

        // Normal vector
        sgVec3 v;
        readVector(fp, v);

        // Dot product reference
#ifdef EXPERIMENTAL_CULL_FACE_CODE
        curr_cull_face_ = ulEndianReadLittle32(fp) >= 0;
#else
        ulEndianReadLittle32(fp);
#endif

        // Read vertex inidices and texture coordinates
        bool flip_y = FALSE;
        if(curr_tex_name_!=NULL)
        {
          char *texture_extension = curr_tex_name_ + strlen(curr_tex_name_) - 3;
          flip_y = ulStrEqual( texture_extension, "BMP" ) != 0 ;
        }

        readTexIndices(fp, numverts, v, flip_y);

        if(!has_normals_)
        {
          while (normal_array_->getNum() < vertex_array_->getNum())
            normal_array_->add(v);
          recalcNormals();
        }

        curr_part_ = new ssgVtxArray( GL_TRIANGLE_FAN, 
          vertex_array_,
          normal_array_,
          tex_coords_,
          NULL,
          curr_index_ );
        curr_part_->setState( createState(true) );

#ifdef EXPERIMENTAL_CULL_FACE_CODE
        curr_part_->setCullFace ( curr_cull_face_ ) ;
#endif

        ssgBranch* grp = getCurrGroup();
        grp->addKid( current_options -> createLeaf(curr_part_, NULL) );
      }
      break;

    case 0x60:  // BGL_FACE_TMAP
      {
        curr_index_ = new ssgIndexArray();

        unsigned short numverts = ulEndianReadLittle16(fp);
        DEBUGPRINT( "New part: (goraud/texture), num indices = " <<
          numverts << std::endl);

        // Point in polygon
        sgVec3 p;
        readPoint(fp, p);

        // Normal vector
        sgVec3 v;
        readVector(fp, v);

        // Read vertex inidices and texture coordinates
        bool flip_y = FALSE;
        if(curr_tex_name_!=NULL)
        { char *texture_extension =
        curr_tex_name_ + strlen(curr_tex_name_) - 3;
        flip_y = ulStrEqual( texture_extension, "BMP" ) != 0 ;
        }

        readTexIndices(fp, numverts, v, flip_y);

        if(!has_normals_)
        {
          while (normal_array_->getNum() < vertex_array_->getNum())
            normal_array_->add(v);
          recalcNormals();
        }

        curr_part_  = new ssgVtxArray( GL_TRIANGLE_FAN,
          vertex_array_,
          normal_array_,
          tex_coords_,
          NULL,
          curr_index_ );
        curr_part_->setState( createState(true) );
#ifdef EXPERIMENTAL_CULL_FACE_CODE
        curr_part_->setCullFace ( curr_cull_face_ ) ;
#endif


        ssgBranch* grp = getCurrGroup();
        grp->addKid( current_options -> createLeaf(curr_part_, NULL) );
      }
      break;

    case 0x1d:  // BGL_FACE
      {
        curr_index_ = new ssgIndexArray();

        unsigned short numverts = ulEndianReadLittle16(fp);
        DEBUGPRINT( "BGL_FACE: num indices = " << numverts << std::endl);

        sgVec3 p;
        readPoint(fp, p);
        // Surface normal
        sgVec3 v;
        readVector(fp, v);

        // Read vertex indices
        readIndices(fp, numverts, v);

        if(!has_normals_)
        {
          while (normal_array_->getNum() < vertex_array_->getNum())
            normal_array_->add(v);
          recalcNormals();
        }

        curr_part_ = new ssgVtxArray(GL_TRIANGLE_FAN,
          vertex_array_,
          normal_array_,
          NULL,
          NULL,
          curr_index_);
        curr_part_->setState( createState(false) );
#ifdef EXPERIMENTAL_CULL_FACE_CODE
        curr_part_->setCullFace ( curr_cull_face_ ) ;
#endif

        ssgBranch* grp = getCurrGroup();
        grp->addKid( current_options -> createLeaf(curr_part_, NULL) );
      }
      break;

    case 0x3e:  // FACETN (no texture)
    case 0x2a:  // Goraud shaded ABCD Facet
      {
        curr_index_ = new ssgIndexArray();
        unsigned short numverts = ulEndianReadLittle16(fp);
        DEBUGPRINT( "BGL_FACETN: num indices = " << numverts << std::endl);

        // Surface normal
        sgVec3 v;
        readVector(fp, v);

        // dot-ref
#ifdef EXPERIMENTAL_CULL_FACE_CODE
        curr_cull_face_ = ulEndianReadLittle32(fp) >= 0;
#else
        ulEndianReadLittle32(fp);
#endif
        // Read vertex indices
        readIndices(fp, numverts, v);

        if(!has_normals_)
        {
          while (normal_array_->getNum() < vertex_array_->getNum())
            normal_array_->add(v);
          recalcNormals();
        }

        curr_part_ = new ssgVtxArray(GL_TRIANGLE_FAN,
          vertex_array_,
          normal_array_,
          NULL,
          NULL,
          curr_index_);
        curr_part_->setState( createState(false) );
#ifdef EXPERIMENTAL_CULL_FACE_CODE
        curr_part_->setCullFace ( curr_cull_face_ ) ;
#endif

        ssgBranch* grp = getCurrGroup();
        grp->addKid( current_options -> createLeaf(curr_part_, NULL) );
      }
      break;

    case 0x18:  // Set texture
      {
        unsigned short id, dx, scale, dy;
        id    = ulEndianReadLittle16(fp);
        dx    = ulEndianReadLittle16(fp);
        scale = ulEndianReadLittle16(fp);
        dy    = ulEndianReadLittle16(fp);
        char tex_name[14];
        fread(tex_name, 1, 14, fp);
        
        int j = 0;
        for(int i = 0; i < 14; i++)
        {
          if(!isspace(tex_name[i]))
            tex_filename[j++] = tolower(tex_name[i]);
        }
        tex_filename[j] = '\0';
        JMPRINT( ios::dec, "Set texture: name = " << tex_filename << ", id = " << id
          << ", dx = " << dx << ", dy = " << dy << ", scale = " << scale);
        setTexture(tex_filename, 0x18);
      }
      break;

    case 0x43:  // TEXTURE2
      {
        unsigned short length, idx;
        unsigned char  flags, chksum;
        unsigned int   color;

        length = ulEndianReadLittle16(fp);
        idx    = ulEndianReadLittle16(fp);
        fread(&flags, 1, 1, fp);
        fread(&chksum, 1, 1, fp);
        color = ulEndianReadLittle32(fp);
        if(chksum != 0)
        {
          DEBUGPRINT( "warning: TEXTURE2 Checksum != 0\n");
        }

        char c;
        int i = 0;
        while((c = fgetc(fp)) != 0)
        {
          if(!isspace(c))
            tex_filename[i++] = tolower(c);
        }
        tex_filename[i] = '\0';

        // Padding byte
        if((strlen(tex_filename) + 1) % 2)
          c = fgetc(fp);

        JMPRINT( ios::hex, "Set texture2: name = " << tex_filename << ", length = 0x" << length
          << ", idx = 0x" << idx << ", flags = 0x" << (short)flags << ", color = 0x" << color);

        setTexture(tex_filename, 0x43);
        break;
      }

    case 0x50:  // GCOLOR (Goraud shaded color)
    case 0x51:  // LCOLOR (Line color)
    case 0x52:  // SCOLOR (Light source shaded surface color)
      {
        unsigned char color, param;
        fread(&color, 1, 1, fp);
        fread(&param, 1, 1, fp);
        JMPRINT( ios::hex, "Set color = " << (int)color << " Para = " << (int)param );
        setColor((int)color, (int)param);
      }
      break;

    case 0x2D:  // BGL_SCOLOR24
    case 0x2E:  // BGL_LCOLOR24
      {
        unsigned char col[4];
        fread(col, 1, 4, fp);
        JMPRINT( ios::hex, "Set color24 = " << (int)col[0] << ", " << (int)col[2] <<
          ", " << (int)col[3] << ", " << (int)col[1] );
        setColor(col[0], col[2], col[3], col[1]);
        break;
      }

    case 0x03:
      {
        unsigned short number_cases;
        fread(&number_cases, 2, 1, fp);
        skip_offset = 6 + 2 * number_cases;
      }
      break;

    case 0x05:    // SURFACE
      {
         poly_from_line = true;
      }
      break;

    case 0x08:    // CLOSE
      {
        sgVec3 v;
        createTriangIndices(curr_index_, poly_from_line_numverts, v);
        if(!has_normals_)
        {
          while (normal_array_->getNum() < vertex_array_->getNum())
            normal_array_->add(v);
          recalcNormals();
        }
        curr_part_ = new ssgVtxArray( GL_LINES,
          vertex_array_,
          normal_array_,
          NULL,
          NULL,
          curr_index_ );
        
        if ( poly_from_line ) {
          curr_part_->setState( createState(true) );
        }
        else {
          curr_part_->setState( createState(true) );
        }
#ifdef EXPERIMENTAL_CULL_FACE_CODE
        curr_part_->setCullFace ( curr_cull_face_ ) ;
#endif

        ssgBranch* grp = getCurrGroup();
        grp->addKid( current_options -> createLeaf(curr_part_, NULL) );

        poly_from_line = false;
      }
      break;

      case 0x81:  // OBSOLETE
        {
          ulEndianReadLittle16(fp);
        }
        break;

      case 0x32:   // ADDOBJ  john: do nothing more than BGL_CALL right now
        {
          PRINT_JOHN("I got a 0x32\n");
          short offset;
          offset = ulEndianReadLittle16(fp);
          long addr = ftell(fp);
          DEBUGPRINT( "BGL_CALL(" << offset << ")\n" );
          
          // push current Branch
          ssgBranch* grp = getCurrGroup();
          push_stack((long int)grp);
          push_stack(addr);
          long dst = addr + offset - 4;
          fseek(fp, dst, SEEK_SET);
        }
        break;
      case 0x76:  // BGL_BGL JM: do nothing
        {
          PRINT_JOHN("I got a 0x76\n");
//          curr_xfm_    = NULL;
          sgMakeIdentMat4( curr_matrix_ );
          sgZeroVec3( curr_rot_pt_ );
        }
        break;

      case 0x3f: // BGL_SHADOW_CALL JM: do nothing
        {
          short offset;
          offset = ulEndianReadLittle16(fp);
          // do nothing jm
        }
        break;

      case 0x3c:
        {
          PRINT_JOHN("I got a 0x3c\n");
          JMPRINT(ios::dec, "Refpoint3c");

          sgVec3 v;
          // Create group nodes for textured and non-textured objects
          if (curr_part_!=NULL)
            curr_part_->recalcBSphere();
          vertex_array_ = new ssgVertexArray();
          normal_array_ = new ssgNormalArray();
          tex_coords_ = new ssgTexCoordArray();

          ulEndianReadLittle16(fp);           // byte offset
          ulEndianReadLittle16(fp);           // visibility related
          ulEndianReadLittle16(fp);           // invisibility cone
          ulEndianReadLittle16(fp);           // ?
          readRefPointTranslation(fp,v);
          v[2] = (double) 0.0;                // set altitude to zero (for now)
          ref_scale = 1.0;                    // scaling alway 1.0
          JMPRINT( ios::dec, " Scale:" << ref_scale << " Delta Latitude:" << v[0] << "m; Delta Longitude:" << v[1] << "m; Delta Altitude:" << v[2] << "m");

          // push current Branch
          curr_branch_ = new ssgTransform();
          curr_branch_->setName("Refpoint3c");
          model_->addKid(curr_branch_);
          curr_xfm_    = NULL;
          if (curr_branch_ !=0)
             curr_branch_->setTransform(v);
        }
        break;

      case 0x77:
        {
          PRINT_JOHN("I got a 0x77\n");
          JMPRINT(ios::dec, "Refpoint77");
          long scale;
          sgVec3 v;

          // Create group nodes for textured and non-textured objects
          if (curr_part_!=NULL)
            curr_part_->recalcBSphere();
          vertex_array_ = new ssgVertexArray();
          normal_array_ = new ssgNormalArray();
          tex_coords_ = new ssgTexCoordArray();

          ulEndianReadLittle16(fp);           // byte offset
          ulEndianReadLittle16(fp);           // visibility related
          ulEndianReadLittle16(fp);           // invisibility cone
          ulEndianReadLittle16(fp);           // ?
          scale = ulEndianReadLittle32(fp);   // scaling
          ref_scale = (double)scale /65536.0;
          readRefPointTranslation(fp,v);
          v[2] = (double) 0.0;                // altitude is always zero
          JMPRINT( ios::dec, " Scale:" << ref_scale << " Delta Latitude:" << v[0] << "m; Delta Longitude:" << v[1] << "m; Delta Altitude:" << v[2] << "m");

          // push current Branch
          curr_branch_ = new ssgTransform();
          curr_branch_->setName("Refpoint & Scale");
          model_->addKid(curr_branch_);
          curr_xfm_    = NULL;
          if (curr_branch_ !=0)
             curr_branch_->setTransform(v);
        }
        break;

      case 0x2f: // BGL_SCALE john:do nothing write now
        {
          PRINT_JOHN("I got a 0x2f\n");
          JMPRINT(ios::dec, "Refpoint2f");
          long scale;
          sgVec3 v;

          // Create group nodes for textured and non-textured objects
          if (curr_part_!=NULL)
            curr_part_->recalcBSphere();
          vertex_array_ = new ssgVertexArray();
          normal_array_ = new ssgNormalArray();
          tex_coords_ = new ssgTexCoordArray();

          ulEndianReadLittle16(fp);           // byte offset
          ulEndianReadLittle16(fp);           // visibility related
          ulEndianReadLittle16(fp);           // invisibility cone
          ulEndianReadLittle16(fp);           // ?
          scale = ulEndianReadLittle32(fp);   // scaling
          ref_scale = (double)scale /65536.0;
          readRefPointTranslation(fp,v);
          v[2] = (double) 0.0;                // set altitude to zero for now
          JMPRINT( ios::dec, " Scale:" << ref_scale << " Delta Latitude:" << v[0] << "m; Delta Longitude:" << v[1] << "m; Delta Altitude:" << v[2] << "m");

          // push current Branch
          curr_branch_ = new ssgTransform();
          curr_branch_->setName("Refpoint2f");
          curr_xfm_    = NULL;
          model_->addKid(curr_branch_);
          if (curr_branch_ !=0)
             curr_branch_->setTransform(v);
        }
        break;

      case 0x33:  // BGL_INSTANCE
        {
          short offset;
          PRINT_JOHN("I got a 0x33\n");
          offset = ulEndianReadLittle16(fp);
          DEBUGPRINT( "BGL_CALL(" << offset << ")\n" );
          long addr = ftell(fp);
          
          unsigned short int_rot_x = ulEndianReadLittle16(fp);
          unsigned short int_rot_z = ulEndianReadLittle16(fp);
          unsigned short int_rot_y = ulEndianReadLittle16(fp);

          float rx =  360.0f*(float)int_rot_x/0xffff;
          float ry =  360.0f*(float)int_rot_y/0xffff;
          float rz =  360.0f*(float)int_rot_z/0xffff;
          JMPRINT( ios::hex, "rx:" << rx << " ry:" << ry << " rz:" << rz );

          // Build up the constant rotations
          sgMat4 rot_mat;
//        sgMakeRotMat4( rot_mat, -ry+360.0, rz, rx );
          sgMakeRotMat4( rot_mat, -ry+360.0, -rz, -rx );
          sgPostMultMat4( curr_matrix_, rot_mat );
          
          // push current Branch
          ssgBranch* grp = getCurrGroup();
          curr_branch_ = new ssgTransform();
          curr_branch_->setName("RotoCall");
          curr_branch_->setTransform(rot_mat);
          grp->addKid(curr_branch_);
          push_stack((long int)grp);
          push_stack(addr+6);  //john: for now I haven't parse the whole command
          long dst = addr + offset - 4;
          fseek(fp, dst, SEEK_SET);
        }
        break;

    case 0x49:  // BGL_BUILDING
      {
      int info, type;
      double stories, size_x, size_y, size_z;
      double ox,oy,oz;
      ssgSimpleState *state = new ssgSimpleState();
      building = new ssgTransform ;

      sgVec3 pos;
      sgVec2 btexcoor;
      info = ulEndianReadLittle16(fp); //info
      ulEndianReadLittle16(fp);        //codes
      oy = (ulEndianReadLittle16(fp) * ref_scale) ;   //offset x
      oz = (ulEndianReadLittle16(fp) * ref_scale) ;   //offset y
      ox = (ulEndianReadLittle16(fp) * ref_scale) ;   //offset z
      stories = ulEndianReadLittle16(fp) * ref_scale; //stories
      size_z = ulEndianReadLittle16(fp) * ref_scale;  //size_x
      size_x = ulEndianReadLittle16(fp) * ref_scale;  //size_z
      size_y = stories * 4;
      ox -= size_x/2;
      oy -= size_z/2;
#ifdef JMDEBUG
      cout.flags(ios::dec);

      cout << " info:" << info;
      cout << " stories:" << stories;
      cout << " size_x:" << size_x;
      cout << " size_z:" << size_z;
#endif
      type = info%8;

      state->setShininess(DEF_SHININESS);
      state->setShadeModel(GL_SMOOTH);
      
      state->disable(GL_BLEND);
      state->disable(GL_ALPHA_TEST);

      state->enable   (GL_LIGHTING);
      state->enable   (GL_CULL_FACE);
      state->disable  (GL_COLOR_MATERIAL);

      state->setMaterial( GL_DIFFUSE, 1.0f, 1.0f, 1.0f, curr_col_[3]);
      state->setMaterial( GL_AMBIENT, 1.0f, 1.0f, 1.0f, curr_col_[3]);
      state->setMaterial( GL_SPECULAR, 1.0f, 1.0f, 1.0f, curr_col_[3] );
      state->setMaterial( GL_EMISSION, 0.0f, 0.0f, 0.0f, 1.0f );
      state->enable     ( GL_TEXTURE_2D ) ;
//     state -> setColourMaterial ( GL_AMBIENT_AND_DIFFUSE ) ;

      switch(type)
        {
          case 0x00: //sandstone skyscraper
            {
            state->setTexture("/usr/local/lib/FlightGear/Textures/sandstone.rgb");
            }
            break;
          case 0x01: //white skyscraper
            {
            state->setTexture("/usr/local/lib/FlightGear/Textures/white.rgb");
            }
            break;
          case 0x02: //black skyscraper
            {
            state->setTexture("/usr/local/lib/FlightGear/Textures/black.rgb");
            }
            break;
          case 0x03: //dark gray skyscraper
            {
            state->setTexture("/usr/local/lib/FlightGear/Textures/darkgray.rgb");
            }
            break;
          case 0x04: //gray skyscraper
            {
            state->setTexture("/usr/local/lib/FlightGear/Textures/gray.rgb");
            }
            break;
          case 0x05: //white horizontal lined skyscraper
            {
            state->setTexture("/usr/local/lib/FlightGear/Textures/whitehorizon.rgb");
            }
            break;
          case 0x06: //sandstone horizontal lined skyscraper
            {
            state->setTexture("/usr/local/lib/FlightGear/Textures/sandstonehorizon.rgb");
            }
            break;
          case 0x07: //gray skyscraper
            {
            state->setTexture("/usr/local/lib/FlightGear/Textures/gray.rgb");
            }
            break;
          default:
            break;
        }

// john temp test
      building_count++;
      PRINT_JOHN2("I am building the %dth building\n", building_count);

      ssgVertexArray   *vertices_walls = new ssgVertexArray(3);
      ssgNormalArray   *normals_walls = new ssgNormalArray(3);
      ssgVertexArray   *vertices_roof = new ssgVertexArray(3);
      ssgNormalArray   *normals_roof = new ssgNormalArray(3);
      ssgVertexArray   *vertices_base = new ssgVertexArray(3);
      ssgNormalArray   *normals_base = new ssgNormalArray(3);

      ssgTexCoordArray *texcoor_walls = new ssgTexCoordArray(10);

      sgSetVec3(pos, 0 + ox, 0 + oy, size_y + oz);   // defining positon
      vertices_walls->add(pos);
      sgSetVec3(pos, 0 + ox, 0 + oy, 0 + oz);
      vertices_walls->add(pos);
      sgSetVec3(pos, size_x + ox, 0 + oy, size_y + oz);
      vertices_walls->add(pos);
      sgSetVec3(pos, size_x + ox, 0 + oy, 0 + oz);
      vertices_walls->add(pos);
      sgSetVec3(pos, size_x + ox, size_z + oy, size_y + oz);
      vertices_walls->add(pos);
      sgSetVec3(pos, size_x + ox, size_z + oy, 0 + oz);
      vertices_walls->add(pos);
      sgSetVec3(pos, 0 + ox, size_z + oy, size_y + oz);
      vertices_walls->add(pos);
      sgSetVec3(pos, 0 + ox, size_z + oy, 0 + oz);
      vertices_walls->add(pos);
      sgSetVec3(pos, 0 + ox, 0 + oy, size_y + oz);
      vertices_walls->add(pos);
      sgSetVec3(pos, 0 + ox, 0 + oy, 0 + oz);
      vertices_walls->add(pos);

      sgSetVec3(pos, 0, -3, 0);                          //defining normal
      normals_walls->add(pos);
      sgSetVec3(pos, 0, -3, 0);
      normals_walls->add(pos);
      sgSetVec3(pos, 0, -3, 0);
      normals_walls->add(pos);
      sgSetVec3(pos, 0, -3, 0);
      normals_walls->add(pos);
      sgSetVec3(pos, 0, -3, 0);
      normals_walls->add(pos);
      sgSetVec3(pos, 0, -3, 0);
      normals_walls->add(pos);
      sgSetVec3(pos, 0, -3, 0);
      normals_walls->add(pos);
      sgSetVec3(pos, 0, -3, 0);
      normals_walls->add(pos);
      sgSetVec3(pos, 0, -3, 0);
      normals_walls->add(pos);
      sgSetVec3(pos, 0, -3, 0);
      normals_walls->add(pos);

      sgSetVec2(btexcoor, 0.1, 0.1);               //defining texture coor
      texcoor_walls->add(btexcoor);
      sgSetVec2(btexcoor, 0.1, 0.9);
      texcoor_walls->add(btexcoor);
      sgSetVec2(btexcoor, 0.9, 0.1);
      texcoor_walls->add(btexcoor);
      sgSetVec2(btexcoor, 0.9, 0.9);
      texcoor_walls->add(btexcoor);
      sgSetVec2(btexcoor, 0.1, 0.1);
      texcoor_walls->add(btexcoor);
      sgSetVec2(btexcoor, 0.1, 0.9);
      texcoor_walls->add(btexcoor);
      sgSetVec2(btexcoor, 0.9, 0.1);
      texcoor_walls->add(btexcoor);
      sgSetVec2(btexcoor, 0.9, 0.9);
      texcoor_walls->add(btexcoor);
      sgSetVec2(btexcoor, 0.1, 0.1);
      texcoor_walls->add(btexcoor);
      sgSetVec2(btexcoor, 0.1, 0.9);
      texcoor_walls->add(btexcoor);


      sgSetVec3(pos, 0 + ox, 0 + oy, size_y + oz);
      vertices_roof->add(pos);
      sgSetVec3(pos, size_x + ox, 0 + oy, size_y + oz);
      vertices_roof->add(pos);
      sgSetVec3(pos, 0 + ox, size_z + oy, size_y + oz);
      vertices_roof->add(pos);
      sgSetVec3(pos, size_x + ox, size_z + oy, size_y + oz);
      vertices_roof->add(pos);

      sgSetVec3(pos, 0, 0, 0);
      normals_roof->add(pos);
      sgSetVec3(pos, 0, 0, 0);
      normals_roof->add(pos);
      sgSetVec3(pos, 0, 0, 0);
      normals_roof->add(pos);
      sgSetVec3(pos, 0, 0, 0);
      normals_roof->add(pos);

      sgSetVec3(pos, 0 + ox, 0 + oy, 0 + oz);
      vertices_base->add(pos);
      sgSetVec3(pos, 0 + ox, size_z + oy, 0 + oz);
      vertices_base->add(pos);
      sgSetVec3(pos, size_x + ox, 0 + oy, 0 + oz);
      vertices_base->add(pos);
      sgSetVec3(pos, size_x + ox, size_z + oy, 0 + oz);
      vertices_base->add(pos);

      sgSetVec3(pos, 0, 0, 0);
      normals_base->add(pos);
      sgSetVec3(pos, 0, 0, 0);
      normals_base->add(pos);
      sgSetVec3(pos, 0, 0, 0);
      normals_base->add(pos);
      sgSetVec3(pos, 0, 0, 0);
      normals_base->add(pos);

      ssgVtxTable *pVtx_walls = new ssgVtxTable(GL_QUAD_STRIP, vertices_walls,
       normals_walls, texcoor_walls, NULL);
      ssgVtxTable *pVtx_roof = new ssgVtxTable(GL_QUAD_STRIP, vertices_roof,
       normals_roof, NULL, NULL);
      ssgVtxTable *pVtx_base = new ssgVtxTable(GL_QUAD_STRIP, vertices_base,
       normals_base, NULL, NULL);

      pVtx_walls->setState(state);
      pVtx_roof ->setState(state);
      pVtx_base ->setState(state);
      building -> addKid (pVtx_walls);
      building -> addKid (pVtx_roof);
      building -> addKid (pVtx_base);

      ssgBranch* grp = getCurrGroup();
      grp    -> addKid (building);
      if (done)
        PRINT_JOHN("Now done\n");
      else
        PRINT_JOHN("Not done yet\n");
      }
      PRINT_JOHN2("Addr now is %x\n", ftell(fp));
      PRINT_JOHN2("object end is %x\n", object_end);
      break;
      
      case 0xa0: {
        short offset = ulEndianReadLittle16(fp);
        long addr = ftell(fp);
        ulEndianReadLittle16(fp); //type
        long dst = addr + offset - 4;
        fseek(fp, dst, SEEK_SET);
        //ignore the rest for now ;) JM
      }
      break;

      case 0x70: { // BGL_AREA_SENSE
        short offset = ulEndianReadLittle16(fp);
        long addr = ftell(fp);
        ulEndianReadLittle16(fp); //type
        long dst = addr + offset - 4;
        fseek(fp, dst, SEEK_SET);
        //ignore the rest for now ;) JM
      }
      break;

      default: // Unknown opcode
        {
          JMPRINT(ios::hex, "Unhandled opcode " << opcode);
          if (opcode < 256)
          {
            if ( opcodes[opcode].size != -1)
            {
              DEBUGPRINT( "** " << opcodes[opcode].name << " (size " <<
                opcodes[opcode].size << ")" << std::endl );
              skip_offset = opcodes[opcode].size - 2; // opcode already read
            }
            else
            {
              DEBUGPRINT( "Unhandled opcode " << opcodes[opcode].name
                << " (" << std::hex << opcode << std::dec << ")" <<
                std::endl );
            }
          }
          else
          {
            DEBUGPRINT( "Op-code out of range: " << std::hex << opcode <<
              std::dec << std::endl );
          }
        }
        break;

      }  //end of bgl parse switch 
       
      if (skip_offset > 0) 
        fseek( fp, skip_offset, SEEK_CUR );
       
    } //end of bgl parse while loop

  } // end of bgl "object header opcode" while loop
    
    fclose(fp);
        
    DEBUGPRINT("\n" << vertex_array_->getNum() << " vertices\n");
    
    return model_;
}

#else


ssgEntity *ssgLoadBGL(const char *fname, const ssgLoaderOptions *options)
{
  return NULL ;
}


#endif

