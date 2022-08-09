
#include "StandardDef.h"
#include "Engine.h"
#include "Rendering.h"

#include "../../LibCode/Engine/ModelRendering.h"
#include "RecalcNormals.h"

void	ModelConvCalcNormals( eNormalFixMode nMode, const VECT* pxVert1, const VECT* pxVert2, const VECT* pxVert3, VECT* pxNormal1, VECT* pxNormal2,  VECT* pxNormal3 )
{
VECT	xCross1, xCross2, xCross3;
VECT	xDiff1, xDiff2;
VECT	xNormal;

	if ( nMode == SMOOTHED_NORMALS )
	{
		VectSub( &xDiff1, pxVert2, pxVert1 );
		VectSub( &xDiff2, pxVert3, pxVert1 );
		xNormal.x = (xDiff1.y * xDiff2.z) - (xDiff1.z * xDiff2.y);
		xNormal.y = (xDiff1.z * xDiff2.x) - (xDiff1.x * xDiff2.z);
		xNormal.z = (xDiff1.x * xDiff2.y) - (xDiff1.y * xDiff2.x);
		VectNormalize( &xNormal );
		VectAdd( pxNormal1, pxNormal1, &xNormal );
		VectNormalize( pxNormal1 );
		VectSub( &xDiff1, pxVert3, pxVert2 );
		VectSub( &xDiff2, pxVert1, pxVert2 );
		xNormal.x = (xDiff1.y * xDiff2.z) - (xDiff1.z * xDiff2.y);
		xNormal.y = (xDiff1.z * xDiff2.x) - (xDiff1.x * xDiff2.z);
		xNormal.z = (xDiff1.x * xDiff2.y) - (xDiff1.y * xDiff2.x);
		VectNormalize( &xNormal );
		VectAdd( pxNormal2, pxNormal2, &xNormal );
		VectNormalize( pxNormal2 );
		VectSub( &xDiff1, pxVert1, pxVert3 );
		VectSub( &xDiff2, pxVert2, pxVert3 );
		xNormal.x = (xDiff1.y * xDiff2.z) - (xDiff1.z * xDiff2.y);
		xNormal.y = (xDiff1.z * xDiff2.x) - (xDiff1.x * xDiff2.z);
		xNormal.z = (xDiff1.x * xDiff2.y) - (xDiff1.y * xDiff2.x);
		VectNormalize( &xNormal );
		VectAdd( pxNormal3, pxNormal3, &xNormal );
		VectNormalize( pxNormal3 );
	}
	else
	{
		VectCross( &xCross1, pxVert1, pxVert2 );
		VectCross( &xCross2, pxVert2, pxVert3 );
		VectCross( &xCross3, pxVert3, pxVert1 );

		xCross1.x = ( xCross1.x + xCross2.x + xCross3.x ) / 3.0f;
		xCross1.y = ( xCross1.y + xCross2.y + xCross3.y ) / 3.0f;
		xCross1.z = ( xCross1.z + xCross2.z + xCross3.z ) / 3.0f;

		VectNormalize( &xCross1 );
		*pxNormal1 = xCross1;
		*pxNormal2 = xCross1;
		*pxNormal3 = xCross1;
	}
}


