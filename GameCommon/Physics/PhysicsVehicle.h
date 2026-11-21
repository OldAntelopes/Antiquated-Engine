#ifndef PHYSICS_VEHICLE_H
#define PHYSICS_VEHICLE_H

#include "PhysicsInternal.h"

class PhysicsVehicleRaycaster;


class PhysicsVehicleSetup
{
	friend class PhysicsVehicle;
	friend class PhysicsVehicleControllerComponent;
public:
	PhysicsVehicleSetup()
	{
		mfMass = 1800.0f;
		mfEnginePowerRating = 1.0f;
		mfSuspensionStiffness = 40.0f;
		mfSuspensionDampingFactor = 0.2f;
		mfSuspensionMaxTravel = 200.0f;
		mfFrictionSlip = 8.0f;
		mfRollInfluence = 0.3f;
		mfSuspensionRestLength = 0.4f;
		mfCentreOfMassOffsetZ = 0.0f;
		mfSteeringSpeed = 0.2f;
		// Added for The Universal's tuning. These defaults do what the code did before they existed.
		mfBrakeForce = 200.0f;
		mfDragCoefficient = 0.0f;
		mfRollingResistance = 0.0f;
		mfCentreOfMassHeight = 0.0f;
		mfSteeringMaxAngle = A30;
		mfSteeringReturnSpeed = 2.0f;
		mfSteeringFadeMPH = 0.0f;
		mfReverseMaxMPH = 0.0f;
		mfMaxSuspensionForce = 6000.0f;
		mfGroundNormalSmoothing = 0.0f;
		mfTopSpeedMPH = 0.0f;
		ClearWheelGeometry();
	}

	BOOL	IsDifferent( const PhysicsVehicleSetup* pOtherSetup)
	{
		if ( ( pOtherSetup->mfMass != mfMass ) ||
			( pOtherSetup->mfEnginePowerRating != mfEnginePowerRating ) ||
			( pOtherSetup->mfSuspensionStiffness != mfSuspensionStiffness ) ||
			( pOtherSetup->mfSuspensionDampingFactor != mfSuspensionDampingFactor ) ||
			( pOtherSetup->mfSuspensionMaxTravel != mfSuspensionMaxTravel ) ||
			( pOtherSetup->mfFrictionSlip != mfFrictionSlip ) ||
			( pOtherSetup->mfRollInfluence != mfRollInfluence ) ||
			( pOtherSetup->mfSteeringSpeed != mfSteeringSpeed ) ||
			( pOtherSetup->mfCentreOfMassOffsetZ != mfCentreOfMassOffsetZ ) ||
			( pOtherSetup->mfSuspensionRestLength != mfSuspensionRestLength ) ||
			( pOtherSetup->mfBrakeForce != mfBrakeForce ) ||
			( pOtherSetup->mfDragCoefficient != mfDragCoefficient ) ||
			( pOtherSetup->mfRollingResistance != mfRollingResistance ) ||
			( pOtherSetup->mfCentreOfMassHeight != mfCentreOfMassHeight ) ||
			( pOtherSetup->mfSteeringMaxAngle != mfSteeringMaxAngle ) ||
			( pOtherSetup->mfSteeringReturnSpeed != mfSteeringReturnSpeed ) ||
			( pOtherSetup->mfSteeringFadeMPH != mfSteeringFadeMPH ) ||
			( pOtherSetup->mfReverseMaxMPH != mfReverseMaxMPH ) ||
			( pOtherSetup->mfMaxSuspensionForce != mfMaxSuspensionForce ) ||
			( pOtherSetup->mfGroundNormalSmoothing != mfGroundNormalSmoothing ) ||
			( pOtherSetup->mfTopSpeedMPH != mfTopSpeedMPH ) ||
			( IsWheelGeometryDifferent( pOtherSetup ) ) )
		{
			return( TRUE );
		}
		return( FALSE );
	}

	BOOL	IsWheelGeometryDifferent( const PhysicsVehicleSetup* pOtherSetup ) const
	{
	int		nLoop;

		if ( pOtherSetup->mbWheelGeometrySet != mbWheelGeometrySet ) return( TRUE );
		if ( mbWheelGeometrySet == FALSE ) return( FALSE );
		for ( nLoop = 0; nLoop < 4; nLoop++ )
		{
			if ( ( pOtherSetup->maxWheelPos[nLoop].x != maxWheelPos[nLoop].x ) ||
				 ( pOtherSetup->maxWheelPos[nLoop].y != maxWheelPos[nLoop].y ) ||
				 ( pOtherSetup->maxWheelPos[nLoop].z != maxWheelPos[nLoop].z ) ||
				 ( pOtherSetup->mafWheelRadius[nLoop] != mafWheelRadius[nLoop] ) )
			{
				return( TRUE );
			}
		}
		return( FALSE );
	}

