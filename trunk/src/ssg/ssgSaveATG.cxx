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
/* 
 .scenery writer for SSG/PLIB
 ATG = ascii Terra Gear
 
 Written by Wolfram Kuss (Wolfram.Kuss@t-online.de) in May 2001

*/

#include <stdio.h>
#include "ssgLocal.h"
#include "ssgLoaderWriterStuff.h"



int ssgSaveATG( const char* fname, ssgEntity *ent ) {
  FILE *fd = fopen ( fname, "w" ) ;
	int i;
 
  if ( fd == NULL ) {
    ulSetError ( UL_WARNING, "ssgSaveATG: Failed to open '%s' for writing", 
		 fname ); 
    return FALSE ;
  }

  ssgVertexArray* vertices = new ssgVertexArray();
  ssgIndexArray*  indices  = new ssgIndexArray ();

  fprintf(fd, "# Created by ssgSaveATG. Original graph structure was:\n");
  ent->print(fd, "#", 0);

  sgMat4 ident;
  sgMakeIdentMat4( ident );
	ssgSimpleStateArray ssa; // = new ssgSimpleStateArray();
	ssgIndexArray*  materialIndices = new  ssgIndexArray();
  ssgAccumVerticesAndFaces( ent, ident, vertices, indices, /* epsilon = */ -1, &ssa,
		                  materialIndices);
  for (i = 0; i < vertices->getNum(); i++) {
    fprintf(fd, "v %f %f %f\n", vertices->get(i)[0],
															vertices->get(i)[1],
															vertices->get(i)[2]);
  }
  for (i = 0; i < vertices->getNum(); i++) 
    fprintf(fd, "vn     1.0000     1.0000     1.0000\n"); // Fixme: Hm, strange normal ...

  fprintf(fd, "vt 0.0 0.0\n");
  fprintf(fd, "vt 0.0 0.0\n");
  int runningIndex=0;
  for (i = 0; i < indices->getNum(); i += 3) {

		// output material
		int matIndex = *(materialIndices->get(runningIndex++));
		if ( matIndex >= 0 )
		{	ssgSimpleState * ss = ssa.get( matIndex );
		  if ( ss->getTextureFilename() != NULL)
			{ // remove .rgb
				char *s1, *s2, * s = new char [ strlen(ss->getTextureFilename()) +1 ];
				assert ( s != NULL );
			  strcpy(s, ss->getTextureFilename());
				char *p = strchr(s, '.');
				if ( p != NULL ) 
					*p = 0;
				s2 = s;
				s1 = strrchr(s, '/');
				if ( s1 != NULL )
					s2 = ++s1;
				s1 = strrchr(s2, '\\');
				if ( s1 != NULL )
					s2 = ++s1;
				fprintf(fd, "# usemtl %s\n", s2); 
			  delete [] s;
			}
 
		}
    // output face
		fprintf(fd, "f %d/1 %d/1 %d/1\n", *indices->get(i    ) ,
															  *indices->get(i + 1) ,
															  *indices->get(i + 2) );
  }
	assert ( runningIndex == materialIndices->getNum() );
	delete materialIndices;
  
  fclose( fd ) ;

  delete vertices;
  delete indices ;
 
  return TRUE;
}
