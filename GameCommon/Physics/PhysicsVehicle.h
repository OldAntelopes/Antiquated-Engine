#ifndef PHYSICS_VEHICLE_H
#define PHYSICS_VEHICLE_H

#include "PhysicsInternal.h"

class PhysicsVehicleRaycaster;


class PhysicsVehicleSetup
{
	friend class PhysicsVehicle;
public:
	PhysicsVehicleSetup()
	{
		mfMass = 1800.0f;
		mfEnginePowerRating = 1.0f;
		mfSuspensionStiffness = 40.0f;
		mfWheelsDampingRelaxation = 2.3f;
		mfWheelsDampingCompression = 1.6f;
		mfFrictionSlip = 8.0f;
		mfRollInfluence = 0.3f;
		mfSuspensionRestLength = 0.4f;
	}

	BOOL	IsDifferent( const PhysicsVehicleSetup* pOtherSetup)
	{
		if ( ( pOtherSetup->mfMass != mfMass ) ||
			( pOtherSetup->mfEnginePowerRating != mfEnginePowerRating ) ||
			( pOtherSetup->mfSuspensionStiffness != mfSuspensionStiffness ) ||
			( pOtherSetup->mfWheelsDampingRelaxation != mfWheelsDampingRelaxation ) ||
			( pOtherSetup->mfWheelsDampingCompression != mfWheelsDampingCompression ) ||
			( pOtherSetup->mfFrictionSlip != mfFrictionSlip ) ||
			( pOtherSetup->mfRollInfluence != mfRollInfluence ) ||
			( pOtherSetup->mfSuspensionRestLength != mfSuspensionRestLength ))
		{
			return( TRUE );
		}
		return( FALSE );	
	}
	
	void	SetMass( float fMass );
	void	SetEnginePower( float fPowerRating );
	void	SetSuspensionStiffness( float fSuspensionStiffness );
	void	SetDampingRelaxation( float fWheelsDampingRelaxation );
	void	SetDampingCompression( float fWheelsDampingCompression );
	void	SetFrictionSlip( float fFrictionSlip );
	void	SetRollInfluence( float mfRollInfluence );
	void	SetSuspensionRestLength( float fSuspensionRestLength );
protected:
	
	float	mfMass;
	float	mfEnginePowerRating;
	float	mfSuspensionStiffness;
	float	mfWheelsDampingRelaxation;
	float	mfWheelsDampingCompression;
	float	mfFrictionSlip;
	float	mfRollInfluence;
	float	mfSuspensionRestLength;

};



enum eVehicleCollisionType
{
	VEHICLECOLLISIONTYPE_LOCALVEHICLE,
	VEHICLECOLLISIONTYPE_REMOTEVEHICLE,
	VEHICLECOLLISIONTYPE_REMOTEPREDICTIONVEHICLE,
};

class PhysicsVehicle
{
public:
	PhysicsVehicle();
	~PhysicsVehicle();

	void		Initialise( const VECT* pxPos, eVehicleCollisionType collisionType, const PhysicsVehicleSetup* pSetup );
	void		ModifySettings( const PhysicsVehicleSetup* pSetup );

	PhysicsVehicleSetup* GetCurrentSetup( void );
	void		ApplyCurrentSetup( void );

	void		Update( float fDelta );

	void		SetControls( float fSteeringAngle, float fEngineAmount, float fBrakeAmount );

	void		GetPosition( VECT* pxOut );
	void		GetWheelPosition( int nWheelNum, ENGINEMATRIX* pxOut );

	void		GetOrientation( ENGINEQUATERNION* pOut );	
	void		GetLinearVelocity( VECT* pxOut );

	float		GetCurrentSpeedMPH( void );
	int			GetCurrentGear( void ) { return( mnCurrentGear ); }
	int			GetCurrentRPM( void ) { return( mnCurrentRPM ); }

	void		OverridePositionAndOrientation( const VECT* pxPos, const ENGINEQUATERNION* pQuat );
	void		OverrideVelocity( const VECT* pxVel );
	void		OverridePosition( const VECT* pxPos );
	void		OverrideOrientation( const ENGINEQUATERNION* pQuat );

	void		RenderDebug( void );

private:

	void		CreateVehicle( const VECT* pxPos, eVehicleCollisionType collisionType, const PhysicsVehicleSetup* pSetup );
	void		UpdateEngine( float fDelta );
	void		SetAllWheelSettings( const PhysicsVehicleSetup* pSetup );

	btRigidBody*	localCreateRigidBody(btScalar mass, const btTransform& startTransform, btCollisionShape* shape, eVehicleCollisionType collisionType);

	btRaycastVehicle*				m_pRaycastVehicle;
//	btDefaultVehicleRaycaster*		m_pVehicleRaycaster;
	PhysicsVehicleRaycaster*		m_pVehicleRaycaster;

	btAlignedObjectArray<btCollisionShape*> m_apCollisionShapes;
	
	btRigidBody*		m_pCarChassisRigidBody;
	btCollisionShape*	m_pFrontWheelShape;
	btCollisionShape*	m_pRearWheelShape;

	float				mfVehicleSteering;
	float				mfVehicleBrakeForce;
	float				mfVehicleEngineForce;

	float				mfVehicleAcceleratorAmount;
	float				mfVehicleBrakeAmount;

	int					mnCurrentGear;
	int					mnCurrentRPM;

	PhysicsVehicleSetup		mPhysicsVehicleSetup;
};








#endif