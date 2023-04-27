#include "StandardDef.h"
#include "Engine.h"
#include "Rendering.h"

#include "../../LibCode/Engine/ModelRendering.h"

#include "RecalcNormals.h"

void	ModelConvSeparateVerts( int nModelHandle )
{
MODEL_RENDER_DATA*	pxModelData = maxModelRenderData + nModelHandle;
CUSTOMVERTEX*		pxVertices;
CUSTOMVERTEX*		pxNewVertices;
ushort*			puwIndices;
uint32*			punIndices;
ushort*			puwNewIndices;
uint32*			punNewIndices;
int				nIndexLoop;
int				nNumFaces;
int				nNumIndices;
uint32			ulIndex;
EngineMesh*		pxNewMesh;
DWORD*			pxAttributes;
DWORD*			pxNewAttributes;

	if ( ( nModelHandle == NOTFOUND ) ||
		 ( pxModelData->pxBaseMesh == NULL ) )
	{
		return;
	}
	nNumFaces = pxModelData->xStats.nNumIndices / 3;
	nNumIndices = pxModelData->xStats.nNumIndices;

	EngineCreateMesh( nNumFaces, nNumIndices, &pxNewMesh, 0 );

	pxModelData->pxBaseMesh->LockAttributeBuffer( kLock_ReadOnly, (BYTE**)( &pxAttributes ) );
	pxNewMesh->LockAttributeBuffer( 0, (BYTE**)( &pxNewAttributes ) );
	
	for( nIndexLoop = 0; nIndexLoop < nNumFaces; nIndexLoop++ )
	{
		*pxNewAttributes++ = *pxAttributes++;

	}
	pxModelData->pxBaseMesh->UnlockAttributeBuffer();
	pxNewMesh->UnlockAttributeBuffer();
	
	pxNewMesh->LockVertexBuffer( 0, (BYTE**)&pxNewVertices );
	
	if ( pxNewMesh->Is32BitIndexBuffer() == TRUE )
	{
		pxNewMesh->LockIndexBuffer( 0, (BYTE**)&punNewIndices );
	}
	else
	{
		pxNewMesh->LockIndexBuffer( 0, (BYTE**)&puwNewIndices );
	}

	if ( pxModelData->pxBaseMesh->Is32BitIndexBuffer() == TRUE )
	{
		pxModelData->pxBaseMesh->LockIndexBuffer( NULL, (byte**)( &punIndices ) );
	}
	else
	{
		pxModelData->pxBaseMesh->LockIndexBuffer( NULL, (byte**)( &puwIndices ) );
	}
	pxModelData->pxBaseMesh->LockVertexBuffer( NULL, (byte**)( &pxVertices ) );

	if ( pxModelData->pxBaseMesh->Is32BitIndexBuffer() == TRUE )
	{
		if ( pxNewMesh->Is32BitIndexBuffer() == TRUE )
		{
			for ( nIndexLoop = 0; nIndexLoop < nNumIndices; nIndexLoop++ )
			{
				ulIndex = punIndices[nIndexLoop];
				pxNewVertices[nIndexLoop] = pxVertices[ulIndex];
				punNewIndices[nIndexLoop] = (uint32)( nIndexLoop );
			}
		}
		else
		{
			for ( nIndexLoop = 0; nIndexLoop < nNumIndices; nIndexLoop++ )
			{
				ulIndex = punIndices[nIndexLoop];
				pxNewVertices[nIndexLoop] = pxVertices[ulIndex];
				puwNewIndices[nIndexLoop] = (ushort)( nIndexLoop );
			}
		}
	}
	else if ( pxNewMesh->Is32BitIndexBuffer() == TRUE )
	{
		for ( nIndexLoop = 0; nIndexLoop < nNumIndices; nIndexLoop++ )
		{
			ulIndex = puwIndices[nIndexLoop];
			pxNewVertices[nIndexLoop] = pxVertices[ulIndex];
			punNewIndices[nIndexLoop] = (uint32)( nIndexLoop );
		}
	}
	else
	{	
		for ( nIndexLoop = 0; nIndexLoop < nNumIndices; nIndexLoop++ )
		{
			ulIndex = puwIndices[nIndexLoop];
			pxNewVertices[nIndexLoop] = pxVertices[ulIndex];
			puwNewIndices[nIndexLoop] = (ushort)( nIndexLoop );
		}
	}

	pxModelData->pxBaseMesh->UnlockVertexBuffer();
	pxModelData->pxBaseMesh->UnlockIndexBuffer();
	
	pxNewMesh->UnlockVertexBuffer();
	pxNewMesh->UnlockIndexBuffer();

	delete pxModelData->pxBaseMesh;
	pxModelData->pxBaseMesh = pxNewMesh;
	pxModelData->xStats.nNumVertices = nNumIndices;

	if ( pxModelData->xMedLODAttachData.nModelHandle != NOTFOUND )
	{
		ModelConvSeparateVerts( pxModelData->xMedLODAttachData.nModelHandle );
	}
	if ( pxModelData->xLowLODAttachData.nModelHandle != NOTFOUND )
	{
		ModelConvSeparateVerts( pxModelData->xLowLODAttachData.nModelHandle );
	}
}


