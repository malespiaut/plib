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
//           - support for transparent and transluctent textures added
//           - support for lines (06,07) added
//           - partial support for 0xa0 and 0x70 added, actally these are ignored
//             at the momnet but at least they are paresed correctly ;)
//           - ssgLoadMDLTexture removed from here. It's now a seperate file.
//           - FACET may now contain textures. Usually used for the ground areas
//           - Fixed normal calculation for concave polygons; culfacing is
//             working now for them
//           - transluctant textures are now drawn in the correct order
//           - clean ups: + DEBUGPRINT removed
//                        + use <iostream> / stl only when JMDEBUG is defined
//                        + M_PI replaced by SGD_PI
//                        + Major clean up for MSVC
//           - Fixed a bug in line drawing
//           - handle PointTo, DrawTo, start/end surface combinations correctly
//           - support for light maps added
//           - support for concave polygons added
//           - Fixed a bug for object positioning with negative longitude
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

#include "ssgLoadMDL.h"

#define DEF_SHININESS 50

//#define JMDEBUG
//#define EXPERIMENTAL_CULL_FACE_CODE

#ifdef JMDEBUG
#include <iostream>
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
#define MAX_PATH_LENGTH 1024

#undef ABS
#undef MIN
#undef MAX
#define ABS(x) ((x) >= 0 ? (x) : -(x))
#define MIN(a,b) ((a) <= (b) ? (a) : (b))
#define MAX(a,b) ((a) >= (b) ? (a) : (b))
#define MIN3(a,b,c) ((a) <= (b) ? MIN(a,c) : MIN(b,c))
#define MAX3(a,b,c) ((a) >= (b) ? MAX(a,c) : MAX(b,c))

static ssgLoaderOptions         *current_options;

// Temporary vertex arrays
static ssgIndexArray            *curr_index_;
static ssgIndexArray            *draw_point_index_;

// Vertex arrays
static ssgVertexArray           *vertex_array_;
static ssgNormalArray           *normal_array_;
static ssgTexCoordArray         *tex_coords_;

// Current part (index array)
static ssgLeaf                  *curr_part_;
static ssgBranch                *model_;
static ssgTransform             *curr_branch_;

// Moving parts
static ssgBranch                *ailerons_grp_, *elevator_grp_, *rudder_grp_;
static ssgBranch                *gear_grp_, *spoilers_grp_, *flaps_grp_;
static ssgBranch                *prop_grp_;

static sgMat4                   curr_matrix_;
static sgVec3                   curr_rot_pt_;
static sgVec4                   curr_col_;
static sgVec4                   curr_emission_;
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

// john ....
static bool                     poly_from_line;
static unsigned short           poly_from_line_numverts;

//john ........
static ssgTransform             *building = NULL;

static long                     object_end;
static long                     object_end_offset;

static int                      building_count = 0; // temp test


// jm's variables
static long                     scenery_center_lat;
static long                     scenery_center_lon;
static bool                     has_color;
static bool                     has_texture;
static bool                     has_emission;

static double                   ref_scale;
static short                    haze_;
static char                     tex_filename[128];
static struct {
  sgVec3 point;
  sgVec3 norm;
  int    index;
  }                             tmp_vtx[400];
static bool                     blending;

// This struct contains variable definitions of MS Flightsimulator
// upt to now only season, complexity and day time are supported
// unfortunately we have to set these varibales befor loading the bgl file
// thus we can't chnage them on the fly
static struct {
  int var;
  int val;
  }                             vardef[100] = { {0x346,4},      // complexity: 0 lowest; 4 most
                                                {0x6f8,2},      // season: 0=winter; 1=spring;
                                                                //         2=summer; 3=autumn;
                                                {0x28c,0x06},   // Day time: 0=Day, 1=Dusk,
                                                                //           2=Night, bit2=light on/off
                                                {0x000,0}       // END of table
                                              };


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

/*static void initLoader()
{
  join_children_    = true;
  override_normals_ = true;
  tex_fmt_          = "tif";
  stack_depth_      = 0;
#ifdef EXPERIMENTAL_CULL_FACE_CODE
  curr_cull_face_   = false ;
#endif
}*/

//===========================================================================

