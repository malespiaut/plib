#include "ode\ode.h"

/* Class ID */
extern int dRayClass;

/* Creates a ray */
dxGeom* dGeomCreateRay(dSpaceID space, dReal Length);

/* Set/Get length */
void dGeomRaySetLength(dxGeom* g, dReal Length);
dReal dGeomRayGetLength(dxGeom* g);

/* Utility function to override the ray's pos + rot */
void dGeomRaySet(dxGeom* g, const dVector3 Origin, const dVector3 Direction);
void dGeomRayGet(dxGeom* g, dVector3 Origin, dVector3 Direction);
