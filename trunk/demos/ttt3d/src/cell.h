

ssgTransform *makeBlueWireCube() ;
ssgTransform *makeGround() ;

#define WIRE_CELL 0
#define    X_CELL 1
#define    O_CELL 2

class Cell
{
  sgCoord c ;
  ssgTransform *posn ;
  ssgSelector  *cell ;
  int what ;

public:

  Cell ( int x, int y, int z, int size ) ;

  ssgBranch *getSSG () { return posn ; }

  int  get () { return what ; }

  void set ( int n )
  {
    what = n ;
    cell -> selectStep ( n ) ;
  }

} ;


