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
 .scenery loader for SSG/PLIB
 ATG = ascii Terra Gear
 
 Written by Wolfram Kuss (Wolfram.Kuss@t-online.de) in May 2001

*/

#include  "ssgLocal.h"
#include "ssgLoaderWriterStuff.h" 
#include "ssgParser.h"
#include "../util/ul.h"


extern sgVec4 currentDiffuse;


static /* const */ ssgLoaderOptions* current_options = NULL ;
static class ssgTexCoordArray *linearListTCPFAV=NULL; // TCPFAV = TextureCoordinatesPerFaceAndVertex

static _ssgParserSpec parser_spec =
{
   "\r\n\t ",  // delim_chars_skipable
   NULL,       // delim_chars_non_skipable
   NULL,          // open_brace_chars
   NULL,          // close_brace_chars
   '"',        // quote_char. not used for scenery
   0,          // comment_char # is handled in this module, since it may contin the name of the material
	 NULL        // comment_string
} ;

   
static _ssgParser parser;
static ssgBranch* top_branch;

static int Ascii2Int(int &retVal, const char *token, const char* name )
// returns TRUE on success
{
  char *endptr;
  retVal = int(strtol( token, &endptr, 10));
	if ( (endptr == NULL) || (*endptr == 0))
    return TRUE;
	else
	{ parser.error("The field %s should contain an integer number but contains %s",name, token) ;
		return FALSE;
	}
}

static int Ascii2UInt(unsigned int &retVal, const char *token, const char* name )
// returns TRUE on success
{
  char *endptr;
  retVal = (unsigned int)(strtol( token, &endptr, 10));
	if ( (endptr == NULL) || (*endptr == 0))
    return TRUE;
	else
	{ parser.error("The field %s should contain an integer number but contains %s",name, token) ;
		return FALSE;
	}
}

// Fixme: have this only once.
static int Ascii2Float(SGfloat &retVal, const char *token, const char* name )
// returns TRUE on success
{
  char *endptr;
  retVal = SGfloat(strtod( token, &endptr));
	if ( (endptr == NULL) || (*endptr == 0))
    return TRUE;
	else
	{ parser.error("The field %s should contain an integer number but contains %s",name, token) ;
		return FALSE;
	}
}




#define MAX_NO_VERTICES_PER_FACE 1000

static class ssgLoaderWriterMesh _theMesh;





static int _ssgNoFaces, _ssgNoVertices, _ssgNoVertexNormals, _ssgNoVertexTC;

static char * _current_usemtl = NULL, * _last_usemtl = NULL;
static int _current_material_index = -1;

static char* parser_getLine()
// replaces parser.getLine, but handles '#'
// Some "comment lines" starting with '#' have important info like material/texture name inside
// if this function finds a #-line with a materialname, it sets _current_usemtl
{ char *token;
  token = parser.getLine(0);
	if ( token == NULL )
		return NULL;

	while ( token[0] == '#' )
	{ 
		if ( ulStrEqual ("usemtl" , parser.parseToken( 0 ))) // name has to be 0 
		{ char * usemtl = parser.parseToken( 0 );
		  if ( usemtl != NULL)
				if ( 0 != usemtl[0] )
				{
					if ( _current_usemtl  != NULL )
						delete _current_usemtl ;
					_current_usemtl = new char [ strlen ( usemtl ) + 1 ] ;
					strcpy ( _current_usemtl, usemtl ) ;
				}
		}
		token = parser.getLine(0);
		if ( token == NULL )
			return NULL;
	}
	return token;
}

