// ssgLoadVRML: loads vrml (and iv) files into the scenegraph
// known bugs/limitations:
// - per vertex texture coordinate mapping is not supported (only per face and vertex texture mapping is)
// - implicit texture mapping is not supported
// - explicit normal definitions not supported (they are calculated automatically)
// - only a very small subset of the inventor spec is supported
// - there is no support for primitives (cubes, spheres, cylinders, or cones)
// - no support for weblinks in vrml(well, this isn't much of a tragedy) :-)
// ..

#include <stdio.h>
#include "ssgLocal.h"
#include "ssgParser.h"
#include "ssgLoaderWriterStuff.h"

#include "ssgLoadVRML.h"

static _ssgParserSpec parser_spec =
{
   "\r\n\t, ",  // delim_chars_skipable 
     0,          // delim_chars_non_skipable
     "{[",        // open_brace_chars
     "}]",        // close_brace_chars
     '"',        // quote_char
     '#',          // comment_char
     0           // comment_string
};

// vrmlProperties: the current properties for a certain point in scene
// traversal
class vrmlData
{
 private:
   ssgVertexArray *vertices;
   ssgTexCoordArray *textureCoordinates;
   ssgTransform *transform;
   ssgTexture *texture;
   bool textureCoordinatesArePerFaceAndVertex;
   GLenum frontFace;
   bool enableCullFace;
 public:
   
   bool getEnableCullFace() { return enableCullFace; }
   void setEnableCullFace( bool newEnableCullFace ) { enableCullFace = newEnableCullFace; }	   
   
   GLenum getFrontFace( void ) { return frontFace; }
   void setFrontFace( GLenum newFrontFace ) { frontFace = newFrontFace; }
   
   ssgTransform * getTransform( void ) { return transform; }
   void setTransform( ssgTransform *newTransform ) { transform = newTransform; }
	
   ssgVertexArray * getVertices( void ) { return vertices; }
   void setVertices( ssgVertexArray *newVertices ) { vertices = newVertices; }	

   ssgTexCoordArray *getTextureCoordinates( void ) { return textureCoordinates; }
   void setTextureCoordinates( ssgTexCoordArray *newTextureCoordinates ) { textureCoordinates = newTextureCoordinates; }	
	    
   ssgTexture * getTexture( void ) { return texture; }
   void setTexture( ssgTexture *newTexture ) { texture = newTexture; }
     
   bool areTextureCoordinatesArePerFaceAndVertex( void ) { return textureCoordinatesArePerFaceAndVertex; }
   
   vrmlData *clone() { return new vrmlData(*this); }
   vrmlData() { vertices = NULL; textureCoordinates = NULL; transform = NULL; texture = NULL; textureCoordinatesArePerFaceAndVertex  = TRUE; enableCullFace = FALSE; }
};


static _ssgParser parser;
static ssgLoaderOptions* currentOptions = NULL ;
static vrmlNodeIndex *definedNodes = NULL;

static bool parseSwitchSeparator( ssgBranch *parentBranch, vrmlData *parentData, bool isASwitch = FALSE, char *defName = NULL );
static bool parseIndexedFaceSet( ssgBranch *parentBranch, vrmlData *currentData, char *defName );
static bool parseCoordinate3( ssgBranch *parentBranch, vrmlData *currentData, char *defName );
static bool parseTextureCoordinate2( ssgBranch *parentBranch, vrmlData *currentData, char *defName );
static bool parseTexture2( vrmlData *currentData, char *defName );
static bool parseShapeHints( vrmlData *currentData, char *defName );
static bool parseMatrixTransform( vrmlData *currentData, char *defName );
static bool parseScale( vrmlData *currentData, char *defName );
static bool parseRotation( vrmlData *currentData, char *defName );
static bool parseUseDirective( ssgBranch *parentBranch, vrmlData *currentData, char *useName, char *defName );
static bool parseUnidentified();

void applyTransform( ssgTransform *currentTransform, vrmlData *currentData );
void mergeTransformNodes( ssgTransform *newTransform, ssgTransform *oldTransform1, ssgTransform *oldTransform2 );

static bool parseVec( SGfloat *v, int vSize );
ssgIndexArray * parseIndexArray( vrmlData *currentData );

