
#include <d3dx9.h>

#include <StandardDef.h>
#include <Engine.h>
#include <Rendering.h>
#include <Interface.h>

#include "LandscapeHeightmap.h"
#include "LandscapeCoords.h"

ID3DXMesh*	mpFullLandscapeMesh = NULL;

#define		BLOCK_MESH_DIVIDER		32
#define		NUM_BLOCK_MESHES		(BLOCK_MESH_DIVIDER*BLOCK_MESH_DIVIDER)

typedef struct
{
	ID3DXMesh*	pMesh;

	VECT		xBoundMin;
	VECT		xBoundMax;

} LANDSCAPE_BLOCKS;

LANDSCAPE_BLOCKS	maLandscapeBlocks[NUM_BLOCK_MESHES];


void LandscapeRenderGenerateBlockMesh( LANDSCAPE_BLOCKS* pBlockOut, int nBaseMapX, int nBaseMapY, int nMapW, int nMapH )
{
LPDIRECT3DDEVICE9		pd3dDevice = EngineGetDXDevice();
int		nNumFaces;
int		nNumVertices;
int		nNumVerticesPerSide;
ID3DXMesh*	pMesh;
ID3DXMesh*	pNewMesh = NULL;
CUSTOMVERTEX*		pxVertices;
ushort*				puwIndices;
uint32*				pulIndices;
int		nVertX;
int		nVertY;
int		nBaseVert;
int		nBaseVertBase;
HRESULT	ret;
BOOL	b32bitIndices = FALSE;
int		nResolution = 1;
int		nMapX;
int		nMapY;
VECT	xBoundMin = { 0.0f, 0.0f, 0.0f };
VECT	xBoundMax = { 0.0f, 0.0f, 0.0f };

	nNumFaces = (nMapW/nResolution) * (nMapH/nResolution);
	nNumFaces *= 2;
	nNumVerticesPerSide = ((nMapW/nResolution) + 1);
	nNumVertices = nNumVerticesPerSide * nNumVerticesPerSide;

	// First create a mesh containing all vertices
	
	if ( ( nNumFaces > 65535 ) ||
		 ( nNumVertices > 65535 ) )
	{
		ret = D3DXCreateMeshFVF( nNumFaces, nNumVertices, D3DXMESH_SYSTEMMEM|D3DXMESH_32BIT, D3DFVF_CUSTOMVERTEX, pd3dDevice, &pMesh );
		b32bitIndices = TRUE;
	}
	else
	{
		ret = D3DXCreateMeshFVF( nNumFaces, nNumVertices, D3DXMESH_SYSTEMMEM, D3DFVF_CUSTOMVERTEX, pd3dDevice, &pMesh );
	}

	if ( pMesh )
	{
		pMesh->LockVertexBuffer( NULL, (void**)( &pxVertices ) );
		// Write vertices
		nMapY = nBaseMapY;
		for ( nVertY = 0; nVertY < nMapH + 1; nVertY += nResolution )
		{
			nMapX = nBaseMapX;
			for ( nVertX = 0; nVertX < nMapW + 1; nVertX += nResolution )
			{
				pxVertices->color = 0xFFFFFFFF;
				pxVertices->position = LandscapeGetWorldPos( nMapX, nMapY );
				pxVertices->normal = LandscapeGetNormal( nMapX, nMapY );
				pxVertices->tu = 0.0f;
				pxVertices->tv = 0.0f;

				if ( ( nVertX == 0 ) &&
					 ( nVertY == 0 ) )
				{
					xBoundMin.x = pxVertices->position.x;
					xBoundMin.y = pxVertices->position.y;
					xBoundMin.z = 0.0f;
				}
				else if ( ( nVertX == nMapW + 1 - nResolution ) &&
						  ( nVertY == nMapH + 1 - nResolution ) )
				{
					xBoundMax.x = pxVertices->position.x;
					xBoundMax.y = pxVertices->position.y;
					xBoundMax.z = 1000.0f;
				}
				
				pxVertices++;
				nMapX += nResolution;
			}
			nMapY+= nResolution;
		}
		pMesh->UnlockVertexBuffer();
		
		// Generate indices
		pMesh->LockIndexBuffer( NULL, (void**)( &puwIndices ) );

		if ( b32bitIndices )
		{
			pulIndices = (uint32*)( puwIndices );
		}
		nBaseVertBase = 0;
		for ( nVertY = 0; nVertY < nMapH; nVertY += nResolution )
		{
			nBaseVert = nBaseVertBase;
			for ( nVertX = 0; nVertX < nMapW; nVertX += nResolution )
			{
				if ( b32bitIndices )
				{
					pulIndices[0] = nBaseVert;
					pulIndices[1] = nBaseVert + 1;
					pulIndices[2] = nBaseVert + nNumVerticesPerSide;
					// Tri 2
					pulIndices[3] = nBaseVert + 1;
					pulIndices[4] = nBaseVert + nNumVerticesPerSide + 1;
					pulIndices[5] = nBaseVert + nNumVerticesPerSide;
					pulIndices += 6;		
				}
				else
				{
					// Tri 1
					puwIndices[0] = nBaseVert;
					puwIndices[1] = nBaseVert + 1;
					puwIndices[2] = nBaseVert + nNumVerticesPerSide;
					// Tri 2
					puwIndices[3] = nBaseVert + 1;
					puwIndices[4] = nBaseVert + nNumVerticesPerSide + 1;
					puwIndices[5] = nBaseVert + nNumVerticesPerSide;
					puwIndices += 6;
				}
				nBaseVert++;
			}
			nBaseVertBase += ((nMapW+1)/nResolution);
		}
		pMesh->UnlockIndexBuffer();
	}

	pBlockOut->pMesh = pMesh;
	pBlockOut->xBoundMin = xBoundMin;
	pBlockOut->xBoundMax = xBoundMax;
}


