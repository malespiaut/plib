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

//
// DXF loader for SSG/PLIB
// Ported from IVCON by Dave McClurg (dpm@efn.org) in March-2000
//   IVCON::dxf_read() by John Burkardt, 23 May 1999
// Added better support for meshes, August 2001
// Added support for colors, blocks and block references, September 2001
//

#include "ssgLocal.h"


#define MAX_LINE_LEN 1024
#define MAX_VERT 100000

struct dxfVert {
  sgVec3 pos ;
  int color_index ;
} ;

enum Entities {
  ENT_NONE,
  ENT_LINE,
  ENT_FACE,
  ENT_POLYLINE,
  ENT_VERTEX,
  ENT_INSERT
};

static int ent_type = ENT_NONE ;

static const ssgLoaderOptions* current_options = NULL ;
static ssgState* current_state = NULL ;
static ssgBranch* top_branch = NULL ;
static ssgBranch* current_block = NULL ;
static ssgBranch* blocks = NULL ;

static int num_line;
static int num_linevert;
static dxfVert* linevert;

static int num_face;
static int num_facevert;
static dxfVert* facevert;

static int num_meshvert;
static dxfVert* meshvert;
static int meshface[4];
static int meshflags;
static int meshsize[2];

static int num_vert;
static int cflags;
static sgVec3 cvec;
static sgVec3 scale_vec;
static float rot_angle;
static char* block_name;
static color_index ;

