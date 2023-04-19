
#include "EngineDX.h"

#include <stdio.h>
#include <time.h>


#include <Engine.h>
#include "EngineDX.h"

#include <StandardDef.h>
#include <Rendering.h>
#include <Interface.h>
#include "System.h"

#include "../SkinnedModel.h"
#include "../BSPRender/BSPModel.h"
#include "../ModelArchive.h"
#include "../ModelMaterialData.h"
#include "../ModelRendering.h"

#include "ShadowMapDX.h"
#include "ShadowVolumeDX.h"
#include "ModelRenderingDX.h"
 

//----------------------------------------------------------------------------------------------------
BOOL	mboDrawAsSprite = FALSE;

D3DXMATERIAL*	mpLastLoadMaterialBuffer = NULL;
LPD3DXBUFFER	mpD3DXMtrlLoadBuffer = NULL;
LPD3DXBUFFER	mpD3DXAdjacencyLoadBuffer = NULL;
int			mnLastLoadNumMaterials = 1;

IDirect3DBaseTexture9* 	mpRenderBaseTexture = NULL;


void	ModelRenderingPlatformInit( void )
{

}

void	ModelRenderingPlatformFree( void )
{

}

int		EngineSimplifyMesh( int nModelHandle, float fReduceAmount, SIMPLIFICATION_PARAMS* pxParams )
{
#if 0
D3DXATTRIBUTEWEIGHTS	xSimplifyWeights;
MODEL_RENDER_DATA*		pxModelData;

	xSimplifyWeights.Tex[0] = 1.0f;
	if ( pxParams )
	{

	}
	pxModelData = &maxModelRenderData[ nModelHandle ];

	// TODO - Will need to optimise animated models too..

//	pxModelData->pxBaseMesh
#endif
	return( NOTFOUND );
}


/***************************************************************************
 * Function    : ModelRenderVertTurret
 * Params      :
 * Description : Draws the specified model
 ***************************************************************************/
