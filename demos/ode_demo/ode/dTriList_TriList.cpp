#include "dxTriList.h"

int dCollideTLTL(dxGeom* TriList0, dxGeom* TriList1, int Flags, dContactGeom* Contacts, int Stride){
#if 0
	dxTriListData* TLData0 = GetTLData(TriList0);
	dxTriListData* TLData1 = GetTLData(TriList1);

	AABBTreeCollider& Collider = TLData0->AABBTreeCollider;

	Collider.SetCallback1((OPC_CALLBACK)dxTriListData::FetchTriangleCB, (udword)TLData1);

	BVTCache Cache;
	Cache.Model0 = &TLData0->BVTree;
	Cache.Model1 = &TLData1->BVTree;
	Collider.Collide(Cache, &MakeMatrix(TriList0, Matrix4x4()), &MakeMatrix(TriList1, Matrix4x4()));

	int TriCount = Collider.GetNbPairs();
	if (TriCount != 0){
		const Pair* Pairs = Collider.GetPairs();
		for (int i = 0; i < TriCount; i++){
			int TriIndex0 = Pairs[i].id0;
			int TriIndex1 = Pairs[i].id1;
		}

		return 0;
	}
	else return 0;
#else
  return 0;
#endif
}
