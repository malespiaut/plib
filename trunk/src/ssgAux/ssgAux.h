
#ifndef _SSGAUX_H_
#define _SSGAUX_H_ 1

#include "sg.h"
#include "ssg.h"
#include "ssgaShapes.h"


#define SSGA_TYPE_SHAPE     0x00008000
#define SSGA_TYPE_CUBE      0x00004000
#define SSGA_TYPE_SPHERE    0x00002000
#define SSGA_TYPE_CYLINDER  0x00001000

inline int ssgaTypeShape   () { return SSGA_TYPE_SHAPE    | ssgTypeBranch () ; }
inline int ssgaTypeCube    () { return SSGA_TYPE_CUBE     | ssgaTypeShape () ; }
inline int ssgaTypeSphere  () { return SSGA_TYPE_SPHERE   | ssgaTypeShape () ; }
inline int ssgaTypeCylinder() { return SSGA_TYPE_CYLINDER | ssgaTypeShape () ; }

void ssgaInit () ;

#endif