ssgEntity *ssgLoadVRML( const char *fname, const ssgLoaderOptions* options )
{
  ssgSetCurrentOptions ( (ssgLoaderOptions*)options ) ;
  currentOptions = ssgGetCurrentOptions () ;

   if ( !parser.openFile( fname, &parser_spec ) ) {
    ulSetError ( UL_WARNING, "ssgLoadVRML: Failed to open '%s' for reading", fname ) ;
    return 0;
  }

   definedNodes = new vrmlNodeIndex();
   
   // check for a valid header header
   char *token;
   if( !(token =  parser.getRawLine()) )
     return 0;
   if( strstr( token, "#VRML V1.0 ascii" ) == NULL )
     if( strstr( token, "#Inventor V2.1 ascii" ) == NULL ) // should we handle different flavours of inventor?
       {
	  ulSetError ( UL_WARNING, "ssgLoadVRML: valid iv/vrml1 header not found" );
	  return 0;
       }
   
   // creating a root node..
   ssgBranch *rootBranch = new ssgBranch();
   
   parser.expectNextToken( "Separator" );

   if( !parseSwitchSeparator( rootBranch, NULL, FALSE, NULL ) )
     {
	ulSetError ( UL_WARNING, "ssgLoadVRML: Failed to extract valid object(s) from %s", fname ) ;
	return NULL ;
     }
   
   parser.closeFile();

   return rootBranch ;
}

static  bool parseSwitchSeparator( ssgBranch *parentBranch, vrmlData *parentData, bool isASwitch, char *defName )
{   
   char *childDefName = NULL;
   
   char *token;
   
   parser.expectNextToken( "{" );

   // create a branch for this node
   ssgBranch *currentBranch;
   if( isASwitch ) 
     {
	currentBranch = new ssgSelector();
	((ssgSelector *)currentBranch)->select( 0 );
	// fixme: allow for children to be traversed
     }
   else
     currentBranch = new ssgBranch();
   
   if( defName != NULL ) 
     {
	currentBranch->setName( defName );
	definedNodes->insert( currentBranch );
     }
   
   // fixme: not sure if this is correct.. maybe we should
   // allow this data to be passed down separator nodes
   // (was: if there's no data passed from above, then create some..)
   vrmlData *currentData;
   if( parentData == NULL )
     currentData = new vrmlData();
   else
     currentData = parentData->clone();
   
   token = parser.getNextToken( NULL );
   while( strcmp( token, "}" ) )
     {
	if( !strcmp( token, "Separator" ) ) 
	  {
	     //printf("Separator: Create a child..\n");
	     if( ! parseSwitchSeparator( currentBranch, currentData, FALSE, childDefName ) )
	       return FALSE;
	  }
	else if( !strcmp( token, "Switch" ) ) 
	  {
	     //printf("Switch: Create a child..\n");
	     if( ! parseSwitchSeparator( currentBranch, currentData, TRUE, childDefName ) )
	       return FALSE;	     
	  }
	else if( !strcmp( token, "DEF" ) ) 
	  {
	     token = parser.getNextToken( NULL );
	     //printf("DEF: Found an object definition %s.\n", token);
	     childDefName = new char[50];
	     strncpy( childDefName, token, 50);
	  }
	else if( !strcmp( token, "USE" ) ) 
	  {
	     token = parser.getNextToken( NULL );
	     //printf("USE: Found a use directive %s.\n", token);
	     if( !parseUseDirective( currentBranch, currentData, token, childDefName ) )
	       return FALSE;
	  }
	else if( !strcmp( token, "IndexedFaceSet" ) )
	  {
	     if( !parseIndexedFaceSet( currentBranch, currentData, childDefName ) )
	       return FALSE;
	  }
	else if( !strcmp( token, "Coordinate3" ) )
	  {
	     if( !parseCoordinate3( currentBranch, currentData, childDefName ) )
	       return FALSE;
	  }
	else if( !strcmp( token, "TextureCoordinate2" ) )
	  {
	     if( !parseTextureCoordinate2( currentBranch, currentData, childDefName ) )
	       return FALSE;
	  }
	else if( !strcmp( token, "Texture2" ) )
	  {
	     if( !parseTexture2( currentData, childDefName ) )
	       return FALSE;
	  }
	else if( !strcmp( token, "ShapeHints" ) )
	  {
	     if( !parseShapeHints( currentData, childDefName ) )
	       return FALSE;
	  }
	else if( !strcmp( token, "MatrixTransform" ) )
	  {
	     if( !parseMatrixTransform( currentData, childDefName ) )
	       return FALSE;
	  }
	else if( !strcmp( token, "Scale" ) )
	  {
	     if( !parseScale( currentData, childDefName ) )
		 return FALSE;
	  }	     
	else if( !strcmp( token, "Rotation" ) )
	  {
	     if( !parseRotation( currentData, childDefName ) )
		 return FALSE;
	  }	     
	else 
	  {
	     //printf("Found an unidentified element: '%s'. Skipping it.\n", token);
	     if( ! parseUnidentified() )
	       return FALSE;
	  }
	
	token = parser.getNextToken( NULL );
     }  

   parentBranch->addKid( currentBranch );
   // todo: if this node is def'd, add it to the use list..
   
   // delete the currentData structure (we may use its content, but not its form)
   delete( currentData );
   
   return TRUE;
}

