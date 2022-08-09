
#include <StandardDef.h>
#include <Engine.h>
#include <Rendering.h>

#include "ModelRendering.h"

#include "Loader.h"


//----------------------------------------------
// ModelConvInitialiseBlankModel
//  Creates an empty model with the required number of faces and vertices
//----------------------------------------------
void	ModelConvInitialiseBlankModel( MODEL_RENDER_DATA* pxModelData, int nNumVerts, int nNumFaces )
{
CUSTOMVERTEX*	pxVertices;
int		nLoop;

	if ( nNumVerts > 65535 )
	{
		EngineCreateMesh( nNumFaces, nNumVerts, &pxModelData->pxBaseMesh, 2 );
	}
	else
	{
		// Create the mesh here now we know the vertex and face counts..
		EngineCreateMesh( nNumFaces, nNumVerts, &pxModelData->pxBaseMesh, 1 );
	}
	pxModelData->xStats.nNumVertices = nNumVerts;
	pxModelData->xStats.nNumIndices = nNumFaces * 3;

	pxModelData->pxBaseMesh->LockVertexBuffer( NULL, (byte**)( &pxVertices ) );
	for ( nLoop = 0; nLoop < nNumVerts; nLoop++ )
	{
		pxVertices[ nLoop ].color = 0xFFFFFFFF;
		pxVertices[ nLoop ].normal.x = 0.0f;
		pxVertices[ nLoop ].normal.y = 0.0f;
		pxVertices[ nLoop ].normal.z = 1.0f;
	}
	RenderingComputeBoundingBox( pxVertices, pxModelData->xStats.nNumVertices,&pxModelData->xStats.xBoundMin, &pxModelData->xStats.xBoundMax );
	pxModelData->pxBaseMesh->UnlockVertexBuffer( );

}
