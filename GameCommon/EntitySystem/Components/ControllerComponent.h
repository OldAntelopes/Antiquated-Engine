#ifndef CONTROLLER_COMPONENT_H
#define CONTROLLER_COMPONENT_H

#include "Component.h"

enum eControl
{
	CONTROL_STEER_LEFT,
	CONTROL_STEER_RIGHT,
	CONTROL_ACCELERATE,
	CONTROL_BRAKE,
	CONTROL_PITCH_FORWARD,
	CONTROL_PITCH_BACK,

	CONTROL_SHIFT,
	
	NUM_CONTROL_STATES,
};

class ControllerComponent : public Component
{
public:
	ControllerComponent();

	virtual eCompronentBehaviourTypes	GetBehaviourType( void ) { return( CONTROL_BEHAVIOUR_COMPONENT ); }

	virtual void		OnUpdateComponent( float fDelta ) = 0;

	void	SetControl( eControl, float fAmount );
	void	ResetControlStates( void );

	float	GetControlState( eControl );
private:

	float		mafControlStates[ NUM_CONTROL_STATES ];
};



#endif