// parseVec: tries to parse a vec (of vSize), returns true if successful, false otherwise
static bool parseVec( SGfloat *v, int vSize )
{
  for( int i=0; i<vSize; i++ ) {
    if( !parser.getNextFloat( v[i], NULL ) )
       {
	  ulSetError ( UL_WARNING, "ssgLoadVRML: Expected a float for a vector, didn't get it." ) ;
	  return FALSE;
       }
  }
   
   return TRUE;
}

// tries to parse an index array (of arbitrary size, delimited by -1), returns true if successful,
// false otherwise
ssgIndexArray * parseIndexArray( vrmlData *currentData )
{
   ssgIndexArray *indexArray = new ssgIndexArray();
   char *token;
      
   token = parser.peekAtNextToken( NULL );
   while( strcmp( token, "-1" ) ) 
     {
	int index;
	if( parser.getNextInt( index, NULL ) )
	  indexArray->add( index );
	else
	  return NULL;
	
	token = parser.peekAtNextToken( NULL );
     }
   parser.expectNextToken( "-1" );

   // we have to reverse vertex ordering if we are going clockwise
   if( currentData->getFrontFace() == GL_CW )
     {
	// so return something else that goes in reverse order
       	ssgIndexArray *reversedIndexArray = new ssgIndexArray( indexArray->getNum() );
	for( int i=(indexArray->getNum()-1); i>=0; i-- )
	  {
	     int newIndex = (int)*indexArray->get( i );
	     reversedIndexArray->add( newIndex );
	  }
	delete( indexArray );
	return reversedIndexArray;
     }
   
   return indexArray;
}

// parseCoordinate3: parses a list of 3d coordinates, adds them to the current
// vertice array
static bool parseCoordinate3( ssgBranch *parentBranch, vrmlData *currentData, char *defName )
{
   char *token;
   int numVertices = 0;
   
   // Ok, now we can allocate a new vertex table
   //ssgVertexArray *currentVertices = new ssgVertexArray();
   ssgVertexArray *currentVertices = new ssgVertexArray();
   if( defName )
     currentVertices->setName( defName );
   
   parser.expectNextToken("{");
   parser.expectNextToken("point");
   
   // an array? most likely..
   token = parser.peekAtNextToken( NULL );
   if( !strcmp( token, "[" ) )
       {
	  parser.expectNextToken("[");
	  // begin parsing vertices
	  token = parser.peekAtNextToken( NULL );

	  while( strcmp( token, "]" ) )
	    {
	       sgVec3 v;
	       if( ! parseVec( v, 3 ) )
		 return FALSE;
	       numVertices++;
	       currentVertices->add( v );
	       
	       token = parser.peekAtNextToken( NULL );
	    }
	  parser.expectNextToken("]");
       }
    
   // otherwise it must be a singular value
   else 
     {
	sgVec3 v;
	if( ! parseVec( v, 3 ) )
	  return FALSE;
	numVertices++;
	currentVertices->add( v );
     }
       
   //printf("Level: %i. Found %i vertices here.\n", parser.level, numVertices);

   parser.expectNextToken("}");
   
   //parentBranch->addKid( (ssgBranch *)currentVertices );
   currentData->setVertices( currentVertices );

   return TRUE;
}

