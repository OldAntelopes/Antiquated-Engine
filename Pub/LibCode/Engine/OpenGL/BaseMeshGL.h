
#ifndef ENGINE_BASE_MESH_OPENGL_H
#define	ENGINE_BASE_MESH_OPENGL_H

#include "../BaseMesh.h"

class EngineVertexBufferGL;
//-----------------------------------------------------
// BaseMeshGL
//
// OpenGL implementation of our mesh class
//-----------------------------------------------------

class BaseMeshGL : public BaseMesh
{
public:
	BaseMeshGL();
	virtual ~BaseMeshGL();

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

	virtual void	RayTest( const VECT* pRayPos, const VECT* pRayDir, BOOL* pbHit, uint32* pFaceIndex, float* pU, float* pV, float* pfDist, void* pIntersectBuffer, uint32* pulHitCount );

	virtual void	ReorderByMaterial( fnVertexRemapCallback, void*  );

	virtual BOOL	Is32BitIndexBuffer( void );

private:
	ushort*						m_puwIndices;
	EngineVertexBufferGL*		m_pVertexBuffer;		// Will need multiple ones of these to do multiple materials
	// todo - attribute buffer

	uint32		m_ulNumVertices;
	uint32		m_ulNumFaces;

};



#endif
