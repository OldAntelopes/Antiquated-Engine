

#ifdef USING_OPENGL

#endif
#ifdef ENGINEDX
#include "DirectX/EngineDX.h"
#endif

#include <StandardDef.h>

#ifdef USING_OPENGL
#include "OpenGL/BaseMeshGL.h"
#endif
#ifdef ENGINEDX
#include "DirectX/BaseMeshDX.h"
#endif
#ifdef IW_SDK
#include "Marmalade/BaseMeshMarmalade.h"
#endif

#include "StandardDef.h"
#include "Interface.h"
#include "Rendering.h"
#include "EngineMesh.h"

#include <math.h>
#include <stdlib.h>


EngineMesh::EngineMesh()
{
	mpBaseMesh = NULL;
	mxCollisionTris.nNumTris = 0;
	mxCollisionTris.pxBounds = NULL;
	mxCollisionTris.pxVerts = NULL;
	mbCollisionTrisValid = FALSE;
}


EngineMesh::~EngineMesh()
{
	FreeCollisionTris();
	if ( mpBaseMesh )
	{
		mpBaseMesh->Release();
	}
}

void	EngineMesh::Release( void )
{
	FreeCollisionTris();
	mpBaseMesh->Release();
	delete mpBaseMesh;
	mpBaseMesh = NULL;
}

void	EngineMesh::DrawSubset( int materialNum )
{
	mpBaseMesh->DrawSubset( materialNum );
}



int	EngineMesh::GetNumFaces( void )
{
	return( mpBaseMesh->GetNumFaces() );
}


int	EngineMesh::GetNumVertices( void )
{
	return( mpBaseMesh->GetNumVertices() );
}


void	EngineMesh::ReorderByMaterial( fnVertexRemapCallback callbackFunction, void* pParam )
{
	mbCollisionTrisValid = FALSE;
	mpBaseMesh->ReorderByMaterial( callbackFunction, pParam );
}


void	EngineMesh::RayTest( const VECT* pRayPos, const VECT* pRayDir, BOOL* pbHit, uint32* pFaceIndex, float* pU, float* pV, float* pfDist, void* pIntersectBuffer, uint32* pulHitCount )
{
	mpBaseMesh->RayTest( pRayPos, pRayDir, pbHit, pFaceIndex, pU, pV, pfDist, pIntersectBuffer, pulHitCount );
}

int		EngineMesh::LockVertexBuffer( int flags, unsigned char** ppVertices )
{
	if ( flags != kLock_ReadOnly )
	{
		mbCollisionTrisValid = FALSE;
	}
	return( mpBaseMesh->LockVertexBuffer( flags, ppVertices ) );
}

int		EngineMesh::UnlockVertexBuffer( void )
{
	return( mpBaseMesh->UnlockVertexBuffer() );
}

BOOL	EngineMesh::Is32BitIndexBuffer( void )
{
	return( mpBaseMesh->Is32BitIndexBuffer() );
}


int		EngineMesh::LockIndexBuffer( int flags, unsigned char** ppIndices )
{
	if ( flags != kLock_ReadOnly )
	{
		mbCollisionTrisValid = FALSE;
	}
	return( mpBaseMesh->LockIndexBuffer( flags, ppIndices ) );
}

int		EngineMesh::UnlockIndexBuffer( void )
{
	return( mpBaseMesh->UnlockIndexBuffer() );
}


int		EngineMesh::LockAttributeBuffer( int flags, unsigned char** ppAttributes )
{
	return( mpBaseMesh->LockAttributeBuffer( flags, ppAttributes ) );
}

void	EngineMesh::UnlockAttributeBuffer()
{
	mpBaseMesh->UnlockAttributeBuffer();
}

void	EngineMesh::Create( int nNumFaces, int nNumVertices, int nFlags )
{
#ifdef USING_OPENGL
BaseMeshGL*		pDXBaseMesh = new BaseMeshGL;
	mpBaseMesh = (BaseMesh*)( pDXBaseMesh );
#else	
#ifdef TUD11
	PANIC_IF( TRUE, "DX11 EngineMesh::Create TBI" );
#else
BaseMeshDX9*		pDXBaseMesh = new BaseMeshDX9;
	mpBaseMesh = (BaseMesh*)( pDXBaseMesh );
#endif
#endif

	mpBaseMesh->Create( nNumFaces, nNumVertices, nFlags );
}