static float dxf_colors [256][3] =
{
  { 0.0000f, 0.0000f, 0.0000f },
  { 1.0000f, 0.0000f, 0.0000f },
  { 1.0000f, 1.0000f, 0.0000f },
  { 0.0000f, 1.0000f, 0.0000f },
  { 0.0000f, 1.0000f, 1.0000f },
  { 0.0000f, 0.0000f, 1.0000f },
  { 1.0000f, 0.0000f, 1.0000f },
  { 0.0000f, 0.0000f, 0.0000f },
  { 0.0000f, 0.0000f, 0.0000f },
  { 0.0000f, 0.0000f, 0.0000f },
  { 1.0000f, 0.0000f, 0.0000f },
  { 1.0000f, 0.5000f, 0.5000f },
  { 0.6500f, 0.0000f, 0.0000f },
  { 0.6500f, 0.3250f, 0.3250f },
  { 0.5000f, 0.0000f, 0.0000f },
  { 0.5000f, 0.2500f, 0.2500f },
  { 0.3000f, 0.0000f, 0.0000f },
  { 0.3000f, 0.1500f, 0.1500f },
  { 0.1500f, 0.0000f, 0.0000f },
  { 0.1500f, 0.0750f, 0.0750f },
  { 1.0000f, 0.2500f, 0.0000f },
  { 1.0000f, 0.6250f, 0.5000f },
  { 0.6500f, 0.1625f, 0.0000f },
  { 0.6500f, 0.4063f, 0.3250f },
  { 0.5000f, 0.1250f, 0.0000f },
  { 0.5000f, 0.3125f, 0.2500f },
  { 0.3000f, 0.0750f, 0.0000f },
  { 0.3000f, 0.1875f, 0.1500f },
  { 0.1500f, 0.0375f, 0.0000f },
  { 0.1500f, 0.0938f, 0.0750f },
  { 1.0000f, 0.5000f, 0.0000f },
  { 1.0000f, 0.7500f, 0.5000f },
  { 0.6500f, 0.3250f, 0.0000f },
  { 0.6500f, 0.4875f, 0.3250f },
  { 0.5000f, 0.2500f, 0.0000f },
  { 0.5000f, 0.3750f, 0.2500f },
  { 0.3000f, 0.1500f, 0.0000f },
  { 0.3000f, 0.2250f, 0.1500f },
  { 0.1500f, 0.0750f, 0.0000f },
  { 0.1500f, 0.1125f, 0.0750f },
  { 1.0000f, 0.7500f, 0.0000f },
  { 1.0000f, 0.8750f, 0.5000f },
  { 0.6500f, 0.4875f, 0.0000f },
  { 0.6500f, 0.5688f, 0.3250f },
  { 0.5000f, 0.3750f, 0.0000f },
  { 0.5000f, 0.4375f, 0.2500f },
  { 0.3000f, 0.2250f, 0.0000f },
  { 0.3000f, 0.2625f, 0.1500f },
  { 0.1500f, 0.1125f, 0.0000f },
  { 0.1500f, 0.1313f, 0.0750f },
  { 1.0000f, 1.0000f, 0.0000f },
  { 1.0000f, 1.0000f, 0.5000f },
  { 0.6500f, 0.6500f, 0.0000f },
  { 0.6500f, 0.6500f, 0.3250f },
  { 0.5000f, 0.5000f, 0.0000f },
  { 0.5000f, 0.5000f, 0.2500f },
  { 0.3000f, 0.3000f, 0.0000f },
  { 0.3000f, 0.3000f, 0.1500f },
  { 0.1500f, 0.1500f, 0.0000f },
  { 0.1500f, 0.1500f, 0.0750f },
  { 0.7500f, 1.0000f, 0.0000f },
  { 0.8750f, 1.0000f, 0.5000f },
  { 0.4875f, 0.6500f, 0.0000f },
  { 0.5688f, 0.6500f, 0.3250f },
  { 0.3750f, 0.5000f, 0.0000f },
  { 0.4375f, 0.5000f, 0.2500f },
  { 0.2250f, 0.3000f, 0.0000f },
  { 0.2625f, 0.3000f, 0.1500f },
  { 0.1125f, 0.1500f, 0.0000f },
  { 0.1313f, 0.1500f, 0.0750f },
  { 0.5000f, 1.0000f, 0.0000f },
  { 0.7500f, 1.0000f, 0.5000f },
  { 0.3250f, 0.6500f, 0.0000f },
  { 0.4875f, 0.6500f, 0.3250f },
  { 0.2500f, 0.5000f, 0.0000f },
  { 0.3750f, 0.5000f, 0.2500f },
  { 0.1500f, 0.3000f, 0.0000f },
  { 0.2250f, 0.3000f, 0.1500f },
  { 0.0750f, 0.1500f, 0.0000f },
  { 0.1125f, 0.1500f, 0.0750f },
  { 0.2500f, 1.0000f, 0.0000f },
  { 0.6250f, 1.0000f, 0.5000f },
  { 0.1625f, 0.6500f, 0.0000f },
  { 0.4063f, 0.6500f, 0.3250f },
  { 0.1250f, 0.5000f, 0.0000f },
  { 0.3125f, 0.5000f, 0.2500f },
  { 0.0750f, 0.3000f, 0.0000f },
  { 0.1875f, 0.3000f, 0.1500f },
  { 0.0375f, 0.1500f, 0.0000f },
  { 0.0938f, 0.1500f, 0.0750f },
  { 0.0000f, 1.0000f, 0.0000f },
  { 0.5000f, 1.0000f, 0.5000f },
  { 0.0000f, 0.6500f, 0.0000f },
  { 0.3250f, 0.6500f, 0.3250f },
  { 0.0000f, 0.5000f, 0.0000f },
  { 0.2500f, 0.5000f, 0.2500f },
  { 0.0000f, 0.3000f, 0.0000f },
  { 0.1500f, 0.3000f, 0.1500f },
  { 0.0000f, 0.1500f, 0.0000f },
  { 0.0750f, 0.1500f, 0.0750f },
  { 0.0000f, 1.0000f, 0.2500f },
  { 0.5000f, 1.0000f, 0.6250f },
  { 0.0000f, 0.6500f, 0.1625f },
  { 0.3250f, 0.6500f, 0.4063f },
  { 0.0000f, 0.5000f, 0.1250f },
  { 0.2500f, 0.5000f, 0.3125f },
  { 0.0000f, 0.3000f, 0.0750f },
  { 0.1500f, 0.3000f, 0.1875f },
  { 0.0000f, 0.1500f, 0.0375f },
  { 0.0750f, 0.1500f, 0.0938f },
  { 0.0000f, 1.0000f, 0.5000f },
  { 0.5000f, 1.0000f, 0.7500f },
  { 0.0000f, 0.6500f, 0.3250f },
  { 0.3250f, 0.6500f, 0.4875f },
  { 0.0000f, 0.5000f, 0.2500f },
  { 0.2500f, 0.5000f, 0.3750f },
  { 0.0000f, 0.3000f, 0.1500f },
  { 0.1500f, 0.3000f, 0.2250f },
  { 0.0000f, 0.1500f, 0.0750f },
  { 0.0750f, 0.1500f, 0.1125f },
  { 0.0000f, 1.0000f, 0.7500f },
  { 0.5000f, 1.0000f, 0.8750f },
  { 0.0000f, 0.6500f, 0.4875f },
  { 0.3250f, 0.6500f, 0.5688f },
  { 0.0000f, 0.5000f, 0.3750f },
  { 0.2500f, 0.5000f, 0.4375f },
  { 0.0000f, 0.3000f, 0.2250f },
  { 0.1500f, 0.3000f, 0.2625f },
  { 0.0000f, 0.1500f, 0.1125f },
  { 0.0750f, 0.1500f, 0.1313f },
  { 0.0000f, 1.0000f, 1.0000f },
  { 0.5000f, 1.0000f, 1.0000f },
  { 0.0000f, 0.6500f, 0.6500f },
  { 0.3250f, 0.6500f, 0.6500f },
  { 0.0000f, 0.5000f, 0.5000f },
  { 0.2500f, 0.5000f, 0.5000f },
  { 0.0000f, 0.3000f, 0.3000f },
  { 0.1500f, 0.3000f, 0.3000f },
  { 0.0000f, 0.1500f, 0.1500f },
  { 0.0750f, 0.1500f, 0.1500f },
  { 0.0000f, 0.7500f, 1.0000f },
  { 0.5000f, 0.8750f, 1.0000f },
  { 0.0000f, 0.4875f, 0.6500f },
  { 0.3250f, 0.5688f, 0.6500f },
  { 0.0000f, 0.3750f, 0.5000f },
  { 0.2500f, 0.4375f, 0.5000f },
  { 0.0000f, 0.2250f, 0.3000f },
  { 0.1500f, 0.2625f, 0.3000f },
  { 0.0000f, 0.1125f, 0.1500f },
  { 0.0750f, 0.1313f, 0.1500f },
  { 0.0000f, 0.5000f, 1.0000f },
  { 0.5000f, 0.7500f, 1.0000f },
  { 0.0000f, 0.3250f, 0.6500f },
  { 0.3250f, 0.4875f, 0.6500f },
  { 0.0000f, 0.2500f, 0.5000f },
  { 0.2500f, 0.3750f, 0.5000f },
  { 0.0000f, 0.1500f, 0.3000f },
  { 0.1500f, 0.2250f, 0.3000f },
  { 0.0000f, 0.0750f, 0.1500f },
  { 0.0750f, 0.1125f, 0.1500f },
  { 0.0000f, 0.2500f, 1.0000f },
  { 0.5000f, 0.6250f, 1.0000f },
  { 0.0000f, 0.1625f, 0.6500f },
  { 0.3250f, 0.4063f, 0.6500f },
  { 0.0000f, 0.1250f, 0.5000f },
  { 0.2500f, 0.3125f, 0.5000f },
  { 0.0000f, 0.0750f, 0.3000f },
  { 0.1500f, 0.1875f, 0.3000f },
  { 0.0000f, 0.0375f, 0.1500f },
  { 0.0750f, 0.0938f, 0.1500f },
  { 0.0000f, 0.0000f, 1.0000f },
  { 0.5000f, 0.5000f, 1.0000f },
  { 0.0000f, 0.0000f, 0.6500f },
  { 0.3250f, 0.3250f, 0.6500f },
  { 0.0000f, 0.0000f, 0.5000f },
  { 0.2500f, 0.2500f, 0.5000f },
  { 0.0000f, 0.0000f, 0.3000f },
  { 0.1500f, 0.1500f, 0.3000f },
  { 0.0000f, 0.0000f, 0.1500f },
  { 0.0750f, 0.0750f, 0.1500f },
  { 0.2500f, 0.0000f, 1.0000f },
  { 0.6250f, 0.5000f, 1.0000f },
  { 0.1625f, 0.0000f, 0.6500f },
  { 0.4063f, 0.3250f, 0.6500f },
  { 0.1250f, 0.0000f, 0.5000f },
  { 0.3125f, 0.2500f, 0.5000f },
  { 0.0397f, 0.0000f, 0.3000f },
  { 0.1875f, 0.1500f, 0.3000f },
  { 0.0375f, 0.0000f, 0.1500f },
  { 0.0938f, 0.0750f, 0.1500f },
  { 0.5000f, 0.0000f, 1.0000f },
  { 0.7500f, 0.5000f, 1.0000f },
  { 0.3250f, 0.0000f, 0.6500f },
  { 0.4875f, 0.3250f, 0.6500f },
  { 0.2500f, 0.0000f, 0.5000f },
  { 0.3750f, 0.2500f, 0.5000f },
  { 0.1500f, 0.0000f, 0.3000f },
  { 0.2250f, 0.1500f, 0.3000f },
  { 0.0750f, 0.0000f, 0.1500f },
  { 0.1125f, 0.0750f, 0.1500f },
  { 0.7500f, 0.0000f, 1.0000f },
  { 0.8750f, 0.5000f, 1.0000f },
  { 0.4875f, 0.0000f, 0.6500f },
  { 0.5688f, 0.3250f, 0.6500f },
  { 0.3750f, 0.0000f, 0.5000f },
  { 0.4375f, 0.2500f, 0.5000f },
  { 0.2250f, 0.0000f, 0.3000f },
  { 0.2625f, 0.1500f, 0.3000f },
  { 0.1125f, 0.0000f, 0.1500f },
  { 0.1313f, 0.0750f, 0.1500f },
  { 1.0000f, 0.0000f, 1.0000f },
  { 1.0000f, 0.5000f, 1.0000f },
  { 0.6500f, 0.0000f, 0.6500f },
  { 0.6500f, 0.3250f, 0.6500f },
  { 0.5000f, 0.0000f, 0.5000f },
  { 0.5000f, 0.2500f, 0.5000f },
  { 0.3000f, 0.0000f, 0.3000f },
  { 0.3000f, 0.1500f, 0.3000f },
  { 0.1500f, 0.0000f, 0.1500f },
  { 0.1500f, 0.0750f, 0.1500f },
  { 1.0000f, 0.0000f, 0.7500f },
  { 1.0000f, 0.5000f, 0.8750f },
  { 0.6500f, 0.0000f, 0.4875f },
  { 0.6500f, 0.3250f, 0.5688f },
  { 0.5000f, 0.0000f, 0.3750f },
  { 0.5000f, 0.2500f, 0.4375f },
  { 0.3000f, 0.0000f, 0.2250f },
  { 0.3000f, 0.1500f, 0.2625f },
  { 0.1500f, 0.0000f, 0.1125f },
  { 0.1500f, 0.0750f, 0.1313f },
  { 1.0000f, 0.0000f, 0.5000f },
  { 1.0000f, 0.5000f, 0.7500f },
  { 0.6500f, 0.0000f, 0.3250f },
  { 0.6500f, 0.3250f, 0.4875f },
  { 0.5000f, 0.0000f, 0.2500f },
  { 0.5000f, 0.2500f, 0.3750f },
  { 0.3000f, 0.0000f, 0.1500f },
  { 0.3000f, 0.1500f, 0.2250f },
  { 0.1500f, 0.0000f, 0.0750f },
  { 0.1500f, 0.0750f, 0.1125f },
  { 1.0000f, 0.0000f, 0.2500f },
  { 1.0000f, 0.5000f, 0.6250f },
  { 0.6500f, 0.0000f, 0.1625f },
  { 0.6500f, 0.3250f, 0.4063f },
  { 0.5000f, 0.0000f, 0.1250f },
  { 0.5000f, 0.2500f, 0.3125f },
  { 0.3000f, 0.0000f, 0.0750f },
  { 0.3000f, 0.1500f, 0.1875f },
  { 0.1500f, 0.0000f, 0.0375f },
  { 0.1500f, 0.0750f, 0.0938f },
  { 0.3300f, 0.3300f, 0.3300f },
  { 0.4640f, 0.4640f, 0.4640f },
  { 0.5980f, 0.5980f, 0.5980f },
  { 0.7320f, 0.7320f, 0.7320f },
  { 0.8660f, 0.8660f, 0.8660f },
  { 1.0000f, 1.0000f, 1.0000f },
};

