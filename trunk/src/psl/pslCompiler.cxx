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


 
int pslCompiler::compile ( const char *fname )
{
  FILE *fd = fopen ( fname, "ra" ) ;
 
  if ( fd == NULL )
  {
    perror ( "PSL:" ) ;
    ulSetError ( UL_WARNING, "PSL: Failed while opening '%s' for reading.",
                                                                  fname );
    return FALSE ;
  }
 
  int res = compile ( fd ) ;
  fclose ( fd ) ;
  return res ;
}
 
 
int pslCompiler::compile ( FILE *fd )
{
  init () ;
 
  setDefaultFile ( fd ) ;
  pushProgram () ;
 
  if ( num_errors != 0 || num_warnings != 0 )
    fprintf ( stderr, "PSL: Compiled with %d Warnings, %d Fatal Errors\n",
             num_warnings, num_errors ) ;
 
  /* If there are errors, prevent the program from running. */

  if ( num_errors != 0 )
  {
    next_code = 0 ;
    pushCodeByte ( OPCODE_HALT ) ;
  }

  return num_errors ;
}
                                                                                


int pslCompiler::pushReturnStatement ()
{
  char c [ MAX_TOKEN ] ;

  getToken ( c ) ;

  if ( c [ 0 ] == ';' )   /* Return without data == "return 0" */
  {
    ungetToken   ( c ) ;
    pushConstant ( "0.0" ) ;
  }
  else
  {
    ungetToken     ( c ) ;
    pushExpression () ;
  }

  pushReturn () ;
  return TRUE ;
}


int pslCompiler::pushDoWhileStatement ()
{
  /* Remember place to jump back to */

  int start_loc = next_code ;

  if ( ! pushStatement () )
  {
    error ( "Missing statement for 'do/while'" ) ;
    return FALSE ;
  }

  char c [ MAX_TOKEN ] ;

  getToken ( c ) ;    /* The final ';' of the action */

  getToken ( c ) ;    /* Hopefully, the word 'while' */
  
  if ( strcmp ( c, "while" ) != 0 )
  {
    error ( "Missing 'while' for 'do/while'" ) ;
    return FALSE ;
  }

  if ( ! pushExpression () )
  {
    error ( "Missing expression for 'while' in a 'do/while'" ) ;
    return FALSE ;
  }

  pushJumpIfTrue ( start_loc ) ;
  return TRUE ;
}


int pslCompiler::pushForStatement ()
{
  char c [ MAX_TOKEN ] ;

  getToken ( c ) ;    /* The initial '(' of the action */

  if ( c [ 0 ] != '(' )
  {
    error ( "Missing '(' for 'for' loop" ) ;
    return FALSE ;
  }

  if ( ! pushStatement () )
  {
    error ( "Missing initialiser for 'if'" ) ;
    return FALSE ;
  }

  getToken ( c ) ;    /* The ';' after the initialiser */

  if ( c [ 0 ] != ';' )
  {
    error ( "Missing ';' after 'for' loop initialisation" ) ;
    return FALSE ;
  }

  /* Remember place to jump back to */

  int start_loc = next_code ;

  /* The test */

  if ( ! pushExpression () )
  {
    error ( "Missing test for 'for' loop" ) ;
    return FALSE ;
  }

  getToken ( c ) ;    /* The ';' after the initialiser */

  if ( c [ 0 ] != ';' )
  {
    error ( "Missing ';' after 'for' loop test" ) ;
    return FALSE ;
  }

  char saved [ MAX_UNGET ][ MAX_TOKEN ] ;
  int next_saved    = 0 ;
  int paren_counter = 0 ;

  do
  {
    getToken ( saved [ next_saved ] ) ;

    if ( saved [ next_saved ][ 0 ] == '(' ) paren_counter++ ;
    if ( saved [ next_saved ][ 0 ] == ')' ) paren_counter-- ;

    if ( next_saved >= MAX_UNGET-1 )
    {
      error ( "Too many tokens in 'increment' part of 'for' loop" ) ;
      return FALSE ;
    }

    next_saved++ ;

  } while ( paren_counter >= 0 ) ;
 
  next_saved-- ;  /* Throw away the ')' */

  int label_loc = pushJumpIfFalse ( 0 ) ;

  if ( ! pushStatement () )
  {
    error ( "Missing action body for 'for' loop" ) ;
    return FALSE ;
  }
 
  getToken ( c ) ;   /* Throw away the ';' */

  /* Put the increment test back onto the token stream */

  ungetToken ( ";" ) ;    

  for ( int i = next_saved-1 ; i >= 0 ; i-- )
    ungetToken ( saved[i] ) ;    

  if ( ! pushStatement () )
  {
    error ( "Missing 'increment' part of 'for' loop" ) ;
    return FALSE ;
  }

  pushJump ( start_loc ) ;

  code [ label_loc   ] = next_code & 0xFF ;
  code [ label_loc+1 ] = ( next_code >> 8 ) & 0xFF ;
  return TRUE ;
}


