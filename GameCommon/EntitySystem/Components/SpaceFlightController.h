#ifndef SPACE_FLIGHT_CONTROLLER_COMPONENT_H
#define	SPACE_FLIGHT_CONTROLLER_COMPONENT_H


#include "ControllerComponent.h"


class SpaceFlightControllerComponent : public ControllerComponent
{
public:
	SpaceFlightControllerComponent();

	virtual const char*			GetComponentType( void ) { return( "SpaceFlightController" ); }

	virtual void		OnUpdateComponent( float fDelta );
	virtual void		OnRenderComponent( void );

	virtual void		OnEntityInitialised( void );

private:
	void	ApplySteering( float fDelta );

	VECT	mxForward;
	VECT	mxRight;
	VECT	mxUp;

	float	mfSpeed;

	float	mfRollSpeed;
	float	mfPitchSpeed;

};




#endif