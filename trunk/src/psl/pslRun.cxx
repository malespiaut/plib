
#include "pslPrivate.h"

PSL_Result PSL_Context::step ()
{
  switch ( code [ pc ] )
  {
    case OPCODE_PUSH_CONSTANT :
      {
        char *ff = (char *) & ( stack [ sp++ ] ) ;
        ff[0] = code [ ++pc ] ;
        ff[1] = code [ ++pc ] ;
        ff[2] = code [ ++pc ] ;
        ff[3] = code [ ++pc ] ;
        pc++ ;
      }
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_POP :
      sp-- ;
      pc++ ;
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_CALLEXT :
      {
        int ext  = code [ ++pc ] ;
        int argc = code [ ++pc ] ;
        int required_argc = extensions [ ext ] . argc ;

        if ( required_argc >= 0 && argc != required_argc )
        {
          fprintf ( stderr, "PSL: Arg count error for function %s\n",
                                          extensions [ ext ] . symbol ) ;
        }

        float argv [ MAX_ARGS ] ;

        /* Pop args off the stack in reverse order */

        for ( int i = argc-1 ; i >= 0 ; i-- )
          argv [ i ] = stack [ --sp ] ;

        stack [ sp++ ] = (*(extensions [ ext ] . func)) ( argc, argv ) ; 
        pc++ ;
      }
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_CALL :
      fprintf ( stderr, "CALL not implemented!\n" ) ;
      pc += 4 ;  /* Not implemented */
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_SUB :
      stack [ sp - 2 ] -= stack [ sp - 1 ] ;
      sp-- ;
      pc++ ;
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_ADD           :
      stack [ sp - 2 ] += stack [ sp - 1 ] ;
      sp-- ;
      pc++ ;
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_DIV           :
      stack [ sp - 2 ] /= stack [ sp - 1 ] ;
      sp-- ;
      pc++ ;
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_MULT          :
      stack [ sp - 2 ] *= stack [ sp - 1 ] ;
      sp-- ;
      pc++ ;
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_NEG           :
      stack [ sp - 1 ] = -stack [ sp - 1 ] ;
      pc++ ;
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_LESS          :
      stack [ sp - 2 ] = ( stack [ sp - 2 ] < stack [ sp - 1 ] ) ;
      sp-- ;
      pc++ ;
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_LESSEQUAL     :
      stack [ sp - 2 ] = ( stack [ sp - 2 ] <= stack [ sp - 1 ] ) ;
      sp-- ;
      pc++ ;
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_GREATER       :
      stack [ sp - 2 ] = ( stack [ sp - 2 ] > stack [ sp - 1 ] ) ;
      sp-- ;
      pc++ ;
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_GREATEREQUAL  :
      stack [ sp - 2 ] = ( stack [ sp - 2 ] >= stack [ sp - 1 ] ) ;
      sp-- ;
      pc++ ;
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_NOTEQUAL      :
      stack [ sp - 2 ] = ( stack [ sp - 2 ] != stack [ sp - 1 ] ) ;
      sp-- ;
      pc++ ;
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_EQUAL         :
      stack [ sp - 2 ] = ( stack [ sp - 2 ] == stack [ sp - 1 ] ) ;
      sp-- ;
      pc++ ;
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_PAUSE :
      pc++ ;
      return PSL_PROGRAM_PAUSE ;

    case OPCODE_HALT :
      return PSL_PROGRAM_END ;   /* Note: PC is *NOT* incremented. */

    case OPCODE_JUMP_FALSE    :
      if ( stack [ --sp ] )
        pc += 3 ;
      else
        pc = code [ pc + 1 ] + ( code [ pc + 2 ] << 8 ) ;
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_JUMP :
      pc = code [ pc + 1 ] + ( code [ pc + 2 ] << 8 ) ;
      return PSL_PROGRAM_CONTINUE ;

    default :
      if ( ( code [ pc ] & 0xF0 ) ==  OPCODE_PUSH_VARIABLE )
      {
        stack [ sp++ ] = variable [ code[pc] & 0x0F ] ;
        pc++ ;
      }
      else
      if ( ( code [ pc ] & 0xF0 ) ==  OPCODE_POP_VARIABLE )
      {
        variable [ code[pc] & 0x0F ] = stack [ --sp ] ;
        pc++ ;
      }
      return PSL_PROGRAM_CONTINUE ;
  }
}


