#include <math.h>
#include <stdio.h>
#include "StandardDef.h"
#include "Engine.h"
#include "Interface.h"

#include "../../Landscape/Landscape.h"
#include "../../Landscape/LandscapeHeightmap.h"
#include "../../Util/MathsUtil.h"

#include "../Entity.h"
#include "VehicleController.h"

#define		VEHICLE_CONTROLLER_ONSCREEN_DEBUG

REGISTER_COMPONENT(VehicleControllerComponent, "VehicleController" );

typedef struct 
{
	float	fTopSpeed;
	float	fTopSpeedReverse;
	float	fAcceleration;
	float	fAccelerationReverse;
	float	fBrake;

	float	fMinSkidSpeed;
	float	fTractionLimit;
	float	fFriction;
	float	fSkidFriction;	
	float	fTurnDampening;

} VEHICLE_SETTINGS;

VEHICLE_SETTINGS		mxVehicleSettings =
{
	1.0f,		// top speed
	0.5f,		// top speed reverse
	0.015f,		// acceleration
	0.015f,		// acceleration reverse
	0.1f,		// Brake

	0.1f,		// Min Skid Speed
	0.9f,		// Traction Limit
	0.01f,		// Friction
	0.004f,		// Skid Friction	
	1.0f,		// fTurnDampening
};

VehicleControllerComponent::VehicleControllerComponent()
{
	memset( &mxVel, 0, sizeof( mxVel ) );
	mfSteeringAngle = 0.0f;
	mbOnGround = TRUE;
}

VECT	VehicleControllerComponent::GetSteeringVector( void )
{
Entity*		pEntity = GetEntity();
float		fTravelAngle = pEntity->GetRot()->z + mfSteeringAngle;
VECT		xVect;
VECT		xRot = *pEntity->GetRot();
ENGINEMATRIX	xMatrix;

	xRot.z += mfSteeringAngle;
	EngineSetMatrixFromRotations( &xRot, &xMatrix );

	xVect.x = 1.0f;
	xVect.y = 0.0f;
	xVect.z = 0.0f;

//	VectRotateAboutZ( &xVect, fTravelAngle );
	VectTransform( &xVect, &xVect, &xMatrix );

	return( xVect );
}

VECT	VehicleControllerComponent::GetForwardVector( void )
{
Entity*		pEntity = GetEntity();
//float		fTravelAngle = pEntity->GetRot()->z;
VECT		xVect;
ENGINEMATRIX	xMatrix;

	EngineSetMatrixFromRotations( pEntity->GetRot(), &xMatrix );

	xVect.x = 1.0f;
	xVect.y = 0.0f;
	xVect.z = 0.0f;

	VectTransform( &xVect, &xVect, &xMatrix );
//	VectRotateAboutZ( &xVect, fTravelAngle );

	return( xVect );
}

void	VehicleControllerComponent::ApplySteering( float fDelta )
{
float	fTurnSpeed = 10.0f;

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
		if ( mfSteeringAngle < 0.0f )
		{
			mfSteeringAngle += fTurnSpeed * fDelta * mxVehicleSettings.fTurnDampening;
			if ( mfSteeringAngle > 0.0f )
			{
				mfSteeringAngle = 0.0f;
			}
		}
		else if ( mfSteeringAngle > 0.0f )
		{
			mfSteeringAngle -= fTurnSpeed * fDelta * mxVehicleSettings.fTurnDampening;
			if ( mfSteeringAngle < 0.0f )
			{
				mfSteeringAngle = 0.0f;
			}
		}
	}
}


