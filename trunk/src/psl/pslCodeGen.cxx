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

int pslParser::parse ( const char *fname )
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


int pslParser::parse ( FILE *fd )
{
  pslSetDefaultFile ( fd ) ;
  pushProgram () ;
  return TRUE ;
}


void pslParser::pushCodeByte ( pslOpcode op )
{
  code [ next_code++ ] = op ;
}


void pslParser::pushCodeAddr ( pslAddress a )
{
  pushCodeByte ( a & 0xFF ) ;
  pushCodeByte ( ( a >> 8 ) & 0xFF ) ;
}


void pslParser::pushConstant ( const char *c )
{
  float f = atof ( c ) ; 
  char *ff = (char *) & f ;

  pushCodeByte ( OPCODE_PUSH_CONSTANT ) ;
  pushCodeByte ( ff [ 0 ] ) ;
  pushCodeByte ( ff [ 1 ] ) ;
  pushCodeByte ( ff [ 2 ] ) ;
  pushCodeByte ( ff [ 3 ] ) ;
}

void pslParser::pushVariable ( const char *c )
{
  int a = getVarSymbol ( c ) ;

  pushCodeByte ( OPCODE_PUSH_VARIABLE | a ) ;
} 

void pslParser::pushAssignment ( const char *c )
{
  int a = getVarSymbol ( c ) ;

  pushCodeByte ( OPCODE_POP_VARIABLE | a ) ;
} 


void pslParser::pushCall ( const char *c, int argc )
{
  int ext = getExtensionSymbol ( c ) ;

  if ( ext < 0 )
  {
    pushCodeByte ( OPCODE_CALL ) ;

    int a = getCodeSymbol ( c, next_code ) ;

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


void pslParser::pushReturn       () { pushCodeByte ( OPCODE_RETURN) ; } 
void pslParser::pushPop          () { pushCodeByte ( OPCODE_POP   ) ; } 
void pslParser::pushSubtract     () { pushCodeByte ( OPCODE_SUB   ) ; } 
void pslParser::pushAdd          () { pushCodeByte ( OPCODE_ADD   ) ; } 
void pslParser::pushDivide       () { pushCodeByte ( OPCODE_DIV   ) ; } 
void pslParser::pushMultiply     () { pushCodeByte ( OPCODE_MULT  ) ; } 
void pslParser::pushNegate       () { pushCodeByte ( OPCODE_NEG   ) ; } 

void pslParser::pushLess         () { pushCodeByte ( OPCODE_LESS ) ; } 
void pslParser::pushLessEqual    () { pushCodeByte ( OPCODE_LESSEQUAL ) ; } 
void pslParser::pushGreater      () { pushCodeByte ( OPCODE_GREATER ) ; } 
void pslParser::pushGreaterEqual () { pushCodeByte ( OPCODE_GREATEREQUAL ) ; } 
void pslParser::pushNotEqual     () { pushCodeByte ( OPCODE_NOTEQUAL ) ; } 
void pslParser::pushEqual        () { pushCodeByte ( OPCODE_EQUAL ) ; } 

int pslParser::pushJumpIfFalse  ( int l )
{
  pushCodeByte ( OPCODE_JUMP_FALSE ) ;

  int res = next_code ;

  pushCodeAddr ( l ) ;

  return res ;
}

int pslParser::pushJump ( int l )
{
  pushCodeByte ( OPCODE_JUMP ) ;

  int res = next_code ;

  pushCodeAddr ( l ) ;

  return res ;
}


int pslParser::pushPauseStatement()
{ 
  pushCodeByte ( OPCODE_PAUSE ) ;
  return TRUE ;
} 


