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
//           - REMOVED until legal issues are resolved
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

ssgEntity *ssgLoadBGLBatch(const char *fname, const ssgLoaderOptions *options)
{
  ulSetError(UL_WARNING, "REMOVED until legal issues are resolved.");
  return NULL ;
}

ssgEntity *ssgLoadBGL(const char *fname, const ssgLoaderOptions *options)
{
  ulSetError(UL_WARNING, "REMOVED until legal issues are resolved.");
  return NULL ;
}

#else

ssgEntity *ssgLoadBGLBatch(const char *fname, const ssgLoaderOptions *options)
{
  return NULL ;
}

ssgEntity *ssgLoadBGL(const char *fname, const ssgLoaderOptions *options)
{
  return NULL ;
}


#endif

