
#include "ssgAux.h"

static ssgBase *createCube ()      { return new ssgaCube ( 0 )     ; }
static ssgBase *createSphere ()    { return new ssgaSphere ( 0 )   ; }
static ssgBase *createCylinder ()  { return new ssgaCylinder ( 0 ) ; }

void ssgaInit ()
{
  ssgRegisterType ( ssgaTypeCube ()      , createCube       ) ;
  ssgRegisterType ( ssgaTypeSphere ()    , createSphere     ) ;
  ssgRegisterType ( ssgaTypeCylinder ()  , createCylinder   ) ;
}
