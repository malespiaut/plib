
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


class PSL_Extension
{
public:
  char *symbol ;
  int   argc ;
  float (*func) ( int, float * ) ;
} ;



class PSL_Program
{
  PSL_Opcode  *code    ;
  PSL_Context *context ;
  PSL_Parser  *parser  ;

public:

   PSL_Program ( PSL_Extension *ext ) ;
  ~PSL_Program () ;
  void       dump  () ;
  int        parse ( char *fname ) ;
  int        parse ( FILE *fd    ) ;
  void       reset () ;
  PSL_Result step  () ;
} ;



