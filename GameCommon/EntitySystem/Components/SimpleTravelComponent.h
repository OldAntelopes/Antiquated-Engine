#ifndef SIMPLE_TRAVEL_COMPONENT_H
#define	SIMPLE_TRAVEL_COMPONENT_H

#include "Component.h"

class SimpleTravelComponent : public MovementComponent
{
public:
	SimpleTravelComponent();

	virtual const char*			GetComponentType( void ) { return( "SimpleTravel" ); }
	virtual eCompronentBehaviourTypes	GetBehaviourType( void ) { return( MOVEMENT_BEHAVIOUR_COMPONENT ); }

	virtual void		OnUpdateComponent( float fDelta );
	virtual void		OnRenderComponent( void );

	virtual BOOL		OnEntitySetDestination( const VECT* pxDestination );
	virtual	BOOL		MoveToDestination( float fDelta, const VECT* pxDestination );
	virtual BOOL		EntityAnimationUpdate( int hModel );

	virtual BOOL		HasDestination( void );
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

	VECT		mxDestination;
	BOOL		mbIsTravelling;
	
	uint32		mulTargetEntityUID;
	float		mfTargetDist;

//	GroundMotionSettings		mMotionSettings;
	//----------------------------------------------------
	char				mszDebugStateString[64];
	char				mszDebugStateString2[64];
};



#endif