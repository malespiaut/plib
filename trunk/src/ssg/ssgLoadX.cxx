//
// .X loader for SSG/PLIB
// .X is the 3D file format for Micro$ofts directX retained mode.
// Written by Wolfram Kuss (Wolfram.Kuss@t-online.de) in Oct/Nov of 2000
//
#include "ssgLocal.h"
#include "ssgParser.h"

#define u32 unsigned int

typedef void HandlerFunctionType(const char *sName, const char *firstToken);
char *globEmpty="";

static ssgBranch                *curr_branch_;

struct EntityType
{
  char * sName;
	HandlerFunctionType *HandleEntity;
	int bMayBeIgnored;
} ;



static /* const */ ssgLoaderOptions* current_options = NULL ;

static _ssgParserSpec parser_spec =
{
   "\r\n\t ",  // delim_chars_skipable
   ",;",       // delim_chars_non_skipable
   "{",          // open_brace_chars
   "}",          // close_brace_chars
   '"',        // quote_char
   '#',          // comment_char
	 "//"        // comment_string
} ;

   
static _ssgParser parser;
static ssgBranch* top_branch;

void HandleHeader(const char *sName, const char *firstToken)
{
	sName; // keep the compiler quiet
	//parser.expectNextToken("{");
	u32 Dummy = atoi(firstToken); // Pfusch!!
		//parser.getNextInt("Header.major");
	parser.expectNextToken(";");
	Dummy = parser.getNextInt("Header.minor");
	parser.expectNextToken(";");
	Dummy = parser.getNextInt("Header.flags");
	parser.expectNextToken(";");
	parser.expectNextToken("}");
}

void IgnoreEntity(int startLevel)
// startLevel should be 0 when you are "in front of the {" (normally)
// or 1 when you have already parsed the "{"
{ 
	int Level = startLevel;
  char *token;

	while ( TRUE)
	{ token = parser.getNextToken(0);
		assert(token!=NULL); // Pfusch kludge
    if ( strcmp(token,"{") == 0 )
			Level++;
		else if ( strcmp(token,"}") == 0 )
		{ assert(Level>0); // Pfusch kludge
			if (Level==1) 
		    return; // found THE closing brace of entitiy
		  Level--; // Found A closing brace
		}
	}      
}

void HandleMesh(const char *sName, const char *firstToken);
void HandleMeshMaterialList(const char *sName, const char *firstToken);
void HandleTextureCoords(const char *sName, const char *firstToken);
void HandleMaterial(const char *sName, const char *firstToken);
void HandleTextureFileName(const char *sName, const char *firstToken);


EntityType aEntities[] =
{
	{ "Header", HandleHeader, FALSE}, 
	{ "Vector", NULL, FALSE}, 
	{ "Coords2d", NULL, FALSE}, 
	{ "Quaternion", NULL, FALSE}, 
	{ "Matrix4x4", NULL, FALSE}, 
	{ "ColorRGBA", NULL, FALSE}, 
	{ "ColorRGB", NULL, FALSE}, 
	{ "Indexed Color", NULL, FALSE}, 
	{ "Boolean", NULL, FALSE}, 
	{ "Boolean2d", NULL, FALSE}, 
	{ "Material", HandleMaterial, FALSE}, 
	{ "TextureFilename", HandleTextureFileName, FALSE},  
	{ "MeshFace", NULL, FALSE}, 
	{ "MeshFaceWraps", NULL, FALSE}, 
	{ "MeshTextureCoords", HandleTextureCoords, FALSE}, 
	{ "MeshNormals", NULL, TRUE}, 
	{ "MeshVertexColors", NULL, FALSE}, 
	{ "MeshMaterialList", HandleMeshMaterialList, FALSE}, 
	{ "Mesh", HandleMesh, FALSE}, 
	{ "FrameTransformMatrix", NULL, FALSE}, 
	{ "Frame", NULL, FALSE}, 
	{ "FloatKeys", NULL, FALSE}, 
	{ "TimedFloatKeys", NULL, FALSE}, 
	{ "AnimationKey", NULL, FALSE}, 
	{ "AnimationOptions", NULL, FALSE}, 
	{ "Animation", NULL, FALSE}, 
	{ "AnimationSet", NULL, FALSE}, 
	{ "template", NULL, TRUE},
  { NULL, NULL, NULL}
};


