//#include  "ssgLocal.h"

// ********************  small utility functions  ************************

void ssgFindOptConvertTexture( char * filepath, char * tfname ) ;

// ***************** class CGlobalSimpleStateList  *******************

// ******************** class ssgLoaderWriterMesh ************************

class ssgListOfLists : public ssgSimpleList
// list of POINTERs to ssgSimpleLists
{
public:

  virtual ssgBase *clone ( int clone_flags = 0 ) { return NULL; }; // Pfusch 2do
  ssgListOfLists ( int init = 3 ) : ssgSimpleList ( sizeof(class ssgSimpleList*), init ) {} 
  class ssgSimpleList **get ( unsigned int n ) { return (class ssgSimpleList **) raw_get ( n ) ; }
  void   add ( class ssgSimpleList **thing ) { raw_add ( (char *) thing ) ; } ;
	void   set ( class ssgSimpleList **thing, unsigned int n ) { raw_set ( (char *) thing, n ) ; } ;
  
  virtual void print ( FILE *fd = stderr, char *indent = "", int how_much = 2 ) {}; // Pfusch 2do
} ;

//ssgSimpleState* 

class ssgSimpleStateList : public ssgSimpleList
// list of POINTERs to ssgSimpleStates
{
public:

  virtual ssgBase *clone ( int clone_flags = 0 ) { return NULL;  }; // Pfusch 2do
  ssgSimpleStateList( int init = 3 ) : ssgSimpleList ( sizeof(class ssgSimpleState*), init ) {} 
  class ssgSimpleState **get ( unsigned int n ) { return (class ssgSimpleState **) raw_get ( n ) ; }
  void   add ( class ssgSimpleState **thing ) { raw_add ( (char *) thing ) ; } ;
  void   set ( class ssgSimpleState **thing, unsigned int n  ) { raw_set ( (char *) thing, n ) ; } ;
  virtual void print ( FILE *fd = stderr, char *indent = "", int how_much = 2 ) {}; // Pfusch 2do
} ;

class ssgLoaderWriterMesh
{
	// ***** general ****
	// array of Vec3s:
  class ssgVertexArray *theVertices; 
	// one index per face:
	class ssgIndexArray *materialIndexes; 

	// Each sublist is of type ssgIndexArray and contains the indexes of the vertices:
	class ssgListOfLists *theFaces; 
  // material list: 
	class ssgSimpleStateList *theMaterials;

	// ***** mode switches *****
	int bTCs_are_per_vertex; // and not per vertex and face
	// ***** complicated mode *****
	// Each sublist is of type ssgTexCoordArray and contains the texture coordinates
	class ssgListOfLists *tCPFAV; // TCPFAV = TextureCoordinatesPerFaceAndVertex
	
	// ***** easy mode *********
	class ssgTexCoordArray *tCPV; // TCPV = TextureCoordinatesPerVertex
	
	void AddOneNode2SSG(class ssgVertexArray *theVertices, 
		class ssgListOfLists *theFaces,
		class ssgSimpleState *currentState,// Pfusch 
		class ssgLoaderOptions* current_options,
		class ssgBranch *curr_branch_);

public:
	class ssgVertexArray *PfuschGetTheVertices(void) { return theVertices; }; 
	class ssgTexCoordArray *PfuschGettCPV(void) { return tCPV; } ;
	void AddFaceFromCArray(int nNoOfVerticesForThisFace, 
																						int *aiVertices);

	void add2SSG(
		class ssgSimpleState *currentstate, 
		class ssgLoaderOptions* current_options,
		class ssgBranch *curr_branch_);


	
	// construction/destruction:
	ssgLoaderWriterMesh();
	~ssgLoaderWriterMesh();
	void ReInit(void);
	void deleteTCPFAV();

  // creation:
	void ThereAreNVertices( int n = 8 ) ;
	void addVertex ( sgVec3 v ) ;

	void ThereAreNFaces( int n = 3 ) ;
	void addFace ( ssgIndexArray **ia ) ;

	void ThereAreNTCPFAV( int n = 3 ) ;
	void addTCPFAV ( ssgTexCoordArray **tca ) ;

	void ThereAreNTCPV( int n = 3 ) ;
	void addTCPV ( sgVec2 tc ) ;

	void ThereAreNMaterialIndexes( int n = 3 ) ;
	void addMaterialIndex ( short mi ) ;

	void ThereAreNMaterials( int n = 3 ) ;
	void addMaterial ( class ssgSimpleState **ss ) ;

  unsigned int getNumVertices(void) { return theVertices->getNum(); } ;
	unsigned int getNumFaces   (void) { return theFaces->getNum(); } ;
	unsigned int getNumMaterials(void) { return theMaterials->getNum(); } ;


	// tools:
	int checkMe();
};


