/*
     PLIB - A Suite of Portable Game Libraries
     Copyright (C) 1998,2002  Steve Baker

     This library is free software; you can redistribute it and/or
     modify it under the terms of the GNU Library General Public
     License as published by the Free Software Foundation; either
     version 2 of the License, or (at your option) any later version.

     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Library General Public License for more details.

     You should have received a copy of the GNU Library General Public
     License along with this library; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

     For further information visit http://plib.sourceforge.net

     $Id$
*/


#include "pslLocal.h"


int pslCompiler::pushPrimitive ()
{
  char c [ MAX_TOKEN ] ;
  getToken ( c ) ;

  if ( strcmp ( c, "(" ) == 0 )
  {
    if ( ! pushExpression () )
    {
      ungetToken ( c ) ;
      return error ( "Missing expression after '('" ) ;
    }

    getToken ( c ) ;

    if ( strcmp ( c, ")" ) != 0 )
    {
      ungetToken ( c ) ;
      return error ( "Missing ')' (found '%s')", c );
    }

    return TRUE ;
  }

  if ( strcmp ( c, "+" ) == 0 )    /* Skip over any unary '+' symbols */
  {
    if ( pushPrimitive () )
      return TRUE ;
    else
    {
      ungetToken ( c ) ;
      return FALSE ;
    }
  }

  if ( strcmp ( c, "!" ) == 0 )    /* Skip over any unary '!' symbols */
  {
    if ( pushPrimitive () )
    {
      pushNot () ;
      return TRUE ;
    }
    else
    {
      ungetToken ( c ) ;
      return FALSE ;
    }
  }

  if ( strcmp ( c, "~" ) == 0 )    /* Skip over any unary '~' symbols */
  {
    if ( pushPrimitive () )
    {
      pushTwiddle () ;
      return TRUE ;
    }
    else
    {
      ungetToken ( c ) ;
      return FALSE ;
    }
  }

  if ( strcmp ( c, "-" ) == 0 )    /* Unary '-' */
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

  if ( c [ 0 ] == '"' )
  {
    pushStringConstant ( & c [ 1 ] ) ;
    return TRUE ;
  }

  if ( isdigit ( c [ 0 ] ) || c [ 0 ] == '.' )
  {
    pushConstant ( c ) ;
    return TRUE ;
  }

  int preInc = FALSE ;
  int preDec = FALSE ;

  if ( strcmp ( c, "++" ) == 0 )
  {
    preInc = TRUE ;
    getToken ( c ) ;
  }
  else
  if ( strcmp ( c, "--" ) == 0 )
  {
    preDec = TRUE ;
    getToken ( c ) ;
  }

  if ( isalpha ( c [ 0 ] ) || c [ 0 ] == '_' )
  {
    char n [ MAX_TOKEN ] ;
    getToken ( n ) ;
    ungetToken ( n ) ;

    if ( n[0] == '(' )
    {
      if ( preInc || preDec )
        error ( "You can't apply '++' or '--' to a function call!" ) ;

      pushFunctionCall ( c ) ;
    }
    else
    {
      if ( preInc ) pushIncrement ( c ) ;
      if ( preDec ) pushDecrement ( c ) ;

      pushVariable ( c ) ;

      getToken ( n ) ;

      if ( strcmp ( n, "++" ) == 0 ) pushIncrement ( c ) ; else
      if ( strcmp ( n, "--" ) == 0 ) pushDecrement ( c ) ; else
        ungetToken ( n ) ;
    }

    return TRUE ;
  }

  ungetToken ( c ) ;
  return FALSE ;
}



int pslCompiler::pushMultExpression ()
{
  if ( ! pushPrimitive () )
    return FALSE ;

  while ( TRUE )
  {
    char c [ MAX_TOKEN ] ;

    getToken ( c ) ;

    if ( strcmp ( c, "*" ) != 0 &&
         strcmp ( c, "/" ) != 0 &&
         strcmp ( c, "%" ) != 0 )
    {
      ungetToken ( c ) ;
      return TRUE ;
    }

    if ( ! pushPrimitive () )
      return FALSE ;

    if ( strcmp ( c, "*" ) == 0 )
      pushMultiply () ;
    else
    if ( strcmp ( c, "/" ) == 0 )
      pushDivide () ;
    else
      pushModulo () ;
  }
}