void ParseEntity(char *token)
// called recursively
{ int i=0;

	while(aEntities[i].sName!=NULL)
	{ if (!strcmp(token,aEntities[i].sName))
		{	if (aEntities[i].HandleEntity)
			{	char *sNextToken, *sName=globEmpty;
				sNextToken=parser.getNextToken(0);
				if (0!=strcmp(sNextToken, "{"))
				{ sName=sNextToken;
					sNextToken=parser.getNextToken(0);
					if (0!=strcmp(sNextToken, "{"))
						parser.error("\"{\" expected\n");
				}
				sNextToken=parser.getNextToken(0);
				if(sNextToken[0]=='<') // UUID
					sNextToken=parser.getNextToken(0);
				aEntities[i].HandleEntity(sName, sNextToken);
			}
			else
				if (aEntities[i].bMayBeIgnored)
					IgnoreEntity ( 0 );
				else
				{
					parser.error("I am sorry, but Entity-typ '%s' is not yet implemented.", aEntities[i].sName);
					return ; //FALSE ;
				}
				
			break;
		}
		i++;
	}
	if (aEntities[i].sName==NULL)
	{
		parser.error("unexpected token %s", token);
		return ; //FALSE ;
	}
}



#define MAX_NO_VERTICES_PER_FACE 1000

class XMesh {
public:
  ssgVertexArray* vl;
  ssgTexCoordArray* tl;
	u32 m_nNoOfVertices, m_nNoOfFaces;
	void ReInit ( int nNoOfVertices ) ;
} currentMesh;

ssgSimpleState *currentState;
sgVec4 currentDiffuse;

void XMesh::ReInit(int nNoOfVertices ) 
{
	m_nNoOfVertices = nNoOfVertices;
  vl = new ssgVertexArray ( nNoOfVertices ) ;
  tl = NULL ;
}

void HandleTextureFileName(const char *sName, const char *firstToken)
{/*
	  TextureFilename {
    "../image/box_top.gif";
   } #TextureFilename
 */
  currentState -> setTexture( current_options -> createTexture( (char *) firstToken) ) ;
  currentState -> enable( GL_TEXTURE_2D );

	parser.expect(";");
	parser.expect("}");
}

void HandleMaterial(const char *sName, const char *firstToken)
{ float power;
  int bFoundTextureFileName = FALSE;
	sgVec4 specularColour, EmissiveColour;

	// read body
	currentDiffuse[0] = atof(firstToken); // parser.getNextFloat("Facecolour R");
	parser.expectNextToken(";");
	currentDiffuse[1] = parser.getNextFloat("Facecolour G");
	parser.expectNextToken(";");
	currentDiffuse[2] = parser.getNextFloat("Facecolour B");
	parser.expectNextToken(";");
	currentDiffuse[3] = parser.getNextFloat("Facecolour A");
	parser.expectNextToken(";");
	parser.expectNextToken(";");
	power = parser.getNextFloat("power");
	parser.expectNextToken(";");
	specularColour[0] = parser.getNextFloat("Specular R");
	parser.expectNextToken(";");
	specularColour[1]  = parser.getNextFloat("Specular G");
	parser.expectNextToken(";");
	specularColour[2] = parser.getNextFloat("Specular B");
	specularColour[3] = 0.0;
	parser.expectNextToken(";");
	parser.expectNextToken(";");
	EmissiveColour[0] = parser.getNextFloat("Emissive R");
	parser.expectNextToken(";");
	EmissiveColour[1] = parser.getNextFloat("Emissive G");
	parser.expectNextToken(";");
	EmissiveColour[2] = parser.getNextFloat("Emissive B");
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
	
	currentState -> setShininess ( power ) ; // Pfusch kludge: Is that correct?

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
		{ return; // Material is finished. success
		}
		
		if ( 0!= strcmp("TextureFilename", nextToken) )
		{ parser.error("TextureFilename expected!\n");
			return; // error
		}
		if ( bFoundTextureFileName )
		{ parser.error("Only one TextureFileName per Material please!\n");
			return; // error
		}
		ParseEntity(nextToken); // read "TextureFileName"
		bFoundTextureFileName = TRUE;
	}
}

