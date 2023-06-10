#ifndef PHYSICS_MOTION_COMPONENT_H
#define	PHYSICS_MOTION_COMPONENT_H

#include "Component.h"

enum ePhysicsCollisionShape
{
	kPhysicsPrimitiveShape_Sphere,
	kPhysicsPrimitiveShape_Plane,
	kPhysicsPrimitiveShape_Box,
};

class PhysicsMotionSettings
{
public:
	PhysicsMotionSettings();

	float	mfMass;
	ePhysicsCollisionShape		mCollisionShape;
	VECT	mxCollisionSize;
	float	mfLinearFriction;
};

class PhysicsMotionComponent : public Component
{
public:
	PhysicsMotionComponent();
	~PhysicsMotionComponent();

	virtual const char*			GetComponentType( void ) { return( "PhysicsMotion" ); }
	virtual eCompronentBehaviourTypes	GetBehaviourType( void ) { return( MOVEMENT_BEHAVIOUR_COMPONENT ); }

	virtual void		OnUpdateComponent( float fDelta );
	virtual void		OnRenderComponent( void );
	virtual void		OnEntityAddToWorld( void );
	virtual void		OnEntitySetVel( const VECT* pxVel );
	virtual void		OnEntityRespawn( const VECT* pxPos );

	void				ApplyLinearForce( const VECT* pxForce );
	void				ApplyAngularForce( const VECT* pxForce );

	void				SetPhysicsSettings( const PhysicsMotionSettings* pxMotionSettings );

private:

	int							mhPhysicsObject;
	PhysicsMotionSettings		mMotionSettings;
};



#endif
