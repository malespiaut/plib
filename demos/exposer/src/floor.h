
class Floor
{
  unsigned int texhandle ;
         float z_coord   ;

public:
  
  void  setZcoord ( float z ) { z_coord = z ; }
  float getZcoord () { return z_coord ; }

  void draw () ;
  Floor () ;
} ;


