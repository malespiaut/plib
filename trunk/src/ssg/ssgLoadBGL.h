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
// File: ssgLoadBGL.h
//
// Created: Tue Feb 29 22:20:31 2000
//
// Author: Juergen Marquardt <juergen_marquardt@t-online.de>
//
//
//===========================================================================
// Copyright (c) 2002 Jürgen Marquardt <juergen_marquardt@t-online.de>
//===========================================================================


// common shininess for all objects other than background surface

#define DEF_SHININESS 50

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
#define RUNAWAY_LAYER 24
#define ZERO_LAYER 24

#undef ABS
#undef MIN
#undef MAX
#define ABS(x) ((x) >= 0 ? (x) : -(x))
#define MIN(a,b) ((a) <= (b) ? (a) : (b))
#define MAX(a,b) ((a) >= (b) ? (a) : (b))
#define MIN3(a,b,c) ((a) <= (b) ? MIN(a,c) : MIN(b,c))
#define MAX3(a,b,c) ((a) >= (b) ? MAX(a,c) : MAX(b,c))

#if defined(WIN32) && !defined(__CYGWIN__)
#define SLASH '\\'
#else
#define SLASH '/'
#endif


// type definitions

// class declarations
class ssgLayeredVtxArray : public ssgVtxArray
{
public:
  ssgLayeredVtxArray ( unsigned int,
                       ssgVertexArray *,
                       ssgNormalArray *,
                       ssgTexCoordArray *,
                       ssgColourArray *,
                       ssgIndexArray * );
  bool isOnGround();
  void moveToBackground();
};

// function declarations
static void parse_proc_scenery(FILE*);
static ssgBranch *ssgLoadBGLFile(const char * );
