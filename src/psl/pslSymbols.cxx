
#include "pslLocal.h"


PSL_Address PSL_Parser::getVarSymbol ( char *s )
{
  for ( int i = 0 ; i < MAX_SYMBOL ; i++ )
  {
    if ( symtab [ i ] . symbol == NULL )
    {
      if ( next_var >= MAX_VARIABLE-1 )
      {
        ulSetError ( UL_WARNING, "PSL: Too many variables." ) ;
        next_var-- ;
      }

      symtab [ i ] . set ( s, next_var++ ) ;
      return symtab [ i ] . address ;
    }
    else
    if ( strcmp ( s, symtab [ i ] . symbol ) == 0 )
      return symtab [ i ] . address ;
  }

  ulSetError ( UL_WARNING, "PSL: Too many symbols." ) ;
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
    if ( code_symtab [ i ] . symbol == NULL )
    {
      code_symtab [ i ] . set ( s, 0 ) ;
      return code_symtab [ i ] . address ;
    }

    if ( strcmp ( s, code_symtab [ i ] . symbol ) == 0 )
      return code_symtab [ i ] . address ;
  }

  ulSetError ( UL_WARNING, "PSL: Undefined Function '%s'.", s ) ;
  return 0 ;
}



void PSL_Parser::setCodeSymbol ( char *s, PSL_Address v )
{
  for ( int i = 0 ; i < MAX_SYMBOL ; i++ )
  {
    if ( code_symtab [ i ] . symbol == NULL )
    {
      code_symtab [ i ] . set ( s, v ) ;
      return ;
    }
    else
    if ( strcmp ( s, code_symtab [ i ] . symbol ) == 0 )
    {
      code_symtab [ i ] . address = v ;
      return ;
    }
  }

  ulSetError ( UL_WARNING, "PSL: Too many function names." ) ;
}