void	VehicleControllerComponent::ApplyControls( float fDelta )
{
Entity*		pEntity = GetEntity();
VECT	xVect;
float	fSpeedMax;
float	fSpeed = VectGetLength( &mxVel );
VECT	xOrientationVector;
float	fDot;
BOOL	boReverse = FALSE;
float	fEnginePitch;
float	fControl;

	xVect = mxVel;
	VectNormalize( &xVect );
	xOrientationVector.x = 0 - (float)( sinf( pEntity->GetRot()->z ) );
	xOrientationVector.y = 0 - (float)( cosf( pEntity->GetRot()->z ) );
	xOrientationVector.z = 0.0f;
	VectNormalize( &xOrientationVector );
	fDot = VectDot( &xVect, &xOrientationVector );
	
	if ( fDot < -0.1f )
	{
		fSpeedMax = mxVehicleSettings.fTopSpeedReverse;
		boReverse = TRUE;
	}
	else
	{
		fSpeedMax = mxVehicleSettings.fTopSpeed;
	}

	fEnginePitch = fabsf( fSpeed / fSpeedMax ) * 0.9f;

	fControl = GetControlState( CONTROL_ACCELERATE );
	if ( fControl > 0.0f )
	{
		mfFrameAcceleration = ( fDelta * mxVehicleSettings.fAcceleration * fControl );
		fEnginePitch += 0.1f;
	}
	else if ( GetControlState( CONTROL_BRAKE ) > 0.0f )
	{
		fControl = GetControlState( CONTROL_BRAKE );
		if ( boReverse == TRUE )
		{
			mfFrameAcceleration = 0 - ( fDelta * mxVehicleSettings.fAccelerationReverse * fControl);
		}
		else
		{
			mfFrameAcceleration = 0 - ( fDelta * mxVehicleSettings.fBrake * fControl );
		}
		fEnginePitch += 0.05f;
	}
	else
	{
		mfFrameAcceleration = 0.0f;
	}

//	EngineNoiseSetPitch( gwWhoIAm, fEnginePitch );

	ApplySteering( fDelta );
}

float	VehicleControllerComponent::ApplyFriction( float fDelta, float fFrictionValue )
{
VECT	xVect;
float	fFrictionThisFrame = 0.0f;

	// Apply friction 
	xVect = mxVel;
	VectNormalize( &xVect );

	fFrictionThisFrame = fDelta * fFrictionValue;

	VectScale( &xVect, &xVect, fFrictionThisFrame );

	VectSub( &mxVel, &mxVel, &xVect );

	return( fFrictionThisFrame );

}

void	VehicleControllerComponent::AddGravity( float fDelta )
{
float	fGravity = 0.9f;

	// TODO
	// not doing anything as we can't leave the ground yet..
	mxVel.z -= fDelta * fGravity;
}

float	VehicleControllerComponent::GetSkidValue( const VECT* pxWheelVector, const VECT* pxVel )
{
VECT	xVect;
float	fSpeed = VectGetLength( pxVel );
VECT	xNormalizedVel = *pxVel;
float	fSpeedFactor;
float	fTraction;
float	fSkidValue;
float	fDot;
float	fReverseDot;

	fTraction = mxVehicleSettings.fTractionLimit;

	// Now work out whether or not we're 'skidding'
	xVect = *pxWheelVector;
	VectNormalize( &xVect );
	VectNormalize( &xNormalizedVel );

	// Are we reversing?
	// Get dot of horiz wheel vector with horiz worldvelocity
	fReverseDot = VectDot( &xNormalizedVel, &xVect );
	fDot = 1.0f - fReverseDot;

	fDot = asinf( fDot );
//	fDot /= fDelta;
	fDot *= 1000.0f;

	fSpeedFactor = (fSpeed - 0.001f) * 500.0f;
	if ( fSpeedFactor < 0.00001f )
	{
		fSpeedFactor = 0.00001f;
	}
		
	fDot *= fSpeedFactor;
	fDot = fabsf(fDot);

	if ( fTraction > 0.0f )
	{
		fSkidValue = fDot * (1.0f / fTraction);
	}
	else
	{	
		fSkidValue = fDot;
	}
		
	fSkidValue = (logf( fSkidValue ) + 4.0f) / 4.0f;
	if ( fSkidValue < 0.0f ) 
	{
		fSkidValue = 0.0f;
	}
	return( fSkidValue );
}

