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
// File: ssgLoadBGL.cxx
//
// Created: Tue Feb 29 22:20:31 2000
//
// Author: Thomas Engh Sevaldrud <tse@math.sintef.no>
//         Juergen Marquardt <juergen_marquardt@t-online.de>
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
//           - support for transparent and translucent textures added
//           - support for lines (06,07) added
//           - partial support for 0xa0 and 0x70 added, actually these are ignored
//             at the moment but at least they are parsed correctly ;)
//           - ssgLoadMDLTexture removed from here. It's now a separate file.
//           - FACET may now contain textures. Usually used for the ground areas
//           - Fixed normal calculation for concave polygons; culfacing is
//             working now for them
//           - translucent textures are now drawn in the correct order
//           - clean ups: + DEBUGPRINT removed
//                        + use <iostream> / stl only when JMDEBUG is defined
//                        + M_PI replaced by SGD_PI
//                        + Major clean up for MSVC
//           - Fixed a bug in line drawing
//           - handle PointTo, st refpoint will be taken as reference point for the scenery, start/end surface combinations correctly
//           - support for light maps added
//           - support for concave polygons added
//           - Fixed a bug for object positioning with negative longitude
//           - Emission improved for colored shaded polygons
//           - Layer Call & Layer Call 32 added
//           - Layering for ground textures added
//           - Single line drawing changed again to support one dot in
//             start / end surface context
//           - fixed texture size for polygons with no texture coordinates
//           - reworked building generation (4 different shapes are generated)
//           - support background textures with correct layering
//           - checked re-usage of loaded vertices more carfully (i.e. condsider
//             its normal before re-using them).
//
//===========================================================================
// Copyright (c) 2000 Thomas E. Sevaldrud <tse@math.sintef.no>
// Copyright (c) 2002 Jürgen Marquardt <juergen_marquardt@t-online.de>
//===========================================================================

#ifdef _MSC_VER
#pragma warning ( disable : 4244 4305 )
#endif

#include "ssgLocal.h"

#ifdef SSG_LOAD_BGL_SUPPORTED

//#define JMDEBUG
#include "ssgLoadBGL.h"
#include "ssgLoadMDL.h"
#include "ssgLoadMDL.h"


static ssgLoaderOptions         *current_options;

// Temporary vertex arrays
static ssgIndexArray            *curr_index_;
static ssgIndexArray            *draw_point_index_;

// Vertex arrays
static ssgVertexArray           *vertex_array_;
static ssgNormalArray           *normal_array_;
static ssgTexCoordArray         *tex_coords_;

// Current part (index array)
static ssgLayeredVtxArray       *curr_part_;
static ssgBranch                *model_;
static ssgBranch                *models_;
static ssgTransform             *curr_branch_;

static sgMat4                   curr_matrix_;

// File Address Stack
static const int                MAX_STACK_DEPTH = 128;
static long                     stack_[MAX_STACK_DEPTH];
static int                      stack_depth_;

// john ....
static bool                     poly_from_line;
static unsigned short           poly_from_line_numverts;

static long                     object_end;
static long                     object_end_offset;

// jm's variables
static int                      nr_strip_branches_ = 0;
static bool                     has_background_;
static ssgBranch                *background_[64];
static bool                     perspective_;
static GLboolean                depth_mask_;
static bool                     for_scenery_center;
static long                     scenery_center_lat;
static long                     scenery_center_lon;

// Color state
static COLOR                    line_;
static COLOR                    surface_;
static COLOR                    goraud_;

//static bool                     has_color;
static bool                     has_emission;
static bool                     emissive_color_;
static sgVec4                   GColor;
static sgVec4                   LColor;
static sgVec4                   SColor;

static int                      layer_;

static double                   ref_scale;
static short                    haze_;
static SGfloat                  alpha_;
static SGfloat                  shininess_;

// texture state variables
static char                     tex_filename[128];
static bool                     has_texture;
static char                     *curr_tex_name_;
static int                      curr_tex_type_;
static int                      curr_tex_wrap_;
static int                      curr_tex_flags_;

static struct {
  sgVec3 point;
  sgVec3 norm;
  int    index;
  }                             tmp_vtx[1024];
//static bool                     blending;

// This struct contains variable definitions of MS Flightsimulator
// up to now only season, complexity and day time are supported
// unfortunately we have to set these variables before loading the bgl file
// thus we can't change them on the fly
static struct {
  int var;
  int val;
  }                             vardef[100] = { {0x346,4},      // complexity: 0 lowest; 4 most
                                                {0x6f8,2},      // season: 0=winter; 1=spring;
                                                                //         2=summer; 3=autumn;
                                                {0x28c,0x00},   // Day time: 0=Day, 1=Dusk,
                                                                //           2=Night, bit2=light on/off
                                                {0x000,0}       // END of table
                                              };

//===========================================================================
static int PreDrawSubface(ssgEntity *entity)
{
//   glGetBooleanv(GL_DEPTH_WRITEMASK, &depth_mask_);
//   if (depth_mask_ != GL_FALSE)
   glPushAttrib(GL_DEPTH_BUFFER_BIT);
   glDepthMask(GL_FALSE);
   return 1;
}

//===========================================================================

static int PostDrawSubface(ssgEntity *)
{
//   if (depth_mask_ != GL_FALSE)
//     glDepthMask(GL_TRUE);
   glPopAttrib();
   return 1;
}

//===========================================================================
// ssgLayeredVtxArray Class definition
//===========================================================================

ssgLayeredVtxArray::ssgLayeredVtxArray ( GLenum ty,
                       ssgVertexArray   *vl,
                       ssgNormalArray   *nl,
                       ssgTexCoordArray *tl,
                       ssgColourArray   *cl,
                       ssgIndexArray    *il ) : ssgVtxArray( ty, vl, nl, tl, cl, il )
{
}

bool ssgLayeredVtxArray::isOnGround()
{
  int i;
  for (i=0; i < getNumIndices (); i++) {
    if ( getVertex(*getIndex(i))[2] > .1f )
      return (false);
  }
  return (true);
}

void ssgLayeredVtxArray::moveToBackground()
{
  setCallback(SSG_CALLBACK_PREDRAW, PreDrawSubface);
  setCallback(SSG_CALLBACK_POSTDRAW, PostDrawSubface);
}


//===========================================================================

static int stripEmptyBranches( ssgBranch *branch )
{
  int NrLeafs = 0;
  ssgEntity *entity;
  entity = branch->getKid(0);
  while ( entity != 0 ) {
    if ( entity -> isAKindOf( ssgTypeLeaf() ) ) {
      if ( ( ( (ssgLeaf *) entity ) -> getNumTriangles() == 0) && 
           ( ( (ssgLeaf *) entity ) -> getNumLines() == 0) ) {
        JMPRINT(ios::dec,">>>>>>>>>>>>>>>>> num of tris or lines == 0!");
      }
      NrLeafs++;
    }
    if ( entity -> isAKindOf( ssgTypeBranch() ) ) {
      int n = stripEmptyBranches( (ssgBranch *) entity );
      if ( n == 0) {
        branch -> removeKid( entity );
        nr_strip_branches_++;
      }
      else {
        NrLeafs += n;
      }
    }
    entity = branch->getNextKid();
  }
 return (NrLeafs); 
}

//===========================================================================

static int getVariableValue(int var, int *val)
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

static void newPart()
{
//  has_color = false;
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
  double ref_lat, ref_lng, ref_alt, lat_radius;
  ref_lat  = (double)(int)ulEndianReadLittle16(fp)/65536.0;
  ref_lat += (double)(int)ulEndianReadLittle32(fp); // Latitude
  ref_lng  = (double)(int)ulEndianReadLittle16(fp)/65536.0;
  ref_lng += (double)(int)ulEndianReadLittle32(fp); // Longitude
  ref_alt  = (double)(int)ulEndianReadLittle16(fp)/65536.0;
  ref_alt += (double)(int)ulEndianReadLittle32(fp); // Alitude
  lat_radius = cos(scenery_center_lat*SGD_PI/(2*10001750.0)) * EARTH_RADIUS;
  v[0] = (SGfloat)((double)scenery_center_lat - ref_lat);
  v[1] = (SGfloat)(ref_lng - (double)scenery_center_lon) * 1.46292e-09 * lat_radius;
  v[2] = (SGfloat)ref_alt;
}

//===========================================================================
/*
  ssgTriangulate - triangulate a simple polygon.
*/

static int triangulateConcave(ssgVertexArray *coords, ssgIndexArray *w, int n, int x, int y, ssgIndexArray *tris) // was: triangulate_concave
{
   struct Vtx {
      unsigned short index;
      float x, y;
      Vtx *next;
   };

   Vtx *p0, *p1, *p2, *m0, *m1, *m2, *t;
   int i, chk, num_tris;
   float a0, a1, a2, b0, b1, b2, c0, c1, c2;

   Vtx *p = new Vtx[n];
   /* construct a circular linked list of the vertices */
   p0 = &p[0];
   p0->index = w ? *w->get(0) : 0;
   p0->x = coords->get(p0->index)[x];
   p0->y = coords->get(p0->index)[y];
   p1 = p0;
   p2 = 0;
   for (i = 1; i < n; i++) {
      p2 = &p[i];
      p2->index = w ? *w->get(i) : i;
      p2->x = coords->get(p2->index)[x];
      p2->y = coords->get(p2->index)[y];
      p1->next = p2;
      p1 = p2;
   }
   p2->next = p0;

   m0 = p0;
   m1 = p1 = p0->next;
   m2 = p2 = p1->next;
   chk = 0;
   num_tris = 0;

   while (p0 != p2->next) {
      if (chk && m0 == p0 && m1 == p1 && m2 == p2) {
         /* no suitable vertex found.. */
         ulSetError(UL_WARNING, "ssgTriangulate: Self-intersecting polygon.");
         delete [] p;
         return 0;
      }
      chk = 1;

      a0 = p1->y - p2->y;
      a1 = p2->y - p0->y;
      a2 = p0->y - p1->y;
      b0 = p2->x - p1->x;
      b1 = p0->x - p2->x;
      b2 = p1->x - p0->x;

      if (b0 * a2 - b2 * a0 < 0) {
         /* current angle is concave */
         p0 = p1;
         p1 = p2;
         p2 = p2->next;
      }
      else {
         /* current angle is convex */
         float xmin = MIN3(p0->x, p1->x, p2->x);
         float xmax = MAX3(p0->x, p1->x, p2->x);
         float ymin = MIN3(p0->y, p1->y, p2->y);
         float ymax = MAX3(p0->y, p1->y, p2->y);

         c0 = p1->x * p2->y - p2->x * p1->y;
         c1 = p2->x * p0->y - p0->x * p2->y;
         c2 = p0->x * p1->y - p1->x * p0->y;

         for (t = p2->next; t != p0; t = t->next) {
            /* see if the triangle contains this vertex */
            if (xmin <= t->x && t->x <= xmax &&
                ymin <= t->y && t->y <= ymax &&
                a0 * t->x + b0 * t->y + c0 > 0 &&
                a1 * t->x + b1 * t->y + c1 > 0 &&
                a2 * t->x + b2 * t->y + c2 > 0)
               break;
         }

         if (t != p0) {
            p0 = p1;
            p1 = p2;
            p2 = p2->next;
         }
         else {
            /* extract this triangle */
            tris->add(p0->index);
            tris->add(p1->index);
            tris->add(p2->index);
            num_tris++;

            p0->next = p1 = p2;
            p2 = p2->next;

            m0 = p0;
            m1 = p1;
            m2 = p2;
            chk = 0;
         }
      }
   }

   tris->add(p0->index);
   tris->add(p1->index);
   tris->add(p2->index);
   num_tris++;
   delete [] p;
   return num_tris;
}