static copy_vert ( dxfVert& dst, const dxfVert& src )
{
  dst.color_index = src.color_index ;
  sgCopyVec3 ( dst.pos, src.pos ) ;
}

static void dxf_flush ( void )
{
  if ( ent_type == ENT_LINE ) {
    if ( num_vert >= 2 ) {
      num_line ++;
      num_linevert += 2;
    }
  }
  else if ( ent_type == ENT_FACE ) {
    if ( num_vert >= 3 ) {
      //quad?
      if ( num_vert >= 4 && num_facevert + 6 < MAX_VERT ) {
        copy_vert( facevert[num_facevert + 4], facevert[num_facevert + 1] );
        copy_vert( facevert[num_facevert + 5], facevert[num_facevert + 2] );
        copy_vert( facevert[num_facevert + 2], facevert[num_facevert + 3] );
        num_face += 2;
        num_facevert += 6;
      }
      else {
        //triangle
        num_face += 1;
        num_facevert += 3;
      }
    }
  }
  else if ( ent_type == ENT_POLYLINE ) {
    meshflags = cflags;
    num_meshvert = 0;
    meshsize[0] = meshface[0];
    meshsize[1] = meshface[1];
  }
  else if ( ent_type == ENT_INSERT ) {

    if ( block_name != NULL )
    {
      //block_name
      ssgEntity* found = blocks -> getByName ( block_name ) ;
      if ( found != NULL )
      {
        //cvec
        //scale_vec
        //rot_angle
        sgMat4 mat ;
        sgMakeTransMat4 ( mat, cvec ) ;

        ssgTransform* block_tr = new ssgTransform ;
        block_tr -> setName ( block_name ) ;
        block_tr -> setTransform ( mat ) ;
        block_tr -> addKid ( found ) ;
        top_branch -> addKid ( block_tr ) ;
      }
    }
  }
  else if ( ent_type == ENT_VERTEX ) {

    if ( (meshflags & 8) != 0 ) {

      //This is a 3D Polyline
      if ( (cflags & 32) != 0 ) {

        //This is a 3D Polyline vertex
        if ( num_meshvert < MAX_VERT ) {
          meshvert[num_meshvert].color_index = color_index;
          sgCopyVec3( meshvert[num_meshvert].pos, cvec );
          num_meshvert ++ ;
        }
      }
    }
    else if ( (meshflags & 16) != 0 ) {

      //This is a 3D polygon MxN mesh. (uniform grid)
      if ( (cflags & 64) != 0 ) {

        //This is a 3D polygon mesh vertex
        if ( num_meshvert < MAX_VERT ) {
          meshvert[num_meshvert].color_index = color_index;
          sgCopyVec3( meshvert[num_meshvert].pos, cvec );
          num_meshvert ++ ;
        }
      }
    }
    else if ( (meshflags & 64) != 0 ) {

      //This Polyline is a polyface mesh.
      if ( (cflags & 128) != 0 ) {

        if ( (cflags & 64) != 0 ) {

          //This is a 3D polygon mesh vertex
          if ( num_meshvert < MAX_VERT ) {
            meshvert[num_meshvert].color_index = color_index;
            sgCopyVec3( meshvert[num_meshvert].pos, cvec );
            num_meshvert ++ ;
          }
        }
        else if ( num_vert >= 3 && num_facevert + num_vert < MAX_VERT ) {

          //copy each vertex where the first is numbered 1
          int error = 0;
          for ( int i=0; i<num_vert; i++ )
          {
            int ival = meshface[i];
            if ( ival < 0 )
              ival = -ival;  //invisible vertex whatever that means :)
            if ( ival > 0 && ival <= num_meshvert )
              copy_vert( facevert[num_facevert + i], meshvert[ival-1] );
            else
              error = 1;
          }

          if ( error == 0 ) {
            //quad?
            if ( num_vert >= 4 && num_facevert + 6 < MAX_VERT ) {
              copy_vert( facevert[num_facevert + 4], facevert[num_facevert + 1] );
              copy_vert( facevert[num_facevert + 5], facevert[num_facevert + 2] );
              copy_vert( facevert[num_facevert + 2], facevert[num_facevert + 3] );
              num_face += 2;
              num_facevert += 6;
            }
            else {
              //triangle
              num_face += 1;
              num_facevert += 3;
            }
          }
        }
      }
    }
  }

  num_vert = 0;
  cflags = 0;
  cvec[0] = 0.0f;
  cvec[1] = 0.0f;
  cvec[2] = 0.0f;
  scale_vec[0] = 0.0f;
  scale_vec[1] = 0.0f;
  scale_vec[2] = 0.0f;
  rot_angle = 0.0f;
  color_index = 7 ;
  meshface[0] = 0;
  meshface[1] = 0;
  meshface[2] = 0;
  meshface[3] = 0;

  delete[] block_name;
  block_name = NULL;
}


