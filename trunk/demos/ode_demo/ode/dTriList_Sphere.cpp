#include "dxTriList.h"

#define MERGECONTACTS	// Merges all contacts into 1 contact
//#define MERGECONTACTNORMALS	// Merges all contact normals

//#define PLANECENTERCULL	// Culls usage of triangles based on the triangle's plane

#define dDOTD(a,b)   ((a)[0]*(b)[0] + (a)[1]*(b)[1] + (a)[2]*(b)[2])	// C version of ODE's dDOT

// Ripped from Opcode 1.1. Uses double precision for stability.
static bool GetContactData(const dVector3& Center, dReal Radius, const dVector3 Origin, const dVector3d Edge0, const dVector3d Edge1, dReal& Dist, double& u, double& v){
	dVector3d Diff;
	Diff[0] = double(Origin[0]) - double(Center[0]);
	Diff[1] = double(Origin[1]) - double(Center[1]);
	Diff[2] = double(Origin[2]) - double(Center[2]);
	Diff[3] = double(Origin[3]) - double(Center[3]);

	double A00 = dDOTD(Edge0, Edge0);
	double A01 = dDOTD(Edge0, Edge1);
	double A11 = dDOTD(Edge1, Edge1);

	double B0 = dDOTD(Diff, Edge0);
	double B1 = dDOTD(Diff, Edge1);

	double C = dDOTD(Diff, Diff);

	double Det = fabs(A00 * A11 - A01 * A01);
	u = A01 * B1 - A11 * B0;
	v = A01 * B0 - A00 * B1;

	double DistSq;

	if (u + v <= Det){
		if(u < 0.0){
			if(v < 0.0){  // region 4
				if(B0 < 0.0){
					v = 0.0;
					if (-B0 >= A00){
						u = 1.0;
						DistSq = A00 + 2.0 * B0 + C;
					}
					else{
						u = -B0 / A00;
						DistSq = B0 * u + C;
					}
				}
				else{
					u = 0.0;
					if(B1 >= 0.0){
						v = 0.0;
						DistSq = C;
					}
					else if(-B1 >= A11){
						v = 1.0;
						DistSq = A11 + 2.0 * B1 + C;
					}
					else{
						v = -B1 / A11;
						DistSq = B1 * v + C;
					}
				}
			}
			else{  // region 3
				u = 0.0;
				if(B1 >= 0.0){
					v = 0.0;
					DistSq = C;
				}
				else if(-B1 >= A11){
					v = 1.0;
					DistSq = A11 + 2.0 * B1 + C;
				}
				else{
					v = -B1 / A11;
					DistSq = B1 * v + C;
				}
			}
		}
		else if(v < 0.0f){  // region 5
			v = 0.0;
			if (B0 >= 0.0){
				u = 0.0;
				DistSq = C;
			}
			else if (-B0 >= A00){
				u = 1.0;
				DistSq = A00 + 2.0 * B0 + C;
			}
			else{
				u = -B0 / A00;
				DistSq = B0 * u + C;
			}
		}
		else{  // region 0
			// minimum at interior point
			if (Det == 0.0){
				u = 0.0;
				v = 0.0;
				DistSq = dInfinity;
			}
			else{
				double InvDet = 1.0 / Det;
				u *= InvDet;
				v *= InvDet;
				DistSq = u * (A00 * u + A01 * v + 2.0 * B0) + v * (A01 * u + A11 * v + 2.0 * B1) + C;
			}
		}
	}
	else{
		double Tmp0, Tmp1, Numer, Denom;

		if(u < 0.0){  // region 2
			Tmp0 = A01 + B0;
			Tmp1 = A11 + B1;
			if (Tmp1 > Tmp0){
				Numer = Tmp1 - Tmp0;
				Denom = A00 - 2.0 * A01 + A11;
				if (Numer >= Denom){
					u = 1.0;
					v = 0.0;
					DistSq = A00 + 2.0 * B0 + C;
				}
				else{
					u = Numer / Denom;
					v = 1.0 - u;
					DistSq = u * (A00 * u + A01 * v + 2.0 * B0) + v * (A01 * u + A11 * v + 2.0 * B1) + C;
				}
			}
			else{
				u = 0.0;
				if(Tmp1 <= 0.0){
					v = 1.0;
					DistSq = A11 + 2.0 * B1 + C;
				}
				else if(B1 >= 0.0){
					v = 0.0;
					DistSq = C;
				}
				else{
					v = -B1 / A11;
					DistSq = B1 * v + C;
				}
			}
		}
		else if(v < 0.0){  // region 6
			Tmp0 = A01 + B1;
			Tmp1 = A00 + B0;
			if (Tmp1 > Tmp0){
				Numer = Tmp1 - Tmp0;
				Denom = A00 - 2.0 * A01 + A11;
				if (Numer >= Denom){
					v = 1.0;
					u = 0.0;
					DistSq = A11 + 2.0 * B1 + C;
				}
				else{
					v = Numer / Denom;
					u = 1.0 - v;
					DistSq =  u * (A00 * u + A01 * v + 2.0 * B0) + v * (A01 * u + A11 * v + 2.0 * B1) + C;
				}
			}
			else{
				v = 0.0;
				if (Tmp1 <= 0.0){
					u = 1.0;
					DistSq = A00 + 2.0 * B0 + C;
				}
				else if(B0 >= 0.0){
					u = 0.0;
					DistSq = C;
				}
				else{
					u = -B0 / A00;
					DistSq = B0 * u + C;
				}
			}
		}
		else{  // region 1
			Numer = A11 + B1 - A01 - B0;
			if (Numer <= 0.0){
				u = 0.0;
				v = 1.0;
				DistSq = A11 + 2.0 * B1 + C;
			}
			else{
				Denom = A00 - 2.0 * A01 + A11;
				if (Numer >= Denom){
					u = 1.0;
					v = 0.0;
					DistSq = A00 + 2.0 * B0 + C;
				}
				else{
					u = Numer / Denom;
					v = 1.0 - u;
					DistSq = u * (A00 * u + A01 * v + 2.0 * B0) + v * (A01 * u + A11 * v + 2.0 * B1) + C;
				}
			}
		}
	}

	Dist = dSqrt(dFabs(DistSq));

	if (Dist <= Radius){
		Dist = Radius - Dist;
		return true;
	}
	else return false;
}

