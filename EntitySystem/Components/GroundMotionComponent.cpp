
#include <stdio.h>
#include "StandardDef.h"
#include "Engine.h"
#include "Rendering.h"
#include "Interface.h"

#include "../../Util/MathsUtil.h"
#include "../../Landscape/LandscapeHeightmap.h"

#include "../Entity.h"
#include "GroundMotionComponent.h"

REGISTER_COMPONENT(GroundMotionComponent, "GroundMotion" );

GRAPH_STRUCT	maStepVelModifier[] = 
{
	{ 0.0f,	 0.8f },
	{ 0.0625f, 0.86f },
	{ 0.125f, 0.92f },
	{ 0.25f, 0.95f },
	{ 0.375f, 0.975f },
	{ 0.5f,	 1.0f },
	{ 0.625f, 0.975f },
	{ 0.75f, 0.95f },
	{ 0.875f, 0.92f },
	{ 0.0625f, 0.86f },
	{ 1.0f,	 0.8f },
};

float	GetStepVelModifier( float fStepVal )
{
	return( GraphStructGetValue( maStepVelModifier, fStepVal ) );
}

//-----------------------------------------------------------------------------
GroundMotionSettings::GroundMotionSettings()
{
	mfTurnMaxSpeed = 1.2f;
	mfWalkSpeed = 4.0f;
	mfAccelerationToTurnSpeed = 0.8f;
	mfAccelerationToWalkSpeed = 1.4f;
	mfTurnRotationSpeed = 0.015f;
	mfDecelerationSpeed = 4.0f;
	mfStrideSpeed = 0.5f;
	mfDecelerationToStop = 3.0f;
}



GroundMotionComponent::GroundMotionComponent()
{
	mMotionState = kStanding;
	mfStepValue = 0.0f;
	memset( &mxVelocity, 0, sizeof( mxVelocity ) );
	mszDebugStateString[0] = 0;
	mszDebugStateString2[0] = 0;
	mbHasImmediateDestination = FALSE;
}


void	GroundMotionComponent::OnUpdateComponent( float fDelta )
{
	if ( mbHasImmediateDestination )
	{
		MoveToDestination( fDelta, &mxImmediateDestination );
	}

	if ( mMotionState == kStanding )
	{
	float	fCurrentSpeed = VectGetLength( &mxVelocity );

		if ( fCurrentSpeed > 0.0f )
		{
			fCurrentSpeed -= mMotionSettings.mfDecelerationToStop * fDelta;
			if ( fCurrentSpeed > 0.0f )
			{
				UpdateVelocityAndPosition( fDelta, fCurrentSpeed );
			}
			else
			{
				mxVelocity.x = 0.0f;
				mxVelocity.y = 0.0f;
				mxVelocity.z = 0.0f;
			}
		}
	}
	else
	{
		mfStepValue += mMotionSettings.mfStrideSpeed * fDelta;
		if ( mfStepValue > 1.0f )
		{
			mfStepValue -= 1.0f;
			if ( mfStepValue > 1.0f )
			{
				mfStepValue = 0.0f;
			}
		}
	}

}

void	GroundMotionComponent::UpdateVelocityAndPosition( float fDelta, float fNewSpeed )
{
VECT	xRot = *GetEntity()->GetRot();
VECT	xPos = *GetEntity()->GetPos();
VECT	xNewVel = { 1.0f, 0.0f, 0.0f };

	VectRotateAboutZ( &xNewVel, xRot.z );
	VectScale( &xNewVel, &xNewVel, fNewSpeed );
	mxVelocity = xNewVel;

	VectScale( &xNewVel, &xNewVel, fDelta * GetStepVelModifier( mfStepValue ) );
	VectAdd( &xPos, &xPos, &xNewVel );

	xPos.z = LandscapeHeightmapGetHeightWorld( xPos.x, xPos.y );
	
	GetEntity()->SetPos( &xPos );

}