void	VehicleControllerComponent::UpdatePhysics( float fDelta )
{
Entity*		pEntity = GetEntity();
VECT	xVect;
float	fMinSkidSpeed;
float	fTraction;
float	fAcceleration;
float	fSpeed = VectGetLength( &mxVel );
BOOL	boIsReversing = FALSE;
float	fFrictionThisFrame = 0.0f;
VECT	xWheelVector;
VECT	xDriveVector = GetForwardVector();
VECT	xVect2;
BOOL	bIsSkidding = FALSE;
VECT	xOriginalVel = mxVel;
float	fSkidValue = 0.0f;
float	fFriction = 0.0f;
float	fSkidFriction = 0.0f;
float	fSpeedFactor = 0.0f;
float	fReverseDot = 0.0f;
VECT	xRot = *pEntity->GetRot();
char	acString[256];

	fMinSkidSpeed = mxVehicleSettings.fMinSkidSpeed;
	fTraction = mxVehicleSettings.fTractionLimit;
	fFriction = mxVehicleSettings.fFriction;
	fSkidFriction = mxVehicleSettings.fSkidFriction;	

	fAcceleration = mfFrameAcceleration;

	xWheelVector = GetSteeringVector();

	if ( VectGetLength( &xOriginalVel ) == 0.0f )
	{
		xOriginalVel = xWheelVector;
	}

	// If on the ground
	if ( mbOnGround )
	{ 
		xDriveVector = LandscapeGetAlignedForwardVector( pEntity->GetPos(), xRot.z, 1.5f, NULL );

		// if we have any velocity
		if ( fSpeed > 0.0f )
		{
			fFrictionThisFrame = ApplyFriction( fDelta, fFriction );
			fSpeed = VectGetLength( &mxVel );

#ifdef VEHICLE_CONTROLLER_ONSCREEN_DEBUG
			sprintf( acString, "Acc = %.4f  Friction = %.4f", fAcceleration, fFrictionThisFrame );
			InterfaceText( 0, 10, 100, acString, 0xFFFFFFFF, 0 );
#endif
		}

		// if on the ground and accelerating
		if ( fAcceleration != 0.0f )
		{
			// If in a skid already
			// wheelvector is the direction the vehicle (wheels) are pointing.. (and hence the direction the acceleration is applied in)
			// (NB fAcceleration already contains frame delta)
			xVect.x = xDriveVector.x * fAcceleration;
			xVect.y = xDriveVector.y * fAcceleration;
			xVect.z = xDriveVector.z * fAcceleration;

			// Add the acceleration to the current world velocity
			VectAdd( &mxVel, &mxVel, &xVect );

#ifdef VEHICLE_CONTROLLER_ONSCREEN_DEBUG
			sprintf( acString, "Acc = %.4f, %.4f, %.4f", xVect.x, xVect.y, xVect.z );
			InterfaceText( 0, 10, 150, acString, 0xFFFFFFFF, 0 );
			sprintf( acString, "Vel = %.4f, %.4f, %.4f", mxVel.x, mxVel.y, mxVel.z );
			InterfaceText( 0, 10, 170, acString, 0xFFFFFFFF, 0 );
#endif
			// Now add the gravity for this frame
			AddGravity( fDelta );				
		}
		else
		{
			// Stop it if we're moving slowly and not pressing accelerate
			if ( fSpeed * fDelta <= fFrictionThisFrame )
			{
				mxVel.x = 0.0f;
				mxVel.y = 0.0f;
				mxVel.z = 0.0f;
			}
			else
			{
				// Now add the gravity for this frame
				AddGravity( fDelta );				
			}
		}

		// If we're moving...
		if ( fSpeed > 0.0001f )
		{
			fSkidValue = GetSkidValue( &xWheelVector, &xOriginalVel );

			fSkidValue = 0.0f;

			// If we're skidding..
			if ( fSkidValue > 0.0f )
			{
				if ( fSkidValue > 1.5f )
				{
					fSkidValue = 1.5f;
				}

	//			pxPlayerDetails->bAccFlag |= ACCFLAG_INSKID;
				bIsSkidding = TRUE;
	//			AddSkidEffects( pxPlayer, pxPlayerDetails, nVehicleNum, fSkidValue, fDelta );

				// Apply extra skid friction
				fFrictionThisFrame = ApplyFriction( fDelta, fSkidFriction * fSkidValue );

				fSpeed = VectGetLength( &mxVel );
				// Travel direction moves away from the wheel vector and towards the previous movement vector as the
				//  skidValue approaches 1.0f
 
				fSkidValue = fSkidValue * 0.7f;
				fSkidValue *= 0.9f;
				xVect = xWheelVector;
				VectNormalize( &xVect );

				xVect2 = mxVel;
				VectNormalize( &xVect2 );

				xVect.x = (xVect.x * (1.0f-fSkidValue) ) + ( xVect2.x * fSkidValue );
				xVect.y = (xVect.y * (1.0f-fSkidValue) ) + ( xVect2.y * fSkidValue );
				xVect.z = (xVect.z * (1.0f-fSkidValue) ) + ( xVect2.z * fSkidValue );
				VectScale( &xVect, &xVect, fSpeed );
				mxVel = xVect;
			}
			// if not skidding.. align movement with wheel vector
			else
			{
	//			pxPlayerDetails->bAccFlag &= ~ACCFLAG_INSKID;
				fSpeed = VectGetLength( &mxVel );
				xVect = xDriveVector;//xWheelVector;
				VectNormalize( &xVect );
				if ( fReverseDot < 0.0f )
				{
					VectScale( &xVect, &xVect, -fSpeed );
				}
				else
				{
					VectScale( &xVect, &xVect, fSpeed );
				}

				mxVel = xVect;
//				AddGravity( fDelta );

				// Rotate chassis 
				float fRotationSpeed = mfSteeringAngle * 0.05f;
				xRot.z += fDelta * fRotationSpeed;
				xRot.z = WrapAngle360( xRot.z );

				pEntity->SetRot( &xRot );

#ifdef VEHICLE_CONTROLLER_ONSCREEN_DEBUG
				sprintf( acString, "steer: %.4f", mfSteeringAngle );
				InterfaceText( 0, 10, 190, acString, 0xFFFFFFFF, 0 );
#endif
			}

			// --------------------- Limit the top speed
			float	fSpeedMax = mxVehicleSettings.fTopSpeed;
			fSpeed = VectGetLength( &mxVel );

			if ( fSpeed > fSpeedMax )
			{
//				VectNormalize( &mxVel );
//				VectScale( &mxVel, &mxVel, fSpeedMax );
			}
#ifdef VEHICLE_CONTROLLER_ONSCREEN_DEBUG
			sprintf( acString, "Spd = %.4f  MaxSpeed = %.4f", fSpeed, fSpeedMax );
			InterfaceText( 0, 10, 120, acString, 0xffffffff, 0 );
#endif
			// Store the current angle of travel
	//		CalcTravelRotationFromVel( &pxPlayer->xWorldVelocity, &pxPlayer->fTravelRotation );
		}
		else	// vel was all 0.0f
		{

		}
	}
	else	// was in air
	{
		// TODO - air resistance
		AddGravity( fDelta );				
	}

	
}


