
#include "pslPrivate.h"

int PSL_Parser::parse ( char *fname )
{
  init () ;

  FILE *fd = fopen ( fname, "ra" ) ;

  if ( fd == NULL )
  {
#ifdef SHOUT_ABOUT_PSL_ERRORS
    perror ( "PSL:" ) ;
    fprintf ( stderr, "PSL: Failed while opening '%s' for reading.\n", fname );
#endif
    return FALSE ;
  }

  parse  ( fd ) ;
  fclose ( fd ) ;
  return TRUE ;
}


int PSL_Parser::parse ( FILE *fd )
{
  setDefaultFile ( fd ) ;
  pushProgram () ;
  return TRUE ;
}


