#ifndef __DXTRILIST_INCLUDED__
#define __DXTRILIST_INCLUDED__

#define OPC_USE_CALLBACKS
#include "OPCODE\Opcode.h"
using namespace Opcode;

#include <ode\ode.h>
#include "ode/dTriList.h"

#define RAYCOLLIDER	// Specifies the availability of the raycollider

struct dxTriListData{
	/* Callbacks */
	dTriCallback* Callback;
	dTriArrayCallback* ArrayCallback;
	dTriRayCallback* RayCallback;

	/* Data */
	OPCODE_Model BVTree;

	dVector3 Center, Extents;
	const dcVector3* Vertices;
	int VertexStride;
	const int* Indices;
	int IndexStride;
	int TriStride;

	/* Colliders */
	PlanesCollider PlanesCollider;
	SphereCollider SphereCollider;
	OBBCollider OBBCollider;
#ifdef RAYCOLLIDER
	RayCollider RayCollider;
#endif	//RAYCOLLIDER
	AABBTreeCollider AABBTreeCollider;

	/* Output */
	static Matrix4x4 Identity;
#ifdef RAYCOLLIDER
	static CollisionFaces Faces;
#endif	//RAYCOLLIDER

	static void FetchTriangleCB(udword TriIndex, VertexPointers& Triangle, dxTriListData* TLData);
};

struct dxTriList{
	dxTriListData* Data;
};

// Double precision vector used for precise calculations
typedef double dVector3d[4];

typedef double dVector4d[4];

// Gets the dxTriList*
inline dxTriList* GetTL(dxGeom* TriList){
	return (dxTriList*)dGeomGetClassData(TriList);
}

// Gets the dxTriListData*
inline dxTriListData* GetTLData(dxTriList* TriList){
	return TriList->Data;
}

// Gets the dxTriListData*
inline dxTriListData* GetTLData(dxGeom* TriList){
	return GetTLData(GetTL(TriList));
}

// Fetches a contact
inline dContactGeom* CONTACT(int Flags, dContactGeom* Contacts, int Index, int Stride){
	dIASSERT(Index >= 0 && Index < (Flags & 0x0ffff));
	return ((dContactGeom*)(((char*)Contacts) + (Index * Stride)));
}

// Performs a callback
inline bool Callback(dxGeom* TriList, dxTriListData* TLData, dxGeom* Object, int TriIndex){
	if (TLData->Callback != null){
		return TLData->Callback(TriList, Object, TriIndex) != 0;
	}
	else return true;
}

// Fetches a vertex
inline const dcVector3& FetchVertex(dxTriListData* TLData, int Index){
	return (const dcVector3&)(((char*)TLData->Vertices)[Index * TLData->VertexStride]);
}

// Fetches and transforms a vertex
inline void FetchVertex(dxTriListData* TLData, int Index, const dVector3 Position, const dMatrix3 Rotation, dVector3 Out){
	const dcVector3& In = FetchVertex(TLData, Index);

	dVector3 Temp;
	Temp[0] = In.x;
	Temp[1] = In.y;
	Temp[2] = In.z;
	Temp[3] = REAL(0.0);

	dMULTIPLY0_331(Out, Rotation, Temp);
	Out[0] += Position[0];
	Out[1] += Position[1];
	Out[2] += Position[2];
}

/*inline int FetchIndex(dxTriListData* TLData, int TriIndex){
	return (int&)(((char*)TLData->Indices)[TriIndex * TLData->TriStride]);
}

inline int FetchIndex(dxTriListData* TLData, int TriIndex, int Vertex){
	return (int&)(((char*)TLData->Indices)[TriIndex * TLData->TriStride + Vertex * TLData->IndexStride]);
}*/

// Fetches a vertex
inline const dcVector3& FetchVertex(dxTriListData* TLData, int TriIndex, int Vertex){
	return FetchVertex(TLData, TLData->Indices[TriIndex * 3 + Vertex]);
}

