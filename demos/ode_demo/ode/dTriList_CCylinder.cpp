#include "dxTriList.h"

int dCollideCCTL(dxGeom* TriList, dxGeom* CCylinder, int Flags, dContactGeom* Contact, int Stride){
#if 0
	dxTriList* TL = GetTL(TriList);
	dxTriListData* TLData = GetTLData(TL);

	const dVector3& TLPosition = *(const dVector3*)dGeomGetPosition(TriList);
	const dMatrix3& TLRotation = *(const dMatrix3*)dGeomGetRotation(TriList);

	OBBCollider& Collider = TLData->OBBCollider;

	// Get box
	const dVector3& CCCenter = *(const dVector3*)dGeomGetPosition(CCylinder);
	const dMatrix3& CCRotation = *(const dMatrix3*)dGeomGetRotation(CCylinder);

	dReal Radius, Length;
	dGeomCCylinderGetParams(CCylinder, &Radius, &Length);

	dVector3 CCExtents;
	CCExtents[0] = Radius;
	CCExtents[1] = Radius;
	CCExtents[2] = Length;
	CCExtents[3] = REAL(0.0);

	// Make OBB
 	OBB Box;
	Box.mCenter.x = CCCenter[0];
	Box.mCenter.y = CCCenter[1];
	Box.mCenter.z = CCCenter[2];

	Box.mExtents.x = CCExtents[0];
	Box.mExtents.y = CCExtents[1];
	Box.mExtents.z = CCExtents[2];

	Box.mRot2.m[0][0] = CCRotation[0];
	Box.mRot2.m[1][0] = CCRotation[1];
	Box.mRot2.m[2][0] = CCRotation[2];

	Box.mRot2.m[0][1] = CCRotation[4];
	Box.mRot2.m[1][1] = CCRotation[5];
	Box.mRot2.m[2][1] = CCRotation[6];

	Box.mRot2.m[0][2] = CCRotation[8];
	Box.mRot2.m[1][2] = CCRotation[9];
	Box.mRot2.m[2][2] = CCRotation[10];

	// Intersect
	OBBCache Cache;
	Collider.Collide(Cache, Box, &TLData->BVTree, null, &MakeMatrix(TLPosition, TLRotation, Matrix4x4()));
	
	// Retrieve data
	int TriCount = Cache.TouchedPrimitives.GetNbEntries();
	
	if (TriCount != 0){
		const int* Triangles = (const int*)Cache.TouchedPrimitives.GetEntries();

		if (TLData->ArrayCallback != null){
			TLData->ArrayCallback(TriList, CCylinder, Triangles, TriCount);
		}

		int OutTriCount = 0;
		for (int i = 0; i < TriCount; i++){
			const int& TriIndex = Triangles[i];
		}

		return 0;
	}
	else return 0;
#else
  return 0;
#endif
}
