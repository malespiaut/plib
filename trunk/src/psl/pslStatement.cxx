
#include "pslLocal.h"


int PSL_Parser::pushReturnStatement ()
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


int PSL_Parser::pushWhileStatement ()
{
  char lab1 [ 5 ] ;
  char lab2 [ 5 ] ;

  /* Remember place to jump back to */

  sprintf ( lab1, "%d", next_label++ ) ;
  setCodeSymbol ( lab1, next_code ) ;

  if ( ! pushExpression () )
  {
    ulSetError ( UL_WARNING, "PSL: Missing expression for 'while'" ) ;
    return FALSE ;
  }

  sprintf ( lab2, "%d", next_label++ ) ;

  int label_loc = pushJumpIfFalse ( lab2 ) ;

  if ( ! pushStatement () )
  {
    ulSetError ( UL_WARNING, "PSL: Missing statement for 'while'" ) ;
    return FALSE ;
  }

  pushJump ( lab1 ) ;

  setCodeSymbol ( lab2, next_code ) ;

  code [ label_loc   ] = next_code & 0xFF ;
  code [ label_loc+1 ] = ( next_code >> 8 ) & 0xFF ;
  return TRUE ;
}


int PSL_Parser::pushIfStatement ()
{
  char lab1 [ 5 ] ;
  char lab2 [ 5 ] ;

  if ( ! pushExpression () )
  {
    ulSetError ( UL_WARNING, "PSL: Missing expression for 'if'" ) ;
    return FALSE ;
  }

  sprintf ( lab1, "%d", next_label++ ) ;
  sprintf ( lab2, "%d", next_label++ ) ;

  int else_loc = pushJumpIfFalse ( lab1 ) ;

  if ( ! pushStatement () )
  {
    ulSetError ( UL_WARNING, "PSL: Missing statement for 'if'" ) ;
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
    setCodeSymbol ( lab1, next_code ) ;

    code [ else_loc   ] = next_code & 0xFF ;
    code [ else_loc+1 ] = ( next_code >> 8 ) & 0xFF ;

    ungetToken ( c ) ;
    ungetToken ( ";" ) ;
    return TRUE ;
  }

  int label_loc = pushJump ( lab2 ) ;

  setCodeSymbol ( lab1, next_code ) ;

  code [ else_loc   ] = next_code & 0xFF ;
  code [ else_loc+1 ] = ( next_code >> 8 ) & 0xFF ;

  if ( ! pushStatement () )
  {
    ulSetError ( UL_WARNING, "PSL: Missing statement for 'else'" ) ;
    return FALSE ;
  }

  setCodeSymbol ( lab2, next_code ) ;

  code [ label_loc   ] = next_code & 0xFF ;
  code [ label_loc+1 ] = ( next_code >> 8 ) & 0xFF ;

  return TRUE ;
}


int PSL_Parser::pushFunctionCall ( char *var )
{
  char c [ MAX_TOKEN ] ;

  getToken ( c ) ;

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
      ulSetError ( UL_WARNING,
                     "PSL: Missing ')' or ',' in call to '%s'", var ) ;
      exit ( -1 ) ;
    }
  }

  pushCall ( var, argc ) ;
  return TRUE ;
}


int PSL_Parser::pushAssignmentStatement ( char *var )
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


int PSL_Parser::pushCompoundStatement ()
{
  char c [ MAX_TOKEN ] ;

  while ( pushStatement () )
  {
    getToken ( c ) ;

    if ( c[0] != ';' )
      return FALSE ;
  }

  getToken ( c ) ;

  if ( c[0] == '}' )
  {
    ungetToken ( ";" ) ;
    return TRUE ;
  }

  ungetToken ( c ) ;
  return FALSE ;
}


int PSL_Parser::pushStatement ()
{
  char c [ MAX_TOKEN ] ;

  getToken ( c ) ;

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

  ungetToken ( c ) ;
  return FALSE ;
}


void PSL_Parser::pushProgram ()
{
  char c [ MAX_TOKEN ] ;

  /* Have the program call 'main' and then halt */

  pushCodeByte ( OPCODE_CALL ) ;

  int main_fixup = next_code ;

  pushCodeAddr ( 0 ) ;  /* Until we know the address of 'main' */
  pushCodeByte ( 0 ) ;  /* Argc */
  pushCodeByte ( OPCODE_HALT ) ;

  /* Compile the program */

  while ( TRUE )
  {
    getToken ( c ) ;

    if ( c[0] == '\0' )
      break ;

    ungetToken ( c ) ;

    pushFunction () ;
  }

  int main_addr = getCodeSymbol ( "main" ) ;

  code [ main_fixup++ ] =   main_addr        & 0xFF ;
  code [ main_fixup   ] = ( main_addr >> 8 ) & 0xFF ;
}




int PSL_Parser::pushFunction ()
{
  char c  [ MAX_TOKEN ] ;
  char fn [ MAX_TOKEN ] ;

  getToken ( c ) ;

  if ( ! (strcmp ( c, "void"  ) == 0) &&
       ! (strcmp ( c, "float" ) == 0) )
  {
    ulSetError ( UL_WARNING,
           "PSL: Expected a declaration of a variable or function - but got '%s'", c ) ;
    return FALSE ;
  }

  getToken ( fn ) ;

  setCodeSymbol ( fn, next_code ) ;

  getToken ( c ) ;

  if ( c[0] != '(' )
  {
    ulSetError ( UL_WARNING,
                    "PSL: Missing '(' in declaration of '%s'", fn ) ;
    return FALSE ;
  }

  getToken ( c ) ;

  if ( c[0] != ')' )
  { 
    ulSetError ( UL_WARNING,
                    "PSL: Missing ')' in declaration of '%s'", fn ) ;
    return FALSE ;
  }

  getToken ( c ) ;

  if ( c [ 0 ] != '{' )
    ulSetError ( UL_WARNING,
       "PSL: Missing '{' in function '%s'", fn ) ;

  if ( ! pushCompoundStatement () )
    ulSetError ( UL_WARNING,
       "PSL: Missing '}' in function '%s'", fn ) ;

  getToken ( c ) ;

  /* If we fall off the end of the function, we still need a return value */

  pushConstant ( "0.0" ) ;
  pushReturn   () ;
}