static void AddTriangle ( const dxfVert* p, const dxfVert* q, const dxfVert* r )
{
  if ( num_facevert + 3 < MAX_VERT ) {
    copy_vert( facevert[num_facevert], *p ) ;
    copy_vert( facevert[num_facevert+1], *q ) ;
    copy_vert( facevert[num_facevert+2], *r ) ;
    num_face ++;
    num_facevert += 3;
  }
}


static void dxf_init ()
{
  linevert = new dxfVert[MAX_VERT];
  facevert = new dxfVert[MAX_VERT];
  meshvert = new dxfVert[MAX_VERT];

  for ( int i=0; i<MAX_VERT; i++ )
  {
    linevert[i].color_index = 7 ;
    facevert[i].color_index = 7 ;
    meshvert[i].color_index = 7 ;
  }
}


static void dxf_free ()
{
  delete[] linevert ;
  delete[] facevert ;
  delete[] meshvert ;
}


static void dxf_reset ()
{
  num_line = 0;
  num_linevert = 0;
  num_face = 0;
  num_facevert = 0;
  num_meshvert = 0;
  num_vert = 0;

  ent_type = ENT_NONE;
}


static sgVec4& get_color ( int ci )
{
  if ( ci < 0 )
    ci = 0 ;
  else if ( ci > 255 )
    ci = 255 ;
  float* rgb = dxf_colors[ci];
  static sgVec4 color ;
  sgSetVec4 ( color, rgb[0], rgb[1], rgb[2], 1.0f ) ;
  return color ;
}