/***************************************************************************
 * Function    : ModelConvFixInsideOutModel
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
void ModelConvFixInsideOutModel( int nHandle )
{
MODEL_RENDER_DATA*	pxModelData = maxModelRenderData;
CUSTOMVERTEX*	pxVertices;
ushort*	puwIndices;
ushort	uwIndex;
int		nVertLoop;
int		nVertsInMesh;
int		nFaceLoop;
int		nFacesInMesh;

	if ( nHandle != NOTFOUND )
	{
		pxModelData += nHandle;
		
		if ( pxModelData->pxBaseMesh != NULL )
		{
			nVertsInMesh = pxModelData->pxBaseMesh->GetNumVertices();
			nFacesInMesh = pxModelData->pxBaseMesh->GetNumFaces();
			pxModelData->pxBaseMesh->LockVertexBuffer( NULL, (byte**)( &pxVertices ) );

			for ( nVertLoop = 0; nVertLoop < nVertsInMesh; nVertLoop++ )
			{
				pxVertices->normal.x *= -1.0f;
				pxVertices->normal.y *= -1.0f;
				pxVertices->normal.z *= -1.0f;
				pxVertices++;
			}
			pxModelData->pxBaseMesh->UnlockVertexBuffer();

			pxModelData->pxBaseMesh->LockIndexBuffer( NULL, (byte**)( &puwIndices ) );
			if ( pxModelData->pxBaseMesh->Is32BitIndexBuffer() )
			{
			int*	punIndices;
			int		nIndex;
				punIndices = (int*)( puwIndices );
				for ( nFaceLoop = 0; nFaceLoop < nFacesInMesh; nFaceLoop++ )
				{
					nIndex = punIndices[1];
					punIndices[1] = punIndices[2];
					punIndices[2] = nIndex;
					punIndices += 3;
				}
			}
			else
			{
				for ( nFaceLoop = 0; nFaceLoop < nFacesInMesh; nFaceLoop++ )
				{
					uwIndex = puwIndices[1];
					puwIndices[1] = puwIndices[2];
					puwIndices[2] = uwIndex;
					puwIndices += 3;
				}
			}
			pxModelData->pxBaseMesh->UnlockIndexBuffer();
		}
	}
}


void ModelConvFixInsideOutModelSelected( int nHandle )
{
MODEL_RENDER_DATA*	pxModelData = maxModelRenderData;
CUSTOMVERTEX*	pxVertices;
ushort*	puwIndices;
ushort	uwIndex;
int		nVertLoop;
int		nVertsInMesh;
int		nFaceLoop;
int		nFacesInMesh;

	if ( nHandle != NOTFOUND )
	{
		pxModelData += nHandle;
		
		if ( pxModelData->pxBaseMesh != NULL )
		{
			nVertsInMesh = pxModelData->pxBaseMesh->GetNumVertices();
			nFacesInMesh = pxModelData->pxBaseMesh->GetNumFaces();
			pxModelData->pxBaseMesh->LockVertexBuffer( NULL, (byte**)( &pxVertices ) );

			for ( nVertLoop = 0; nVertLoop < nVertsInMesh; nVertLoop++ )
			{
				pxVertices->normal.x *= -1.0f;
				pxVertices->normal.y *= -1.0f;
				pxVertices->normal.z *= -1.0f;
				pxVertices++;
			}
			pxModelData->pxBaseMesh->UnlockVertexBuffer();

			pxModelData->pxBaseMesh->LockIndexBuffer( NULL, (byte**)( &puwIndices ) );
			if ( pxModelData->pxBaseMesh->Is32BitIndexBuffer() )
			{
			int*	punIndices;
			int		nIndex;
				punIndices = (int*)( puwIndices );
				for ( nFaceLoop = 0; nFaceLoop < nFacesInMesh; nFaceLoop++ )
				{
					nIndex = punIndices[1];
					punIndices[1] = punIndices[2];
					punIndices[2] = nIndex;
					punIndices += 3;
				}
			}
			else
			{
				for ( nFaceLoop = 0; nFaceLoop < nFacesInMesh; nFaceLoop++ )
				{
					uwIndex = puwIndices[1];
					puwIndices[1] = puwIndices[2];
					puwIndices[2] = uwIndex;
					puwIndices += 3;
				}
			}
			pxModelData->pxBaseMesh->UnlockIndexBuffer();
		}
	}
}




void	ModelConvConvertYUpToZUp( int nHandle )
{
MODEL_RENDER_DATA*	pxModelData = maxModelRenderData;
CUSTOMVERTEX*	pxVertices;
CUSTOMVERTEX*	pxVertBase;
int		nVertLoop;
int		nVertsInMesh;
float	fStoreY;

	if ( nHandle != NOTFOUND )
	{
		pxModelData += nHandle;

		if ( pxModelData->pxBaseMesh != NULL )
		{
			nVertsInMesh = pxModelData->pxBaseMesh->GetNumVertices();
			pxModelData->pxBaseMesh->LockVertexBuffer( NULL, (byte**)( &pxVertices ) );
			pxVertBase = pxVertices;
			for ( nVertLoop = 0; nVertLoop < nVertsInMesh; nVertLoop++ )
			{
				fStoreY = pxVertices->position.y;
				pxVertices->position.y = pxVertices->position.z;
				pxVertices->position.z = fStoreY;

				fStoreY = pxVertices->normal.y;
				pxVertices->normal.y = pxVertices->normal.z;
				pxVertices->normal.z = fStoreY;

				pxVertices++;
			}
			RenderingComputeBoundingBox( pxVertBase, nVertsInMesh,&pxModelData->xStats.xBoundMin, &pxModelData->xStats.xBoundMax );
			pxModelData->pxBaseMesh->UnlockVertexBuffer();
		}
	}

	// TODO - process animation vertex positions too ?

	ModelConvFixInsideOutModel( nHandle );
	ModelConvReverseNormals( nHandle );

}


/***************************************************************************
 * Function    : ModelConvReverseFaceOrientation
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
void ModelConvReverseFaceOrientation( int nHandle )
{
MODEL_RENDER_DATA*	pxModelData = maxModelRenderData;
CUSTOMVERTEX*	pxVertices;
int		nVertLoop;
int		nVertsInMesh;
int		nFaceLoop;
ushort		uwIndex;
int		nFacesInMesh;
ushort*			puwIndices;

	if ( nHandle != NOTFOUND )
	{
		pxModelData += nHandle;
		
		if ( pxModelData->pxBaseMesh != NULL )
		{
			nVertsInMesh = pxModelData->pxBaseMesh->GetNumVertices();
			pxModelData->pxBaseMesh->LockVertexBuffer( NULL, (byte**)( &pxVertices ) );

			for ( nVertLoop = 0; nVertLoop < nVertsInMesh; nVertLoop++ )
			{
				pxVertices->position.x *= -1.0f;
				pxVertices->normal.x *= -1.0f;
				pxVertices++;
			}
			pxModelData->pxBaseMesh->UnlockVertexBuffer();
			
			nFacesInMesh = pxModelData->pxBaseMesh->GetNumFaces();
			pxModelData->pxBaseMesh->LockIndexBuffer( NULL, (byte**)( &puwIndices ) );
			for ( nFaceLoop = 0; nFaceLoop < nFacesInMesh; nFaceLoop++ )
			{
				uwIndex = puwIndices[1];
				puwIndices[1] = puwIndices[2];
				puwIndices[2] = uwIndex;
				puwIndices += 3;
			}
			pxModelData->pxBaseMesh->UnlockIndexBuffer();
		}
	}
}


void	ModelConvDeleteIsolatedVertices( int nModelHandle )
{
MODEL_RENDER_DATA*	pxModelData = maxModelRenderData + nModelHandle;
CUSTOMVERTEX*		pxVertices;
CUSTOMVERTEX*		pxNewVertices;
ushort*			puwIndices;
ushort*			puwNewIndices;
int				nIndexLoop;
int				nNumFaces;
int				nNumIndices;
ushort			uwIndex;
EngineMesh*		pxNewMesh;
uint32*			pulNewVertexLookup;
uint32			ulNewVertCount = 0;
int				nNewNumVerts;
uint32			ulNewVertNum;

	if ( ( nModelHandle == NOTFOUND ) ||
		 ( pxModelData->pxBaseMesh == NULL ) )
	{
		return;
	}

	nNumFaces = pxModelData->xStats.nNumIndices / 3;
	nNumIndices = pxModelData->xStats.nNumIndices;

	pulNewVertexLookup = (uint32*)( malloc( pxModelData->xStats.nNumVertices * sizeof( uint32 ) ) );
	memset( pulNewVertexLookup, 0xFFFFFFFF, pxModelData->xStats.nNumVertices * sizeof( uint32 ) );

	pxModelData->pxBaseMesh->LockIndexBuffer( NULL, (byte**)( &puwIndices ) );

	for ( nIndexLoop = 0; nIndexLoop < nNumIndices; nIndexLoop++ )
	{
		// TODO - Cope with 32 bit index buffers
		uwIndex = puwIndices[nIndexLoop];

		if ( pulNewVertexLookup[ uwIndex ] == 0xFFFFFFFF )
		{
			pulNewVertexLookup[ uwIndex ] = ulNewVertCount;
			ulNewVertCount++;
		}
	}
	
	pxModelData->pxBaseMesh->UnlockIndexBuffer();

	nNewNumVerts = ulNewVertCount;


	EngineCreateMesh( nNumFaces, nNewNumVerts, &pxNewMesh, 0 );

	pxNewMesh->LockVertexBuffer( 0, (BYTE**)&pxNewVertices );
	pxNewMesh->LockIndexBuffer( 0, (BYTE**)&puwNewIndices );

	pxModelData->pxBaseMesh->LockIndexBuffer( NULL, (byte**)( &puwIndices ) );
	pxModelData->pxBaseMesh->LockVertexBuffer( NULL, (byte**)( &pxVertices ) );
	
	for ( nIndexLoop = 0; nIndexLoop < nNumIndices; nIndexLoop++ )
	{
		// TODO - Cope with 32 bit index buffers
		uwIndex = puwIndices[nIndexLoop];

		ulNewVertNum = pulNewVertexLookup[ uwIndex ];

		pxNewVertices[ulNewVertNum] = pxVertices[uwIndex];
		puwNewIndices[nIndexLoop] = (ushort)( ulNewVertNum );
	}

	pxModelData->pxBaseMesh->UnlockVertexBuffer();
	pxModelData->pxBaseMesh->UnlockIndexBuffer();
	
	pxNewMesh->UnlockVertexBuffer();
	pxNewMesh->UnlockIndexBuffer();

	free( pulNewVertexLookup );

	delete pxModelData->pxBaseMesh;
	pxModelData->pxBaseMesh = pxNewMesh;
	pxModelData->xStats.nNumVertices = nNewNumVerts;

}
