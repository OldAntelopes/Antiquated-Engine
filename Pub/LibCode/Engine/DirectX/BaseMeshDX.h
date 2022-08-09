
#ifndef ENGINE_BASE_MESH_DX9_H
#define	ENGINE_BASE_MESH_DX9_H

#include "../BaseMesh.h"

#ifdef TUD9

//-----------------------------------------------------
// BaseMeshDX9
//
// DirectX9 implementation of our mesh class
//-----------------------------------------------------

class BaseMeshDX9 : public BaseMesh
{
public:
	BaseMeshDX9();
	virtual ~BaseMeshDX9();

	virtual void	Create( int nNumFaces, int nNumVertices, int nFlags );
	virtual void	Release( void );

	virtual int		GetNumVertices( void );
	virtual int		GetNumFaces( void );

	virtual int		LockVertexBuffer( int, unsigned char** ppVertices );
	virtual int		UnlockVertexBuffer( void );

	virtual int		LockIndexBuffer( int, unsigned char** ppIndices );
	virtual int		UnlockIndexBuffer( void );

	virtual int		LockAttributeBuffer( int, unsigned char** );
	virtual void	UnlockAttributeBuffer();

	virtual void	DrawSubset( int );

	virtual void	RayTest( const VECT* pRayPos, const VECT* pRayDir, BOOL* pbHit, ulong* pFaceIndex, float* pU, float* pV, float* pfDist, void* pIntersectBuffer, ulong* pulHitCount );

	virtual void	ReorderByMaterial( fnVertexRemapCallback, void* );

	virtual BOOL	Is32BitIndexBuffer( void );

	//-------------
	virtual void*	GetPlatformMeshImpl() { return( (void*) mpDXPlatformMesh ); }
	virtual void	CreateFromPlatformMeshImpl( void* pPlatformMesh );		// TEMP!
	//-------------

private:
	
	LPD3DXMESH		mpDXPlatformMesh;
};



#endif // ifdef TUD9

#endif