int	ModelRenderAttachedModel( MODEL_RENDER_DATA*	pxModelData, const VECT* pxPos, const VECT* pxRot, ulong ulRenderFlags, D3DXMATRIX	matOriginalWorld, int nWhichAttachedModel, const ENGINEQUATERNION* pxQuat )
{
MODEL_RENDER_DATA*		pxSubModelData;
CUSTOMVERTEX* pxVertices;
CUSTOMVERTEX* pxVert;
VECT			xPos;
D3DXMATRIX	matWorld;
D3DXMATRIX	matRotX;
D3DXMATRIX	matRotY;
D3DXMATRIX	matRotZ;
D3DXVECTOR3	xRot;
VECT		xAttachPointTransformed;
VECT		xAttachPoint;
int		nRenderHandle;
int		nAttachVertex;
float	fZRotOffset = 0.0f;
float	fWheelTurn = 0.0f;
int		nHorizTurretAttachVertex = -1;

	if ( pxModelData->pxBaseMesh != NULL )
	{
		// Get the rotation part of the world matrix used to draw the main model
		matWorld = matOriginalWorld;
		matWorld._41 = 0.0f;
		matWorld._42 = 0.0f;
		matWorld._43 = 0.0f;

		xRot.x = 0.0f;
		xRot.y = 0.0f;
		xRot.z = 0.0f;

		switch( nWhichAttachedModel )
		{
		case MODEL_ATTACHED_TURRET_VERT:
			nRenderHandle = pxModelData->xVertTurretData.nModelHandle;
			nAttachVertex = pxModelData->xVertTurretData.nAttachVertex;

			// Check if we've got an attachvertex in the horiz turret (if there is one) and use that instead
			if ( pxModelData->xHorizTurretData.nModelHandle != NOTFOUND )
			{
			MODEL_RENDER_DATA*		pxHorizTurretModelData;
				pxHorizTurretModelData = &maxModelRenderData[ pxModelData->xHorizTurretData.nModelHandle ];
				if ( pxHorizTurretModelData->xVertTurretData.nAttachVertex != 0 )
				{
					nHorizTurretAttachVertex =  pxHorizTurretModelData->xVertTurretData.nAttachVertex;
				}
			}

			xRot.x = pxModelData->xVertTurretData.xCurrentRotations.x;
			if ( pxRot )
			{
				xRot.z = pxModelData->xHorizTurretData.xCurrentRotations.z;// - pxRot->z;
			}
			else
			{
				xRot.z = pxModelData->xHorizTurretData.xCurrentRotations.z;
			}
			break;
		case MODEL_ATTACHED_WHEEL_1:
			nRenderHandle = pxModelData->xWheel1AttachData.nModelHandle;
			nAttachVertex = pxModelData->xWheel1AttachData.nAttachVertex;
			// If its a plane, the 'wheels' (i.e. the props) rotate about the x axis )
			switch ( pxModelData->bWheelDisplayMode )
			{
			case 0:   // 4 wheels, x-forward
			default:
				xRot.y = TwoPi - pxModelData->xWheel1AttachData.xCurrentRotations.y;
				fWheelTurn = pxModelData->xWheel1AttachData.xCurrentRotations.z;
				xRot.z = fWheelTurn;
				break;
			case 1:  // plane 1
				xRot.x = pxModelData->xWheel1AttachData.xCurrentRotations.x;
				break;
			case 2:  // plane 2
				xRot.x = pxModelData->xWheel1AttachData.xCurrentRotations.x;
				break;
			case 3:   // helicopter
			case 4:
				xRot.z = pxModelData->xWheel1AttachData.xCurrentRotations.x;
				xRot.z = pxModelData->xWheel1AttachData.xCurrentRotations.x;
				break;
			case 5:		// bike
				xRot.y = TwoPi - pxModelData->xWheel1AttachData.xCurrentRotations.y;
				fWheelTurn = pxModelData->xWheel1AttachData.xCurrentRotations.z;
				xRot.z = fWheelTurn;
				break;
			case 6:     // 4-wheels, y-forward
				xRot.x = TwoPi - pxModelData->xWheel1AttachData.xCurrentRotations.y;
				fWheelTurn = pxModelData->xWheel1AttachData.xCurrentRotations.z;
				xRot.z = fWheelTurn;
				break;
			case 7:     // bike, y-forward
				xRot.x = TwoPi - pxModelData->xWheel1AttachData.xCurrentRotations.y;
				fWheelTurn = pxModelData->xWheel1AttachData.xCurrentRotations.z;
				xRot.z = fWheelTurn;
				break;
			case 8:  // plane 1  (y-forward)
				xRot.y = pxModelData->xWheel1AttachData.xCurrentRotations.x;
				break;
			case 9:  // plane 2   (y-forward)
				xRot.y = pxModelData->xWheel1AttachData.xCurrentRotations.x;
				break;
			}
			break;
		case MODEL_ATTACHED_WHEEL_2:
			nRenderHandle = pxModelData->xWheel2AttachData.nModelHandle;
			nAttachVertex = pxModelData->xWheel2AttachData.nAttachVertex;
			switch ( pxModelData->bWheelDisplayMode )
			{
			case 0:		// 4 wheels
				xRot.y = TwoPi-pxModelData->xWheel2AttachData.xCurrentRotations.y;
				break;
			case 6:		 // 4-wheels, y-forward
				xRot.x = TwoPi-pxModelData->xWheel2AttachData.xCurrentRotations.y;
				break;
			default:
				return( 0 );
				break;
			}
			fWheelTurn = pxModelData->xWheel2AttachData.xCurrentRotations.z;
			xRot.z = fWheelTurn + PI;
			fZRotOffset = PI;
			break;
		case MODEL_ATTACHED_WHEEL_3:
			nRenderHandle = pxModelData->xWheel3AttachData.nModelHandle;
			nAttachVertex = pxModelData->xWheel3AttachData.nAttachVertex;
			// If its a plane, the 'wheels' (i.e. the props) rotate about the x axis )
			switch ( pxModelData->bWheelDisplayMode )
			{
			case 1:		// plane 1 (x-forward)
				xRot.x = pxModelData->xWheel1AttachData.xCurrentRotations.x;
				break;
			case 2:    // plane 2 (x-forward)
				return( 0 );
				break;
			case 6:		// 4 wheels, y-forward
			case 7:		// bike, y-forward
			case 8:		// plane1, y-forward
			case 9:		// plane2, y-forward
				xRot.x = TwoPi - pxModelData->xWheel3AttachData.xCurrentRotations.y;
				break;
			case 0:
			default:
				xRot.y = TwoPi - pxModelData->xWheel3AttachData.xCurrentRotations.y;
				break;
			}
			break;
		case MODEL_ATTACHED_WHEEL_4:
			switch ( pxModelData->bWheelDisplayMode )
			{
			case 0:
				xRot.y = TwoPi-pxModelData->xWheel4AttachData.xCurrentRotations.y;
				break;
			case 6:
				xRot.x = TwoPi-pxModelData->xWheel4AttachData.xCurrentRotations.y;
				break;
			default:
				return( 0 );
				break;
			}
			nRenderHandle = pxModelData->xWheel4AttachData.nModelHandle;
			nAttachVertex = pxModelData->xWheel4AttachData.nAttachVertex;
			xRot.z = PI;
			break;
		default:
			return( 0 );
			break;
		}

		// Calculate the current position of the attach point on the base model, by 
		// transforming it with the world rotation
		if ( nHorizTurretAttachVertex != -1 )
		{
		MODEL_RENDER_DATA*		pxHorizTurretModelData;
		VECT					xOffsetTransformed;

			pxHorizTurretModelData = &maxModelRenderData[ pxModelData->xHorizTurretData.nModelHandle ];
			if ( pxHorizTurretModelData->pxBaseMesh )
			{
#ifndef SIMPLE
				EngineMatrixIdentity( &matWorld );

				EngineMatrixRotationZ( &matRotZ, xRot.z );
				if ( pxRot )
				{
					EngineMatrixMultiply( &matWorld, &matRotZ );
	
					EngineMatrixRotationX( &matRotX, pxRot->x ); 
					EngineMatrixRotationY( &matRotY, pxRot->y ); 
				    EngineMatrixRotationZ( &matRotZ, pxRot->z );

					EngineMatrixMultiply( &matWorld, &matRotX );
					EngineMatrixMultiply( &matWorld, &matRotY );
					EngineMatrixMultiply( &matWorld, &matRotZ );
				}
				else
				{
					matWorld = matOriginalWorld;
					EngineMatrixMultiply( &matWorld, &matRotZ );
				}

				pxHorizTurretModelData->pxBaseMesh->LockVertexBuffer( kLock_ReadOnly , (byte**)( &pxVertices ) );
				pxVert = pxVertices + nHorizTurretAttachVertex;
				xAttachPoint = pxVert->position;	
				VectTransform( &xAttachPointTransformed, &pxVert->position, &matWorld );
				VectTransform( &xOffsetTransformed, &pxModelData->xHorizTurretData.xAttachOffset, &matWorld );
				VectAdd( &xAttachPointTransformed, &xAttachPointTransformed, &xOffsetTransformed );
				pxModelData->pxBaseMesh->UnlockVertexBuffer();		

				// If the horizontal turret is also attached, we'll need to apply that transform to the position too
				if ( pxModelData->xHorizTurretData.nAttachVertex != 0 )
				{
				VECT	xHorizAttachTransformed;
				VECT	xOffsetTransformed;

					matWorld = matOriginalWorld;
					matWorld._41 = 0.0f;
					matWorld._42 = 0.0f;
					matWorld._43 = 0.0f;
	
					pxModelData->pxBaseMesh->LockVertexBuffer( kLock_ReadOnly , (byte**)( &pxVertices ) );
					pxVert = pxVertices + pxModelData->xHorizTurretData.nAttachVertex;
					VectTransform( &xHorizAttachTransformed, &pxVert->position, &matWorld );
					VectTransform( &xOffsetTransformed, &pxModelData->xHorizTurretData.xAttachOffset, &matWorld );
					VectAdd( &xHorizAttachTransformed, &xHorizAttachTransformed, &xOffsetTransformed );
					pxModelData->pxBaseMesh->UnlockVertexBuffer();		

					VectAdd( &xAttachPointTransformed, &xAttachPointTransformed, &xHorizAttachTransformed );
				}
#else
				pxHorizTurretModelData->pxBaseMesh->LockVertexBuffer( kLock_ReadOnly , (byte**)( &pxVertices ) );
				pxVert = pxVertices + nHorizTurretAttachVertex;
				xAttachPoint = pxVert->position;	
				D3DXVec3TransformCoord( (D3DXVECTOR3*)&xAttachPointTransformed, (D3DXVECTOR3*)&pxVert->position, &matWorld );
				pxModelData->pxBaseMesh->UnlockVertexBuffer();

				// I believe we should really transform the attachpoint by the matrix thats applied to the horizontal turret, rather
				// than just the rotation (and i believe thats why the vert turret floats a bit on the tank)
				VectRotateAboutZ( &xAttachPointTransformed,	pxModelData->xHorizTurretData.xCurrentRotations.z );
#endif
			}
		}
		else if ( nAttachVertex > 0 )
		{
			pxModelData->pxBaseMesh->LockVertexBuffer( kLock_ReadOnly , (byte**)( &pxVertices ) );
			pxVert = pxVertices + nAttachVertex;
			xAttachPoint = pxVert->position;
			switch( nWhichAttachedModel )
			{
			case MODEL_ATTACHED_WHEEL_1:
				pxModelData->xWheel1AttachData.xRawOffset = xAttachPoint;
				break;
			case MODEL_ATTACHED_WHEEL_2:
				pxModelData->xWheel2AttachData.xRawOffset = xAttachPoint;
				break;
			case MODEL_ATTACHED_WHEEL_3:
				pxModelData->xWheel3AttachData.xRawOffset = xAttachPoint;
				break;
			case MODEL_ATTACHED_WHEEL_4:
				pxModelData->xWheel4AttachData.xRawOffset = xAttachPoint;
				break;
			default:
				break;
			}
			
			VectTransform( &xAttachPointTransformed, &pxVert->position, &matWorld );
			pxModelData->pxBaseMesh->UnlockVertexBuffer();
		}
		else
		{
			xAttachPointTransformed.x = 0.0f;
			xAttachPointTransformed.y = 0.0f;
			xAttachPointTransformed.z = 0.0f;
		}

		// The position the vert turret is drawn in is the original world position of the model plus
		// the transformed position of the attach point.
		xPos.x = matOriginalWorld._41 + xAttachPointTransformed.x;
		xPos.y = matOriginalWorld._42 + xAttachPointTransformed.y;
		xPos.z = matOriginalWorld._43 + xAttachPointTransformed.z;
	
		EngineMatrixIdentity( &matWorld );

		pxSubModelData = &maxModelRenderData[ nRenderHandle ];

#ifdef WORKING_METHOD
		// We should have a copy of the base vertices
		if ( pxSubModelData->pxBaseVertices != NULL )
		{
		D3DXMATRIX	matRotX;
		D3DXMATRIX	matRotY;
		D3DXMATRIX	matRotZ;

			// Apply the current rotation about the vertical to vertices of 
			// the turret (this aint efficient, but needs to be done separately to
			// avoid weirdness with the matrix calculations (which results in the turrets rotating off all over the place)
			ModelRenderVertTurretTransformVerticesY( pxSubModelData, 0.0f- xRot.y );
			if ( pxRot != NULL )
			{
				D3DXMatrixRotationX( &matRotX, pxRot->x ); 
				D3DXMatrixRotationY( &matRotY, pxRot->y ); 
			    D3DXMatrixRotationZ( &matRotZ, pxRot->z + fWheelTurn );

				D3DXMatrixMultiply( &matWorld, &matRotX, &matRotY );
				D3DXMatrixMultiply( &matWorld, &matWorld, &matRotZ );
			}
		}
		else
		{		// Old method, just using multiplied matrices ends up with turrets rotating off
			D3DXMatrixRotationY( &matRotY, xRot.y ); 
		    D3DXMatrixRotationZ( &matRotZ, xRot.z + fZRotOffset );
			D3DXMatrixMultiply( &matWorld, &matRotZ, &matRotY );
		}
#else
		{
		ENGINEMATRIX	matRotX;
		ENGINEMATRIX	matRotY;
		ENGINEMATRIX	matRotZ;

			EngineMatrixRotationX( &matRotX, xRot.x ); 
			EngineMatrixRotationY( &matRotY, xRot.y ); 
		    EngineMatrixRotationZ( &matRotZ, xRot.z );
			matWorld = matRotX;
			EngineMatrixMultiply( &matWorld, &matRotY );
			EngineMatrixMultiply( &matWorld, &matRotZ );
	
			if ( pxRot != NULL )
			{
				EngineMatrixRotationX( &matRotX, pxRot->x ); 
				EngineMatrixRotationY( &matRotY, pxRot->y ); 
			    EngineMatrixRotationZ( &matRotZ, pxRot->z );

				EngineMatrixMultiply( &matWorld, &matRotX );
				EngineMatrixMultiply( &matWorld, &matRotY );
				EngineMatrixMultiply( &matWorld, &matRotZ );
			}
			else	
			{
				EngineMatrixMultiply( &matWorld, &matOriginalWorld );
				if ( (ulRenderFlags & RENDER_FLAGS_SHADOW_PASS) == RENDER_FLAGS_SHADOW_PASS )
				{
					// bodge.. stuff below assumes its a shadow rend, which (in the game) it usually is if it gets here.
				    EngineMatrixRotationZ( &matRotZ, xRot.z - (PI*0.5f) );
					matWorld = matRotX;
					EngineMatrixMultiply( &matWorld, &matRotY );
					EngineMatrixMultiply( &matWorld, &matRotZ );
#ifndef TOOL
					EngineGetGameInterface()->GetShadowMatrix( &matRotX, (VECT*)&xAttachPoint );
#else
					D3DXVECTOR3	xPlane1(- xAttachPoint.x,- xAttachPoint.y, - xAttachPoint.z );
					D3DXVECTOR3	xPlane2(1.0f- xAttachPoint.x,-1.0f-xAttachPoint.y, - xAttachPoint.z );
					D3DXVECTOR3	xPlane3(1.0f- xAttachPoint.x,1.0f- xAttachPoint.y, - xAttachPoint.z );
					D3DXVECTOR4	xLightPt(-200.0f,200.0f,120.0f,0.0f);
					D3DXPLANE	xShadowPlane;
					D3DXPlaneFromPoints( &xShadowPlane, &xPlane1, &xPlane2, &xPlane3 );
					// Set project to plane matrix
					D3DXMatrixShadow( (D3DXMATRIX*)&matRotX, &xLightPt, &xShadowPlane );
#endif
					EngineMatrixMultiply( &matWorld, &matRotX );
					xPos.x = matWorld._41;
					xPos.y = matWorld._42;
					xPos.z = matWorld._43;
				}
			}
		}
#endif

		matWorld._41 = xPos.x;
		matWorld._42 = xPos.y;
		matWorld._43 = xPos.z;
		EngineSetWorldMatrix( &matWorld );

		switch( nWhichAttachedModel )
		{
		case MODEL_ATTACHED_WHEEL_1:
			pxModelData->xWheel1AttachData.xCurrentOrigin = xPos;
			break;
		case MODEL_ATTACHED_WHEEL_2:
			pxModelData->xWheel2AttachData.xCurrentOrigin = xPos;
			break;
		case MODEL_ATTACHED_WHEEL_3:
			pxModelData->xWheel3AttachData.xCurrentOrigin = xPos;
			break;
		case MODEL_ATTACHED_WHEEL_4:
			pxModelData->xWheel4AttachData.xCurrentOrigin = xPos;
			break;
		default:
			break;
		}

		// Now we've set the world matrix we need, jus render the model
		ModelRender( nRenderHandle, NULL, NULL, 0 );

		// Special case for vertical turrets..
		if ( nWhichAttachedModel == MODEL_ATTACHED_TURRET_VERT )
		{
			// If the vert turret has a weapon attach vertex,
			// calculate the transformed point of the vertex and store it in the root model
			if ( pxSubModelData->xAttachData.xGenericWeaponFireAttach.nAttachVertex != 0 )
			{
			VECT	xOffsetTransformed;

				pxSubModelData->pxBaseMesh->LockVertexBuffer( kLock_ReadOnly , (byte**)( &pxVertices ) );
				pxVert = pxVertices + pxSubModelData->xAttachData.xGenericWeaponFireAttach.nAttachVertex;
				VectTransform( &xAttachPointTransformed, &pxVert->position, &matWorld );
				matWorld._41 = 0.0f;
				matWorld._42 = 0.0f;
				matWorld._43 = 0.0f;
				VectTransform( &xOffsetTransformed, &pxSubModelData->xAttachData.xGenericWeaponFireAttach.xAttachOffset, &matWorld );
				VectAdd( &xAttachPointTransformed, &xAttachPointTransformed, &xOffsetTransformed );

				pxSubModelData->pxBaseMesh->UnlockVertexBuffer();
				pxModelData->xAttachData.xGenericWeaponFireAttach.xTransformedPos.x = xAttachPointTransformed.x;
				pxModelData->xAttachData.xGenericWeaponFireAttach.xTransformedPos.y = xAttachPointTransformed.y;
				pxModelData->xAttachData.xGenericWeaponFireAttach.xTransformedPos.z = xAttachPointTransformed.z;		
				pxModelData->xAttachData.xGenericWeaponFireAttach.boTransformValid = TRUE;
			}
		}
		return( pxModelData->xStats.nNumIndices/3 );
	}
	return( 0 );

}


