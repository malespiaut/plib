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


int pslParser::pushReturnStatement ()
{
  char c [ MAX_TOKEN ] ;

  pslGetToken ( c ) ;

  if ( c [ 0 ] == ';' )   /* Return without data == "return 0" */
  {
    pslUngetToken   ( c ) ;
    pushConstant ( "0.0" ) ;
  }
  else
  {
    pslUngetToken     ( c ) ;
    pushExpression () ;
  }

  pushReturn () ;
  return TRUE ;
}


int pslParser::pushWhileStatement ()
{

  /* Remember place to jump back to */

  int start_loc = next_code ;

  if ( ! pushExpression () )
  {
    ulSetError ( UL_WARNING, "PSL: Missing expression for 'while'" ) ;
    return FALSE ;
  }

  int label_loc = pushJumpIfFalse ( 0 ) ;

  if ( ! pushStatement () )
  {
    ulSetError ( UL_WARNING, "PSL: Missing statement for 'while'" ) ;
    return FALSE ;
  }

  pushJump ( start_loc ) ;

  code [ label_loc   ] = next_code & 0xFF ;
  code [ label_loc+1 ] = ( next_code >> 8 ) & 0xFF ;
  return TRUE ;
}


int pslParser::pushIfStatement ()
{
  if ( ! pushExpression () )
  {
    ulSetError ( UL_WARNING, "PSL: Missing expression for 'if'" ) ;
    return FALSE ;
  }

  int else_loc = pushJumpIfFalse ( 0 ) ;

  if ( ! pushStatement () )
  {
    ulSetError ( UL_WARNING, "PSL: Missing statement for 'if'" ) ;
    return FALSE ;
  }

  char c [ MAX_TOKEN ] ;

  pslGetToken ( c ) ;

  if ( c [ 0 ] != ';' )
  {
    pslUngetToken ( c ) ;
    return FALSE ;
  }

  pslGetToken ( c ) ;

  if ( strcmp ( c, "else" ) != 0 )
  {
    code [ else_loc   ] = next_code & 0xFF ;
    code [ else_loc+1 ] = ( next_code >> 8 ) & 0xFF ;

    pslUngetToken ( c ) ;
    pslUngetToken ( ";" ) ;
    return TRUE ;
  }

  int label_loc = pushJump ( 0 ) ;

  code [ else_loc   ] = next_code & 0xFF ;
  code [ else_loc+1 ] = ( next_code >> 8 ) & 0xFF ;

  if ( ! pushStatement () )
  {
    ulSetError ( UL_WARNING, "PSL: Missing statement for 'else'" ) ;
    return FALSE ;
  }

  code [ label_loc   ] = next_code & 0xFF ;
  code [ label_loc+1 ] = ( next_code >> 8 ) & 0xFF ;

  return TRUE ;
}


int pslParser::pushFunctionCall ( const char *var )
{
  char c [ MAX_TOKEN ] ;

  pslGetToken ( c ) ;

  /*
    'var' should be the name of a function,
    'c'   should be an '('
  */

  if ( c[0] != '(' )
  {
    ulSetError ( UL_WARNING,
                    "PSL: Missing '(' in call to '%s'", var ) ;
    return FALSE ;
  }

  pslGetToken ( c ) ;

  int argc = 0 ;

  while ( c[0] != ')' )
  { 
    pslUngetToken ( c ) ;
    pushExpression () ;
    argc++ ;
    pslGetToken ( c ) ;

    if ( c[0] == ')' )
      break ;

    if ( c[0] == ',' )
      pslGetToken ( c ) ;
    else
    {
      ulSetError ( UL_WARNING,
                     "PSL: Missing ')' or ',' in call to '%s'", var ) ;
      exit ( -1 ) ;
    }
  }

  pushCall ( var, argc ) ;
  return TRUE ;
}


int pslParser::pushAssignmentStatement ( const char *var )
{
  char c [ MAX_TOKEN ] ;

  pslGetToken ( c ) ;

  if ( c [ 0 ] != '=' )
  {
    pslUngetToken ( c ) ;
    pushFunctionCall ( var ) ;
    pushPop () ;
    return TRUE ;
  }

  if ( pushExpression () )
  {
    pushAssignment ( var ) ;
    return TRUE ; 
  }

  return FALSE ;
}


int pslParser::pushCompoundStatement ()
{
  char c [ MAX_TOKEN ] ;

  pushLocality () ;

  while ( pushStatement () )
  {
    pslGetToken ( c ) ;

    if ( c[0] != ';' )
    {
      popLocality () ;
      return FALSE ;
    }
  }

  pslGetToken ( c ) ;

  if ( c[0] == '}' )
  {
    popLocality () ;
    pslUngetToken ( ";" ) ;
    return TRUE ;
  }

  popLocality () ;
  pslUngetToken ( c ) ;
  return FALSE ;
}