void		LandscapeRenderGenerateBlocks( void )
{
int		nBlockSize = LandscapeHeightmapGetSizeX() / BLOCK_MESH_DIVIDER;
int		nLoopX;
int		nLoopY;
int		nBlockNum = 0;

	for( nLoopY = 0; nLoopY < BLOCK_MESH_DIVIDER; nLoopY++ )
	{
		for( nLoopX = 0; nLoopX < BLOCK_MESH_DIVIDER; nLoopX++ )
		{
			LandscapeRenderGenerateBlockMesh( &maLandscapeBlocks[nBlockNum], nLoopX * nBlockSize, nLoopY * nBlockSize, nBlockSize, nBlockSize );
			nBlockNum++;
		}
	}
}


void LandscapeRenderGenerateFullLandscapeMeshDX( void )
{
LPDIRECT3DDEVICE9		pd3dDevice = EngineGetDXDevice();
int		nNumFaces;
int		nNumVertices;
int		nNumVerticesPerSide;
ID3DXMesh*	pMesh;
ID3DXMesh*	pNewMesh = NULL;
CUSTOMVERTEX*		pxVertices;
ushort*				puwIndices;
uint32*				pulIndices;
int		nVertX;
int		nVertY;
int		nMapX;
int		nMapY;
int		nBaseVert;
int		nBaseVertBase;
int		nMapW = LandscapeHeightmapGetSizeX();
int		nMapH = LandscapeHeightmapGetSizeY();
HRESULT	ret;
BOOL	b32bitIndices = FALSE;
int		nResolution = 1;

	nNumFaces = (nMapW/nResolution) * (nMapH/nResolution);
	nNumFaces *= 2;
	nNumVerticesPerSide = ((nMapW/nResolution) + 1);
	nNumVertices = nNumVerticesPerSide * nNumVerticesPerSide;

	// First create a mesh containing all vertices
	
	if ( ( nNumFaces > 65535 ) ||
		 ( nNumVertices > 65535 ) )
	{
		ret = D3DXCreateMeshFVF( nNumFaces, nNumVertices, D3DXMESH_SYSTEMMEM|D3DXMESH_32BIT, D3DFVF_CUSTOMVERTEX, pd3dDevice, &pMesh );
		b32bitIndices = TRUE;
	}
	else
	{
		ret = D3DXCreateMeshFVF( nNumFaces, nNumVertices, D3DXMESH_SYSTEMMEM, D3DFVF_CUSTOMVERTEX, pd3dDevice, &pMesh );
	}
	if ( pMesh )
	{
		pMesh->LockVertexBuffer( NULL, (void**)( &pxVertices ) );
		// Write vertices
		nMapY = 0;
		for ( nVertY = 0; nVertY < nMapH; nVertY += nResolution )
		{
			nMapX = 0;
			for ( nVertX = 0; nVertX < nMapW; nVertX += nResolution )
			{
				pxVertices->color = 0xFFFFFFFF;
				pxVertices->position = LandscapeGetWorldPos( nMapX, nMapY );
				pxVertices->normal = LandscapeGetNormal( nMapX, nMapY );
				pxVertices->tu = 0.0f;
				pxVertices->tv = 0.0f;
				pxVertices++;
				nMapX += nResolution;
			}
			nMapY+= nResolution;
		}
		pMesh->UnlockVertexBuffer();
		
		// Generate indices
		pMesh->LockIndexBuffer( NULL, (void**)( &puwIndices ) );

		if ( b32bitIndices )
		{
			pulIndices = (uint32*)( puwIndices );
		}
		nBaseVertBase = 0;
		for ( nVertY = 0; nVertY < nMapH; nVertY += nResolution )
		{
			nBaseVert = nBaseVertBase;
			for ( nVertX = 0; nVertX < nMapW; nVertX += nResolution )
			{
				if ( b32bitIndices )
				{
					pulIndices[0] = nBaseVert;
					pulIndices[1] = nBaseVert + 1;
					pulIndices[2] = nBaseVert + nNumVerticesPerSide;
					// Tri 2
					pulIndices[3] = nBaseVert + 1;
					pulIndices[4] = nBaseVert + nNumVerticesPerSide + 1;
					pulIndices[5] = nBaseVert + nNumVerticesPerSide;
					pulIndices += 6;		
				}
				else
				{
					// Tri 1
					puwIndices[0] = nBaseVert;
					puwIndices[1] = nBaseVert + 1;
					puwIndices[2] = nBaseVert + nNumVerticesPerSide;
					// Tri 2
					puwIndices[3] = nBaseVert + 1;
					puwIndices[4] = nBaseVert + nNumVerticesPerSide + 1;
					puwIndices[5] = nBaseVert + nNumVerticesPerSide;
					puwIndices += 6;
				}
				nBaseVert++;
			}
			nBaseVertBase += (nMapW/nResolution);
		}
		pMesh->UnlockIndexBuffer();
	}
	mpFullLandscapeMesh = pMesh;
}


