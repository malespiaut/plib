
class pslSymbol
{
public:
  char      *symbol   ;
  pslAddress address  ;
  int        locality ;

  pslSymbol ()
  {
    symbol   = NULL ;
    address  = 0 ;
    locality = 0 ;
  }

  void set ( const char *s, pslAddress v, int loc )
  {
    symbol   = new char [ strlen ( s ) + 1 ] ;
    strcpy ( symbol, s ) ;
    address  = v ;
    locality = loc ;
  }

  ~pslSymbol () { delete symbol ; }
} ;


