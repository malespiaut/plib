#include "ode/dRay.h"
#include "dxTriList.h"

int dTriListClass = -1;

void dAABBTL(dxGeom* TriList, dReal AABB[6]){
	// Warning! This sometimes appears to crash the hashspace.
	//dInfiniteAABB(TriList, AABB);
	//return;
	
	const dVector3& Position = *(const dVector3*)dGeomGetPosition(TriList);
	const dMatrix3& Rotation = *(const dMatrix3*)dGeomGetRotation(TriList);

	dxTriListData* Data = GetTLData(TriList);

	dVector3 Center;
	dMULTIPLY0_331(Center, Rotation, Data->Center);

	dReal xrange = dFabs(Rotation[0] * Data->Extents[0]) + dFabs(Rotation[1] * Data->Extents[1]) + dFabs(Rotation[2] * Data->Extents[2]);
	dReal yrange = dFabs(Rotation[4] * Data->Extents[0]) + dFabs(Rotation[5] * Data->Extents[1]) + dFabs(Rotation[6] * Data->Extents[2]);
	dReal zrange = dFabs(Rotation[8] * Data->Extents[0]) + dFabs(Rotation[9] * Data->Extents[1]) + dFabs(Rotation[10] * Data->Extents[2]);

	AABB[0] = Center[0] + Position[0] - xrange;
	AABB[1] = Center[0] + Position[0] + xrange;
	AABB[2] = Center[1] + Position[1] - yrange;
	AABB[3] = Center[1] + Position[1] + yrange;
	AABB[4] = Center[2] + Position[2] - zrange;
	AABB[5] = Center[2] + Position[2] + zrange;

	// It might be useful to use the convex hull to compute the minimum AABB.
}

dColliderFn* dTriListColliderFn(int num){
	if (num == dPlaneClass) return (dColliderFn*)&dCollidePTL;	// Experimental
	if (num == dSphereClass) return (dColliderFn*)&dCollideSTL;	// Works great
	if (num == dBoxClass) return (dColliderFn*)&dCollideBTL;	// Works reasonably
	//if (num == dCCylinderClass) return (dColliderFn*)&dCollideCCTL;	// Not implemented yet
#ifdef RAYCOLLIDER
	if (num == dRayClass) return (dColliderFn*)&dCollideRTL;	// Works great
#endif	//RAYCOLLIDER
	//if (num == dTriListClass) return (dColliderFn*)&dCollideTLTL;	// Not implemented
	
	return 0;
}

void dDestroyTriList(dGeomID g){
	delete GetTLData(g);
}

/* External functions */
void dGeomTriListSetCallback(dGeomID g, dTriCallback* Callback){
	dxTriListData* Data = GetTLData(g);
	Data->Callback = Callback;
}

dTriCallback* dGeomTriListGetCallback(dGeomID g){
	dxTriListData* Data = GetTLData(g);
	return Data->Callback;
}

void dGeomTriListSetArrayCallback(dGeomID g, dTriArrayCallback* ArrayCallback){
	dxTriListData* Data = GetTLData(g);
	Data->ArrayCallback = ArrayCallback;
}

dTriArrayCallback* dGeomTriListGetArrayCallback(dGeomID g){
	dxTriListData* Data = GetTLData(g);
	return Data->ArrayCallback;
}

void dGeomTriListSetRayCallback(dGeomID g, dTriRayCallback* Callback){
	dxTriListData* Data = GetTLData(g);
	Data->RayCallback = Callback;
}

dTriRayCallback* dGeomTriListGetRayCallback(dGeomID g){
	dxTriListData* Data = GetTLData(g);
	return Data->RayCallback;
}

dxGeom* dCreateTriList(dSpaceID space, dTriCallback* Callback, dTriArrayCallback* ArrayCallback, dTriRayCallback* RayCallback){
	if (dTriListClass == -1){
		dGeomClass c;
		c.bytes = sizeof(dxTriList);	// Userdata is a pointer to the data we need so we can call the constructor
		c.collider = &dTriListColliderFn;
		c.aabb = &dAABBTL;
		c.aabb_test = 0;
		c.dtor = &dDestroyTriList;

		dTriListClass = dCreateGeomClass(&c);

		dxTriListData::Identity.Identity();	// Static variable
	}

	dxGeom* g = dCreateGeom(dTriListClass);
	if (space) dSpaceAdd(space, g);

	GetTL(g)->Data = new dxTriListData();
	dxTriListData* Data = GetTLData(g);

	Data->Callback = Callback;
	Data->ArrayCallback = ArrayCallback;
	Data->RayCallback = RayCallback;

	Data->PlanesCollider.SetCallback((OPC_CALLBACK)dxTriListData::FetchTriangleCB, (udword)Data);
	Data->SphereCollider.SetCallback((OPC_CALLBACK)dxTriListData::FetchTriangleCB, (udword)Data);
	Data->OBBCollider.SetCallback((OPC_CALLBACK)dxTriListData::FetchTriangleCB, (udword)Data);
#ifdef RAYCOLLIDER
	Data->RayCollider.SetCallback((OPC_CALLBACK)dxTriListData::FetchTriangleCB, (udword)Data);
#endif	//RAYCOLLIDER
	Data->AABBTreeCollider.SetCallback0((OPC_CALLBACK)dxTriListData::FetchTriangleCB, (udword)Data);

#ifdef RAYCOLLIDER
	Data->RayCollider.SetDestination(&dxTriListData::Faces);
#endif	//RAYCOLLIDER
	return g;
}

