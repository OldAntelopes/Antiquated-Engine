#include <math.h>
#include <stdio.h>
#include "StandardDef.h"
#include "Engine.h"
#include "Rendering.h"
#include "Interface.h"

#include "../../Physics/PhysicsVehicle.h"

#include "../Entity.h"
#include "../EntityGraphics.h"

#include "RemotePhysicsVehicleController.h"

#define POS_TOLERANCE		0.4f
#define POSZ_TOLERANCE		0.1f

REGISTER_COMPONENT(RemotePhysicsVehicleControllerComponent, "RemotePhysicsVehicleController" );


RemotePhysicsVehicleControllerComponent::RemotePhysicsVehicleControllerComponent()
{
VECT	xZero = { 0.0f, 0.0f, 0.0f };

	mxLastReceivedPos = xZero;
	mxLastReceivedLinearVel = xZero;
	mxLastReceivedAngularVel = xZero;
	mxLastReceivedOrientation.x = 0.0f;
	mxLastReceivedOrientation.y = 0.0f;
	mxLastReceivedOrientation.z = 0.0f;
	mxLastReceivedOrientation.w = 0.0f;
	mulLastReceivedPosTimestamp = 0;
	mfPosBlendPhase = -1.0f;
}


void		RemotePhysicsVehicleControllerComponent::OnUpdateComponent( float fDelta )
{
	mpPhysicsPredictionVehicle->Update( fDelta );

	PhysicsVehicleControllerComponent::OnUpdateComponent( fDelta );

	if ( mulLastReceivedPosTimestamp != 0 )
	{
		BlendToRemotePosition( fDelta );
	}

}

void		RemotePhysicsVehicleControllerComponent::OnRenderComponent( void )
{
	// todo - player name label..

	int		nEntityGraphicNum = GetEntity()->GetEntityGraphicNum();
	int		hModel = EntityGraphicsGetModelHandle( nEntityGraphicNum );
	if ( hModel != NOTFOUND )
	{
	VECT	xPos;
	ENGINEQUATERNION		xOrientation;

		mpPhysicsPredictionVehicle->GetPosition( &xPos );
		mpPhysicsPredictionVehicle->GetOrientation( &xOrientation );

		EngineSetTexture( 0, 0 );
		EngineEnableWireframe( TRUE );
		EngineSetBlendMode( BLEND_MODE_SRCALPHA_ADDITIVE );
		ModelRenderQuat( hModel, &xPos, &xOrientation, RENDER_FLAGS_NO_STATE_CHANGE );
		EngineEnableWireframe( FALSE );
	}
}

void		RemotePhysicsVehicleControllerComponent::OnEntityInitialised( void )
{
	PhysicsVehicleControllerComponent::OnEntityInitialised();

	// TODO - Here we should create another PhysicsVehicle with flags set to not collide with the local vehicle
	// when we get position updates we should apply them to the prediction entity
	// and for the regular update we should blend the actualy vehicle with the position of the prediction entity

//	Entity*				mpPredictionEntity;
	mpPhysicsPredictionVehicle = new PhysicsVehicle;

	mpPhysicsPredictionVehicle->Initialise( GetEntity()->GetPos(), VEHICLECOLLISIONTYPE_REMOTEPREDICTIONVEHICLE, NULL );

}