int pslParser::pushStatement ()
{
  char c [ MAX_TOKEN ] ;

  pslGetToken ( c ) ;

  if ( strcmp ( c, "static" ) == 0 )
    return pushStaticVariableDeclaration () ;

  if ( strcmp ( c, "float" ) == 0 )
    return pushLocalVariableDeclaration () ;

  if ( strcmp ( c, "return" ) == 0 )
    return pushReturnStatement () ;

  if ( strcmp ( c, "pause" ) == 0 )
    return pushPauseStatement () ;

  if ( strcmp ( c, "while" ) == 0 )
    return pushWhileStatement () ;

  if ( strcmp ( c, "if" ) == 0 )
    return pushIfStatement () ;

  if ( isalnum ( c [ 0 ] ) )
    return pushAssignmentStatement ( c ) ;

  if ( c [ 0 ] == '{' )
    return pushCompoundStatement () ;

  pslUngetToken ( c ) ;
  return FALSE ;
}


void pslParser::pushProgram ()
{
  char c [ MAX_TOKEN ] ;

  /* Compile the program */

  while ( TRUE )
  {
    pslGetToken ( c ) ;

    if ( c[0] == '\0' )
      break ;

    pslUngetToken ( c ) ;

    pushGlobalDeclaration () ;
  }

  /* Have the program call 'main' and then halt */

  pushCodeByte ( OPCODE_CALL ) ;
  pushCodeAddr ( getCodeSymbol ( "main", next_code ) ) ;
  pushCodeByte ( 0 ) ;  /* Argc */
  pushCodeByte ( OPCODE_HALT ) ;

  checkUnresolvedSymbols () ;
}



int pslParser::pushLocalVariableDeclaration ()
{
  char c  [ MAX_TOKEN ] ;
  char s  [ MAX_TOKEN ] ;

  pslGetToken ( s ) ;

  setVarSymbol ( s ) ;

  pslGetToken ( c ) ;

  if ( c[0] == '=' )
  {
    pslUngetToken ( c ) ;
    pushAssignmentStatement ( s ) ;
    return TRUE ;
  }
 
  pslUngetToken ( c ) ;
  return TRUE ;
}



int pslParser::pushStaticVariableDeclaration ()
{
  ulSetError ( UL_WARNING,
       "PSL: Local Variables are Not Supported Yet." ) ;
  return FALSE ;
}



int pslParser::pushGlobalVariableDeclaration ( const char *s )
{
  char c  [ MAX_TOKEN ] ;

  setVarSymbol ( s ) ;

  pslGetToken ( c ) ;

  if ( c[0] == '=' )
  {
    pslUngetToken ( c ) ;
    pushAssignmentStatement ( s ) ;
    pslGetToken ( c ) ;
  }
 
  if ( c[0] != ';' )
  {
    ulSetError ( UL_WARNING,
         "PSL: Missing ';' after declaration of '%s'", s ) ;
    return FALSE ;
  }

  return TRUE ;
}



int pslParser::pushGlobalDeclaration ()
{
  char c  [ MAX_TOKEN ] ;
  char fn [ MAX_TOKEN ] ;

  pslGetToken ( c ) ;

  if ( strcmp ( c, "static" ) == 0 ||
       strcmp ( c, "extern" ) == 0 )
  {
    /* Something complicated should probably happen here! */
    pslGetToken ( c ) ;
  }

  if ( ! (strcmp ( c, "void"  ) == 0) &&
       ! (strcmp ( c, "float" ) == 0) )
  {
    ulSetError ( UL_WARNING,
           "PSL: Expected a declaration of a variable or function - but got '%s'", c ) ;
    return FALSE ;
  }

  pslGetToken ( fn ) ;

  pslGetToken ( c ) ;

  if ( c[0] == '(' )
  {
    pslUngetToken ( c ) ;
    return pushFunctionDeclaration ( fn ) ;
  }

  if ( c[0] == '=' || c[0] == ';' )
  {
    pslUngetToken ( c ) ;
    return pushGlobalVariableDeclaration ( fn ) ;
  }

  ulSetError ( UL_WARNING,
     "PSL: Expected a declaration of a variable or function - but got '%s'", c);
  return FALSE ;
}


int pslParser::pushFunctionDeclaration ( const char *fn )
{
  char c  [ MAX_TOKEN ] ;

  pslAddress jump_target = pushJump ( 0 ) ;

  setCodeSymbol ( fn, next_code ) ;

  pslGetToken ( c ) ;

  if ( c[0] != '(' )
  {
    ulSetError ( UL_WARNING,
                    "PSL: Missing '(' in declaration of '%s'", fn ) ;
    return FALSE ;
  }

  pslGetToken ( c ) ;

  if ( c[0] != ')' )
  { 
    ulSetError ( UL_WARNING,
                    "PSL: Missing ')' in declaration of '%s'", fn ) ;
    return FALSE ;
  }

  pslGetToken ( c ) ;

  if ( c [ 0 ] != '{' )
    ulSetError ( UL_WARNING,
       "PSL: Missing '{' in function '%s'", fn ) ;

  if ( ! pushCompoundStatement () )
    ulSetError ( UL_WARNING,
       "PSL: Missing '}' in function '%s'", fn ) ;

  pslGetToken ( c ) ;

  /* If we fall off the end of the function, we still need a return value */

  pushConstant ( "0.0" ) ;
  pushReturn   () ;

  code [  jump_target  ] =  next_code       & 0xFF ;
  code [ jump_target+1 ] = (next_code >> 8) & 0xFF ;

  return TRUE ;
}