void	ModelConvFixNormalsForHandle( int nHandle, eNormalFixMode nMode )
{
MODEL_RENDER_DATA*	pxModelData = maxModelRenderData;
CUSTOMVERTEX*	pxVertices;
CUSTOMVERTEX*	pxVerticesBase;
int		nVertLoop;
int		nVertsInMesh;
VECT	xVert1, xVert2, xVert3;
ushort	uwIndex1, uwIndex2, uwIndex3;
ushort*	puwIndices;
int		nFaceLoop;

	pxModelData += nHandle;

	if ( pxModelData->pxBaseMesh != NULL )
	{
		nVertsInMesh = pxModelData->pxBaseMesh->GetNumVertices();
		pxModelData->pxBaseMesh->LockVertexBuffer( NULL, (byte**)( &pxVerticesBase ) );
		pxModelData->pxBaseMesh->LockIndexBuffer( NULL, (byte**)( &puwIndices ) );
		
		pxVertices = pxVerticesBase;
		// First clear out all the normals
		for ( nVertLoop = 0; nVertLoop < nVertsInMesh; nVertLoop++ )
		{
			pxVertices->normal.x = 0.0f;
			pxVertices->normal.y = 0.0f;
			pxVertices->normal.z = 0.0f;
			pxVertices++;
		}

		pxVertices = pxVerticesBase;

		for ( nFaceLoop = 0; nFaceLoop < pxModelData->xStats.nNumIndices / 3; nFaceLoop++ )
		{
			uwIndex1 = puwIndices[0];
			uwIndex2 = puwIndices[1];
			uwIndex3 = puwIndices[2];

			xVert1 = *((VECT*)&pxVertices[uwIndex1].position);
			xVert2 = *((VECT*)&pxVertices[uwIndex2].position);
			xVert3 = *((VECT*)&pxVertices[uwIndex3].position);

			ModelConvCalcNormals( nMode, &xVert1, &xVert2, &xVert3, &pxVertices[uwIndex1].normal, &pxVertices[uwIndex2].normal, &pxVertices[uwIndex3].normal );
			puwIndices += 3;
		}

		pxModelData->pxBaseMesh->UnlockVertexBuffer();

		if ( pxModelData->pxNormalKeyframes != NULL )
		{
		int		nKeyframeLoop;
		VECT*	pxNormals;
		VECT*	pxNormal1;
		VECT*	pxNormal2;
		VECT*	pxNormal3;

			for ( nKeyframeLoop = 0; nKeyframeLoop < pxModelData->xStats.nNumVertKeyframes; nKeyframeLoop++ )
			{
				ModelRenderKeyframeAnimationUpdateBaseMeshFromKeyframes( pxModelData, pxModelData->pxVertexKeyframes + nKeyframeLoop );

				nVertsInMesh = pxModelData->pxBaseMesh->GetNumVertices();
				pxModelData->pxBaseMesh->LockVertexBuffer( NULL, (byte**)( &pxVerticesBase ) );
				pxModelData->pxBaseMesh->LockIndexBuffer( NULL, (byte**)( &puwIndices ) );
		
				pxVertices = pxVerticesBase;

				pxVertices = pxVerticesBase;
				pxNormals = pxModelData->pxNormalKeyframes + nKeyframeLoop;

				// First clear out all the normals
				for ( nVertLoop = 0; nVertLoop < pxModelData->xStats.nNumNormals; nVertLoop++ )
				{
					pxNormals->x = 0.0f;
					pxNormals->y = 0.0f;
					pxNormals->z = 0.0f;
					pxNormals += pxModelData->xStats.nNumVertKeyframes;
				}

				pxNormals = pxModelData->pxNormalKeyframes + nKeyframeLoop;
				for ( nFaceLoop = 0; nFaceLoop < pxModelData->xStats.nNumIndices / 3; nFaceLoop++ )
				{
					uwIndex1 = puwIndices[0];
					uwIndex2 = puwIndices[1];
					uwIndex3 = puwIndices[2];

					xVert1 = *((VECT*)&pxVertices[uwIndex1].position);
					xVert2 = *((VECT*)&pxVertices[uwIndex2].position);
					xVert3 = *((VECT*)&pxVertices[uwIndex3].position);

					pxNormal1 = pxNormals + (uwIndex1 * pxModelData->xStats.nNumVertKeyframes);
					pxNormal2 = pxNormals + (uwIndex2 * pxModelData->xStats.nNumVertKeyframes);
					pxNormal3 = pxNormals + (uwIndex3 * pxModelData->xStats.nNumVertKeyframes);
					ModelConvCalcNormals( nMode, &xVert1, &xVert2, &xVert3, pxNormal1, pxNormal2, pxNormal3 );
					puwIndices += 3;
				}

				pxModelData->pxBaseMesh->UnlockVertexBuffer();
			}
		}

	}
}

