
#include "ssgAux.h"

static ssgBase *createCube ()      { return new ssgaCube     ; }
static ssgBase *createSphere ()    { return new ssgaSphere   ; }
static ssgBase *createCylinder ()  { return new ssgaCylinder ; }

void ssgaInit ()
{
  ssgRegisterType ( ssgaTypeCube ()      , createCube       ) ;
  ssgRegisterType ( ssgaTypeSphere ()    , createSphere     ) ;
  ssgRegisterType ( ssgaTypeCylinder ()  , createCylinder   ) ;
}