void	RemotePhysicsVehicleControllerComponent::BlendToRemotePosition( float fDelta )
{
PhysicsVehicle*		pPhysicsVehicle = GetPhysicsVehicle();
VECT	xCurrentPos = *GetEntity()->GetPos();
ENGINEQUATERNION	xCurrentOrientation = *GetEntity()->GetOrientation();
VECT	xPredictionPos;
ENGINEQUATERNION	xPredictionOrientation;
VECT	xPredictionLinearVel;
//VECT	xAngularVel;
float	fDist;
VECT	xPredictedFuturePos;
VECT	xPredictionVelMod;
VECT	xCurrentVel;
VECT	xVec;

	mpPhysicsPredictionVehicle->GetPosition( &xPredictionPos );
	mpPhysicsPredictionVehicle->GetOrientation( &xPredictionOrientation );
	mpPhysicsPredictionVehicle->GetLinearVelocity( &xPredictionLinearVel);
	pPhysicsVehicle->GetLinearVelocity( &xCurrentVel );

	// Calculate where prediction vehicle would be in 2 seconds if the vel remains
	// constant (and no collisions affect the movement)
	VectScale( &xPredictionVelMod, &xPredictionLinearVel, 2.0f );
	VectAdd( &xPredictedFuturePos, &xPredictionPos, &xPredictionVelMod );

	// Calculate the velocity we'd need to get to that future point from our current location
	VectSub( &xVec, &xPredictedFuturePos, &xCurrentPos );
	VectScale( &xVec, &xVec, 1.0f / 2.0f );
	VectAdd( &xVec, &xPredictionLinearVel, &xVec );
	VectScale( &xVec, &xVec, 0.5f );

	VectLerp( &xVec, &xCurrentVel, &xVec, 0.2f );

	xVec = xPredictionLinearVel;
//	xVec.z = xPredictionLinearVel.z;
	pPhysicsVehicle->OverrideVelocity( &xVec );

	if ( mfPosBlendPhase < 0.0f )
	{
		fDist = VectDist( &xPredictionPos, &xCurrentPos );
		if ( fDist > POS_TOLERANCE )
		{
			mfPosBlendPhase = 0.0f;
		}
		pPhysicsVehicle->OverridePositionAndOrientation( &xPredictionPos, &xPredictionOrientation );
//		pPhysicsVehicle->OverrideOrientation( &xPredictionOrientation );
	}
	else
	{
	float	fPosBlendSpeed = 5.0f;
	char	acString[256];

		sprintf( acString, "Pos blend %f", mfPosBlendPhase );
		InterfaceText( 1, 100, 100, acString, 0xd0d0d0d0, 0 );
		mfPosBlendPhase += fDelta * fPosBlendSpeed;
		if ( mfPosBlendPhase < 1.0f )
		{
			VectLerp( &xPredictionPos, &xCurrentPos, &xPredictionPos, mfPosBlendPhase );
		}
		else
		{
			mfPosBlendPhase = -1.0f;
		}
		pPhysicsVehicle->OverridePositionAndOrientation( &xPredictionPos, &xPredictionOrientation );
//		pPhysicsVehicle->OverridePosition( &xPredictionPos );
	}
	
}

void		RemotePhysicsVehicleControllerComponent::OnRemotePositionUpdate( const REMOTEPHYSICSVEHICLE_POSITION_UPDATE* pxUpdate )
{
VECT	xPos;
ENGINEQUATERNION	xOrientation;
VECT	xLinearVel;
VECT	xAngularVel;
uint32	ulReceivedTimestamp = SysGetTick();
ENGINEMATRIX	xMat;
VECT			xUp = { 0.0f, 0.0f, 1.0f };

	xOrientation.x = pxUpdate->fQuat1;
	xOrientation.y = pxUpdate->fQuat2;
	xOrientation.z = pxUpdate->fQuat3;
	xOrientation.w = pxUpdate->fQuat4;
	
	EngineQuaternionNormalize( &xOrientation );
	mxLastReceivedOrientation = xOrientation;
	mulLastReceivedOrientationTimestamp = ulReceivedTimestamp;

	xPos.x = pxUpdate->fPosX;
	xPos.y = pxUpdate->fPosY;
	xPos.z = pxUpdate->fPosZ;

	EngineMatrixFromQuaternion( &xMat, &xOrientation );
	VectTransform( &xUp, &xUp, &xMat );
	VectScale( &xUp, &xUp, 0.25f );
	VectAdd( &xPos, &xPos, &xUp );

	mxLastReceivedPos = xPos;
	mulLastReceivedPosTimestamp = ulReceivedTimestamp;

	xLinearVel.x = pxUpdate->fLinearVelX;
	xLinearVel.y = pxUpdate->fLinearVelY;
	xLinearVel.z = pxUpdate->fLinearVelZ;

	mxLastReceivedLinearVel = xLinearVel;
	mulLastReceivedLinearVelTimestamp = ulReceivedTimestamp;

	xAngularVel.x = pxUpdate->fAngularVelX;
	xAngularVel.y = pxUpdate->fAngularVelY;
	xAngularVel.z = pxUpdate->fAngularVelZ;
	mxLastReceivedAngularVel = xAngularVel;
	mulLastReceivedAngularVelTimestamp = ulReceivedTimestamp;

	// when we get position updates we should apply them to the prediction entity
	mpPhysicsPredictionVehicle->OverridePositionAndOrientation( &xPos, &xOrientation );
	mpPhysicsPredictionVehicle->OverrideVelocity( &xLinearVel );

	// todo - store control states
}
