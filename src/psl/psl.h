
#include <stdio.h>

enum PSL_Result
{
  PSL_PROGRAM_END,
  PSL_PROGRAM_PAUSE,
  PSL_PROGRAM_CONTINUE
} ;


typedef unsigned char PSL_Opcode ;
class PSL_Context ;
class PSL_Parser  ;
class PSL_Program ;


class PSL_Extension
{
public:
  char *symbol ;
  int   argc ;
  float (*func) ( int, float *, PSL_Program *p ) ;
} ;



class PSL_Program
{
  PSL_Opcode     *code       ;
  PSL_Context    *context    ;
  PSL_Parser     *parser     ;
  PSL_Extension  *extensions ;

  void *userData ;

public:

   PSL_Program ( PSL_Extension *ext ) ;
   PSL_Program ( PSL_Program *src ) ;

  ~PSL_Program () ;

  PSL_Context   *getContext     () { return context    ; }
  PSL_Opcode    *getCode        () { return code       ; }
  PSL_Parser    *getParser      () { return parser     ; }
  PSL_Extension *getExtensions  () { return extensions ; }

  void      *getUserData ()           { return userData ; }
  void       setUserData ( void *ud ) { userData = ud ; }

  void       dump  () ;
  int        parse ( char *fname ) ;
  int        parse ( FILE *fd    ) ;
  void       reset () ;
  PSL_Result step  () ;
} ;



