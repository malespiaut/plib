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


void PSL_Parser::pushConstant ( const char *c )
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

int PSL_Parser::pushJumpIfFalse  ( int l )
{
  pushCodeByte ( OPCODE_JUMP_FALSE ) ;

  int res = next_code ;

  pushCodeAddr ( l ) ;

  return res ;
}

int PSL_Parser::pushJump ( int l )
{
  pushCodeByte ( OPCODE_JUMP ) ;

  int res = next_code ;

  pushCodeAddr ( l ) ;

  return res ;
}


int PSL_Parser::pushPauseStatement()
{ 
  pushCodeByte ( OPCODE_PAUSE ) ;
  return TRUE ;
} 


