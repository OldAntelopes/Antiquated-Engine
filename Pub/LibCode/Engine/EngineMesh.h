
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

	void	RayTest( const VECT* pRayPos, const VECT* pRayDir, BOOL* pbHit, ulong* pFaceIndex, float* pU, float* pV, float* pfDist, void* pIntersectBuffer, ulong* pulHitCount );

	void	ReorderByMaterial( fnVertexRemapCallback, void* );

	BOOL	Is32BitIndexBuffer( void );

	//------------------------
	void	CreateFromPlatformMeshImpl( void* pDXPlatformMesh );		// TEMP!
	void*	GetPlatformMeshImpl( void );		// TEMP!
private:

	BaseMesh*	mpBaseMesh;

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
