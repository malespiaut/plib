
#include "pslLocal.h"

PSL_Result PSL_Context::step ()
{
  switch ( code [ pc ] )
  {
    case OPCODE_PUSH_CONSTANT :
      {
        PSL_Variable ff ;

        memcpy ( & ff, & code [ pc+1 ], sizeof(PSL_Variable) ) ;

        pushVariable ( ff ) ;

        pc += sizeof(PSL_Variable) + 1 ;
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
          ulSetError ( UL_WARNING,
                    "PSL: Wrong number of parameters for function %s\n",
                                          extensions [ ext ] . symbol ) ;
        }

        PSL_Variable argv [ MAX_ARGS ] ;

        /* Pop args off the stack in reverse order */

        for ( int i = argc-1 ; i >= 0 ; i-- )
          argv [ i ] = popVariable () ;

        pushVariable ( (*(extensions [ ext ] . func)) (argc,argv,program) ) ; 
        pc++ ;
      }
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_CALL :
      pushInt ( pc+4 ) ;
      memcpy ( & pc, & code [ pc+1 ], sizeof ( int ) ) ;
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_RETURN :
      {
        PSL_Variable result = popVariable () ;
        pc = popInt () ;
        pushVariable ( result ) ;
      }
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_SUB :
      stack [ sp - 2 ].f -= stack [ sp - 1 ].f ;
      popVoid () ;
      pc++ ;
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_ADD           :
      stack [ sp - 2 ].f += stack [ sp - 1 ].f ;
      popVoid () ;
      pc++ ;
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_DIV           :
      stack [ sp - 2 ].f /= stack [ sp - 1 ].f ;
      popVoid () ;
      pc++ ;
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_MULT          :
      stack [ sp - 2 ].f *= stack [ sp - 1 ].f ;
      popVoid () ;
      pc++ ;
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_NEG           :
      stack [ sp - 1 ].f = -stack [ sp - 1 ].f ;
      pc++ ;
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_LESS          :
      stack [ sp - 2 ].f = ( stack [ sp - 2 ].f < stack [ sp - 1 ].f ) ;
      popVoid () ;
      pc++ ;
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_LESSEQUAL     :
      stack [ sp - 2 ].f = ( stack [ sp - 2 ].f <= stack [ sp - 1 ].f ) ;
      popVoid () ;
      pc++ ;
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_GREATER       :
      stack [ sp - 2 ].f = ( stack [ sp - 2 ].f > stack [ sp - 1 ].f ) ;
      popVoid () ;
      pc++ ;
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_GREATEREQUAL  :
      stack [ sp - 2 ].f = ( stack [ sp - 2 ].f >= stack [ sp - 1 ].f ) ;
      popVoid () ;
      pc++ ;
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_NOTEQUAL      :
      stack [ sp - 2 ].f = ( stack [ sp - 2 ].f != stack [ sp - 1 ].f ) ;
      popVoid () ;
      pc++ ;
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_EQUAL         :
      stack [ sp - 2 ].f = ( stack [ sp - 2 ].f == stack [ sp - 1 ].f ) ;
      popVoid () ;
      pc++ ;
      return PSL_PROGRAM_CONTINUE ;

    case OPCODE_PAUSE :
      pc++ ;
      return PSL_PROGRAM_PAUSE ;

    case OPCODE_HALT :
      return PSL_PROGRAM_END ;   /* Note: PC is *NOT* incremented. */

    case OPCODE_JUMP_FALSE    :
      if ( popFloat () )
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
        pushVariable ( variable [ code[pc] & 0x0F ] ) ;
        pc++ ;
      }
      else
      if ( ( code [ pc ] & 0xF0 ) ==  OPCODE_POP_VARIABLE )
      {
        variable [ code[pc] & 0x0F ] = popVariable () ;
        pc++ ;
      }
      return PSL_PROGRAM_CONTINUE ;
  }
}


