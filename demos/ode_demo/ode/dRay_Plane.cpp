// Ripped from Paul Bourke

#include "ode/dRay.h"
#include "dxRay.h"

int dCollidePR(dxGeom* RayGeom, dxGeom* PlaneGeom, int Flags, dContactGeom* Contact, int Stride){
	dVector3 Plane;
	dGeomPlaneGetParams(PlaneGeom, Plane);

	dVector3 Origin, Direction;
	dGeomRayGet(RayGeom, Origin, Direction);

	dReal Length = dGeomRayGetLength(RayGeom);

	dReal Denom = dDOT(Plane, Direction);
	if (dFabs(Denom) < REAL(0.00001)){
		return 0;	// Ray never hits
	}
	
	float T = -(dDOT(Plane, Origin) + Plane[3]) / Denom;
	
	if (T < 0 || T > Length){
		return 0;	// Ray hits but not within boundaries
	}

	Contact->pos[0] = Origin[0] + T * Direction[0];
	Contact->pos[1] = Origin[1] + T * Direction[1];
	Contact->pos[2] = Origin[2] + T * Direction[2];
	Contact->pos[3] = REAL(0.0);

#ifdef FINDNORMALS
	Contact->normal[0] = Plane[0];
	Contact->normal[1] = Plane[1];
	Contact->normal[2] = Plane[2];
	Contact->normal[3] = REAL(0.0);
#endif	//FINDNORMALS

	Contact->depth = Length - T;
	Contact->g1 = RayGeom;
	Contact->g2 = PlaneGeom;
	return 1;
}