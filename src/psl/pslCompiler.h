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


struct pslFwdRef
{
  char       *symbol ;
  pslAddress  where ;

  void set ( const char *s, pslAddress w )
  {
    symbol = ulStrDup ( s ) ;
    where = w ;
  }

  pslAddress getWhere () const { return where ; }

  int matches ( const char *s ) const
  {
    return symbol != NULL && strcmp ( s, symbol ) == 0 ;
  }

} ;



class pslCompiler
{
  void pushCodeByte ( unsigned char b ) ;
  void pushCodeAddr ( pslAddress a ) ;

  /* Basic low level code generation.  */

  void pushPop          () ;
  void pushSubtract     () ;
  void pushAdd          () ;
  void pushDivide       () ;
  void pushMultiply     () ;
  void pushModulo       () ;
  void pushNegate       () ;
  void pushLess         () ;
  void pushLessEqual    () ;
  void pushGreater      () ;
  void pushGreaterEqual () ;
  void pushNotEqual     () ;
  void pushEqual        () ;
  int  pushJumpIfFalse  ( int l ) ;
  int  pushJump         ( int l ) ;

  void makeIntVariable   ( const char *s ) ;
  void makeFloatVariable ( const char *s ) ;
  void makeStringVariable( const char *s ) ;

  void pushConstant      ( const char *s ) ;
  void pushIntConstant   ( const char *s ) ;
  void pushFloatConstant ( const char *s ) ;
  void pushStringConstant( const char *s ) ;

  void pushVoidConstant  () ;

  void pushVariable      ( const char *s ) ;
  void pushAssignment    ( const char *s ) ;
  void pushCall          ( const char *s, int argc ) ;
  void pushReturn        () ;

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
  int  pushFunctionCall        ( const char *s ) ;
  int  pushAssignmentStatement ( const char *s ) ;
  int  pushCompoundStatement   () ;
  int  pushStatement           () ;

  int  pushFunctionDeclaration       ( const char *fn ) ;
  int  pushLocalVariableDeclaration  ( pslType t ) ;
  int  pushGlobalVariableDeclaration ( const char *fn, pslType t ) ;
  int  pushStaticVariableDeclaration () ;

  int  pushGlobalDeclaration         () ;
  void pushProgram                   () ;

  int printOpcode      ( FILE *fd, int addr ) const ;

  pslAddress getVarSymbol       ( const char *s ) ;
  pslAddress setVarSymbol       ( const char *s ) ;

  pslAddress getCodeSymbol      ( const char *s, pslAddress fixupLoc ) ;
  void       setCodeSymbol      ( const char *s, pslAddress v ) ;

  int        getExtensionSymbol ( const char *s ) const ;

private:

  int next_var   ;
  int next_label ;
  int next_code  ;
  int next_code_symbol ;
  int next_fwdref ;

  pslSymbol         symtab [ MAX_SYMBOL ] ;
  pslSymbol    code_symtab [ MAX_SYMBOL ] ;
  pslFwdRef    forward_ref [ MAX_SYMBOL ] ;

  int locality_stack [ MAX_NESTING ] ;
  int locality_sp ;

  pslOpcode    *code       ;
  pslContext   *context    ;
  pslExtension *extensions ;

  void fixup                  ( const char *s, pslAddress v ) ;
  void addFwdRef              ( const char *s, pslAddress where ) ;
  void checkUnresolvedSymbols () ;

  void pushLocality ()
  {
    if ( locality_sp >= MAX_NESTING-1 )
      ulSetError ( UL_WARNING, "PSL: Too many nested {}'s" ) ;
    else
      locality_stack [ locality_sp++ ] = next_var ;
  }

  void popLocality  ()
  {
    if ( locality_sp <= 0 )
      ulSetError ( UL_FATAL, "PSL: Locality stack underflow !!" ) ;

    /* Delete any local symbols */

    for ( int i = locality_stack [ locality_sp-1 ] ;
              i < next_var ; i++ )
    {
      delete [] symtab [ i ] . symbol ;
      symtab [ i ] . symbol = NULL ;
    }

    /* Put the next_var pointer back where it belongs */

    next_var = locality_stack [ --locality_sp ] ;
  }

public:

  pslCompiler ( pslOpcode *_code, pslExtension *_extn )
  {
    code       = _code ;
    extensions = _extn ;

    for ( int i = 0 ; i < MAX_SYMBOL ; i++ )
    {
      symtab      [ i ] . symbol = NULL ;
      forward_ref [ i ] . symbol = NULL ;
      code_symtab [ i ] . symbol = NULL ;
    }

    init () ;
  }

  ~pslCompiler ()
  {
    for ( int i = 0 ; i < MAX_SYMBOL ; i++ )
    {
      delete [] symtab      [ i ] . symbol ;
      delete [] code_symtab [ i ] . symbol ;
      delete [] forward_ref [ i ] . symbol ;
    }
  }

  pslExtension *getExtensions () const { return extensions ; }

  int printInstruction ( FILE *fd, int addr ) const ;

  void init () 
  {
    int i ;

    for ( i = 0 ; i < MAX_CODE   ; i++ ) code   [ i ] = OPCODE_HALT ; 

    for ( i = 0 ; i < MAX_SYMBOL ; i++ )
    {
      delete [] symtab      [ i ] . symbol ; symtab      [ i ] . symbol = NULL ;
      delete [] code_symtab [ i ] . symbol ; code_symtab [ i ] . symbol = NULL ;
      delete [] forward_ref [ i ] . symbol ; forward_ref [ i ] . symbol = NULL ;
    }

    locality_sp = 0 ;
    next_fwdref = 0 ;
    next_label = 0 ;
    next_code_symbol = 0 ;
    next_code  = 0 ;
    next_var   = 0 ;
  }

  void dump () const ;
  int  compile ( const char *fname ) ;
  int  compile ( FILE *fd ) ;
} ;