int pslCompiler::pushAddExpression ()
{
  if ( ! pushMultExpression () )
    return FALSE ;

  while ( TRUE )
  {
    char c [ MAX_TOKEN ] ;

    getToken ( c ) ;

    if ( strcmp ( c, "+" ) != 0 &&
         strcmp ( c, "-" ) != 0 )
    {
      ungetToken ( c ) ;
      return TRUE ;
    }

    if ( ! pushMultExpression () )
      return FALSE ;

    if ( strcmp ( c, "+" ) == 0 )
      pushAdd () ;
    else
      pushSubtract () ;
  }
}




int pslCompiler::pushShiftExpression ()
{
  if ( ! pushAddExpression () )
    return FALSE ;

  while ( TRUE )
  {
    char c [ MAX_TOKEN ] ;

    getToken ( c ) ;

    if ( strcmp ( c, "<<" ) != 0 &&
         strcmp ( c, ">>" ) != 0 )
    {
      ungetToken ( c ) ;
      return TRUE ;
    }

    if ( ! pushAddExpression () )
      return FALSE ;

    if ( strcmp ( c, "<<" ) == 0 )
      pushShiftLeft () ;
    else
      pushShiftRight () ;
  }
}



int pslCompiler::pushBitwiseExpression ()
{
  if ( ! pushShiftExpression () )
    return FALSE ;

  while ( TRUE )
  {
    char c [ MAX_TOKEN ] ;

    getToken ( c ) ;

    if ( strcmp ( c, "|" ) != 0 &&
         strcmp ( c, "&" ) != 0 &&
         strcmp ( c, "^" ) != 0 )
    {
      ungetToken ( c ) ;
      return TRUE ;
    }

    if ( ! pushShiftExpression () )
      return FALSE ;

    if ( strcmp ( c, "|" ) == 0 )
      pushOr () ;
    else
    if ( strcmp ( c, "&" ) == 0 )
      pushAnd () ;
    else
      pushXor () ;
  }
}




int pslCompiler::pushRelExpression ()
{
  if ( ! pushBitwiseExpression () )
    return FALSE ;

  while ( TRUE )
  {
    char c [ MAX_TOKEN ] ;

    getToken ( c ) ;

    if ( strcmp ( c, "<"  ) != 0 &&
         strcmp ( c, ">"  ) != 0 &&
         strcmp ( c, "<=" ) != 0 &&
         strcmp ( c, ">=" ) != 0 &&
         strcmp ( c, "!=" ) != 0 &&
         strcmp ( c, "==" ) != 0 )
    {
      ungetToken ( c ) ;
      return TRUE ;
    }

    if ( ! pushBitwiseExpression () )
      return FALSE ;

    if ( strcmp ( c, "<"  ) == 0 ) pushLess         () ; else
    if ( strcmp ( c, ">"  ) == 0 ) pushGreater      () ; else
    if ( strcmp ( c, "<=" ) == 0 ) pushLessEqual    () ; else
    if ( strcmp ( c, ">=" ) == 0 ) pushGreaterEqual () ; else
    if ( strcmp ( c, "!=" ) == 0 ) pushNotEqual     () ; else
    if ( strcmp ( c, "==" ) == 0 ) pushEqual        () ;
  }
}



int pslCompiler::pushBoolExpression ()
{
  if ( ! pushRelExpression () )
    return FALSE ;

  while ( TRUE )
  {
    char c [ MAX_TOKEN ] ;

    getToken ( c ) ;

    if ( strcmp ( c, "&&"  ) != 0 &&
         strcmp ( c, "||"  ) != 0 )
    {
      ungetToken ( c ) ;
      return TRUE ;
    }

    if ( ! pushRelExpression () )
      return FALSE ;

    if ( strcmp ( c, "&&"  ) == 0 )
      pushAndAnd () ;
    else
      pushOrOr   () ;
  }
}


int pslCompiler::pushExpression ()
{
  return pushBoolExpression () ;
}


