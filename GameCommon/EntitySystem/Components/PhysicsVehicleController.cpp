
#include <math.h>
#include <stdio.h>
#include "StandardDef.h"
#include "Engine.h"
#include "Rendering.h"
#include "Interface.h"

#include "../../Physics/PhysicsVehicle.h"

#include "../Entity.h"
#include "../EntityGraphics.h"

#include "PhysicsVehicleController.h"


REGISTER_COMPONENT(PhysicsVehicleControllerComponent, "PhysicsVehicleController" );




//-----------------------------------

PhysicsVehicleControllerComponent::PhysicsVehicleControllerComponent()
{
	mfSteeringAngle = 0.0f;
	mbIsReverse = FALSE;
	mfWheelSpin = 0.0F;
	mpPhysicsVehicle = NULL;
	mfSteeringSpeed = 0.1f;
	mbCanReverse = FALSE;				// These two were left uninitialised
	mfBrakeReverseDelayTimer = 0.0f;
	memset( matWheelTransforms, 0, sizeof( matWheelTransforms ) );
}


PhysicsVehicleControllerComponent::~PhysicsVehicleControllerComponent()
{
	SAFE_DELETE( mpPhysicsVehicle );
}


void	PhysicsVehicleControllerComponent::ModifySettings( const PhysicsVehicleSetup* pSetup )
{
	mfSteeringSpeed = pSetup->mfSteeringSpeed;
	mpPhysicsVehicle->ModifySettings( pSetup );
}



void	PhysicsVehicleControllerComponent::InitialisePhysicsVehicle( const PhysicsVehicleSetup* pSetup )
{
	mfSteeringSpeed = pSetup->mfSteeringSpeed;
	if ( mpPhysicsVehicle )
	{
		delete mpPhysicsVehicle;
	}

	mpPhysicsVehicle = new PhysicsVehicle;

	if ( IsLocalVehicle() )
	{
		mpPhysicsVehicle->Initialise( GetEntity()->GetPos(), VEHICLECOLLISIONTYPE_LOCALVEHICLE, pSetup );
	}
	else
	{
		mpPhysicsVehicle->Initialise( GetEntity()->GetPos(), VEHICLECOLLISIONTYPE_REMOTEVEHICLE, pSetup );
	}
}


void	PhysicsVehicleControllerComponent::OnEntityInitialised( void )
{
}

void	PhysicsVehicleControllerComponent::RenderDebug( void )
{
	mpPhysicsVehicle->RenderDebug();
}

void	PhysicsVehicleControllerComponent::ApplySteering( float fDelta )
{
float	fTurnSpeed = mfSteeringSpeed;			// Should lower as we get faster?		// SHould come from the game
PhysicsVehicleSetup*	pSetup = mpPhysicsVehicle->GetCurrentSetup();
float	fMaxAngle = pSetup->GetSteeringMaxAngle();
float	fTurnSpeedWhenReturningToMiddle = pSetup->GetSteeringReturnSpeed();
float	fCounterSteerSpeed;

	// The lock narrows with speed when a fade speed is set: halved at that many mph
	if ( pSetup->GetSteeringFadeMPH() > 0.0f )
	{
		fMaxAngle = fMaxAngle / ( 1.0f + ( fabsf( mpPhysicsVehicle->GetCurrentSpeedMPH() ) / pSetup->GetSteeringFadeMPH() ) );
	}
	// Steering back across the middle goes at least as fast as letting go would
	fCounterSteerSpeed = ( fTurnSpeedWhenReturningToMiddle > fTurnSpeed ) ? fTurnSpeedWhenReturningToMiddle : fTurnSpeed;

	if ( GetControlState( CONTROL_STEER_LEFT ) > 0.0f )
	{
		mfSteeringAngle -= ( ( mfSteeringAngle > 0.0f ) ? fCounterSteerSpeed : fTurnSpeed ) * fDelta;
		if ( mfSteeringAngle < -fMaxAngle )
		{
			mfSteeringAngle = -fMaxAngle;
		}
		if ( mfSteeringAngle > fMaxAngle ) mfSteeringAngle = fMaxAngle;
	}
	else if ( GetControlState( CONTROL_STEER_RIGHT ) > 0.0f )
	{
		mfSteeringAngle += ( ( mfSteeringAngle < 0.0f ) ? fCounterSteerSpeed : fTurnSpeed ) * fDelta;
		if ( mfSteeringAngle > fMaxAngle )
		{
			mfSteeringAngle = fMaxAngle;
		}
		if ( mfSteeringAngle < -fMaxAngle ) mfSteeringAngle = -fMaxAngle;
	}
	else
	{

		if ( mfSteeringAngle < 0.0f )
		{
			mfSteeringAngle += fTurnSpeedWhenReturningToMiddle * fDelta; // mxVehicleSettings.fTurnDampening;
			if ( mfSteeringAngle > 0.0f )
			{
				mfSteeringAngle = 0.0f;
			}
		}
		else if ( mfSteeringAngle > 0.0f )
		{
			mfSteeringAngle -= fTurnSpeedWhenReturningToMiddle * fDelta; //  mxVehicleSettings.fTurnDampening;
			if ( mfSteeringAngle < 0.0f )
			{
				mfSteeringAngle = 0.0f;
			}
		}
	}
}