//===========================================================================

static int _ssgTriangulate( ssgVertexArray *coords, ssgIndexArray *w, int n, ssgIndexArray *tris )
{
   float *a, *b;
   int i, x, y;

   /* trivial case */
   if (n <= 3) {
      if (n == 3) {
         tris->add( w ? *w->get(0) : 0 );
         tris->add( w ? *w->get(1) : 1 );
         tris->add( w ? *w->get(2) : 2 );
         return 1;
      }
      ulSetError(UL_WARNING, "ssgTriangulate: Invalid number of vertices (%d).", n);
      return 0;
   }

   /* compute areas */
   {
      float s[3], t[3];
      int swap;

      s[0] = s[1] = s[2] = 0;
      b = coords->get(w ? *w->get(n - 1) : n - 1);

      for (i = 0; i < n; i++) {
         a = b;
         b = coords->get(w ? *w->get(i) : i);
         s[0] += a[1] * b[2] - a[2] * b[1];
         s[1] += a[2] * b[0] - a[0] * b[2];
         s[2] += a[0] * b[1] - a[1] * b[0];
      }

      /* select largest area */
      t[0] = ABS(s[0]);
      t[1] = ABS(s[1]);
      t[2] = ABS(s[2]);
      i = t[0] > t[1] ? t[0] > t[2] ? 0 : 2 : t[1] > t[2] ? 1 : 2;
      swap = (s[i] < 0); /* swap coordinates if clockwise */
      x = (i + 1 + swap) % 3;
      y = (i + 2 - swap) % 3;
   }

   /* concave check */
   {
      float x0, y0, x1, y1;

      a = coords->get(w ? *w->get(n - 2) : n - 2);
      b = coords->get(w ? *w->get(n - 1) : n - 1);
      x1 = b[x] - a[x];
      y1 = b[y] - a[y];

      for (i = 0; i < n; i++) {
         a = b;
         b = coords->get(w ? *w->get(i) : i);
         x0 = x1;
         y0 = y1;
         x1 = b[x] - a[x];
         y1 = b[y] - a[y];
         if (x0 * y1 - x1 * y0 < 0)
            return triangulateConcave(coords, w, n, x, y, tris);
      }
   }

   /* convert to triangles */
   {
      int v0 = 0, v1 = 1, v = n - 1;
      int even = 1;
      for (i = 0; i < n - 2; i++) {
         if (even) {
            tris->add( w ? *w->get(v0) : v0);
            tris->add( w ? *w->get(v1) : v1);
            tris->add( w ? *w->get(v) : v);
            v0 = v1;
            v1 = v;
            v = v0 + 1;
         }
         else {
            tris->add( w ? *w->get(v1) : v1);
            tris->add( w ? *w->get(v0) : v0);
            tris->add( w ? *w->get(v) : v);
            v0 = v1;
            v1 = v;
            v = v0 - 1;
         }
         even = !even;
      }
   }
   return n - 2;
}

//===========================================================================

static GLenum createTriangIndices(ssgIndexArray *ixarr,
                                int numverts, const sgVec3 s_norm)
{
  sgVec3 v1, v2, cross;

  if ( numverts > ixarr->getNum() ) {
    ulSetError( UL_WARNING, "[ssgLoadBGL] Index array with too few entries." );
    return(GL_TRIANGLE_FAN);
  }

  // triangulate polygons
  if(numverts == 1)
  {
    unsigned short ix0 = *ixarr->get(0);
    if ( ix0 >= vertex_array_->getNum() ) {
      ulSetError(UL_WARNING, "[ssgLoadBGL] Index out of bounds (%d/%d).",
        ix0, vertex_array_->getNum() );
      return(GL_TRIANGLE_FAN);
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
      ulSetError(UL_WARNING, "[ssgLoadBGL] Index out of bounds. (%d,%d / %d",
        ix0, ix1, vertex_array_->getNum() );
      return(GL_TRIANGLE_FAN);
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
      ulSetError(UL_WARNING, "[ssgLoadBGL] Index out of bounds. " \
        "(%d,%d,%d / %d)", ix0, ix1, ix2, vertex_array_->getNum());
      return(GL_TRIANGLE_FAN);
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
    if ( ( ix0 >= vertex_array_->getNum() ) ||
         ( ix1 >= vertex_array_->getNum() ) ||
         ( ix2 >= vertex_array_->getNum() ) ) {
      ulSetError(UL_WARNING, "[ssgLoadBGL] Index out of bounds. " \
        "(%d,%d,%d / %d)", ix0, ix1, ix2, vertex_array_->getNum());
      return(GL_TRIANGLE_FAN);
    }

    // check for concave polygon
    sgVec3 poly_dir;
    sgZeroVec3(poly_dir);
    int up = 0;
    int down = 0;
    bool dir[100];
    sgVec3 cross;
    sgVec3 v0, v1;
    sgVec3 p0, p1, p2;
    sgCopyVec3( p0, vertex_array_->get(*ixarr->get(numverts-2)));
    sgCopyVec3( p1, vertex_array_->get(*ixarr->get(numverts-1)));
    v0[0] = p0[0]-p1[0];
    v0[1] = p0[1]-p1[1];
    v0[2] = p0[2]-p1[2];

    int i;
    for(i = 0; i < numverts; i++)
    {
      sgCopyVec3( p2, vertex_array_->get(*ixarr->get(i)));
      v1[0] = p2[0]-p1[0];
      v1[1] = p2[1]-p1[1];
      v1[2] = p2[2]-p1[2];
      sgVectorProductVec3 ( cross, v0, v1 );
      sgAddVec3(poly_dir, cross);
      dir[i] = (sgScalarProductVec3(cross, s_norm)<= 0.0);
      if ( dir[i] == true) up++; else down++;
      sgCopyVec3( p1, p2);
      sgCopyVec3( v0, v1);
      sgNegateVec3( v0 );
    }
#ifdef JMDEBUG
    if ( (up != 0 ) && (down != 0)) {
      JMPRINT( ios::dec, "concave poly detected:" << " up:" << up << " down:" << down );
      for(int i = 0; i < numverts; i++)
      {
        JMPRINT(ios::dec, "index:" << *ixarr->get(i) << \
        " " << dir[(i-1)%numverts] << " " \
        " point[0]:" << vertex_array_->get(*ixarr->get(i))[0] << \
        " point[1]:" << vertex_array_->get(*ixarr->get(i))[1] << \
        " point[2]:" << vertex_array_->get(*ixarr->get(i))[2] );
      }
    }
#endif
    ssgIndexArray *strips = new ssgIndexArray(3* (numverts-2));
    ssgIndexArray *idx_array;
    GLenum ret;
    if ( (up != 0 ) && (down != 0)) {
      numverts = _ssgTriangulate(vertex_array_, ixarr, numverts, strips);
      idx_array = strips;
      // polygon was concave: we have to get the direction again
      for ( i = 0; i < numverts; i++) {
        sgCopyVec3( p0, vertex_array_->get(*idx_array->get(i*3+0)));
        sgCopyVec3( p1, vertex_array_->get(*idx_array->get(i*3+1)));
        sgCopyVec3( p2, vertex_array_->get(*idx_array->get(i*3+2)));
        v0[0] = p0[0]-p1[0];
        v0[1] = p0[1]-p1[1];
        v0[2] = p0[2]-p1[2];
        v1[0] = p2[0]-p1[0];
        v1[1] = p2[1]-p1[1];
        v1[2] = p2[2]-p1[2];
        sgVectorProductVec3 ( poly_dir, v0, v1 );
        sgZeroVec3(v0);
        if (sgCompareVec3(poly_dir, v0, 0.1f ) == false) break; // we have a result: break
      }
      numverts *= 3;
      ret = GL_TRIANGLES;
    }
    else {
      idx_array = ixarr;
      ret = GL_TRIANGLE_FAN;
    }

    // Ensure counter-clockwise ordering
    // and write to curr_index_
    bool flip = (sgScalarProductVec3(poly_dir, s_norm) > 0.0);

    for(i = 0; i < numverts; i++)
    {
      unsigned short ix = *idx_array->get( flip ? numverts-i-1 : i);

      if ( ix >= vertex_array_->getNum() ) {
        ulSetError(UL_WARNING, "[ssgLoadBGL] Index out of bounds. (%d/%d)",
          ix, vertex_array_->getNum());
        continue;
      }
      curr_index_->add(ix);
    }
    return(ret);
  }
  return (GL_TRIANGLE_FAN);    // Ensure counter-clockwise ordering
}

//===========================================================================

static GLenum readTexIndices(FILE *fp, int numverts, sgVec3 s_norm, bool flip_y)
{
  if(numverts <= 0)
    return GL_FALSE;

  ssgIndexArray *curr_index_ = new ssgIndexArray();

  // add dummy texture coordinates if necessary
  if(tex_coords_->getNum() < vertex_array_->getNum())
  {
    sgVec2 dummy_pt;
    sgSetVec2(dummy_pt, FLT_MAX, FLT_MAX);
    for(int i = tex_coords_->getNum(); i < vertex_array_->getNum(); i++)
      tex_coords_->add(dummy_pt);
  }

  // add dummy normal vectors if necessary
  while (normal_array_->getNum() < vertex_array_->getNum())
    normal_array_->add(s_norm);

  sgVec3 zeroVec3;
  sgZeroVec3(zeroVec3);
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
      normal_array_->add(sgEqualVec3(tmp_vtx[ix].norm, zeroVec3)==TRUE ? s_norm : tmp_vtx[ix].norm);
      tex_coords_->add(dummy_pt);
      tmp_vtx[ix].index = tex_idx;
    }
    else {
      // vertex is already in use
      // we can reuse this vertex if normals match
      sgVec3 v1, v2;
      sgCopyVec3(v1, normal_array_->get(tmp_vtx[ix].index));
      sgCopyVec3(v2, sgEqualVec3(tmp_vtx[ix].norm, zeroVec3)==TRUE ? s_norm : tmp_vtx[ix].norm);
      // if angle between normals < 10 degrees ..
      if (sgScalarProductVec3(v1, v2) / (sgLengthVec3(v1)*sgLengthVec3(v2) ) < 0.9848f ) {

        // normals don't match we have to add a new vertex
        sgVec2 dummy_pt;
        tex_idx = vertex_array_->getNum();
        vertex_array_->add(tmp_vtx[ix].point);
        normal_array_->add(v2);
        tex_coords_->add(dummy_pt);
        tmp_vtx[ix].index = tex_idx;
      }
      else {
        tex_idx = tmp_vtx[ix].index;
      }
    }

    sgVec2 tc;
    sgSetVec2(tc, tx_int/255.0f, ty_int/255.0f);

    sgVec2 curr_tc;

    if ( tex_idx >= 0 && tex_idx < tex_coords_->getNum() ) {
      sgCopyVec2(curr_tc, tex_coords_->get(tex_idx));
    } else {
      ulSetError( UL_WARNING, "[ssgLoadBGL] Texture coord out of range (%d).", tex_idx );
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
      normal_array_->add(sgEqualVec3(tmp_vtx[ix].norm, zeroVec3)==TRUE ? s_norm : tmp_vtx[ix].norm);

      tex_coords_->add(tc);

    }

    curr_index_->add(tex_idx);

#ifdef JMDEBUG
    int check_index = *curr_index_->get(v);
    float *check_tc = tex_coords_->get(check_index);
    JMPRINT(ios::dec, "ix[" << v << "] = " << check_index <<
      " (u=" << check_tc[0] << ", v=" << check_tc[1] << ")");
#endif

  }

  return (createTriangIndices(curr_index_, numverts, s_norm));
}