void	VehicleControllerComponent::UpdatePositionAndOrientation( float fDelta )
{
Entity*		pEntity = GetEntity();
VECT	xVelThisFrame;
VECT	xPos;
VECT	xRot = *pEntity->GetRot();
float	fNewPosZ;

	VectScale( &xVelThisFrame, &mxVel, fDelta );
	VectAdd( &xPos, pEntity->GetPos(), &xVelThisFrame );

	fNewPosZ = LandscapeHeightmapGetHeightWorld( xPos.x, xPos.y );
#ifdef VEHICLE_CONTROLLER_ONSCREEN_DEBUG
	char	acString[256];
	sprintf( acString, "posZ = %.4f  floor = %.4f", xPos.z, fNewPosZ );
	InterfaceText( 0, 10, 210, acString, 0xffffffff, 0 );
#endif
	if ( xPos.z <= fNewPosZ + 0.01f )
	{
	float	fSpeed = VectGetLength( &mxVel );

		// Align velocity to ground
		mxVel = LandscapeGetAlignedForwardVector( &xPos, xRot.z, 1.5f, &xRot );
		VectScale( &mxVel, &mxVel, fSpeed );
		xPos.z = fNewPosZ;
		mbOnGround = TRUE;
	}
	else
	{
		mbOnGround = FALSE;
	}

	pEntity->SetPos( &xPos );
	pEntity->SetRot( &xRot );
	
}

void	VehicleControllerComponent::OnUpdateComponent( float fDelta )
{
	ApplyControls( fDelta );

	UpdatePhysics( fDelta );

	UpdatePositionAndOrientation( fDelta );

}
