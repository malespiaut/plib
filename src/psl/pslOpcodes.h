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


/* Code Opcodes */

#define OPCODE_NOOP                  0x00
#define OPCODE_PUSH_INT_CONSTANT     0x01
#define OPCODE_PUSH_FLOAT_CONSTANT   0x02
#define OPCODE_PUSH_STRING_CONSTANT  0x03
#define OPCODE_CALL                  0x04
#define OPCODE_SUB                   0x05
#define OPCODE_ADD                   0x06
#define OPCODE_DIV                   0x07
#define OPCODE_MULT                  0x08
#define OPCODE_MOD                   0x09
#define OPCODE_NEG                   0x0A
#define OPCODE_LESS                  0x0B
#define OPCODE_LESSEQUAL             0x0C
#define OPCODE_GREATER               0x0D
#define OPCODE_GREATEREQUAL          0x0E
#define OPCODE_NOTEQUAL              0x0F
#define OPCODE_EQUAL                 0x10
#define OPCODE_JUMP_FALSE            0x11
#define OPCODE_JUMP_TRUE             0x12
#define OPCODE_JUMP                  0x13
#define OPCODE_POP                   0x14
#define OPCODE_HALT                  0x15
#define OPCODE_CALLEXT               0x16
#define OPCODE_PAUSE                 0x17
#define OPCODE_RETURN                0x18
#define OPCODE_PUSH_VARIABLE         0x19
#define OPCODE_POP_VARIABLE          0x1A
#define OPCODE_SET_INT_VARIABLE      0x1B
#define OPCODE_SET_FLOAT_VARIABLE    0x1C
#define OPCODE_SET_STRING_VARIABLE   0x1D

