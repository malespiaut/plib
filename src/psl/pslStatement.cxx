
#include "pslPrivate.h"


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


int PSL_Parser::pushWhileStatement ()
{
  char lab1 [ 5 ] ;
  char lab2 [ 5 ] ;

  /* Remember place to jump back to */

  sprintf ( lab1, "%d", next_label++ ) ;
  setCodeSymbol ( lab1, next_code ) ;

  if ( ! pushExpression () )
  {
    fprintf ( stderr, "PSL: Missing expression for 'while'\n" ) ;
    return FALSE ;
  }

  sprintf ( lab2, "%d", next_label++ ) ;

  int label_loc = pushJumpIfFalse ( lab2 ) ;

  if ( ! pushStatement () )
  {
    fprintf ( stderr, "PSL: Missing statement for 'while'\n" ) ;
    return FALSE ;
  }

  pushJump ( lab1 ) ;

  setCodeSymbol ( lab2, next_code ) ;

  code [ label_loc   ] = next_code & 0xFF ;
  code [ label_loc+1 ] = ( next_code >> 8 ) & 0xFF ;
  return TRUE ;
}


int PSL_Parser::pushIfStatement ()
{
  char lab1 [ 5 ] ;
  char lab2 [ 5 ] ;

  if ( ! pushExpression () )
  {
    fprintf ( stderr, "PSL: Missing expression for 'if'\n" ) ;
    return FALSE ;
  }

  sprintf ( lab1, "%d", next_label++ ) ;
  sprintf ( lab2, "%d", next_label++ ) ;

  int else_loc = pushJumpIfFalse ( lab1 ) ;

  if ( ! pushStatement () )
  {
    fprintf ( stderr, "PSL: Missing statement for 'if'\n" ) ;
    return FALSE ;
  }

  char c [ MAX_TOKEN ] ;

  getToken ( c ) ;

  if ( c [ 0 ] != ';' )
  {
    ungetToken ( c ) ;
    return FALSE ;
  }

  getToken ( c ) ;

  if ( strcmp ( c, "else" ) != 0 )
  {
    setCodeSymbol ( lab1, next_code ) ;

    code [ else_loc   ] = next_code & 0xFF ;
    code [ else_loc+1 ] = ( next_code >> 8 ) & 0xFF ;

    ungetToken ( c ) ;
    ungetToken ( ";" ) ;
    return TRUE ;
  }

  int label_loc = pushJump ( lab2 ) ;

  setCodeSymbol ( lab1, next_code ) ;

  code [ else_loc   ] = next_code & 0xFF ;
  code [ else_loc+1 ] = ( next_code >> 8 ) & 0xFF ;

  if ( ! pushStatement () )
  {
    fprintf ( stderr, "PSL: Missing statement for 'else'\n" ) ;
    return FALSE ;
  }

  setCodeSymbol ( lab2, next_code ) ;

  code [ label_loc   ] = next_code & 0xFF ;
  code [ label_loc+1 ] = ( next_code >> 8 ) & 0xFF ;

  return TRUE ;
}


int PSL_Parser::pushFunctionCall ( char *var )
{
  char c [ MAX_TOKEN ] ;

  getToken ( c ) ;

  /*
    'var' should be the name of a function,
    'c'   should be an '('
  */

  if ( c[0] != '(' )
  {
    fprintf ( stderr, "PSL: Missing '(' in call to '%s'\n", var ) ;
    return FALSE ;
  }

  getToken ( c ) ;

  int argc = 0 ;

  while ( c[0] != ')' )
  { 
    ungetToken ( c ) ;
    pushExpression () ;
    argc++ ;
    getToken ( c ) ;

    if ( c[0] == ')' )
      break ;

    if ( c[0] == ',' )
      getToken ( c ) ;
    else
    {
      fprintf ( stderr, "Missing ')' or ',' in call to '%s'\n", var ) ;
      exit ( -1 ) ;
    }
  }

  pushCall ( var, argc ) ;
  return TRUE ;
}


int PSL_Parser::pushAssignmentStatement ( char *var )
{
  char c [ MAX_TOKEN ] ;

  getToken ( c ) ;

  if ( c [ 0 ] != '=' )
  {
    ungetToken ( c ) ;
    pushFunctionCall ( var ) ;
    pushPop () ;
    return TRUE ;
  }

  if ( pushExpression () )
  {
    pushAssignment ( var ) ;
    return TRUE ; 
  }

  return FALSE ;
}


int PSL_Parser::pushCompoundStatement ()
{
  char c [ MAX_TOKEN ] ;

  while ( pushStatement () )
  {
    getToken ( c ) ;

    if ( c[0] != ';' )
      return FALSE ;
  }

  getToken ( c ) ;

  if ( c[0] == '}' )
  {
    ungetToken ( ";" ) ;
    return TRUE ;
  }

  ungetToken ( c ) ;
  return FALSE ;
}


int PSL_Parser::pushStatement ()
{
  char c [ MAX_TOKEN ] ;

  getToken ( c ) ;

  if ( strcmp ( c, "pause" ) == 0 )
    return pushPauseStatement () ;

  if ( strcmp ( c, "while" ) == 0 )
    return pushWhileStatement () ;

  if ( strcmp ( c, "if" ) == 0 )
    return pushIfStatement () ;

  if ( isalnum ( c [ 0 ] ) )
    return pushAssignmentStatement ( c ) ;

  if ( c [ 0 ] == '{' )
    return pushCompoundStatement () ;

  ungetToken ( c ) ;
  return FALSE ;
}


void PSL_Parser::pushProgram ()
{
  char c [ MAX_TOKEN ] ;

  while ( pushStatement () )
  {
    getToken ( c ) ;

    if ( c[0] == '\0' )
      break ;

    if ( c[0] != ';' )
    {
      fprintf ( stderr, "PSL: Premature end of program or missing ';' (\"%s\")\n", c ) ;
      break ;
    }
  }

  pushCodeByte ( OPCODE_HALT ) ;
}


void PSL_Parser::dump ()
{
  int i ;

  printf ( "\n" ) ;
  printf ( "Bytecode:\n" ) ;

  for ( i = 0 ; i < MAX_CODE ; i++ )
  {
    if ( code [ i ] == OPCODE_HALT )
      break ;

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
      printf ( "\t%s %d", extensions[ext].symbol, argc ) ;
      i += 2 ;
    }
    else
    if ( code [ i ] == OPCODE_CALL )
    {
      printf ( "%3d: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x  ", i, code [ i ],
                   code [i+1], code [i+2], code [i+3], code [i+4] ) ;
      print_opcode ( stdout, code [ i ] ) ;
      i += 4 ;
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
}


