class ssgListOfNodes : public ssgSimpleList
// list of POINTERs to ssgBase
// used for storing/querying DEF'd info
{
public:

   virtual ssgBase *clone ( int clone_flags = 0 ) { return NULL; }; // Fixme NIV14: 2do
   ssgListOfNodes ( int init = 3 ) : ssgSimpleList ( sizeof(class ssgBase*), init ) {} 
   class ssgBase *get ( unsigned int n ) { return *( (class ssgBase **) raw_get ( n ) ) ; }
   void   add ( class ssgBase *thing ) { raw_add ( (char *) &thing ) ; } ;
   void replace( class ssgBase *thing, unsigned int n ) { raw_set( (char *) &thing, n); }
   
   virtual void print ( FILE *fd = stderr, char *indent = "", int how_much = 2 ) {}; // Fixme NIV14: 2do
};


class _nodeIndex
{
 private:
   ssgListOfNodes *nodeList;
 public:
   _nodeIndex() 
     {
	nodeList = new ssgListOfNodes();
     }
   ~_nodeIndex()
     {
	for( int i=0; i<nodeList->getNum(); i++ )
	  {
	     ssgBase *extractedThing = nodeList->get( i );
	     if( extractedThing->getRef() == 0 )
	       delete( extractedThing );
	  }
     }
   
   void insert( ssgBase *thing ) 
     {
	// replace the node if a node with an identical tag already exists
	for( int i=0; i<nodeList->getNum(); i++ ) 
	  {
	     ssgBase *tempThing = nodeList->get( i );
	     if( !strcmp( tempThing->getName(), thing->getName() ) )
	       {
		  nodeList->replace( thing, i );
		  printf("Replaced element %i.\n", i);
		  return;
	       }
	  }
	// otherwise add it to end of list
	nodeList->add( thing );
     }
   
   ssgBase * extract( char *defName )
     {
	for( int i=0; i<nodeList->getNum(); i++ ) 
	  {
	     ssgBase *extractedThing = nodeList->get( i );
	     if( !strcmp( extractedThing->getName(), defName ) ) 
	       return extractedThing;
	  }
	
	return NULL;
     }   
};

// the current properties for a certain point in scene traversal
class _traversalState
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
   
   _traversalState *clone() { return new _traversalState(*this); }
   _traversalState() { vertices = NULL; textureCoordinates = NULL; transform = NULL; texture = NULL; textureCoordinatesArePerFaceAndVertex  = TRUE; enableCullFace = FALSE; }
};

// tags for functions which may actually modify the scene graph
struct _parseTag
{
  char *token ;
  bool (*func) ( ssgBranch *parentBranch, _traversalState *parentData, char *defName ) ;
} ;

// the vrml1 common subset that is shared with inventor
bool vrml1_parseCoordinate3( ssgBranch *parentBranch, _traversalState *currentData, char *defName );
bool vrml1_parseTextureCoordinate2( ssgBranch *parentBranch, _traversalState *currentData, char *defName );
bool vrml1_parseShapeHints( ssgBranch *parentBranch, _traversalState *currentData, char *defName );
bool vrml1_parseMatrixTransform( ssgBranch *parentBranch, _traversalState *currentData, char *defName );
bool vrml1_parseScale( ssgBranch *parentBranch, _traversalState *currentData, char *defName );
bool vrml1_parseRotation( ssgBranch *parentBranch, _traversalState *currentData, char *defName );
bool vrml1_parseUseDirective( ssgBranch *parentBranch, _traversalState *currentData, char *useName, char *defName );
bool vrml1_parseCoordIndex( ssgLoaderWriterMesh *loaderMesh, _traversalState *currentData );
bool vrml1_parseTextureCoordIndex( ssgLoaderWriterMesh *loaderMesh, _traversalState *currentData );
bool parseUnidentified();

void applyTransform( ssgTransform *currentTransform, _traversalState *currentData );
void mergeTransformNodes( ssgTransform *newTransform, ssgTransform *oldTransform1, ssgTransform *oldTransform2 );
bool parseVec( SGfloat *v, int vSize );
ssgIndexArray * parseIndexArray( _traversalState *currentData );

extern _ssgParser vrmlParser;