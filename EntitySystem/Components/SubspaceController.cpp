#include <math.h>
#include <stdio.h>
#include "StandardDef.h"
#include "Engine.h"
#include "Rendering.h"
#include "Interface.h"

#include "../../RenderUtil/Lines3D.h"

#include "../Entity.h"
#include "../EntityGraphics.h"

#include "SubspaceController.h"


REGISTER_COMPONENT(SubspaceControllerComponent, "SubspaceController" );

SubspaceControllerComponent::SubspaceControllerComponent()
{
	mfAccThisFrame = 0.0f;
	mfRotationSpeed = 0.0f;
	mfRotation = 0.0f;
}

void	SubspaceControllerComponent::OnEntityInitialised( void )
{

}

void	SubspaceControllerComponent::ApplySteering( float fDelta )
{
float	fBankSpeed = 0.1f;
float	fAcc = 15.0f;
float	fReturnToHorizontalSpeed = 0.15f;
float	fMaxRotation = 0.05f;

	if ( GetControlState( CONTROL_STEER_LEFT ) > 0.0f )
	{
		mfRotationSpeed -= fDelta * fBankSpeed;
		if ( mfRotationSpeed < -fMaxRotation )
		{
			mfRotationSpeed = -fMaxRotation;
		}
	}
	else if ( GetControlState( CONTROL_STEER_RIGHT ) > 0.0f )
	{
		mfRotationSpeed += fDelta * fBankSpeed;
		if ( mfRotationSpeed > fMaxRotation )
		{
			mfRotationSpeed = fMaxRotation;
		}
	}
	else if ( mfRotationSpeed != 0.0f )
	{
		if ( mfRotationSpeed < 0.0f )
		{
			mfRotationSpeed += fDelta * fReturnToHorizontalSpeed;
			if ( mfRotationSpeed > 0.0f )
			{
				mfRotationSpeed = 0.0f;
			}

		}
		else
		{
			mfRotationSpeed -= fDelta * fReturnToHorizontalSpeed;
			if ( mfRotationSpeed < 0.0f )
			{
				mfRotationSpeed = 0.0f;
			}
		}
	}

	if ( GetControlState( CONTROL_ACCELERATE ) > 0.0f )
	{
		// TODO - Ramp up
		mfAccThisFrame = fDelta * fAcc;
	}
	else if ( GetControlState( CONTROL_BRAKE ) > 0.0f )
	{
		// TODO - Ramp up
		mfAccThisFrame = fDelta * -fAcc;
	}
	else
	{
		// TODO - Ramp down
		mfAccThisFrame = 0.0f;
	}
}



void	SubspaceControllerComponent::OnRenderComponent( void )
{
char		acString[256];
VECT		xVel;
float		fSpeed;

	xVel = *GetEntity()->GetVel();
	fSpeed = VectGetLength( &xVel );

	sprintf( acString, "Rot spd: %f", mfRotationSpeed );
	InterfaceText( 1, 5, 5, acString, 0xd0d0d0d0, 3 );
	sprintf( acString, "Spd: %f", fSpeed );
	InterfaceText( 1, 5, 20, acString, 0xd0d0d0d0, 3 );
}


void	SubspaceControllerComponent::OnUpdateComponent( float fDelta )
{
VECT		xAccThisFrame;
VECT		xVelThisFrame;
VECT		xVel;
VECT		xPos = *GetEntity()->GetPos();
ENGINEQUATERNION	xQuat;
float		fRollSpeedMod = 0.1f;
float		fPitchSpeedMod = 0.1f;
VECT		xForward = { 0.0f, 1.0f, 0.0f };
VECT		xUp = { 0.0f, 0.0f, 1.0f };
VECT		xRight;
float		fBankModifier = -8.0f;
float		fSpeedMax = 30.0f;
float		fSpeed;

	ApplySteering( fDelta );

	mfRotation += mfRotationSpeed;
	if ( mfRotation < 0.0f ) 
	{
		mfRotation += A360;
	}
	else
	{
		mfRotation -= A360;
	}

	VectRotateAboutAxis( &xUp, &xUp, &xForward, mfRotationSpeed * fBankModifier );
	VectRotateAboutZ( &xForward, mfRotation );
	VectRotateAboutZ( &xUp, mfRotation );
	
	VectScale( &xAccThisFrame, &xForward, mfAccThisFrame );

	xVel = *GetEntity()->GetVel();
	VectAdd( &xVel, &xVel, &xAccThisFrame );
	
	fSpeed = VectGetLength( &xVel );	
	// Cap at max speed
	if ( fSpeed > fSpeedMax )
	{
		VectNormalize( &xVel );
		VectScale( &xVel, &xVel, fSpeedMax );
	}
	GetEntity()->SetVel( &xVel );
	
	VectScale( &xVelThisFrame, &xVel, fDelta );

	VectAdd( &xPos, &xPos, &xVelThisFrame );
	GetEntity()->SetPos( &xPos );

#ifdef USE_QUATERNIONS_FOR_ENTITY_ORIENTATION
	VectCross( &xRight, &xForward, &xUp );

	EngineQuaternionFromAxes( &xQuat, &xForward, &xRight, &xUp );
	GetEntity()->SetOrientation( &xQuat );
#else
	VECT	xRot = { 0.0f, 0.0f, 0.0f };

	xRot.z = mfRotation;
	GetEntity()->SetRot( &xRot );
#endif
}
