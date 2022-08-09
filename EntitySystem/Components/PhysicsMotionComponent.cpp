
#include <stdio.h>
#include "StandardDef.h"
#include "Engine.h"
#include "Rendering.h"
#include "Interface.h"

#include "../../Util/MathsUtil.h"
#include "../../Landscape/LandscapeHeightmap.h"
#include "../Entity.h"
#include "../../Physics/PhysicsObjects.h"

#include "PhysicsMotionComponent.h"

REGISTER_COMPONENT(PhysicsMotionComponent, "PhysicsMotion" );

//-----------------------------------------------------------------------------
PhysicsMotionSettings::PhysicsMotionSettings()
{
	mfMass = 1.0f;
	mxCollisionSize.x = 0.4f;
	mxCollisionSize.y = 0.4f;
	mxCollisionSize.z = 0.4f;
	mCollisionShape = kPhysicsPrimitiveShape_Box;
	mfLinearFriction = 0.5f;
}


PhysicsMotionComponent::PhysicsMotionComponent()
{
	mhPhysicsObject = 0;
}

void	PhysicsMotionComponent::SetPhysicsSettings( const PhysicsMotionSettings* pxMotionSettings )
{
	if ( pxMotionSettings )
	{
		mMotionSettings = *pxMotionSettings;
	}
}


void	PhysicsMotionComponent::OnEntityAddToWorld( void )
{
VECT	xPos = *GetEntity()->GetPos();
VECT	xRot = *GetEntity()->GetRot();

	mhPhysicsObject = PhysicsObjectAddPrimitive( (int)mMotionSettings.mCollisionShape, &xPos, &xRot, &mMotionSettings.mxCollisionSize, mMotionSettings.mfMass, mMotionSettings.mfLinearFriction );
}


void	PhysicsMotionComponent::OnUpdateComponent( float fDelta )
{
PhysicsObjectTransform		mPhysicsTransform;
VECT	xLinearVel;

	PhysicsObjectGetTransform( mhPhysicsObject, &mPhysicsTransform );
	GetEntity()->SetOrientation( &mPhysicsTransform.mxQuatRotation );
	GetEntity()->SetPos( &mPhysicsTransform.mxPosition );

	PhysicsObjectGetLinearVelocity( mhPhysicsObject, &xLinearVel );
	GetEntity()->SetVelInternal( &xLinearVel );

}

void	PhysicsMotionComponent::OnEntitySetVel( const VECT* pxVel )
{
	if ( mhPhysicsObject )
	{
		PhysicsObjectSetVelocity( mhPhysicsObject, pxVel );
	}
}

void	PhysicsMotionComponent::OnEntityRespawn( const VECT* pxPos )
{
ENGINEQUATERNION	xQuat = { 0.0f, 0.0f, 0.0f, 1.0f };
//VECT				xVel = { 0.0f, 0.0f, 0.0f };

	if ( mhPhysicsObject )
	{
		PhysicsObjectOverridePositionAndOrientation( mhPhysicsObject, pxPos, &xQuat );
	}
}

void	PhysicsMotionComponent::ApplyLinearForce( const VECT* pxForce )
{
	if ( mhPhysicsObject )
	{
		PhysicsObjectApplyLinearForce( mhPhysicsObject, pxForce );
	}
}

void	PhysicsMotionComponent::ApplyAngularForce( const VECT* pxForce )
{
	if ( mhPhysicsObject )
	{
		PhysicsObjectApplyAngularForce( mhPhysicsObject, pxForce );
	}
}




void	PhysicsMotionComponent::OnRenderComponent( void )
{
/*
VECT	xPos = *GetEntity()->GetPos();
char	acString[256];

	sprintf( acString, "Pos: %.2f, %.2f, %.2f", xPos.x, xPos.y, xPos.z );
	InterfaceText( 1, 100, 100, acString, 0xD0D0D0D0, 0 );
*/
}
