#include "StandardDef.h"
#include "Engine.h"
#include "Rendering.h"

#include "../../LibCode/Engine/ModelRendering.h"
#include "../Scene/ModelEditorSceneObject.h"
#include "../ModelConverter.h"
#include "VertexManipulate.h"

int		mnVertexManipulateControlMode = VERTEX_MANIPULATE_MOVE;
VECT*	mpbVertexBasePositions = NULL;


void		VertexManipulateMouseDownStore( CSceneObject* pSceneObject )
{
int		nNumVertsSelected = pSceneObject->GetNumVerticesSelected();

	if ( nNumVertsSelected > 0 )
	{
	MODEL_RENDER_DATA*		pxModelData = maxModelRenderData + pSceneObject->GetModelHandle();
	BYTE*					pbVertexSelections = pSceneObject->GetVertexSelections();
	int			nVertsInMesh;
	int			nVertLoop;
	VECT*		pxVertexStore;
	CUSTOMVERTEX*	pxVertices;
	
		// todo - copy all the (selected) vertices from the current mesh
		mpbVertexBasePositions = (VECT*)malloc( nNumVertsSelected * sizeof( VECT ) );
		pxVertexStore = mpbVertexBasePositions;

		nVertsInMesh = pxModelData->xStats.nNumVertices;

		pxModelData->pxBaseMesh->LockVertexBuffer( NULL, (byte**)( &pxVertices ) );

		for ( nVertLoop = 0; nVertLoop < nVertsInMesh; nVertLoop++ )
		{
			if ( *pbVertexSelections )
			{
				*pxVertexStore = pxVertices->position;
				pxVertexStore++;
			}
			pxVertices++;
			pbVertexSelections++;
		}
		pxModelData->pxBaseMesh->UnlockVertexBuffer();

	}
}

void		VertexManipulateMouseMoveUpdate( CSceneObject* pSceneObject, float fScreenDeltaX, float fScreenDeltaY )
{
MODEL_RENDER_DATA*		pxModelData = maxModelRenderData + pSceneObject->GetModelHandle();
BYTE*					pbVertexSelections = pSceneObject->GetVertexSelections();
int			nVertsInMesh;
int			nVertLoop;
VECT*		pxVertexStore;
CUSTOMVERTEX*	pxVertices;
VECT		xMove;

	if ( pxModelData->pxBaseMesh )
	{
		xMove = ModelConvGetScreenMoveVector( fScreenDeltaX, fScreenDeltaY, 0.01f );

		pxVertexStore = mpbVertexBasePositions;
		// todo - update the mesh's vertex positions by applying an offset to the previously stored positions
		nVertsInMesh = pxModelData->xStats.nNumVertices;

		pxModelData->pxBaseMesh->LockVertexBuffer( NULL, (byte**)( &pxVertices ) );

		for ( nVertLoop = 0; nVertLoop < nVertsInMesh; nVertLoop++ )
		{
			if ( *pbVertexSelections )
			{
				VectAdd( &pxVertices->position, pxVertexStore, &xMove );
				pxVertexStore++;
			}
			pxVertices++;
			pbVertexSelections++;
		}
		pxModelData->pxBaseMesh->UnlockVertexBuffer();
		pSceneObject->OnModelChanged( FALSE );
	}
}

void		VertexManipulateSetControlMode( int mode )
{
	// set the current control mode (move, rotate, scale)
	mnVertexManipulateControlMode = mode;
}

void		VertexManipulateMouseUp( CSceneObject* pSceneObject )
{
	// free up our base store
	if ( mpbVertexBasePositions )
	{
		free( mpbVertexBasePositions );
		mpbVertexBasePositions = NULL;

		pSceneObject->OnModelChanged( FALSE );
	}
}