/***************************************************************************
 * Function    : ModelRenderMaterialRestoreStatesPostRender
 * Params      :
 * Description : Draws the specified model
 ***************************************************************************/
void	ModelRenderMaterialRestoreStatesPostRender( MODEL_RENDER_DATA* pxModelData )
{
	// Note if we're a low lod we probably don't want to turn this back on
	if ( pxModelData->xGlobalProperties.bNoFiltering == 1 )
	{
		// If you're using the engine.lib without the interface.dll then ya'll probably need
	// to remove this line.. nae biggee.. :)
		InterfaceSetGlobalParam( INTF_TEXTURE_FILTERING, 1 );
	}

	if ( pxModelData->xGlobalProperties.bDontClampUVs == 1 )
	{
		EngineEnableTextureAddressClamp( 1 );
	}

	if ( pxModelData->xGlobalProperties.bBackfaceFlag == 1 )
	{
		mpEngineDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
	}

	if (pxModelData->xGlobalProperties.bBlendType != MODEL_BLENDTYPES_NONE )
	{
		EngineEnableZWrite( TRUE );
		EngineEnableBlend( TRUE );
		EngineSetBlendMode( BLEND_MODE_ALPHABLEND );
	}

}

/***************************************************************************
 * Function    : ModelRenderSetMaterialRenderStates
 * Params      :
 * Description : 
 ***************************************************************************/
void	ModelRenderSetMaterialRenderStates( MODEL_RENDER_DATA* pxModelData )
{
#ifdef TUD11
	PANIC_IF( TRUE, "DX11 ModelRenderSetMaterialRenderStates TBI" );
#else
	if ( pxModelData->xGlobalProperties.bBackfaceFlag == 1 )
	{
		mpEngineDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	}

	if ( pxModelData->xGlobalProperties.bSpriteFlags == 1 )
	{
		mboDrawAsSprite = TRUE;
	}
	else
	{
		mboDrawAsSprite = FALSE;
	}

	// ? if we're a low lod we don't have the filtering flag set in old models, so we should
	// not do this and let in inherit from its owner
	if ( pxModelData->xGlobalProperties.bNoFiltering == 1 )
	{
		InterfaceSetGlobalParam( INTF_TEXTURE_FILTERING, 0 );
	}
	else
	{
		InterfaceSetGlobalParam( INTF_TEXTURE_FILTERING, 1 );
	}

	if ( pxModelData->xGlobalProperties.bDontClampUVs == 1 )
	{
		EngineEnableTextureAddressClamp( 0 );
	}

	switch (pxModelData->xGlobalProperties.bBlendType) 
	{
	case MODEL_BLENDTYPES_NONE:
		if ( EngineGetMaterialBlendOverride() == FALSE )
		{
			EngineEnableBlend( FALSE );
		} 
//		EngineEnableZWrite( TRUE );
		break;
	case MODEL_BLENDTYPES_BLEND_ALPHA_NO_CUTOFF:
	case MODEL_BLENDTYPES_BLEND_ALPHA:
		EngineEnableBlend( TRUE );
		mpEngineDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		mpEngineDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		EngineEnableZWrite( FALSE );
		break;
	case MODEL_BLENDTYPES_ADDITIVE_ALPHA:
		EngineEnableBlend( TRUE );
		mpEngineDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		mpEngineDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
		EngineEnableZWrite( FALSE );
		break;
	case MODEL_BLENDTYPES_SUBTRACTIVE_ALPHA:
		EngineEnableBlend( TRUE );
		mpEngineDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
		mpEngineDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		break;
	case MODEL_BLENDTYPES_BLEND_COLOUR:
		EngineEnableBlend( TRUE );
		mpEngineDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCCOLOR);
		mpEngineDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);
		break;
	case MODEL_BLENDTYPES_ADDITIVE_COLOUR:
		EngineEnableBlend( TRUE );
		mpEngineDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCCOLOR);
		mpEngineDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
		break;
	case MODEL_BLENDTYPES_SUBTRACTIVE_COLOUR:
		EngineEnableBlend( TRUE );
		mpEngineDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
		mpEngineDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);
		break;
	}
#endif
}


/***************************************************************************
 * Function    : ModelRenderAttachedModelBasic
 * Params      :
 * Description : Draws the specified model
 ***************************************************************************/