	void	SetMass( float fMass );
	void	SetEnginePower( float fPowerRating );
	void	SetSuspensionStiffness( float fSuspensionStiffness );
	void	SetSuspensionDampingFactor( float fSuspensionDamping );
	void	SetSuspensionMaxTravel( float mfSuspensionMaxTravel );
	void	SetFrictionSlip( float fFrictionSlip );
	void	SetRollInfluence( float mfRollInfluence );
	void	SetSuspensionRestLength( float fSuspensionRestLength );
	void	SetCentreOfMassOffset( float fCentreOfMassOffsetZ );
	void	SetSteeringSpeed( float fSteeringSpeed );

	// Brake impulse per wheel per 1/60s physics step (rear wheels). Default 200.
	void	SetBrakeForce( float fBrakeForce ) { mfBrakeForce = fBrakeForce; }
	// Air drag: force (N) = fDrag * speed (m/s) squared, against the velocity. Default 0 (none).
	void	SetDragCoefficient( float fDrag ) { mfDragCoefficient = fDrag; }
	// Rolling resistance as a deceleration in m/s/s while moving. Default 0 (none).
	void	SetRollingResistance( float fRollingResistance ) { mfRollingResistance = fRollingResistance; }
	// Real centre of mass height above the ground (m), at rest. Moves the chassis shape and wheels down relative to the
	// rigid body, so the body (its centre of mass) sits this high. Positions reported stay at ground level. Default 0.
	// (mfCentreOfMassOffsetZ only moves the start height: Bullet's motion state offset doesn't move the mass.)
	void	SetCentreOfMassHeight( float fHeight ) { mfCentreOfMassHeight = fHeight; }
	// Steering lock (radians), the speed the wheel returns to the middle (rad/s), and the speed (mph) at which the
	// lock has halved (0 = full lock at any speed). Defaults A30, 2.0, 0.
	void	SetSteeringLimits( float fMaxAngle, float fReturnSpeed, float fFadeMPH ) { mfSteeringMaxAngle = fMaxAngle; mfSteeringReturnSpeed = fReturnSpeed; mfSteeringFadeMPH = fFadeMPH; }

	// The engine stops pushing backwards above this speed in mph (0 = no limit, the default)
	void	SetReverseMaxSpeed( float fMPH ) { mfReverseMaxMPH = fMPH; }
	float	GetReverseMaxSpeed( void ) const { return( mfReverseMaxMPH ); }

	// The most force (N) one wheel's suspension can push with. Bullet's default, 6000, is nearly used up by a heavy
	// vehicle's own weight (1800 kg puts 4400 N on each wheel), so the outside springs give way in a corner and it rolls.
	void	SetMaxSuspensionForce( float fForce ) { mfMaxSuspensionForce = fForce; }
	float	GetMaxSuspensionForce( void ) const { return( mfMaxSuspensionForce ); }
	// Wheel rays that hit the landscape take its slope from heights this far (m) either side of the hit, instead of the
	// hit triangle's own normal, which jumps at every triangle edge and kicks the suspension. 0 = off (the default).
	void	SetGroundNormalSmoothing( float fDistance ) { mfGroundNormalSmoothing = fDistance; }
	float	GetGroundNormalSmoothing( void ) const { return( mfGroundNormalSmoothing ); }
	// Wheel centres in the vehicle's own space (x right, y forward, z up from the ground under it) and radii, e.g. from
	// the model, in the vehicle's wheel order (0, 1 front, 2, 3 rear). Unset = the built-in layout (the default).
	void	SetWheelGeometry( int nWheelNum, const VECT* pxCentre, float fRadius )
	{
		if ( ( nWheelNum >= 0 ) && ( nWheelNum < 4 ) )
		{
			maxWheelPos[nWheelNum] = *pxCentre;
			mafWheelRadius[nWheelNum] = fRadius;
			mbWheelGeometrySet = TRUE;
		}
	}
	void	ClearWheelGeometry( void )
	{
	int		nLoop;

		mbWheelGeometrySet = FALSE;
		for ( nLoop = 0; nLoop < 4; nLoop++ )
		{
			maxWheelPos[nLoop].x = maxWheelPos[nLoop].y = maxWheelPos[nLoop].z = 0.0f;
			mafWheelRadius[nLoop] = 0.0f;
		}
	}
	BOOL	HasWheelGeometry( void ) const { return( mbWheelGeometrySet ); }
	// Top speed on the flat, mph: the drag is worked out so full throttle in the top gear only just holds it, and the
	// drag coefficient is ignored. 0 = use the drag coefficient (the default)
	void	SetTopSpeed( float fMPH ) { mfTopSpeedMPH = fMPH; }
	float	GetTopSpeed( void ) const { return( mfTopSpeedMPH ); }