//===========================================================================

static GLenum readIndices(FILE* fp, int numverts, sgVec3 s_norm, bool use_texture)
{
  if(numverts <= 0)
    return GL_FALSE;

  ssgIndexArray *curr_index_ = new ssgIndexArray();

  if (use_texture == true) {
  // add dummy texture coordinates if necessary
    if(tex_coords_->getNum() < vertex_array_->getNum())
    {
      sgVec2 dummy_pt;
      sgSetVec2(dummy_pt, FLT_MAX, FLT_MAX);
      for(int i = tex_coords_->getNum(); i < vertex_array_->getNum(); i++)
        tex_coords_->add(dummy_pt);
    }
  }

  // add dummy normal vectors if necessary
  while (normal_array_->getNum() < vertex_array_->getNum())
    normal_array_->add(s_norm);

  sgVec3 zeroVec3;
  sgZeroVec3(zeroVec3);
  // Read index values
  for(int v = 0; v < numverts; v++)
  {
    unsigned short ix;
    ix = ulEndianReadLittle16(fp);
    // is Vertex already used ?
    if (tmp_vtx[ix].index == -1) { // no: add vertex
      int last_idx = vertex_array_->getNum();
      vertex_array_->add(tmp_vtx[ix].point);
      normal_array_->add(sgEqualVec3(tmp_vtx[ix].norm, zeroVec3)==TRUE ? s_norm : tmp_vtx[ix].norm);
      tmp_vtx[ix].index = last_idx;
      if (use_texture == true) {
        sgVec2 tc;
        sgSetVec2(tc, FLT_MAX, FLT_MAX);
        tex_coords_->add(tc);
      }
    }
    else {
      // vertex is already in use
      // we can reuse this vertex if normals match
      sgVec3 v1, v2;
      sgCopyVec3(v1, normal_array_->get(tmp_vtx[ix].index));
      sgCopyVec3(v2, sgEqualVec3(tmp_vtx[ix].norm, zeroVec3)==TRUE ? s_norm : tmp_vtx[ix].norm);
      // if angle between normals < 10 degrees ..
      if (sgScalarProductVec3(v1, v2) / (sgLengthVec3(v1)*sgLengthVec3(v2) ) < 0.9848f ) {

        // normals don't match we have to add a new vertex
        int last_idx = vertex_array_->getNum();
        vertex_array_->add(tmp_vtx[ix].point);
        normal_array_->add(v2);
        tmp_vtx[ix].index = last_idx;
        if (use_texture == true) {
          sgVec2 tc;
          sgSetVec2(tc, FLT_MAX, FLT_MAX);
          tex_coords_->add(tc);
        }
      }
    }
    int tex_idx = tmp_vtx[ix].index;
    if (use_texture == true) {
      sgVec2 tc;
      sgVec3 x,y;
      sgSetVec3(x, 1.0f, 0.0f, 0.0f);
      sgVectorProductVec3(y, s_norm, x );

      tc[0] = sgScalarProductVec3(x, tmp_vtx[ix].point)*ref_scale/256.0f;  // texture coordinates are given in delta units
      tc[1] = sgScalarProductVec3(y, tmp_vtx[ix].point)*ref_scale/256.0f;  // texture coordinates are given in delta units

      sgVec2 curr_tc;
      if ( tex_idx >= 0 && tex_idx < tex_coords_->getNum() ) {
        sgCopyVec2(curr_tc, tex_coords_->get(tex_idx));
      } else {
        ulSetError( UL_WARNING, "[ssgLoadBGL] Texture coord out of range (%d).", tex_idx );
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
        normal_array_->add(sgEqualVec3(tmp_vtx[ix].norm, zeroVec3)==TRUE ? s_norm : tmp_vtx[ix].norm);

        tex_coords_->add(tc);

      }

    }
    curr_index_->add(tex_idx);
    JMPRINT( ios::dec, "ix:" << ix << "idx:" << tmp_vtx[ix].index);
  }

  return(createTriangIndices(curr_index_, numverts, s_norm));
}

//===========================================================================

static void setColor(COLOR *color, int cindex, int pal_id)
{
  if(pal_id == 0x68)
  {
    color->color[0] = fsAltPalette[cindex].r / 255.0f;
    color->color[1] = fsAltPalette[cindex].g / 255.0f;
    color->color[2] = fsAltPalette[cindex].b / 255.0f;
    color->color[3] = 0.2f;
  }
  else
  {
    color->color[0] = fsAcPalette[cindex].r / 255.0f;
    color->color[1] = fsAcPalette[cindex].g / 255.0f;
    color->color[2] = fsAcPalette[cindex].b / 255.0f;
    color->color[3] = 1.0f;
  }
  color->has_emission = ( (cindex > 0xe) && (cindex < 0x17) ) ? true:false;
  color->has_alpha = false;
}

//===========================================================================

static void setColor(COLOR *color, int r, int g, int b, int attr)
{
  switch (attr) {
  
  case 0xf0: setColor( color, r, 0xf0); break;
  
//  case 0xbf: color->emissive = true; //???
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
               color->color[0] = r / 255.0f;
               color->color[1] = g / 255.0f;
               color->color[2] = b / 255.0f;
               color->color[3] = (attr & 0xf) / 15.0;
               color->has_alpha = (attr == 0xef)? false : true;
               color->has_emission = false;
             } break;
   default: {
               color->color[0] = r / 255.0f;
               color->color[1] = g / 255.0f;
               color->color[2] = b / 255.0f;
               color->color[3] = 1.0f;
               color->has_alpha = false;
               color->has_emission = false;
               ulSetError( UL_WARNING, "[ssgLoadBGL] Set Color unknown color attribut: %x", (short)attr);
             } break;

  }
}

//===========================================================================

static void setColor(FILE *fp, COLOR *color)
{
  unsigned char cindex, param;
  fread(&cindex, 1, 1, fp);
  fread(&param, 1, 1, fp);
  JMPRINT( ios::hex, "Set Color = " << (int)cindex << " Para = " << (int)param );
  setColor(color, (int)cindex, (int)param);
}

//===========================================================================

static void setTrueColor(FILE *fp, COLOR *color)
{
  unsigned char rgba[4];
  fread(rgba, 1, 4, fp);
  JMPRINT( ios::hex, "Set Color24 = " << (int)rgba[0] << ", " << (int)rgba[2] <<
                     ", " << (int)rgba[3] << ", " << (int)rgba[1] );
  setColor(color, rgba[0], rgba[2], rgba[3], rgba[1]);
}

//===========================================================================

static bool setTexture(char* name, int type, int flags)
{
  curr_tex_name_ = name;
  curr_tex_type_ = type;
  curr_tex_flags_ = flags;

  return true;
}


//===========================================================================
// tex_name must have space for length of tex_name + length of tex_ext

static bool lookUpTexture(char *tex_name, const char *tex_ext)
{
  char *p =strrchr(tex_name,'.');
  if ( p != NULL ) {
    char tex_org[MAX_PATH_LENGTH];
    strcpy(tex_org, tex_name);

    char tname[MAX_PATH_LENGTH];
    *p = '\0';
    strcat(tex_name, tex_ext);
    strcat(tex_name, strrchr(tex_org,'.'));
    current_options->makeTexturePath(tname, tex_name);
    if ( ulFileExists ( tname ) == true) {       // look if light map exists
      return(true);
    }
    else {
      strcpy(tex_name, tex_org);
      return(false);
    }
  }
  return(false);
}

//===========================================================================

