#include "dxTriList.h"

//#define MERGECONTACTS	// Merges all contacts in less contacts

// What we need is a convex set of contacts. It would also be nice to have a simplification for this set.
// Currently the algorithm just generates contacts for each point, which isnt useable at all. Too slow.

int dCollidePTL(dxGeom* TriList, dxGeom* PlaneGeom, int Flags, dContactGeom* Contacts, int Stride){
	dxTriListData* TLData = GetTLData(TriList);

	const dVector3& Position = *(const dVector3*)dGeomGetPosition(TriList);
	const dMatrix3& Rotation = *(const dMatrix3*)dGeomGetRotation(TriList);

	PlanesCollider& Collider = TLData->PlanesCollider;

	dVector4 PlaneData;
	dGeomPlaneGetParams(PlaneGeom, PlaneData);

	PlanesCache Cache;
	Plane Plane;
	Plane.n.x = PlaneData[0];
	Plane.n.y = PlaneData[1];
	Plane.n.z = PlaneData[2];
	Plane.d = PlaneData[3];

	Collider.Collide(Cache, &Plane, 1, &TLData->BVTree, &MakeMatrix(Position, Rotation, Matrix4x4()));

	/* Retrieve data */
	int TriCount = Cache.TouchedPrimitives.GetNbEntries();
	
	if (TriCount != 0){
		const int* Triangles = (const int*)Cache.TouchedPrimitives.GetEntries();

		if (TLData->ArrayCallback != null){
			TLData->ArrayCallback(TriList, PlaneGeom, Triangles, TriCount);
		}

		int OutTriCount = 0;
		for (int i = 0; i < TriCount; i++){
			const int& TriIndex = Triangles[i];

			dContactGeom* Contact = CONTACT(Flags, Contacts, OutTriCount, Stride);	// Reserved contact for this triangle
			Contact->depth = REAL(0.0);

			for (int j = 0; j < 3; j++){
				dVector3 dv;
				FetchVertex(TLData, TriIndex, j, Position, Rotation, dv);
				
				dReal Depth = -(dDOT(PlaneData, dv) - PlaneData[3]);
				if (Depth > REAL(0.0)){
					if (Depth > Contact->depth){
						Contact->pos[0] = dv[0];
						Contact->pos[1] = dv[1];
						Contact->pos[2] = dv[2];
						Contact->pos[3] = dv[3];
						Contact->depth = Depth;
					}
				}
			}
			
			if (Contact->depth > REAL(0.0)){
				int Index;
				for (Index = 0; Index < OutTriCount; Index++){
					dContactGeom* TempContact = CONTACT(Flags, Contacts, i, Stride);

					dVector3 Diff;
					Diff[0] = TempContact->pos[0] - Contact->pos[0];
					Diff[1] = TempContact->pos[1] - Contact->pos[1];
					Diff[2] = TempContact->pos[2] - Contact->pos[2];
					Diff[3] = TempContact->pos[3] - Contact->pos[3];

					dReal DistSq = dDOT(Diff, Diff);

					if (DistSq < REAL(0.01)){
						break;
					}
				}
				if (Index == OutTriCount){
					Contact->normal[0] = PlaneData[0];
					Contact->normal[1] = PlaneData[1];
					Contact->normal[2] = PlaneData[2];
					Contact->normal[3] = REAL(0.0);
					Contact->g1 = TriList;
					Contact->g2 = PlaneGeom;

					OutTriCount++;
				}
			}
		}
#ifdef MERGECONTACTS
		if (OutTriCount != 0){
			if (OutTriCount != 1){
				dContactGeom* Contact = CONTACT(Flags, Contacts, 0, Stride);

				Contact->pos[0] *= Contact->depth;
				Contact->pos[1] *= Contact->depth;
				Contact->pos[2] *= Contact->depth;
				Contact->pos[3] *= Contact->depth;

				for (int i = 1; i < OutTriCount; i++){
					dContactGeom* TempContact = CONTACT(Flags, Contacts, i, Stride);

					Contact->pos[0] += TempContact->pos[0] * TempContact->depth;
					Contact->pos[1] += TempContact->pos[1] * TempContact->depth;
					Contact->pos[2] += TempContact->pos[2] * TempContact->depth;
					Contact->pos[3] += TempContact->pos[3] * TempContact->depth;

					Contact->depth += TempContact->depth;
				}

				Contact->pos[0] /= Contact->depth;
				Contact->pos[1] /= Contact->depth;
				Contact->pos[2] /= Contact->depth;
				Contact->pos[3] /= Contact->depth;

				Contact->depth /= OutTriCount;
			}
			return 1;
		}
		else return 0;
#endif	//MERGECONTACTS
		return OutTriCount;
	}
	else return 0;
}