EngineMesh*		EngineMesh::CreateCopy()
{
BaseMesh*		pNewBaseMesh;
EngineMesh*		pNewEngineMesh;

#ifdef USING_OPENGL
	PANIC_IF( TRUE, "OpenGL EngineMesh::CreateCopy TBI" );
#else	
#ifdef TUD11
	PANIC_IF( TRUE, "DX11 EngineMesh::CreateCopy TBI" );
#else
	BaseMeshDX9*		pNewDXBaseMesh = new BaseMeshDX9;
	pNewBaseMesh = (BaseMesh*)( pNewDXBaseMesh );
#endif
#endif
	pNewBaseMesh->Create( mpBaseMesh->GetNumFaces(), mpBaseMesh->GetNumVertices(), 0 );
	// Copy indices
	ushort*		puwOutIndices;
	ushort*		puwSrcIndices;
	int			nVertLoop;

	pNewBaseMesh->LockIndexBuffer( kLock_Normal, (BYTE**)&puwOutIndices );
	mpBaseMesh->LockIndexBuffer( kLock_ReadOnly, (BYTE**)&puwSrcIndices );
	for ( nVertLoop = 0; nVertLoop < mpBaseMesh->GetNumFaces() * 3; nVertLoop++ )
	{
		*(puwOutIndices++) = *(puwSrcIndices++);
	}
	mpBaseMesh->UnlockIndexBuffer();
	pNewBaseMesh->UnlockIndexBuffer();

	CUSTOMVERTEX*		pxOutVertices;
	CUSTOMVERTEX*		pxSrcVertices;

	pNewBaseMesh->LockVertexBuffer( kLock_Normal, (BYTE**)&pxOutVertices );
	mpBaseMesh->LockVertexBuffer( kLock_ReadOnly, (BYTE**)&pxSrcVertices );
	for ( nVertLoop = 0; nVertLoop < mpBaseMesh->GetNumVertices(); nVertLoop++ )
	{
		*(pxOutVertices++) = *(pxSrcVertices++);
	}
	mpBaseMesh->UnlockVertexBuffer();
	pNewBaseMesh->UnlockVertexBuffer();

	// TODO - Copy materials and etc


	pNewEngineMesh = new EngineMesh;
	pNewEngineMesh->mpBaseMesh = pNewBaseMesh;
	return( pNewEngineMesh );
}


void	EngineMesh::CreateFromPlatformMeshImpl( void* pDXPlatformMesh )
{
#ifdef USING_OPENGL
BaseMeshGL*		pDXBaseMesh = new BaseMeshGL;
	mpBaseMesh = (BaseMesh*)( pDXBaseMesh );
#else		//--------- #ifdef DIRECTX
#ifdef TUD11
	PANIC_IF( TRUE, "DX11 EngineMesh::Create TBI" );
#else
BaseMeshDX9*		pDXBaseMesh = new BaseMeshDX9;
	mpBaseMesh = (BaseMesh*)( pDXBaseMesh );
#endif
#endif

	mpBaseMesh->CreateFromPlatformMeshImpl( pDXPlatformMesh );
}


void*	EngineMesh::GetPlatformMeshImpl( void )
{
	return( mpBaseMesh->GetPlatformMeshImpl() );
}


void	EngineMesh::FreeCollisionTris( void )
{
	free( mxCollisionTris.pxBounds );
	free( mxCollisionTris.pxVerts );
	mxCollisionTris.nNumTris = 0;
	mxCollisionTris.pxBounds = NULL;
	mxCollisionTris.pxVerts = NULL;
	mbCollisionTrisValid = FALSE;
}


/***************************************************************************
 * Function    : EngineMesh::GetCollisionTris
 * Params      : nNumIndices - how many indices of the mesh to use
 * Returns     : The triangles, or NULL if the mesh couldn't be read
 * Description : The first call reads the whole mesh back, which is as slow as one old brute force collision test.
 *				 Indices are read as 16 bit, as the collision code always has.
 ***************************************************************************/
