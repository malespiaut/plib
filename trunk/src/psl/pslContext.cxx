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

pslResult pslContext::step ()
{
  switch ( code [ pc ] )
  {
    case OPCODE_NOOP :
      error ( "Suspicious opcode 0x00?!", code[pc] ) ;
      pc++ ;
      return PSL_PROGRAM_END ;

    case OPCODE_PUSH_INT_CONSTANT :
      {
        int ii ;

        memcpy ( & ii, & code [ pc+1 ], sizeof(int) ) ;

        pushInt ( ii ) ;

        pc += sizeof(int) + 1 ;
      }
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_PUSH_FLOAT_CONSTANT :
      {
        float ff ;

        memcpy ( & ff, & code [ pc+1 ], sizeof(float) ) ;

        pushFloat ( ff ) ;

        pc += sizeof(float) + 1 ;
      }
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_PUSH_STRING_CONSTANT :
      {
        char *ff = (char *) & code [ pc+1 ] ;

        pushString ( ff ) ;

        pc += strlen ( ff ) + 1 + 1 ;
      }
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_GET_PARAMETER :
      {
        int var = code [ ++pc ] ;
        int nargs = stack [ sp - 2 ] . getInt () ;
        int off = sp - ( nargs + 2 )  + code [ ++pc ] ;

        variable [ var ] . set ( & stack [ off ] ) ;
        pc++ ;
      }
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_POP :
      popVoid() ;
      pc++ ;
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_CALLEXT :
      {
        int ext  = code [ ++pc ] ;
        int argc = code [ ++pc ] ;
        int required_argc = extensions [ ext ] . argc ;

        if ( required_argc >= 0 && argc != required_argc )
        {
          warning ( "Wrong number of parameters for function %s\n",
                                          extensions [ ext ] . symbol ) ;
        }

        pslValue argv [ MAX_ARGS ] ;

        /* Pop args off the stack in reverse order */

        for ( int i = argc-1 ; i >= 0 ; i-- )
          popNumber ( & argv[i] ) ;

        /* Euwww! */

        pslValue v = (*(extensions [ ext ] . func)) (argc,argv,program) ; 

        pushNumber ( & v ) ;
        pc++ ;
      }
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_CALL :
      pushInt ( pc+4 ) ;
      memcpy ( & pc, & code [ pc+1 ], sizeof ( int ) ) ;
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_RETURN :
      {
        int nargs ;
        pslValue result ;

        popNumber ( &result ) ;
        pc = popInt () ;
        nargs = popInt () ;
        popVoid ( nargs ) ;
        pushNumber ( &result ) ;
      }
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_STACK_DUPLICATE :
      {
        pslValue *v1 = & stack [ sp - 1 ] ;
        pushNumber ( v1 ) ;
        pc++ ;
      }
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_SUB :
      {
        pslValue *v1 = & stack [ sp - 1 ] ;
        pslValue *v2 = & stack [ sp - 2 ] ;

        if ( v1->getType() == PSL_FLOAT || v2->getType() == PSL_FLOAT )
          v2 -> set ( v2 -> getFloat() - v1 -> getFloat() ) ;
        else
          v2 -> set ( v2 -> getInt() - v1 -> getInt() ) ;

        popVoid () ;
        pc++ ;
      }
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_SHIFTLEFT :
      {
        pslValue *v1 = & stack [ sp - 1 ] ;
        pslValue *v2 = & stack [ sp - 2 ] ;

        v2 -> set ( v2 -> getInt () << v1 -> getInt () ) ;

        popVoid () ;
        pc++ ;
      }
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_SHIFTRIGHT :
      {
        pslValue *v1 = & stack [ sp - 1 ] ;
        pslValue *v2 = & stack [ sp - 2 ] ;

        v2 -> set ( v2 -> getInt () >> v1 -> getInt () ) ;

        popVoid () ;
        pc++ ;
      }
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_ADD :
      {
        pslValue *v1 = & stack [ sp - 1 ] ;
        pslValue *v2 = & stack [ sp - 2 ] ;

        if ( v1->getType() == PSL_FLOAT || v2->getType() == PSL_FLOAT )
          v2 -> set ( v2 -> getFloat() + v1 -> getFloat() ) ;
        else
          v2 -> set ( v2 -> getInt  () + v1 -> getInt  () ) ;

        popVoid () ;
        pc++ ;
      }
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_NOT :
      {
        pslValue *v1 = & stack [ sp - 1 ] ;

        v1 -> set ( ! v1 -> getInt () ) ;

        pc++ ;
      }
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_TWIDDLE :
      {
        pslValue *v1 = & stack [ sp - 1 ] ;

        v1 -> set ( ~ v1 -> getInt () ) ;

        pc++ ;
      }
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_OROR :
      {
        pslValue *v1 = & stack [ sp - 1 ] ;
        pslValue *v2 = & stack [ sp - 2 ] ;

        v2 -> set ( v2 -> getInt () || v1 -> getInt () ) ;

        popVoid () ;
        pc++ ;
      }
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_ANDAND :
      {
        pslValue *v1 = & stack [ sp - 1 ] ;
        pslValue *v2 = & stack [ sp - 2 ] ;

        v2 -> set ( v2 -> getInt () && v1 -> getInt () ) ;

        popVoid () ;
        pc++ ;
      }
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_OR :
      {
        pslValue *v1 = & stack [ sp - 1 ] ;
        pslValue *v2 = & stack [ sp - 2 ] ;

        v2 -> set ( v2 -> getInt () | v1 -> getInt () ) ;

        popVoid () ;
        pc++ ;
      }
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_AND :
      {
        pslValue *v1 = & stack [ sp - 1 ] ;
        pslValue *v2 = & stack [ sp - 2 ] ;

        v2 -> set ( v2 -> getInt () & v1 -> getInt () ) ;

        popVoid () ;
        pc++ ;
      }
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_XOR :
      {
        pslValue *v1 = & stack [ sp - 1 ] ;
        pslValue *v2 = & stack [ sp - 2 ] ;

        v2 -> set ( v2 -> getInt () ^ v1 -> getInt  () ) ;

        popVoid () ;
        pc++ ;
      }
      return PSL_PROGRAM_CONTINUE ;


    case OPCODE_DIV :
      {
        pslValue *v1 = & stack [ sp - 1 ] ;
        pslValue *v2 = & stack [ sp - 2 ] ;

        if ( v1->getType() == PSL_FLOAT || v2->getType() == PSL_FLOAT )
        {
          if ( v1 -> getFloat() != 0.0f )
            v2 -> set ( v2 -> getFloat() / v1 -> getFloat() ) ;
          else
            warning ( "Floating Point Divide by Zero!" ) ;
        }
        else
        {
          if ( v1 -> getInt() != 0 )
            v2 -> set ( v2 -> getInt() / v1 -> getInt() ) ;
          else
            warning ( "Integer Divide by Zero!" ) ;
        }
        popVoid () ;
        pc++ ;
      }
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_MOD :
      {
        pslValue *v1 = & stack [ sp - 1 ] ;
        pslValue *v2 = & stack [ sp - 2 ] ;

        if ( v1->getType() == PSL_FLOAT || v2->getType() == PSL_FLOAT )
        {
          warning ( "Floating Point Modulo!" ) ;
        }
        else
        {
          if ( v1 -> getInt() != 0 )
            v2 -> set ( v2 -> getInt () % v1 -> getInt () ) ;
          else
            warning ( "Integer Modulo Zero!" ) ;
        }

        popVoid () ;
        pc++ ;
      }
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_MULT :
      {
        pslValue *v1 = & stack [ sp - 1 ] ;
        pslValue *v2 = & stack [ sp - 2 ] ;

        if ( v1->getType() == PSL_FLOAT || v2->getType() == PSL_FLOAT )
          v2 -> set ( v2 -> getFloat() * v1 -> getFloat() ) ;
        else
          v2 -> set ( v2 -> getInt  () * v1 -> getInt  () ) ;

        popVoid () ;
        pc++ ;
      }
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_NEG :
      {
        pslValue *v1 = & stack [ sp - 1 ] ;

        if ( v1->getType() == PSL_FLOAT )
          v1 -> set ( - v1 -> getFloat() ) ;
        else
          v1 -> set ( - v1 -> getInt  () ) ;

        pc++ ;
      }
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_LESS :
      {
        pslValue *v1 = & stack [ sp - 1 ] ;
        pslValue *v2 = & stack [ sp - 2 ] ;

        if ( v1->getType() == PSL_FLOAT || v2->getType() == PSL_FLOAT )
          v2 -> set ( v2 -> getFloat() < v1 -> getFloat() ) ;
        else
          v2 -> set ( v2 -> getInt  () < v1 -> getInt  () ) ;

        popVoid () ;
        pc++ ;
      }
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_LESSEQUAL :
      {
        pslValue *v1 = & stack [ sp - 1 ] ;
        pslValue *v2 = & stack [ sp - 2 ] ;

        if ( v1->getType() == PSL_FLOAT || v2->getType() == PSL_FLOAT )
          v2 -> set ( v2 -> getFloat() <= v1 -> getFloat() ) ;
        else
          v2 -> set ( v2 -> getInt  () <= v1 -> getInt  () ) ;

        popVoid () ;
        pc++ ;
      }
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_GREATER :
      {
        pslValue *v1 = & stack [ sp - 1 ] ;
        pslValue *v2 = & stack [ sp - 2 ] ;

        if ( v1->getType() == PSL_FLOAT || v2->getType() == PSL_FLOAT )
          v2 -> set ( v2 -> getFloat() > v1 -> getFloat() ) ;
        else
          v2 -> set ( v2 -> getInt  () > v1 -> getInt  () ) ;

        popVoid () ;
        pc++ ;
      }
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_GREATEREQUAL :
      {
        pslValue *v1 = & stack [ sp - 1 ] ;
        pslValue *v2 = & stack [ sp - 2 ] ;

        if ( v1->getType() == PSL_FLOAT || v2->getType() == PSL_FLOAT )
          v2 -> set ( v2 -> getFloat() >= v1 -> getFloat() ) ;
        else
          v2 -> set ( v2 -> getInt  () >= v1 -> getInt  () ) ;

        popVoid () ;
        pc++ ;
      }
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_NOTEQUAL :
      {
        pslValue *v1 = & stack [ sp - 1 ] ;
        pslValue *v2 = & stack [ sp - 2 ] ;

        if ( v1->getType() == PSL_FLOAT || v2->getType() == PSL_FLOAT )
          v2 -> set ( v2 -> getFloat() != v1 -> getFloat() ) ;
        else
          v2 -> set ( v2 -> getInt  () != v1 -> getInt  () ) ;

        popVoid () ;
        pc++ ;
      }
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_EQUAL :
      {
        pslValue *v1 = & stack [ sp - 1 ] ;
        pslValue *v2 = & stack [ sp - 2 ] ;

        if ( v1->getType() == PSL_FLOAT || v2->getType() == PSL_FLOAT )
          v2 -> set ( v2 -> getFloat() == v1 -> getFloat() ) ;
        else
          v2 -> set ( v2 -> getInt  () == v1 -> getInt  () ) ;

        popVoid () ;
        pc++ ;
      }
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_PAUSE :
      pc++ ;
      return PSL_PROGRAM_PAUSE ;

    case OPCODE_HALT :
      return PSL_PROGRAM_END ;   /* Note: PC is *NOT* incremented. */

    case OPCODE_JUMP_TRUE :
      if ( popInt () )
        pc = code [ pc + 1 ] + ( code [ pc + 2 ] << 8 ) ;
      else
        pc += 3 ;
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_JUMP_FALSE :
      if ( popInt () )
        pc += 3 ;
      else
        pc = code [ pc + 1 ] + ( code [ pc + 2 ] << 8 ) ;
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_JUMP :
      pc = code [ pc + 1 ] + ( code [ pc + 2 ] << 8 ) ;
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_PUSH_VARIABLE :
      pushNumber ( & variable [ code[++pc] ] ) ;
      pc++ ;
      return PSL_PROGRAM_CONTINUE ;


    case OPCODE_POP_ADD_VARIABLE :
      {
        pslVariable *v = & ( variable [ code[++pc] ] ) ;

        if ( v -> getType () == PSL_INT )
          v -> set ( v -> getInt() + stack[--sp].getInt()) ;
        else
          v -> set ( v -> getFloat() + stack[--sp].getFloat()) ;
        pc++ ;
      }
      return PSL_PROGRAM_CONTINUE ;


    case OPCODE_POP_SUB_VARIABLE :
      {
        pslVariable *v = & ( variable [ code[++pc] ] ) ;

        if ( v -> getType () == PSL_INT )
          v -> set ( v -> getInt() - stack[--sp].getInt()) ;
        else
          v -> set ( v -> getFloat() - stack[--sp].getFloat()) ;
        pc++ ;
      }
      return PSL_PROGRAM_CONTINUE ;


    case OPCODE_POP_MUL_VARIABLE :
      {
        pslVariable *v = & ( variable [ code[++pc] ] ) ;

        if ( v -> getType () == PSL_INT )
          v -> set ( v -> getInt() * stack[--sp].getInt()) ;
        else
          v -> set ( v -> getFloat() * stack[--sp].getFloat()) ;
        pc++ ;
      }
      return PSL_PROGRAM_CONTINUE ;


    case OPCODE_POP_MOD_VARIABLE :
      {
        pslVariable *v = & ( variable [ code[++pc] ] ) ;
        pslValue   *vv = & stack[--sp] ;

        if ( v -> getType () == PSL_INT )
        {
          if ( vv -> getInt () != 0 )
            v -> set ( v -> getInt() % vv->getInt()) ;
          else
            warning ( "Integer Modulo by Zero!" ) ;
        }
        else
          warning ( "Floating Point Modulo!" ) ;

        pc++ ;
      }
      return PSL_PROGRAM_CONTINUE ;



    case OPCODE_POP_DIV_VARIABLE :
      {
        pslVariable *v = & ( variable [ code[++pc] ] ) ;
        pslValue   *vv = & stack[--sp] ;

        if ( v -> getType () == PSL_INT )
        {
          if ( vv -> getInt () != 0 )
            v -> set ( v -> getInt() / vv->getInt()) ;
          else
            warning ( "Integer Divide by Zero!" ) ;
        }
        else
        {
          if ( vv -> getFloat () != 0.0f )
            v -> set ( v -> getFloat() / vv->getFloat()) ;
          else
            warning ( "Floating Point Divide by Zero!" ) ;
        }
        pc++ ;
      }
      return PSL_PROGRAM_CONTINUE ;


    case OPCODE_POP_AND_VARIABLE :
      {
        pslVariable *v = & ( variable [ code[++pc] ] ) ;

        v -> set ( v -> getInt() & stack[--sp].getInt()) ;
        pc++ ;
      }
      return PSL_PROGRAM_CONTINUE ;


    case OPCODE_POP_OR_VARIABLE  :
      {
        pslVariable *v = & ( variable [ code[++pc] ] ) ;

        v -> set ( v -> getInt() | stack[--sp].getInt()) ;
        pc++ ;
      }
      return PSL_PROGRAM_CONTINUE ;


    case OPCODE_POP_XOR_VARIABLE :
      {
        pslVariable *v = & ( variable [ code[++pc] ] ) ;

        v -> set ( v -> getInt() ^ stack[--sp].getInt()) ;
        pc++ ;
      }
      return PSL_PROGRAM_CONTINUE ;


    case OPCODE_POP_SHL_VARIABLE :
      {
        pslVariable *v = & ( variable [ code[++pc] ] ) ;

        v -> set ( v -> getInt() << stack[--sp].getInt()) ;
        pc++ ;
      }
      return PSL_PROGRAM_CONTINUE ;


    case OPCODE_POP_SHR_VARIABLE :
      {
        pslVariable *v = & ( variable [ code[++pc] ] ) ;

        v -> set ( v -> getInt() >> stack[--sp].getInt()) ;
        pc++ ;
      }
      return PSL_PROGRAM_CONTINUE ;


    case OPCODE_POP_VARIABLE :
      popNumber ( & variable [ code[++pc] ] ) ;
      pc++ ;
      return PSL_PROGRAM_CONTINUE ;


    case OPCODE_SET_INT_VARIABLE :
      variable [ code[++pc] ] . setType ( PSL_INT ) ;
      pc++ ;
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_SET_FLOAT_VARIABLE :
      variable [ code[++pc] ] . setType ( PSL_FLOAT ) ;
      pc++ ;
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_SET_STRING_VARIABLE :
      variable [ code[++pc] ] . setType ( PSL_STRING ) ;
      pc++ ;
      return PSL_PROGRAM_CONTINUE ;

    default :
      error ( "Suspicious opcode 0x02x?!", code[pc] ) ;
      return PSL_PROGRAM_END ;
  }
}