void dGeomTriListBuild(dGeomID g, const dcVector3* Vertices, int VertexStride, int VertexCount, const int* Indices, int IndexStride, int IndexCount, int TriStride){
	dxTriListData* Data = GetTLData(g);
	Data->Vertices = Vertices;
	Data->VertexStride = VertexStride;

	Data->Indices = Indices;
	Data->IndexStride = IndexStride;
	Data->TriStride = TriStride;

	dcVector3* TempVertices;
	if (VertexStride != sizeof(dcVector3)){	// We must generate a temporary vertexlist to match Opcode.
		TempVertices = (dcVector3*)dALLOCA16(VertexCount * sizeof(dcVector3));
		for (int i = 0; i < VertexCount; i++){
			TempVertices[i] = FetchVertex(Data, i);
		}
	}
	else TempVertices = (dcVector3*)Data->Vertices;	// We can simply pass the user's data

	// Compute boundingbox
	// Cant we get this from Opcode??
	dVector3 Min;
	Min[0] = TempVertices[0].x;
	Min[1] = TempVertices[0].y;
	Min[2] = TempVertices[0].z;
	Min[3] = REAL(0.0);

	dVector3 Max;
	Max[0] = TempVertices[0].x;
	Max[1] = TempVertices[0].y;
	Max[2] = TempVertices[0].z;
	Max[3] = REAL(0.0);

	for (int i = 1; i < VertexCount; i++){
		for (int j = 0; j < 3; j++){
			Min[j] = dcMIN(Min[j], ((float*)&TempVertices[i])[j]);
			Max[j] = dcMAX(Max[j], ((float*)&TempVertices[i])[j]);
		}
	}

	Data->Center[0] = (Min[0] + Max[0]) / REAL(2.0);
	Data->Center[1] = (Min[1] + Max[1]) / REAL(2.0);
	Data->Center[2] = (Min[2] + Max[2]) / REAL(2.0);
	Data->Center[3] = (Min[3] + Max[3]) / REAL(2.0);

	Data->Extents[0] = Max[0] - Data->Center[0];
	Data->Extents[1] = Max[1] - Data->Center[1];
	Data->Extents[2] = Max[2] - Data->Center[2];
	Data->Extents[3] = Max[3] - Data->Center[3];

	// Build tree
	OPCODECREATE TreeBuilder;
	TreeBuilder.NbTris = IndexCount / 3;
	TreeBuilder.NbVerts	= VertexCount;
	TreeBuilder.Tris = (unsigned int*)Indices;
	TreeBuilder.Verts = (Point*)TempVertices;

	TreeBuilder.Rules = SPLIT_COMPLETE | SPLIT_SPLATTERPOINTS;
	TreeBuilder.NoLeaf = true;
	TreeBuilder.Quantized = false;
	TreeBuilder.KeepOriginal = true;
	Data->BVTree.Build(TreeBuilder);
}

void dGeomTriListGetTriangle(dGeomID g, int Index, dVector3* v0, dVector3* v1, dVector3* v2){
	dxTriListData* Data = GetTLData(g);
	if (v0){
		const dcVector3& v = FetchVertex(Data, Index, 0);
		(*v0)[0] = v.x;
		(*v0)[1] = v.y;
		(*v0)[2] = v.z;
		(*v0)[3] = REAL(0.0);
	}
	if (v1){
		const dcVector3& v = FetchVertex(Data, Index, 1);
		(*v1)[0] = v.x;
		(*v1)[1] = v.y;
		(*v1)[2] = v.z;
		(*v1)[3] = REAL(0.0);
	}
	if (v2){
		const dcVector3& v = FetchVertex(Data, Index, 2);
		(*v2)[0] = v.x;
		(*v2)[1] = v.y;
		(*v2)[2] = v.z;
		(*v2)[3] = REAL(0.0);
	}
}

void dGeomTriListGetVertex(dGeomID g, int Vertex, dVector3 dv){
	dxTriListData* Data = GetTLData(g);

	const dcVector3& v = FetchVertex(Data, Vertex);
	dv[0] = v.x;
	dv[1] = v.y;
	dv[2] = v.z;
	dv[3] = REAL(0.0);
}

void dGeomTriListGetPoint(dGeomID g, int Index, dReal u, dReal v, dVector3 Out){
	dVector3 v0, v1, v2;
	dGeomTriListGetTriangle(g, Index, &v0, &v1, &v2);

	dReal w = REAL(1.0) - u - v;

	Out[0] = (v0[0] * w) + (v1[0] * u) + (v2[0] * v);
	Out[1] = (v0[1] * w) + (v1[1] * u) + (v2[1] * v);
	Out[2] = (v0[2] * w) + (v1[2] * u) + (v2[2] * v);
	Out[3] = REAL(0.0);
}

Matrix4x4 dxTriListData::Identity;
#ifdef RAYCOLLIDER
CollisionFaces dxTriListData::Faces;
#endif	//RAYCOLLIDER

void dxTriListData::FetchTriangleCB(udword TriIndex, VertexPointers& Triangle, dxTriListData* TLData){
	for (int i = 0; i < 3; i++){
		Triangle.Vertex[i] = (Point*)&FetchVertex(TLData, TriIndex, i);
	}
}