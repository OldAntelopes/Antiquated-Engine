
#ifndef ENGINE_MESH_H
#define	ENGINE_MESH_H

#include "BaseMesh.h"

enum
{
	kLock_Normal = 0,
	kLock_ReadOnly,
	kLock_Discard,
};

#ifdef __cplusplus

// A copy of a mesh's triangles in ordinary memory, for CPU collision tests. Reading a mesh back through a lock is very
// slow, because its buffers are in write-combined memory (D3D9Ex meshes can't be in the managed pool).
typedef struct
{
	VECT	xBoundCentre;		// Bounding sphere of each triangle
	float	fBoundRadius;
} ENGINEMESH_TRI_BOUND;

typedef struct
{
	int						nNumTris;
	ENGINEMESH_TRI_BOUND*	pxBounds;		// nNumTris of these
	VECT*					pxVerts;		// nNumTris * 3 of these, in the mesh's index order
} ENGINEMESH_COLLISION_TRIS;

class EngineMesh
{
public:
	EngineMesh();
	~EngineMesh();

	void	Create( int nNumFaces, int nNumVertices, int nFlags );
	EngineMesh*		CreateCopy();
	void	Release( void );

	int		GetNumVertices( void );
	int		GetNumFaces( void );

	int		LockVertexBuffer( int, unsigned char** ppVertices );
	int		UnlockVertexBuffer( void );

	int		LockIndexBuffer( int, unsigned char** ppIndices );
	int		UnlockIndexBuffer( void );

	int		LockAttributeBuffer( int, unsigned char** );
	void	UnlockAttributeBuffer();

	void	DrawSubset( int );

	void	RayTest( const VECT* pRayPos, const VECT* pRayDir, BOOL* pbHit, uint32* pFaceIndex, float* pU, float* pV, float* pfDist, void* pIntersectBuffer, uint32* pulHitCount );

	void	ReorderByMaterial( fnVertexRemapCallback, void* );

	BOOL	Is32BitIndexBuffer( void );

	// The mesh's first nNumIndices/3 triangles. Built on first use, and again after anything locks the mesh to write to it
	const ENGINEMESH_COLLISION_TRIS*	GetCollisionTris( int nNumIndices );

	//------------------------
	void	CreateFromPlatformMeshImpl( void* pDXPlatformMesh );		// TEMP!
	void*	GetPlatformMeshImpl( void );		// TEMP!
private:

	void	FreeCollisionTris( void );

	BaseMesh*	mpBaseMesh;

	ENGINEMESH_COLLISION_TRIS	mxCollisionTris;
	BOOL						mbCollisionTrisValid;

};

#endif // ifdef __cplusplus

//----------------------------------------------------------------------------------


#ifdef __cplusplus
extern "C"				// C-interface
{
#endif
	

void	EngineCreateMesh( int nNumFaces, int nNumVertices, EngineMesh** ppMeshEngine, int nFlags );


#ifdef __cplusplus
}
#endif


#endif
