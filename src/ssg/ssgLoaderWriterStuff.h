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

//#include  "ssgLocal.h"

// ********************  small utility functions  ************************

void ssgFindOptConvertTexture( char * filepath, char * tfname ) ;
void ssgAccumVerticesAndFaces( ssgEntity* node, sgMat4 transform, ssgVertexArray* vertices,
																ssgIndexArray*  indices, SGfloat epsilon, 
																ssgSimpleStateArray* ssa = NULL,
																ssgIndexArray*  materialIndices = NULL);

/*
  ssgTriangulate -
  Triangulate a simple polygon (possibly concave, but not self-intersecting).
  The number of triangles written to 'triangles' is returned, which is always
  less than or equal to num - 2. The polygon index array 'indices' may be null,
  in which case an identity mapping is assumed.
  Note that the implementation is optimized for small polygons, and since 
  the algorithm is O(num^2) it is not efficient on large polygons.
*/
int _ssgTriangulate( sgVec3 *vertices, int *indices, int num, int *triangles );


// ******************** class ssgLoaderWriterMesh ************************

class ssgListOfLists : public ssgSimpleList
// list of POINTERs to ssgSimpleLists
{
public:

  virtual ssgBase *clone ( int clone_flags = 0 ) { return NULL; }; // Fixme NIV14: 2do
  ssgListOfLists ( int init = 3 ) : ssgSimpleList ( sizeof(class ssgSimpleList*), init ) {} 
  class ssgSimpleList **get ( unsigned int n ) { return (class ssgSimpleList **) raw_get ( n ) ; }
  void   add ( class ssgSimpleList **thing ) { raw_add ( (char *) thing ) ; } ;
	void   set ( class ssgSimpleList **thing, unsigned int n ) { raw_set ( (char *) thing, n ) ; } ;
  
  virtual void print ( FILE *fd = stderr, char *indent = "", int how_much = 2 ) {}; // Fixme NIV14: 2do
} ;

//ssgSimpleState* 

class ssgSimpleStateList : public ssgSimpleList
// list of POINTERs to ssgSimpleStates
{
public:

  virtual ssgBase *clone ( int clone_flags = 0 ) { return NULL;  }; // Fixme NIV14: 2do
  ssgSimpleStateList( int init = 3 ) : ssgSimpleList ( sizeof(class ssgSimpleState*), init ) {} 
  class ssgSimpleState **get ( unsigned int n ) { return (class ssgSimpleState **) raw_get ( n ) ; }
  void   add ( class ssgSimpleState **thing ) { raw_add ( (char *) thing ) ; } ;
  void   set ( class ssgSimpleState **thing, unsigned int n  ) { raw_set ( (char *) thing, n ) ; } ;
  virtual void print ( FILE *fd = stderr, char *indent = "", int how_much = 2 ) {}; // Fixme NIV14: 2do
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
	
	void AddOneNode2SSGFromCPV(class ssgVertexArray *theVertices, 
		class ssgTexCoordArray *theTC,
		class ssgListOfLists *theFaces,
		class ssgSimpleState *currentState,// Pfusch, kludge. NIV135
		class ssgLoaderOptions* current_options,
		class ssgBranch *curr_branch_);
	void AddOneNode2SSGFromCPFAV(class ssgVertexArray *theVertices, 
		class ssgListOfLists *theTCPFAV,
		class ssgListOfLists *theFaces,
		class ssgSimpleState *currentState,// Pfusch, kludge. NIV135
		class ssgLoaderOptions* current_options,
		class ssgBranch *curr_branch_);

public:
	class ssgVertexArray *PfuschGetTheVertices(void) { return theVertices; }; 
	class ssgTexCoordArray *PfuschGettCPV(void) { return tCPV; } ;
	void AddFaceFromCArray(int nNoOfVerticesForThisFace, 
																						int *vertices);
  
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


