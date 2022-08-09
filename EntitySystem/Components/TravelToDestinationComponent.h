#ifndef TRAVEL_TO_DESTINATION_COMPONENT_H
#define	TRAVEL_TO_DESTINATION_COMPONENT_H

#include "Component.h"

class TravelToDestinationComponent : public Component
{
public:
	TravelToDestinationComponent();

	virtual eCompronentBehaviourTypes	GetBehaviourType( void ) { return( MOVEMENT_BEHAVIOUR_COMPONENT ); }

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

	GroundMotionSettings		mMotionSettings;
	//----------------------------------------------------
	char				mszDebugStateString[64];
	char				mszDebugStateString2[64];
};
