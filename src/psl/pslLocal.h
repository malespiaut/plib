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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "psl.h"
#include "ul.h"

#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif

/* Limits */

#define MAX_ARGS     16
#define MAX_VARIABLE 16
#define MAX_LABEL    16
#define MAX_TOKEN   256 
#define MAX_CODE    512
#define MAX_STACK    32 
#define MAX_SYMBOL  (MAX_VARIABLE + MAX_LABEL)

/* Code Opcodes */

/* Low nybble is variable address */

#define OPCODE_PUSH_VARIABLE   0xE0
#define OPCODE_POP_VARIABLE    0xF0

/* Remaining opcodes must be in range 0x00 to 0xDF */

#define OPCODE_PUSH_CONSTANT   0x00
#define OPCODE_CALL            0x01
#define OPCODE_SUB             0x02
#define OPCODE_ADD             0x03
#define OPCODE_DIV             0x04
#define OPCODE_MULT            0x05
#define OPCODE_NEG             0x06
#define OPCODE_LESS            0x07
#define OPCODE_LESSEQUAL       0x08
#define OPCODE_GREATER         0x09
#define OPCODE_GREATEREQUAL    0x0A
#define OPCODE_NOTEQUAL        0x0B
#define OPCODE_EQUAL           0x0C
#define OPCODE_JUMP_FALSE      0x0D
#define OPCODE_JUMP            0x0E
#define OPCODE_POP             0x0F
#define OPCODE_HALT            0x10
#define OPCODE_CALLEXT         0x11
#define OPCODE_PAUSE           0x12
#define OPCODE_RETURN          0x13


/* Token Parser */

void  ungetToken     ( const char *c ) ;
void  getToken       ( char *c, FILE *fd = NULL ) ;
void  setDefaultFile ( FILE *fd ) ;

/*
  Address/Opcodes are:

  ???? ????   -- 8 bit opcode.
  ???? xxxx   -- 4 bit opcode with 4 bit variable address.
  ???? ????   -- ditto - plus a float constant stored in the next 4 bytes.
  ???? ????   -- ditto - plus a code address stored in the next 2 bytes.
*/

typedef unsigned short PSL_Address  ;


class PSL_Symbol
{
public:
  char *symbol ;
  PSL_Address address ;

  PSL_Symbol ()
  {
    symbol = NULL ;
    address = 0 ;
  }

  void set ( const char *s, PSL_Address v )
  {
    symbol = new char [ strlen ( s ) + 1 ] ;
    strcpy ( symbol, s ) ;
    address = v ;
  }

  ~PSL_Symbol () { delete symbol ; }
} ;



class PSL_Context
{
  PSL_Opcode    *code       ;
  PSL_Extension *extensions ;
  PSL_Program   *program    ;

  PSL_Variable   variable [ MAX_VARIABLE ] ;
  PSL_Variable   stack    [ MAX_STACK    ] ; 
  int            sp ;
  PSL_Address    pc ;

public:

  PSL_Context ( PSL_Program *p )
  {
    code       = p -> getCode       () ;
    extensions = p -> getExtensions () ;
    program    = p ;
    reset () ;
  }

  ~PSL_Context () {} ;

  void pushInt      ( int          x ) { stack [ sp++ ] . i = x ; }
  void pushFloat    ( float        x ) { stack [ sp++ ] . f = x ; }
  void pushVariable ( PSL_Variable x ) { stack [ sp++ ]     = x ; }

  void         popVoid     () {                --sp       ; }
  int          popInt      () { return stack [ --sp ] . i ; }
  float        popFloat    () { return stack [ --sp ] . f ; }
  PSL_Variable popVariable () { return stack [ --sp ]     ; }

  PSL_Result step () ;

  void reset ()
  {
    memset ( variable, 0, MAX_VARIABLE * sizeof ( PSL_Variable ) ) ;
    sp = 0 ;
    pc = 0 ;
  }
} ;


class PSL_Parser
{
  void pushCodeByte ( unsigned char b ) ;
  void pushCodeAddr ( PSL_Address a ) ;