void	PhysicsVehicleControllerComponent::GetLinearVelocity( VECT* pxOut )
{
	mpPhysicsVehicle->GetLinearVelocity( pxOut );
}

void	PhysicsVehicleControllerComponent::RespawnVehicle( const VECT* pxSpawnPos )
{
ENGINEQUATERNION	xQuat = { 0.0f, 0.0f, 0.0f, 1.0f };
//VECT				xVel = { 0.0f, 0.0f, 0.0f };

	mpPhysicsVehicle->OverridePositionAndOrientation( pxSpawnPos, &xQuat );
	// Without this a vehicle respawned after falling through the world kept its falling speed
	mpPhysicsVehicle->StopMoving();

}

void	PhysicsVehicleControllerComponent::GetEngineValues( int* pnGear, int* pnRPM )
{
	*pnGear = mpPhysicsVehicle->GetCurrentGear();
	*pnRPM = mpPhysicsVehicle->GetCurrentRPM();
}

void	PhysicsVehicleControllerComponent::OnUpdateComponent( float fDelta )
{
VECT	xPos;
VECT	xVel;
ENGINEQUATERNION	xQuat;
float		fAccelerateState;
float		fBrakeState;
VECT	xUp = { 0.0f, 0.0f, 1.0f };
//ENGINEMATRIX	xMat;
int		nLoop;
float		fSpeed = mpPhysicsVehicle->GetCurrentSpeedMPH();

	fAccelerateState = GetControlState( CONTROL_ACCELERATE );
	fBrakeState = GetControlState( CONTROL_BRAKE );

	if ( !mbIsReverse )
	{
		if ( fAccelerateState == 0.0f )
		{
			if ( !mbCanReverse ) 
			{
				if ( mpPhysicsVehicle->GetCurrentSpeedMPH() < 1.0f )
				{
					if ( ( fBrakeState == 0.0f ) ||
						 ( mfBrakeReverseDelayTimer <= 0.0f ) )
					{
						mbCanReverse = TRUE;
					}
					else
					{
						mfBrakeReverseDelayTimer -= fDelta;
					}
				}
				else
				{
					mfBrakeReverseDelayTimer = 0.3f;
				}
			}
			else if ( mpPhysicsVehicle->GetCurrentSpeedMPH() >= 1.0f )
			{
				// Moving forward again (coasting): the brake brakes until we've stopped
				mbCanReverse = FALSE;
				mfBrakeReverseDelayTimer = 0.3f;
			}
			else
			{
				if ( fBrakeState > 0.0f )
				{
					mbIsReverse = TRUE;
					fAccelerateState = 0.0f - fBrakeState;
				}
			}
		}
		else
		{
			// Driving forward. Without this, having once stopped left reverse armed, so the brake at speed drove
			// the engine backwards instead of braking
			mbCanReverse = FALSE;
			mfBrakeReverseDelayTimer = 0.3f;
		}
	}
	else  // we're currently reversing
	{
		if ( fAccelerateState == 0.0f )
		{
			fAccelerateState = 0.0f - fBrakeState;
			fBrakeState = 0.0f;
		}
		else  // we're pressing accelerate
		{
			// If we're reversing 
			if ( fSpeed < -0.0001f )
			{
				fBrakeState = fAccelerateState * 1.25f;
				fAccelerateState = 0.0f;
			}
			else
			{
				mbIsReverse = FALSE;
				mbCanReverse = FALSE;
			}
		}
	}

	ApplySteering( fDelta );

	int		nEntityGraphicNum = GetEntity()->GetEntityGraphicNum();
	int		hModel = EntityGraphicsGetModelHandle( nEntityGraphicNum );
	if ( hModel != NOTFOUND )
	{
	float		fSpeed = mpPhysicsVehicle->GetCurrentSpeedMPH();

		mfWheelSpin += (fSpeed * -0.002f);
		if ( mfWheelSpin > A360 ) mfWheelSpin -= A360;
		if ( mfWheelSpin < 0.0f ) mfWheelSpin += A360;

		ModelSetFrontWheelTurnRotation( hModel, 0.0f - mfSteeringAngle );
		ModelSetWheelSpinRotation( hModel, mfWheelSpin );
	}

	float fRollingFriction = 0.1f;

	mpPhysicsVehicle->SetControls( mfSteeringAngle * 1.0f, fAccelerateState, fBrakeState + fRollingFriction );
	mpPhysicsVehicle->Update( fDelta );

	mpPhysicsVehicle->GetPosition( &xPos );
	mpPhysicsVehicle->GetOrientation( &xQuat );

	GetEntity()->SetOrientation( &xQuat );
	GetEntity()->SetPos( &xPos );

	for ( nLoop = 0; nLoop < 4; nLoop++ )
	{
		mpPhysicsVehicle->GetWheelPosition( nLoop, &matWheelTransforms[nLoop] );
	}

	mpPhysicsVehicle->GetLinearVelocity( &xVel );
	GetEntity()->SetVel( &xVel );
}
