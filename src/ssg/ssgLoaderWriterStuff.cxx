// ssgLoaderWriterStuff.cxx
// Here you will find classes and functions you can use to 
// implement loaders and writers for ssg
// Also, there is the parser for loading ASCII files, which 
// has its own file ssgParser.cxx and there are functions like
// the stripifier that are usefull not only for loaders/writers.
//
// 1. Version written by Wolfram Kuss (Wolfram.Kuss@t-online.de) 
// in Nov of 2000
// Distributed with Steve Bakers plib under the LGPL licence

#include  "ssgLocal.h"
#include "ssgLoaderWriterStuff.h"
sgVec4 currentDiffuse;


// ***********************************************************************
// ********************  small utility functions  ************************
// ***********************************************************************

void ssgFindOptConvertTexture( char * filepath, char * tfname ) 
// Find and optionally (= if necessary) convert texture
{
	char tmp[1024], command [1024], *extension ;

	strcpy( tmp, tfname);
	extension = strrchr(tmp, '.');
	if ( extension == NULL )
	{ strcpy( filepath, tfname );
	  return ;
	}
	extension[1] = 'r';
	extension[2] = 'g';
	extension[3] = 'b';
	extension[4] = 0;
			
	ulFindFile( filepath, _ssgTexturePath, tmp, ssgGetAPOM() ) ;
	if ( ulFileExists ( filepath ) )
		return; // found *.rgb-file
	// look for original, non-rgb - file
  ulFindFile( filepath, _ssgTexturePath, tfname, ssgGetAPOM() ) ;
	if ( !ulFileExists ( filepath ) )
		return; // found *.rgb nor original file
	// found original file. convert it.
	strcpy( tmp, filepath );

	extension = strrchr(tmp, '.');
	if ( extension == NULL )
	{ strcpy( filepath, tfname );
	  return ;
	}
	extension[1] = 'r';
	extension[2] = 'g';
	extension[3] = 'b';
	extension[4] = 0;

#ifdef WIN32
	sprintf(command, "convert -verbose %s sgi:%s", filepath, tmp);
	unsigned int ui = WinExec(command, SW_HIDE );	
	if ( ui < 32 )
		ulSetError(UL_WARNING, "Couldn't convert texture. Did you install ImageMagick?");
#else
  ulSetError(UL_WARNING, "Converting textures not yet implemented. Please convert %s manually.",
		    filepath);
	//sprintf(command, "-verbose %s sgi:%s", filepath, tmp);
	//execlp ( "convert", "convert",  command, NULL ) ;

#endif
	// Pfusch: Kludge; warning?
	strcpy( filepath, tmp );
}



// ***********************************************************************
// ******************** class ssgLoaderWriterMesh ************************
// ***********************************************************************
 


void ssgLoaderWriterMesh::ReInit(void)
{
  theVertices					= NULL ; 
	materialIndexes			= NULL ; 
	theFaces						= NULL ; 
	tCPFAV							= NULL ; 
	theMaterials				= NULL ;
	tCPV								= NULL ;
	bTCs_are_per_vertex	= TRUE ;
}
	
ssgLoaderWriterMesh::ssgLoaderWriterMesh()
{ ReInit();
}

ssgLoaderWriterMesh::~ssgLoaderWriterMesh()
{}

void ssgLoaderWriterMesh::deleteTCPFAV()
{}

// creation stuff:

void ssgLoaderWriterMesh::ThereAreNVertices( int n ) 
{
	assert( theVertices == NULL );
	theVertices = new ssgVertexArray ( n );
}

void ssgLoaderWriterMesh::addVertex ( sgVec3 v ) 
{
	theVertices->add ( v );
}


void ssgLoaderWriterMesh::ThereAreNFaces( int n ) 
{
	assert( theFaces == NULL );
	theFaces = new ssgListOfLists ( n );
}

void ssgLoaderWriterMesh::addFace ( ssgIndexArray **ia ) 
{
	theFaces->add ( (ssgSimpleList **)ia );
}


void ssgLoaderWriterMesh::ThereAreNTCPFAV( int n ) 
{
	assert( tCPFAV == NULL );
	tCPFAV = new ssgListOfLists ( n );
}

void ssgLoaderWriterMesh::addTCPFAV ( ssgTexCoordArray **tca ) 
{
	tCPFAV->add ( (ssgSimpleList **)tca );
}

