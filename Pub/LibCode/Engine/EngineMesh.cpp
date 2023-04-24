

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


EngineMesh::EngineMesh()
{
	mpBaseMesh = NULL;
}


EngineMesh::~EngineMesh()
{
	if ( mpBaseMesh )
	{
		mpBaseMesh->Release();
	}
}

void	EngineMesh::Release( void )
{
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
	mpBaseMesh->ReorderByMaterial( callbackFunction, pParam );
}


void	EngineMesh::RayTest( const VECT* pRayPos, const VECT* pRayDir, BOOL* pbHit, ulong* pFaceIndex, float* pU, float* pV, float* pfDist, void* pIntersectBuffer, ulong* pulHitCount )
{
	mpBaseMesh->RayTest( pRayPos, pRayDir, pbHit, pFaceIndex, pU, pV, pfDist, pIntersectBuffer, pulHitCount );
}

int		EngineMesh::LockVertexBuffer( int flags, unsigned char** ppVertices )
{
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




//----------------------------------------------------------------------------------
//--- C Interfaces


//----------------------------------------------------------------------------------
void	EngineCreateMesh( int nNumFaces, int nNumVertices, EngineMesh** ppMeshEngine, int nFlags )
{
EngineMesh*		pEngineMesh = new EngineMesh;

	pEngineMesh->Create( nNumFaces, nNumVertices, nFlags );
	*ppMeshEngine = pEngineMesh;
}