const ENGINEMESH_COLLISION_TRIS*	EngineMesh::GetCollisionTris( int nNumIndices )
{
CUSTOMVERTEX*	pxVertices = NULL;
ushort*			puwIndices = NULL;
int				nNumTris = nNumIndices / 3;
int				nLoop;

	if ( ( mbCollisionTrisValid ) &&
		 ( mxCollisionTris.nNumTris == nNumTris ) )
	{
		return( &mxCollisionTris );
	}

	if ( ( mxCollisionTris.nNumTris != nNumTris ) ||
		 ( mxCollisionTris.pxBounds == NULL ) )
	{
		FreeCollisionTris();
		mxCollisionTris.pxBounds = (ENGINEMESH_TRI_BOUND*)malloc( nNumTris * sizeof( ENGINEMESH_TRI_BOUND ) );
		mxCollisionTris.pxVerts = (VECT*)malloc( nNumTris * 3 * sizeof( VECT ) );
		if ( ( mxCollisionTris.pxBounds == NULL ) ||
			 ( mxCollisionTris.pxVerts == NULL ) )
		{
			FreeCollisionTris();
			return( NULL );
		}
		mxCollisionTris.nNumTris = nNumTris;
	}

	mpBaseMesh->LockVertexBuffer( kLock_ReadOnly, (byte**)( &pxVertices ) );
	mpBaseMesh->LockIndexBuffer( kLock_ReadOnly, (byte**)( &puwIndices ) );
	if ( ( pxVertices == NULL ) ||
		 ( puwIndices == NULL ) )
	{
		if ( pxVertices ) mpBaseMesh->UnlockVertexBuffer();
		if ( puwIndices ) mpBaseMesh->UnlockIndexBuffer();
		return( NULL );
	}

	for ( nLoop = 0; nLoop < nNumTris; nLoop++ )
	{
	VECT*					pxVerts = mxCollisionTris.pxVerts + ( nLoop * 3 );
	ENGINEMESH_TRI_BOUND*	pxBound = mxCollisionTris.pxBounds + nLoop;
	float					fMaxDistSq = 0.0f;
	int						nVert;

		pxVerts[0] = *( (VECT*)&pxVertices[ puwIndices[ nLoop*3 ] ].position );
		pxVerts[1] = *( (VECT*)&pxVertices[ puwIndices[ (nLoop*3) + 1 ] ].position );
		pxVerts[2] = *( (VECT*)&pxVertices[ puwIndices[ (nLoop*3) + 2 ] ].position );

		pxBound->xBoundCentre.x = ( pxVerts[0].x + pxVerts[1].x + pxVerts[2].x ) / 3;
		pxBound->xBoundCentre.y = ( pxVerts[0].y + pxVerts[1].y + pxVerts[2].y ) / 3;
		pxBound->xBoundCentre.z = ( pxVerts[0].z + pxVerts[1].z + pxVerts[2].z ) / 3;
		for ( nVert = 0; nVert < 3; nVert++ )
		{
		float	fX = pxVerts[nVert].x - pxBound->xBoundCentre.x;
		float	fY = pxVerts[nVert].y - pxBound->xBoundCentre.y;
		float	fZ = pxVerts[nVert].z - pxBound->xBoundCentre.z;
		float	fDistSq = ( fX * fX ) + ( fY * fY ) + ( fZ * fZ );

			if ( fDistSq > fMaxDistSq )
			{
				fMaxDistSq = fDistSq;
			}
		}
		pxBound->fBoundRadius = sqrtf( fMaxDistSq );
	}

	mpBaseMesh->UnlockVertexBuffer();
	mpBaseMesh->UnlockIndexBuffer();
	mbCollisionTrisValid = TRUE;
	return( &mxCollisionTris );
}




//----------------------------------------------------------------------------------
//--- C Interfaces


//----------------------------------------------------------------------------------
void	EngineCreateMesh( int nNumFaces, int nNumVertices, EngineMesh** ppMeshEngine, int nFlags )
{
EngineMesh*		pEngineMesh = new EngineMesh;

	pEngineMesh->Create( nNumFaces, nNumVertices, nFlags );
	*ppMeshEngine = pEngineMesh;
}


