

#include "pslLocal.h"

PSL_Program::PSL_Program ( PSL_Extension *ext )
{
  code = new PSL_Opcode [ MAX_CODE ] ;

  extensions = ext ;

  parser  = new PSL_Parser  ( code, ext ) ;
  context = new PSL_Context ( this ) ;

  parser  -> init  () ;
  context -> reset () ;
}
 

PSL_Program::PSL_Program ( PSL_Program *src )
{
  code       = src -> getCode       () ;
  parser     = src -> getParser     () ;
  extensions = src -> getExtensions () ;
  userData   = src -> getUserData   () ;

  context = new PSL_Context ( this ) ;
  context -> reset () ;
}
 

PSL_Program::~PSL_Program ()
{
  delete parser ;
  delete context ;
  delete [] code ;
}


void       PSL_Program::dump  ()             {        parser -> dump () ; }
int        PSL_Program::parse ( char *fname ){ return parser -> parse(fname) ; }
int        PSL_Program::parse ( FILE *fd )   { return parser -> parse( fd  ) ; }

void       PSL_Program::reset ()             {        context -> reset () ; }
PSL_Result PSL_Program::step  ()             { return context -> step  () ; }                           