static int parse()
{
	// ************* Init ************************
  char* token;
	_ssgNoFaces= 0;
	_ssgNoVertices = 0;
	_ssgNoVertexNormals = 0;
	_ssgNoVertexTC = 0;

	_theMesh.ReInit ();
	_theMesh.ThereAreNVertices(); 
	_theMesh.ThereAreNFaces(); 
	_theMesh.ThereAreNTCPFAV();
	_theMesh.ThereAreNMaterials();
	_theMesh.ThereAreNMaterialIndexes() ;
  
	token = parser_getLine();
	if ( token == NULL )
	{ parser.error("The file seems to be empty");
		return FALSE;
	}
	// ****** read vertices ************

	while ( 0 == strcmp( token, "v") ) 
	{ sgVec3 vert;
		
		if (! parser.parseFloat( vert[0], "vertex.x") )
	    return FALSE;
		if (! parser.parseFloat( vert[1], "vertex.y") )
	    return FALSE;
		if (! parser.parseFloat( vert[2], "vertex.z") )
	    return FALSE;
		_theMesh.addVertex(vert); // add to mesh
		_ssgNoVertices++;
		token = parser_getLine();
		if ( token == NULL )
		{ parser.error("There are only vertices in the scenery, no faces");
			return FALSE;
		}
	}
	// ****** read vertex normals ************
	while ( 0 == strcmp( token, "vn") )
	{ sgVec3 vert_norm;
		
		if (! parser.parseFloat( vert_norm[0], "vertex normal.x") )
	    return FALSE;
		if (! parser.parseFloat( vert_norm[1], "vertex normal.y") )
	    return FALSE;
		if (! parser.parseFloat( vert_norm[2], "vertex normal.z") )
	    return FALSE;
		//_theMesh.addVertex(vert);
		_ssgNoVertexNormals++;
		token = parser_getLine();
		if ( token == NULL )
		{ parser.error("There are only vertices in the scenery, no faces");
			return FALSE;
		}
	}
	/*if ( _ssgNoVertexNormals != _ssgNoVertices)
	{ parser.error("The number of vertex normals and vertices doesn't match");
		return FALSE;
	}*/
	// ***************** read texture coords. **********************
	// this format has a "linear" list of texture coords, that is indexed from the faces.
	while ( 0 == strcmp( token, "vt") )
	{ sgVec2 vert_tc;
		
		if (! parser.parseFloat( vert_tc[0], "vertex texture.x") )
	    return FALSE;
		if (! parser.parseFloat( vert_tc[1], "vertex texture.y") )
	    return FALSE;
		linearListTCPFAV->add(vert_tc);
		_ssgNoVertexTC++;
		token = parser_getLine();
		if ( token == NULL )
		{ parser.error("There are only vertices in the scenery, no faces");
			return FALSE;
		}
	}
	/*if ( _ssgNoVertexTC != _ssgNoVertices)
	{ parser.error("The number of texture coords and vertices doesn't match");
		return FALSE;
	}*/
	
	// ********* Read Faces **********
	int aiVertices[MAX_NO_VERTICES_PER_FACE], aiTCs[MAX_NO_VERTICES_PER_FACE];
	unsigned int nNoOfVerticesForThisFace;
	
	while ( 0 == strcmp( token, "f") )
	{ char *ptr, buffer[1024], *ptr2Slash;

		printf("%s\n",_current_usemtl);

		if ( !ulStrEqual (_current_usemtl, _last_usemtl))
		// ***************+ material has changed. ***********
		{ 
			// bring _last_usemtl up to date
			if ( _last_usemtl != NULL )
				delete _last_usemtl;
			_last_usemtl= new char [ strlen ( _current_usemtl ) + 1 ] ;
			strcpy ( _last_usemtl, _current_usemtl ) ;


			// create SimpleState

			ssgSimpleState * currentState = new ssgSimpleState () ; // (0) ?

			currentState -> setMaterial ( GL_AMBIENT, 0.5, 0.5, 0.5, 1.0);
			currentState -> setMaterial ( GL_DIFFUSE, 1.0, 1.0, 1.0, 1.0) ; // 0.8, 0.8, 1.0, 1.0f
			currentState -> setMaterial ( GL_SPECULAR, 1.0, 1.0, 1.0, 1.0);
			currentState -> setMaterial ( GL_EMISSION, 0.0, 0.0, 0.0, 1.0);
			currentState -> setShininess ( 20 ) ; // Fixme, NIV14: Is that correct?

			currentState -> disable ( GL_COLOR_MATERIAL ) ;
			//currentState -> enable ( GL_COLOR_MATERIAL ) ;
			//currentState -> setColourMaterial ( GL_AMBIENT_AND_DIFFUSE ) ;

			currentState -> enable  ( GL_LIGHTING       ) ;
			currentState -> setShadeModel ( GL_SMOOTH ) ;

			currentState  ->disable(GL_ALPHA_TEST); //needed?

			if ( FALSE ) //currentDiffuse[3] > 0.0f )
			{
				currentState -> disable ( GL_ALPHA_TEST ) ;
				currentState -> enable  ( GL_BLEND ) ;
				currentState -> setTranslucent () ;
			}
			else
			{
				currentState -> disable ( GL_BLEND ) ;
				currentState -> setOpaque () ;
			}
			currentState -> disable( GL_TEXTURE_2D );
			if (TRUE) // textured
			{	
				char * fileName =	new char [ strlen ( _current_usemtl ) + 5 ] ;
				strcpy ( fileName, _current_usemtl ) ;
				strcat( fileName, ".rgb" );
      
				currentState -> setTexture( current_options -> createTexture( fileName ) );
			  delete fileName;
				currentState -> enable( GL_TEXTURE_2D );
			}
			// add SimpleState
			_theMesh.addMaterial( &currentState );

			// bring _current_material_index up to date
			_current_material_index ++;

		} // if new material

	  // ************* parse vertex and TC indices of this face ******************
		nNoOfVerticesForThisFace = 0;
		ptr = parser.parseToken( "vertex index");
		while ( ptr != NULL )
		{ strncpy(buffer, ptr, 1024);
			ptr2Slash = strchr(buffer, '/');
			if ( ptr2Slash != NULL)
				*ptr2Slash = 0;
			ptr2Slash++;
					  
			if ( ! Ascii2Int( aiVertices[ nNoOfVerticesForThisFace ], buffer, "vertex index"))
				return FALSE;
			if ( ! Ascii2Int( aiTCs[ nNoOfVerticesForThisFace ], ptr2Slash, "texture coord. index"))
				return FALSE;
			nNoOfVerticesForThisFace++;
			assert(nNoOfVerticesForThisFace<MAX_NO_VERTICES_PER_FACE);
			ptr = parser.parseToken( 0 ); // name has to be 0 
			if ( parser.eol ) break;
		}
		
		_ssgNoFaces++;
////////////////////
		// use array aiVertices
		(nNoOfVerticesForThisFace, aiTCs); 

		unsigned int j;
		ssgTexCoordArray * sca = new ssgTexCoordArray ();
		sca->ref();
		for(j=0;j<nNoOfVerticesForThisFace;j++)
			sca->add(linearListTCPFAV->get(aiTCs[j]));

		// ****** add face to mesh *****
		_theMesh.addTCPFAV ( &sca ) ;
		_theMesh.AddFaceFromCArray(nNoOfVerticesForThisFace, aiVertices); 
		_theMesh.addMaterialIndex ( _current_material_index ) ;
		


		token = parser_getLine();
		if ( token == NULL )
			break;
	} // ******* end of reading faces *****
	if ( token != NULL )
		// Fixme
		printf("f expected, got %s\n", token);
	
	if (! parser.eof )
		// Fixme
		printf("Warning: no eof\n");
	
	
	// kludge. We need a state for add2SSG:

	ssgSimpleState * ss = new ssgSimpleState () ; // (0) ?
	ss -> setMaterial ( GL_AMBIENT, 0.5, 0.5, 0.5, 1.0);
	ss -> setMaterial ( GL_DIFFUSE, 1.0, 1.0, 1.0, 1.0) ; // 0.8, 0.8, 1.0, 1.0f
	ss -> setMaterial ( GL_SPECULAR, 1.0, 1.0, 1.0, 1.0);
	ss -> setMaterial ( GL_EMISSION, 0.0, 0.0, 0.0, 1.0);
	ss -> setShininess ( 20 ) ; // Fixme, NIV14: Is that correct?

	ss -> disable ( GL_COLOR_MATERIAL ) ;
	//ss -> enable ( GL_COLOR_MATERIAL ) ;
	//ss -> setColourMaterial ( GL_AMBIENT_AND_DIFFUSE ) ;

	ss -> enable  ( GL_LIGHTING       ) ;
	ss -> setShadeModel ( GL_SMOOTH ) ;

	ss  ->disable(GL_ALPHA_TEST); //needed?

	if ( FALSE ) //currentDiffuse[3] > 0.0f )
	{
		ss -> disable ( GL_ALPHA_TEST ) ;
		ss -> enable  ( GL_BLEND ) ;
		ss -> setTranslucent () ;
	}
	else
	{
		ss -> disable ( GL_BLEND ) ;
		ss -> setOpaque () ;
	}
	ss -> disable( GL_TEXTURE_2D );
  _theMesh.checkMe(); // For debug
	_theMesh.add2SSG(
		ss, // super kludge. NIV135
		current_options,
		top_branch);

  return TRUE ;
}


ssgEntity *ssgLoadATG ( const char *fname, const ssgLoaderOptions* options )
{
  ssgSetCurrentOptions ( (ssgLoaderOptions*)options ) ;
  current_options = ssgGetCurrentOptions () ;

  top_branch = new ssgBranch ;
	_current_usemtl = NULL;
	_last_usemtl = NULL;
  _current_material_index = -1;


	if ( !parser.openFile( fname, &parser_spec ))
	{
    delete top_branch ;
		return 0;
  }
  linearListTCPFAV=new ssgTexCoordArray();
  if ( !parse() )
  {
		delete linearListTCPFAV;
		delete top_branch ;
		top_branch = 0 ;
  }
	delete linearListTCPFAV;
//  parse_free();
  parser.closeFile();

  return top_branch ;
}
