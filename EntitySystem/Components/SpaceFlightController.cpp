#include <math.h>
#include <stdio.h>
#include "StandardDef.h"
#include "Engine.h"
#include "Rendering.h"
#include "Interface.h"

#include "../../RenderUtil/Lines3D.h"

#include "../Entity.h"
#include "../EntityGraphics.h"

#include "SpaceFlightController.h"


REGISTER_COMPONENT(SpaceFlightControllerComponent, "SpaceFlightController" );

SpaceFlightControllerComponent::SpaceFlightControllerComponent()
{
	mfRollSpeed = 0.0f;
	mfPitchSpeed = 0.0f;
}

void	SpaceFlightControllerComponent::OnEntityInitialised( void )
{
	mxForward.x = 0.0f;
	mxForward.y = 1.0f;
	mxForward.z = 0.0f;

	mxUp.x = 0.0f;
	mxUp.y = 0.0f;
	mxUp.z = 1.0f;

	VectCross( &mxRight, &mxForward, &mxUp );

	mfSpeed = 1.0f;
}

void	SpaceFlightControllerComponent::ApplySteering( float fDelta )
{
float	fBankSpeed = 0.2f;
float	fPitchSpeed = 0.2f;
float	fReturnToHorizontalSpeed = 0.25f;

	if ( GetControlState( CONTROL_STEER_LEFT ) > 0.0f )
	{
		mfRollSpeed += fDelta * fBankSpeed;
	}
	else if ( GetControlState( CONTROL_STEER_RIGHT ) > 0.0f )
	{
		mfRollSpeed -= fDelta * fBankSpeed;
	}
	else if ( mfRollSpeed != 0.0f )
	{
		if ( mfRollSpeed < 0.0f )
		{
			mfRollSpeed += fDelta * fReturnToHorizontalSpeed;
			if ( mfRollSpeed > 0.0f )
			{
				mfRollSpeed = 0.0f;
			}

		}
		else
		{
			mfRollSpeed -= fDelta * fReturnToHorizontalSpeed;
			if ( mfRollSpeed < 0.0f )
			{
				mfRollSpeed = 0.0f;
			}
		}
	}

	if ( GetControlState( CONTROL_PITCH_FORWARD ) > 0.0f )
	{
		mfPitchSpeed -= fDelta * fPitchSpeed;
	}
	else if ( GetControlState( CONTROL_PITCH_BACK ) > 0.0f )
	{
		mfPitchSpeed += fDelta * fPitchSpeed;
	}
	else if ( mfPitchSpeed != 0.0f )
	{
		if ( mfPitchSpeed < 0.0f )
		{
			mfPitchSpeed += fDelta * fReturnToHorizontalSpeed;
			if ( mfPitchSpeed > 0.0f )
			{
				mfPitchSpeed = 0.0f;
			}

		}
		else
		{
			mfPitchSpeed -= fDelta * fReturnToHorizontalSpeed;
			if ( mfPitchSpeed < 0.0f )
			{
				mfPitchSpeed = 0.0f;
			}
		}
	}
}


void	SetQuaternion( ENGINEQUATERNION* pxQuatOut, const VECT* pxForward, const VECT* pxRight, const VECT* pxUp )
{
ENGINEMATRIX		xMat;
ENGINEMATRIX*		pMatrix = &xMat;

	pMatrix->_11 = pxRight->x;
	pMatrix->_12 = pxRight->y;
	pMatrix->_13 = pxRight->z;
	pMatrix->_14 = 0.0f;
	pMatrix->_21 = pxForward->x;
	pMatrix->_22 = pxForward->y;
	pMatrix->_23 = pxForward->z;
	pMatrix->_24 = 0.0f;
	pMatrix->_31 = pxUp->x;
	pMatrix->_32 = pxUp->y;
	pMatrix->_33 = pxUp->z;
	pMatrix->_34 = 0.0f;
	pMatrix->_41 = 0.0f;
	pMatrix->_42 = 0.0f;
	pMatrix->_43 = 0.0f;
	pMatrix->_44 = 1.0f;

	EngineQuaternionFromMatrix( pxQuatOut, &xMat );
	EngineQuaternionNormalize( pxQuatOut );
}

void	SpaceFlightControllerComponent::OnRenderComponent( void )
{
VECT	xDebugPos1;
VECT	xDebugPos2;
VECT	xOffset;

	xDebugPos1 = *GetEntity()->GetPos();

	VectScale( &xOffset, &mxForward, 13.0f );
	VectAdd( &xDebugPos2, &xDebugPos1, &xOffset );

	Lines3DAddLine( &xDebugPos1, &xDebugPos2, 0xf0f01010, 0xf0f01010 );

	VectScale( &xOffset, &mxUp, 13.0f );
	VectAdd( &xDebugPos2, &xDebugPos1, &xOffset );

	Lines3DAddLine( &xDebugPos1, &xDebugPos2, 0xf010f010, 0xf010f010 );

	VectScale( &xOffset, &mxRight, 13.0f );
	VectAdd( &xDebugPos2, &xDebugPos1, &xOffset );

	Lines3DAddLine( &xDebugPos1, &xDebugPos2, 0xf01010f0, 0xf01010f0 );

}


void	SpaceFlightControllerComponent::OnUpdateComponent( float fDelta )
{
VECT		xVelThisFrame = mxForward;
VECT		xPos = *GetEntity()->GetPos();
ENGINEQUATERNION	xQuat;
float		fRollSpeedMod = 0.1f;
float		fPitchSpeedMod = 0.1f;

	ApplySteering( fDelta );

	mfSpeed = 100.0f;

	VectScale( &xVelThisFrame, &xVelThisFrame, fDelta * mfSpeed );
	VectAdd( &xPos, &xPos, &xVelThisFrame );
	GetEntity()->SetPos( &xPos );

	VectRotateAboutAxis( &mxUp, &mxUp, &mxForward, mfRollSpeed * fRollSpeedMod );
	VectRotateAboutAxis( &mxRight, &mxRight, &mxForward, mfRollSpeed * fRollSpeedMod );
	VectNormalize( &mxUp );
	VectNormalize( &mxRight );

	VectRotateAboutAxis( &mxUp, &mxUp, &mxRight, mfPitchSpeed * fPitchSpeedMod );
	VectRotateAboutAxis( &mxForward, &mxForward, &mxRight, mfPitchSpeed * fPitchSpeedMod );
	VectNormalize( &mxUp );
	VectNormalize( &mxForward );

	SetQuaternion( &xQuat, &mxForward, &mxRight, &mxUp );
	GetEntity()->SetOrientation( &xQuat );


/*

	EngineMatrixFromQuaternion( &xMat, &xQuat );
	VectTransform( &xUp, &xUp, &xMat );
	VectScale( &xUp, &xUp, -0.25f );
	VectAdd( &xPos, &xPos, &xUp );
*/
}