int		ModelRenderAttachedModelBasic( MODEL_RENDER_DATA*	pxModelData, const VECT* pxPos, const VECT* pxRot, ulong ulRenderFlags, D3DXMATRIX	matOriginalWorld, int nWhichAttachedModel )
{
MODEL_RENDER_DATA*		pxSubModelData = NULL;
CUSTOMVERTEX* pxVertices;
CUSTOMVERTEX* pxVert;
VECT			xPos;
VECT		xAttachPointTransformed;
int		nRenderHandle = NOTFOUND;
int		nAttachVertex;
VECT	xRot;
ENGINEMATRIX	matWorld;
ENGINEMATRIX	matRotX;
ENGINEMATRIX	matRotY;
ENGINEMATRIX	matRotZ;
int			nRet;

	if ( pxModelData->pxBaseMesh != NULL )
	{
		xRot.x = xRot.y = xRot.z = 0.0f;

		// Only horizontal turrets should be using this function now
		if ( nWhichAttachedModel != MODEL_ATTACHED_TURRET_HORIZ )
		{
			PANIC_IF(TRUE,"Please inform the programmer that he is an idiot");
			return( 0 );
		}

		nRenderHandle = pxModelData->xHorizTurretData.nModelHandle;
		nAttachVertex = pxModelData->xHorizTurretData.nAttachVertex;
		pxSubModelData = &maxModelRenderData[ nRenderHandle ];

		// Only apply extra rotation in z for horizontal turrets
		xRot.z = pxModelData->xHorizTurretData.xCurrentRotations.z;

		matWorld = matOriginalWorld;
		matWorld._41 = 0.0f;
		matWorld._42 = 0.0f;
		matWorld._43 = 0.0f;

		if ( nAttachVertex != 0 )
		{
		VECT	xOffsetTransformed;

			pxModelData->pxBaseMesh->LockVertexBuffer( kLock_ReadOnly , (byte**)( &pxVertices ) );
			pxVert = pxVertices + nAttachVertex;
			VectTransform( &xAttachPointTransformed, &pxVert->position, &matWorld );
			VectTransform( &xOffsetTransformed, &pxModelData->xHorizTurretData.xAttachOffset, &matWorld );
			VectAdd( &xAttachPointTransformed, &xAttachPointTransformed, &xOffsetTransformed );

			pxModelData->pxBaseMesh->UnlockVertexBuffer();
		}
		else
		{
			xAttachPointTransformed.x = 0.0f;
			xAttachPointTransformed.y = 0.0f;
			xAttachPointTransformed.z = 0.0f;
		}

		xPos.x = matOriginalWorld._41 + xAttachPointTransformed.x;
		xPos.y = matOriginalWorld._42 + xAttachPointTransformed.y;
		xPos.z = matOriginalWorld._43 + xAttachPointTransformed.z;

		// if no rotation passed in, matWorld will just take the value of matOriginalWorld
		if ( pxRot != NULL )
		{
			EngineMatrixIdentity( &matWorld );

			EngineMatrixRotationZ( &matRotZ, xRot.z );
			EngineMatrixMultiply( &matWorld, &matRotZ );
	
			EngineMatrixRotationX( &matRotX, pxRot->x ); 
			EngineMatrixRotationY( &matRotY, pxRot->y ); 
		    EngineMatrixRotationZ( &matRotZ, pxRot->z );

			EngineMatrixMultiply( &matWorld, &matRotX );
			EngineMatrixMultiply( &matWorld, &matRotY );
			EngineMatrixMultiply( &matWorld, &matRotZ );
		}

		matWorld._41 = xPos.x;
		matWorld._42 = xPos.y;
		matWorld._43 = xPos.z;
		EngineSetWorldMatrix( &matWorld );
	}

	nRet = ModelRender( nRenderHandle, NULL, NULL, 0 );

	if ( nWhichAttachedModel == MODEL_ATTACHED_TURRET_HORIZ )
	{
		if ( pxSubModelData )
		{
			// If the vert turret has a weapon attach vertex,
			// calculate the transformed point of the vertex and store it in the root model
			if ( pxSubModelData->xAttachData.xGenericWeaponFireAttach.nAttachVertex != 0 )
			{
			VECT	xOffsetTransformed;

				pxSubModelData->pxBaseMesh->LockVertexBuffer( kLock_ReadOnly , (byte**)( &pxVertices ) );
				pxVert = pxVertices + pxSubModelData->xAttachData.xGenericWeaponFireAttach.nAttachVertex;
				VectTransform( &xAttachPointTransformed, &pxVert->position, &matWorld );
				VectTransform( &xOffsetTransformed, &pxSubModelData->xAttachData.xGenericWeaponFireAttach.xAttachOffset, &matWorld );
				VectAdd( &xAttachPointTransformed, &xAttachPointTransformed, &xOffsetTransformed );
				pxSubModelData->pxBaseMesh->UnlockVertexBuffer();
				pxModelData->xAttachData.xGenericWeaponFireAttach.xTransformedPos.x = xAttachPointTransformed.x;
				pxModelData->xAttachData.xGenericWeaponFireAttach.xTransformedPos.y = xAttachPointTransformed.y;
				pxModelData->xAttachData.xGenericWeaponFireAttach.xTransformedPos.z = xAttachPointTransformed.z;		
				pxModelData->xAttachData.xGenericWeaponFireAttach.boTransformValid = TRUE;
			}
		}
	}
	return( nRet );
}


#define SHOW_SHADOW_TEXTURE
ulong	mulShadowDisplayTick = 0;
D3DXMATRIX matWorldStore;
D3DXMATRIX matProjStore;
D3DXMATRIX matViewStore;

void	StoreCameraMatrices(void)
{
	mpEngineDevice->GetTransform( D3DTS_PROJECTION, &matProjStore );
	mpEngineDevice->GetTransform( D3DTS_VIEW, &matViewStore );
	EngineSetWorldMatrix( &matWorldStore );
}

void	RestoreCameraMatrices(void)
{
	EngineSetProjectionMatrix( &matProjStore );
	EngineSetViewMatrix( &matViewStore );
	EngineSetWorldMatrix( &matWorldStore );
}



void		RenderingComputeBoundingBox( const CUSTOMVERTEX* pVertices, int nNumVertices, VECT* pxBoundMin, VECT* pxBoundMax )
{
	D3DXComputeBoundingBox( (LPD3DXVECTOR3)&pVertices->position, nNumVertices,sizeof(CUSTOMVERTEX), (D3DXVECTOR3*)pxBoundMin, (D3DXVECTOR3*)pxBoundMax );
} 

void		RenderingComputeBoundingSphere( const CUSTOMVERTEX* pVertices, int nNumVertices, VECT* pxSpherePos, float* pfSphereRadius )
{
	D3DXComputeBoundingSphere( (LPD3DXVECTOR3)&pVertices->position, nNumVertices,sizeof(CUSTOMVERTEX), (D3DXVECTOR3*)pxSpherePos, pfSphereRadius );
}


#ifndef TOOL	// None of this stuff is included when not in the game - the shadow casting
				// relies on knowing where the game camera is etc..

ulong		mulLastShadowCastTime = 0;
BOOL	mboDidCastShadow = FALSE;

BOOL		ModelRenderDidCastShadow( void )
{
	return( mboDidCastShadow );
}

/***************************************************************************
 * Function    : ModelRenderGeneratePreviewImage
 * Params      :
 * Description : 
 ***************************************************************************/
