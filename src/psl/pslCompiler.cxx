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
 
  return compile ( fd, fname ) ;
}
 
 
int pslCompiler::compile ( FILE *fd, const char *fname )
{
  init () ;
 
  _pslPushDefaultFile ( fd, (fname == NULL) ? progName : fname ) ;
  pushProgram     () ;
  _pslPopDefaultFile  () ;
 
  if ( num_errors != 0 || num_warnings != 0 )
    fprintf ( stderr, "PSL: '%s' Compiled with %d Warnings, %d Fatal Errors\n",
             progName, num_warnings, num_errors ) ;
 
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



/* Administer the break/continue jump addresses */

void pslCompiler::pushBreakToLabel ()
{
  if ( next_break >= MAX_LABEL-1 )
    error ( "Too many nested 'break' contexts" ) ;
  else
    breakToAddressStack [ next_break++ ] = next_tmp_label++ ;
}

void pslCompiler::pushNoContinue ()
{
  continueToAddressStack [ next_continue++ ] = -1 ;
}

int pslCompiler::pushContinueToLabel ()
{
  if ( next_continue >= MAX_LABEL-1 )
    error ( "Too many nested 'continue' contexts" ) ;
  else
    continueToAddressStack [ next_continue++ ] = next_tmp_label++ ;

  return next_tmp_label-1 ;
}

void pslCompiler::setContinueToLabel ( int which )
{
  char s [ 10 ] ;
  sprintf ( s, "L%d", which ) ;
  setCodeSymbol ( s, next_code ) ;
}

void pslCompiler::popBreakToLabel ()
{
  char s [ 10 ] ;
  sprintf ( s, "L%d", breakToAddressStack[next_break-1] ) ;
  setCodeSymbol ( s, next_code ) ;
  next_break-- ;
}

void pslCompiler::popContinueToLabel ()
{
  next_continue-- ;
}



/* Implement actual break and continue statements. */

int pslCompiler::pushBreakStatement ()
{
  if ( next_break <= 0 )
    return error ( "'break' statement is not inside a 'switch' or a loop." ) ;

  char s [ 10 ] ;
  sprintf ( s, "L%d", breakToAddressStack [ next_break-1 ] ) ;
  pushJump ( getCodeSymbol ( s, next_code+1 ) ) ;
  return TRUE ;
}


int pslCompiler::pushContinueStatement ()
{
  if ( next_break <= 0 )
    return error ( "'continue' statement is not inside a loop." ) ;

  if ( continueToAddressStack [ next_continue-1 ] < 0 )
    return error ( "'continue' statement not allowed inside a 'switch'." ) ;

  char s [ 10 ] ;
  sprintf ( s, "L%d", continueToAddressStack [ next_continue-1 ] ) ;
  pushJump ( getCodeSymbol ( s, next_code+1 ) ) ;
  return TRUE ;
}





int pslCompiler::pushSwitchStatement ()
{
  if ( ! pushExpression () )
    return error ( "Missing control expression for 'switch'" ) ;

  char c [ MAX_TOKEN ] ;

  getToken ( c ) ;    /* Hopefully, the word 'while' */
  
  if ( c [ 0 ] != '{' )
    return error ( "Missing '{' after 'switch'" ) ;

  int jumpToNextCase = pushJump ( 0 ) ;
  int jumpAfterTest  = 0 ;

  pushBreakToLabel () ;
  pushNoContinue   () ;

  while ( TRUE )
  {
    getToken ( c ) ;

    if ( strcmp ( c, "case" ) == 0 )
    {
      jumpAfterTest = pushJump ( 0 ) ;

      code [ jumpToNextCase   ] =   next_code        & 0xFF ;
      code [ jumpToNextCase+1 ] = ( next_code >> 8 ) & 0xFF ;

      pushStackDup () ;

      if ( ! pushExpression () )
        error ( "Missing expression after 'case'." ) ;

      getToken ( c ) ;

      if ( c[0] != ':' )
        error ( "Missing ':' after 'case' expression." ) ;

      pushEqual () ;

      jumpToNextCase = pushJumpIfFalse ( 0 ) ;

      code [ jumpAfterTest   ] = next_code & 0xFF ;
      code [ jumpAfterTest+1 ] = ( next_code >> 8 ) & 0xFF ;
    }
    else
    if ( strcmp ( c, "default" ) == 0 )
    {
      code [ jumpToNextCase   ] =   next_code        & 0xFF ;
      code [ jumpToNextCase+1 ] = ( next_code >> 8 ) & 0xFF ;

      getToken ( c ) ;

      if ( c[0] != ':' )
        error ( "Missing ':' after 'default'." ) ;
    }
    else
    if ( strcmp ( c, "}" ) == 0 )
    {
      ungetToken ( ";" ) ;
      break ;
    }
    else
    {
      ungetToken ( c ) ;

      if ( ! pushStatement () )
        error ( "Missing statement within switch." ) ;

      getToken ( c ) ;

      if ( c [ 0 ] != ';' )
        error ( "Missing semicolon." ) ;
    }
  }

  popBreakToLabel    () ;
  popContinueToLabel () ;
  pushPop () ;
  return TRUE ;
}



int pslCompiler::pushDoWhileStatement ()
{
  /* Remember place to jump back to */

  int start_loc = next_code ;

  pushBreakToLabel    () ;
  setContinueToLabel ( pushContinueToLabel () ) ;

  if ( ! pushStatement () )
    return error ( "Missing statement for 'do/while'" ) ;

  char c [ MAX_TOKEN ] ;

  getToken ( c ) ;    /* The final ';' of the action */

  getToken ( c ) ;    /* Hopefully, the word 'while' */
  
  if ( strcmp ( c, "while" ) != 0 )
    return error ( "Missing 'while' for 'do/while'" ) ;

  if ( ! pushExpression () )
    return error ( "Missing expression for 'while' in a 'do/while'" ) ;

  pushJumpIfTrue ( start_loc ) ;

  popBreakToLabel    () ;
  popContinueToLabel () ;
  return TRUE ;
}


int pslCompiler::pushForStatement ()
{
  char c [ MAX_TOKEN ] ;

  pushBreakToLabel    () ;
  int ct_lab = pushContinueToLabel () ;

  getToken ( c ) ;    /* The initial '(' of the action */

  if ( c [ 0 ] != '(' )
    return error ( "Missing '(' for 'for' loop" ) ;

  if ( ! pushStatement () )
    return error ( "Missing initialiser for 'if'" ) ;

  getToken ( c ) ;    /* The ';' after the initialiser */

  if ( c [ 0 ] != ';' )
    return error ( "Missing ';' after 'for' loop initialisation" ) ;

  /* Remember place to jump back to */

  int start_loc = next_code ;

  /* The test */

  if ( ! pushExpression () )
    return error ( "Missing test for 'for' loop" ) ;

  getToken ( c ) ;    /* The ';' after the initialiser */

  if ( c [ 0 ] != ';' )
    return error ( "Missing ';' after 'for' loop test" ) ;

  char saved [ MAX_UNGET ][ MAX_TOKEN ] ;
  int next_saved    = 0 ;
  int paren_counter = 0 ;

  do
  {
    getToken ( saved [ next_saved ] ) ;

    if ( saved [ next_saved ][ 0 ] == '(' ) paren_counter++ ;
    if ( saved [ next_saved ][ 0 ] == ')' ) paren_counter-- ;

    if ( next_saved >= MAX_UNGET-1 )
      return error ( "Too many tokens in 'increment' part of 'for' loop" ) ;

    next_saved++ ;

  } while ( paren_counter >= 0 ) ;
 
  next_saved-- ;  /* Throw away the ')' */

  int label_loc = pushJumpIfFalse ( 0 ) ;

  if ( ! pushStatement () )
    return error ( "Missing action body for 'for' loop" ) ;
 
  setContinueToLabel ( ct_lab ) ;

  getToken ( c ) ;   /* Throw away the ';' */

  /* Put the increment test back onto the token stream */

  ungetToken ( ";" ) ;    

  for ( int i = next_saved-1 ; i >= 0 ; i-- )
    ungetToken ( saved[i] ) ;    

  if ( ! pushStatement () )
    return error ( "Missing 'increment' part of 'for' loop" ) ;

  pushJump ( start_loc ) ;

  code [ label_loc   ] = next_code & 0xFF ;
  code [ label_loc+1 ] = ( next_code >> 8 ) & 0xFF ;

  popBreakToLabel    () ;
  popContinueToLabel () ;
  return TRUE ;
}


int pslCompiler::pushWhileStatement ()
{
  /* Remember place to jump back to */

  pushBreakToLabel    () ;
  setContinueToLabel ( pushContinueToLabel () ) ;

  int start_loc = next_code ;

  if ( ! pushExpression () )
    return error ( "Missing expression for 'while'" ) ;

  int label_loc = pushJumpIfFalse ( 0 ) ;

  if ( ! pushStatement () )
    return error ( "Missing statement for 'while'" ) ;

  pushJump ( start_loc ) ;

  code [ label_loc   ] = next_code & 0xFF ;
  code [ label_loc+1 ] = ( next_code >> 8 ) & 0xFF ;

  popBreakToLabel    () ;
  popContinueToLabel () ;

  return TRUE ;
}


int pslCompiler::pushIfStatement ()
{
  if ( ! pushExpression () )
    return error ( "Missing expression for 'if'" ) ;

  int else_loc = pushJumpIfFalse ( 0 ) ;

  if ( ! pushStatement () )
    return error ( "Missing statement for 'if'" ) ;

  char c [ MAX_TOKEN ] ;

  getToken ( c ) ;

  if ( c [ 0 ] != ';' )
  {
    ungetToken ( c ) ;
    return error ( "Missing ';' or 'else' after 'if' statement" ) ;
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
    return error ( "Missing statement for 'else'" ) ;

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
    return error ( "Missing '(' in call to '%s'", var ) ;

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

    if ( c[0] != ',' )
      return error ( "Missing ')' or ',' in call to '%s'", var ) ;

    getToken ( c ) ;
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

  return warning ( "Unexpected '%s' after Assignment statement", c ) ;
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
      return warning ( "Unexpected '%s' in Compound statement", c ) ;
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
  return warning ( "Unexpected '%s' in Compound statement", c ) ;
}


int pslCompiler::pushStatement ()
{
  char c [ MAX_TOKEN ] ;

  getToken ( c ) ;

  if ( strcmp ( c, "static"   ) == 0 ) return pushStaticVarDecl      () ;
  if ( strcmp ( c, "string"   ) == 0 ) return pushLocalVarDecl ( PSL_STRING) ;
  if ( strcmp ( c, "int"      ) == 0 ) return pushLocalVarDecl ( PSL_INT   ) ;
  if ( strcmp ( c, "float"    ) == 0 ) return pushLocalVarDecl ( PSL_FLOAT ) ;
  if ( strcmp ( c, "return"   ) == 0 ) return pushReturnStatement    () ;
  if ( strcmp ( c, "break"    ) == 0 ) return pushBreakStatement     () ;
  if ( strcmp ( c, "continue" ) == 0 ) return pushContinueStatement  () ;
  if ( strcmp ( c, "pause"    ) == 0 ) return pushPauseStatement     () ;
  if ( strcmp ( c, "for"      ) == 0 ) return pushForStatement       () ;
  if ( strcmp ( c, "do"       ) == 0 ) return pushDoWhileStatement   () ;
  if ( strcmp ( c, "switch"   ) == 0 ) return pushSwitchStatement    () ;
  if ( strcmp ( c, "while"    ) == 0 ) return pushWhileStatement     () ;
  if ( strcmp ( c, "if"       ) == 0 ) return pushIfStatement        () ;
  if ( isalnum ( c [ 0 ] )           ) return pushAssignmentStatement(c);
  if ( c [ 0 ] == '{'                ) return pushCompoundStatement  () ;

  if ( strcmp ( c, "case"     ) == 0 || strcmp ( c, "default" ) == 0 )
    return error ( "'%s' encountered - not inside 'switch' statement", c ) ;

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

  pushIntConstant ( 0 ) ;  /* No arguments to main *YET*  */

  pushCodeByte ( OPCODE_CALL ) ;
  pushCodeAddr ( getCodeSymbol ( "main", next_code ) ) ;
  pushCodeByte ( 0 ) ;  /* Argc */
  pushCodeByte ( OPCODE_HALT ) ;

  checkUnresolvedSymbols () ;
}



int pslCompiler::pushLocalVarDecl ( pslType t )
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



int pslCompiler::pushStaticVarDecl ()
{
  return error ( "Local Variables are Not Supported Yet." ) ;
}



int pslCompiler::pushGlobalVarDecl ( const char *s, pslType t )
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
    return error ( "Missing ';' after declaration of '%s'", s ) ;

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
          return error ( "Expected declaration - but got '%s'", c ) ;

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
    return pushGlobalVarDecl ( fn, t ) ;
  }

  return error ( "Expected a declaration - but got '%s'", c);
}