//BOOL		EngineCollisionBoxBoundProbe( VECT* pxBoxMin, VECT* pxBoxMax, VECT* pxRayStart, VECT* pxRayDir ); 


BOOL		LandscapeMeshRayTest( const VECT* pxRayStart, const VECT* pxRayDir, VECT* pxIntersect )
{
int		nBlockNum = 0;
int		nLoopX;
int		nLoopY;
LANDSCAPE_BLOCKS*		pBlocks = maLandscapeBlocks;
VECT	xRayDir = *pxRayDir;
float	fCollisionDist = 10000.0f;
BOOL	boCollision = FALSE;
uint32	uwCollIndex;

	VectNormalize( &xRayDir );

	for( nLoopY = 0; nLoopY < BLOCK_MESH_DIVIDER; nLoopY++ )
	{
		for( nLoopX = 0; nLoopX < BLOCK_MESH_DIVIDER; nLoopX++ )
		{
			if ( EngineCollisionBoxBoundProbe( &pBlocks->xBoundMin, &pBlocks->xBoundMax, (VECT*)pxRayStart, (VECT*)pxRayDir ) == TRUE )
			{
				D3DXIntersect( pBlocks->pMesh, (D3DXVECTOR3*)pxRayStart,(D3DXVECTOR3*)&xRayDir, &boCollision, &uwCollIndex, NULL, NULL, &fCollisionDist, NULL, NULL );

				if ( boCollision == TRUE )
				{
					VectScale( &xRayDir, &xRayDir, fCollisionDist );
					VectAdd( pxIntersect, pxRayStart, &xRayDir );
					return( TRUE );
				}
			}
			pBlocks++;
		}
	}
	return( FALSE );

#if 0 
						
	if ( mpFullLandscapeMesh ) 
	{
	float	fCollisionDist = 10000.0f;
	BOOL	boCollision = FALSE;
	VECT	xRayDir = *pxRayDir;
	uint32	uwCollIndex;

		VectNormalize( &xRayDir );
		D3DXIntersect( mpFullLandscapeMesh, (D3DXVECTOR3*)pxRayStart,(D3DXVECTOR3*)&xRayDir, &boCollision, &uwCollIndex, NULL, NULL, &fCollisionDist, NULL, NULL );

		if ( boCollision == TRUE )
		{
			VectScale( &xRayDir, &xRayDir, fCollisionDist );
			VectAdd( pxIntersect, pxRayStart, &xRayDir );
			return( TRUE );
		}
	}
	return( FALSE );
#endif

}
