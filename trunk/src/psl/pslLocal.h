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

#define MAX_ARGS      64
#define MAX_VARIABLE 256
#define MAX_LABEL    256
#define MAX_TOKEN    256 
#define MAX_CODE     512
#define MAX_STACK    256 
#define MAX_NESTING   32 
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

void  pslUngetToken     ( const char *c ) ;
void  pslGetToken       ( char *c, FILE *fd = NULL ) ;
void  pslSetDefaultFile ( FILE *fd ) ;

/*
  Address/Opcodes are:

  ???? ????   -- 8 bit opcode.
  ???? xxxx   -- 4 bit opcode with 4 bit variable address.
  ???? ????   -- ditto - plus a float constant stored in the next 4 bytes.
  ???? ????   -- ditto - plus a code address stored in the next 2 bytes.
*/

typedef unsigned short pslAddress  ;

extern int _pslInitialised ;

#include "pslSymbol.h"
#include "pslContext.h"
#include "pslCompiler.h"


