#include "ode\ode.h"

/* Class ID */
extern int dTriListClass;

/* Single precision, no padding vector used for storage */
struct dcVector3{
	float x, y, z;
};

/* Per triangle callback */
typedef int dTriCallback(dGeomID TriList, dGeomID RefObject, int TriangleIndex);
void dGeomTriListSetCallback(dGeomID g, dTriCallback* Callback);
dTriCallback* dGeomTriListGetCallback(dGeomID g);

/* Per object callback */
typedef void dTriArrayCallback(dGeomID TriList, dGeomID RefObject, const int* TriIndices, int TriCount);
void dGeomTriListSetArrayCallback(dGeomID g, dTriArrayCallback* ArrayCallback);
dTriArrayCallback* dGeomTriListGetArrayCallback(dGeomID g);

/* Ray callback */
typedef int dTriRayCallback(dGeomID TriList, dGeomID Ray, int TriangleIndex, dReal u, dReal v);
void dGeomTriListSetRayCallback(dGeomID g, dTriRayCallback* Callback);
dTriRayCallback* dGeomTriListGetRayCallback(dGeomID g);

/* Construction */
dxGeom* dCreateTriList(dSpaceID space, dTriCallback* Callback, dTriArrayCallback* ArrayCallback, dTriRayCallback* RayCallback);

/* Setting data */
void dGeomTriListBuild(dGeomID g, const dcVector3* Vertices, int VertexStide, int VertexCount, const int* Indices, int IndexStride, int IndexCount, int TriStride);

/* Getting data */
void dGeomTriListGetTriangle(dGeomID g, int Index, dVector3* v0, dVector3* v1, dVector3* v2);
void dGeomTriListGetVertex(dGeomID g, int Vertex, dVector3 v);
void dGeomTriListGetPoint(dGeomID g, int Index, dReal u, dReal v, dVector3 Out);

/*struct StridedVertex{
	dcVector3 Vertex;
	// Userdata
};

int VertexStride = sizeof(StridedVertex);

struct StridedTri{
	struct StridedIndex{
		int Index;
		// Userdata
	};
	StridedIndex Indices[3];
	// Userdata
};

int IndexStride = sizeof(StridedIndex);
int TriStride = sizeof(StridedTri);*/