void	ModelConvFixNormals( int nHandle, eNormalFixMode nMode )
{
MODEL_RENDER_DATA*	pxModelData = maxModelRenderData;

	if ( nHandle != NOTFOUND )
	{
		pxModelData = maxModelRenderData + nHandle;
		ModelConvFixNormalsForHandle( nHandle, nMode );

		if ( pxModelData->xMedLODAttachData.nModelHandle != NOTFOUND )
		{
			ModelConvFixNormalsForHandle( pxModelData->xMedLODAttachData.nModelHandle, nMode );
		}
		if ( pxModelData->xLowLODAttachData.nModelHandle != NOTFOUND )
		{
			ModelConvFixNormalsForHandle( pxModelData->xLowLODAttachData.nModelHandle, nMode );
		}

		if ( pxModelData->xWheel1AttachData.nModelHandle != NOTFOUND )
		{
			ModelConvFixNormalsForHandle( pxModelData->xWheel1AttachData.nModelHandle, nMode );
		}

		if ( pxModelData->xWheel3AttachData.nModelHandle != NOTFOUND )
		{
			ModelConvFixNormalsForHandle( pxModelData->xWheel3AttachData.nModelHandle, nMode );
		}

		// todo - ETC
	}

}


void	ModelConvSetNormals( int nHandle, eNormalFixMode nMode )
{
MODEL_RENDER_DATA*	pxModelData = maxModelRenderData;
CUSTOMVERTEX*	pxVertices;
CUSTOMVERTEX*	pxVerticesBase;
int		nVertLoop;
int		nVertsInMesh;

	if ( nHandle != NOTFOUND )
	{
		pxModelData += nHandle;
		
		if ( pxModelData->pxBaseMesh != NULL )
		{
			nVertsInMesh = pxModelData->pxBaseMesh->GetNumVertices();
			pxModelData->pxBaseMesh->LockVertexBuffer( NULL, (byte**)( &pxVerticesBase ) );
			
			pxVertices = pxVerticesBase;
			for ( nVertLoop = 0; nVertLoop < nVertsInMesh; nVertLoop++ )
			{
				pxVertices->normal.x = 0.0f;
				pxVertices->normal.y = 0.0f;
				pxVertices->normal.z = 1.0f;
				pxVertices++;
			}
			pxModelData->pxBaseMesh->UnlockVertexBuffer();

 			if ( pxModelData->pxNormalKeyframes != NULL )
			{
			VECT*		pxNormals;

				pxNormals = pxModelData->pxNormalKeyframes;
				nVertsInMesh = pxModelData->xStats.nNumVertKeyframes * pxModelData->xStats.nNumNormals;
				for ( nVertLoop = 0; nVertLoop < nVertsInMesh; nVertLoop++ )
				{
					pxNormals->x = 0.0f;
					pxNormals->y = 0.0f;
					pxNormals->z = 1.0f;
					pxNormals++;
				}
			}
		}
	}
}


void	ModelConvReverseNormals( int nHandle )
{
MODEL_RENDER_DATA*	pxModelData = maxModelRenderData;
CUSTOMVERTEX*	pxVertices;
int		nVertLoop;
int		nVertsInMesh;
VECT*	pxVerts;

	if ( nHandle != NOTFOUND )
	{
		pxModelData += nHandle;
		
		if ( pxModelData->pxBaseMesh != NULL )
		{
			nVertsInMesh = pxModelData->pxBaseMesh->GetNumVertices();
			pxModelData->pxBaseMesh->LockVertexBuffer( NULL, (byte**)( &pxVertices ) );

			for ( nVertLoop = 0; nVertLoop < nVertsInMesh; nVertLoop++ )
			{
				pxVertices->normal.x *= -1.0f;
				pxVertices->normal.y *= -1.0f;
				pxVertices->normal.z *= -1.0f;
				VectNormalize( &pxVertices->normal );
				pxVertices++;
			}
			pxModelData->pxBaseMesh->UnlockVertexBuffer();

			if ( pxModelData->pxNormalKeyframes != NULL )
			{
				pxVerts = pxModelData->pxNormalKeyframes;
				nVertsInMesh = pxModelData->xStats.nNumVertKeyframes * pxModelData->xStats.nNumNormals;
				for ( nVertLoop = 0; nVertLoop < nVertsInMesh; nVertLoop++ )
				{
					pxVerts->x *= -1.0f;
					pxVerts->y *= -1.0f;
					pxVerts->z *= -1.0f;
					VectNormalize( pxVerts );
					pxVerts++;
				}
			}
		}
	}

}
