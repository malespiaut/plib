#include "ssgLocal.h"

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


class vrmlNodeIndex
{
 private:
   ssgListOfNodes *nodeList;
 public:
   vrmlNodeIndex() 
     {
	nodeList = new ssgListOfNodes();
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


   
   /*
// a linked list for storing USE instances
// (yes, I know its not robust and it breaks
// all the rules I tried so hard to follow elsewhere), 
// fix it if you feel like it
class vrmlNodeElement
{
 public:
   vrmlNodeElement( ssgBase *element ) 
     {
	this->element = element;
	next=NULL;
     }
   
   ssgBase *element;
   vrmlNodeElement *next;
}
*/