TEXTURE_HANDLE		ModelRenderGeneratePreviewImage( int nModelHandle, VECT* pxPos, VECT* pxRot, float fScale )
{
MODEL_RENDER_DATA* pxModelData;
IGRAPHICSSURFACE*	pNormalRenderTarget;
IGRAPHICSSURFACE*	pNormalDepthStencil;
float	fNearClipPlane = 0.001f;
float	fFarClipPlane = 50.0f;
float	fFOV = (float)(PI/4);
D3DXMATRIX matWorld;
D3DXMATRIX matProj;
D3DCOLOR	xColor;
D3DXMATRIX matView;
LPGRAPHICSTEXTURE	pRenderTargetTexture;
TEXTURE_HANDLE		hNewTexture;
IGRAPHICSSURFACE*	pNewSurface;
IGRAPHICSSURFACE*	pTempDepthSurface;
float	fDist = 1.0f;
VECT	xCamPos;
VECT	xLookAtPos;
float		fScaleMod = 0.8f * fScale;

	pxModelData = &maxModelRenderData[ nModelHandle ];

	StoreCameraMatrices();
	mpEngineDevice->GetRenderTarget( 0, &pNormalRenderTarget );
	mpEngineDevice->GetDepthStencilSurface( &pNormalDepthStencil );

	hNewTexture = EngineCreateRenderTargetTexture( 256, 256, 0 );
	if ( hNewTexture != 0 )
	{
		pRenderTargetTexture = EngineGetTextureDirectDX(hNewTexture);
		if ( pRenderTargetTexture )
		{
		VECT	xUp = { 0.0f, 0.0f, 1.0f };

			pRenderTargetTexture->GetSurfaceLevel(0,&pNewSurface);
			mpEngineDevice->CreateDepthStencilSurface( 256, 256, D3DFMT_D16, D3DMULTISAMPLE_NONE, 0, FALSE, &pTempDepthSurface, NULL );
			mpEngineDevice->SetRenderTarget( 0,pNewSurface );
			mpEngineDevice->SetDepthStencilSurface( pTempDepthSurface );

			xColor = (D3DCOLOR)( 0 );// 0xFFFFFFFF );
			mpEngineDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, xColor, 1.0f, 0 );
			// Set the projection matrix
			float	fAspect = (float)(InterfaceGetWidth()) / (float)(InterfaceGetHeight());
			D3DXMatrixPerspectiveFovLH( &matProj, fFOV, fAspect, fNearClipPlane, fFarClipPlane );
			EngineSetProjectionMatrix( &matProj );

			fDist = (pxModelData->xStats.fBoundSphereRadius*fScaleMod);
			
			xLookAtPos = pxModelData->xStats.xBoundSphereCentre;
			xCamPos = xLookAtPos;
			xCamPos.x += (fDist * 1.9f);
			xCamPos.y += (fDist * 1.2f);
			xCamPos.z += (fDist * 0.6f);
			xLookAtPos.z += (fDist * 0.025f);

			// Set view matrix at camera position looking at 0,0,0
			EngineMatrixLookAt( &matView, &xCamPos, &xLookAtPos, &xUp );
			EngineSetViewMatrix( &matView );

			EngineEnableFog( FALSE );
			EngineEnableZWrite( TRUE );
			EngineEnableZTest( TRUE );
			EngineSetWorldMatrix( NULL );

			ModelRender( nModelHandle, NULL, NULL, 0 );
			// Put the normal render target back
			mpEngineDevice->SetRenderTarget( 0, (IGRAPHICSSURFACE*)pNormalRenderTarget );
			mpEngineDevice->SetDepthStencilSurface( pNormalDepthStencil );
			pNormalDepthStencil->Release();
			pNormalRenderTarget->Release();
			pNewSurface->Release();
			pTempDepthSurface->Release();
			RestoreCameraMatrices();

			EngineEnableFog( EngineGetGameInterface()->GetFogSetting() );

		}
	}
	return( hNewTexture );
}



/***************************************************************************
 * Function    : ModelRenderCastShadow
 * Params      :
 * Description : 
 ***************************************************************************/
void		ModelRenderCastShadow( int nModelHandle, const VECT* pxPos, const VECT* pxRot )
{
}
#endif

D3DXMATRIX	mxMatIdent;

void EngineSetMatrixFromRotations( const VECT* pxRot, ENGINEMATRIX* pxMatrix )
{
ENGINEMATRIX	matWorld;
ENGINEMATRIX	matRotX;
ENGINEMATRIX	matRotY;
ENGINEMATRIX	matRotZ;

	EngineMatrixIdentity( &matWorld );
	if ( pxRot != NULL )
	{
		if ( pxRot->x != 0.0f )
		{
			if ( pxRot->y == 0.0f )
			{
				EngineMatrixRotationX( &matRotX, pxRot->x ); 
			    EngineMatrixRotationZ( &matRotZ, pxRot->z );
				matWorld = matRotX;
				EngineMatrixMultiply( &matWorld, &matRotZ );		
			}
			else
			{
				EngineMatrixRotationX( &matRotX, pxRot->x ); 
				EngineMatrixRotationY( &matRotY, pxRot->y ); 
				EngineMatrixRotationZ( &matRotZ, pxRot->z );
				matWorld = matRotX;			
				EngineMatrixMultiply( &matWorld, &matRotY );
				EngineMatrixMultiply( &matWorld, &matRotZ );
			}
		}
		else if ( pxRot->y == 0.0f )	// if x and y are 0.0
		{
		    EngineMatrixRotationZ( &matWorld, pxRot->z );
		}
		else
		{
			EngineMatrixRotationY( &matRotY, pxRot->y ); 
		    EngineMatrixRotationZ( &matRotZ, pxRot->z );
			matWorld = matRotY;	
			EngineMatrixMultiply( &matWorld, &matRotZ );		
		}
	}
	*pxMatrix = matWorld;
}


/***************************************************************************
 * Function    : RenderDXMesh
 * Params      :
 * Description : Draws the specified model
 ***************************************************************************/
int		RenderDXMesh( MODEL_RENDER_DATA* pxModelData, ulong ulRenderFlags )
{
int		ret = 0;
int		nPolyCount = 0;
ENGINEMATERIAL		xBaseMaterial; 

	EngineGetMaterial( &xBaseMaterial );

	if ( pxModelData->pxBaseMesh != NULL )
	{
	int		matloop;
	ModelMaterialData*		pMaterialData;
	int		nNumSubsetsToDraw = pxModelData->xStats.nNumMaterials;

		if ( ulRenderFlags != 0 )
		{
			if ( (ulRenderFlags & RENDER_FLAGS_MATERIAL1_ONLY) == RENDER_FLAGS_MATERIAL1_ONLY )
			{
				pxModelData->pxBaseMesh->DrawSubset( 0 );
				return( 1 );
			}
			else if ( (ulRenderFlags & RENDER_FLAGS_MATERIAL2_ONLY) == RENDER_FLAGS_MATERIAL2_ONLY )
			{
				pxModelData->pxBaseMesh->DrawSubset( 1 );
				return( 1 );
			}
			else if ( (ulRenderFlags & RENDER_FLAGS_MATERIAL3_ONLY) == RENDER_FLAGS_MATERIAL3_ONLY )
			{
				pxModelData->pxBaseMesh->DrawSubset( 2 );
				return( 1 );
			}
			else if ( (ulRenderFlags & RENDER_FLAGS_MATERIAL4_ONLY) == RENDER_FLAGS_MATERIAL4_ONLY )
			{
				pxModelData->pxBaseMesh->DrawSubset( 3 );
				return( 1 );
			}
		}

		if ( nNumSubsetsToDraw < 1 ) nNumSubsetsToDraw = 1;
		for ( matloop = 0; matloop < nNumSubsetsToDraw; matloop++ )
		{	
			// Apply material changes and set shaders if required
			if ( ( ulRenderFlags & RENDER_FLAGS_NO_STATE_CHANGE) == 0 )
			{
				// apply material texture and states if they're included in the model
				pMaterialData = FindMaterial( pxModelData, matloop );
				if ( pMaterialData )
				{
					ret = pMaterialData->Apply();
				}
			}

			if ( !mpRenderBaseTexture )
			{
				mpEngineDevice->GetTexture( 0, &mpRenderBaseTexture );
			}

			pxModelData->pxBaseMesh->DrawSubset( matloop );

			if ( ret != 0 )
			{
				// Restore standard blend modes, material etc if pMaterialData->Apply changed em..
				if ( ret & ModelMaterialData::BLEND_RENDER_STATES_CHANGED )
				{
					ModelRenderSetMaterialRenderStates( pxModelData );
				}

				if ( ret & ModelMaterialData::MATERIAL_CHANGED )
				{
					EngineSetMaterial( &xBaseMaterial );
					mpEngineDevice->SetRenderState(	D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1 );
					mpEngineDevice->SetRenderState(	D3DRS_SPECULARMATERIALSOURCE, D3DMCS_COLOR2 );
				}
				if ( ret & ModelMaterialData::SPECULAR_ACTIVATED )
				{
					EngineEnableSpecular( FALSE );
				}
				if ( ret & ModelMaterialData::NORMALMAP_ACTIVATED )
				{
					ModelMaterialsDeactivateNormalShader();
				}

			}
		} // end material for loop

		nPolyCount = (pxModelData->xStats.nNumIndices/3);
	}
	return( nPolyCount );
}



