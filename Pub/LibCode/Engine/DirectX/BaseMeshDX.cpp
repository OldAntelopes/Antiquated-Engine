
#ifdef TUD9

#include "EngineDX.h"

#include <StandardDef.h>

#include "../EngineMesh.h"
#include "BaseMeshDX.h"


BaseMeshDX9::BaseMeshDX9()
{
	mpDXPlatformMesh = NULL;
}


BaseMeshDX9::~BaseMeshDX9()
{
	if ( mpDXPlatformMesh )
	{
		mpDXPlatformMesh->Release();
	}
}

void	BaseMeshDX9::Release( void )
{
	mpDXPlatformMesh->Release();
	mpDXPlatformMesh = NULL;
}

void	BaseMeshDX9::DrawSubset( int materialNum )
{
	mpDXPlatformMesh->DrawSubset( materialNum );
}


int	BaseMeshDX9::GetNumFaces( void )
{
	return( mpDXPlatformMesh->GetNumFaces() );
}


int	BaseMeshDX9::GetNumVertices( void )
{
	return( mpDXPlatformMesh->GetNumVertices() );
}


void	BaseMeshDX9::ReorderByMaterial( fnVertexRemapCallback remapCallbackFunction, void* pParam )
{
int		nNumFaces = GetNumFaces();

	// Reorder the vertices according to subset and optimize the mesh for this graphics 
	// card's vertex cache. When rendering the mesh's triangle list the vertices will 
	// cache hit more often so it won't have to re-execute the vertex shader.
	DWORD* aAdjacency = new DWORD[(nNumFaces*3) + 1000];
	if( aAdjacency == NULL )
	{
//		PANIC_IF(TRUE, "Out of memory" );
	}
	else
	{
	LPD3DXBUFFER	pVertexRemap = NULL;

		mpDXPlatformMesh->GenerateAdjacency(1e-6f,aAdjacency);
		mpDXPlatformMesh->OptimizeInplace(D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_VERTEXCACHE, aAdjacency, NULL, NULL, &pVertexRemap);

		// TODO - Calling this breaks any animation data stored with this model (so we can't currently animate multi-material bodies).
		// TODO - Here we will need to re-process any animation data stored with the model so the animation vertex numbers match
		// TODO - the new arrangement generated here
		// ( pVertexRemap should include all the info we need... )
		if ( remapCallbackFunction )
		{
			remapCallbackFunction( pParam, pVertexRemap->GetBufferPointer() );
		}

		SAFE_DELETE_ARRAY( aAdjacency );
		if ( pVertexRemap )
		{
			pVertexRemap->Release();
		}
	}
}


void	BaseMeshDX9::RayTest( const VECT* pRayPos, const VECT* pRayDir, BOOL* pbHit, uint32* pFaceIndex, float* pU, float* pV, float* pfDist, void* pIntersectBuffer, uint32* pulHitCount )
{
	D3DXIntersect( mpDXPlatformMesh, (const D3DXVECTOR3*)pRayPos, (const D3DXVECTOR3*)pRayDir, pbHit, (LPDWORD)pFaceIndex, pU, pV, pfDist, (LPD3DXBUFFER*)pIntersectBuffer, (LPDWORD)pulHitCount );
}

int		BaseMeshDX9::LockVertexBuffer( int flags, unsigned char** ppVertices )
{
	if ( mpDXPlatformMesh )
	{
		switch( flags )
		{
		case kLock_Normal:	
			mpDXPlatformMesh->LockVertexBuffer( 0, (VERTEX_LOCKTYPE)ppVertices );
			break;
		case kLock_ReadOnly:
			mpDXPlatformMesh->LockVertexBuffer( D3DLOCK_READONLY, (VERTEX_LOCKTYPE)ppVertices );
			break;
		}
	}
	return( 0 );
}

int		BaseMeshDX9::UnlockVertexBuffer( void )
{
	if ( mpDXPlatformMesh )
	{
		mpDXPlatformMesh->UnlockVertexBuffer();
	}
	return( 0 );
}


int		BaseMeshDX9::LockIndexBuffer( int flags, unsigned char** ppIndices )
{
	if ( !mpDXPlatformMesh ) return 0;

	switch( flags )
	{
	case kLock_Normal:	
		mpDXPlatformMesh->LockIndexBuffer( 0, (VERTEX_LOCKTYPE)ppIndices );
		break;
	case kLock_ReadOnly:
		mpDXPlatformMesh->LockIndexBuffer( D3DLOCK_READONLY, (VERTEX_LOCKTYPE)ppIndices );
		break;
	}
	return( 0 );
}

int		BaseMeshDX9::UnlockIndexBuffer( void )
{
	mpDXPlatformMesh->UnlockIndexBuffer();
	return( 0 );
}


int		BaseMeshDX9::LockAttributeBuffer( int flags, BYTE** ppAttributes )
{
	switch( flags )
	{
	case kLock_Normal:	
		mpDXPlatformMesh->LockAttributeBuffer( 0, (DWORD**)ppAttributes );
		break;
	case kLock_ReadOnly:
		mpDXPlatformMesh->LockAttributeBuffer( D3DLOCK_READONLY, (DWORD**)ppAttributes );
		break;
	case kLock_Discard:
		mpDXPlatformMesh->LockAttributeBuffer( D3DLOCK_DISCARD, (DWORD**)ppAttributes );
		break;
	}
	return( 0 );
}

void	BaseMeshDX9::UnlockAttributeBuffer()
{
	mpDXPlatformMesh->UnlockAttributeBuffer();
}

BOOL	BaseMeshDX9::Is32BitIndexBuffer( void )
{
	if ( mpDXPlatformMesh->GetOptions() & D3DXMESH_32BIT )
	{
		return( TRUE );
	}
	return( FALSE );
}


void	BaseMeshDX9::Create( int nNumFaces, int nNumVertices, int nFlags )
{
	switch( nFlags )
	{
	case 0:		// Standard managed model
		if ( nNumVertices > 65535 )
		{
			D3DXCreateMeshFVF( nNumFaces, nNumVertices, D3DXMESH_MANAGED|D3DXMESH_32BIT, D3DFVF_CUSTOMVERTEX, mpEngineDevice, &mpDXPlatformMesh );
		}
		else
		{
			D3DXCreateMeshFVF( nNumFaces, nNumVertices, D3DXMESH_MANAGED, D3DFVF_CUSTOMVERTEX, mpEngineDevice, &mpDXPlatformMesh );
		}
		break;
	case 1:		// Standard system-mem model
		D3DXCreateMeshFVF( nNumFaces, nNumVertices, D3DXMESH_SYSTEMMEM, D3DFVF_CUSTOMVERTEX, mpEngineDevice, &mpDXPlatformMesh );
		break;
	case 2:		// 32 bit index buffer, managed
		D3DXCreateMeshFVF( nNumFaces, nNumVertices, D3DXMESH_MANAGED|D3DXMESH_32BIT, D3DFVF_CUSTOMVERTEX, mpEngineDevice, &mpDXPlatformMesh );
		break;
	default:
		// TODO - Error?
		break;
	}
}

// TEMP!
void	BaseMeshDX9::CreateFromPlatformMeshImpl( void* pPlatformMesh ) 
{
	mpDXPlatformMesh = (LPD3DXMESH)pPlatformMesh;
}


#endif