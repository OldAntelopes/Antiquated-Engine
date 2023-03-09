
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

}


PhysicsVehicleControllerComponent::~PhysicsVehicleControllerComponent()
{
	SAFE_DELETE( mpPhysicsVehicle );
}


void	PhysicsVehicleControllerComponent::ModifySettings( const PhysicsVehicleSetup* pSetup )
{
	mpPhysicsVehicle->ModifySettings( pSetup );
}



void	PhysicsVehicleControllerComponent::InitialisePhysicsVehicle( const PhysicsVehicleSetup* pSetup )
{
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
float	fTurnSpeed = 0.3f;			// Should lower as we get faster?

	if ( GetControlState( CONTROL_STEER_LEFT ) > 0.0f )
	{
		mfSteeringAngle -= fTurnSpeed * fDelta;
		if ( mfSteeringAngle < -A30 )
		{ 
			mfSteeringAngle = -A30;
		}
	}
	else if ( GetControlState( CONTROL_STEER_RIGHT ) > 0.0f )
	{
		mfSteeringAngle += fTurnSpeed * fDelta;
		if ( mfSteeringAngle > A30 )
		{
			mfSteeringAngle = A30;
		}
	}
	else
	{
	float	fTurnSpeedWhenReturningToMiddle = 2.0f;

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
ENGINEMATRIX	xMat;
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

//	xPos.z -= 0.29f;		// TODO - PhysicsVehicle position is base of the chassis rigid body, remove half height from vehicle pos

	GetEntity()->SetOrientation( &xQuat );

	EngineMatrixFromQuaternion( &xMat, &xQuat );
	VectTransform( &xUp, &xUp, &xMat );
//	VectScale( &xUp, &xUp, -0.6f );			// WRONG !!!!
//	VectAdd( &xPos, &xPos, &xUp );
	GetEntity()->SetPos( &xPos );

	for ( nLoop = 0; nLoop < 4; nLoop++ )
	{
		mpPhysicsVehicle->GetWheelPosition( nLoop, &matWheelTransforms[nLoop] );
	}

	mpPhysicsVehicle->GetLinearVelocity( &xVel );
	GetEntity()->SetVel( &xVel );
}
