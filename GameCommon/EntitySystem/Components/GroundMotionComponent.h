#ifndef GROUND_MOTION_COMPONENT_H
#define	GROUND_MOTION_COMPONENT_H

#include "Component.h"

class GroundMotionSettings
{
public:
	GroundMotionSettings();

	float	mfTurnMaxSpeed;
	float	mfWalkSpeed;
	float	mfAccelerationToTurnSpeed;
	float	mfAccelerationToWalkSpeed;
	float	mfTurnRotationSpeed;
	float	mfDecelerationSpeed;
	float	mfStrideSpeed;
	float	mfDecelerationToStop;
};

class GroundMotionComponent : public Component
{
public:
	GroundMotionComponent();

	virtual const char*			GetComponentType( void ) { return( "GroundMotion" ); }

	virtual eCompronentBehaviourTypes	GetBehaviourType( void ) { return( MOVEMENT_BEHAVIOUR_COMPONENT ); }
	virtual BOOL		OnEntitySetImmediateDestination( const VECT* pxDestination );

	virtual void		OnUpdateComponent( float fDelta );
	virtual void		OnRenderComponent( void );

	virtual	BOOL		MoveToDestination( float fDelta, const VECT* pxDestination );
	virtual BOOL		EntityAnimationUpdate( int hModel );

private:
	enum eMotionState
	{
		kStanding,
		kSettingOff,
		kWalking,
		kRunning,
	};

	void	MoveToDestinationSettingOff( float fDelta, const VECT* pxDestination );
	void	MoveToDestinationWalking( float fDelta, const VECT* pxDestination );
	void	MoveToDestinationRunning( float fDelta, const VECT* pxDestination );
	
	void	UpdateVelocityAndPosition( float fDelta, float fNewSpeed );

	eMotionState		mMotionState;

	float				mfStepValue;
	VECT				mxVelocity;
	BOOL				mbHasImmediateDestination;
	VECT				mxImmediateDestination;

	GroundMotionSettings		mMotionSettings;
	//----------------------------------------------------
	char				mszDebugStateString[64];
	char				mszDebugStateString2[64];
};




#endif