void HandleTextureCoords(const char *sName, const char *firstToken)
{
	u32 nNoOfVertices, i;

	sName; // keep the compiler quiet
	  
	nNoOfVertices = atoi(firstToken); //Pfusch!! kludge
	if ( nNoOfVertices != currentMesh.m_nNoOfVertices )
	{ parser.error("No of vertices of mesh (%d) and no "
	            "of texture coordinates (%d) do not match!\n" 
							"Therefore the texture coordinates are ignored!",
							( int ) currentMesh.m_nNoOfVertices, ( int ) nNoOfVertices );
	  IgnoreEntity ( 1 ); // ignores TC.
		return;
	}
	currentMesh.tl = new ssgTexCoordArray ( nNoOfVertices ) ;

	parser.expectNextToken(";");
	for(i=0;i<nNoOfVertices;i++)
	{ 
		sgVec3 tv;
      
    tv[0]=parser.getNextFloat("x");
		parser.expectNextToken(";");
		tv[1]=parser.getNextFloat("y");
		parser.expectNextToken(";");
		if(i==nNoOfVertices-1)
			parser.expectNextToken(";");
		else
			parser.expectNextToken(",");

		currentMesh.tl->add ( tv ) ;
	}
	parser.expectNextToken("}");
	parser.error("Did read TC!\n");
}

void HandleMeshMaterialList(const char *sName, const char *firstToken)
{
	u32 i, nMaterials, nMaterialsRead = 0, nFaceIndexes;

	sName; // keep the compiler quiet
	  
	nMaterials = atoi(firstToken); //Pfusch!! kludge
	parser.expectNextToken(";");
	nFaceIndexes = parser.getNextInt("number of Face Indexes");
	parser.expectNextToken(";");


	if ( nFaceIndexes > currentMesh.m_nNoOfFaces)
	{ parser.error("No of face indexes of materiallist (%d) is greater than then no "
	            "of faces (%d)!\n" 
							"Therefore the material list is ignored!",
							( int ) nFaceIndexes, ( int ) currentMesh.m_nNoOfFaces );
	  IgnoreEntity ( 1 ); // ignores TC.
		return;
	}
	if ( nFaceIndexes > currentMesh.m_nNoOfFaces)
	  parser.message("Informationl: No of face indexes of materiallist (%d) is less than then no "
	            "of faces (%d)\n" ,
							( int ) nFaceIndexes, ( int ) currentMesh.m_nNoOfFaces );
	for ( i=0 ; i<nFaceIndexes ; i++ )
	{
		int iIndex = parser.getNextInt("Face index");
		parser.expectNextToken(";");
		if(i==nFaceIndexes -1)
		{ if ( nFaceIndexes > 1 )  // this is important for files created by MSs convx, for example rocket1_new.x
			  parser.expectNextToken(";");
		}
		else
			parser.expectNextToken(",");
	}
	while(TRUE)
	{ char *nextToken =parser.getNextToken(0);
	  if (0==strcmp("}", nextToken))
		{ if ( nMaterialsRead < nMaterials )
		    parser.error("Too few Materials!\n");
			else
				parser.error("Success! MeshMaterialList!\n");
			return; // Mesh is finished. success
		}
		if ( 0!= strcmp("Material", nextToken) )
		{ parser.error("Material expected!\n");
			return; // error
		}
		if ( nMaterialsRead >= nMaterials )
		{ parser.error("Too many Materials!\n");
			return; // error
		}
		ParseEntity(nextToken); // read "Material"
		nMaterialsRead++;
	}
}