void ssgLoaderWriterMesh::ThereAreNTCPV( int n ) 
{
	assert( tCPV == NULL );
	tCPV = new ssgTexCoordArray ( n );
}

void ssgLoaderWriterMesh::addTCPV ( sgVec2 tc ) 
{
	tCPV->add ( tc );
}



void ssgLoaderWriterMesh::ThereAreNMaterialIndexes( int n ) 
{
	assert( materialIndexes == NULL );
	materialIndexes = new ssgIndexArray ( n );
}

void ssgLoaderWriterMesh::addMaterialIndex ( short mi ) 
{
	materialIndexes->add ( mi );
}


void ssgLoaderWriterMesh::ThereAreNMaterials( int n ) 
{
	assert( theMaterials == NULL );
	theMaterials = new ssgSimpleStateList ( n );
}

void ssgLoaderWriterMesh::addMaterial ( class ssgSimpleState **ss ) 
{
	theMaterials->add ( ss );
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
		float f= sqrt(n[0]*n[0]+n[1]*n[1]+n[2]*n[2]);
		if (f>0.00001)
		{ f=1.0/f;
		  n[0] *= f; n[1] *= f; n[2] *= f;
			assert(((n[0]*n[0]+n[1]*n[1]+n[2]*n[2])-1)<0.001);
			assert(((n[0]*n[0]+n[1]*n[1]+n[2]*n[2])-1)>-0.001);
		}
		sgCopyVec3( nl->get(ix0), n );
    sgCopyVec3( nl->get(ix1), n );
    sgCopyVec3( nl->get(ix2), n );
  }
}

void ssgLoaderWriterMesh::AddFaceFromCArray(int nNoOfVerticesForThisFace, 
																						int *aiVertices)
{
	int j;
	class ssgIndexArray *oneFace = new ssgIndexArray( nNoOfVerticesForThisFace ); 
	oneFace->ref();
	for(j=0;j<nNoOfVerticesForThisFace;j++)
		oneFace->add(aiVertices[j]);
	theFaces->add( (ssgSimpleList **) &oneFace ); 

}


void ssgLoaderWriterMesh::AddOneNode2SSG(class ssgVertexArray *theVertices, 
	class ssgListOfLists *theFaces,
	class ssgSimpleState *currentState,// Pfusch 
	class ssgLoaderOptions* current_options,
	class ssgBranch *curr_branch_)

{ int i, j;
		//start Normalen, (z.T.?) Pfusch

	ssgNormalArray *nl = new ssgNormalArray(theVertices->getNum());
	sgVec3 Pfusch;
	for (i=0;i<theVertices->getNum();i++)
		nl->add(Pfusch); //currentMesh.vl->get(i));


	class ssgIndexArray* il = new ssgIndexArray ( theFaces->getNum() * 3 ) ; // there are MINIMAL n * 3 indexes

	for(i=0;i<theFaces->getNum();i++)
	{
		class ssgIndexArray *oneFace = *((class ssgIndexArray **) theFaces->get( i )); 
		if ( oneFace->getNum() >= 3 )
		{	for(j=0;j<oneFace->getNum();j++)
			{ 
				if (j<3)
					il->add(*oneFace->get(j));
				else // add a complete triangle
				{ il->add(*oneFace->get(0));
					il->add(*oneFace->get(j-1));
					il->add(*oneFace->get(j));
				}
			}
		}
	}
	recalcNormals(il,theVertices, nl); // Pfusch kludge; only do this if there are no nmormals in the file
	
	ssgColourArray* cl = NULL ;
  
  if ( currentState -> isEnabled ( GL_LIGHTING ) )
  {
    if ( cl == NULL )
    {
      cl = new ssgColourArray ( 1 ) ;
      cl -> add ( currentDiffuse ) ;
    }
  }

	ssgVtxArray* leaf = new ssgVtxArray ( GL_TRIANGLES,
		theVertices, nl , 
		PfuschGettCPV(), // super Pfusch kludge
		cl, il ) ;
	leaf -> setCullFace ( TRUE ) ;


// debug
/*  float *af=currentState->getMaterial ( GL_DIFFUSE  );
	parser.message("vor leaf-SetState: af[0] = %f, %f, %f, %f\n",
					af[0], af[1], af[2], af[3]);*/
// debug stop