static ssgSimpleState *createState(bool use_texture, COLOR *cs)
{
  JMPRINT(ios::dec,"new State: col = " << cs->color[0] << ", " \
                                       << cs->color[1] << ", " \
                                       << cs->color[2] << ", " \
                                       << cs->color[3]);

  ssgSimpleState *state = new ssgSimpleState();

  state->setShadeModel(GL_SMOOTH);
  state->enable   (GL_LIGHTING);
  state->enable   (GL_CULL_FACE);
  state->disable  (GL_COLOR_MATERIAL);

  if(curr_tex_name_ != NULL && use_texture)
  {
    char tex_name[MAX_PATH_LENGTH];
    strcpy(tex_name, curr_tex_name_);

    bool enable_emission = false;
    int day_time;
    getVariableValue(0x28c, &day_time);
    if ( curr_tex_type_ == 0x43) {
      if ( ((curr_tex_flags_ & TF_LIGHT_MAP) == TF_LIGHT_MAP) && ((day_time&0x04) == 0x04) ){ // texture has a light map
        if ( lookUpTexture(tex_name, "_lm") == true ) {
          enable_emission = true;
        }
      }
      else {
        int season;
        if(getVariableValue(0x6f8, &season) == 0 ){           // get season
          if ( ((curr_tex_flags_ & TF_SPRING) == TF_SPRING) && (season == 1) ) {
            lookUpTexture(tex_name, "_sp");
          }
          if ( ((curr_tex_flags_ & TF_FALL) == TF_FALL) && (season == 3) ) {
            lookUpTexture(tex_name, "_fa");
          }
          if ( ((curr_tex_flags_ & TF_WINTER) == TF_WINTER) && (season == 0) ) {
            lookUpTexture(tex_name, "_wi");
          }
        }
      }
    }
    char tname[MAX_PATH_LENGTH];
    if (haze_ > 0) {
      sprintf(tname,"%s_%d",tex_name, haze_);
    }
    else {
      sprintf(tname,"%s",tex_name);
    }
    JMPRINT(ios::dec,".. using Texture:" << tname);
    ssgTexture* tex = current_options ->createTexture(tname, curr_tex_wrap_, curr_tex_wrap_ );
    state->setTexture( tex ) ;

    // handle opaque/translucent textures
    if ( ( (curr_tex_type_ == 0x18)&&(haze_ > 0) ) ||
         ( alpha_ != 1.0f ) ||
         ( cs->has_alpha == true ) ||
         ( (curr_tex_type_ == 0x43)&&(tex->hasAlpha() == true) ) ){
      state->enable(GL_BLEND);
      state->enable(GL_ALPHA_TEST);
      state->setAlphaClamp(.2) ;
      state->setTranslucent() ;
    } else {
      state->disable(GL_BLEND);
      state->disable(GL_ALPHA_TEST);
      state->setOpaque();
    }
    state->setMaterial( GL_AMBIENT, 1.0f, 1.0f, 1.0f, alpha_ );
    state->setMaterial( GL_DIFFUSE, 1.0f, 1.0f, 1.0f, alpha_ );
    if (enable_emission == true) {
      state->setMaterial( GL_EMISSION, 1.0f, 1.0f, 1.0f, 1.0f );
    }
    else {
      if  ( (has_emission == true) && ((day_time&0x06) != 0x0) ) {
        state->setMaterial( GL_EMISSION, cs->color[0], cs->color[1], cs->color[2], 1.0 );
      }
      else {
        state->setMaterial( GL_EMISSION, 0.0f, 0.0f, 0.0f, 1.0f );
      }
    }
    state->enable(GL_TEXTURE_2D);
  }
  else
  // solid color 
  {
    if ( (cs->color[3] < 0.99f) || (alpha_ < 0.99f) )
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
//    int day_time;
//    getVariableValue(0x28c, &day_time);               // get daytime
    
    if (cs->has_emission == true) {
      state->setMaterial( GL_AMBIENT, 0.0f, 0.0f, 0.0f, 1.0f );
      state->setMaterial( GL_DIFFUSE, 0.0f, 0.0f, 0.0f, 1.0f );
      state->setMaterial( GL_EMISSION, cs->color[0],
                                       cs->color[1],
                                       cs->color[2],
                                       (alpha_ < 0.99f)? alpha_:cs->color[3] );
      state->setShininess(0);
    }
    else {
      state->setMaterial( GL_AMBIENT,  cs->color[0],
                                       cs->color[1],
                                       cs->color[2],
                                       (alpha_ < 0.99f)? alpha_:cs->color[3] );
      state->setMaterial( GL_DIFFUSE,  cs->color[0],
                                       cs->color[1],
                                       cs->color[2],
                                       (alpha_ < 0.99f)? alpha_:cs->color[3] );
      state->setMaterial( GL_EMISSION, 0.0f, 0.0f, 0.0f, 1.0f );
      state->setShininess(shininess_);
      state->setMaterial( GL_SPECULAR, 1.0f, 1.0f, 1.0f, 1.0f );
    }
    state->disable(GL_TEXTURE_2D);
  }

  has_emission = false;
  return state;
}

//===========================================================================

static ssgBranch *getCurrBranch() {
  if(perspective_ == true)
    return model_;
  else
  {
    return background_[layer_];
  }
}
//===========================================================================

static ssgBranch *getCurrGroup() {
// Find the correct parent for the new group
  if(curr_branch_)
    return curr_branch_;
  else
  {
    return getCurrBranch();
  }
}

//===========================================================================

static void readFacet(FILE *fp, COLOR *color, bool use_texture)
{
  curr_index_ = new ssgIndexArray();
  unsigned short numverts = ulEndianReadLittle16(fp);

  // Surface normal
  sgVec3 v;
  readVector(fp, v);
  JMPRINT(ios::dec, "surface normal:" << v[0] << "; " << v[1] << "; " << v[2] );

  // dot-ref
  ulEndianReadLittle32(fp);
  
  // Read vertex indices
  GLenum type = readIndices(fp, numverts, v, use_texture);
  
  curr_part_ = new ssgLayeredVtxArray(type,
          vertex_array_,
          normal_array_,
          use_texture==true?tex_coords_:NULL,
          NULL,
          curr_index_);

  if ((perspective_ == false) && (curr_part_->isOnGround() == true)) {
    // we have a background texture
    curr_part_->moveToBackground();
    shininess_ = 0;        // background does not shine
    curr_part_->setState( createState(use_texture, color) );
    shininess_ = DEF_SHININESS;
  }
  else {
    curr_part_->setState( createState(use_texture, color) );
  }
  ssgBranch* grp = getCurrGroup();
  ((ssgVtxArray *)curr_part_)->removeUnusedVertices();
  grp->addKid( current_options -> createLeaf(curr_part_, NULL) );
}

//===========================================================================

static void readTMapFacet(FILE *fp, COLOR *color)
{
  curr_index_ = new ssgIndexArray();
  unsigned short numverts = ulEndianReadLittle16(fp);

  // Normal vector
  sgVec3 v;
  readVector(fp, v);

  JMPRINT(ios::dec, "surface normal:" << v[0] << "; " << v[1] << "; " << v[2] );
  // Dot product reference
  ulEndianReadLittle32(fp);

  // Read vertex inidices and texture coordinates
  bool flip_y = FALSE;
  if(curr_tex_name_!=NULL)
  {
    char *texture_extension = curr_tex_name_ + strlen(curr_tex_name_) - 3;
    flip_y = ulStrEqual( texture_extension, "BMP" ) != 0 ;
  }

  GLenum type = readTexIndices(fp, numverts, v, flip_y);
  
  curr_part_ = new ssgLayeredVtxArray( type,
          vertex_array_,
          normal_array_,
          tex_coords_,
          NULL,
          curr_index_ );
  curr_part_->setState( createState(true, color) );

  ssgBranch* grp = getCurrGroup();
	((ssgVtxArray *)curr_part_)->removeUnusedVertices();
  grp->addKid( current_options -> createLeaf(curr_part_, NULL) );
}

//===========================================================================

ssgEntity *ssgLoadBGLBatch(const char *fname, const ssgLoaderOptions *options)
{

  ssgSetCurrentOptions ( (ssgLoaderOptions*)options ) ;
  current_options = ssgGetCurrentOptions () ;
  for_scenery_center = true;
  char filename [ 1024 ] ;
  current_options -> makeModelPath ( filename, fname ) ;

  FILE *fp = fopen(filename, "rb");
  if(!fp)
  {
    ulSetError( UL_WARNING, "[ssgLoadBGL] Couldn't open BGL batch file '%s'!",
      filename );
    return NULL;
  }
  // check for path in batch file name
  strcpy(filename, fname);
  char *p = strrchr(filename, SLASH);
  if ( p != 0) {
    p++;
  }
  else {
    p = filename;
  }
  *p = '\0';
  models_ = new ssgBranch();
  char* model_name = new char[128];
  char *ptr = (char*)&fname[strlen(fname) - 1];
  while(ptr != &fname[0] && *ptr != SLASH) ptr--;
  if(*ptr == SLASH) ptr++;
  strcpy(model_name, ptr);
  ptr = &model_name[strlen(model_name)];
  while(*ptr != '.' && ptr != &model_name[0]) ptr--;
  *ptr = '\0';
  models_->setName(model_name);

  int i;
  for (i = 0; i < 64; i++) {
    background_[i] = new ssgBranch();
    sprintf(model_name,"background (Layer %d)",i);
    background_[i]->setName(model_name);
    models_->addKid(background_[i]);
  }
  has_background_ = true;
  nr_strip_branches_ = 0;
  while (feof(fp) == 0 ) {
    fscanf(fp, "%s", p);
    models_->addKid(ssgLoadBGLFile( filename )) ;
  }
  int leafs = stripEmptyBranches (models_);
  JMPRINT(ios::dec,"Leafs:" << leafs << "; stripped Branches:" << nr_strip_branches_);
  return(models_);
}

ssgEntity *ssgLoadBGL(const char *fname, const ssgLoaderOptions *options)
{

  ssgSetCurrentOptions ( (ssgLoaderOptions*)options ) ;
  current_options = ssgGetCurrentOptions () ;
  for_scenery_center = true;
  has_background_ = false;
  nr_strip_branches_ = 0;
  models_ = ssgLoadBGLFile( fname );
  int leafs = stripEmptyBranches (models_);
  JMPRINT(ios::dec,"Leafs:" << leafs << "; stripped Branches:" << nr_strip_branches_);
  return (models_) ;
}

static ssgBranch *ssgLoadBGLFile(const char *fname )
{
  char filename [ 1024 ] ;
  current_options -> makeModelPath ( filename, fname ) ;

  FILE *fp = fopen(filename, "rb");
  if(!fp)
  {
    ulSetError( UL_WARNING, "[ssgLoadBGL] Couldn't open BGL file '%s'!",
      filename );
    return NULL;
  }

  // Initialize object graph
  model_ = new ssgBranch();
  char* model_name = new char[128];
  char *ptr = (char*)&fname[strlen(fname) - 1];
  while(ptr != &fname[0] && *ptr != SLASH) ptr--;
  if(*ptr == SLASH) ptr++;
  strcpy(model_name, ptr);
  ptr = &model_name[strlen(model_name)];
  while(*ptr != '.' && ptr != &model_name[0]) ptr--;
  *ptr = '\0';
  model_->setName(model_name);
  if ( has_background_ == false) {         // add layer branches only once
    int i;
    for (i = 0; i < 64; i++) {
      background_[i] = new ssgBranch();
      sprintf(model_name,"background (Layer %d)",i);
      background_[i]->setName(model_name);
      model_->addKid(background_[i]);
    }
    has_background_ = true;
  }

  short object_data;
  long band_addr;
  fseek(fp, 0x3a, SEEK_SET);
  fread(&object_data, 2, 1, fp);        // get adress of procedural scenery section
  fseek(fp, object_data, SEEK_SET);     // seek to procedural scenery section
  while ( getc(fp) != 0) {              // end of table ?
    fseek(fp, 4, SEEK_CUR);
    band_addr = ulEndianReadLittle32(fp);
    int obj_adr =ftell(fp);
    JMPRINT(ios::hex," Band adr:" << band_addr);
    fseek(fp, band_addr+object_data, SEEK_SET);   // seek to object header
    parse_proc_scenery(fp);
    fseek(fp, obj_adr, SEEK_SET);
  }
  fclose(fp);
  return model_;
}

