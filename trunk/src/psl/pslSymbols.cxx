
#include "pslPrivate.h"


PSL_Address PSL_Parser::getVarSymbol ( char *s )
{
  for ( int i = 0 ; i < MAX_SYMBOL ; i++ )
  {
    if ( symtab [ i ] . symbol == NULL )
    {
      if ( next_var >= MAX_VARIABLE-1 )
      {
        fprintf ( stderr, "PSL: Too many variables.\n" ) ;
        next_var-- ;
      }

      symtab [ i ] . set ( s, next_var++ ) ;
      return symtab [ i ] . address ;
    }
    else
    if ( strcmp ( s, symtab [ i ] . symbol ) == 0 )
      return symtab [ i ] . address ;
  }

  fprintf ( stderr, "PSL: Too many symbols in one program.\n" ) ;
  return MAX_VARIABLE-1 ;
}


int PSL_Parser::getExtensionSymbol ( char *s )
{
  for ( int i = 0 ; extensions [ i ] . symbol != NULL ; i++ )
    if ( strcmp ( s, extensions [ i ] . symbol ) == 0 )
      return i ;

  return -1 ;
}


PSL_Address PSL_Parser::getCodeSymbol ( char *s )
{
  for ( int i = 0 ; i < MAX_SYMBOL ; i++ )
  {
    if ( symtab [ i ] . symbol == NULL )
    {
      symtab [ i ] . set ( s, 0 ) ;
      return 0 ;
    }
    else
    if ( strcmp ( s, symtab [ i ] . symbol ) == 0 )
      return symtab [ i ] . address ;
  }

  fprintf ( stderr, "PSL: Too many symbols in one program.\n" ) ;
  return 0 ;
}



void PSL_Parser::setCodeSymbol ( char *s, PSL_Address v )
{
  for ( int i = 0 ; i < MAX_SYMBOL ; i++ )
  {
    if ( symtab [ i ] . symbol == NULL )
    {
      symtab [ i ] . set ( s, v ) ;
      return ;
    }
    else
    if ( strcmp ( s, symtab [ i ] . symbol ) == 0 )
    {
      symtab [ i ] . address = v ;
      return ;
    }
  }

  fprintf ( stderr, "PSL: Too many symbols in one program.\n" ) ;
}


