#ifndef PHYSICS_VEHICLE_CONTROLLER_COMPONENT_H
#define	PHYSICS_VEHICLE_CONTROLLER_COMPONENT_H


#include "ControllerComponent.h"
#include "../../Physics/PhysicsVehicle.h"



class PhysicsVehicleControllerComponent : public ControllerComponent
{
public:
	PhysicsVehicleControllerComponent();
	virtual ~PhysicsVehicleControllerComponent();

	virtual const char*			GetComponentType( void ) { return( "PhysicsVehicleController" ); }

	virtual void		OnUpdateComponent( float fDelta );
	virtual void		OnEntityInitialised( void );

	void				InitialisePhysicsVehicle( const PhysicsVehicleSetup* pSetup );

	void				GetLinearVelocity( VECT* pxOut );

	PhysicsVehicle*		GetPhysicsVehicle( void ) { return( mpPhysicsVehicle ); }
	void				RespawnVehicle( const VECT* pxSpawnPos );
	virtual BOOL		IsLocalVehicle( void ) { return( TRUE ); }

	ENGINEMATRIX*		GetWheelTransform( int nWheelNum ) { return( &matWheelTransforms[nWheelNum] ); }
	void				GetEngineValues( int* pnGear, int* pnRPM );

	void				RenderDebug( void );
private:
	void	ApplySteering( float fDelta );

	float		mfSteeringAngle;
	float		mfWheelSpin;
	BOOL		mbIsReverse;
	BOOL		mbCanReverse;
	float		mfBrakeReverseDelayTimer;
	
	ENGINEMATRIX	matWheelTransforms[4];

	PhysicsVehicle*		mpPhysicsVehicle;
};




#endif