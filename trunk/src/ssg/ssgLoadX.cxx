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
   0,          // comment_char
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

void IgnoreEntity(void)
{ int Level=0;
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
	{ "Material", NULL, TRUE}, 
	{ "TextureFilename", NULL, TRUE},  // ?
	{ "MeshFace", NULL, FALSE}, 
	{ "MeshFaceWraps", NULL, FALSE}, 
	{ "MeshTextureCoords", NULL, TRUE}, 
	{ "MeshNormals", NULL, TRUE}, 
	{ "MeshVertexColors", NULL, FALSE}, 
	{ "MeshMaterialList", NULL, TRUE}, 
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
					IgnoreEntity();
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

void HandleMesh(const char *sName, const char *firstToken)
{ u32 nNoOfVertices, nNoOfVerticesForThisFace, nNoOfFaces, i, j, iVertex;
	int index=0, aiVertices[MAX_NO_VERTICES_PER_FACE];
	
	
	sName; // keep the compiler quiet
	  
	//char *sMeshName = parser.getNextToken("Mesh name");
	//parser.expectNextToken("{");
	nNoOfVertices = atoi(firstToken); //Pfusch!! kludge
	//parser.getNextInt("number of vertices");

  ssgVertexArray* vl = new ssgVertexArray ( nNoOfVertices ) ;
	
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

		vl->add(vert);
	}
	nNoOfFaces = parser.getNextInt("number of faces");

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
  ssgVtxArray* leaf = new ssgVtxArray ( GL_TRIANGLES,
    vl, NULL, NULL, NULL, il ) ;
  leaf -> setCullFace ( TRUE ) ;
  //leaf -> setState ( st ) ;
  //return 
	current_options -> createLeaf ( leaf, NULL) ;
	curr_branch_->addKid(leaf);
	  
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
					"AFAIK this is the only published version.\n"
					"Your file has version %d.%d\n"
					"Use the file at your own risk\n", 
					(int)(token[0])*256+token[1],
					(int)(token[2])*256+token[3]);
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