// parseCoordinate3: parses a list of 3d coordinates, adds them to the current
// vertice array
static bool parseTextureCoordinate2( ssgBranch *parentBranch, vrmlData *currentData, char *defName )
{
   char *token;
   int numTextureCoordinates = 0;
   
   ssgTexCoordArray *currentTextureCoordinates = new ssgTexCoordArray();
   if( defName )
     currentTextureCoordinates->setName( defName );
   
   parser.expectNextToken("{");
   parser.expectNextToken("point");
   
   // an array? most likely..
   token = parser.peekAtNextToken( NULL );
   if( !strcmp( token, "[" ) )
       {
	  parser.expectNextToken("[");
	  // begin parsing TexCoords
	  token = parser.peekAtNextToken( NULL );

	  while( strcmp( token, "]" ) )
	    {
	       sgVec2 v;
	       if( ! parseVec( v, 2 ) )
		 return FALSE;
	       numTextureCoordinates++;
	       currentTextureCoordinates->add( v );
	       
	       token = parser.peekAtNextToken( NULL );
	    }
	  parser.expectNextToken("]");
       }
    
   // otherwise it must be a singular value
   else 
     {
	sgVec2 v;
	if( ! parseVec( v, 2 ) )
	  return FALSE;
	numTextureCoordinates++;
	currentTextureCoordinates->add( v );
     }
       
   //printf("Level: %i. Found %i TexCoords here.\n", parser.level, numTextureCoordinates);

   parser.expectNextToken("}");
   
   //parentBranch->addKid( (ssgBranch *)currentTexCoords );
   currentData->setTextureCoordinates( currentTextureCoordinates );

   return TRUE;
}