int dCollideSTL(dxGeom* TriList, dxGeom* SphereGeom, int Flags, dContactGeom* Contacts, int Stride){
	dxTriListData* TLData = GetTLData(TriList);

	const dVector3& TLPosition = *(const dVector3*)dGeomGetPosition(TriList);
	const dMatrix3& TLRotation = *(const dMatrix3*)dGeomGetRotation(TriList);

	SphereCollider& Collider = TLData->SphereCollider;

	const dVector3& Position = *(const dVector3*)dGeomGetPosition(SphereGeom);
	dReal Radius = dGeomSphereGetRadius(SphereGeom);

	dVector3d PositionD;
	PositionD[0] = double(Position[0]);
	PositionD[1] = double(Position[1]);
	PositionD[2] = double(Position[2]);
	PositionD[3] = double(Position[3]);

	/* Make Sphere */
	Sphere CollisionSphere;
	CollisionSphere.mCenter.x = Position[0];
	CollisionSphere.mCenter.y = Position[1];
	CollisionSphere.mCenter.z = Position[2];
	CollisionSphere.mRadius = Radius;

	/* Intersect */
  Matrix4x4 mat;
  MakeMatrix(TLPosition, TLRotation, mat);
	SphereCache Cache;
	Collider.Collide(Cache, CollisionSphere, &TLData->BVTree, null, &mat);
	
	/* Retrieve data */
	int TriCount = Cache.TouchedPrimitives.GetNbEntries();
	
	if (TriCount != 0){
		const int* Triangles = (const int*)Cache.TouchedPrimitives.GetEntries();

		if (TLData->ArrayCallback != null){
			TLData->ArrayCallback(TriList, SphereGeom, Triangles, TriCount);
		}

		int OutTriCount = 0;
		for (int i = 0; i < TriCount; i++){
			const int& TriIndex = Triangles[i];

			dVector3 dv[3];
			FetchVertex(TLData, TriIndex, 0, TLPosition, TLRotation, dv[0]);
			FetchVertex(TLData, TriIndex, 1, TLPosition, TLRotation, dv[1]);
			FetchVertex(TLData, TriIndex, 2, TLPosition, TLRotation, dv[2]);

			// Find the best 'origin' vertex. The best one is the one with the minimum difference between the length of the edges
			dReal MinDifSq = dInfinity;
			int MinDifIndex = -1;
			for (int j = 0; j < 3; j++){
				/*dVector3 Diff;
				Diff[0] = dv[j][0] - Position[0];
				Diff[1] = dv[j][1] - Position[1];
				Diff[2] = dv[j][2] - Position[2];
				Diff[3] = REAL(0.0);

				dReal DifSq = dDOT(Diff, Diff);
				if (DifSq < MinDifSq){
					MinDifSq = DifSq;
					MinDifIndex = 0;
				}*/

				dVector3 vu;
				vu[0] = dv[1][0] - dv[0][0];
				vu[1] = dv[1][1] - dv[0][1];
				vu[2] = dv[1][2] - dv[0][2];
				vu[3] = REAL(0.0);

				dVector3 vv;
				vv[0] = dv[2][0] - dv[0][0];
				vv[1] = dv[2][1] - dv[0][1];
				vv[2] = dv[2][2] - dv[0][2];
				vv[3] = REAL(0.0);

				dReal DifSq = dFabs(dDOT(vu, vu) - dDOT(vv, vv));
				if (DifSq < MinDifSq){
					MinDifSq = DifSq;
					MinDifIndex = j;
				}
			}

			dVector3& v0 = dv[MinDifIndex];
			dVector3& v1 = dv[(MinDifIndex + 1) % 3];
			dVector3& v2 = dv[(MinDifIndex + 2) % 3];

			dVector3d vu;
			vu[0] = double(v1[0]) - double(v0[0]);
			vu[1] = double(v1[1]) - double(v0[1]);
			vu[2] = double(v1[2]) - double(v0[2]);
			vu[3] = 0.0;

			dVector3d vv;
			vv[0] = double(v2[0]) - double(v0[0]);
			vv[1] = double(v2[1]) - double(v0[1]);
			vv[2] = double(v2[2]) - double(v0[2]);
			vv[3] = 0.0;

			dReal Depth;
			double u, v;
			if (!GetContactData(Position, Radius, v0, vu, vv, Depth, u, v)){
				continue;	// Sphere doesnt hit triangle
			}
			double w = 1.0 - u - v;

			dContactGeom* Contact = CONTACT(Flags, Contacts, OutTriCount, Stride);

			Contact->pos[0] = dReal((double(v0[0]) * w) + (double(v1[0]) * u) + (double(v2[0]) * v));
			Contact->pos[1] = dReal((double(v0[1]) * w) + (double(v1[1]) * u) + (double(v2[1]) * v));
			Contact->pos[2] = dReal((double(v0[2]) * w) + (double(v1[2]) * u) + (double(v2[2]) * v));
			Contact->pos[3] = REAL(0.0);

			dVector3d dv0;
			dv0[0] = double(v0[0]);
			dv0[1] = double(v0[1]);
			dv0[2] = double(v0[2]);
			dv0[3] = 0.0;

			dVector4d Plane;
			dCROSS(Plane, =, vv, vu);	// Reversed
			Plane[3] = dDOTD(Plane, dv0);	// Using normal as plane.

			double Area = sqrt(dDOTD(Plane, Plane));	// We can use this later
			Plane[0] /= Area;
			Plane[1] /= Area;
			Plane[2] /= Area;
			Plane[3] /= Area;

			Depth = dcMIN(Depth, dReal(dDOTD(Plane, PositionD) - Plane[3] + double(Radius)));

			Contact->normal[0] = dReal(Plane[0]) * Depth;
			Contact->normal[1] = dReal(Plane[1]) * Depth;
			Contact->normal[2] = dReal(Plane[2]) * Depth;
			Contact->normal[3] = REAL(0.0);
			
#ifdef PLANECENTERCULL
			// This is very obsolete now. Remove it?
			if (dDOT(Contact->normal, Position) < dDOT(Contact->pos, Contact->normal)){
				OutTriCount++;
			}
#else	//PLANECENTERCULL
			OutTriCount++;
#endif	//PLANECENTERCULL
		}
#ifdef MERGECONTACTS	// Merge all contacts into 1
		if (OutTriCount != 0){
			dContactGeom* Contact = CONTACT(Flags, Contacts, 0, Stride);
						
			for (int i = 1; i < OutTriCount; i++){
				dContactGeom* TempContact = CONTACT(Flags, Contacts, i, Stride);
				
				Contact->pos[0] += TempContact->pos[0];
				Contact->pos[1] += TempContact->pos[1];
				Contact->pos[2] += TempContact->pos[2];
				Contact->pos[3] += TempContact->pos[3];
				
				Contact->normal[0] += TempContact->normal[0];
				Contact->normal[1] += TempContact->normal[1];
				Contact->normal[2] += TempContact->normal[2];
				Contact->normal[3] += TempContact->normal[3];
			}
			
			Contact->pos[0] /= OutTriCount;
			Contact->pos[1] /= OutTriCount;
			Contact->pos[2] /= OutTriCount;
			Contact->pos[3] /= OutTriCount;
			
			// Remember to divide in square space.
			Contact->depth = dSqrt(dDOT(Contact->normal, Contact->normal) / OutTriCount);

			dNormalize3(Contact->normal);

			Contact->g1 = TriList;
			Contact->g2 = SphereGeom;

			return 1;
		}
		else return 0;
#elif defined MERGECONTACTNORMALS	// Merge all normals, and distribute between all contacts
		if (OutTriCount != 0){
			dVector3& Normal = CONTACT(Flags, Contacts, 0, Stride)->normal;
			CONTACT(Flags, Contacts, 0, Stride)->depth = dSqrt(dDOT(Normal, Normal));	// Length of the normal

			for (int i = 1; i < OutTriCount; i++){
				dContactGeom* Contact = CONTACT(Flags, Contacts, i, Stride);

				Contact->depth = dSqrt(dDOT(Contact->normal, Contact->normal));

				Normal[0] += Contact->normal[0];
				Normal[1] += Contact->normal[1];
				Normal[2] += Contact->normal[2];
				Normal[3] += Contact->normal[3];
			}
			dNormalize3(Normal);

			for (int i = 1; i < OutTriCount; i++){
				dContactGeom* Contact = CONTACT(Flags, Contacts, i, Stride);

				Contact->normal[0] = Normal[0];
				Contact->normal[1] = Normal[1];
				Contact->normal[2] = Normal[2];
				Contact->normal[3] = Normal[3];

				Contact->g1 = TriList;
				Contact->g2 = SphereGeom;
			}
			return OutTriCount;
		}
		else return 0;
#else	//MERGECONTACTNORMALS	// Just gather penetration depths and return
		for (int i = 0; i < OutTriCount; i++){
			dContactGeom* Contact = CONTACT(Flags, Contacts, i, Stride);

			Contact->depth = dSqrt(dDOT(Contact->normal, Contact->normal));

			Contact->normal[0] /= Contact->depth;
			Contact->normal[1] /= Contact->depth;
			Contact->normal[2] /= Contact->depth;
			Contact->normal[3] /= Contact->depth;

			Contact->g1 = TriList;
			Contact->g2 = SphereGeom;
		}

		return OutTriCount;
#endif	// MERGECONTACTS
	}
	else return 0;
}