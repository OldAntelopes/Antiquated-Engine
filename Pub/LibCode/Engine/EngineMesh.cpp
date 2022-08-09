

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
#ifdef IW_SDK
BaseMeshMarmalade*		pMarmaladeBaseMesh = new BaseMeshMarmalade;
	mpBaseMesh = (BaseMesh*)( pMarmaladeBaseMesh );
#else		//DX
#ifdef TUD11
	PANIC_IF( TRUE, "DX11 EngineMesh::Create TBI" );
#else
BaseMeshDX9*		pDXBaseMesh = new BaseMeshDX9;
	mpBaseMesh = (BaseMesh*)( pDXBaseMesh );
#endif
#endif
#endif

	mpBaseMesh->Create( nNumFaces, nNumVertices, nFlags );
}


void	EngineMesh::CreateFromPlatformMeshImpl( void* pDXPlatformMesh )
{
#ifdef USING_OPENGL
BaseMeshGL*		pDXBaseMesh = new BaseMeshGL;
	mpBaseMesh = (BaseMesh*)( pDXBaseMesh );
#else		//--------- #ifdef DIRECTX
#ifdef IW_SDK
BaseMeshMarmalade*		pMarmaladeBaseMesh = new BaseMeshMarmalade;
	mpBaseMesh = (BaseMesh*)( pMarmaladeBaseMesh );
#else		//dx9
#ifdef TUD11
	PANIC_IF( TRUE, "DX11 EngineMesh::Create TBI" );
#else
BaseMeshDX9*		pDXBaseMesh = new BaseMeshDX9;
	mpBaseMesh = (BaseMesh*)( pDXBaseMesh );
#endif
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