static void newPart()
{
  has_color = false;
  has_texture = false;

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
    if ( ix0 >= vertex_array_->getNum() ||
      ix1 >= vertex_array_->getNum() ||
      ix2 >= vertex_array_->getNum() ) {
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
    sgVec3 v0, v1;
    sgVec3 p0, p1, p2;
    sgCopyVec3( p0, vertex_array_->get(*ixarr->get(numverts-2)));
    sgCopyVec3( p1, vertex_array_->get(*ixarr->get(numverts-1)));
    v0[0] = p1[0]-p0[0];
    v0[1] = p1[1]-p0[1];
    v0[2] = p1[2]-p0[2];

    int i;
    for(i = 0; i < numverts; i++)
    {
      sgVec3 cross;
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
      numverts *= 3;
      idx_array = strips;
      ret = GL_TRIANGLES;
    }
    else {
      idx_array = ixarr;
      ret = GL_TRIANGLE_FAN;
    }

    // Ensure counter-clockwise ordering
    // and write to curr_index_
    bool flip = (sgScalarProductVec3(poly_dir, s_norm) < 0.0);

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

static GLenum readTexIndices(FILE *fp, int numverts, const sgVec3 s_norm, bool flip_y)
{
  if(numverts <= 0)
    return GL_FALSE;

  ssgIndexArray *curr_index_ = new ssgIndexArray();

  if(tex_coords_->getNum() < vertex_array_->getNum())
  {
    sgVec2 dummy_pt;
    sgSetVec2(dummy_pt, FLT_MAX, FLT_MAX);
    for(int i = tex_coords_->getNum(); i < vertex_array_->getNum(); i++)
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
      normal_array_->add(tmp_vtx[ix].norm);

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

static GLenum readIndices(FILE* fp, int numverts, const sgVec3 s_norm)
{
  if(numverts <= 0)
    return GL_FALSE;

  ssgIndexArray *curr_index_ = new ssgIndexArray();

  if (has_texture == true) {
  // add dummy texture coordinates if necessairy
    if(tex_coords_->getNum() < vertex_array_->getNum())
    {
      sgVec2 dummy_pt;
      sgSetVec2(dummy_pt, FLT_MAX, FLT_MAX);
      for(int i = tex_coords_->getNum(); i < vertex_array_->getNum(); i++)
        tex_coords_->add(dummy_pt);
    }
  }

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
      if (has_texture == true) {
        sgVec2 tc;
        sgSetVec2(tc, FLT_MAX, FLT_MAX);
        tex_coords_->add(tc);
      }
    }

    int tex_idx = tmp_vtx[ix].index;
    if (has_texture == true) {
      sgVec2 tc;
      sgVec3 point;
      sgMat4 rot_norm;
      
      // rotate the point according to the normal vector
      // to ensure the correct texture coordinates
      // parallel projection to plane spaned by normal vector
      sgCopyVec3(point, tmp_vtx[ix].point);
      JMPRINT( ios::dec|ios::scientific|ios::floatfield, "Before rotation point[0]" << \
               point[0] << " point[1]" << point[1] << " point[2]" << point[2]);
      JMPRINT( ios::dec|ios::scientific|ios::floatfield, "              norm[0]" << \
               s_norm[0] << " norm[1]" << s_norm[1] << " norm[2]" << s_norm[2]);
      sgMakeRotMat4 ( rot_norm, s_norm );
      sgXformPnt3 ( point, rot_norm ) ;
      JMPRINT( ios::dec|ios::scientific|ios::floatfield, "After rotation point[0]" << \
               point[0] << " point[1]" << point[1] << " point[2]" << point[2]);

      tc[0] = point[0];
      tc[1] = point[1];

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
        normal_array_->add(tmp_vtx[ix].norm);

        tex_coords_->add(tc);

      }

    }
    curr_index_->add(tex_idx);
    JMPRINT( ios::dec, "ix:" << ix << "idx:" << tmp_vtx[ix].index);
  }

  return(createTriangIndices(curr_index_, numverts, s_norm));
}

//===========================================================================

static void setEmission(int color, int pal_id)
{
  if(pal_id == 0x68)
  {
    curr_emission_[0] = fsAltPalette[color].r / 255.0f;
    curr_emission_[1] = fsAltPalette[color].g / 255.0f;
    curr_emission_[2] = fsAltPalette[color].b / 255.0f;
    curr_emission_[3] = 0.0f;
  }
  else
  {

    curr_emission_[0] = fsAcPalette[color].r / 255.0f;
    curr_emission_[1] = fsAcPalette[color].g / 255.0f;
    curr_emission_[2] = fsAcPalette[color].b / 255.0f;
    curr_emission_[3] = 0.0f;
  }
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
  has_texture = false;
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
               curr_col_[3] = 1.0f;
               blending = false;
               JMPRINT( ios::hex, "HINT: Unknown color Attribut: " << (short)attr );
             } break;

  }
  has_texture = false;
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
  JMPRINT(ios::dec,"new State: col = " << curr_col_[0] << ", " \
                                       << curr_col_[1] << ", " \
                                       << curr_col_[2] << ", " \
                                       << curr_col_[3]);

  ssgSimpleState *state = new ssgSimpleState();

  state->setShininess(DEF_SHININESS);
  state->setShadeModel(GL_SMOOTH);

  state->enable   (GL_LIGHTING);
  state->enable   (GL_CULL_FACE);
  state->disable  (GL_COLOR_MATERIAL);
  
  if(curr_tex_name_ != NULL && use_texture)
  {
    // Handle light maps.
    // For the moment light maps are applied only when the light flag is selected in
    // variable (0x28c). If so we check whether there an equivalent light map  exists
    // <texture name>+"_lm"+<extension>
    int day_time;
    bool enable_emission = false;
    char tex_name[MAX_PATH_LENGTH];
    strcpy(tex_name, curr_tex_name_);
    if(getVariableValue(0x28c, &day_time) == 0 ){  // get daytime
      if ((day_time&0x04) == 0x04) {               // light enabled
        char *p =strrchr(tex_name,'.');
        if ( p != NULL ) {
          char tname[MAX_PATH_LENGTH];
          *p = '\0';
          strcat(tex_name, "_lm");
          strcat(tex_name, strrchr(curr_tex_name_,'.'));
          current_options->makeTexturePath(tname, tex_name);
          if ( ulFileExists ( tname ) == true) {    // look if light map exists
            enable_emission = true;
          }
          else {
            strcpy(tex_name, curr_tex_name_);
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
    ssgTexture* tex = current_options ->createTexture(tname, TRUE, TRUE) ;
    state->setTexture( tex ) ;
    state->setOpaque();
    if ( ( (curr_tex_type_ == 0x18)&&(haze_ > 0) ) ||
             ( blending == true ) || 
         ( (curr_tex_type_ == 0x43)&&(tex->hasAlpha() == true) ) ){
      state->enable(GL_BLEND);
      state->enable(GL_ALPHA_TEST);
      state->setAlphaClamp(.2) ;
      state->setTranslucent() ;
    } else {
      state->disable(GL_BLEND);
      state->disable(GL_ALPHA_TEST);
    }
    state->setMaterial( GL_AMBIENT, 1.0f, 1.0f, 1.0f, curr_col_[3]);
    state->setMaterial( GL_DIFFUSE, 1.0f, 1.0f, 1.0f, curr_col_[3]);
    if (enable_emission == true) {
      state->setMaterial( GL_EMISSION, 1.0f, 1.0f, 1.0f, 0.0f );
    }
    else {
      if (has_emission == true) {
        state->setMaterial( GL_EMISSION, curr_emission_ );
      }
      else {
        state->setMaterial( GL_EMISSION, 0.0f, 0.0f, 0.0f, 1.0f );
      }
    }
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
    if (has_emission == true) {
      int day_time;
      if(getVariableValue(0x28c, &day_time) == 0 ){  // get daytime
        if ((day_time&0x04) == 0x04) {               // light enabled
          state->setMaterial( GL_EMISSION, curr_emission_ );
        }
        else {
          state->setMaterial( GL_AMBIENT, curr_emission_ );
          state->setMaterial( GL_DIFFUSE, curr_emission_ );
          state->setMaterial( GL_EMISSION, 0.0f, 0.0f, 0.0f, 1.0f  );
        }
      }
    }
    else {
      state->setMaterial( GL_AMBIENT, curr_col_ );
      state->setMaterial( GL_DIFFUSE, curr_col_ );
      state->setMaterial( GL_EMISSION, 0.0f, 0.0f, 0.0f, 1.0f );
    }
    state->disable(GL_TEXTURE_2D);
  }

  state->setMaterial( GL_SPECULAR, 1.0f, 1.0f, 1.0f, curr_col_[3] );
  state->enable   (GL_CULL_FACE);
  has_emission = false;
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

  sgSetVec4(curr_emission_, 1.0f, 1.0f, 1.0f, 0.0f);      // set Emission value to default

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
    ulSetError( UL_WARNING, "[ssgLoadBGL] Couldn't open MDL file '%s'!",
      filename );
    return NULL;
  }
  
  // Find beginning of BGL Code segment
  if(feof(fp))
  {
    ulSetError( UL_WARNING, "[ssgLoadBGL] No BGL Code found in file '%s'!",
      filename );
    return NULL;
  }
  
  // Initialize object graph
  glDepthFunc(GL_LEQUAL);
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
      ulSetError ( UL_DEBUG, "[ssgLoadBGL] Reference Lat: %f; Reference Lon: %f \n", object_position_lat*90.0/10001750.0, object_position_lon*90.0/(256.0* 4194304.0)) ;
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
        int stat; 
        bool s = false;
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
        sgMakeIdentMat4( curr_matrix_ );
        sgZeroVec3( curr_rot_pt_ );
        if(stack_depth_ == 0) {
//        done = true;
        }
        else
        {
          long addr = pop_stack();
          curr_branch_ = (ssgTransform *)pop_stack();
          fseek(fp, addr, SEEK_SET);
        }
      }
      break;

    case 0x1a:  // RESLIST (point list with no normals)
      {
        newPart();
        has_normals_ = false;

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
        has_normals_ = true;

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
        readPoint(fp, tmp_vtx[0].point);
        tmp_vtx[0].index=-1; // mark as dirty
       }
       break;
    case 0x07:  // STRRES: Ends line definition
      {
        readPoint(fp, tmp_vtx[1].point);
        tmp_vtx[1].index=-1; // mark as dirty
        if (tmp_vtx[0].index == -1) { // add vertex
          int ix = vertex_array_->getNum();
          curr_index_ = new ssgIndexArray();
          vertex_array_->add(tmp_vtx[0].point);
          vertex_array_->add(tmp_vtx[1].point);
          curr_index_->add(ix);
          curr_index_->add(ix+1);
          curr_part_ = new ssgVtxArray( GL_LINES,
            vertex_array_,
            NULL,
            NULL,
            NULL,
            curr_index_ );
          curr_part_->setState( createState(false) );
          ssgBranch* grp = getCurrGroup();
          grp->addKid( current_options -> createLeaf(curr_part_, NULL) );
        }
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

          curr_part_ = new ssgVtxArray( GL_LINES,
            vertex_array_,
            normal_array_,
            NULL,
            NULL,
            draw_point_index_ );

          if ( has_texture ) {
            curr_part_->setState( createState(true) );
          }
          else {
            curr_part_->setState( createState(false) );
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
                has_emission=true;
    case 0x20:  // Goraud shaded Texture-mapped ABCD Facet
      {
        curr_index_ = new ssgIndexArray();
        
        unsigned short numverts = ulEndianReadLittle16(fp);

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

        GLenum type = readTexIndices(fp, numverts, v, flip_y);

        if(!has_normals_)
        {
          while (normal_array_->getNum() < vertex_array_->getNum())
            normal_array_->add(v);
          recalcNormals();
        }

        curr_part_ = new ssgVtxArray( type, 
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

        if(!has_normals_)
        {
          while (normal_array_->getNum() < vertex_array_->getNum())
            normal_array_->add(v);
          recalcNormals();
        }

        curr_part_  = new ssgVtxArray( type,
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

        sgVec3 p;
        readPoint(fp, p);
        // Surface normal
        sgVec3 v;
        readVector(fp, v);

        // Read vertex indices
        GLenum type = readIndices(fp, numverts, v);

        if(!has_normals_)
        {
          while (normal_array_->getNum() < vertex_array_->getNum())
            normal_array_->add(v);
          recalcNormals();
        }

        curr_part_ = new ssgVtxArray(type,
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

    case 0x2a:  // Goraud shaded ABCD Facet
                has_emission=true;
    case 0x3e:  // FACETN (no texture)
      {
        curr_index_ = new ssgIndexArray();
        unsigned short numverts = ulEndianReadLittle16(fp);

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
        GLenum type = readIndices(fp, numverts, v);

        if(!has_normals_)
        {
          while (normal_array_->getNum() < vertex_array_->getNum())
            normal_array_->add(v);
          recalcNormals();
        }

        curr_part_ = new ssgVtxArray(type,
          vertex_array_,
          normal_array_,
          has_texture==true?tex_coords_:NULL,
          NULL,
          curr_index_);
        curr_part_->setState( createState(has_texture) );
#ifdef EXPERIMENTAL_CULL_FACE_CODE
        curr_part_->setCullFace ( curr_cull_face_ ) ;
#endif

        ssgBranch* grp = getCurrGroup();
        grp->addKid( current_options -> createLeaf(curr_part_, NULL) );
      }
      has_texture = false;
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

        setTexture(tex_filename, 0x43);
        break;
      }

    case 0x50:  // GCOLOR (Goraud shaded color)
      {
        unsigned char color, param;
        fread(&color, 1, 1, fp);
        fread(&param, 1, 1, fp);
        JMPRINT( ios::hex, "Set color = " << (int)color << " Para = " << (int)param );
        setEmission((int)color, (int)param);
      }
      break;
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

        if ( poly_from_line ) {  // closed surface
          sgVec3 v;
          if(!has_normals_)
          {
            while (normal_array_->getNum() < vertex_array_->getNum())
              normal_array_->add(v);
            recalcNormals();
          }
          curr_part_ = new ssgVtxArray(
            createTriangIndices(draw_point_index_, poly_from_line_numverts, v),
            vertex_array_,
            normal_array_,
            NULL,
            NULL,
            curr_index_ );
          delete draw_point_index_;
          curr_part_->setState( createState(true) );
        }
        else {                   // open surface draw a ployline
          curr_part_ = new ssgVtxArray(
            GL_LINES,
            vertex_array_,
            NULL,
            NULL,
            NULL,
            draw_point_index_ );
          curr_part_->setState( createState(false) );
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
          short offset;
          offset = ulEndianReadLittle16(fp);
          long addr = ftell(fp);

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
          sgMakeIdentMat4( curr_matrix_ );
          sgZeroVec3( curr_rot_pt_ );
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

      JMPRINT(ios::dec, " info:" << info << " stories:" << stories << " size_x:" << size_x << " size_z:" << size_z);

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
            state->setTexture("sandstone.rgb");
            }
            break;
          case 0x01: //white skyscraper
            {
            state->setTexture("white.rgb");
            }
            break;
          case 0x02: //black skyscraper
            {
            state->setTexture("black.rgb");
            }
            break;
          case 0x03: //dark gray skyscraper
            {
            state->setTexture("darkgray.rgb");
            }
            break;
          case 0x04: //gray skyscraper
            {
            state->setTexture("gray.rgb");
            }
            break;
          case 0x05: //white horizontal lined skyscraper
            {
            state->setTexture("whitehorizon.rgb");
            }
            break;
          case 0x06: //sandstone horizontal lined skyscraper
            {
            state->setTexture("sandstonehorizon.rgb");
            }
            break;
          case 0x07: //gray skyscraper
            {
            state->setTexture("gray.rgb");
            }
            break;
          default:
            break;
        }

// john temp test
        building_count++;

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

        sgSetVec2(btexcoor, 0.1f, 0.1f);               //defining texture coor
        texcoor_walls->add(btexcoor);
        sgSetVec2(btexcoor, 0.1f, 0.9f);
        texcoor_walls->add(btexcoor);
        sgSetVec2(btexcoor, 0.9f, 0.1f);
        texcoor_walls->add(btexcoor);
        sgSetVec2(btexcoor, 0.9f, 0.9f);
        texcoor_walls->add(btexcoor);
        sgSetVec2(btexcoor, 0.1f, 0.1f);
        texcoor_walls->add(btexcoor);
        sgSetVec2(btexcoor, 0.1f, 0.9f);
        texcoor_walls->add(btexcoor);
        sgSetVec2(btexcoor, 0.9f, 0.1f);
        texcoor_walls->add(btexcoor);
        sgSetVec2(btexcoor, 0.9f, 0.9f);
        texcoor_walls->add(btexcoor);
        sgSetVec2(btexcoor, 0.1f, 0.1f);
        texcoor_walls->add(btexcoor);
        sgSetVec2(btexcoor, 0.1f, 0.9f);
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
      }
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

      }  //end of bgl parse switch 
       
      if (skip_offset > 0) 
        fseek( fp, skip_offset, SEEK_CUR );
       
    } //end of bgl parse while loop

  } // end of bgl "object header opcode" while loop
    
  fclose(fp);

  return model_;
}

#else


ssgEntity *ssgLoadBGL(const char *fname, const ssgLoaderOptions *options)
{
  return NULL ;
}


#endif