// When 'Setting Off', the beast first walks forward slowly and turns to the face the direction 
// it wants to go..
void	GroundMotionComponent::MoveToDestinationSettingOff( float fDelta, const VECT* pxDestination )
{
VECT	xRot = *GetEntity()->GetRot();
VECT	xPos = *GetEntity()->GetPos();
float	fDestinationRotZ;
float	fDistToDestination = VectDistNoZ( &xPos, pxDestination );
float	fCurrentSpeed = VectGetLength( &mxVelocity );
float	fNewSpeed = fCurrentSpeed;
float	fTurnRotationSpeed = mMotionSettings.mfTurnRotationSpeed;
VECT	xNewVel = { 1.0f, 0.0f, 0.0f };

	sprintf( mszDebugStateString2, "Dist: %.3f", fDistToDestination );
	fDestinationRotZ = GetDestinationRotZ( pxDestination, &xPos );

	if ( fDistToDestination < 1.0f )
	{
		if ( GetEntity()->ArrivedAtImmediateDestination() == FALSE )
		{
			GetEntity()->SetDestination( NULL );
		}
		return;
	}

	// If we need to turn
	if ( ( xRot.z - fDestinationRotZ < -A10 ) ||
		 ( xRot.z - fDestinationRotZ > A10 ) )
	{	
		// If we need to accelerate
		if ( fCurrentSpeed < mMotionSettings.mfTurnMaxSpeed )
		{
			fNewSpeed += mMotionSettings.mfAccelerationToTurnSpeed * fDelta;
			fTurnRotationSpeed *= (fCurrentSpeed/mMotionSettings.mfTurnMaxSpeed);
		}
		else if ( fCurrentSpeed > mMotionSettings.mfTurnMaxSpeed * 1.5f )
		{
			fNewSpeed -= mMotionSettings.mfDecelerationSpeed * fDelta;
		}
		else
		{
			fNewSpeed -= mMotionSettings.mfAccelerationToTurnSpeed * fDelta;
		}

		xRot.z = RotateAngleTowardsAngle( xRot.z, fDestinationRotZ, fTurnRotationSpeed );
		GetEntity()->SetRot( &xRot );

		sprintf( mszDebugStateString, "Turning %.3f", fTurnRotationSpeed );
	}
	else   // Facing the right direction (more or less)
	{
		if ( fCurrentSpeed < mMotionSettings.mfWalkSpeed )
		{
			fNewSpeed += mMotionSettings.mfAccelerationToWalkSpeed * fDelta;
			strcpy( mszDebugStateString, "Acc to walk" );
		}
		else
		{
			mMotionState = kWalking;
			mszDebugStateString[0] = 0;
		}
	}

	UpdateVelocityAndPosition( fDelta, fNewSpeed );
}



// walking needs to have velocity change as shown in :
//  http://jeb.biologists.org/content/212/16/2668/F2.large.jpg


// Once facing the right direction..
void	GroundMotionComponent::MoveToDestinationWalking( float fDelta, const VECT* pxDestination )
{
VECT	xRot = *GetEntity()->GetRot();
VECT	xPos = *GetEntity()->GetPos();
float	fDestinationRotZ;
float	fCurrentSpeed = VectGetLength( &mxVelocity );
float	fNewSpeed = fCurrentSpeed;
float	fDistToDestination = VectDistNoZ( &xPos, pxDestination );
float	fNewDistToDestination;
VECT	xNewVel = { 1.0f, 0.0f, 0.0f };

//	sprintf( mszDebugStateString2, "Dist: %.3f", fDistToDestination );

	fDestinationRotZ = GetDestinationRotZ( pxDestination, &xPos );

	// If we need to turn
	if ( ( xRot.z - fDestinationRotZ < -A10*3.0f ) ||
		 ( xRot.z - fDestinationRotZ > A10*3.0f ) )
	{
		xRot.z = RotateAngleTowardsAngle( xRot.z, fDestinationRotZ, mMotionSettings.mfTurnRotationSpeed );
		GetEntity()->SetRot( &xRot );
		strcpy( mszDebugStateString, "Turning" );
	}
	else
	{
		mszDebugStateString[0] = 0;
	}

	UpdateVelocityAndPosition( fDelta, fNewSpeed );

	fNewDistToDestination = VectDistNoZ( GetEntity()->GetPos(), pxDestination );

	if ( fNewDistToDestination > fDistToDestination )
	{
		// If within 4 metres
		if ( fNewDistToDestination < 4.0f )
		{
			// we're there or there abouts
			if ( GetEntity()->ArrivedAtImmediateDestination() == FALSE )
			{
				GetEntity()->SetDestination( NULL );
			}
		}
		else
		{
			// todo? - if we've got a path, check the next waypoint and if we're pointing in the right direction lets go 
			// to that one instead..

			// we got lost :)
			mMotionState = kSettingOff;
		}
	}
}