// Fetches and transforms a vertex
inline void FetchVertex(dxTriListData* TLData, int TriIndex, int Vertex, const dVector3 Position, const dMatrix3 Rotation, dVector3 Out){
	FetchVertex(TLData, TLData->Indices[TriIndex * 3 + Vertex], Position, Rotation, Out);
}

// Outputs a matrix to 3 vectors
inline void Decompose(const dMatrix3 Matrix, dVector3 Right, dVector3 Up, dVector3 Direction){
	Right[0] = Matrix[0 * 4 + 0];
	Right[1] = Matrix[1 * 4 + 0];
	Right[2] = Matrix[2 * 4 + 0];
	Right[3] = Matrix[3 * 4 + 0];
	Up[0] = Matrix[0 * 4 + 1];
	Up[1] = Matrix[1 * 4 + 1];
	Up[2] = Matrix[2 * 4 + 1];
	Up[3] = Matrix[3 * 4 + 1];
	Direction[0] = Matrix[0 * 4 + 2];
	Direction[1] = Matrix[1 * 4 + 2];
	Direction[2] = Matrix[2 * 4 + 2];
	Direction[3] = Matrix[3 * 4 + 2];
}

// Outputs a matrix to 3 vectors
inline void Decompose(const dMatrix3 Matrix, dVector3 Vectors[3]){
	Decompose(Matrix, Vectors[0], Vectors[1], Vectors[2]);
}

// Creates an OPCODE matrix from an ODE matrix
inline Matrix4x4& MakeMatrix(const dVector3 Position, const dMatrix3 Rotation, Matrix4x4& Out){
	Out.m[0][0] = Rotation[0];
	Out.m[1][0] = Rotation[1];
	Out.m[2][0] = Rotation[2];

	Out.m[0][1] = Rotation[4];
	Out.m[1][1] = Rotation[5];
	Out.m[2][1] = Rotation[6];

	Out.m[0][2] = Rotation[8];
	Out.m[1][2] = Rotation[9];
	Out.m[2][2] = Rotation[10];

	Out.m[3][0] = Position[0];
	Out.m[3][1] = Position[1];
	Out.m[3][2] = Position[2];

	Out.m[0][3] = 0.0f;
	Out.m[1][3] = 0.0f;
	Out.m[2][3] = 0.0f;
	Out.m[3][3] = 1.0f;

	return Out;
}

// Creates an OPCODE matrix from an ODE matrix
inline Matrix4x4& MakeMatrix(dxGeom* g, Matrix4x4& Out){
	const dVector3& Position = *(const dVector3*)dGeomGetPosition(g);
	const dMatrix3& Rotation = *(const dMatrix3*)dGeomGetRotation(g);
	return MakeMatrix(Position, Rotation, Out);
}

// Our colliders
int dCollidePTL(dxGeom* TriList, dxGeom* PlaneGeom, int Flags, dContactGeom* Contacts, int Stride);
int dCollideSTL(dxGeom* TriList, dxGeom* SphereGeom, int Flags, dContactGeom* Contacts, int Stride);
int dCollideBTL(dxGeom* TriList, dxGeom* Box, int Flags, dContactGeom* Contacts, int Stride);
int dCollideCCTL(dxGeom* TriList, dxGeom* CCylinder, int Flags, dContactGeom* Contacts, int Stride);
#ifdef RAYCOLLIDER
int dCollideRTL(dxGeom* TriList, dxGeom* RayGeom, int Flags, dContactGeom* Contacts, int Stride);
#endif	//RAYCOLLIDER
int dCollideTLTL(dxGeom* TriList0, dxGeom* TriList1, int Flags, dContactGeom* Contacts, int Stride);

// Some utilities
template<class T> const T& dcMAX(const T& x, const T& y){
	return x > y ? x : y;
}

template<class T> const T& dcMIN(const T& x, const T& y){
	return x < y ? x : y;
}

// MSVC fixup.
#define for if (false); else for

#endif	//__DXTRILIST_INCLUDED__