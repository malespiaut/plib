/*
     PLIB - A Suite of Portable Game Libraries
     Copyright (C) 1998,2002  Steve Baker
 
     This library is free software; you can redistribute it and/or
     modify it under the terms of the GNU Library General Public
     License as published by the Free Software Foundation; either
     version 2 of the License, or (at your option) any later version.
 
     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Library General Public License for more details.
 
     You should have received a copy of the GNU Library General Public
     License along with this library; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 
     For further information visit http://plib.sourceforge.net

*/

//
// .ASC loader for SSG/PLIB
// ASC = ascii files, used for example by 3DS R4 (the DOS version)
// Written by Wolfram Kuss (Wolfram.Kuss@t-online.de) on 3.10.2004
//
#include  "ssgLocal.h"
#include "ssgLoaderWriterStuff.h" 
#include "ssgParser.h"

#define u32 unsigned int

// These functions return TRUE on success
typedef int AscHandlerFunctionType();


static char *globEmpty="";

static ssgBranch *curr_branch_;

struct AscEntityType
{
  const char * sName;
	AscHandlerFunctionType *HandleEntity;
  bool appearsInsideAMesh;
} ;

void AscLinePreProcessor(char *line)
// there are lines like this
//                                     Page 2
// in *.asc files. They should be completely ignored.

{ char *p = line;
  while((*p == ' ') || (*p == 9))
    p++;
  // p points to the first word (or the line end) now.
  if(0 == ulStrNEqual(p, "Page", strlen("Page")))
  {
    // Since this loader ignores the line structure, 
    // we can also ignore the \n or \r and just "delete" the whole line
    line[0] = 0;
  }
  if(0 == ulStrNEqual(p, "Camera", strlen("Camera")))
    line[0] = 0;
}


static /* const */ ssgLoaderOptions* current_options = NULL ;

