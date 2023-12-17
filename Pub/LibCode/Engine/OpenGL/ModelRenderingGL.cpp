
#include <windows.h>		// For OpenGL
#include <gl/gl.h>

#include <StandardDef.h>
#include <Interface.h>
#include <Engine.h>
#include <Rendering.h>

#include "../ModelMaterialData.h"
#include "../ModelRendering.h"
#include "ModelRenderingGL.h"

BOOL		ModelRayTest( int nModelHandle, const VECT* pxPos, const VECT* pxRot, const VECT* pxRayStart, const VECT* pxRayEnd, VECT* pxHit, VECT* pxHitNormal, int* pnFaceNum, int flags )
{

	return( FALSE );
}


int		EngineSimplifyMesh( int nModelHandle, float fReduceAmount, SIMPLIFICATION_PARAMS* pxParams )
{
	return( NOTFOUND );
}

void		RenderingComputeBoundingBox( const CUSTOMVERTEX* pVertices, int nNumVertices, VECT* pxBoundMin, VECT* pxBoundMax )
{

}


void		RenderingComputeBoundingSphere( const CUSTOMVERTEX* pVertices, int nNumVertices, VECT* pxSpherePos, float* pfSphereRadius )
{
}


/***************************************************************************
 * Function    : ModelRenderSetMaterialRenderStates
 * Params      :
 * Description : 
 ***************************************************************************/