int pslCompiler::pushWhileStatement ()
{
  /* Remember place to jump back to */

  int start_loc = next_code ;

  if ( ! pushExpression () )
  {
    error ( "Missing expression for 'while'" ) ;
    return FALSE ;
  }

  int label_loc = pushJumpIfFalse ( 0 ) ;

  if ( ! pushStatement () )
  {
    error ( "Missing statement for 'while'" ) ;
    return FALSE ;
  }

  pushJump ( start_loc ) ;

  code [ label_loc   ] = next_code & 0xFF ;
  code [ label_loc+1 ] = ( next_code >> 8 ) & 0xFF ;
  return TRUE ;
}


int pslCompiler::pushIfStatement ()
{
  if ( ! pushExpression () )
  {
    error ( "Missing expression for 'if'" ) ;
    return FALSE ;
  }

  int else_loc = pushJumpIfFalse ( 0 ) ;

  if ( ! pushStatement () )
  {
    error ( "Missing statement for 'if'" ) ;
    return FALSE ;
  }

  char c [ MAX_TOKEN ] ;

  getToken ( c ) ;

  if ( c [ 0 ] != ';' )
  {
    ungetToken ( c ) ;
    return FALSE ;
  }

  getToken ( c ) ;

  if ( strcmp ( c, "else" ) != 0 )
  {
    code [ else_loc   ] = next_code & 0xFF ;
    code [ else_loc+1 ] = ( next_code >> 8 ) & 0xFF ;

    ungetToken ( c ) ;
    ungetToken ( ";" ) ;
    return TRUE ;
  }

  int label_loc = pushJump ( 0 ) ;

  code [ else_loc   ] = next_code & 0xFF ;
  code [ else_loc+1 ] = ( next_code >> 8 ) & 0xFF ;

  if ( ! pushStatement () )
  {
    error ( "Missing statement for 'else'" ) ;
    return FALSE ;
  }

  code [ label_loc   ] = next_code & 0xFF ;
  code [ label_loc+1 ] = ( next_code >> 8 ) & 0xFF ;

  return TRUE ;
}


int pslCompiler::pushFunctionCall ( const char *var )
{
  char c [ MAX_TOKEN ] ;

  getToken ( c ) ;

  /*
    'var' should be the name of a function,
    'c'   should be an '('
  */

  if ( c[0] != '(' )
  {
    error ( "Missing '(' in call to '%s'", var ) ;
    return FALSE ;
  }

  getToken ( c ) ;

  int argc = 0 ;

  while ( c[0] != ')' )
  { 
    ungetToken ( c ) ;
    pushExpression () ;
    argc++ ;
    getToken ( c ) ;

    if ( c[0] == ')' )
      break ;

    if ( c[0] == ',' )
      getToken ( c ) ;
    else
    {
      error ( "Missing ')' or ',' in call to '%s'", var ) ;
      exit ( -1 ) ;
    }
  }

  pushCall ( var, argc ) ;
  return TRUE ;
}


