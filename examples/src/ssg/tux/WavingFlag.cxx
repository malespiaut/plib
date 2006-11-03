/*
     PLIB - A Suite of Portable Game Libraries
     Copyright (C) 2001  Steve Baker

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

     For further information visit http://plib.sourceforge.net

     $Id$
*/


#ifdef WIN32
#  include <windows.h>
#else
#  include <unistd.h>
#endif

#ifdef FREEGLUT_IS_PRESENT
#  include <GL/freeglut.h>
#else
#  ifdef __APPLE__
#    include <GLUT/glut.h>
#  else
#    include <GL/glut.h>
#  endif
#endif

#include <plib/ssg.h>
#include "WavingFlag.h"

WavingFlag::WavingFlag( sgVec4 color, char *texture1, char *texture2, int s )
{
	size = s;
	int numVertices = size*size;
	branch = new ssgBranch();
	branch->clrTraversalMaskBits ( SSGTRAV_HOT ) ;

	colors = new ssgColourArray( numVertices );
	normals = new ssgNormalArray( numVertices );
	texCoords = new ssgTexCoordArray( numVertices );
	vertices = new ssgVertexArray( numVertices );
	int i;
	for ( i = 0; i < numVertices; i++ )
	{
		sgVec3 n = { 0.0, 0.0, 0.0 };
		sgVec2 t = { 0.0, 0.0 };
		colors->add( color );
		normals->add( n );
		texCoords->add( t );
		vertices->add( n );
	}
	ssgSimpleState *state1 = new ssgSimpleState ;
	if ( texture1 )
	{
		state1->setTexture( texture1 ) ;
		state1->enable( GL_TEXTURE_2D ) ;
	}
	else
	{
		state1->disable( GL_TEXTURE_2D ) ;
	}
	state1->enable( GL_LIGHTING ) ;
	state1->setShadeModel( GL_SMOOTH ) ;
	state1->enable( GL_COLOR_MATERIAL ) ;
	state1->setColourMaterial( GL_AMBIENT_AND_DIFFUSE ) ;
	state1->setMaterial( GL_EMISSION, 0.91f, 0.41f, 0.81f, .94f ) ;
	state1->setMaterial( GL_SPECULAR, 0.91f, 0.41f, 0.81f, .94f ) ;
	state1->setShininess( 0.3f ) ;
	state1->setTranslucent() ;
	state1->enable( GL_BLEND ) ;
    state1->enable ( GL_ALPHA_TEST ) ;
    state1->setAlphaClamp ( 0.0f ) ;

	ssgSimpleState *state2 = new ssgSimpleState ;
	if ( texture2 )
	{
		state2->setTexture( texture2 ) ;
		state2->enable( GL_TEXTURE_2D ) ;
	}
	else
	{
		if ( texture1 )
		{
			state2->setTexture( texture1 ) ;
			state2->enable( GL_TEXTURE_2D ) ;
		}
		else
		{
			state2->disable( GL_TEXTURE_2D ) ;
		}
	}
	state2->enable( GL_LIGHTING ) ;
	state2->setShadeModel( GL_SMOOTH ) ;
	state2->enable( GL_COLOR_MATERIAL ) ;
	state2->setColourMaterial( GL_AMBIENT_AND_DIFFUSE ) ;
	state2->setMaterial( GL_EMISSION, 0.91f, 0.41f, 0.81f, .94f ) ;
	state2->setMaterial( GL_SPECULAR, 0.91f, 0.41f, 0.81f, .94f ) ;
	state2->setShininess( 0.3f ) ;
	state2->setTranslucent() ;
	state2->enable( GL_BLEND ) ;
    state2->enable ( GL_ALPHA_TEST ) ;
    state2->setAlphaClamp ( 0.0f ) ;

	branch = new ssgBranch;
	int x,z;
	for ( x = 0; x < size; x++ )
	{
		ssgIndexArray *indices = new ssgIndexArray( size*2 );  // one side of the flag
		ssgIndexArray *indices2 = new ssgIndexArray( size*2 ); // and the other side
		for ( z = 0; z < size; z++ )
		{
			int i = x*size + z;

			float *v = vertices->get( i );
			v[0] = (float)x/(size - 1.0f);
			v[1] = 0.0;
			v[2] = (float)z/(size - 1.0f);

			float *t = texCoords->get( i );
			sgSetVec2( t, v[0], v[2] );

			indices->add( i );
			indices->add( i+size );
			indices2->add( i+size );
			indices2->add( i );
		}
		if ( x < size-1 )
		{
			ssgLeaf *l = new ssgVtxArray( GL_TRIANGLE_STRIP,
				vertices,
				normals,
				texCoords,
				colors,
				indices );
			l->setState ( state1 );
			branch->addKid( l );
			ssgLeaf *l2 = new ssgVtxArray( GL_TRIANGLE_STRIP,
				vertices,
				normals,
				texCoords,
				colors,
				indices2 );
			l2->setState ( state2 );
			branch->addKid( l2 );
		}
	}
}

WavingFlag::~WavingFlag()
{
}

void WavingFlag::animate( float time, float windVelocity )
{
	if ( windVelocity < 3.0 ) windVelocity = 3.0;
	if ( windVelocity > 25.0 ) windVelocity = 25.0;
	float r = 0.6f/windVelocity;  // half wave height
	float w = windVelocity/2.0f;  // angular acceleration
	float k = windVelocity*3.0f;  // wave number
	float d = (1.0f-windVelocity/25.0f)*(SG_PI/2.0f);

	int x,z;
	for ( z = size-1; z >= 0; z-- )
	{
		for ( x = 1; x < size; x++ )
		{
			int i = x*size + z;

			float *v = vertices->get( i );
			v[0] = ((float)x * (float)cos(d)) / (size - 1.0f);
			float a = k*(v[0]) - w*time;  // phase angle
			v[1] = r * (float)cos( a );
			v[0] -= v[1];
			v[2] = ((float)z - (float)x * (float)sin(d)) / (size - 1.0f);
			float *n = normals->get( i );
			n[0] = (float)cos(d);
			n[1] = v[1];
			//d *= 1.002;
		}
		k *= 0.9f;
	}
	if ( d > SG_PI/2.0 ) printf ("Problems\n");
	branch -> dirtyBSphere () ; 
}

ssgEntity *WavingFlag::getObject()
{
	return branch;
}

