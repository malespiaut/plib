
class pslContext
{
  pslOpcode    *code       ;
  pslExtension *extensions ;
  pslProgram   *program    ;

  pslVariable   variable [ MAX_VARIABLE ] ;
  pslVariable   stack    [ MAX_STACK    ] ; 
  int            sp ;
  pslAddress    pc ;

public:

  pslContext ( pslProgram *p )
  {
    code       = p -> getCode       () ;
    extensions = p -> getExtensions () ;
    program    = p ;
    reset () ;
  }

  ~pslContext () {} ;

  void pushInt      ( int          x ) { stack [ sp++ ] . i = x ; }
  void pushFloat    ( float        x ) { stack [ sp++ ] . f = x ; }
  void pushVariable ( pslVariable x ) { stack [ sp++ ]     = x ; }

  void         popVoid     () {                --sp       ; }
  int          popInt      () { return stack [ --sp ] . i ; }
  float        popFloat    () { return stack [ --sp ] . f ; }
  pslVariable popVariable () { return stack [ --sp ]     ; }

  pslResult step () ;

  void reset ()
  {
    memset ( variable, 0, MAX_VARIABLE * sizeof ( pslVariable ) ) ;
    sp = 0 ;
    pc = 0 ;
  }
} ;


