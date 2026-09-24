
#include <math.h>
#include "StandardDef.h"
#include "Engine.h"
#include "Rendering.h"
#include "../../LibCode/Engine/ModelRendering.h"
#include "../../LibCode/Engine/Loader.h"

#include "../Tools/RecalcNormals.h"
#include "../Tools/MeshManipulate.h"

#include "../ModelConverter.h"

#include "Disc.h"


void	AddPrimitiveDisc()
{
int		nNewHandle = ModelRenderGetNextHandle();
MODEL_RENDER_DATA*		pxModelData;
CUSTOMVERTEX*		pVertexBuffer;
CUSTOMVERTEX*		pVertexBufferBase;
ushort*				puwIndexBuffer;
float				fScale = 1.0f;
int				nLoop;
int				nNumSegments = 16;
int				nNumVerts = nNumSegments + 1;
int				nNumFaces = nNumSegments;
float		fAngle = 0.0f;
float		fAngleStep = A360 / nNumSegments;

	pxModelData = maxModelRenderData + nNewHandle;
	
	ModelConvInitialiseBlankModel( pxModelData, nNumVerts, nNumFaces );

	pxModelData->pxBaseMesh->LockVertexBuffer( 0, (BYTE**)&pVertexBufferBase );

	pVertexBuffer = pVertexBufferBase;
	for( nLoop = 0; nLoop < nNumSegments; nLoop++ )
	{
		pVertexBuffer->position = VECT( sinf(fAngle), cosf(fAngle), 0.0f );
		pVertexBuffer->color = 0xFFFFFFFF;
		pVertexBuffer->tu = 0.0f;
		pVertexBuffer->tv = 0.0f;

		pVertexBuffer++;
		fAngle += fAngleStep;
	}

	pVertexBuffer->position = VECT( 0.0f,0.0f, 0.0f );
	pVertexBuffer->color = 0xFFFFFFFF;
	pVertexBuffer->tu = 0.5f;
	pVertexBuffer->tv = 0.5f;

	RenderingComputeBoundingBox( pVertexBufferBase, pxModelData->xStats.nNumVertices, &pxModelData->xStats.xBoundMin, &pxModelData->xStats.xBoundMax );
	RenderingComputeBoundingSphere( pVertexBufferBase, pxModelData->xStats.nNumVertices, &pxModelData->xStats.xBoundSphereCentre, &pxModelData->xStats.fBoundSphereRadius );

	pxModelData->pxBaseMesh->UnlockVertexBuffer();
	
	pxModelData->pxBaseMesh->LockIndexBuffer( 0, (BYTE**)&puwIndexBuffer );
	for( nLoop = 0; nLoop < nNumFaces; nLoop++ )
	{
		puwIndexBuffer[(nLoop*3)] = nNumVerts - 1;
		puwIndexBuffer[(nLoop*3)+1] = nLoop;
		puwIndexBuffer[(nLoop*3)+2] = ( nLoop + 1 ) % (nNumVerts - 1);
	}

	pxModelData->pxBaseMesh->UnlockIndexBuffer();
	
	ModelConvSeparateVerts( nNewHandle );

	ModelConvFixNormals( nNewHandle, FLAT_FACES );

	ModelConvSetCurrentModel( nNewHandle );

}