static void dxf_create ( ssgBranch* br )
{
  dxf_flush () ;

  //create ssg nodes
  if ( num_face )
  {
    ssgVertexArray* vlist = new ssgVertexArray ( num_facevert ) ;
    ssgColourArray* clist = new ssgColourArray ( num_facevert ) ;
    ssgNormalArray* nlist = new ssgNormalArray ( num_facevert ) ;
    sgVec3 normal ;
    for ( int i=0; i<num_facevert; i++ )
    {
      if ( (i % 3) == 0 )
      {
        sgMakeNormal ( normal,
          facevert[i].pos,
          facevert[i+1].pos,
          facevert[i+2].pos ) ;
      }
      
      vlist -> add ( facevert[i].pos ) ;
      nlist -> add ( normal ) ;
      clist -> add ( get_color(facevert[i].color_index) ) ;
    }
    ssgVtxTable *vtab = new ssgVtxTable ( GL_TRIANGLES, vlist, nlist, 0, clist );
    vtab -> setState ( current_state ) ;
    br -> addKid ( vtab ) ;
  }

  if ( num_line )
  {
    ssgVertexArray* vlist = new ssgVertexArray ( num_linevert ) ;
    ssgColourArray* clist = new ssgColourArray ( num_facevert ) ;
    for ( int i=0; i<num_linevert; i++ )
    {
      vlist -> add ( linevert[i].pos ) ;
      clist -> add ( get_color(linevert[i].color_index) ) ;
    }
    ssgVtxTable *vtab = new ssgVtxTable ( GL_LINES, vlist, 0, 0, clist );
    vtab -> setState ( current_state ) ;
    br -> addKid ( vtab ) ;
  }

  dxf_reset () ;
}


