#ifndef VEHICLE_CONTROLLER_COMPONENT_H
#define	VEHICLE_CONTROLLER_COMPONENT_H


#include "ControllerComponent.h"


class VehicleControllerComponent : public ControllerComponent
{
public:
	VehicleControllerComponent();

	virtual const char*			GetComponentType( void ) { return( "VehicleController" ); }

	virtual void		OnUpdateComponent( float fDelta );

private:
	VECT	GetSteeringVector( void );
	VECT	GetForwardVector( void );
	float	GetSkidValue( const VECT* pxWheelVector, const VECT* pxVel );

	void	ApplySteering( float fDelta );
	void	ApplyControls( float fDelta );
	void	UpdatePhysics( float fDelta );
	void	UpdatePositionAndOrientation( float fDelta );
	float	ApplyFriction( float fDelta, float fFrictionValue );
	void	AddGravity( float fDelta );

	VECT		mxVel;
	float		mfSteeringAngle;
	float		mfFrameAcceleration;
	BOOL		mbOnGround;
};




#endif