	float	GetMass( void ) const { return( mfMass ); }
	float	GetEnginePower( void ) const { return( mfEnginePowerRating ); }
	float	GetSuspensionStiffness( void ) const { return( mfSuspensionStiffness ); }
	float	GetSuspensionDampingFactor( void ) const { return( mfSuspensionDampingFactor ); }
	float	GetSuspensionMaxTravel( void ) const { return( mfSuspensionMaxTravel ); }
	float	GetFrictionSlip( void ) const { return( mfFrictionSlip ); }
	float	GetRollInfluence( void ) const { return( mfRollInfluence ); }
	float	GetSuspensionRestLength( void ) const { return( mfSuspensionRestLength ); }
	float	GetSteeringSpeed( void ) const { return( mfSteeringSpeed ); }
	float	GetBrakeForce( void ) const { return( mfBrakeForce ); }
	float	GetDragCoefficient( void ) const { return( mfDragCoefficient ); }
	float	GetRollingResistance( void ) const { return( mfRollingResistance ); }
	float	GetCentreOfMassHeight( void ) const { return( mfCentreOfMassHeight ); }
	float	GetSteeringMaxAngle( void ) const { return( mfSteeringMaxAngle ); }
	float	GetSteeringReturnSpeed( void ) const { return( mfSteeringReturnSpeed ); }
	float	GetSteeringFadeMPH( void ) const { return( mfSteeringFadeMPH ); }
protected:
	
	float	mfMass;
	float	mfEnginePowerRating;
	float	mfSuspensionStiffness;
	float	mfSuspensionDampingFactor;
	float	mfSuspensionMaxTravel;
	float	mfFrictionSlip;
	float	mfRollInfluence;
	float	mfSuspensionRestLength;
	float	mfCentreOfMassOffsetZ;
	float	mfSteeringSpeed;

	float	mfBrakeForce;
	float	mfDragCoefficient;
	float	mfRollingResistance;
	float	mfCentreOfMassHeight;
	float	mfSteeringMaxAngle;
	float	mfSteeringReturnSpeed;
	float	mfSteeringFadeMPH;
	float	mfReverseMaxMPH;
	float	mfMaxSuspensionForce;
	float	mfGroundNormalSmoothing;
	float	mfTopSpeedMPH;
	BOOL	mbWheelGeometrySet;
	VECT	maxWheelPos[4];
	float	mafWheelRadius[4];
};



// Optional, for a game whose drawn ground differs from its physics landscape (e.g. edge-smoothed): how far (m) the
// drawn ground is above the physics landscape at a point. When set, wheel rays that hit the landscape take the drawn
// ground's height (and slope, for SetGroundNormalSmoothing) instead. NULL (the default) = the landscape as it is.
typedef float (*PhysicsGroundHeightOffsetFunc)( float fX, float fY );
extern void		PhysicsVehicleSetGroundHeightOffsetFunc( PhysicsGroundHeightOffsetFunc pfnOffset );

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
	void		StopMoving( void );		// Zero linear and angular velocity (respawns)

	void		RenderDebug( void );
	void		RenderDebugLines( void );

	// For tuning logs: per wheel, whether it touches the ground, its suspension length (m) and whether it's sliding.
	int			GetNumWheelsOnGround( void );
	float		GetWheelSuspensionLength( int nWheelNum );
	BOOL		IsWheelSliding( int nWheelNum );
	float		GetEngineForce( void ) { return( mfVehicleEngineForce ); }
	float		GetBrakeForceApplied( void ) { return( mfVehicleBrakeForce ); }
	// The controls as last set: accelerator -1 (full reverse) to 1, brake 0 to 1
	float		GetAcceleratorAmount( void ) { return( mfVehicleAcceleratorAmount ); }
	float		GetBrakeAmount( void ) { return( mfVehicleBrakeAmount ); }

private:
	void		ApplyGeometry( void );
	void		ApplyDragForces( void );
	float		GetEngineForceAtMPH( float fSpeed );
	float		GetDragCoefficientInUse( void );

	void		CreateVehicle( const VECT* pxPos, eVehicleCollisionType collisionType, const PhysicsVehicleSetup* pSetup );
	void		UpdateEngine( float fDelta );
	void		SetAllWheelSettings( const PhysicsVehicleSetup* pSetup );

	btRigidBody*	localCreateRigidBody(btScalar mass, const btTransform& startTransform, btCollisionShape* shape, eVehicleCollisionType collisionType, float fCOMOffset);

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

	btCompoundShape*	m_pChassisCompoundShape;
	float				mfChassisShapeHeight;		// Height of the chassis box centre above the ground at rest
	float				mafWheelRadius[4];
	VECT				maxDefaultWheelPos[4];		// The built-in layout's wheel centres, for a setup that gives none
	float				mafDefaultWheelRadius[4];
};








#endif