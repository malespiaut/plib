
#include "pslLocal.h"


struct OpcodeDecode
{
  char *s ;
  unsigned char opcode ;
} ;



OpcodeDecode opcodeDecode [] =
{
  { "PUSH_CONSTANT",   OPCODE_PUSH_CONSTANT },
  { "CALL",            OPCODE_CALL          },
  { "PAUSE",           OPCODE_PAUSE         },
  { "JUMP_FALSE",      OPCODE_JUMP_FALSE    },
  { "JUMP",            OPCODE_JUMP          },
  { "CALLEXT",         OPCODE_CALLEXT       },
  { "SUB",             OPCODE_SUB           },
  { "ADD",             OPCODE_ADD           },
  { "DIV",             OPCODE_DIV           },
  { "MULT",            OPCODE_MULT          },
  { "NEG",             OPCODE_NEG           },
  { "LESS",            OPCODE_LESS          },
  { "LESSEQUAL",       OPCODE_LESSEQUAL     },
  { "GREATER",         OPCODE_GREATER       },
  { "GREATEREQUAL",    OPCODE_GREATEREQUAL  },
  { "NOTEQUAL",        OPCODE_NOTEQUAL      },
  { "EQUAL",           OPCODE_EQUAL         },
  { "POP",             OPCODE_POP           },
  { "HALT",            OPCODE_HALT          },
  { "RETURN",          OPCODE_RETURN        },
  { NULL, 0 }
} ;


void PSL_Parser::print_opcode ( FILE *fd, unsigned char op )
{
  if ( ( op & 0xF0 ) == OPCODE_PUSH_VARIABLE )
    fprintf ( fd, "  PUSH_VAR\t%s", symtab [ op & 0x0F ] . symbol ) ;
  else
  if ( ( op & 0xF0 ) == OPCODE_POP_VARIABLE )
    fprintf ( fd, "  POP_VAR\t%s", symtab [ op & 0x0F ] . symbol ) ;
  else
  for ( int i = 0 ; opcodeDecode [ i ] . s != NULL ; i++ )
    if ( opcodeDecode [ i ] . opcode == op )
    {
      fprintf ( fd, "  %s", opcodeDecode [ i ] . s ) ;
      break ;
    }
}


void PSL_Parser::dump ()
{
  int i ;

  printf ( "\n" ) ;
  printf ( "Bytecode:\n" ) ;

  for ( i = 0 ; i < next_code ; i++ )
  {
    if ( code [ i ] == OPCODE_PUSH_CONSTANT )
    {
      printf ( "%3d: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x  ", i, code [ i ],
                   code [i+1], code [i+2], code [i+3], code [i+4] ) ;
      print_opcode ( stdout, code [ i ] ) ;

      float f ;
      memcpy ( &f, &code[i+1], sizeof(float) ) ;
      printf ( "\t%f", f ) ;
      i += 4 ;
    }
    else
    if ( code [ i ] == OPCODE_CALLEXT )
    {
      printf ( "%3d: 0x%02x 0x%02x 0x%02x            ", i, code [ i ], code [i+1], code [i+2] ) ;
      print_opcode ( stdout, code [ i ] ) ;

      int ext  = code[i+1] ;
      int argc = code[i+2] ;
      printf ( "\t%s,nargs=%d", extensions[ext].symbol, argc ) ;
      i += 2 ;
    }
    else
    if ( code [ i ] == OPCODE_CALL )
    {
      printf ( "%3d: 0x%02x 0x%02x 0x%02x 0x%02x       ", i, code [ i ],
                   code [i+1], code [i+2], code [i+3] ) ;
      print_opcode ( stdout, code [ i ] ) ;

      unsigned short lab   = code[i+1] + ( code[i+2] << 8 ) ;
      unsigned char  nargs = code[i+3] ;

      printf ( "\t\t%d,nargs=%d", lab, nargs ) ;

      i += 3 ;
    }
    else
    if ( code [ i ] == OPCODE_JUMP_FALSE || code [ i ] == OPCODE_JUMP )
    {
      printf ( "%3d: 0x%02x 0x%02x 0x%02x            ", i, code [ i ],
                   code [i+1], code [i+2] ) ;
      print_opcode ( stdout, code [ i ] ) ;

      unsigned short lab = code[i+1] + ( code[i+2] << 8 ) ;

      if ( code [ i ] == OPCODE_JUMP )
        printf ( "\t\t%d", lab ) ;
      else
        printf ( "\t%d", lab ) ;

      i += 2 ;
    }
    else
    {
      printf ( "%3d: 0x%02x                      ", i, code [ i ] ) ;
      print_opcode ( stdout, code [ i ] ) ;
    }

    printf ( "\n" ) ;
  }

  printf ( "\n" ) ;
  printf ( "Variables:\n" ) ;

  for ( i = 0 ; i < MAX_SYMBOL ; i++ )
    if ( symtab [ i ] . symbol != NULL )
    {
      printf ( "\t%5s => %4d", symtab[i].symbol,
                               symtab[i].address ) ;

      if ( i & 1 )
        printf ( "\n" ) ;
      else
        printf ( "  " ) ;
    }

  printf ( "\n" ) ;
  printf ( "Labels:\n" ) ;

  for ( i = 0 ; i < MAX_SYMBOL ; i++ )
    if ( code_symtab [ i ] . symbol != NULL )
    {
      printf ( "\t%5s => %4d", code_symtab[i].symbol,
                               code_symtab[i].address ) ;

      if ( i & 1 )
        printf ( "\n" ) ;
      else
        printf ( "  " ) ;
    }

  printf ( "\n" ) ;
}


