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


int PSL_Parser::pushPrimitive ()
{
  char c [ MAX_TOKEN ] ;
  PSL_GetToken ( c ) ;

  if ( c [ 0 ] == '(' )
  {
    if ( ! pushExpression () )
    {
      ulSetError ( UL_WARNING, "PSL: Missing expression after '('" ) ;
      PSL_UngetToken ( c ) ;
      return FALSE ;
    }

    PSL_GetToken ( c ) ;

    if ( c [ 0 ] != ')' )
    {
      ulSetError ( UL_WARNING, "PSL: Missing ')' (found '%s')", c );
      PSL_UngetToken ( c ) ;
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
      PSL_UngetToken ( c ) ;
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
      PSL_UngetToken ( c ) ;
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
    PSL_GetToken ( n ) ;
    PSL_UngetToken ( n ) ;

    if ( n[0] == '(' )
      pushFunctionCall ( c ) ;
    else
      pushVariable ( c ) ;

    return TRUE ;
  }

  PSL_UngetToken ( c ) ;
  return FALSE ;
}



int PSL_Parser::pushMultExpression ()
{
  if ( ! pushPrimitive () )
    return FALSE ;

  while ( TRUE )
  {
    char c [ MAX_TOKEN ] ;

    PSL_GetToken ( c ) ;

    if ( c [ 0 ] != '*' && c [ 0 ] != '/' )
    {
      PSL_UngetToken ( c ) ;
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

    PSL_GetToken ( c ) ;

    if ( c [ 0 ] != '+' && c [ 0 ] != '-' )
    {
      PSL_UngetToken ( c ) ;
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

    PSL_GetToken ( c ) ;

    if ( c [ 0 ] != '<' &&
         c [ 0 ] != '>' &&
         c [ 0 ] != '!' &&
         c [ 0 ] != '=' )
    {
      PSL_UngetToken ( c ) ;
      return TRUE ;
    }

    char c2 [ MAX_TOKEN ] ;

    PSL_GetToken ( c2 ) ;

    if ( c2 [ 0 ] == '=' )
    {
      c[1] = '=' ;
      c[2] = '\0' ;
    }
    else
      PSL_UngetToken ( c2 ) ;

    if (( c [ 0 ] == '!' || c [ 0 ] == '=' ) && c [ 1 ] != '=' )
    {
      PSL_UngetToken ( c2 ) ;
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