static void parse_proc_scenery(FILE* fp)
{
//  sgSetVec4(GColor, 1.0f, 1.0f, 1.0f, 0.0f);      // set Emission value to default
  alpha_ = 1.0f;                                  // set aplpha value to default: opaque
  shininess_ = DEF_SHININESS;                     // set aplpha shininess to default value

  poly_from_line = false;
  emissive_color_ = false;

  // Create group nodes for textured and non-textured objects
  vertex_array_ = new ssgVertexArray();
  normal_array_ = new ssgNormalArray();
  tex_coords_ = new ssgTexCoordArray();

  curr_branch_=0;
  stack_depth_ = 0;
  layer_ = ZERO_LAYER; // default layer
  sgMakeIdentMat4(curr_matrix_);

  // Parse opcodes
  bool done = false;
  long object_position_lat;
  long object_position_lon;

  // john ... we now consider BGL "object header opcode"
  while(!feof(fp)) {
    char header_opcode;
    char image_power;
    fread (&header_opcode, 1, 1, fp);
    JMPRINT(ios::hex,"Header Opcode is " << (int)header_opcode);
    if (header_opcode == 0)
      break;
    object_position_lat = ulEndianReadLittle32(fp);
    object_position_lon = ulEndianReadLittle32(fp);
    if (for_scenery_center == true) {
      scenery_center_lat = object_position_lat;
      scenery_center_lon = object_position_lon;
      JMPRINT(ios::hex, "Reference Lat:    " <<  object_position_lat << "; Reference Lon:" << object_position_lon);
      ulSetError ( UL_DEBUG, "[ssgLoadBGL] Reference Lat: %f; Reference Lon: %f \n", object_position_lat*90.0/10001750.0, object_position_lon*90.0/(256.0* 4194304.0)) ;
      for_scenery_center = false;
    }
//    JMPRINT(ios::hex, ftell(fp)-2 << "obj lat:" << object_position_lat << "  obj lon" << object_position_lon);
    fread (&image_power, 1, 1, fp);
    object_end_offset = ulEndianReadLittle16(fp);
    object_end = object_end_offset + ftell(fp) - 12;
    JMPRINT( ios::hex, "Sector start:" << ftell(fp) << "; Object end offset:" << object_end_offset << "; Object end:" << object_end );
    done = false;
    perspective_ = false;

    while(!feof(fp) && !done && (ftell(fp) < object_end) )
    {
      unsigned int   skip_offset = 0;
      unsigned short opcode;

      fread(&opcode, 2, 1, fp);
      JMPRINT(ios::hex, ftell(fp)-2 << ":    " << opcode << "  " << (opcode<=255?opcodes[opcode].name:"error: opcode out of range"));

      switch(opcode)
      {
      case 0x23:        // BGL_CALL
        {
          short offset;
          offset = ulEndianReadLittle16(fp);
          long addr = ftell(fp);
          ssgBranch* grp = getCurrGroup();
          push_stack(perspective_);
          push_stack(layer_);
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
          ssgBranch* grp = getCurrGroup();
          push_stack(perspective_);
          push_stack(layer_);
          push_stack((long int)grp);
          push_stack(addr);
          long dst = addr + offset - 6;
          fseek(fp, dst, SEEK_SET);
        }
        break;

      case 0x74:        // Layer call
        {
          short offset;
          offset = ulEndianReadLittle16(fp);
          push_stack(perspective_);
          push_stack(layer_);
          layer_ = ulEndianReadLittle16(fp)&0x3f;
          JMPRINT(ios::dec, "layer:" << layer_);
          long addr = ftell(fp);
          ssgBranch* grp = getCurrGroup();
          push_stack((long int)grp);
          push_stack(addr);
          long dst = addr + offset - 6;
          fseek(fp, dst, SEEK_SET);
        }
        break;

      case 0x8b:        // Layer call 32
        {
          int offset;
          offset = ulEndianReadLittle32(fp);
          push_stack(perspective_);
          push_stack(layer_);
          layer_ = ulEndianReadLittle32(fp)&0x3f;
          JMPRINT(ios::dec, "layer:" << layer_);
          long addr = ftell(fp);
          ssgBranch* grp = getCurrGroup();
          push_stack((long int)grp);
          push_stack(addr);
          long dst = addr + offset - 10;
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
        }
        break;
      
      case 0x88:        // BGL_JUMP32
        {
          int offset;
          offset = ulEndianReadLittle32(fp);
          long addr = ftell(fp);
          long dst = addr + offset - 6;
          fseek(fp, dst, SEEK_SET);
        }
        break;
      
      case 0x8e:        // BGL_VFILE_MARKER
        {
          short offset;
          offset = ulEndianReadLittle16(fp);
          break;
        }

      case 0x76:        // Perspectve
      case 0x7d:        // Perspectve32
        {
          perspective_ = true;
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

      case 0x25:  // BGL_SEPARATION_PLANE
        {
          int offset = ulEndianReadLittle16(fp);
          long addr = ftell(fp);
          long dst = addr + offset - 4;
          int east = ulEndianReadLittle16(fp);
          int alt = ulEndianReadLittle16(fp);
          int north = ulEndianReadLittle16(fp);
          int distance = ulEndianReadLittle32(fp);
          JMPRINT(ios::hex, "BGL_SEPARATION_PLANE: offset:" << offset <<
                            ", east " << east << ", alt " << alt <<
                            ", north " << north << ", distance " << distance * ref_scale );
//          fseek(fp, dst, SEEK_SET);
        }
        break;

      case 0x21:  // BGL_IFIN3
        {
          short offset, lo, hi;
          int val;
          int stat;
          bool s = false;
          unsigned short var;
          offset = ulEndianReadLittle16(fp);
          JMPRINT(ios::dec, "BGL_IFIN3: Jump on 3 vars: Offset" << offset);
          long addr = ftell(fp);
          for (int i=0; i<3; i++) {
            var    = ulEndianReadLittle16(fp);
            lo     = ulEndianReadLittle16(fp);
            hi     = ulEndianReadLittle16(fp);
            stat   = getVariableValue(var, &val);
            JMPRINT(ios::dec, "BGL_IFIN3: var: 0x" << std::hex << var <<", lo:" << std::dec << lo << ", hi:" << hi );
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

      case 0x1c:  // BGL_IFIN2
        {
          short offset, lo, hi;
          int val;
          int stat;
          bool s = false;
          unsigned short var;
          offset = ulEndianReadLittle16(fp);
          JMPRINT(ios::dec, "BGL_IFIN2: Jump on 2 vars: Offset" << offset);
          long addr = ftell(fp);
          for (int i=0; i<2; i++) {
            var    = ulEndianReadLittle16(fp);
            lo     = ulEndianReadLittle16(fp);
            hi     = ulEndianReadLittle16(fp);
            stat   = getVariableValue(var, &val);
            JMPRINT(ios::dec, "BGL_IFIN2: var: 0x" << std::hex << var <<", lo:" << std::dec << lo << ", hi:" << hi );
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
          JMPRINT(ios::dec, "BGL_IFIN1: Jump on var: Offset" << offset << ", var: 0x" << std::hex << var <<
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
          ssgAxisTransform* tmp = NULL;
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
          push_stack(perspective_);
          push_stack(layer_);
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
         break;
        }

      case 0x3b:  // BGL_VINSTANCE
        {
          short offset, var;
          offset = ulEndianReadLittle16(fp);
          var    = ulEndianReadLittle16(fp);
          JMPRINT(ios::hex,"Call3b var:" << var );

          long addr = ftell(fp);
          long dst = addr + offset - 6;
          // push current branch
          ssgBranch* grp = getCurrGroup();
          push_stack(perspective_);
          push_stack(layer_);
          push_stack((long int)grp);
          push_stack(addr);
          fseek(fp, dst, SEEK_SET);
        }
        break;

      case 0x0:   // EOF
        {
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
          if(stack_depth_ == 0) {
          }
          else
          {
            long addr = pop_stack();
            curr_branch_ = (ssgTransform *)pop_stack();
            layer_ = pop_stack();
            perspective_ = pop_stack();
            fseek(fp, addr, SEEK_SET);
          }
        }
        break;

      case 0x1a:  // RESLIST (point list with no normals)
        {
          newPart();

          int start_idx            = ulEndianReadLittle16(fp);
          unsigned short numpoints = ulEndianReadLittle16(fp);

          JMPRINT( ios::dec, "RESLIST: First:" << start_idx << " Nr:" << numpoints);
          for(int i = 0; i < numpoints; i++)
          {
            readPoint(fp, tmp_vtx[start_idx+i].point);
            sgZeroVec3( tmp_vtx[start_idx+i].norm );
            tmp_vtx[start_idx+i].index=-1; // mark as dirty
            JMPRINT( ios::dec, "RESLIST: 0:" << tmp_vtx[start_idx+i].point[0] << \
                                       " 1:" << tmp_vtx[start_idx+i].point[1] << \
                                       " 2:" << tmp_vtx[start_idx+i].point[2]);
          }
        }
        break;

      case 0x29:  // GORAUD RESLIST (point list with normals)
        {
          newPart();

          int start_idx            = ulEndianReadLittle16(fp);
          unsigned short numpoints = ulEndianReadLittle16(fp);

          JMPRINT( ios::dec, "GORAUD RESLIST: First:" << start_idx << " Nr:" << numpoints);
          for(int i = 0; i < numpoints; i++)
          {
            readPoint(fp, tmp_vtx[start_idx+i].point);
            readVector(fp, tmp_vtx[start_idx+i].norm);
            JMPRINT( ios::dec, "GORAUD RESLIST: 0:" << tmp_vtx[start_idx+i].point[0] << \
                                              " 1:" << tmp_vtx[start_idx+i].point[1] << \
                                              " 2:" << tmp_vtx[start_idx+i].point[2]);
            tmp_vtx[start_idx+i].index=-1; // mark as dirty
          }
        }
        break;
      case 0x06:  // STRRES: Start line definition
        {
          curr_index_ = new ssgIndexArray();
          draw_point_index_ = new ssgIndexArray();
          readPoint(fp, tmp_vtx[0].point);
          int ix = vertex_array_->getNum();
          vertex_array_->add(tmp_vtx[0].point);
          tmp_vtx[0].index=ix;                      // mark as used
          draw_point_index_->add(tmp_vtx[0].index);
          poly_from_line_numverts = 1;
         }
         break;
      case 0x07:  // STRRES: Ends line definition
        {
          if ( poly_from_line_numverts == 0) {      // do we have to continue a poly ? YES
            curr_index_ = new ssgIndexArray();
            vertex_array_->add(tmp_vtx[1].point);
          }
          else {
            delete draw_point_index_;
          }
          readPoint(fp, tmp_vtx[1].point);
          int ix = vertex_array_->getNum();
          vertex_array_->add(tmp_vtx[1].point);
          curr_index_->add(ix-1);
          curr_index_->add(ix);
          tmp_vtx[1].index=ix; // mark as dirty
          curr_part_ = new ssgLayeredVtxArray( GL_LINES,
            vertex_array_,
            NULL,
            NULL,
            NULL,
            curr_index_ );
          curr_part_->setState( createState(false, &line_) );
          ssgBranch* grp = getCurrGroup();
          ((ssgVtxArray *)curr_part_)->removeUnusedVertices();
					grp->addKid( current_options -> createLeaf(curr_part_, NULL) );
          poly_from_line_numverts = 0;
        }
        break;
      case 0x0f:  // STRRES: Start poly line definition
        {
          unsigned short idx = ulEndianReadLittle16(fp);

          curr_index_ = new ssgIndexArray();
          draw_point_index_ = new ssgIndexArray();

          if (tmp_vtx[idx].index == -1) { // add vertex
            int ix = vertex_array_->getNum();
            vertex_array_->add(tmp_vtx[idx].point);
            normal_array_->add(tmp_vtx[idx].norm);
            tmp_vtx[idx].index = ix;
          }
          draw_point_index_->add(tmp_vtx[idx].index);

          poly_from_line_numverts = 1;

          // john .....
          if ( !poly_from_line ) {  // don't add this kid right now
            curr_part_ = new ssgLayeredVtxArray( GL_LINE_STRIP,
              vertex_array_,
              normal_array_,
              NULL,
              NULL,
              draw_point_index_ );

            if ( has_texture ) {
              curr_part_->setState( createState(true, &line_) );
            }
            else {
              curr_part_->setState( createState(false, &line_) );
            }
            ssgBranch *grp = getCurrGroup();
//            grp->addKid(curr_part_);
          grp->addKid( current_options -> createLeaf(curr_part_, NULL) );
          }
        }
        break;

      case 0x10:  // CNTRES: Continue poly line definition
        {
          unsigned short idx = ulEndianReadLittle16(fp);
          if (tmp_vtx[idx].index == -1) { // add vertex
            int ix = vertex_array_->getNum();
            vertex_array_->add(tmp_vtx[idx].point);
            normal_array_->add(tmp_vtx[idx].norm);
            tmp_vtx[idx].index = ix;
          }
          draw_point_index_->add(tmp_vtx[idx].index);
          poly_from_line_numverts++;
        }
        break;

      case 0x7a:  // Goraud shaded Texture-mapped ABCD Facet with night emission
        {
          curr_tex_wrap_ = (curr_tex_type_==0x18)?TRUE:FALSE;
          has_emission=true;
          readTMapFacet(fp, &goraud_);
          has_texture=false;
        }
        break;

      case 0x20:  // Goraud shaded Texture-mapped ABCD Facet
        {
          curr_tex_wrap_ = (curr_tex_type_==0x18)?TRUE:FALSE;
          has_emission=false;
          readTMapFacet(fp, &surface_);
          has_texture=false;
        }
        break;

      case 0x60:  // BGL_FACE_TMAP
        {
          curr_index_ = new ssgIndexArray();

          unsigned short numverts = ulEndianReadLittle16(fp);

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

          GLenum type = readTexIndices(fp, numverts, v, flip_y);

          curr_part_  = new ssgLayeredVtxArray( type,
            vertex_array_,
            normal_array_,
            tex_coords_,
            NULL,
            curr_index_ );
          
          curr_tex_wrap_ = (curr_tex_type_==0x18)?TRUE:FALSE;

          curr_part_->setState( createState(true, &goraud_) );
          ssgBranch* grp = getCurrGroup();
          ((ssgVtxArray *)curr_part_)->removeUnusedVertices();
					grp->addKid( current_options -> createLeaf(curr_part_, NULL) );
        }
        break;

      case 0x1d:  // BGL_FACE
        {
          curr_index_ = new ssgIndexArray();

          unsigned short numverts = ulEndianReadLittle16(fp);

          sgVec3 p;
          readPoint(fp, p);
          // Surface normal
          sgVec3 v;
          readVector(fp, v);

          // Read vertex indices
          GLenum type = readIndices(fp, numverts, v, has_texture);

          curr_part_ = new ssgLayeredVtxArray(type,
            vertex_array_,
            normal_array_,
            NULL,
            NULL,
            curr_index_);

          curr_tex_wrap_ = TRUE;
          curr_part_->setState( createState(false, &surface_) );
          ssgBranch* grp = getCurrGroup();
          ((ssgVtxArray *)curr_part_)->removeUnusedVertices();
					grp->addKid( current_options -> createLeaf(curr_part_, NULL) );
        }
        break;

      case 0x2a:  // Goraud shaded ABCD Facet
        {
          curr_tex_wrap_ = TRUE;
          has_emission = true;
//          has_emission = emissive_color_;
          has_texture = false;
          readFacet(fp, &goraud_, has_texture );
        }
        break;

      case 0x3e:  // FACETN
        {
          curr_tex_wrap_ = TRUE;
          has_emission = false;
          readFacet(fp, &surface_, has_texture );
          has_texture = false;
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
          setTexture(tex_filename, 0x18, 0x00);
          has_texture = true;
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
            ulSetError( UL_WARNING, "[ssgLoadBGL] TEXTURE2 Checksum != 0" );
          }

          char c;
          int i = 0;
          while((c = getc(fp)) != 0)
          {
            if(!isspace(c))
              tex_filename[i++] = tolower(c);
          }
          tex_filename[i] = '\0';

          // Padding byte
          if((strlen(tex_filename) + 1) % 2)
            c = getc(fp);

          JMPRINT( ios::hex, "Set texture2: name = " << tex_filename << ", length = 0x" << length
            << ", idx = 0x" << idx << ", flags = 0x" << (short)flags << ", color = 0x" << color);

          setTexture(tex_filename, 0x43, flags);
          has_texture = true;
          break;
        }

      case 0x50:  // GCOLOR (Goraud shaded color)
        {
          setColor(fp, &goraud_);
          has_texture = false;
        }
        break;

      case 0x51:  // LCOLOR (Line color)
        {
          setColor(fp, &line_);
        }
        break;

      case 0x52:  // SCOLOR (Light source shaded surface color)
        {
          setColor(fp, &surface_);
          has_texture = false;
        }
        break;

      case 0x2D:  // BGL_SCOLOR24
        {
          setTrueColor(fp, &surface_);
          has_texture = false;
        }
        break;

      case 0x2E:  // BGL_LCOLOR24
        {
          setTrueColor(fp, &line_);
        }
        break;

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

          if ( poly_from_line ) {  // closed surface

            sgVec3 v;
            sgSetVec3(v, 0.0f, 0.0f, 1.0f);

            if(has_texture == true) {
              sgVec2 tc;
              while (tex_coords_->getNum() < vertex_array_->getNum())
                tex_coords_->add(tc);


              sgVec3 s_norm;
              sgSetVec3(s_norm, 0.0f, 0.0f, 1.0f);
              JMPRINT( ios::dec|ios::scientific|ios::floatfield, "              norm[0]" << \
                       s_norm[0] << " norm[1]" << s_norm[1] << " norm[2]" << s_norm[2]);
              sgMat4 rot_norm;
              sgMakeRotMat4 ( rot_norm, s_norm );
              int i;
              for( i=0; i<poly_from_line_numverts; i++) {
                sgVec3 point;

                // rotate the point according to the normal vector
                // to ensure the correct texture coordinates
                // parallel projection to plane given by normal vector
                sgCopyVec3(point, vertex_array_->get(*draw_point_index_->get(i)));
                JMPRINT( ios::dec|ios::scientific|ios::floatfield, "Before rotation point[0]" << \
                 point[0] << " point[1]" << point[1] << " point[2]" << point[2]);
                sgXformPnt3 ( point, rot_norm ) ;
                JMPRINT( ios::dec|ios::scientific|ios::floatfield, "After rotation point[0]" << \
                         point[0] << " point[1]" << point[1] << " point[2]" << point[2]);

                tc[0] = point[0]*ref_scale/256.0f;  // texture coordinates are given in delta units
                tc[1] = point[1]*ref_scale/256.0f;  // texture coordinates are given in delta units
                sgCopyVec2(tex_coords_->get(*draw_point_index_->get(i)), tc);
              }
            }
            GLenum type = createTriangIndices(draw_point_index_, poly_from_line_numverts, v);

            while (normal_array_->getNum() < vertex_array_->getNum())
              normal_array_->add(v);

            curr_part_ = new ssgLayeredVtxArray(
              type,
              vertex_array_,
              normal_array_,
              has_texture==true?tex_coords_:NULL,
              NULL,
              curr_index_ );

            delete draw_point_index_;
            curr_part_->moveToBackground();
            shininess_ = 0;        // background does not shine
            curr_tex_wrap_ = TRUE;
            curr_part_->setState( createState(true, &surface_) );
            shininess_ = DEF_SHININESS;
          }
          else {                   // open surface draw a ployline
            curr_part_ = new ssgLayeredVtxArray(
              GL_LINES,
              vertex_array_,
              NULL,
              NULL,
              NULL,
              draw_point_index_ );
            curr_part_->setState( createState(false, &line_) );
          }
          ssgBranch* grp = getCurrGroup();
          ((ssgVtxArray *)curr_part_)->removeUnusedVertices();
					grp->addKid( current_options -> createLeaf(curr_part_, NULL) );

          poly_from_line = false;
        }
        break;

      case 0x81:  // OBSOLETE
        {
          ulEndianReadLittle16(fp);
        }
        break;

      case 0x32:   // Perspective
      case 0x2b:   // Perspective call 32
        {
          short offset;
          offset = ulEndianReadLittle16(fp);
          long addr = ftell(fp);

          // push current Branch
          ssgBranch* grp = getCurrGroup();
          push_stack(perspective_);
          push_stack(layer_);
          push_stack((long int)grp);
          push_stack(addr);
          long dst = addr + offset - 4;
          fseek(fp, dst, SEEK_SET);
        }
        break;

      case 0x3f: // BGL_SHADOW_CALL JM: do nothing
        {
          short offset;
          offset = ulEndianReadLittle16(fp);
        }
        break;

      case 0x3c:
        {
          JMPRINT(ios::dec, "Refpoint3c");

          sgVec3 v;
          // Create group nodes for textured and non-textured objects
          vertex_array_ = new ssgVertexArray();
          normal_array_ = new ssgNormalArray();
          tex_coords_ = new ssgTexCoordArray();

          ulEndianReadLittle16(fp);           // byte offset
          ulEndianReadLittle16(fp);           // visibility related
          ulEndianReadLittle16(fp);           // invisibility cone
          ulEndianReadLittle16(fp);           // ?
          readRefPointTranslation(fp,v);
          v[2] = (double) 0.0;                // set altitude to zero (for now)
          ref_scale = 1.0;                    // scaling is 1.0
          JMPRINT( ios::dec, " Scale:" << ref_scale << " Delta Latitude:" << v[0] << "m; Delta Longitude:" << v[1] << "m; Delta Altitude:" << v[2] << "m");

          // push current Branch
          curr_branch_ = new ssgTransform();
          curr_branch_->setName("Refpoint3c");
          if (curr_branch_ !=0)
             curr_branch_->setTransform(v);
          getCurrBranch()->addKid(curr_branch_);
        }
        break;

      case 0x77:
        {
          JMPRINT(ios::dec, "Refpoint77");
          long scale;
          sgVec3 v;

          // Create group nodes for textured and non-textured objects
          vertex_array_ = new ssgVertexArray();
          normal_array_ = new ssgNormalArray();
          tex_coords_ = new ssgTexCoordArray();

          ulEndianReadLittle16(fp);           // byte offset
          ulEndianReadLittle16(fp);           // visibility related
          ulEndianReadLittle16(fp);           // invisibility cone
          ulEndianReadLittle16(fp);           // ?
          scale = ulEndianReadLittle32(fp);   // get scale
          ref_scale = (double)scale /65536.0;
          readRefPointTranslation(fp,v);
          v[2] = (double) 0.0;                // altitude is zero
          JMPRINT( ios::dec, " Scale:" << ref_scale << " Delta Latitude:" << v[0] << "m; Delta Longitude:" << v[1] << "m; Delta Altitude:" << v[2] << "m");

          // push current Branch
          curr_branch_ = new ssgTransform();
          curr_branch_->setName("Refpoint & Scale");
          if (curr_branch_ !=0)
             curr_branch_->setTransform(v);
          getCurrBranch()->addKid(curr_branch_);
        }
        break;

      case 0x2f: // BGL_SCALE john:do nothing write now
        {
          JMPRINT(ios::dec, "Refpoint2f");
          long scale;
          sgVec3 v;

          // Create group nodes for textured and non-textured objects
          vertex_array_ = new ssgVertexArray();
          normal_array_ = new ssgNormalArray();
          tex_coords_ = new ssgTexCoordArray();

          ulEndianReadLittle16(fp);           // byte offset
          ulEndianReadLittle16(fp);           // visibility related
          ulEndianReadLittle16(fp);           // invisibility cone
          ulEndianReadLittle16(fp);           // ?
          scale = ulEndianReadLittle32(fp);   // get scale
          ref_scale = (double)scale /65536.0;
          readRefPointTranslation(fp,v);
          v[2] = (double) 0.0;                // set altitude to zero for now
          JMPRINT( ios::dec, " Scale:" << ref_scale << " Delta Latitude:" << v[0] << "m; Delta Longitude:" << v[1] << "m; Delta Altitude:" << v[2] << "m");

          // push current Branch
          curr_branch_ = new ssgTransform();
          curr_branch_->setName("Refpoint2f");
          if (curr_branch_ !=0)
             curr_branch_->setTransform(v);
          getCurrBranch()->addKid(curr_branch_);
        }
        break;

      case 0x33:  // BGL_INSTANCE
        {
          short offset;
          offset = ulEndianReadLittle16(fp);
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
          sgMakeRotMat4( rot_mat, -ry+360.0, -rz, -rx );
          
          // push current Branch
          ssgBranch* grp = getCurrGroup();
          curr_branch_ = new ssgTransform();
          curr_branch_->setName("RotoCall");
          curr_branch_->setTransform(rot_mat);
          grp->addKid(curr_branch_);
          push_stack(perspective_);
          push_stack(layer_);
          push_stack((long int)grp);
          push_stack(addr+6);  //john: for now I haven't parse the whole command
          long dst = addr + offset - 4;
          fseek(fp, dst, SEEK_SET);
        }
        break;


#define ROOF 0x8000
#define FLOOR 0x4000
    case 0x49: {  // BGL_BUILDING

      static char *textures[8] = { {"sandstone.rgb"},
                                   {"white.rgb"},
                                   {"black.rgb"},
                                   {"darkgray.rgb"},
                                   {"gray.rgb"},
                                   {"whitehorizon.rgb"},
                                   {"sandstonehorizon.rgb"},
                                   {"gray.rgb"} };

      static sgVec3 block_points[8] = { { -.5, -.5, 0 },
                                        {  .5, -.5, 0 },
                                        {  .5,  .5, 0 },
                                        { -.5,  .5, 0 },
                                        { -.5, -.5, 1 },
                                        {  .5, -.5, 1 },
                                        {  .5,  .5, 1 },
                                        { -.5,  .5, 1 } };

      static int block_faces[] = { {4|FLOOR}, {3}, {2}, {1}, {0},
                                         {4}, {0}, {1}, {5}, {4},
                                         {4}, {1}, {2}, {6}, {5},
                                         {4}, {2}, {3}, {7}, {6},
                                         {4}, {3}, {0}, {4}, {7},
                                    {4|ROOF}, {4}, {5}, {6}, {7} };

      static sgVec3 iroof_points[8] = { { -.5, -.5, 0 },
                                        {  .5, -.5, 0 },
                                        {  .5,  .5, 0 },
                                        { -.5,  .5, 0 },
                                        { -.5, -.5, .75 },
                                        {  .5, -.5, .75 },
                                        {  .5,  .5, 1 },
                                        { -.5,  .5, 1 } };
      
      static sgVec3 pyramid_points[8]={ { -.5, -.5, 0 },
                                        {  .5, -.5, 0 },
                                        {  .5,  .5, 0 },
                                        { -.5,  .5, 0 },
                                        { -.3, -.3, 1 },
                                        {  .3, -.3, 1 },
                                        {  .3,  .3, 1 },
                                        { -.3,  .3, 1 } };
      
      static sgVec3 oct_points[16] =  { { -.25, -.5, 0 },
                                        {  .25, -.5, 0 },
                                        {  .5, -.25, 0 },
                                        {  .5,  .25, 0 },
                                        {  .25,  .5, 0 },
                                        { -.25,  .5, 0 },
                                        { -.5,  .25, 0 },
                                        { -.5, -.25, 0 },
                                        { -.25, -.5, 1 },
                                        {  .25, -.5, 1 },
                                        {  .5, -.25, 1 },
                                        {  .5,  .25, 1 },
                                        {  .25,  .5, 1 },
                                        { -.25,  .5, 1 },
                                        { -.5,  .25, 1 },
                                        { -.5, -.25, 1 } };

      static int oct_faces[] =  { {8|FLOOR}, {7}, {6}, {5},  {4}, {3}, {2}, {1}, {0},
                                        {4}, {0}, {1}, {9},  {8},
                                        {4}, {1}, {2}, {10}, {9},
                                        {4}, {2}, {3}, {11}, {10},
                                        {4}, {3}, {4}, {12}, {11},
                                        {4}, {4}, {5}, {13}, {12},
                                        {4}, {5}, {6}, {14}, {13},
                                        {4}, {6}, {7}, {15}, {14},
                                        {4}, {7}, {0}, {8},  {15},
                                   {8|ROOF}, {8}, {9}, {10}, {11}, {12}, {13}, {14}, {15} };
      struct {
         int    face_nr;
         int    *face;
         sgVec3 *point;
         sgVec3 *tcoord;
         } build_type[] = { { 6, block_faces, block_points, block_points },
                            { 6, block_faces, iroof_points, iroof_points },
                            { 6, block_faces, pyramid_points, pyramid_points },
                            { 10, oct_faces, oct_points, oct_points }
                             };

      int info, type;
      double stories;
      sgVec3 offset, scale;
      ssgSimpleState *state = new ssgSimpleState();
//      building = new ssgTransform ;
      ssgBranch* building = getCurrGroup();

      info = ulEndianReadLittle16(fp); //info
      ulEndianReadLittle16(fp);        //codes
      offset[2] = (ulEndianReadLittle16(fp) * ref_scale) ;   //offset x
      offset[1] = (ulEndianReadLittle16(fp) * ref_scale) ;   //offset y
      offset[0] = (ulEndianReadLittle16(fp) * ref_scale) ;   //offset z
      stories = ulEndianReadLittle16(fp) * ref_scale;   //stories
      scale[1] = ulEndianReadLittle16(fp) * ref_scale;  //size_x
      scale[0] = ulEndianReadLittle16(fp) * ref_scale;  //size_z
      scale[2] = stories * 4;

      state->setShininess(DEF_SHININESS);
      state->setShadeModel(GL_SMOOTH);

      state->disable(GL_BLEND);
      state->disable(GL_ALPHA_TEST);

      state->enable   (GL_LIGHTING);
      state->enable   (GL_CULL_FACE);
      state->disable  (GL_COLOR_MATERIAL);

      state->setMaterial( GL_DIFFUSE, 1.0f, 1.0f, 1.0f, 0.0f);
      state->setMaterial( GL_AMBIENT, 1.0f, 1.0f, 1.0f, 0.0f);
      state->setMaterial( GL_SPECULAR, 1.0f, 1.0f, 1.0f, 0.0f );
      state->setMaterial( GL_EMISSION, 0.0f, 0.0f, 0.0f, 1.0f );
      state->enable     ( GL_TEXTURE_2D ) ;

      if ((info & 0x8000) == 0x8000) {
        state->setTexture(curr_tex_name_);
      }
      else {
        state->setTexture(textures[info&7]);
      }

      type = (info >> 3) & 3;

      int i,j;
      sgVec3 norm;
      sgVec3 vert;
      float toffset = 0.0f;
      float ntoffset = 0.0f;
      for (i=0, j=0; i < build_type[type].face_nr; i++) {
        int n = build_type[type].face[j++];
        ssgVertexArray *vertices = new ssgVertexArray(4);
        ssgNormalArray *normals  = new ssgNormalArray(4);
        ssgTexCoordArray *texcoords = NULL;

        if ( (n & (ROOF|FLOOR)) == 0 ) {
          texcoords = new ssgTexCoordArray(4);
        }

        // normal for wall
        sgMakeNormal(norm, build_type[type].point[build_type[type].face[j+0]],
                           build_type[type].point[build_type[type].face[j+1]],
                           build_type[type].point[build_type[type].face[j+2]]);

        sgVec3 x,y,p;
        sgSubVec3(x, build_type[type].point[build_type[type].face[j+1]],
                     build_type[type].point[build_type[type].face[j+0]]);
        sgNormalizeVec3(x);
        sgVectorProductVec3(y, norm, x );
        sgCopyVec3(p, build_type[type].point[build_type[type].face[j+0]]);
        JMPRINT(ios::dec, "x0:" << x[0] << " x1:" << x[1] << " x2:" << x[2]);
        JMPRINT(ios::dec, "y0:" << y[0] << " y1:" << y[1] << " y2:" << y[2]);
        JMPRINT(ios::dec, "norm0:" << norm[0] << " norm1:" << norm[1] << " norm2:" << norm[2]);
        int k;
        for (k=1; k <= (n&0x3fff); k++, j++ ) {
          // add normal
          normals->add(norm);
          // add vertice
          sgCopyVec3(vert, build_type[type].point[build_type[type].face[j]]);
          vert[0]*=scale[0]; vert[1]*=scale[1]; vert[2]*=scale[2];
          sgAddVec3(vert, offset);
          vertices->add(vert);
          // add texture coordinate
          if (texcoords != NULL) {
            sgVec2 tc;
            sgCopyVec3(vert, build_type[type].tcoord[build_type[type].face[j]]);
            sgSubVec3(vert,p);
            vert[0]*=scale[0]; vert[1]*=scale[1]; vert[2]*=scale[2];
            JMPRINT(ios::dec, "vert0:" << vert[0] << " vert1:" << vert[1] << " vert2:" << vert[2]);

            tc[0] = sgScalarProductVec3(x, vert)*ref_scale/256.0f;  // texture coordinates are given in delta units
            if (k == 2) ntoffset = tc[0];
            tc[0] += toffset;
            tc[1] = sgScalarProductVec3(y, vert)*ref_scale/256.0f;  // texture coordinates are given in delta units
            texcoords->add(tc);
            JMPRINT(ios::dec, "tc0:" << tc[0] << " tc1:" << tc[1]);
          }
        }
        ssgVtxTable *Vtx = new ssgVtxTable(GL_TRIANGLE_FAN, vertices, normals, texcoords, NULL);
        Vtx->setState(state);
//        building -> addKid (Vtx);
        building->addKid( current_options -> createLeaf(Vtx, NULL) );
        toffset += ntoffset;
      }
    }
    break;

      case 0x15: {  //elevation map
        int ny = ulEndianReadLittle16(fp)+1;                     // number of points in x dir
        int nx = ulEndianReadLittle16(fp)+1;                     // number of points in y dir
        double wy = ulEndianReadLittle16(fp) / ref_scale;        // width in x dir
        double wx = -ulEndianReadLittle16(fp) / ref_scale;       // width in y dir
        double dy = (short)ulEndianReadLittle16(fp) / ref_scale; // offset in x dir
        double dx = -(short)ulEndianReadLittle16(fp) / ref_scale;// offset in y dir
        ulEndianReadLittle16(fp);                                // minalt
        ulEndianReadLittle16(fp);                                // maxalt
        JMPRINT(ios::dec,"nx: " << nx << " ny:" << ny <<
                         "wx: " << wx << " wy:" << wy <<
                         "dx: " << dx << " dy:" << dy );

        int offset = vertex_array_->getNum();

        // create Vertices
        int x,y;
        double z;
        sgVec3 v;
        sgVec2 tc;
        for (x = 0; x < nx; x++) {
          for (y = 0; y < ny; y++) {
            tc[0] = (float)getc(fp)/255.0f; // Texture coordinate X
            tc[1] = (float)getc(fp)/255.0f; // Texture coordinate Y
            z = ulEndianReadLittle16(fp) / ref_scale;  // altitude
//            JMPRINT(ios::dec,"z: " << z << " tx:" << tc[0] << " ty:" << tc[1] );
            v[0]=dx+x*wx;                   // x coordinate
            v[1]=dy+y*wy;                   // y coordinate
            v[2]=z;                         // z coordinate
            vertex_array_->add(v);
            tex_coords_->add(tc);
          }
        }
        // create normals
        int short stat = 0;
        sgVec3 p0, p1, p2, cross, norm;
        int o[7] = {0,-1,0,1,0,-1,0};
        for (x = 0; x < nx; x++) {
          stat&=0x0c;
          stat|=((x==0)?1:0);
          stat|=((x==nx-1)?2:0);
          JMPRINT(ios::hex,"stat:" << stat);
          for (y = 0; y < ny; y++) {
            stat&=0x03;
            stat|=((y==0)?4:0);
            stat|=((y==ny-1)?8:0);
            sgCopyVec3( p0, vertex_array_->get(offset+x*ny+y));
            switch (stat) {

            case 0: // 4 connecting points
            {
              sgZeroVec3(norm);
              sgCopyVec3( p1, vertex_array_->get(offset+(x+1)*ny+y));
              int i;
              for(i = 0; i < 4 ; i++) {
                sgCopyVec3( p2, vertex_array_->get(offset+(x+o[i])*ny+y+o[i+1]));
                sgMakeNormal(cross, p0, p1, p2);
                sgAddVec3(norm, cross);
                sgCopyVec3( p1, p2);
              }
            }
            break;
            case 5: // x == 0 && y == 0
            {
              sgCopyVec3( p1, vertex_array_->get(offset+(x)*ny+y+1));
              sgCopyVec3( p2, vertex_array_->get(offset+(x+1)*ny+y));
              sgMakeNormal(norm, p0, p1, p2);
            }
            break;
            case 9: // x == 0 && y == max
            {
              sgCopyVec3( p1, vertex_array_->get(offset+(x+1)*ny+y));
              sgCopyVec3( p2, vertex_array_->get(offset+(x)*ny+y-1));
              sgMakeNormal(norm, p0, p1, p2);
            }
            break;
            case 10: // x == max && y == max
            {
              sgCopyVec3( p1, vertex_array_->get(offset+(x)*ny+y-1));
              sgCopyVec3( p2, vertex_array_->get(offset+(x-1)*ny+y));
              sgMakeNormal(norm, p0, p1, p2);
            }
            break;
            case 6: // x == max && y == 0
            {
              sgCopyVec3( p1, vertex_array_->get(offset+(x-1)*ny+y));
              sgCopyVec3( p2, vertex_array_->get(offset+(x)*ny+y+1));
              sgMakeNormal(norm, p0, p1, p2);
            }
            break;
            case 1: // x==0
            {
              sgZeroVec3(norm);
              sgCopyVec3( p1, vertex_array_->get(offset+(x)*ny+y+1));
              int i;
              for(i = 3; i < 5 ; i++) {
                sgCopyVec3( p2, vertex_array_->get(offset+(x+o[i])*ny+y+o[i+1]));
                sgMakeNormal(cross, p0, p1, p2);
                sgAddVec3(norm, cross);
                sgCopyVec3( p1, p2);
              }
            }
            break;
            case 2: // x==max
            {
              sgZeroVec3(norm);
              sgCopyVec3( p1, vertex_array_->get(offset+(x)*ny+y-1));
              int i;
              for(i = 1; i < 3 ; i++) {
                sgCopyVec3( p2, vertex_array_->get(offset+(x+o[i])*ny+y+o[i+1]));
                sgMakeNormal(cross, p0, p1, p2);
                sgAddVec3(norm, cross);
                sgCopyVec3( p1, p2);
              }
            }
            break;
            case 4: // y==0
            {
              sgZeroVec3(norm);
              sgCopyVec3( p1, vertex_array_->get(offset+(x-1)*ny+y));
              int i;
              for(i = 2; i < 4 ; i++) {
                sgCopyVec3( p2, vertex_array_->get(offset+(x+o[i])*ny+y+o[i+1]));
                sgMakeNormal(cross, p0, p1, p2);
                sgAddVec3(norm, cross);
                sgCopyVec3( p1, p2);
              }
            }
            break;
            case 8: // y==max
            {
              sgZeroVec3(norm);
              sgCopyVec3( p1, vertex_array_->get(offset+(x+1)*ny+y));
              int i;
              for(i = 4; i < 6 ; i++) {
                sgCopyVec3( p2, vertex_array_->get(offset+(x+o[i])*ny+y+o[i+1]));
                sgMakeNormal(cross, p0, p1, p2);
                sgAddVec3(norm, cross);
                sgCopyVec3( p1, p2);
              }
            }
            break;

            break;
            }
            sgNormaliseVec3(norm);
            normal_array_->add(norm);
//            JMPRINT(ios::dec, " Normal: " << norm[0] << "; " << norm[1] << "; " << norm[2]);
          }
        }
        // create faces & leafs
        for ( x = 0; x < nx-1; x++ ) {
          for ( y = 0; y < ny-1; y++ ) {
            ssgIndexArray *strips = new ssgIndexArray(4);
            strips->add(offset+x*ny+y);	           // 1st point
            strips->add(offset+x*ny+y+1);          // 2nd point
            strips->add(offset+(x+1)*ny+y+1);      // 3rd point
            strips->add(offset+(x+1)*ny+y);        // 4th point
            curr_part_ = new ssgLayeredVtxArray( GL_TRIANGLE_FAN,
                                          vertex_array_,
                                          normal_array_,
                                          tex_coords_,
                                          NULL,
                                          strips );
            curr_part_->setState( createState(true, &surface_) );
            ssgBranch* grp = getCurrGroup();
            ((ssgVtxArray *)curr_part_)->removeUnusedVertices();
						grp->addKid( current_options -> createLeaf(curr_part_, NULL) );
          }
        }
      }
      break;

      case 0x8f: { // alpha color
        int alpha = ulEndianReadLittle32(fp);
        if ( alpha == 0) {
          alpha_ = 1.0;
        }
        else if (alpha < 16) {
          alpha_ = (double)alpha/15.0;
        }
        else {
             ulSetError( UL_WARNING, "[ssgLoadBGL] Alpha Color(x8f) unknown value: %x", alpha);
        }
      }
      break;

      case 0xa0: { // new building
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

      case 0xaa: { // New Runaway
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
              JMPRINT(ios::dec, "Unsupported opcode '" << opcodes[opcode].name << \
                                "' (size " << opcodes[opcode].size << "); skipped" );
              skip_offset = opcodes[opcode].size - 2; // opcode already read
            }
            else
            {
             ulSetError( UL_WARNING, "[ssgLoadBGL] Unhandled opcode '%s' (%dx)", \
                                     opcodes[opcode].name, opcode );
            }
          }
          else
          {
            ulSetError( UL_FATAL, "[ssgLoadBGL] Op-code out of range: %dx", opcode );
          }
        }
        break;

      } // end of bgl parse switch 
       
      if (skip_offset > 0) 
        fseek( fp, skip_offset, SEEK_CUR );

    } // end of bgl parse while loop

  } // end of bgl "object header opcode" while loop
}

#else


ssgEntity *ssgLoadBGL(const char *fname, const ssgLoaderOptions *options)
{
  return NULL ;
}


#endif

