// Ripped from Magic Software

#include "ode/dRay.h"
#include "dxRay.h"

static int Clip(dReal Denom, dReal Numer, dReal T[2]){
    if (Denom > REAL(0.0)){
		if (Numer > Denom * T[1]){
            return -1;
		}

        if (Numer > Denom * T[0]){
            T[0] = Numer / Denom;
			return 0;
		}
        return 3;
    }
    else if (Denom < REAL(0.0)){
        if (Numer > Denom * T[0]){
            return -1;
		}

        if (Numer > Denom * T[1]){
            T[1] = Numer / Denom;
			return 1;
		}
        return 3;
    }
    else if (Numer <= REAL(0.0)){
		return 3;
	}
  else return -1;
}

static bool FindIntersection(const dVector3 Origin, const dVector3 Direction, const dVector3 Extents, dReal T[2], int I[2]){
	int PlaneCounter = 0;

	for (dReal Dir = REAL(-1.0); Dir <= REAL(1.0); Dir += REAL(2.0)){
		for (int Side = 0; Side < 3; Side++, PlaneCounter++){
			int Value = Clip(Direction[Side] * Dir, (Origin[Side] * -Dir) - Extents[Side], T);
			if (Value == -1){
				return false;
			}
			else if (Value < 2){
				I[Value] = PlaneCounter;
			}
		}
	}
    return I[0] != -1 || I[1] != -1;
}

int dCollideBR(dxGeom* RayGeom, dxGeom* BoxGeom, int Flags, dContactGeom* Contacts, int Stride){
	const dVector3& Position = *(const dVector3*)dGeomGetPosition(BoxGeom);
	const dMatrix3& Rotation = *(const dMatrix3*)dGeomGetRotation(BoxGeom);
	dVector3 Extents;
	dGeomBoxGetLengths(BoxGeom, Extents);
	Extents[0] /= 2;
	Extents[1] /= 2;
	Extents[2] /= 2;
	Extents[3] /= 2;

	dVector3 Origin, Direction;
	dGeomRayGet(RayGeom, Origin, Direction);
	dReal Length = dGeomRayGetLength(RayGeom);

	dVector3 Diff;
	Diff[0] = Origin[0] - Position[0];
	Diff[1] = Origin[1] - Position[1];
	Diff[2] = Origin[2] - Position[2];
	Diff[3] = Origin[3] - Position[3];

	Direction[0] *= Length;
	Direction[1] *= Length;
	Direction[2] *= Length;
	Direction[3] *= Length;

	dVector3 Axis[3];
	Decompose(Rotation, Axis);

	dVector3 TransOrigin;
	TransOrigin[0] = dDOT(Diff, Axis[0]);
	TransOrigin[1] = dDOT(Diff, Axis[1]);
	TransOrigin[2] = dDOT(Diff, Axis[2]);
	TransOrigin[3] = REAL(0.0);

	dVector3 TransDirection;
	TransDirection[0] = dDOT(Direction, Axis[0]);
	TransDirection[1] = dDOT(Direction, Axis[1]);
	TransDirection[2] = dDOT(Direction, Axis[2]);
	TransDirection[3] = REAL(0.0);

	dReal T[2];
	T[0] = 0.0f;
	T[1] = dInfinity;

	int I[2];
	I[0] = -1;
	I[1] = -1;
	
	bool Intersect = FindIntersection(TransOrigin, TransDirection, Extents, T, I);

	if (Intersect){
#ifdef FINDNORMALS
		dVector3 Normals[2];
		int PlaneCounter = 0;
		
		for (dReal Dir = REAL(-1.0); Dir <= REAL(1.0); Dir += REAL(2.0)){
			for (int Side = 0; Side < 3; Side++, PlaneCounter++){
				dVector3* NormalPtr;
				if (I[0] == PlaneCounter){
					NormalPtr = &Normals[0];
				}
				else if (I[1] == PlaneCounter){
					NormalPtr = &Normals[1];
				}
				else continue;

				dVector3& Normal = *NormalPtr;

				Normal[0] = Dir * Axis[Side][0];
				Normal[1] = Dir * Axis[Side][1];
				Normal[2] = Dir * Axis[Side][2];
				Normal[3] = REAL(0.0);
				dNormalize3(Normal);
			}
		}
#endif	//FINDNORMALS

		if (T[0] > REAL(0.0)){
			dContactGeom* Contact0 = CONTACT(Flags, Contacts, 0, Stride);
			Contact0->pos[0] = Origin[0] + T[0] * Direction[0];
			Contact0->pos[1] = Origin[1] + T[0] * Direction[1];
			Contact0->pos[2] = Origin[2] + T[0] * Direction[2];
			Contact0->pos[3] = Origin[3] + T[0] * Direction[3];

#ifdef FINDNORMALS
			Contact0->normal[0] = Normals[0][0];
			Contact0->normal[1] = Normals[0][1];
			Contact0->normal[2] = Normals[0][2];
			Contact0->normal[3] = Normals[0][3];
#endif	//FINDNORMALS
			Contact0->depth = (REAL(1.0) - T[0]) * Length;
			Contact0->g1 = RayGeom;
			Contact0->g2 = BoxGeom;

			dContactGeom* Contact1 = CONTACT(Flags, Contacts, 1, Stride);
			Contact1->pos[0] = Origin[0] + T[1] * Direction[0];
			Contact1->pos[1] = Origin[1] + T[1] * Direction[1];
			Contact1->pos[2] = Origin[2] + T[1] * Direction[2];
			Contact1->pos[3] = Origin[3] + T[1] * Direction[3];

#ifdef FINDNORMALS
			Contact1->normal[0] = Normals[1][0];
			Contact1->normal[1] = Normals[1][1];
			Contact1->normal[2] = Normals[1][2];
			Contact1->normal[3] = Normals[1][3];
#endif	//FINDNORMALS
			Contact1->depth = (REAL(1.0) - T[1]) * Length;
			Contact1->g1 = RayGeom;
			Contact1->g2 = BoxGeom;

			return 2;
		}
		else{
			dContactGeom* Contact = CONTACT(Flags, Contacts, 0, Stride);
			Contact->pos[0] = Origin[0] + T[1] * Direction[0];
			Contact->pos[1] = Origin[1] + T[1] * Direction[1];
			Contact->pos[2] = Origin[2] + T[1] * Direction[2];
			Contact->pos[3] = Origin[3] + T[1] * Direction[3];

#ifdef FINDNORMALS
			Contact->normal[0] = Normals[1][0];
			Contact->normal[1] = Normals[1][1];
			Contact->normal[2] = Normals[1][2];
			Contact->normal[3] = Normals[1][3];
#endif	//FINDNORMALS
			Contact->depth = (REAL(1.0) - T[1]) * Length;
			Contact->g1 = RayGeom;
			Contact->g2 = BoxGeom;

			return 1;
		}
	}
	else return 0;
}