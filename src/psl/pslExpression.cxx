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
  pslGetToken ( c ) ;

  if ( c [ 0 ] == '(' )
  {
    if ( ! pushExpression () )
    {
      ulSetError ( UL_WARNING, "PSL: Missing expression after '('" ) ;
      pslUngetToken ( c ) ;
      return FALSE ;
    }

    pslGetToken ( c ) ;

    if ( c [ 0 ] != ')' )
    {
      ulSetError ( UL_WARNING, "PSL: Missing ')' (found '%s')", c );
      pslUngetToken ( c ) ;
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
      pslUngetToken ( c ) ;
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
      pslUngetToken ( c ) ;
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
    pslGetToken ( n ) ;
    pslUngetToken ( n ) ;

    if ( n[0] == '(' )
      pushFunctionCall ( c ) ;
    else
      pushVariable ( c ) ;

    return TRUE ;
  }

  pslUngetToken ( c ) ;
  return FALSE ;
}



int pslCompiler::pushMultExpression ()
{
  if ( ! pushPrimitive () )
    return FALSE ;

  while ( TRUE )
  {
    char c [ MAX_TOKEN ] ;

    pslGetToken ( c ) ;

    if ( c [ 0 ] != '*' && c [ 0 ] != '/' )
    {
      pslUngetToken ( c ) ;
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




int pslCompiler::pushAddExpression ()
{
  if ( ! pushMultExpression () )
    return FALSE ;

  while ( TRUE )
  {
    char c [ MAX_TOKEN ] ;

    pslGetToken ( c ) ;

    if ( c [ 0 ] != '+' && c [ 0 ] != '-' )
    {
      pslUngetToken ( c ) ;
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




int pslCompiler::pushRelExpression ()
{
  if ( ! pushAddExpression () )
    return FALSE ;

  while ( TRUE )
  {
    char c [ MAX_TOKEN ] ;

    pslGetToken ( c ) ;

    if ( c [ 0 ] != '<' &&
         c [ 0 ] != '>' &&
         c [ 0 ] != '!' &&
         c [ 0 ] != '=' )
    {
      pslUngetToken ( c ) ;
      return TRUE ;
    }

    char c2 [ MAX_TOKEN ] ;

    pslGetToken ( c2 ) ;

    if ( c2 [ 0 ] == '=' )
    {
      c[1] = '=' ;
      c[2] = '\0' ;
    }
    else
      pslUngetToken ( c2 ) ;

    if (( c [ 0 ] == '!' || c [ 0 ] == '=' ) && c [ 1 ] != '=' )
    {
      pslUngetToken ( c2 ) ;
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


int pslCompiler::pushExpression ()
{
  return pushRelExpression () ;
}


