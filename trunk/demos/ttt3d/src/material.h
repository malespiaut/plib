
struct Material
{
public:
  ssgSimpleState **gst ;

  char *texture_map  ;
  int   clamp_tex    ;
  int   transparency ;
  float alpha_ref    ;
  int   lighting     ;
  float friction     ;
  unsigned int flags ;

  int  isNull () { return gst == NULL ; } ;
  void install ( int index ) ;
  
  ssgState *getState    () { return *gst ; }
  char     *getTexFname () { return texture_map ; }
} ;


void initMaterials () ;
/*Material *getMaterial ( ssgState *s ) ;
Material *getMaterial ( ssgLeaf  *l ) ;*/

extern ssgSimpleState *default_gst, *O_gst, *X_gst, *ground_gst, *ctrls_gst ;

