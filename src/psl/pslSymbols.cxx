
#include "pslLocal.h"


PSL_Address PSL_Parser::setVarSymbol ( char *s )
{
  for ( int i = 0 ; i < next_var ; i++ )
    if ( strcmp ( s, symtab [ i ] . symbol ) == 0 )
    {
      ulSetError ( UL_WARNING, "PSL: Multiple definition of '%s'.", s ) ;
      return symtab [ i ] . address ;
    }

  if ( next_var >= MAX_VARIABLE-1 )
  {
    ulSetError ( UL_WARNING, "PSL: Too many variables." ) ;
    next_var-- ;
  }

  symtab [ next_var ] . set ( s, next_var ) ;

  return symtab [ next_var++ ] . address ;
}



PSL_Address PSL_Parser::getVarSymbol ( char *s )
{
  for ( int i = 0 ; i < next_var ; i++ )
    if ( strcmp ( s, symtab [ i ] . symbol ) == 0 )
      return symtab [ i ] . address ;

  ulSetError ( UL_WARNING, "PSL: Undefined symbol '%s'.", s ) ;

  return setVarSymbol ( s ) ;
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
  for ( int i = 0 ; i < next_code_symbol ; i++ )
    if ( strcmp ( s, code_symtab [ i ] . symbol ) == 0 )
      return code_symtab [ i ] . address ;

  ulSetError ( UL_WARNING, "PSL: Undefined Function '%s'.", s ) ;

  setCodeSymbol ( s, 0 ) ;

  return 0 ;
}



void PSL_Parser::setCodeSymbol ( char *s, PSL_Address v )
{
  for ( int i = 0 ; i < next_code_symbol ; i++ )
    if ( strcmp ( s, code_symtab [ i ] . symbol ) == 0 )
    {
      ulSetError ( UL_WARNING, "PSL: Multiple definition of '%s'.", s ) ;
      code_symtab [ i ] . address = v ;
      return ;
    }

  if ( next_code_symbol >= MAX_VARIABLE-1 )
  {
    ulSetError ( UL_WARNING, "PSL: Too many labels." ) ;
    next_code_symbol-- ;
  }

  code_symtab [ next_code_symbol++ ] . set ( s, v ) ;
}