static int dxf_read ( FILE *filein )
{
  dxf_init () ;

/* 
  Read the next two lines of the file into INPUT1 and INPUT2. 
*/
  for ( ;; ) {

    int   count;
    int   code;
    int   width;
    char  input1[MAX_LINE_LEN];
    char  input2[MAX_LINE_LEN];

/* 
  INPUT1 should contain a single integer, which tells what INPUT2
  will contain.
*/
    if ( fgets ( input1, MAX_LINE_LEN, filein ) == NULL ) {
      break;
    }

    count = sscanf ( input1, "%d%n", &code, &width );
    if ( count <= 0 ) {
      break;
    }
/*
  Read the second line, and interpret it according to the code.
*/
    if ( fgets ( input2, MAX_LINE_LEN, filein ) == NULL ) {
      break;
    }

    if ( code == 0 ) {

      dxf_flush () ;

      //set ent_type
      if ( strncmp( input2, "LINE", 4 ) == 0 )
        ent_type = ENT_LINE ;
      else if ( strncmp( input2, "3DFACE", 6 ) == 0 )
        ent_type = ENT_FACE ;
      else if ( strncmp( input2, "POLYLINE", 8 ) == 0 )
        ent_type = ENT_POLYLINE ;
      else if ( strncmp( input2, "VERTEX", 6 ) == 0 )
        ent_type = ENT_VERTEX;
      else if ( strncmp( input2, "INSERT", 6 ) == 0 )
        ent_type = ENT_INSERT;
      else if ( strncmp( input2, "BLOCK", 5 ) == 0 ) {
        if ( current_block == NULL ) {
          current_block = new ssgBranch ;
          blocks -> addKid ( current_block ) ;
        }
      }
      else if ( strncmp( input2, "ENDBLK", 6 ) == 0 ) {
        if ( current_block != NULL ) {
          dxf_create ( current_block ) ;
          current_block = NULL ;
        }
      }
      else if ( strncmp( input2, "SEQEND", 6 ) == 0 ) {

#define PL_CLOSED_IN_M 		0x01
#define PL_CURVE_FIT_ADDED 	0x02
#define PL_SPLINE_FIT_ADDED 	0x04
#define PL_3D_POLYLINE 		0x08
#define PL_3D_MESH 		0x10
#define PL_CLOSED_IN_N 		0x20
#define PL_POLYFACE_MESH 	0x40
#define PL_USE_LINETYPE 	0x80

        int polyline_flags = meshflags ;

        if ( (polyline_flags & PL_3D_POLYLINE) != 0 ) {

          //This is a 3D Polyline
          int last = 0;
          int i = 1;

          if ( (polyline_flags & (PL_CLOSED_IN_M|PL_CLOSED_IN_N)) != 0 ) {

            //Polyline is closed
            last = num_meshvert - 1;
            i = 0;
          }

          for ( ; i<num_meshvert; i++ ) {

            if ( num_linevert + 2 < MAX_VERT ) {

              copy_vert( linevert[num_linevert], meshvert[last] );
              copy_vert( linevert[num_linevert+1], meshvert[i] );
              num_line ++;
              num_linevert += 2;
            }
          }
        }
        else if ( (polyline_flags & PL_3D_MESH) != 0 ) {

          //This is a 3D polygon MxN mesh. (uniform grid)
          if ( num_meshvert >= ( meshsize[0] * meshsize[1] ) ) {

            int mesh_m = meshsize[0];
            int mesh_n = meshsize[1];

            dxfVert *buff[2];
            buff[0] = &meshvert[0];
            buff[1] = &meshvert[mesh_n];

            /* create triangles */
            int i,j;
            for (i=1;i<mesh_m;i++) {
              buff[1] = &meshvert[ mesh_n*i ] ;
              dxfVert* p = &buff[0][0];
              dxfVert* q = &buff[0][1];
              for (j=1;j<mesh_n;j++) {
                dxfVert* r = &buff[1][j-1];
                AddTriangle ( p, q, r ) ;
                p = q;
                q = &buff[1][j];
                AddTriangle ( p, q, r ) ;
                q = &buff[0][j+1];
              }
              if (polyline_flags & PL_CLOSED_IN_N) {
                dxfVert* p = &buff[0][mesh_n-1];
                dxfVert* q = &buff[0][0];
                dxfVert* r = &buff[1][mesh_n-1];
                AddTriangle ( p, q, r ) ;
                p = q;
                q = &buff[1][0];
                AddTriangle ( p, q, r ) ;
              }
              buff[0] = buff[1];
            }
            if (polyline_flags & PL_CLOSED_IN_M) {
              dxfVert* p = &buff[0][0];
              dxfVert* q = &buff[0][1];
              for (j=1;j<mesh_n;j++) {
                dxfVert* r = &meshvert[j-1];
                AddTriangle ( p, q, r ) ;
                p = q;
                q = &meshvert[j];
                AddTriangle ( p, q, r ) ;
                q = &buff[0][j+1];
              }
            }
          }
        }

        meshflags = 0;
        ent_type = ENT_NONE;
      }
      else
        ent_type = ENT_NONE;
    }
    else {
      
      int cpos;
      for (cpos = 0; input1[cpos] == ' '; cpos++)
        ;
      
      if ( input1[cpos] == '1' || input1[cpos] == '2' || input1[cpos] == '3' ) {

        char ch = input1[cpos+1];

        if ( input1[cpos] == '2' && ( ch == 0 || ch == ' ' || ch == '\n' ) ) {

          if ( ent_type == ENT_INSERT || current_block != NULL ) {

            if ( ent_type == ENT_INSERT && block_name == NULL ) {
              block_name = new char[ strlen(input2)+1 ] ;
              strcpy ( block_name, input2 ) ;
            }

            if ( current_block != NULL && current_block->getName() == NULL ) {
              char* name = new char[ strlen(input2)+1 ] ;
              strcpy ( name, input2 ) ;
              current_block->setName( name ) ;
            }
          }
        }
        else if ( ch == '0' || ch == '1' || ch == '2' || ch == '3' ) {

          float rval;
          count = sscanf ( input2, "%e%n", &rval, &width );

          switch ( input1[cpos] )
          {
            case '1':
              cvec[0] = rval;
              break;
  
            case '2':
              cvec[1] = rval;
              break;
  
            case '3':
              cvec[2] = rval;
  
              if ( ent_type == ENT_LINE ) {
                if ( num_linevert + num_vert < MAX_VERT ) {
                  linevert[num_linevert + num_vert].color_index = color_index;
                  sgCopyVec3( linevert[num_linevert + num_vert].pos, cvec );
                  num_vert ++ ;
                }
              }
              else if ( ent_type == ENT_FACE ) {
                if ( num_facevert + num_vert < MAX_VERT ) {
                  facevert[num_facevert + num_vert].color_index = color_index;
                  sgCopyVec3( facevert[num_facevert + num_vert].pos, cvec );
                  num_vert ++ ;
                }
              }
              break;
          }
        }
      }
      else if ( input1[cpos] == '4' ) {

        float rval;
        count = sscanf ( input2, "%e%n", &rval, &width );

        switch ( input1[cpos+1] )
        {
          case '1':
            scale_vec[0] = rval;
            break;

          case '2':
            scale_vec[1] = rval;
            break;

          case '3':
            scale_vec[2] = rval;
            break;
        }
      }
      else if ( input1[cpos] == '5' ) {

        char ch = input1[cpos+1];
        if ( ch == 0 || ch == ' ' || ch == '\n' ) {

          count = sscanf ( input2, "%e%n", &rot_angle, &width );
        }
      }
      else if ( input1[cpos] == '6' ) {

        if ( input1[cpos+1] == '2' ) {

          count = sscanf ( input2, "%d%n", &color_index, &width );
        }
      }
      else if ( input1[cpos] == '7' ) {

        int ival ;
        count = sscanf ( input2, "%d%n", &ival, &width );

        switch ( input1[cpos+1] )
        {
          case '0':
            cflags = ival;
            break;

          case '1':
            meshface[0] = ival;
            num_vert = 1;
            break;

          case '2':
            meshface[1] = ival;
            num_vert = 2;
            break;

          case '3':
            meshface[2] = ival;
            num_vert = 3;
            break;

          case '4':
            meshface[3] = ival;
            num_vert = 4;
            break;
        }
      }
    }
  }

  dxf_create ( top_branch ) ;
  dxf_free () ;

  return TRUE;
}