	leaf -> setState ( currentState ) ;
	//return 
	current_options -> createLeaf ( leaf, NULL) ;
	curr_branch_->addKid(leaf);
}

void ssgLoaderWriterMesh::add2SSG(
		class ssgSimpleState *currentState,// Pfusch 
		class ssgLoaderOptions* current_options,
		class ssgBranch *curr_branch_)
{ int i, j, k;
  unsigned short oldVertexIndex, newVertexIndex;
	class ssgIndexArray *thisFace;

  

	
	//stop Normalen
	
	if ( theMaterials == NULL )
		fprintf(stdout, "( theMaterials == NULL )\n");
	else
	{	
		fprintf(stdout, "%d Materials:\n", theMaterials->getNum());
		for(i=0;i<theMaterials->getNum();i++)
		{ fprintf(stdout, "%ld\n", (long)theMaterials->get(i));
		}
	}
	if ( materialIndexes == NULL )
		fprintf(stdout, "( materialIndexes == NULL )\n");
	else
	{
		fprintf(stdout, "%d Material Indexes:\n", materialIndexes->getNum());
		for(i=0;i<materialIndexes->getNum();i++)
		{ short s=*(materialIndexes->get(i));
			fprintf(stdout, "%ld\n", (long)s);
		}
	}

	if ( theMaterials == NULL )
		AddOneNode2SSG(theVertices, theFaces, currentState, current_options, curr_branch_);
	else
	{	
		for(i=0;i<theMaterials->getNum();i++)
		{	
			// I often allocate too much; This is wastefull on memory, but fast since it never "resizes":
			class ssgVertexArray *newVertices = new ssgVertexArray ( theVertices->getNum() );
			class ssgListOfLists *newFaces = new ssgListOfLists ( theFaces->getNum() );
			class ssgIndexArray *oldVertexInd2NewVertexInd = new ssgIndexArray ( theVertices->getNum() );
			for (j=0;j<theVertices->getNum();j++)
				oldVertexInd2NewVertexInd->add ( 0xFFFF ); // 0xFFFF stands for "unused in new Mesh"

			// Go through all the old Faces, look for the correct material and copy those
			// faces and indexes into the new
			// Pfusch, kludge, 2do: if the Materials just differ through the colour, one would not need
			// several meshes, but could use the colour array. However, this is not possible,
			// if they differ by for example the texture
					thisFace = *((class ssgIndexArray **) theFaces->get( 0 )); 
					thisFace = *((class ssgIndexArray **) theFaces->get( 1 )); 
			for (j=0;j<theFaces->getNum();j++)
				if ( i == *(materialIndexes->get(
					       // for *.x-files, there may be less materialIndexes than faces. I then simply repeat 
								 // the last index all the time:
					            j<materialIndexes->getNum() ? j : materialIndexes->getNum()-1 )) 
											)
				{ 
					// take this face
					thisFace = *((class ssgIndexArray **) theFaces->get( j )); 
					newFaces->add( theFaces->get( j ) );
					for(k=0;k<thisFace->getNum();k++)
					{ oldVertexIndex = * thisFace->get(k);
					  newVertexIndex = *oldVertexInd2NewVertexInd->get(oldVertexIndex);
					  if ( 0xFFFF == newVertexIndex )
						{ newVertexIndex = newVertices->getNum();
						  newVertices->add ( theVertices->get(oldVertexIndex) );
							oldVertexInd2NewVertexInd->set(newVertexIndex, oldVertexIndex);
						}
				    thisFace->set(newVertexIndex, k);
					}	
				}
			fprintf(stdout, "NumVert: %d\n", newVertices->getNum());
			for(j=0;j<newVertices->getNum();j++)
			{ float *f=newVertices->get(j);
			  fprintf(stdout, "%f, %f, %f\n",f[0], f[1], f[2]);
			}
			for(j=0;j<newFaces->getNum();j++)
			{
				thisFace = *((class ssgIndexArray **) newFaces->get( j )); 	
				fprintf(stdout, "%d EP:", thisFace->getNum());
				for(k=0;k<thisFace->getNum();k++)
				{ oldVertexIndex = * thisFace->get(k);
				  fprintf(stdout, "%d, ", oldVertexIndex);
				}
				fprintf(stdout, "\n");
				
			}

			if ( newFaces->getNum() > 0 )
			{
				currentState = *theMaterials->get(i);
			  AddOneNode2SSG(newVertices, newFaces, currentState, current_options, curr_branch_);
			}
		}
	}
}
	