static bool parseIndexedFaceSet( ssgBranch *parentBranch, vrmlData *currentData, char *defName )
{
   char *token1, *token2;
   int numFaces = 0; 
   //bool texCoordIndexGiven = FALSE;
   
   //ssgBranch *currentBranch = new ssgBranch();
   
   ssgLoaderWriterMesh *loaderMesh = new ssgLoaderWriterMesh();
   loaderMesh->createFaces();
   loaderMesh->setVertices( currentData->getVertices() );
   if( currentData->getTexture() != NULL && currentData->getTextureCoordinates() != NULL )
     loaderMesh->createPerFaceAndVertexTextureCoordinates2();

   parser.expectNextToken("{");
   
   token1 = parser.peekAtNextToken( NULL );
   while( strcmp( token1, "}" ) )
     {
	if( !strcmp( token1, "coordIndex" ) )
	  {
	     parser.expectNextToken("coordIndex");
	     token2 = parser.peekAtNextToken( NULL );
	     
	     // an array? most likely..
	     if( !strcmp( token2, "[" ) ) 
	       {
		  parser.expectNextToken("[");
		  token2 = parser.peekAtNextToken( NULL );
		  while( strcmp( token2, "]" ) ) 
		    {
		       ssgIndexArray *currentFaceIndices = parseIndexArray( currentData );
		       if( currentFaceIndices == NULL )
			 {
			    ulSetError ( UL_WARNING, "ssgLoadVRML: invalid index list" ) ;
			    return FALSE;
			 }
		       loaderMesh->addFace( (ssgIndexArray **) &currentFaceIndices );
		       //printf("Level: %i. Added a face with %i vertices\n", parser.level, numVerticesInFace );
		       numFaces++;
		       
		       token2 = parser.peekAtNextToken( NULL );
		    }
		     parser.expectNextToken( "]" );
	       }
	     
	     // otherwise a single point
	     else 
	       {
		  ssgIndexArray *currentFaceIndices = parseIndexArray( currentData );
		  if( currentFaceIndices == NULL )
		    {
		       ulSetError ( UL_WARNING, "ssgLoadVRML: invalid index list" ) ;
		       return FALSE;
		    }
		  loaderMesh->addFace( (ssgIndexArray **) &currentFaceIndices );
		  numFaces++;
		  
		  parser.expectNextToken( "-1" ); 
	       }	     
	  }
	
	else if( !strcmp( token1, "textureCoordIndex" ) )
	  {
	     parser.expectNextToken("textureCoordIndex");
	     token2 = parser.peekAtNextToken( NULL );

	     // an array? most likely..
	     if( !strcmp( token2, "[" ) ) 
	       {
		  parser.expectNextToken("[");
		  token2 = parser.peekAtNextToken( NULL );
		  while( strcmp( token2, "]" ) ) 
		    {
		       ssgIndexArray *currentTextureCoordinateIndices = parseIndexArray( currentData );
		       if( currentTextureCoordinateIndices == NULL )
			 return FALSE;
		       ssgTexCoordArray *currentPerFaceAndVertexTextureCoordinateList = new ssgTexCoordArray( currentTextureCoordinateIndices->getNum() );
		       for( int i=0; i<currentTextureCoordinateIndices->getNum(); i++ )
			 currentPerFaceAndVertexTextureCoordinateList->add( (currentData->getTextureCoordinates())->get( (unsigned int)*currentTextureCoordinateIndices->get( i ) ) );
		       loaderMesh->addPerFaceAndVertexTextureCoordinate2( (ssgTexCoordArray **) &currentPerFaceAndVertexTextureCoordinateList );
		       
		       delete( currentTextureCoordinateIndices );
		       //printf("Level: %i. Added a face with %i vertices\n", parser.level, numVerticesInFace );
			    
		       token2 = parser.peekAtNextToken( NULL );
		    }
		  parser.expectNextToken( "]" );
	       }
	     // otherwise a single point
	     else
	       {
		  ssgIndexArray *currentTextureCoordinateIndices = parseIndexArray( currentData );
		  if( currentTextureCoordinateIndices == NULL )
		    return FALSE;
		  ssgTexCoordArray *currentPerFaceAndVertexTextureCoordinateList = new ssgTexCoordArray( currentTextureCoordinateIndices->getNum() );
		  for( int i=0; i<currentTextureCoordinateIndices->getNum(); i++ )
		    currentPerFaceAndVertexTextureCoordinateList->add( (currentData->getTextureCoordinates())->get( (unsigned int)*currentTextureCoordinateIndices->get( i ) ) );
		  loaderMesh->addPerFaceAndVertexTextureCoordinate2( (ssgTexCoordArray **) &currentPerFaceAndVertexTextureCoordinateList );
		  
		  delete( currentTextureCoordinateIndices );
		  //printf("Level: %i. Added a face with %i vertices\n", parser.level, numVerticesInFace );
	       }
	     
	  }
	else
	  token1 = parser.getNextToken( NULL );
   
	token1 = parser.peekAtNextToken( NULL );
     }
   
   //printf("Level: %i. Found %i faces here.\n", parser.level, numFaces);

   parser.expectNextToken( "}" );
   
   // -------------------------------------------------------
   // add the face set to ssg
   // -------------------------------------------------------

   // kludge. We need a state for addToSSG:
   ssgSimpleState * ss = new ssgSimpleState () ; // (0) ?
   ss -> setMaterial ( GL_AMBIENT, 0.5, 0.5, 0.5, 1.0);
   ss -> setMaterial ( GL_DIFFUSE, 1.0, 1.0, 1.0, 1.0) ; // 0.8, 0.8, 1.0, 1.0f
   ss -> setMaterial ( GL_SPECULAR, 1.0, 1.0, 1.0, 1.0);
   ss -> setMaterial ( GL_EMISSION, 0.0, 0.0, 0.0, 1.0);
   ss -> setShininess ( 20 ) ; // Fixme, NIV14: Is that correct?

   // -------------------------------------------------------
   // texturing stuff
   // -------------------------------------------------------
   // todo: give an implicit mapping if texture coordinates are not given
   // todo: add support for per-vertex texturing
   if( currentData->getTexture() != NULL && currentData->getTextureCoordinates() != NULL ) 
     {
	ss -> setTexture ( currentData->getTexture() );
	ss -> enable( GL_TEXTURE_2D );
	//if( currentData->areTextureCoordinatesArePerFaceAndVertex() )
	//  {
	     //todo: IMPORTANT add support for texturecoordindex per face vertex mappings
	     
	     // if texturecoordindex is not defined, map onto face from texture coordinates
	     //if( !texCoordIndexGiven )
	     //  loaderMesh->mapTextureCoordinatesOntoFaceAndVertex( currentData->getTextureCoordinates() );
	     // otherwise use the provided texture mapping indices
	     
	//  }
	
     }
   else
	ss -> disable( GL_TEXTURE_2D );
   
   ss -> disable ( GL_COLOR_MATERIAL ) ;
   //ss -> enable ( GL_COLOR_MATERIAL ) ;
   //ss -> setColourMaterial ( GL_AMBIENT_AND_DIFFUSE ) ;
   
   ss -> enable  ( GL_LIGHTING       ) ;
   ss -> setShadeModel ( GL_SMOOTH ) ;
   
   ss  ->disable(GL_ALPHA_TEST); //needed?
   
   ss -> disable ( GL_BLEND ) ;
   
   ss -> setOpaque () ;

   if( !currentData->getEnableCullFace() )
     ss->disable( GL_CULL_FACE );
   
   if( !loaderMesh->checkMe() )
     return FALSE; 
   
   if( currentData->getTransform() != NULL ) 
     {
	parentBranch->addKid( currentData->getTransform() );
 	loaderMesh->addToSSG( ss, currentOptions, currentData->getTransform() );
     }
   else
 	loaderMesh->addToSSG( ss, currentOptions, parentBranch );
   
   return TRUE;
}