static ssgState *make_state ()
{
  ssgSimpleState *st = new ssgSimpleState () ;

  sgVec4 spec = { 0.0f, 0.0f, 0.0f, 1.0f };
  sgVec4 emis = { 0.0f, 0.0f, 0.0f, 1.0f } ;
  float  shi  = { 0.0f } ;

  st -> setMaterial ( GL_SPECULAR, spec ) ;
  st -> setMaterial ( GL_EMISSION, emis ) ;
  st -> setShininess ( shi ) ;
  st -> enable ( GL_COLOR_MATERIAL ) ;
  st -> setColourMaterial ( GL_AMBIENT_AND_DIFFUSE ) ;
  st -> enable  ( GL_LIGHTING ) ;
  st -> setShadeModel ( GL_SMOOTH ) ;

  st -> disable ( GL_BLEND ) ;
  st -> setOpaque () ;
  st -> disable( GL_TEXTURE_2D ) ;

  return st ;
}


ssgEntity *ssgLoadDXF ( const char *fname, const ssgLoaderOptions* options )
{
  ssgSetCurrentOptions ( (ssgLoaderOptions*)options ) ;
  current_options = ssgGetCurrentOptions () ;

  top_branch = NULL ;

  char filename [ 1024 ] ;
  current_options -> makeModelPath ( filename, fname ) ;

  FILE *loader_fd = fopen ( filename, "ra" ) ;

  if ( loader_fd == NULL )
  {
    ulSetError ( UL_WARNING, "ssgLoadDXF: Failed to open '%s' for reading", filename ) ;
    return NULL ;
  }

  top_branch = new ssgTransform () ;

  blocks = new ssgBranch ;
  current_state = make_state () ;

  blocks -> ref () ;
  current_state -> ref () ;

  dxf_read ( loader_fd ) ;

  fclose ( loader_fd ) ;

  ssgDeRefDelete ( current_state ) ;
  ssgDeRefDelete ( blocks ) ;

  return top_branch ;
}