int		ModelRenderDXRenderSkinnedModel( int nModelHandle, const VECT* pxPos, const VECT* pxRot, ulong ulRenderFlags, const ENGINEQUATERNION* pxQuat )
{
D3DXMATRIX	matWorld;
MODEL_RENDER_DATA*		pxModelData = &maxModelRenderData[ nModelHandle ];

	// temp - if the pos is set here, do all the basic rendering setup too.
	// this'll move to be based on the RenderFlags..
	if ( (ulRenderFlags & RENDER_FLAGS_NO_STATE_CHANGE) == 0 )
	{
		if ( pxModelData->xGlobalProperties.bBackfaceFlag == 0 )
		{
//				mpEngineDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
		}
		mpEngineDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
		EngineEnableAlphaTest( TRUE );
		EngineEnableLighting( TRUE );
		EngineEnableZTest( TRUE );

	}
	
	if ( pxQuat )
	{
		EngineMatrixFromQuaternion( &matWorld, pxQuat );
	}
	else
	{
		EngineSetMatrixFromRotations( pxRot, &matWorld );
	}

	matWorld._41 = pxPos->x;
	matWorld._42 = pxPos->y;
	matWorld._43 = pxPos->z;
	EngineSetWorldMatrix( &matWorld );
	
	if ( ModelRenderingIsShadowPass() )
	{
		EngineShadowMapSetWorldMatrix( (ENGINEMATRIX*)&matWorld );
	}

	pxModelData->pSkinnedModel->Render();

	return( 1 );
}

/***************************************************************************
 * Function    : ModelRenderDXAddAttachedEffect
 * Params      :
 * Description : 
 ***************************************************************************/
void		ModelRenderDXAddAttachedEffect( MODEL_RENDER_DATA* pxModelData, D3DXMATRIX* pxMatOriginalWorld )
{
VECT		xTransformedPos;
CUSTOMVERTEX*	pxVertices;
CUSTOMVERTEX*	pxVert;
D3DXVECTOR3		xAttachPointTransformed;

	if ( pxModelData->xEffectAttachData.nAttachVertex != 0 )
	{
	D3DXMATRIX	matOriginalWorld2;

		matOriginalWorld2 = *pxMatOriginalWorld;
		matOriginalWorld2._41 = 0.0f;
		matOriginalWorld2._42 = 0.0f;
		matOriginalWorld2._43 = 0.0f;
		pxModelData->pxBaseMesh->LockVertexBuffer( kLock_ReadOnly , (byte**)( &pxVertices ) );
		pxVert = pxVertices + pxModelData->xEffectAttachData.nAttachVertex;
		D3DXVec3TransformCoord( &xAttachPointTransformed, (D3DXVECTOR3*)&pxVert->position, &matOriginalWorld2 );
		pxModelData->pxBaseMesh->UnlockVertexBuffer();
		xTransformedPos.x = xAttachPointTransformed.x + pxMatOriginalWorld->_41;
		xTransformedPos.y = xAttachPointTransformed.y + pxMatOriginalWorld->_42;
		xTransformedPos.z = xAttachPointTransformed.z + pxMatOriginalWorld->_43;		
	}
	else
	{
		xTransformedPos.x = pxMatOriginalWorld->_41;
		xTransformedPos.y = pxMatOriginalWorld->_42;
		xTransformedPos.z = pxMatOriginalWorld->_43;		
	}

	if ( ModelRenderingIsShadowPass() == FALSE )
	{
		ModelRenderingAddEffect( pxModelData, (VECT*)&xTransformedPos );
	}

}


void		ModelRenderDXUpdateWeaponAttachTransform( MODEL_RENDER_DATA* pxModelData, D3DXMATRIX* pxMatOriginalWorld )
{
D3DXMATRIX	matOriginalWorld2;
CUSTOMVERTEX*	pxVertices;
CUSTOMVERTEX*	pxVert;
D3DXVECTOR3		xAttachPointTransformed;

	matOriginalWorld2 = *pxMatOriginalWorld;
	matOriginalWorld2._41 = 0.0f;
	matOriginalWorld2._42 = 0.0f;
	matOriginalWorld2._43 = 0.0f;
	pxModelData->pxBaseMesh->LockVertexBuffer( kLock_ReadOnly , (byte**)( &pxVertices ) );
	pxVert = pxVertices + pxModelData->xAttachData.xGenericWeaponFireAttach.nAttachVertex;
	D3DXVec3TransformCoord( &xAttachPointTransformed, (D3DXVECTOR3*)&pxVert->position, &matOriginalWorld2 );
	pxModelData->pxBaseMesh->UnlockVertexBuffer();

	pxModelData->xAttachData.xGenericWeaponFireAttach.xTransformedPos.x = xAttachPointTransformed.x + pxMatOriginalWorld->_41;
	pxModelData->xAttachData.xGenericWeaponFireAttach.xTransformedPos.y = xAttachPointTransformed.y + pxMatOriginalWorld->_42;
	pxModelData->xAttachData.xGenericWeaponFireAttach.xTransformedPos.z = xAttachPointTransformed.z + pxMatOriginalWorld->_43;		
	pxModelData->xAttachData.xGenericWeaponFireAttach.boTransformValid = TRUE;
}


/***************************************************************************
 * Function    : ModelRenderImplDX
 * Params      :
 * Description : Draws the specified model
 ***************************************************************************/
int		ModelRenderImplDX( int nModelHandle, const VECT* pxPos, const VECT* pxRot, ulong ulRenderFlags, const ENGINEQUATERNION* pxQuat )
{
MODEL_RENDER_DATA*		pxModelData = &maxModelRenderData[ nModelHandle ];
ulong		ulTick = mulLastRenderingTick;
D3DXMATRIX	matOriginalWorld;
int		nPolyCount = 0;
ENGINEMATRIX	matRestore;

	mpEngineDevice->GetTexture( 0, &mpRenderBaseTexture );

	if ( pxModelData->pSkinnedModel )
	{
		return( ModelRenderDXRenderSkinnedModel( nModelHandle, pxPos, pxRot, ulRenderFlags, pxQuat ) );
	}
	else if ( pxModelData->pxBaseMesh == NULL )
	{
		return( 0 );
	}

	if ( (ulRenderFlags & RENDER_FLAGS_NO_STATE_CHANGE) == 0 )
	{
		if ( pxModelData->xGlobalProperties.bBackfaceFlag == 0 ) 
		{
			EngineEnableCulling( 1 );
		}
		EngineEnableAlphaTest( TRUE );
//		EngineEnableLighting( TRUE );
		EngineEnableZTest( TRUE );
	}

	if ( pxPos != NULL )
	{
	D3DXMATRIX	matWorld;

		EngineGetWorldMatrix( &matRestore );
		if ( pxQuat )
		{
			EngineMatrixFromQuaternion( &matWorld, pxQuat );
		}
		else
		{
			EngineSetMatrixFromRotations( pxRot, &matWorld );
		}
		matWorld._41 = pxPos->x;
		matWorld._42 = pxPos->y;
		matWorld._43 = pxPos->z;

		EngineSetWorldMatrix( &matWorld );
		matOriginalWorld = matWorld;
	}
	else
	{
		mpEngineDevice->GetTransform( D3DTS_WORLD, &matOriginalWorld );
	
		if ( ModelRenderingIsShadowPass() )
		{
			EngineShadowMapSetWorldMatrix( (ENGINEMATRIX*)&matOriginalWorld );
		}
		else 
		{
			ModelMaterialsNormalShaderUpdateWorldTransform( (ENGINEMATRIX*)&matOriginalWorld );
		}
	}

	
	// If theres an effect attached - render it
	if ( pxModelData->bHasEffect != 0 )
	{
		if ( (ulRenderFlags & RENDER_FLAGS_NO_EFFECTS) == 0 )
		{
			ModelRenderDXAddAttachedEffect( pxModelData, &matOriginalWorld );
		}
	}

	// If theres a weapon attach vertex, store its transformed position
	if ( pxModelData->xAttachData.xGenericWeaponFireAttach.nAttachVertex != 0 )
	{
		ModelRenderDXUpdateWeaponAttachTransform( pxModelData, &matOriginalWorld );
	}

	if ( ( pxModelData->xGlobalProperties.bNoLighting == 1 ) &&
		 ( (ulRenderFlags & RENDER_FLAGS_LIGHTING_OVERRIDE) == 0 ) )
	{
		EngineEnableLighting( FALSE );
	}

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
		nPolyCount += RenderDXMesh( pxModelData, ulRenderFlags );
		break;
	default:
		// If its a static mesh
		nPolyCount += RenderDXMesh( pxModelData, ulRenderFlags );
		break;
	}

	ModelRenderMaterialRestoreStatesPostRender( pxModelData );

	// --- If this is the base model, continue on to render the various submodels (wheels, turrets etc)
	if ( (ulRenderFlags & RENDER_FLAGS_NO_SUBMODELS) == 0 )
	{
		// Once we've rendered the base model, reset the texture back to the game-selected one
		// (As materials in the base model may have changed it, and subsequent models may well expect it)
		mpEngineDevice->SetTexture( 0, 	mpRenderBaseTexture );
		
		if ( ( pxModelData->xWheel1AttachData.nModelHandle > 0 ) &&
			 ( (ulRenderFlags & RENDER_FLAGS_NO_FRONT_WHEELS) == 0 ) )		
		{
			nPolyCount += ModelRenderAttachedModel( pxModelData, pxPos, pxRot, ulRenderFlags, matOriginalWorld, MODEL_ATTACHED_WHEEL_1, pxQuat );
		}
		if ( ( pxModelData->xWheel2AttachData.nModelHandle > 0 ) &&
			 ( (ulRenderFlags & RENDER_FLAGS_NO_FRONT_WHEELS) == 0 ) )
		{
			nPolyCount += ModelRenderAttachedModel( pxModelData, pxPos, pxRot, ulRenderFlags, matOriginalWorld, MODEL_ATTACHED_WHEEL_2, pxQuat );
		}
		if ( ( pxModelData->xWheel3AttachData.nModelHandle > 0 ) &&
			 ( (ulRenderFlags & RENDER_FLAGS_NO_REAR_WHEELS) == 0 ) )
		{
			nPolyCount += ModelRenderAttachedModel( pxModelData, pxPos, pxRot, ulRenderFlags, matOriginalWorld, MODEL_ATTACHED_WHEEL_3, pxQuat );
		}
		if ( ( pxModelData->xWheel4AttachData.nModelHandle > 0 ) &&
			 ( (ulRenderFlags & RENDER_FLAGS_NO_REAR_WHEELS) == 0 ) )
		{
			nPolyCount += ModelRenderAttachedModel( pxModelData, pxPos, pxRot, ulRenderFlags, matOriginalWorld, MODEL_ATTACHED_WHEEL_4, pxQuat );
		}
		if ( pxModelData->xHorizTurretData.nModelHandle > 0 )
		{
			nPolyCount += ModelRenderAttachedModelBasic( pxModelData, pxPos, pxRot, ulRenderFlags, matOriginalWorld, MODEL_ATTACHED_TURRET_HORIZ );
		}
		if ( pxModelData->xVertTurretData.nModelHandle > 0 )
		{
			nPolyCount += ModelRenderAttachedModel( pxModelData, pxPos, pxRot, ulRenderFlags, matOriginalWorld, MODEL_ATTACHED_TURRET_VERT, pxQuat );
		}
	}

	if ( pxPos != NULL )
	{
		EngineSetWorldMatrix( &matRestore );
	}

	if ( ( pxModelData->xGlobalProperties.bNoLighting == 1 ) &&
		 ( (ulRenderFlags & RENDER_FLAGS_LIGHTING_OVERRIDE) == 0 ) )
	{
		EngineEnableLighting( TRUE );
	}
	return( nPolyCount );
}
 

