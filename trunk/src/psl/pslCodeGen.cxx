
#include "pslLocal.h"

int PSL_Parser::parse ( char *fname )
{
  init () ;

  FILE *fd = fopen ( fname, "ra" ) ;

  if ( fd == NULL )
  {
    perror ( "PSL:" ) ;
    ulSetError ( UL_WARNING, "PSL: Failed while opening '%s' for reading.",
                                                                  fname );
    return FALSE ;
  }

  parse  ( fd ) ;
  fclose ( fd ) ;
  return TRUE ;
}


int PSL_Parser::parse ( FILE *fd )
{
  setDefaultFile ( fd ) ;
  pushProgram () ;
  return TRUE ;
}


void PSL_Parser::pushCodeByte ( PSL_Opcode op )
{
  code [ next_code++ ] = op ;
}


void PSL_Parser::pushCodeAddr ( PSL_Address a )
{
  pushCodeByte ( a & 0xFF ) ;
  pushCodeByte ( ( a >> 8 ) & 0xFF ) ;
}


void PSL_Parser::pushConstant ( char *c )
{
  float f = atof ( c ) ; 
  char *ff = (char *) & f ;

  pushCodeByte ( OPCODE_PUSH_CONSTANT ) ;
  pushCodeByte ( ff [ 0 ] ) ;
  pushCodeByte ( ff [ 1 ] ) ;
  pushCodeByte ( ff [ 2 ] ) ;
  pushCodeByte ( ff [ 3 ] ) ;
}

void PSL_Parser::pushVariable ( char *c )
{
  int a = getVarSymbol ( c ) ;

  pushCodeByte ( OPCODE_PUSH_VARIABLE | a ) ;
} 

void PSL_Parser::pushAssignment ( char *c )
{
  int a = getVarSymbol ( c ) ;

  pushCodeByte ( OPCODE_POP_VARIABLE | a ) ;
} 


void PSL_Parser::pushCall ( char *c, int argc )
{
  int ext = getExtensionSymbol ( c ) ;

  if ( ext < 0 )
  {
    int a = getCodeSymbol ( c ) ;

    pushCodeByte ( OPCODE_CALL ) ;
    pushCodeAddr ( a ) ;
    pushCodeByte ( argc ) ;
  }
  else
  {
    pushCodeByte ( OPCODE_CALLEXT ) ;
    pushCodeByte ( ext ) ;
    pushCodeByte ( argc ) ;
  }
} 


void PSL_Parser::pushReturn       () { pushCodeByte ( OPCODE_RETURN) ; } 
void PSL_Parser::pushPop          () { pushCodeByte ( OPCODE_POP   ) ; } 
void PSL_Parser::pushSubtract     () { pushCodeByte ( OPCODE_SUB   ) ; } 
void PSL_Parser::pushAdd          () { pushCodeByte ( OPCODE_ADD   ) ; } 
void PSL_Parser::pushDivide       () { pushCodeByte ( OPCODE_DIV   ) ; } 
void PSL_Parser::pushMultiply     () { pushCodeByte ( OPCODE_MULT  ) ; } 
void PSL_Parser::pushNegate       () { pushCodeByte ( OPCODE_NEG   ) ; } 

void PSL_Parser::pushLess         () { pushCodeByte ( OPCODE_LESS ) ; } 
void PSL_Parser::pushLessEqual    () { pushCodeByte ( OPCODE_LESSEQUAL ) ; } 
void PSL_Parser::pushGreater      () { pushCodeByte ( OPCODE_GREATER ) ; } 
void PSL_Parser::pushGreaterEqual () { pushCodeByte ( OPCODE_GREATEREQUAL ) ; } 
void PSL_Parser::pushNotEqual     () { pushCodeByte ( OPCODE_NOTEQUAL ) ; } 
void PSL_Parser::pushEqual        () { pushCodeByte ( OPCODE_EQUAL ) ; } 

int PSL_Parser::pushJumpIfFalse  ( char *c )
{
  int a = getCodeSymbol ( c ) ;

  pushCodeByte ( OPCODE_JUMP_FALSE ) ;

  int res = next_code ;

  pushCodeAddr ( a ) ;

  return res ;
}

int PSL_Parser::pushJump ( char *c )
{
  int a = getCodeSymbol ( c ) ;

  pushCodeByte ( OPCODE_JUMP ) ;

  int res = next_code ;

  pushCodeAddr ( a ) ;

  return res ;
}


int PSL_Parser::pushPauseStatement()
{ 
  pushCodeByte ( OPCODE_PAUSE ) ;
  return TRUE ;
} 