int ssgLoaderWriterMesh::checkMe()
// returns TRUE; if ok.
// Writes out errors by calling ulSetError with severity UL_WARNING,
// and a bit of debug info as UL_DEBUG
// May stop on first error.

// Pfusch; todo: tCPV and tCPFAV
{ int i, oneIndex;
  class ssgIndexArray * vertexIndsForOneFace;
	class ssgTexCoordArray * textureCoordsForOneFace;

  // **** check theVertices *****
	if ( theVertices == NULL )
	{ if (( materialIndexes == NULL ) &&
				(theFaces == NULL ) &&
				( tCPFAV == NULL ))
		{	ulSetError( UL_DEBUG, "LoaderWriterMesh::checkMe(): The mesh is empty\n");
			return TRUE;
		}
		else
		{	ulSetError( UL_WARNING, "LoaderWriterMesh::checkMe(): No theVertices is NULL; but not the rest!\n");
			return FALSE;
		}

	}
	// **** check materialIndexes and theMaterials *****
	/* Pfusch; kludge 2do
	// one index per face:
	class ssgIndexArray *materialIndexes; 
	theMaterials
	*/
	if ((( theMaterials == NULL ) && ( materialIndexes != NULL )) ||
		  (( theMaterials != NULL ) && ( materialIndexes == NULL )))
	{	ulSetError( UL_WARNING, "LoaderWriterMesh::checkMe(): "
	                     "One of theMaterials and materialIndexes was NULL and the other != NULL!\n");
		return FALSE;
	}
	if ( materialIndexes != NULL ) 
	{ for (i=0;i<materialIndexes->getNum();i++)
		{ oneIndex = *materialIndexes->get(i);
	    if (( oneIndex < 0 ) || ( oneIndex >= theMaterials->getNum()))
			{	ulSetError( UL_WARNING, "LoaderWriterMesh::checkMe(): "
													 "Material index out of range. Index = %d, "
													 "theMaterials->getNum() = %d.\n",
													 oneIndex, theMaterials->getNum());
				return FALSE;
			}
		}
	}


	// **** check theFaces *****
	// Each sublist is of type ssgIndexArray and contains the indexes of the vertices:
	if ( theFaces == NULL )
	{	ulSetError( UL_WARNING, "LoaderWriterMesh::checkMe(): There are vertices but no faces.\n");
		return FALSE;
	}
	for(i=0;i<theFaces->getNum();i++)
	{ vertexIndsForOneFace = *((ssgIndexArray **) theFaces->get ( i ));
	  if ( vertexIndsForOneFace == NULL )
		{	ulSetError( UL_WARNING, "LoaderWriterMesh::checkMe(): the vertexindexes for one face are NULL!\n");
			return FALSE;
		}
	}
	// **** check textureCoordinates *****
  // Each sublist is of type ssgTexCoordArray and contains the texture coordinates
	if ( tCPFAV != NULL ) // may be NULL
	{ if ( theFaces->getNum() != tCPFAV->getNum())
		{	ulSetError( UL_WARNING, "LoaderWriterMesh::checkMe(): "
		              "There must be as many faces in theFaces as in textureCoordinates. But "
		              "theFaces->getNum() =%d, tCPFAV->getNum() = %d!\n",
									theFaces->getNum(), tCPFAV->getNum());
			return FALSE;
		}
		for(i=0;i<tCPFAV->getNum();i++)
		{ textureCoordsForOneFace = *((ssgTexCoordArray **) tCPFAV->get ( i ));
			if ( textureCoordsForOneFace  != NULL ) // It is allowed that some or even all faces are untextured.
			{ vertexIndsForOneFace = *((ssgIndexArray **) theFaces->get ( i ));
			  if ( textureCoordsForOneFace->getNum() != vertexIndsForOneFace ->getNum())
				{	ulSetError( UL_WARNING, "LoaderWriterMesh::checkMe(): Face %d: "
											"Each face must have as many texture corrdinates (or none) as vertices. But "
											"textureCoordsForOneFace->getNum() =%d, vertexIndsForOneFace ->getNum() = %d!\n",
											i, textureCoordsForOneFace->getNum(), vertexIndsForOneFace ->getNum());
					return FALSE;
				}
			}
		}
	}
	return TRUE; // success
}
