
#include "pslLocal.h"


int PSL_Parser::pushPrimitive ()
{
  char c [ MAX_TOKEN ] ;
  getToken ( c ) ;

  if ( c [ 0 ] == '(' )
  {
    if ( ! pushExpression () )
    {
      ulSetError ( UL_WARNING, "PSL: Missing expression after '('" ) ;
      ungetToken ( c ) ;
      return FALSE ;
    }

    getToken ( c ) ;

    if ( c [ 0 ] != ')' )
    {
      ulSetError ( UL_WARNING, "PSL: Missing ')' (found '%s')", c );
      ungetToken ( c ) ;
      return FALSE ;
    }

    return TRUE ;
  }

  if ( c [ 0 ] == '+' )    /* Skip over any unary '+' symbols */
  {
    if ( pushPrimitive () )
      return TRUE ;
    else
    {
      ungetToken ( c ) ;
      return FALSE ;
    }
  }

  if ( c [ 0 ] == '-' )  /* Unary minus */
  {
    if ( pushPrimitive () )
    {
      pushNegate () ;
      return TRUE ;
    }
    else
    {
      ungetToken ( c ) ;
      return FALSE ;
    }
  }

  if ( isdigit ( c [ 0 ] ) || c [ 0 ] == '.' )
  {
    pushConstant ( c ) ;
    return TRUE ;
  }

  if ( isalpha ( c [ 0 ] ) || c [ 0 ] == '_' )
  {
    char n [ MAX_TOKEN ] ;
    getToken ( n ) ;
    ungetToken ( n ) ;

    if ( n[0] == '(' )
      pushFunctionCall ( c ) ;
    else
      pushVariable ( c ) ;

    return TRUE ;
  }

  ungetToken ( c ) ;
  return FALSE ;
}



int PSL_Parser::pushMultExpression ()
{
  if ( ! pushPrimitive () )
    return FALSE ;

  while ( TRUE )
  {
    char c [ MAX_TOKEN ] ;

    getToken ( c ) ;

    if ( c [ 0 ] != '*' && c [ 0 ] != '/' )
    {
      ungetToken ( c ) ;
      return TRUE ;
    }

    if ( ! pushPrimitive () )
      return FALSE ;

    if ( c [ 0 ] == '*' )
      pushMultiply () ;
    else
      pushDivide () ;
  }
}




int PSL_Parser::pushAddExpression ()
{
  if ( ! pushMultExpression () )
    return FALSE ;

  while ( TRUE )
  {
    char c [ MAX_TOKEN ] ;

    getToken ( c ) ;

    if ( c [ 0 ] != '+' && c [ 0 ] != '-' )
    {
      ungetToken ( c ) ;
      return TRUE ;
    }

    if ( ! pushMultExpression () )
      return FALSE ;

    if ( c [ 0 ] == '+' )
      pushAdd () ;
    else
      pushSubtract () ;
  }
}




int PSL_Parser::pushRelExpression ()
{
  if ( ! pushAddExpression () )
    return FALSE ;

  while ( TRUE )
  {
    char c [ MAX_TOKEN ] ;

    getToken ( c ) ;

    if ( c [ 0 ] != '<' &&
         c [ 0 ] != '>' &&
         c [ 0 ] != '!' &&
         c [ 0 ] != '=' )
    {
      ungetToken ( c ) ;
      return TRUE ;
    }

    char c2 [ MAX_TOKEN ] ;

    getToken ( c2 ) ;

    if ( c2 [ 0 ] == '=' )
    {
      c[1] = '=' ;
      c[2] = '\0' ;
    }
    else
      ungetToken ( c2 ) ;

    if (( c [ 0 ] == '!' || c [ 0 ] == '=' ) && c [ 1 ] != '=' )
    {
      ungetToken ( c2 ) ;
      return TRUE ;
    }

    if ( ! pushMultExpression () )
      return FALSE ;

    if ( c [ 0 ] == '<' )
    {
      if ( c [ 1 ] == '=' )
        pushLessEqual () ;
      else
        pushLess () ;
    }
    else
    if ( c [ 0 ] == '>' )
    {
      if ( c [ 1 ] == '=' )
        pushGreaterEqual () ;
      else
        pushGreater () ;
    }
    else
    if ( c [ 0 ] == '!' )
      pushNotEqual () ;
    else
      pushEqual () ;
  }
}


int PSL_Parser::pushExpression ()
{
  return pushRelExpression () ;
}


