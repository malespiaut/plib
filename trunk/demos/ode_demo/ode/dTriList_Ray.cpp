#include "dxTriList.h"

#ifdef RAYCOLLIDER

#include "ode/dRay.h"

int dCollideRTL(dxGeom* TriList, dxGeom* RayGeom, int Flags, dContactGeom* Contacts, int Stride){
	dxTriListData* TLData = GetTLData(TriList);

	const dVector3& TLPosition = *(const dVector3*)dGeomGetPosition(TriList);
	const dMatrix3& TLRotation = *(const dMatrix3*)dGeomGetRotation(TriList);

	RayCollider& Collider = TLData->RayCollider;

	dReal Length = dGeomRayGetLength(RayGeom);

	Collider.SetCulling(false);
	Collider.SetMaxDist(Length);

	dVector3 Origin, Direction;
	dGeomRayGet(RayGeom, Origin, Direction);

	/* Make Ray */
	Ray WorldRay;
	WorldRay.mOrig.x = Origin[0];
	WorldRay.mOrig.y = Origin[1];
	WorldRay.mOrig.z = Origin[2];
	WorldRay.mDir.x = Direction[0];
	WorldRay.mDir.y = Direction[1];
	WorldRay.mDir.z = Direction[2];

	/* Intersect */
	Collider.Collide(WorldRay, &TLData->BVTree, &MakeMatrix(TLPosition, TLRotation, Matrix4x4()));

	/* Retrieve data */
	int TriCount = TLData->Faces.GetNbFaces();
	
	if (TriCount != 0){
		const CollisionFace* Faces = TLData->Faces.GetFaces();

		int OutTriCount = 0;
		for (int i = 0; i < TriCount; i++){
			if (TLData->RayCallback == null || TLData->RayCallback(TriList, RayGeom, Faces[i].mFaceID, Faces[i].mU, Faces[i].mV)){
				const int& TriIndex = Faces[i].mFaceID;
				dContactGeom* Contact = CONTACT(Flags, Contacts, OutTriCount, Stride);

				dVector3 Temp;
				dGeomTriListGetPoint(TriList, TriIndex, Faces[i].mU, Faces[i].mV, Temp);
				
				dMULTIPLY0_331(Contact->pos, TLRotation, Temp);
				Contact->pos[0] += TLPosition[0];
				Contact->pos[1] += TLPosition[1];
				Contact->pos[2] += TLPosition[2];
				Contact->pos[3] = REAL(0.0);

				dVector3 dv[3];
				FetchVertex(TLData, TriIndex, 0, TLPosition, TLRotation, dv[0]);
				FetchVertex(TLData, TriIndex, 1, TLPosition, TLRotation, dv[1]);
				FetchVertex(TLData, TriIndex, 2, TLPosition, TLRotation, dv[2]);

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

				dCROSS(Contact->normal, =, vv, vu);	// Reversed

				float T = -(dDOT(Contact->normal, Origin) - dDOT(Contact->normal, dv[0])) / dDOT(Contact->normal, Direction);

				dNormalize3(Contact->normal);

				Contact->depth = Length - T;
				Contact->g1 = TriList;
				Contact->g2 = RayGeom;
				
				OutTriCount++;
			}
		}
		return OutTriCount;
	}
	else return 0;
}

#endif	//RAYCOLLIDER