int pslCompiler::pushAssignmentStatement ( const char *var )
{
  char c [ MAX_TOKEN ] ;

  getToken ( c ) ;

  if ( c [ 0 ] != '=' )
  {
    ungetToken ( c ) ;
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


int pslCompiler::pushCompoundStatement ()
{
  char c [ MAX_TOKEN ] ;

  pushLocality () ;

  while ( pushStatement () )
  {
    getToken ( c ) ;

    if ( c[0] != ';' )
    {
      popLocality () ;
      return FALSE ;
    }
  }

  getToken ( c ) ;

  if ( c[0] == '}' )
  {
    popLocality () ;
    ungetToken ( ";" ) ;
    return TRUE ;
  }

  popLocality () ;
  ungetToken ( c ) ;
  return FALSE ;
}


int pslCompiler::pushStatement ()
{
  char c [ MAX_TOKEN ] ;

  getToken ( c ) ;

  if ( strcmp ( c, "static" ) == 0 )
    return pushStaticVariableDeclaration () ;

  if ( strcmp ( c, "string" ) == 0 )
    return pushLocalVariableDeclaration ( PSL_STRING ) ;

  if ( strcmp ( c, "int" ) == 0 )
    return pushLocalVariableDeclaration ( PSL_INT ) ;

  if ( strcmp ( c, "float" ) == 0 )
    return pushLocalVariableDeclaration ( PSL_FLOAT ) ;

  if ( strcmp ( c, "return" ) == 0 )
    return pushReturnStatement () ;

  if ( strcmp ( c, "pause" ) == 0 )
    return pushPauseStatement () ;

  if ( strcmp ( c, "for" ) == 0 )
    return pushForStatement () ;

  if ( strcmp ( c, "do" ) == 0 )
    return pushDoWhileStatement () ;

  if ( strcmp ( c, "while" ) == 0 )
    return pushWhileStatement () ;

  if ( strcmp ( c, "if" ) == 0 )
    return pushIfStatement () ;

  if ( isalnum ( c [ 0 ] ) )
    return pushAssignmentStatement ( c ) ;

  if ( c [ 0 ] == '{' )
    return pushCompoundStatement () ;

  ungetToken ( c ) ;
  return FALSE ;
}


void pslCompiler::pushProgram ()
{
  char c [ MAX_TOKEN ] ;

  /* Compile the program */

  while ( TRUE )
  {
    getToken ( c ) ;

    if ( c[0] == '\0' )
      break ;

    ungetToken ( c ) ;

    pushGlobalDeclaration () ;
  }

  /* Have the program call 'main' and then halt */

  pushCodeByte ( OPCODE_CALL ) ;
  pushCodeAddr ( getCodeSymbol ( "main", next_code ) ) ;
  pushCodeByte ( 0 ) ;  /* Argc */
  pushCodeByte ( OPCODE_HALT ) ;

  checkUnresolvedSymbols () ;
}



int pslCompiler::pushLocalVariableDeclaration ( pslType t )
{
  char c  [ MAX_TOKEN ] ;
  char s  [ MAX_TOKEN ] ;

  getToken ( s ) ;

  setVarSymbol ( s ) ;

  switch ( t )
  {
    case PSL_VOID   :
    case PSL_INT    : makeIntVariable    ( s ) ; break ;
    case PSL_FLOAT  : makeFloatVariable  ( s ) ; break ;
    case PSL_STRING : makeStringVariable ( s ) ; break ;
  }
 
  getToken ( c ) ;

  if ( c[0] == '=' )
  {
    ungetToken ( c ) ;
    pushAssignmentStatement ( s ) ;
    return TRUE ;
  }
 
  ungetToken ( c ) ;
  return TRUE ;
}



int pslCompiler::pushStaticVariableDeclaration ()
{
  error ( "Local Variables are Not Supported Yet." ) ;
  return FALSE ;
}



int pslCompiler::pushGlobalVariableDeclaration ( const char *s, pslType t )
{
  char c  [ MAX_TOKEN ] ;

  setVarSymbol ( s ) ;

  switch ( t )
  {
    case PSL_VOID   :
    case PSL_INT    : makeIntVariable    ( s ) ; break ;
    case PSL_FLOAT  : makeFloatVariable  ( s ) ; break ;
    case PSL_STRING : makeStringVariable ( s ) ; break ;
  }
 

  getToken ( c ) ;

  if ( c[0] == '=' )
  {
    ungetToken ( c ) ;
    pushAssignmentStatement ( s ) ;
    getToken ( c ) ;
  }
 
  if ( c[0] != ';' )
  {
    error ( "Missing ';' after declaration of '%s'", s ) ;
    return FALSE ;
  }

  return TRUE ;
}



int pslCompiler::pushGlobalDeclaration ()
{
  char c  [ MAX_TOKEN ] ;
  char fn [ MAX_TOKEN ] ;

  getToken ( c ) ;

  if ( strcmp ( c, "static" ) == 0 ||
       strcmp ( c, "extern" ) == 0 )
  {
    /* Something complicated should probably happen here! */
    getToken ( c ) ;
  }

  pslType t ;

  if ( strcmp ( c, "void"   ) == 0 ) t = PSL_VOID   ; else
    if ( strcmp ( c, "int"    ) == 0 ) t = PSL_INT    ; else
      if ( strcmp ( c, "float"  ) == 0 ) t = PSL_FLOAT  ; else
        if ( strcmp ( c, "string" ) == 0 ) t = PSL_STRING ; else
        {
          error ( "Expected declaration - but got '%s'", c ) ;
          return FALSE ;
        }

  getToken ( fn ) ;

  getToken ( c ) ;

  if ( c[0] == '(' )
  {
    ungetToken ( c ) ;
    return pushFunctionDeclaration ( fn ) ;
  }

  if ( c[0] == '=' || c[0] == ';' )
  {
    ungetToken ( c ) ;
    return pushGlobalVariableDeclaration ( fn, t ) ;
  }

  error ( "Expected a declaration - but got '%s'", c);
  return FALSE ;
}


int pslCompiler::pushFunctionDeclaration ( const char *fn )
{
  char c  [ MAX_TOKEN ] ;

  pslAddress jump_target = pushJump ( 0 ) ;

  setCodeSymbol ( fn, next_code ) ;

  getToken ( c ) ;

  if ( c[0] != '(' )
  {
    error ( "Missing '(' in declaration of '%s'", fn ) ;
    return FALSE ;
  }

  getToken ( c ) ;

  if ( c[0] != ')' )
  { 
    error ( "Missing ')' in declaration of '%s'", fn ) ;
    return FALSE ;
  }

  getToken ( c ) ;

  if ( c [ 0 ] != '{' )
    error ( "Missing '{' in function '%s'", fn ) ;

  if ( ! pushCompoundStatement () )
    error ( "Missing '}' in function '%s'", fn ) ;

  getToken ( c ) ;

  /* If we fall off the end of the function, we still need a return value */

  pushConstant ( "0.0" ) ;
  pushReturn   () ;

  code [  jump_target  ] =  next_code       & 0xFF ;
  code [ jump_target+1 ] = (next_code >> 8) & 0xFF ;

  return TRUE ;
}