static void recalcNormals( ssgIndexArray* il, ssgVertexArray* vl, ssgNormalArray *nl ) 
{
//  DEBUGPRINT( "Calculating normals." << std::endl);
  sgVec3 v1, v2, n;

  for (int i = 0; i < il->getNum() / 3; i++) {
    short ix0 = *il->get(i*3    );
    short ix1 = *il->get(i*3 + 1);
    short ix2 = *il->get(i*3 + 2);

    sgSubVec3(v1, vl->get(ix1), vl->get(ix0));
    sgSubVec3(v2, vl->get(ix2), vl->get(ix0));
    
    sgVectorProductVec3(n, v1, v2);

    sgCopyVec3( nl->get(ix0), n );
    sgCopyVec3( nl->get(ix1), n );
    sgCopyVec3( nl->get(ix2), n );
  }
}

void HandleMesh(const char *sName, const char *firstToken)
{ u32 nNoOfVertices, nNoOfVerticesForThisFace, nNoOfFaces, i, j, iVertex;
	int index=0, aiVertices[MAX_NO_VERTICES_PER_FACE];
	
	
	sName; // keep the compiler quiet
	  
	//char *sMeshName = parser.getNextToken("Mesh name");
	//parser.expectNextToken("{");
	nNoOfVertices = atoi(firstToken); //Pfusch!! kludge
	//parser.getNextInt("number of vertices");

	currentMesh.ReInit ( nNoOfVertices );
  
	
	parser.expectNextToken(";");
	for(i=0;i<nNoOfVertices;i++)
	{ 
		sgVec3 vert;
      
    vert[0]=parser.getNextFloat("x");
		parser.expectNextToken(";");
		vert[1]=parser.getNextFloat("y");
		parser.expectNextToken(";");
		vert[2]=parser.getNextFloat("z");
		parser.expectNextToken(";");
		if(i==nNoOfVertices-1)
			parser.expectNextToken(";");
		else
			parser.expectNextToken(",");

		currentMesh.vl->add(vert);
	}
	nNoOfFaces = parser.getNextInt("number of faces");
	currentMesh.m_nNoOfFaces = nNoOfFaces;

  ssgIndexArray* il = new ssgIndexArray ( nNoOfFaces * 3 ) ;

  
	parser.expectNextToken(";");
	for(i=0;i<nNoOfFaces;i++)
	{ nNoOfVerticesForThisFace = parser.getNextInt("number of vertices for this face");
		assert(nNoOfVerticesForThisFace<MAX_NO_VERTICES_PER_FACE);
	
		// parse faces and put the info into the array aiVertices

		parser.expectNextToken(";");
		for(j=0;j<nNoOfVerticesForThisFace;j++)
		{ iVertex= parser.getNextInt("Vertex index");

			aiVertices[j]=iVertex;
			
			if(j==nNoOfVerticesForThisFace-1)
				parser.expectNextToken(";");
			else
				parser.expectNextToken(",");
		}
		if(i==nNoOfFaces-1)
			parser.expectNextToken(";");
		else
			parser.expectNextToken(",");
		
		// use array aiVertices
#ifdef NOT_PLIB
		CreateFaceInMdi_Edit(vl, nNoOfVerticesForThisFace, aiVertices); // This line is for the "Mdi" 3D Editor, NOT for plib
#else
		for(j=0;j<nNoOfVerticesForThisFace;j++)
		{ 
			if (j<3)
				il->add(aiVertices[j]);
			else // add a complete triangle
			{ il->add(aiVertices[0]);
				il->add(aiVertices[j-1]);
				il->add(aiVertices[j]);
			}
		}
#endif
	}
	while(TRUE)
	{ char *nextToken =parser.getNextToken(0);
	  if (0==strcmp("}", nextToken))
			break; // Mesh is finished
		ParseEntity(nextToken);
	}

//
	ssgColourArray* cl = NULL ;
  
  if ( currentState -> isEnabled ( GL_LIGHTING ) )
  {
    if ( cl == NULL )
    {
      cl = new ssgColourArray ( 1 ) ;
      cl -> add ( currentDiffuse ) ;
    }
  
  }


//start

	//stop

  ssgNormalArray *nl = new ssgNormalArray(currentMesh.vl->getNum());
	for (i=0;i<currentMesh.vl->getNum();i++)
		nl->add(currentMesh.vl->get(i));

  
	recalcNormals(il,currentMesh.vl, nl); // Pfusch kludge; only do this if there are no nmormals in the file
fprintf(stdout, "nl->getNum()=%d\n",nl->getNum());
for(i=0;i<nl->getNum();i++)
{ float *f=nl->get(i);
  fprintf(stdout, "n= %f, %f, %f\n", f[0], f[1], f[2]);
}
  ssgVtxArray* leaf = new ssgVtxArray ( GL_TRIANGLES,
    currentMesh.vl, nl , 
		currentMesh.tl, cl, il ) ;
  leaf -> setCullFace ( TRUE ) ;



  float *af=currentState->getMaterial ( GL_DIFFUSE  );
  parser.message("vor leaf-SetState: af[0] = %f, %f, %f, %f\n",
	        af[0], af[1], af[2], af[3]);







  leaf -> setState ( currentState ) ;
  //return 
	current_options -> createLeaf ( leaf, NULL) ;
	curr_branch_->addKid(leaf);
	  
}