void	ModelRenderSetMaterialRenderStates( MODEL_RENDER_DATA* pxModelData )
{
	if ( pxModelData->xGlobalProperties.bBackfaceFlag == 1 )
	{
//		mpEngineDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	}

	/*
	if ( pxModelData->xGlobalProperties.bSpriteFlags == 1 )
	{
		mboDrawAsSprite = TRUE;
	}
	else
	{
		mboDrawAsSprite = FALSE;
	}
*/
	if ( pxModelData->xGlobalProperties.bNoFiltering == 1 )
	{
		InterfaceSetGlobalParam( INTF_TEXTURE_FILTERING, 0 );
	}
	switch (pxModelData->xGlobalProperties.bBlendType) 
	{
	case MODEL_BLENDTYPES_NONE:
//		mpEngineDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE );
		break;
	case MODEL_BLENDTYPES_BLEND_ALPHA:
/*		mpEngineDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE );
		mpEngineDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		mpEngineDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		EngineEnableZWrite( FALSE );
*/		break;
	case MODEL_BLENDTYPES_ADDITIVE_ALPHA:
/*		mpEngineDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE );
		mpEngineDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		mpEngineDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
		EngineEnableZWrite( FALSE );
*/		break;
	case MODEL_BLENDTYPES_SUBTRACTIVE_ALPHA:
/*		mpEngineDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE );
		mpEngineDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
		mpEngineDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
*/		break;
	case MODEL_BLENDTYPES_BLEND_COLOUR:
/*		mpEngineDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE );
		mpEngineDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCCOLOR);
		mpEngineDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);
*/		break;
	case MODEL_BLENDTYPES_ADDITIVE_COLOUR:
/*		mpEngineDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE );
		mpEngineDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCCOLOR);
		mpEngineDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
*/		break;
	case MODEL_BLENDTYPES_SUBTRACTIVE_COLOUR:
/*		mpEngineDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE );
		mpEngineDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
		mpEngineDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);
*/		break;
	}
}


/***************************************************************************
 * Function    : RenderGLMesh
 * Params      :
 * Description : Draws the specified model
 ***************************************************************************/
int		RenderGLMesh( MODEL_RENDER_DATA* pxModelData )
{
int		ret = 0;
int		nPolyCount = 0;

//	mpEngineDevice->GetMaterial( &xBaseMaterial );

	if ( pxModelData->pxBaseMesh != NULL )
	{
	int		matloop;
	ModelMaterialData*		pMaterialData;
	int		nNumSubsetsToDraw = pxModelData->xStats.nNumMaterials;

		if ( nNumSubsetsToDraw < 1 ) nNumSubsetsToDraw = 1;
		for ( matloop = 0; matloop < nNumSubsetsToDraw; matloop++ )
		{
			// apply material texture and states if they're included in the model
			pMaterialData = FindMaterial( pxModelData, matloop );
			if ( pMaterialData )
			{
				ret = pMaterialData->Apply();
			}
/*
			if ( !mpRenderBaseTexture )
			{
				mpEngineDevice->GetTexture( 0, &mpRenderBaseTexture );
			}
*/
			pxModelData->pxBaseMesh->DrawSubset( matloop );

			// Restore standard blend modes, material etc if pMaterialData->Apply changed em..
			if ( ret & ModelMaterialData::BLEND_RENDER_STATES_CHANGED )
			{
				ModelRenderSetMaterialRenderStates( pxModelData );
			}
			if ( ret & ModelMaterialData::MATERIAL_CHANGED )
			{
//				mpEngineDevice->SetMaterial( &xBaseMaterial );
			}
			if ( ret & ModelMaterialData::SPECULAR_ACTIVATED )
			{
				EngineEnableSpecular( FALSE );
			}
			if ( ret & ModelMaterialData::DOUBLESIDED_ACTIVATED )
			{
				EngineEnableCulling( 1 );
			}
		}

		pxModelData->pxBaseMesh->DrawSubset( 0 );
		nPolyCount = (pxModelData->xStats.nNumIndices/3);

	}
	return( nPolyCount );
}

/***************************************************************************
 * Function    : ModelRenderImplGL
 * Params      :
 * Description : Draws the specified model
 ***************************************************************************/
int		ModelRenderImplGL( int nModelHandle, const VECT* pxPos, const VECT* pxRot, uint32 ulRenderFlags )
{
MODEL_RENDER_DATA*		pxModelData = &maxModelRenderData[ nModelHandle ];
uint32		ulTick = mulLastRenderingTick;
BOOL		boDoChangeMat = FALSE;
int			nLODToUse = 0;
int		nPolyCount = 0;

//	mpEngineDevice->GetTexture( 0, &mpRenderBaseTexture );
//	mboDrawAsSprite = FALSE;

	if ( pxModelData->pSkinnedModel )
	{
		return( 0 );
	}

/*
	if ( pxPos != NULL )
	{
		// temp - if the pos is set here, do all the basic rendering setup too.
		// this'll move to be based on the RenderFlags..
		if ( ulRenderFlags == 0 )
		{
			if ( pxModelData->xGlobalProperties.bBackfaceFlag == 0 )
			{
				mpEngineDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
			}
			mpEngineDevice->SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
			mpEngineDevice->SetRenderState( D3DRS_ALPHAREF,        0x1 );
			mpEngineDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );
			EngineEnableLighting( TRUE );
			EngineEnableZTest( TRUE );
		}
		EngineSetMatrixFromRotations( pxRot, &matWorld );
		matWorld._41 = pxPos->x;
		matWorld._42 = pxPos->y;
		matWorld._43 = pxPos->z;

		mpEngineDevice->SetTransform( D3DTS_WORLD, &matWorld );
	}

	mpEngineDevice->GetTransform( D3DTS_WORLD, &matOriginalWorld );
*/

	switch( pxModelData->bModelType )
	{
	case ASSETTYPE_KEYFRAME_ANIMATION:
		// Very ropey timing stuff on the animation here - just temp!!!
		if ( pxModelData->xAnimationState.ulLastFrameTick == 0 )
		{
			pxModelData->xAnimationState.ulLastFrameTick = ulTick;
			pxModelData->xAnimationState.ulNextFrameTick = ulTick + 300;
			pxModelData->xAnimationState.uwAnimNextFrame = 1;
		}
			
		ModelRenderKeyframeAnimationGenerateBaseMesh( pxModelData, ulTick );
		nPolyCount += RenderGLMesh( pxModelData );
		break;
	default:
		// If its a static mesh
		nPolyCount += RenderGLMesh( pxModelData );
		break;
	}

/*
	// --- If this is the base model, continue on to render the various submodels (wheels, turrets etc)
	if ( ulRenderFlags != 0xFF )
	{
		// Once we've rendered the base model, reset the texture back to the game-selected one
		// (As materials in the base model may have changed it, and subsequent models may well expect it)
		mpEngineDevice->SetTexture( 0, 	mpRenderBaseTexture );

		ModelRenderMaterialRestoreStatesPostRender( pxModelData );
		
		if ( pxModelData->xWheel1AttachData.nModelHandle > 0 )
		{
			nPolyCount += ModelRenderVertTurret( pxModelData, pxPos, pxRot, ulRenderFlags, matOriginalWorld, MODEL_ATTACHED_WHEEL_1 );
		}
		if ( pxModelData->xWheel2AttachData.nModelHandle > 0 )
		{
			nPolyCount += ModelRenderVertTurret( pxModelData, pxPos, pxRot, ulRenderFlags, matOriginalWorld, MODEL_ATTACHED_WHEEL_2 );
		}
		if ( pxModelData->xWheel3AttachData.nModelHandle > 0 )
		{
			nPolyCount += ModelRenderVertTurret( pxModelData, pxPos, pxRot, ulRenderFlags, matOriginalWorld, MODEL_ATTACHED_WHEEL_3 );
		}
		if ( pxModelData->xWheel4AttachData.nModelHandle > 0 )
		{
			nPolyCount += ModelRenderVertTurret( pxModelData, pxPos, pxRot, ulRenderFlags, matOriginalWorld, MODEL_ATTACHED_WHEEL_4 );
		}
		if ( pxModelData->xHorizTurretData.nModelHandle > 0 )
		{
			nPolyCount += ModelRenderAttachedModelBasic( pxModelData, pxPos, pxRot, ulRenderFlags, matOriginalWorld, MODEL_ATTACHED_TURRET_HORIZ );
		}
		if ( pxModelData->xVertTurretData.nModelHandle > 0 )
		{
			nPolyCount += ModelRenderVertTurret( pxModelData, pxPos, pxRot, ulRenderFlags, matOriginalWorld, MODEL_ATTACHED_TURRET_VERT );
		}
	}

	if ( ( pxModelData->xGlobalProperties.bNoLighting == 1 ) &&
		 ( ulRenderFlags == 0 ) )
	{
		EngineEnableLighting( TRUE );
	}
*/

	return( nPolyCount );
}



void	ModelRenderingPlatformInit( void )
{

}

void	ModelRenderingPlatformFree( void )
{

}