int pslCompiler::pushFunctionDeclaration ( const char *fn )
{
  char c  [ MAX_TOKEN ] ;

  pslAddress jump_target = pushJump ( 0 ) ;

  setCodeSymbol ( fn, next_code ) ;

  getToken ( c ) ;

  if ( c[0] != '(' )
    return error ( "Missing '(' in declaration of '%s'", fn ) ;

  pushLocality () ;

  int argpos = 0 ;

  while ( 1 )
  {
    getToken ( c ) ;

    if ( c [ 0 ] == ')' || c [ 0 ] == '\0' )
      break ;

    char s [ MAX_TOKEN ] ;

    getToken ( s ) ;

    pslAddress a = setVarSymbol ( s ) ;

    if ( strcmp ( c, "int" ) == 0 ) makeIntVariable    ( s ) ; else
    if ( strcmp ( c, "float" ) == 0 ) makeFloatVariable  ( s ) ; else
    if ( strcmp ( c, "string" ) == 0 ) makeStringVariable ( s ) ; else
    {
      popLocality () ;
      return error ( "Missing ')' in declaration of '%s'", fn ) ;
    }
 
    pushGetParameter ( a, argpos++ ) ;

    getToken ( c ) ;

    if ( c[0] == ',' )
      continue ;

    if ( c[0] == ')' )
      break ;

    popLocality () ;
    return error ( "Missing ',' or ')' in declaration of '%s'", fn ) ;
  }

  if ( c[0] != ')' )
  {
    popLocality () ;
    return error ( "Missing ')' in declaration of '%s'", fn ) ;
  }

  getToken ( c ) ;

  if ( c [ 0 ] != '{' )
  {
    popLocality () ;
    return error ( "Missing '{' in function '%s'", fn ) ;
  }

  if ( ! pushCompoundStatement () )
  {
    popLocality () ;
    return error ( "Missing '}' in function '%s'", fn ) ;
  }

  getToken ( c ) ;

  /* If we fall off the end of the function, we still need a return value */

  pushConstant ( "0.0" ) ;
  pushReturn   () ;

  code [  jump_target  ] =  next_code       & 0xFF ;
  code [ jump_target+1 ] = (next_code >> 8) & 0xFF ;

  popLocality () ;
  return TRUE ;
}