static bool parseTexture2( vrmlData *currentData, char *defName )
{
   char *token;
   char *fileName = NULL; bool wrapU = FALSE, wrapV = FALSE;
   parser.expectNextToken("{");

   token = parser.peekAtNextToken( NULL );
   while( strcmp( token, "}" ) )
     {
	if( !strcmp( token, "filename") )
	  {
	     // todo: handle quotes
	     parser.expectNextToken("filename");
	     token = parser.getNextToken( NULL );
	     fileName = new char[50];
	     strncpy( fileName, token, 50);
	  }
	else if( !strcmp( token, "wrapS") )
	  {
	     parser.expectNextToken("wrapS");
	     token = parser.getNextToken( NULL );
	     if( !strcmp( token, "REPEAT") )
	       wrapU = TRUE;
	  }
	else if( !strcmp( token, "wrapT") ) 
	  {
	     parser.expectNextToken("wrapT");
	     token = parser.getNextToken( NULL );
	     if( !strcmp( token, "REPEAT") )
	       wrapV = TRUE;
	  }
	else
	  token = parser.getNextToken( NULL );
	
	token = parser.peekAtNextToken( NULL );
     }
   
   
   if( fileName == NULL )
     return FALSE;
   
   ssgTexture *currentTexture = new ssgTexture( fileName, wrapU, wrapV );
   currentData->setTexture( currentTexture );
   parser.expectNextToken("}");

   delete( fileName );
   
   return TRUE;
}

static bool parseShapeHints( vrmlData *currentData, char *defName )
{
   char *token;
   parser.expectNextToken("{");

   token = parser.peekAtNextToken( NULL );
   while( strcmp( token, "}" ) )
     {
	if( !strcmp( token, "vertexOrdering") )
	  {
	     parser.expectNextToken("vertexOrdering");
	     token = parser.getNextToken( NULL );
	     if( !strcmp( token, "CLOCKWISE") )
	       {
		  currentData->setEnableCullFace( TRUE );
		  currentData->setFrontFace( GL_CW );
	       }
	     else if( !strcmp( token, "COUNTERCLOCKWISE") ) 
	       {
		  currentData->setEnableCullFace( TRUE );
		  currentData->setFrontFace( GL_CCW );
	       }
	     else if( !strcmp( token, "UNKNOWN_ORDERING") )
		  currentData->setEnableCullFace( FALSE );
	     else
	       {
		  ulSetError ( UL_WARNING, "ssgLoadVRML: invalid vertex ordering directive" ) ;
		  return FALSE;
	       }
	     
	  }
	else
	  token = parser.getNextToken( NULL );

	token = parser.peekAtNextToken( NULL );	
     }
   parser.expectNextToken("}");
   
   return TRUE;
}

static bool parseMatrixTransform( vrmlData *currentData, char *defName )
{
   ssgTransform *currentTransform = new ssgTransform();
   sgMat4 transformMat;

   parser.expectNextToken("{");
   parser.expectNextToken("matrix");
   for( unsigned int i=0; i<4; i++ )
     for( unsigned int j=0; j<4; j++ ) 
       {
	  if( !parser.getNextFloat( transformMat[i][j], NULL ) ) 
	    {
	       ulSetError ( UL_WARNING, "ssgLoadVRML: Expected a float for a matrix, didn't get it." ) ;
	       return FALSE;
	    }
       }
   parser.expectNextToken("}");

   currentTransform->setTransform( transformMat );

   applyTransform( currentTransform, currentData );

   //printf("Found a Matrix Transform (%f, %f, %f %f), (%f, %f, %f %f), (%f, %f, %f %f), (%f, %f, %f %f)\n", xForm[0][0], xForm[1][0], xForm[2][0], xForm[3][0],
   //	  xForm[0][1], xForm[1][1], xForm[2][1], xForm[3][1],
   //	  xForm[0][2], xForm[1][2], xForm[2][2], xForm[3][2],
   //	  xForm[0][3], xForm[1][3], xForm[2][3], xForm[3][3] );
    
   return TRUE;
}