void ModelMoveVerts( int nModelHandle, float fX, float fY, float fZ )
{
MODEL_RENDER_DATA*	pxModelData;
CUSTOMVERTEX*	pxVertices;
int		nVertLoop;
int		nVertsInMesh;
VECT*	pxVerts;
int		nVertsTotal;

	if ( nModelHandle == NOTFOUND ) return;

	pxModelData = &maxModelRenderData[ nModelHandle ];

	nVertsInMesh = pxModelData->xStats.nNumVertices;
		
	if ( pxModelData->pxBaseMesh != NULL )
	{
		pxModelData->pxBaseMesh->LockVertexBuffer( NULL, (byte**)( &pxVertices ) );
		for ( nVertLoop = 0; nVertLoop < nVertsInMesh; nVertLoop++ )
		{
			pxVertices->position.x += fX;
			pxVertices->position.y += fY;
			pxVertices->position.z += fZ;
			pxVertices++;
		}
		pxModelData->pxBaseMesh->UnlockVertexBuffer();
	}

	if ( pxModelData->pxVertexKeyframes != NULL )
	{
		pxVerts = pxModelData->pxVertexKeyframes;
		nVertsTotal = pxModelData->xStats.nNumVertKeyframes * nVertsInMesh;
		for ( nVertLoop = 0; nVertLoop < nVertsTotal; nVertLoop++ )
		{
			pxVerts->x += fX;
			pxVerts->y += fY;
			pxVerts->z += fZ;
			pxVerts++;
		}
	}
}

int		ModelRenderScaled( int nModelHandle, VECT* pxPos, VECT* pxRot, ulong ulRenderFlags, VECT* pxScale )
{
ENGINEMATRIX	matWorld;
ENGINEMATRIX	matRotX;
ENGINEMATRIX	matRotY;
ENGINEMATRIX	matRotZ;
MODEL_RENDER_DATA*		pxModelData;

	if ( nModelHandle == NOTFOUND ) return( 0 );

	pxModelData = &maxModelRenderData[ nModelHandle ];

	if ( pxModelData->pxBaseMesh != NULL )
	{

		EngineMatrixScaling( &matWorld, pxScale->x, pxScale->y, pxScale->z );
		// if no rotation passed in, matWorld will just take the value of matOriginalWorld
		if ( pxRot != NULL )
		{
			EngineMatrixRotationX( &matRotX, pxRot->x ); 
			EngineMatrixRotationY( &matRotY, pxRot->y ); 
		    EngineMatrixRotationZ( &matRotZ, pxRot->z );

			EngineMatrixMultiply( &matWorld, &matRotX );
			EngineMatrixMultiply( &matWorld, &matRotY );
			EngineMatrixMultiply( &matWorld, &matRotZ );
		}
		matWorld._41 = pxPos->x;
		matWorld._42 = pxPos->y;
		matWorld._43 = pxPos->z;
		EngineSetWorldMatrix( &matWorld );
	}

	return( ModelRender( nModelHandle, NULL, NULL, ulRenderFlags ) );
}




//--------------------------------------------------------------------------------------------
//  .X file loader

void	ModelXFileLoaderProcessLoadedMaterials( MODEL_RENDER_DATA* pxModelData )
{
int		loop;
const char*		pcFilename;
ModelMaterialData*	pMaterialData;
int		loop2;
int*	pnMaterialRemap;
int		nNumUniqueMaterials = 0;
DWORD*	pxMeshAttributes = NULL;

	pnMaterialRemap = (int*)( malloc( pxModelData->xStats.nNumMaterials * sizeof( int ) ) );

	// First lets go through the materials and see which ones are duplicates
	for ( loop = 0; loop < pxModelData->xStats.nNumMaterials; loop++ )
	{
		for ( loop2 = 0; loop2 < loop; loop2++ )
		{
			// NOTE - If we actually parse the material properties (specular, diffuse, etc) from the X
			// then this process will need to check those fields match too before considering it a duplicate material
			// At the mo, this just assumes that if 2 materials have the same texture filename then they're the same material
			if ( stricmp( mpLastLoadMaterialBuffer[loop2].pTextureFilename, mpLastLoadMaterialBuffer[loop].pTextureFilename ) == 0 )
			{
				pnMaterialRemap[loop] = pnMaterialRemap[loop2];
				break;
			}
		}
		// Didn't find a duplicate material earlier in the list
		if ( loop2 == loop )
		{
			pnMaterialRemap[loop] = nNumUniqueMaterials;
			nNumUniqueMaterials++;
		}
	}

	SysDebugPrint( "Processed duplicate materials - %d in .X file reduced to %d", pxModelData->xStats.nNumMaterials, nNumUniqueMaterials );

	// Now we need to rebuild the attribute buffer
	pxModelData->pxBaseMesh->LockAttributeBuffer( kLock_Normal, (BYTE**)&pxMeshAttributes );
	if ( pxMeshAttributes )
	{
	int		nLoop;
	int		nNumFaces = pxModelData->xStats.nNumIndices / 3;
	int		nOriginalAttrib;
	
		for ( nLoop = 0; nLoop < nNumFaces; nLoop++ )
		{ 
			nOriginalAttrib = (int)*pxMeshAttributes;
			if ( ( nOriginalAttrib >= 0 ) &&
				 ( nOriginalAttrib < pxModelData->xStats.nNumMaterials ) )
			{
				*pxMeshAttributes = (DWORD)( pnMaterialRemap[ nOriginalAttrib ] );		
			}
			else
			{
				*pxMeshAttributes = 0;
			}
			pxMeshAttributes++;
		}
		pxModelData->pxBaseMesh->UnlockAttributeBuffer();
	}

	loop2 = 0;
	for ( loop = 0; loop < pxModelData->xStats.nNumMaterials; loop++ )
	{
		// If its a unique material
		if ( pnMaterialRemap[loop] >= loop2 )
		{
			pcFilename = mpLastLoadMaterialBuffer[loop].pTextureFilename;

			pMaterialData = new ModelMaterialData;

			pMaterialData->SetAttrib( loop2 );
			pMaterialData->LoadTextureFilename( 0, pcFilename );

			// Add to the modeldata linked list of materials
			pMaterialData->SetNext(pxModelData->pMaterialData);
			pxModelData->pMaterialData = pMaterialData;
			loop2++;
		}
		else
		{
			// Ignore it..
		}
	}

	pxModelData->xStats.nNumMaterials = nNumUniqueMaterials;

}