void	GroundMotionComponent::MoveToDestinationRunning( float fDelta, const VECT* pxDestination )
{


}


BOOL	GroundMotionComponent::MoveToDestination( float fDelta, const VECT* pxDestination )
{
	switch( mMotionState )
	{
	case kStanding:
		mMotionState = kSettingOff;
		MoveToDestinationSettingOff( fDelta, pxDestination );
		break;
	case kSettingOff:
		MoveToDestinationSettingOff( fDelta, pxDestination );
		break;
	case kWalking:
		MoveToDestinationWalking( fDelta, pxDestination );
		break;
	case kRunning:
		MoveToDestinationRunning( fDelta, pxDestination );
		break;
	}
	return( TRUE );
}

BOOL	GroundMotionComponent::OnEntitySetImmediateDestination( const VECT* pxDestination )
{
	if ( pxDestination )
	{
		mbHasImmediateDestination = TRUE;
		mxImmediateDestination = *pxDestination;
	}
	else
	{
		mbHasImmediateDestination = FALSE;
		mMotionState = kStanding;
	}
	return( TRUE );
}

void	GroundMotionComponent::OnRenderComponent( void )
{
	if ( 0 ) 
//	if ( MainGameShowDebug() == TRUE )
	{
	int	nScreenX, nScreenY;
	char	acString[256];
	float	fCurrentSpeed = VectGetLength( &mxVelocity );

		EngineGetScreenCoordForWorldCoord( GetEntity()->GetPos(), &nScreenX, &nScreenY );

		nScreenY -= 50;
		nScreenX -= 20;

		switch( mMotionState )
		{
		case kStanding:
			sprintf( acString, "Standing" );
			break;
		case kSettingOff:
			sprintf( acString, "Setting off (%.3f)", fCurrentSpeed );
			break;
		case kWalking:
			sprintf( acString, "Walking (%.3f)", fCurrentSpeed );
			break;
		case kRunning:
			sprintf( acString, "Running (%.3f)", fCurrentSpeed );
			break;
		}
	
		InterfaceSetFontFlags( FONT_FLAG_SMALL | FONT_FLAG_DROP_SHADOW );
		InterfaceText( 0, nScreenX, nScreenY, acString, 0xd0d0d0d0, 0 );
		nScreenY += 12;
		if ( mszDebugStateString[0] != 0 )
		{
			InterfaceText( 0, nScreenX, nScreenY, mszDebugStateString, 0xd0d0d0d0, 0 );
		}
		nScreenY += 12;

		if ( mszDebugStateString2[0] != 0 )
		{
			InterfaceText( 0, nScreenX, nScreenY, mszDebugStateString2, 0xd0d0d0d0, 0 );
		}
		nScreenY += 12;
		InterfaceSetFontFlags( 0 );
	}
}

BOOL	GroundMotionComponent::EntityAnimationUpdate( int hModel )
{
	switch( mMotionState )
	{
	case kRunning:
		if ( ModelGetCurrentAnimation( hModel ) == ANIM_STANDING )
		{
			ModelSetAnimationImmediate( hModel, ANIM_RUN, 0, 0, 0 );
		}	
		ModelSetAnimationNext( hModel, ANIM_RUN, 0, 0, 0 );
		break;
	case kStanding:
		ModelSetAnimationNext( hModel, ANIM_STANDING, 0, 0, 0 );
		break;
	default:
		if ( ModelGetCurrentAnimation( hModel ) == ANIM_STANDING )
		{
			ModelSetAnimationImmediate( hModel, ANIM_WALK, 0, 0, 0 );
		}	
		ModelSetAnimationNext( hModel, ANIM_WALK, 0, 0, 0 );
		break;
	}
	return( TRUE );
}