  /* Basic low level code generation.  */

  void pushPop          () ;
  void pushSubtract     () ;
  void pushAdd          () ;
  void pushDivide       () ;
  void pushMultiply     () ;
  void pushNegate       () ;
  void pushLess         () ;
  void pushLessEqual    () ;
  void pushGreater      () ;
  void pushGreaterEqual () ;
  void pushNotEqual     () ;
  void pushEqual        () ;
  int  pushJumpIfFalse  ( int l ) ;
  int  pushJump         ( int l ) ;

  void pushConstant   ( const char *c ) ;
  void pushVariable   ( const char *c ) ;
  void pushAssignment ( const char *c ) ;
  void pushCall       ( const char *c, int argc ) ;
  void pushReturn     () ;

  /* Higher level parsers.  */

  int pushPrimitive      () ;
  int pushMultExpression () ;
  int pushAddExpression  () ;
  int pushRelExpression  () ;
  int pushExpression     () ;

  /* Top level parsers. */

  int  pushReturnStatement     () ;
  int  pushPauseStatement      () ;
  int  pushWhileStatement      () ;
  int  pushIfStatement         () ;
  int  pushFunctionCall        ( const char *c ) ;
  int  pushAssignmentStatement ( const char *c ) ;
  int  pushCompoundStatement   () ;
  int  pushStatement           () ;

  int  pushFunctionDeclaration       ( const char *fn ) ;
  int  pushLocalVariableDeclaration  () ;
  int  pushGlobalVariableDeclaration ( const char *fn ) ;
  int  pushStaticVariableDeclaration () ;

  int  pushGlobalDeclaration         () ;
  void pushProgram                   () ;

  void print_opcode ( FILE *fd, unsigned char op ) const ;

  PSL_Address    getVarSymbol       ( const char *s ) ;
  PSL_Address    setVarSymbol       ( const char *s ) ;

  PSL_Address    getCodeSymbol      ( const char *s ) ;
  void           setCodeSymbol      ( const char *s, PSL_Address v ) ;
  int            getExtensionSymbol ( const char *s ) ;

private:

  int next_var   ;
  int next_label ;
  int next_code  ;
  int next_code_symbol ;

  PSL_Symbol         symtab [ MAX_SYMBOL ] ;
  PSL_Symbol    code_symtab [ MAX_SYMBOL ] ;

  PSL_Opcode    *code       ;
  PSL_Context   *context    ;
  PSL_Extension *extensions ;

public:

  PSL_Parser ( PSL_Opcode *_code, PSL_Extension *_extn )
  {
    code       = _code ;
    extensions = _extn ;

    for ( int i = 0 ; i < MAX_SYMBOL ; i++ )
    {
      symtab [ i ] . symbol = NULL ;
      code_symtab [ i ] . symbol = NULL ;
    }

    init () ;
  }

  ~PSL_Parser ()
  {
    for ( int i = 0 ; i < MAX_SYMBOL ; i++ )
    {
      delete symtab [ i ] . symbol ;
      delete code_symtab [ i ] . symbol ;
    }
  }

  PSL_Extension *getExtensions () const { return extensions ; }

  void init () 
  {
    int i ;

    for ( i = 0 ; i < MAX_CODE   ; i++ ) code   [ i ] = OPCODE_HALT ; 
    for ( i = 0 ; i < MAX_SYMBOL ; i++ ) delete symtab [ i ] . symbol ;
    for ( i = 0 ; i < MAX_SYMBOL ; i++ ) symtab [ i ] . symbol = NULL ;
    for ( i = 0 ; i < MAX_SYMBOL ; i++ ) delete code_symtab [ i ] . symbol ;
    for ( i = 0 ; i < MAX_SYMBOL ; i++ ) code_symtab [ i ] . symbol = NULL ;

    next_label = 0 ;
    next_code_symbol = 0 ;
    next_code  = 0 ;
    next_var   = 0 ;
  }

  void dump () const ;
  int  parse ( const char *fname ) ;
  int  parse ( FILE *fd ) ;
} ;