static _ssgParserSpec parser_spec =
{
   "\r\n\t ",  // delim_chars_skipable
   "=:",       // delim_chars_non_skipable so that things like "red=0.5" or "vertex 0:" work
   AscLinePreProcessor,      // pre_processor
   "{",          // open_brace_chars - unused
   "}",          // close_brace_chars - unused
   '"',        // quote_char
   '#',          // comment_char - I am not sure there are comments and if so which char starts them
	 "//"        // comment_string - I am not sure there are comments and if so which string starts them
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


struct MeshStatusType {
  bool isvalid; // rest of this is valid, IOW I am inside reading a mesh
  char *sName;
  bool mapped;
  bool materialed_but_not_mapped; // only defined if mapped = FALSE
  bool hidden; // unused
  int maxVertex, maxFace, curVertex, curFace;
  void set_sName(char *s);
  // material + smoothing is per poly!!!
} MeshStatus;


void MeshStatusType::set_sName(char *s)
{
  if(sName)
    delete [] sName;
  sName = new char[strlen(s)+1];
  strcpy(sName, s);
}

void CheckIndex(int &i, int max)
{
  if((i<0) || (i>=max))
  { fprintf(stderr, "Mesh '%s': Index %d is not in the range 0 to %d!", MeshStatus.sName, i, max);
    i = 0;
  }
}
 
void CheckWhetherMeshHasEnded();
static int HandleMeshMaterialList(const char *firstToken);
static int HandleMaterialOld(const char *firstToken);
static int HandleTextureFileName(const char *firstToken);

static int HandleAmbient();
static int HandleSolid();
static int HandleNamed();
static int HandleTriMesh();
static int HandleHidden();
static int HandleMapped();
static int HandleVertex();
static int HandleFace();
static int HandleMaterial();
static int HandleSmoothing();
static int HandleDirect();
static int HandlePosition();
static int HandleLight();
static int HandleSpotlight();
static int HandleHotspot ();
static int HandleFalloff ();
static int HandleGlobal ();
// unneeded static int HandleCamera();
static int HandleTarget();
static int HandleBank();
static int HandleNear();

static AscEntityType aEntities[] =
{
  { "Spotlight", HandleSpotlight, FALSE},
  { "Hotspot", HandleHotspot , FALSE},
  { "Falloff", HandleFalloff , FALSE},
  { "Global", HandleGlobal , FALSE},
// unneeded   { "Camera", HandleCamera, FALSE},
  { "Target", HandleTarget, FALSE},
  { "Bank", HandleBank, FALSE},
  { "Near", HandleNear, FALSE},
  { "Ambient", HandleAmbient, FALSE},
  { "Solid", HandleSolid, FALSE},
  { "Named", HandleNamed, FALSE},
  { "Tri-mesh,", HandleTriMesh, TRUE},
  { "Hidden", HandleHidden, TRUE},
  { "Mapped", HandleMapped, TRUE},
  { "Vertex", HandleVertex, TRUE},
  { "Face", HandleFace, TRUE},
  { "Material", HandleMaterial, TRUE},
  { "Smoothing", HandleSmoothing},
  { "Direct", HandleDirect, FALSE},
  { "Position", HandlePosition, FALSE},
  { "Light", HandleLight, FALSE},
  { NULL, NULL}
};


static int ParseEntity(char *token)
// called recursively
// copied from ssgLoadX; A bit (?) of overkill for *.asc.
{ int i=0;

	while(aEntities[i].sName!=NULL)
	{ if (!strcmp(token,aEntities[i].sName))
		{	if (aEntities[i].HandleEntity != NULL)
			{	/*char *sNextToken, *sName=globEmpty;
				sNextToken=parser.getNextToken(0);
				if ( parser.eof ) 
				{ parser.error("unexpected end of file\n");
					return FALSE;
				}*/
        if(!aEntities[i].appearsInsideAMesh)  
          CheckWhetherMeshHasEnded();
				if (!aEntities[i].HandleEntity(/*sNextToken*/))
					return FALSE;
			}
			else
			{
				parser.error("I am sorry, but Entity-type '%s' is not yet implemented.", aEntities[i].sName);
				return FALSE ;
			}
				
			break;
		}
		i++;
	}
	if (aEntities[i].sName==NULL)
	{
		parser.error("unexpected token %s", token);
		return FALSE ;
	}
	return TRUE;
}

#define MAX_ASC_MATERIALS 1024

struct AscMaterialType {
  char *sName;
  ssgSimpleState *ss;  
} AscMaterials[MAX_ASC_MATERIALS];
static int noOfAscmaterials=0;


#define MAX_NO_VERTICES_PER_FACE 1000

static class ssgLoaderWriterMesh currentMesh;

static ssgSimpleState *currentState;

// we only need one of these, since obviously the texture name is the only materialproperty in asc:
static ssgSimpleState *untexturedState = NULL; 

extern sgVec4 currentDiffuse;


static int HandleDirect()
// example line:
// "Direct light"
{
  parser.expectNextToken("light");
	return TRUE;
}
/* not needed since handled via the PreProcessor
static int HandleCamera()
// example line:
// Camera (51.944435mm)
{
  float Dummy;
  parser.expectNextToken("(");
	if (!parser.getNextFloat(Dummy, "Camera")) return FALSE;
	parser.expectNextToken("mm)");
	return TRUE;
}
*/
static int HandleTarget()
// example line:
// Target:  X:7.573029 Y:-989.009766 Z:6.24827()
{
  float Dummy;
  parser.expectNextToken(":");
	parser.expectNextToken("X");
	parser.expectNextToken(":");
	if (!parser.getNextFloat(Dummy, "Target X")) return FALSE;
	parser.expectNextToken("Y");
	parser.expectNextToken(":");
	if (!parser.getNextFloat(Dummy, "Target Y")) return FALSE;
	parser.expectNextToken("Z");
	parser.expectNextToken(":");
	if (!parser.getNextFloat(Dummy, "Target Z")) return FALSE;
	return TRUE;
}

static int HandleBank()
// example line:
// Bank angle: 0 degrees
{
  float Dummy;
  parser.expectNextToken("angle");
	parser.expectNextToken(":");
	if (!parser.getNextFloat(Dummy, "Bank angle")) return FALSE;
	parser.expectNextToken("degrees");
	return TRUE;
}

static int HandleNear()
// example line:
// Near 0 Far 1000
{
  float Dummy;
  if (!parser.getNextFloat(Dummy, "Near")) return FALSE;
	parser.expectNextToken("Far");
	if (!parser.getNextFloat(Dummy, "Far")) return FALSE;
	return TRUE;
}

static int HandlePosition()
// example line:
// "Position:  X:-534.261475 Y:484.437958 Z:203.464706"
{
  float Dummy;
  parser.expectNextToken(":");
	parser.expectNextToken("X");
	parser.expectNextToken(":");
	if (!parser.getNextFloat(Dummy, "light X")) return FALSE;
	parser.expectNextToken("Y");
	parser.expectNextToken(":");
	if (!parser.getNextFloat(Dummy, "light Y")) return FALSE;
	parser.expectNextToken("Z");
	parser.expectNextToken(":");
	if (!parser.getNextFloat(Dummy, "light Z")) return FALSE;
	return TRUE;

}

static int HandleSpotlight()
// example line:
// "Spotlight to:  X:-1.4 Y:-0.607239 Z:-16.277397"
{
 float Dummy;
  parser.expectNextToken("to");
	parser.expectNextToken(":");
	parser.expectNextToken("X");
	parser.expectNextToken(":");
	if (!parser.getNextFloat(Dummy, "Spotlight to X")) return FALSE;
	parser.expectNextToken("Y");
	parser.expectNextToken(":");
	if (!parser.getNextFloat(Dummy, "Spotlight to Y")) return FALSE;
	parser.expectNextToken("Z");
	parser.expectNextToken(":");
	if (!parser.getNextFloat(Dummy, "Spotlight to Z")) return FALSE;
	return TRUE;
}

static int HandleHotspot ()
// example line:
// "Hotspot size: 54 degrees"
{
  float Dummy;
  parser.expectNextToken("size");
	parser.expectNextToken(":");
	if (!parser.getNextFloat(Dummy, "Hotspot size")) return FALSE;
	parser.expectNextToken("degrees");
 	return TRUE;
}

static int HandleFalloff ()
// example line:
// "Falloff size: 54.5 degrees"
{
  float Dummy;
  parser.expectNextToken("size");
	parser.expectNextToken(":");
	if (!parser.getNextFloat(Dummy, "Falloff size")) return FALSE;
	parser.expectNextToken("degrees");
	return TRUE;
}

static int HandleGlobal ()
// example line:
// "Global Shadow"
{
  parser.expectNextToken("Shadow");
	return TRUE;
}


static int HandleLight()
// example line:
// "Light color: Red=1 Green=1 Blue=1"
{
  float Dummy;
  parser.expectNextToken("color");
	parser.expectNextToken(":");
	parser.expectNextToken("Red");
	parser.expectNextToken("=");
	if (!parser.getNextFloat(Dummy, "Light red")) return FALSE;
	parser.expectNextToken("Green");
	parser.expectNextToken("=");
	if (!parser.getNextFloat(Dummy, "Light Green")) return FALSE;
	parser.expectNextToken("Blue");
	parser.expectNextToken("=");
	if (!parser.getNextFloat(Dummy, "Light Blue")) return FALSE;
	return TRUE;
}



static int HandleAmbient()
// example line:
// "Ambient light color: Red=0.180392 Green=0.180392 Blue=0.180392"
{
  float Dummy;
	parser.expectNextToken("light");
	parser.expectNextToken("color");
  parser.expectNextToken(":");
	parser.expectNextToken("Red");
  parser.expectNextToken("=");
	if (!parser.getNextFloat(Dummy, "Ambient red")) return FALSE;
	parser.expectNextToken("Green");
  parser.expectNextToken("=");
	if (!parser.getNextFloat(Dummy, "Ambient green")) return FALSE;
	parser.expectNextToken("Blue");
  parser.expectNextToken("=");
	if (!parser.getNextFloat(Dummy, "Ambient blue")) return FALSE;
	return TRUE;
}

static int HandleSolid()
// example line:
// "Solid background color: Red=0.345098 Green=0.6 Blue=0.807843"
{
  parser.expectNextToken("background");
	parser.expectNextToken("color");
  parser.expectNextToken(":");
	parser.expectNextToken("Red");
  parser.expectNextToken("=");
  float Dummy;
	if (!parser.getNextFloat(Dummy, "Background red")) return FALSE;
	parser.expectNextToken("Green");
  parser.expectNextToken("=");
	if (!parser.getNextFloat(Dummy, "Background green")) return FALSE;
	parser.expectNextToken("Blue");
  parser.expectNextToken("=");
	if (!parser.getNextFloat(Dummy, "Background blue")) return FALSE;
	return TRUE;
}

static int HandleNamed()
// example line:
// "Named object: "EX1_A""
{
  parser.expectNextToken("object");
  parser.expectNextToken(":");
//  assert(MeshStatus.isvalid); comes before trimesh !
  char *s; // cleaned up sName
  s = parser.getNextToken("Mesh name");
  if(*s=='"')
    s++;
  if(s[strlen(s)-1]=='"')
    s[strlen(s)-1]=0;

  MeshStatus.set_sName(s);

	return TRUE;
}

static int HandleTriMesh()
// example line:
// "Tri-mesh, Vertices: 22     Faces: 18"
{
  parser.expectNextToken("Vertices");
  parser.expectNextToken(":");
  if (!parser.getNextInt(MeshStatus.maxVertex, "Number vertices")) return FALSE;
	parser.expectNextToken("Faces");
  parser.expectNextToken(":");
  if (!parser.getNextInt(MeshStatus.maxFace, "Number faces")) return FALSE;
  if(MeshStatus.isvalid)
  {
    fprintf(stderr, "MeshStatus.isvalid is TRUE. Probably, in the mesh preceding '%s', there are more faces than predicted", MeshStatus.sName);
    return FALSE;
  }
  MeshStatus.isvalid = TRUE;
  MeshStatus.curVertex = -1;
  MeshStatus.curFace = -1;
  MeshStatus.mapped = FALSE;
  MeshStatus.materialed_but_not_mapped = FALSE;
  MeshStatus.hidden = FALSE; // unused
  noOfAscmaterials=0;
  currentState = untexturedState; 

  
	currentMesh.reInit ();
	currentMesh.setName( MeshStatus.sName );
	currentMesh.createVertices( MeshStatus.maxVertex );
  currentMesh.createFaces( MeshStatus.maxFace );

 	return TRUE;
}

static int HandleHidden()
// example line:
// "Hidden"
{
  assert(MeshStatus.isvalid);
  MeshStatus.hidden = TRUE; // unused
  return TRUE;
}

static int HandleMapped()
// example line:
// "Mapped"
{
  assert(MeshStatus.isvalid);
	currentMesh.createPerVertexTextureCoordinates2( MeshStatus.maxVertex ) ;
  currentMesh.createMaterials();
  currentMesh.createMaterialIndices( MeshStatus.maxFace );
  MeshStatus.mapped = TRUE; 
	return TRUE;
}

static int HandleSmoothing()
// example line:
// "Smoothing:  1"
{
  int i;
  parser.expectNextToken(":");
	if (!parser.getNextInt(i, "Smoothing")) return FALSE;
	return TRUE;	
}


int SetOrGetMaterial(char *sMatName)
// if exists, gets the index. Else sets sName and creates the ss
{
  char *s; // cleaned up sName
  s = sMatName;
  if(*s=='"')
    s++;
  if(s[strlen(s)-1]=='"')
    s[strlen(s)-1]=0;
  // **** search for it ****
  for(int i=0; i<noOfAscmaterials; i++)
    if(0==ulStrEqual(s, AscMaterials[i].sName))
      return i;
  // **** not found - create it ****
  AscMaterials[noOfAscmaterials].sName = new char[strlen(s)+1];
  strcpy(AscMaterials[noOfAscmaterials].sName, s);
  AscMaterials[noOfAscmaterials++].ss = currentState  = new ssgSimpleState () ;
  // set "defaults"
	currentState->setOpaque();
	currentState->disable(GL_BLEND);
	currentState->disable(GL_ALPHA_TEST);
	currentState->disable(GL_TEXTURE_2D);
	currentState->enable(GL_COLOR_MATERIAL);
	currentState->enable(GL_LIGHTING);
	currentState->setShadeModel(GL_SMOOTH);
	currentState->setMaterial(GL_AMBIENT , 0.7f, 0.7f, 0.7f, 1.0f);
	currentState->setMaterial(GL_DIFFUSE , 0.7f, 0.7f, 0.7f, 1.0f);
	currentState->setMaterial(GL_SPECULAR, 1.0f, 1.0f, 1.0f, 1.0f);
	currentState->setMaterial(GL_EMISSION, 0.0f, 0.0f, 0.0f, 1.0f);
	currentState->setShininess(50);
  // set texture
  currentState -> setTexture( current_options -> createTexture( s ) );
  currentState -> setName(s);
  currentState -> enable( GL_TEXTURE_2D );
  currentMesh.addMaterial(&currentState);
  return noOfAscmaterials-1;
}


static int HandleMaterial()
// example line:
// "Material:"BEXHUST""
{
  parser.expectNextToken(":");
  if(!MeshStatus.mapped && !MeshStatus.materialed_but_not_mapped)
  {
    currentMesh.createMaterials();
    currentMesh.createMaterialIndices( MeshStatus.maxFace );
    MeshStatus.materialed_but_not_mapped = TRUE;
  }


	char *sMaterialName = parser.getNextToken("Material name");
  int i = SetOrGetMaterial(sMaterialName);
  currentMesh.addMaterialIndex(i);

	return TRUE;
}

static int HandleVertex()
// example line:
// "Vertex list:"
// *or* (!!)
// "Vertex 0:  X:115.710701     Y:180.000305     Z:-60.796242     U:0.048042     V:0.996284"
{
  char *sNextToken = parser.peekAtNextToken( "vertex list?" );
  if(0 == ulStrEqual("list", sNextToken))
  {
    parser.expectNextToken("list");
    parser.expectNextToken(":");
  }
  else
  {
    sgVec3 vert;
    int i;
    if (!parser.getNextInt(i, "vertex index")) return FALSE;
    assert(MeshStatus.isvalid);
    assert(MeshStatus.maxVertex > i);
    assert(MeshStatus.curVertex == i-1);
    MeshStatus.curVertex ++;
    parser.expectNextToken(":");
    parser.expectNextToken("X");
    parser.expectNextToken(":");
	  if (!parser.getNextFloat(vert[0], "X")) return FALSE;
    parser.expectNextToken("Y");
    parser.expectNextToken(":");
    if (!parser.getNextFloat(vert[1], "Y")) return FALSE;
    parser.expectNextToken("Z");
    parser.expectNextToken(":");
    if (!parser.getNextFloat(vert[2], "Z")) return FALSE;
    if(MeshStatus.mapped)
    {
      // read texture coordinates    
		  sgVec2 tv;
      parser.expectNextToken("U");
      parser.expectNextToken(":");
      if (!parser.getNextFloat(tv[0], "U")) return FALSE;
      parser.expectNextToken("V");
      parser.expectNextToken(":");
      if (!parser.getNextFloat(tv[1], "V")) return FALSE;
		  currentMesh.addPerVertexTextureCoordinate2( tv ) ;
    }
		currentMesh.addVertex(vert);
  }
		
	return TRUE;  
}

static int HandleFace()
// example line:
// "Face list:"
// *or* (!!)
// "Face 0:    A:0 B:10 C:7 AB:1 BC:1 CA:1"
{
  char *sNextToken = parser.peekAtNextToken( "Face list?" );
  if(0 == ulStrEqual("list", sNextToken))
  {
    parser.expectNextToken("list");
    parser.expectNextToken(":");
  }
  else
  {
    int iFace, AB, _BC, CA;
    int aiVertices[3];
	  if (!parser.getNextInt(iFace, "Face index")) return FALSE;
    assert(MeshStatus.isvalid);
    assert(MeshStatus.maxFace > iFace);
    assert(MeshStatus.curFace == iFace-1);
    MeshStatus.curFace ++;
    parser.expectNextToken(":");
    parser.expectNextToken("A");
    parser.expectNextToken(":");
	  if (!parser.getNextInt(aiVertices[0], "A")) return FALSE;
    CheckIndex(aiVertices[0], MeshStatus.maxVertex);
    parser.expectNextToken("B");
    parser.expectNextToken(":");
	  if (!parser.getNextInt(aiVertices[1], "B")) return FALSE;
    CheckIndex(aiVertices[1], MeshStatus.maxVertex);
    parser.expectNextToken("C");
    parser.expectNextToken(":");
	  if (!parser.getNextInt(aiVertices[2], "C")) return FALSE;
    CheckIndex(aiVertices[2], MeshStatus.maxVertex); 
    parser.expectNextToken("AB");
    parser.expectNextToken(":");
	  if (!parser.getNextInt(AB, "AB")) return FALSE;
    parser.expectNextToken("BC");
    parser.expectNextToken(":");
	  if (!parser.getNextInt(_BC, "BC")) return FALSE; // "BC" is already used :-/
    parser.expectNextToken("CA");
    parser.expectNextToken(":");
	  if (!parser.getNextInt(CA, "CA")) return FALSE;
    currentMesh.addFaceFromIntegerArray(3, aiVertices); 
  }
  return TRUE;
}


// end todo ##############################################################



/////////////////////////////////////////////////// old
// start alt todo ##############################################################




static int HandleTextureFileName(const char *firstToken)
{/*
	  TextureFilename {
    "../image/box_top.gif";
   } #TextureFilename
 */
  char *filename_ptr, *filename = new char [ strlen(firstToken)+1 ] ;
	assert(filename!=NULL);
  strcpy ( filename, firstToken ) ;
	filename_ptr = filename ;
	
	if ( filename_ptr[0] == '"' ) 
		filename_ptr++;
	if (filename_ptr[strlen(filename_ptr)-1] == '"')
		filename_ptr[strlen(filename_ptr)-1] = 0;
  currentState -> setTexture( current_options -> createTexture( filename_ptr ) );
  currentState -> enable( GL_TEXTURE_2D );


	parser.expectNextToken(";");
	parser.expectNextToken("}");
	delete [] filename;
	return TRUE;
}

static int HandleMaterialOld(const char *firstToken)
// return TRUE on success
{ SGfloat power;
  int bFoundTextureFileName = FALSE;
	sgVec4 specularColour, EmissiveColour;

	// read body
	if (! Ascii2Float(currentDiffuse[0], firstToken, "Facecolour R"))
		return FALSE;

	parser.expectNextToken(";");
	if (!parser.getNextFloat(currentDiffuse[1], "Facecolour G")) return FALSE;
	parser.expectNextToken(";");
	if (!parser.getNextFloat(currentDiffuse[2], "Facecolour B")) return FALSE;
	parser.expectNextToken(";");
	if (!parser.getNextFloat(currentDiffuse[3], "Facecolour A")) return FALSE;
	parser.expectNextToken(";");
	parser.expectNextToken(";");
	if (!parser.getNextFloat(power, "power")) return FALSE;
	parser.expectNextToken(";");
	if (!parser.getNextFloat(specularColour[0], "Specular R")) return FALSE;
	parser.expectNextToken(";");
	if (!parser.getNextFloat(specularColour[1], "Specular G")) return FALSE;
	parser.expectNextToken(";");
	if (!parser.getNextFloat(specularColour[2], "Specular B")) return FALSE;
	specularColour[3] = 0.0;
	parser.expectNextToken(";");
	parser.expectNextToken(";");
	if (!parser.getNextFloat(EmissiveColour[0], "Emissive R")) return FALSE;
	parser.expectNextToken(";");
	if (!parser.getNextFloat(EmissiveColour[1], "Emissive G")) return FALSE;
	parser.expectNextToken(";");
	if (!parser.getNextFloat(EmissiveColour[2], "Emissive B")) return FALSE;
	EmissiveColour[3] = 0.0;
	parser.expectNextToken(";");
	parser.expectNextToken(";");

	// create SimpleState

  currentState = new ssgSimpleState () ;

//  currentState -> setMaterial ( GL_AMBIENT, mat -> amb ) ;
  currentState -> setMaterial ( GL_DIFFUSE, currentDiffuse) ;
  currentState -> setMaterial ( GL_SPECULAR, specularColour) ;
  currentState -> setMaterial ( GL_SPECULAR, specularColour[0], 
		                      specularColour[1], specularColour[2], currentDiffuse[3] ) ;
  currentState -> setMaterial ( GL_EMISSION, EmissiveColour[0], 
		                      EmissiveColour[1], EmissiveColour[2], currentDiffuse[3] ) ;
	
	currentState -> setShininess ( power ) ; // Fixme, NIV14: Is that correct?

  currentState -> enable ( GL_COLOR_MATERIAL ) ;
  currentState -> setColourMaterial ( GL_AMBIENT_AND_DIFFUSE ) ;

  currentState -> enable  ( GL_LIGHTING       ) ;
  currentState -> setShadeModel ( GL_SMOOTH ) ;

  if ( currentDiffuse[3] > 0.0f )
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



	while(TRUE)
	{ char *nextToken =parser.getNextToken(0);
	  if (0==strcmp("}", nextToken))
		{ currentMesh.addMaterial( &currentState );
			return TRUE; // Material is finished. success
		}
		
		if ( 0!= strcmp("TextureFilename", nextToken) )
		{ parser.error("TextureFilename expected!\n");
			return FALSE; 
		}
		if ( bFoundTextureFileName )
		{ parser.error("Only one TextureFileName per Material please!\n");
			return FALSE; 
		}
		if (!ParseEntity(nextToken)) // read "TextureFileName"
			return FALSE;
		bFoundTextureFileName = TRUE;
	}
	return TRUE; //lint !e527
}


static int HandleMeshMaterialList(const char *firstToken)
{
	u32 i, nFaceIndexes, nMaterialsRead = 0, nMaterials;
	  
	if (! Ascii2UInt(nMaterials, firstToken, "nMaterials"))
		return FALSE;

	parser.expectNextToken(";");
	currentMesh.createMaterials( nMaterials );
	if (!parser.getNextUInt(nFaceIndexes, "number of Face Indexes"))
		return FALSE;
	currentMesh.createMaterialIndices( nFaceIndexes ) ;
	parser.expectNextToken(";");


	if ( nFaceIndexes > currentMesh.getNumFaces())
	{ parser.error("No of face indexes of materiallist (%d) is greater than then no "
	            "of faces (%d)!\n" 
							"Therefore the material list is ignored!",
							( int ) nFaceIndexes, ( int ) currentMesh.getNumFaces());
	  return TRUE; // go on parsing
	}
	if ( nFaceIndexes > currentMesh.getNumFaces())
	  parser.message("Informational: No of face indexes of materiallist (%d) is less than then no "
	            "of faces (%d)\n" ,
							( int ) nFaceIndexes, ( int ) currentMesh.getNumFaces());
	for ( i=0 ; i<nFaceIndexes ; i++ )
	{
		int iIndex, j;
		char *ptr;
		if (!parser.getNextInt(iIndex, "Face index"))
			return FALSE;
		currentMesh.addMaterialIndex ( iIndex ) ;
		// I don't quite know why, but different .X files I have have a 
		// different syntax here, some have one ";" and some two.
		// Therefore, the following code
		for (j=0;j<2;j++)
		{ ptr = parser.peekAtNextToken( "," );
		  if ( strlen(ptr) == 1)
				if ( (ptr[0]==',') || (ptr[0]==';') )
				{ ptr = parser.getNextToken( "," ); // ignore this token
				}
  
		}
	}
	while(TRUE)
	{ char *nextToken =parser.getNextToken(0);
	  if (0==strcmp("}", nextToken))
		{ if ( nMaterialsRead < nMaterials )
		    parser.error("Too few Materials!\n");
			//else	parser.error("Success! MeshMaterialList!\n");
			return TRUE; // Mesh is finished. success
		}
		if ( 0!= strcmp("Material", nextToken) )
		{ parser.error("Material expected!\n");
			return FALSE; 
		}
		if ( nMaterialsRead >= nMaterials )
		{ parser.error("Too many Materials!\n");
			return FALSE; 
		}
		if (!ParseEntity(nextToken)) // read "Material"
			return FALSE;
		nMaterialsRead++;
	}
	return TRUE; //lint !e527
}



// end alt todo ##############################################################

void CheckWhetherMeshHasEnded()   
{ if(MeshStatus.isvalid &&
     (MeshStatus.curFace == MeshStatus.maxFace-1) &&
     (MeshStatus.curVertex == MeshStatus.maxVertex-1))
  {
    // end mesh PART FROM MATREEIAL AND SMOOTH
    //u32 i, j, nNoOfVertices, nNoOfVerticesForThisFace, nNoOfFaces;
	 
	  
	  if ( currentState == NULL )
	  {	currentState = new ssgSimpleState();
	    currentState->setOpaque();
		  currentState->disable(GL_BLEND);
		  currentState->disable(GL_ALPHA_TEST);
		  currentState->disable(GL_TEXTURE_2D);
		  currentState->enable(GL_COLOR_MATERIAL);
		  currentState->enable(GL_LIGHTING);
		  currentState->setShadeModel(GL_SMOOTH);
		  currentState->setMaterial(GL_AMBIENT , 0.7f, 0.7f, 0.7f, 1.0f);
		  currentState->setMaterial(GL_DIFFUSE , 0.7f, 0.7f, 0.7f, 1.0f);
		  currentState->setMaterial(GL_SPECULAR, 1.0f, 1.0f, 1.0f, 1.0f);
		  currentState->setMaterial(GL_EMISSION, 0.0f, 0.0f, 0.0f, 1.0f);
		  currentState->setShininess(50);
  /*
		  currentState -> setMaterial ( GL_AMBIENT, 0.5, 0.5, 0.5);
   	  currentState -> setMaterial ( GL_DIFFUSE,  0.7, 0.7, 0.7); // light grey
		  currentState -> setMaterial ( GL_SPECULAR, 1.0, 1.0, 1.0);
		  currentState -> setMaterial ( GL_EMISSION, 0.0, 0.0, 0.0);
		  
		  currentState -> setShininess ( 3 ) ; 

		  //currentState -> enable ( GL_COLOR_MATERIAL ) ;
		  //currentState -> setColourMaterial ( GL_AMBIENT_AND_DIFFUSE ) ;

		  currentState -> enable  ( GL_LIGHTING       ) ;
		  currentState -> setShadeModel ( GL_SMOOTH ) ;

		  currentState -> disable ( GL_BLEND ) ;
		  currentState -> setOpaque () ;
		  currentState -> disable( GL_TEXTURE_2D );
		  */
	  }

	  currentMesh.addToSSG(
		  currentState // Pfusch, kludge. NIV135
		  ,
		  current_options,
		  curr_branch_);
    MeshStatus.isvalid = FALSE;
  }
}

static int parse()
{
  char* token;
  token = parser.getNextToken(0); // getNextToken -> line structure is ignored (IOW line ends are ignored)
  while (! parser.eof )
	{ 
		if (!ParseEntity(token))
		  return FALSE;
		
		token = parser.getNextToken(0);
	}
  CheckWhetherMeshHasEnded();
  return TRUE ;
}


ssgEntity *ssgLoadASC ( const char *fname, const ssgLoaderOptions* options )
{
  ssgSetCurrentOptions ( (ssgLoaderOptions*)options ) ;
  current_options = ssgGetCurrentOptions () ;


  MeshStatus.isvalid=FALSE;
  MeshStatus.sName = NULL;


  untexturedState = new ssgSimpleState;
	untexturedState->setOpaque();
	untexturedState->disable(GL_BLEND);
	untexturedState->disable(GL_ALPHA_TEST);
	untexturedState->disable(GL_TEXTURE_2D);
	untexturedState->enable(GL_COLOR_MATERIAL);
	untexturedState->enable(GL_LIGHTING);
	untexturedState->setShadeModel(GL_SMOOTH);
	untexturedState->setMaterial(GL_AMBIENT , 0.7f, 0.7f, 0.7f, 1.0f);
	untexturedState->setMaterial(GL_DIFFUSE , 0.7f, 0.7f, 0.7f, 1.0f);
	untexturedState->setMaterial(GL_SPECULAR, 1.0f, 1.0f, 1.0f, 1.0f);
	untexturedState->setMaterial(GL_EMISSION, 0.0f, 0.0f, 0.0f, 1.0f);
	untexturedState->setShininess(50);
  untexturedState->ref();

  currentState = untexturedState; 
  top_branch = new ssgBranch ;
	curr_branch_ = top_branch;
	if ( !parser.openFile( fname, &parser_spec ))
	{
    delete top_branch ;
		return 0;
  }
  if ( !parse() )
  {
		delete top_branch ;
		top_branch = 0 ;
  }
//  parse_free();
  parser.closeFile();

  untexturedState->deRef();
  if(untexturedState->getRef()==0)
    delete untexturedState;
  return top_branch ;
}
