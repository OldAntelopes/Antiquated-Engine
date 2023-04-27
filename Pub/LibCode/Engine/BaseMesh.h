
#ifndef ENGINE_BASE_MESH_H
#define	ENGINE_BASE_MESH_H


typedef	void(*fnVertexRemapCallback)( void* pModelRenderData, void* pVertexRemapData );

//-----------------------------------------------------
// BaseMesh
//
// Pure virtual mesh class that each graphics system has to implement
//-----------------------------------------------------

class BaseMesh
{
public:
	BaseMesh() {}
	virtual ~BaseMesh() {}

	virtual void	Create( int nNumFaces, int nNumVertices, int nFlags ) = 0;
	virtual void	Release( void ) = 0;

	virtual int		GetNumVertices( void ) = 0;
	virtual int		GetNumFaces( void ) = 0;

	virtual int		LockVertexBuffer( int, unsigned char** ppVertices ) = 0;
	virtual int		UnlockVertexBuffer( void ) = 0;

	virtual int		LockIndexBuffer( int, unsigned char** ppIndices ) = 0;
	virtual int		UnlockIndexBuffer( void ) = 0;

	virtual int		LockAttributeBuffer( int, unsigned char** ) = 0;
	virtual void	UnlockAttributeBuffer() = 0;

	virtual void	DrawSubset( int ) = 0;

	virtual void	RayTest( const VECT* pRayPos, const VECT* pRayDir, BOOL* pbHit, uint32* pFaceIndex, float* pU, float* pV, float* pfDist, void* pIntersectBuffer, uint32* pulHitCount ) = 0;

	virtual void	ReorderByMaterial( fnVertexRemapCallback, void* ) = 0;

	virtual BOOL	Is32BitIndexBuffer( void ) = 0;

	//--------- Temp - to support X loading only
	virtual void*	GetPlatformMeshImpl() { return( 0 ); };	
	virtual void	CreateFromPlatformMeshImpl( void* pPlatformMesh ) {}
	//----------------------
private:

};



#endif