/***************************************************************************
 * Function    : ModelLoadXFile
 * Params      : Returns the modelHandle of the model loaded, or NOTFOUND if unsuccessful
 * Description : Attempts to load the specified model
 ***************************************************************************/
LPD3DXMESH		ModelLoadXFile( MODEL_RENDER_DATA* pxModelData, const char* szFilename, float fScale, VECT* pxBoundMin, VECT* pxBoundMax )
{
CUSTOMVERTEX*	pxVertices;
CUSTOMVERTEX*	pxVertBase;
LPD3DXMESH		pxMesh = NULL;
LPD3DXMESH		pxTempMesh;
DWORD			dwNumMaterials;
char			acString[2048];
int				nVertsInMesh;
int				nVertLoop;
float	fLowestZ;
BOOL	boFlipUVs = FALSE;
BOOL	boNoColours = TRUE;
HRESULT ret;
int		nCloneFlags;

	if ( mpD3DXMtrlLoadBuffer )
	{
		mpD3DXMtrlLoadBuffer->Release();
		mpD3DXMtrlLoadBuffer = NULL;
		mpLastLoadMaterialBuffer = NULL;
	}

	if ( mpD3DXAdjacencyLoadBuffer )
	{
		mpD3DXAdjacencyLoadBuffer->Release();
		mpD3DXAdjacencyLoadBuffer = NULL;
	}

	// TODO - as the loadmeshfromx can take a long time for big .x files..
	// create a dialog here to show the current progress..
	// that is.. if we wanted to be really proper and neat and everything

	strcpy( acString, szFilename );

/*
	// Try to load it as a skinned mesh
	SkinnedModel*	pSkinnedModel = new SkinnedModel;

	if ( pSkinnedModel->Load( acString ) == true )
	{
		// We have a skinned model
		pxModelData->pSkinnedModel = pSkinnedModel;
//		pxMesh = pSkinnedModel->GetBaseMesh();
		// ...
	}
	else
*/
	{
//		delete pSkinnedModel;

		// Load the mesh from the specified file
		ret = D3DXLoadMeshFromX( acString, D3DXMESH_SYSTEMMEM, 
									   mpEngineDevice, &mpD3DXAdjacencyLoadBuffer, 
										   &mpD3DXMtrlLoadBuffer, NULL, &dwNumMaterials, 
										   &pxTempMesh );
		if( ret != D3D_OK )
		{
#ifdef DXERR_INCLUDED			
			// removed for removal of dxerr9.lib - july 2017 - in transition to vs 2017
			sprintf( acString, "Error reading .x model file :\n%s\n\nDirectX Error code (%08x) :\n\n%s\n%s\n", szFilename, ret, DXGetErrorString9(ret), DXGetErrorDescription9(ret) );	
#else
			sprintf(acString, "Error reading .x model file :\n%s\n\nDirectX Error code (%08x) :\n\n", szFilename, ret);
#endif
			PANIC_IF(TRUE, acString );
			return( NULL );
		}

		nCloneFlags = D3DXMESH_SYSTEMMEM;
		if ( pxTempMesh->GetOptions() & D3DXMESH_32BIT )
		{
			nCloneFlags |= D3DXMESH_32BIT;
		}
		mnLastLoadNumMaterials = dwNumMaterials;
		// TODOO - MAKE SURE THIS IS RELEASED *PROPERLY* OR WE'LL LIKELY HAVE RESIZE-WINDOW PROBLEMS
		mpLastLoadMaterialBuffer = (D3DXMATERIAL*)mpD3DXMtrlLoadBuffer->GetBufferPointer();

		ret = pxTempMesh->CloneMeshFVF( nCloneFlags, D3DFVF_CUSTOMVERTEX,mpEngineDevice,&pxMesh );
		SAFE_RELEASE( pxTempMesh );
			
		if ( pxMesh )
		{
			nVertsInMesh = pxMesh->GetNumVertices();
			pxMesh->LockVertexBuffer( NULL, (VERTEX_LOCKTYPE)( &pxVertices ) );
			pxVertBase = pxVertices;

			for ( nVertLoop = 0; nVertLoop < nVertsInMesh; nVertLoop++ )
			{
				if ( pxVertices->color != 0 )
				{
					boNoColours = FALSE;
					break;
				}
				pxVertices++;
			}
			// If all colours in the x file were 0 (i.e. no vertexcolours present), then reset em all to FFFFFFFF
			if ( boNoColours )
			{
				pxVertices = pxVertBase;
				for ( nVertLoop = 0; nVertLoop < nVertsInMesh; nVertLoop++ )
				{
					pxVertices->color = 0xFFFFFFFF;
					pxVertices++;
				}
			}

			pxVertices = pxVertBase;
			if ( fScale != 1.0f )
			{
				fLowestZ = (pxVertices->position.z * fScale);
				for ( nVertLoop = 0; nVertLoop < nVertsInMesh; nVertLoop++ )
				{
					pxVertices->position.x *= fScale;
					pxVertices->position.y *= fScale;
					pxVertices->position.z *= fScale;

					if ( pxVertices->position.z < fLowestZ )
					{
						fLowestZ = pxVertices->position.z;
					}
					pxVertices++;
				}

				if ( strstr( szFilename, "Vhcls" ) != NULL )
				{
					pxVertices = pxVertBase;
					for ( nVertLoop = 0; nVertLoop < nVertsInMesh; nVertLoop++ )
					{	
						pxVertices->position.z -= fLowestZ;
						pxVertices++;
					}
				}
			}

			D3DXComputeBoundingBox( (LPD3DXVECTOR3)pxVertBase, nVertsInMesh,sizeof(CUSTOMVERTEX), (D3DXVECTOR3*)pxBoundMin, (D3DXVECTOR3*)pxBoundMax );
		//	D3DXComputerBoundingSphere( pxVertBase, nVertsInMesh,D3DFVF_CUSTOMVERTEX, (D3DXVECTOR3*)&pxModelData->xStats.xBoundSphereCentre, &pxModelData->xStats.fBoundSphereRadius );
			pxMesh->UnlockVertexBuffer();
		}
		else
		{
			if ( ret == E_OUTOFMEMORY )
			{
				sprintf( acString, "OutOfMemory error cloning mesh during .x load process\n(Filename: %s)\n", szFilename );
			}
			else if ( ret == D3DERR_INVALIDCALL  )
			{
				sprintf( acString, "InvalidCall error cloning mesh during .x load process\n(Filename: %s)\n", szFilename );
			}
			else
			{
				sprintf( acString, "Error cloning mesh during .x load process\n(Filename: %s)\n", szFilename );
			}
			PANIC_IF(TRUE, acString );
		}
	}
	return( pxMesh );	
}


BOOL		ModelCreateFromXFile( MODEL_RENDER_DATA* pxModelData, const char* szFilename, float fScale )
{
LPD3DXMESH		pDXMesh = NULL;
BOOL	bLoadSuccess = FALSE;

	mnLastLoadNumMaterials = 1;

	pDXMesh = ModelLoadXFile( pxModelData, szFilename, fScale, &pxModelData->xStats.xBoundMin, &pxModelData->xStats.xBoundMax );

	if ( pDXMesh )
	{
		pxModelData->pxBaseMesh = new EngineMesh;
		pxModelData->pxBaseMesh->CreateFromPlatformMeshImpl( pDXMesh );

		if ( pxModelData->pSkinnedModel )
		{
			bLoadSuccess = TRUE;
		}
		else if ( pxModelData->pxBaseMesh )
		{
			pxModelData->xStats.nNumMaterials = mnLastLoadNumMaterials;
			pxModelData->xStats.nNumVertices = pxModelData->pxBaseMesh->GetNumVertices();
			pxModelData->xStats.nNumNormals = pxModelData->xStats.nNumVertices;
			pxModelData->xStats.nNumIndices = pxModelData->pxBaseMesh->GetNumFaces() * 3;

			// Only create MaterialData if the model has more than one material..
			//  (TODO? probably want to make this optional)
			if ( pxModelData->xStats.nNumMaterials > 1 )
			{
				ModelXFileLoaderProcessLoadedMaterials( pxModelData );
			}	
			bLoadSuccess = TRUE;
		}
		else
		{
			bLoadSuccess = FALSE;
		}
	}
	else
	{
		bLoadSuccess = FALSE;
	}

	return( bLoadSuccess );
}