static bool parseScale( vrmlData *currentData, char *defName )
{
   ssgTransform *currentTransform = new ssgTransform();
   sgVec3 scaleFactor;
   
   sgCoord moveFactor; sgZeroCoord( &moveFactor );
   
   parser.expectNextToken("{");
   parser.expectNextToken("scaleFactor");
   if( !parseVec( scaleFactor, 3 ) )
     return FALSE;
   parser.expectNextToken("}");

   currentTransform->setTransform( &moveFactor, scaleFactor[0], scaleFactor[1], scaleFactor[2] );
   
   applyTransform( currentTransform, currentData );
   
   //printf("Found a scale transform: %f %f %f\n", scaleFactor[0], scaleFactor[1], scaleFactor[2] );
   
   return TRUE;
}

static bool parseRotation( vrmlData *currentData, char *defName )
{
   ssgTransform *currentTransform = new ssgTransform();
   sgVec3 axis;
   SGfloat angle;
   sgMat4 rotation;
   
   parser.expectNextToken("{");
   parser.expectNextToken("rotation");
   if( !parseVec( axis, 3 ) )
     return FALSE;
   if( !parser.getNextFloat( angle, NULL ) )
     return FALSE;
   parser.expectNextToken("}");

   angle /= (3.14159265/360); // convert radians to degrees 
   
   sgMakeRotMat4( rotation, angle, axis ) ;
   currentTransform->setTransform( rotation );
   
   applyTransform( currentTransform, currentData );
   
   //printf("Found a rotation: %f %f %f %f\n", axis[0], axis[1], axis[2], angle );
   
   return TRUE;
}

static bool parseUseDirective( ssgBranch *parentBranch, vrmlData *currentData, char *useName, char *defName )
{
   // find the node within the list of defined nodes
   ssgBase *node = definedNodes->extract( useName );

   if( node==NULL )
     return TRUE;

   if( node->isA( ssgTypeBranch() ) ) 
     {
	ssgBranch *currentBranch = NULL;
	if( currentData->getTransform() != NULL )
	  {
	     currentBranch = currentData->getTransform();
	     currentBranch->addKid( (ssgEntity *)node );
	  }
	else 
	  currentBranch = (ssgBranch *)node;
	
	parentBranch->addKid( currentBranch );
	
	return TRUE;
     }
   
   return TRUE;
}


// parseUnidentified: A node that we either don't support or isn't part of the
// VRML spec. Just skip it.
static bool parseUnidentified()
{
   char *token;
   
   int startLevel = parser.level;
   int currentLevel = startLevel + 1;
   
   parser.expectNextToken("{");
   
   while( currentLevel != startLevel )
     {
	token = parser.getNextToken( NULL );

	if( !strcmp( token, "{" ) )
	  currentLevel++;
	else if( !strcmp( token, "}" ) )
	  currentLevel--;	
     }
   
   return TRUE;
}

void applyTransform( ssgTransform *currentTransform, vrmlData *currentData )
{
   if( currentData->getTransform() == NULL )
     currentData->setTransform( currentTransform );
   else
     {
	ssgTransform *newTransform = new ssgTransform();
	mergeTransformNodes( newTransform, currentTransform, currentData->getTransform() );
	// this will have to be changed when we allow use declarations on transforms
	delete( currentTransform );
	currentData->setTransform( newTransform );
     }
}

void mergeTransformNodes( ssgTransform *newTransform, ssgTransform *oldTransform1, ssgTransform *oldTransform2 )
{
   sgMat4 oldTransformMat1;
   sgMat4 oldTransformMat2;
   sgMat4 newTransformMat;

   oldTransform1->getTransform( oldTransformMat1 );
   oldTransform2->getTransform( oldTransformMat2 );
   sgMultMat4( newTransformMat, oldTransformMat1, oldTransformMat2 ) ;
   newTransform->setTransform( newTransformMat );
}
