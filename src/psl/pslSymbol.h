
class pslSymbol
{
public:
  char *symbol ;
  pslAddress address ;

  pslSymbol ()
  {
    symbol = NULL ;
    address = 0 ;
  }

  void set ( const char *s, pslAddress v )
  {
    symbol = new char [ strlen ( s ) + 1 ] ;
    strcpy ( symbol, s ) ;
    address = v ;
  }

  ~pslSymbol () { delete symbol ; }
} ;