static int TwoCharsToInt(char char1, char char2)
{
  return ((int)(char1-'0'))*256+char2-'0';
}

static int HeaderIsValid(char *firstToken)
{	// xof 0302txt 0064
  if (strcmp(firstToken,"xof"))
  {
    parser.error("not X format, invalid Header");
    return FALSE ;
  }
	char* token = parser.getNextToken("2nd Header field");
	if (strlen(token)!=7)
	{
    parser.error("not X format, invalid Header");
    return FALSE ;
  }
	if (strcmp(&(token[4]),"txt"))
  {
    parser.error("not X format, invalid Header");
    return FALSE ;
  }
	if (strncmp(token, "0302", 4))
		parser.message("This loader is written for X-file-format version 3.2.\n" 
					"AFAIK this is the only documented version.\n"
					"Your file has version %d.%d\n"
					"Use the file at your own risk\n", 
					TwoCharsToInt(token[0], token[1]),
					TwoCharsToInt(token[2], token[3]));
	token = parser.getNextToken("3rd Header field");
	if (strcmp(token,"0032") && strcmp(token,"0064"))
  {
    parser.error("not X format, invalid Header");
    return FALSE ;
  }
  return TRUE;
}

static int parse()
{
  int firsttime = TRUE;
  char* token;
  int startLevel = parser.level;
  while ((token = parser.getNextToken(0)) != NULL)
	{ if (firsttime)
		{ 
			if(!HeaderIsValid(token))
					return FALSE;
			firsttime = FALSE;
		}
		else 
		{ int i=0;
			ParseEntity(token);
		}
	}
  return TRUE ;
}


ssgEntity *ssgLoadX ( const char *fname, const ssgLoaderOptions* options )
{
  current_options = (ssgLoaderOptions*) (options? options: &_ssgDefaultOptions) ;
  current_options -> begin () ;

  top_branch = new ssgBranch ;
	curr_branch_ = top_branch;
  parser.openFile( fname, &parser_spec );
  if ( !parse() )
  {
     delete top_branch ;
     top_branch = 0 ;
  }
//  parse_free();
  parser.closeFile();

  current_options -> end () ;
  return top_branch ;
}
