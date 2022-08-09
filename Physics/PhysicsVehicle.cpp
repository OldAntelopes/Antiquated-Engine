
#include "StandardDef.h"
#include "Engine.h"
#include "Interface.h"

#include "PhysicsInternal.h"
#include "PhysicsWorld.h"
#include "PhysicsVehicle.h"

btRaycastVehicle::btVehicleTuning	m_tuning;

btVector3 wheelDirectionCS0(0,0,-1);
btVector3 wheelAxleCS(1,0,0);


void	PhysicsVehicleSetup::SetMass( float fMass )
{
	mfMass = fMass;
}

void	PhysicsVehicleSetup::SetEnginePower( float fPowerRating )
{
	mfEnginePowerRating = fPowerRating;
}

void	PhysicsVehicleSetup::SetSuspensionStiffness( float fSuspensionStiffness )
{
	mfSuspensionStiffness = fSuspensionStiffness;
}

void	PhysicsVehicleSetup::SetDampingRelaxation( float fWheelsDampingRelaxation )
{
	mfWheelsDampingRelaxation = fWheelsDampingRelaxation;
}

void	PhysicsVehicleSetup::SetDampingCompression( float fWheelsDampingCompression )
{
	mfWheelsDampingCompression = fWheelsDampingCompression;
}

void	PhysicsVehicleSetup::SetFrictionSlip( float fFrictionSlip )
{
	mfFrictionSlip = fFrictionSlip;
}

void	PhysicsVehicleSetup::SetRollInfluence( float fRollInfluence )
{
	mfRollInfluence = fRollInfluence;
}

void	PhysicsVehicleSetup::SetSuspensionRestLength( float fSuspensionRestLength )
{
	mfSuspensionRestLength = fSuspensionRestLength;
}

//----------------------------------------------------------------------------

class PhysicsVehicleRaycaster : public btVehicleRaycaster
{
public:
	PhysicsVehicleRaycaster(btDynamicsWorld* world, eVehicleCollisionType collisionType)
	{
		mpDynamicsWorld = world;
		switch( collisionType )
		{
		case VEHICLECOLLISIONTYPE_LOCALVEHICLE:
			mUseFilter = true;
			mCollisionFilterMask = COLLISION_FILTER_LANDSCAPE | COLLISION_FILTER_STATIC_OBJECT | COLLISION_FILTER_REMOTE_VEHICLE | COLLISION_FILTER_REMOTE_PREDICTION_VEHICLE;
			mCollisionFilterGroup = COLLISION_FILTER_LOCAL_VEHICLE;
			break;
		case VEHICLECOLLISIONTYPE_REMOTEVEHICLE:
			mUseFilter = true;
			mCollisionFilterMask = COLLISION_FILTER_LANDSCAPE | COLLISION_FILTER_STATIC_OBJECT | COLLISION_FILTER_LOCAL_VEHICLE;
			mCollisionFilterGroup = COLLISION_FILTER_REMOTE_VEHICLE;
			break;
		case VEHICLECOLLISIONTYPE_REMOTEPREDICTIONVEHICLE:
			mUseFilter = true;
			mCollisionFilterMask = COLLISION_FILTER_LANDSCAPE | COLLISION_FILTER_STATIC_OBJECT | COLLISION_FILTER_LOCAL_VEHICLE;
			mCollisionFilterGroup = COLLISION_FILTER_REMOTE_PREDICTION_VEHICLE;
			break;
		}
	}

	virtual void* castRay(const btVector3& from,const btVector3& to, btVehicleRaycasterResult& result);

	btDynamicsWorld*	mpDynamicsWorld;
	bool	mUseFilter;
	ushort	mCollisionFilterMask;
	ushort	mCollisionFilterGroup;

};


void* PhysicsVehicleRaycaster::castRay(const btVector3& from,const btVector3& to, btVehicleRaycasterResult& result)
{
	btCollisionWorld::ClosestRayResultCallback rayCallback(from,to);

	if(mUseFilter)
	{
		rayCallback.m_collisionFilterMask = mCollisionFilterMask;
		rayCallback.m_collisionFilterGroup = mCollisionFilterGroup;
	}

	mpDynamicsWorld->rayTest(from, to, rayCallback);

	if (rayCallback.hasHit())
	{
	btRigidBody* body = btRigidBody::upcast((btCollisionObject*)rayCallback.m_collisionObject);
	
		if (body && body->hasContactResponse())
		{
			result.m_hitPointInWorld = rayCallback.m_hitPointWorld;
			result.m_hitNormalInWorld = rayCallback.m_hitNormalWorld;
			result.m_hitNormalInWorld.normalize();
			result.m_distFraction = rayCallback.m_closestHitFraction;
			return body;
		}
	}
	return 0;
 }

//-------------------------------------------------------------

PhysicsVehicle::PhysicsVehicle()
{
	m_pRaycastVehicle = NULL;
	mfVehicleSteering = 0.0f;
	mfVehicleBrakeForce = 0.0f;
	mfVehicleEngineForce = 0.0f;
	mfVehicleAcceleratorAmount = 0.0f;
	mfVehicleBrakeAmount = 0.0f;

	mnCurrentGear = 1;
}

PhysicsVehicle::~PhysicsVehicle()
{
	if ( m_pCarChassisRigidBody )
	{
		if ( mpDynamicsWorld )
		{
			mpDynamicsWorld->removeRigidBody( m_pCarChassisRigidBody );
		}
		delete m_pCarChassisRigidBody;
		m_pCarChassisRigidBody = NULL;
	}

	if ( m_pRaycastVehicle )
	{
		if ( mpDynamicsWorld )
		{
			mpDynamicsWorld->removeVehicle(m_pRaycastVehicle);
		}
		delete m_pRaycastVehicle;
		m_pRaycastVehicle = NULL;
	}
}

btRigidBody* PhysicsVehicle::localCreateRigidBody(btScalar mass, const btTransform& startTransform, btCollisionShape* shape, eVehicleCollisionType collisionType)
{
	btAssert((!shape || shape->getShapeType() != INVALID_SHAPE_PROXYTYPE));

	//rigidbody is dynamic if and only if mass is non zero, otherwise static
	bool isDynamic = (mass != 0.f);

	btVector3 localInertia(0,0,0);
	if (isDynamic)
		shape->calculateLocalInertia(mass,localInertia);

	//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects

	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);

	btRigidBody::btRigidBodyConstructionInfo cInfo(mass,myMotionState,shape,localInertia);

	btRigidBody* body = new btRigidBody(cInfo);
	//body->setContactProcessingThreshold(m_defaultContactProcessingThreshold);

	ushort		uwCollidesWith = COLLISION_FILTER_LANDSCAPE | COLLISION_FILTER_STATIC_OBJECT | COLLISION_FILTER_PHYSICS_OBJECTS;

	switch ( collisionType )
	{
	case VEHICLECOLLISIONTYPE_LOCALVEHICLE:
		uwCollidesWith |= COLLISION_FILTER_REMOTE_VEHICLE | COLLISION_FILTER_REMOTE_PREDICTION_VEHICLE;
		mpDynamicsWorld->addRigidBody(body, COLLISION_FILTER_LOCAL_VEHICLE, uwCollidesWith);
		break;
	case VEHICLECOLLISIONTYPE_REMOTEVEHICLE:
		uwCollidesWith |= COLLISION_FILTER_LOCAL_VEHICLE;
		mpDynamicsWorld->addRigidBody(body, COLLISION_FILTER_REMOTE_VEHICLE, uwCollidesWith);
		break;
	case VEHICLECOLLISIONTYPE_REMOTEPREDICTIONVEHICLE:
		uwCollidesWith |= COLLISION_FILTER_LOCAL_VEHICLE;
		mpDynamicsWorld->addRigidBody(body, COLLISION_FILTER_REMOTE_PREDICTION_VEHICLE, uwCollidesWith);
		break;
	}

	return body;
}


void		PhysicsVehicle::RenderDebug( void )
{
int		nX = 100;
int		nY = 100;
char	acString[256];
int		nLoop;

	for( nLoop = 0; nLoop < 4; nLoop++ )
	{
		sprintf( acString, "Wheel %d", nLoop+1 );
		InterfaceText( 1, nX, nY, acString, 0xd0d0d0d0, 0 );
		nY += 20;
		sprintf( acString, "Susp Rel Vel: %f", m_pRaycastVehicle->getWheelInfo(0).m_suspensionRelativeVelocity );
		InterfaceText( 1, nX, nY, acString, 0xd0d0d0d0, 0 );
		nY += 20;
	}
}


void		PhysicsVehicle::CreateVehicle( const VECT* pxPos, eVehicleCollisionType collisionType, const PhysicsVehicleSetup* pSetup )
{
float		fSuspensionRestLength = 0.0f;
btTransform tr;
PhysicsVehicleSetup		xDefaultSetup;
float		wheelFriction = 1000;//BT_LARGE_FLOAT;
//float		fChassisHeightOffGround = 0.2f;
float		wheelWidth = 0.2f;
float		fChassisHalfWidth = 1.0f;
float		fChassisHalfLength = 2.2f;
float		fChassisHalfHeight = 1.2f;
float		fFrontWheelRadius = 0.55f;
float		fRearWheelRadius = 0.85f;
float		fChassisCOMAdjustHeight = fRearWheelRadius + (fChassisHalfHeight - 0.4f);
float		fWheelConnectionHeight = fFrontWheelRadius;// - fSuspensionRestLength;// + 0.2f;

	// if wheelconnectionheight - (fWheelRadius+suspensionrestlength) > fComadjustHeight - fChassisHalfHeight
	// vehicle will be stuck on the ground

	if ( pSetup == NULL )
	{
		pSetup = &xDefaultSetup;
	}
	fSuspensionRestLength = pSetup->mfSuspensionRestLength;

	mPhysicsVehicleSetup = *pSetup;

	tr.setIdentity();

	btCollisionShape* chassisShape = new btBoxShape(btVector3(fChassisHalfWidth,fChassisHalfLength,fChassisHalfHeight));
	m_apCollisionShapes.push_back(chassisShape);

	btCompoundShape* compound = new btCompoundShape();
	m_apCollisionShapes.push_back(compound);

	btTransform localTrans;
	localTrans.setIdentity();
	//localTrans effectively shifts the center of mass with respect to the chassis
	localTrans.setOrigin(btVector3(0,0,fChassisCOMAdjustHeight));

	compound->addChildShape(localTrans,chassisShape);

/*
	{
		btCollisionShape* suppShape = new btBoxShape(btVector3(0.5f,0.1f,0.5f));
		btTransform suppLocalTrans;
		suppLocalTrans.setIdentity();
		//localTrans effectively shifts the center of mass with respect to the chassis
		suppLocalTrans.setOrigin(btVector3(0,1.0,2.5));
		compound->addChildShape(suppLocalTrans, suppShape);
	}
*/

	tr.setOrigin(btVector3(pxPos->x, pxPos->y, pxPos->z + 5.5f ));

	m_pCarChassisRigidBody = localCreateRigidBody(pSetup->mfMass,tr,compound, collisionType);//chassisShape);
	//m_pCarChassisRigidBody->setDamping(0.2,0.2);
	m_pFrontWheelShape = new btCylinderShapeX(btVector3(wheelWidth,fFrontWheelRadius,fFrontWheelRadius));
	m_pRearWheelShape = new btCylinderShapeX(btVector3(wheelWidth,fRearWheelRadius,fRearWheelRadius));

//	m_guiHelper->createCollisionShapeGraphicsObject(m_pWheelShape);

//	int wheelGraphicsIndex = m_pWheelShape->getUserIndex();

	const float position[4]={0,10,10,0};
	const float quaternion[4]={0,0,0,1};
	const float color[4]={0,1,0,1};
	const float scaling[4] = {1,1,1,1};

	for (int i=0;i<4;i++)
	{
//		m_wheelInstances[i] = m_guiHelper->registerGraphicsInstance(wheelGraphicsIndex, position, quaternion, color, scaling);
	}

	
	/// create vehicle
	{
		
		m_pVehicleRaycaster = new PhysicsVehicleRaycaster(mpDynamicsWorld, collisionType);
		m_pRaycastVehicle = new btRaycastVehicle(m_tuning, m_pCarChassisRigidBody, (btVehicleRaycaster*)m_pVehicleRaycaster);
		
		///never deactivate the vehicle
		m_pCarChassisRigidBody->setActivationState(DISABLE_DEACTIVATION);

		mpDynamicsWorld->addVehicle(m_pRaycastVehicle);

//		float connectionHeight = 0.28f;
		bool isFrontWheel=true;

		//choose coordinate system
		m_pRaycastVehicle->setCoordinateSystem( 0, 2, 1 );

		// Front Right ( +X, -Y )
		btVector3 connectionPointCS0(fChassisHalfWidth-(0.5f*wheelWidth),fChassisHalfLength-(fFrontWheelRadius*1.8f), fFrontWheelRadius + fSuspensionRestLength);
		m_pRaycastVehicle->addWheel(connectionPointCS0,wheelDirectionCS0,wheelAxleCS,fSuspensionRestLength,fFrontWheelRadius,m_tuning,isFrontWheel);

		// Front Left ( -X, -Y )
		connectionPointCS0 = btVector3((0.0f - fChassisHalfWidth)+(0.5f*wheelWidth),fChassisHalfLength-(fFrontWheelRadius*1.8f),fFrontWheelRadius + fSuspensionRestLength);
		m_pRaycastVehicle->addWheel(connectionPointCS0,wheelDirectionCS0,wheelAxleCS,fSuspensionRestLength,fFrontWheelRadius,m_tuning,isFrontWheel);

		isFrontWheel = false;
		// Rear Left ( -X, +Y )
		connectionPointCS0 = btVector3((0.0f - fChassisHalfWidth)+(0.3f*wheelWidth),(0.0f - fChassisHalfLength)+fRearWheelRadius,fRearWheelRadius + fSuspensionRestLength);
		m_pRaycastVehicle->addWheel(connectionPointCS0,wheelDirectionCS0,wheelAxleCS,fSuspensionRestLength,fRearWheelRadius,m_tuning,isFrontWheel);

		// Rear Right ( +X, +Y )
		connectionPointCS0 = btVector3(fChassisHalfWidth-(0.3f*wheelWidth),(0.0f-fChassisHalfLength)+fRearWheelRadius,fRearWheelRadius + fSuspensionRestLength);
		m_pRaycastVehicle->addWheel(connectionPointCS0,wheelDirectionCS0,wheelAxleCS,fSuspensionRestLength,fRearWheelRadius,m_tuning,isFrontWheel);
	
		for (int i=0;i<m_pRaycastVehicle->getNumWheels();i++)
		{
			btWheelInfo& wheel = m_pRaycastVehicle->getWheelInfo(i);
			// From https://docs.google.com/document/d/18edpOwtGgCwNyvakS78jxMajCuezotCU_0iezcwiFQc/edit ...
			// The stiffness constant for the suspension.  10.0 - Offroad buggy, 50.0 - Sports car, 200.0 - F1 Car
			wheel.m_suspensionStiffness = mPhysicsVehicleSetup.mfSuspensionStiffness;

			wheel.m_maxSuspensionTravelCm = 500.0f;//fSuspensionRestLength * 0.95f * 100.0f;//500.0f;//fChassisCOMAdjustHeight * 0.9f * 100.0f;
			/// The damping coefficient for when the suspension is expanding.  See the comments for m_wheelsDampingCompression for how to set k.
              /// m_wheelsDampingRelaxation should be slightly larger than m_wheelsDampingCompression, eg k = 0.2 to 0.5
			wheel.m_wheelsDampingRelaxation = mPhysicsVehicleSetup.mfWheelsDampingRelaxation;
			  /// The damping coefficient for when the suspension is compressed. Set to k * 2.0 * btSqrt(m_suspensionStiffness) so k is proportional to critical damping.
              /// k = 0.0 undamped & bouncy, k = 1.0 critical damping
              /// k = 0.1 to 0.3 are good values
			wheel.m_wheelsDampingCompression = mPhysicsVehicleSetup.mfWheelsDampingCompression;
			  
			/// The coefficient of friction between the tyre and the ground.
              /// Should be about 0.8 for realistic cars, but can increased for better handling.
              /// Set large (10000.0) for kart racers
			wheel.m_frictionSlip = mPhysicsVehicleSetup.mfFrictionSlip;
			wheel.m_rollInfluence = mPhysicsVehicleSetup.mfRollInfluence;
		}
	}



}

void		PhysicsVehicle::Initialise( const VECT* pxPos, eVehicleCollisionType collisionType, const PhysicsVehicleSetup* pSetup )
{
	CreateVehicle( pxPos, collisionType, pSetup );

}

void	PhysicsVehicle::GetLinearVelocity( VECT* pxOut )
{
btVector3		xVel;

	xVel = m_pRaycastVehicle->getRigidBody()->getLinearVelocity();
	*pxOut = *( (VECT*)( &xVel ) );
}

void	PhysicsVehicle::GetPosition( VECT* pxOut )
{
btTransform trans;

	m_pRaycastVehicle->getRigidBody()->getMotionState()->getWorldTransform( trans );
	*pxOut = *( (VECT*)&trans.getOrigin() );
}

void	PhysicsVehicle::GetWheelPosition( int nWheelNum, ENGINEMATRIX* pxOut )
{
btTransform rTrans;
btMatrix3x3 rRotMat;

	memset( pxOut, 0, sizeof( ENGINEMATRIX ) );
	m_pRaycastVehicle->updateWheelTransform( nWheelNum );
	rTrans = m_pRaycastVehicle->getWheelTransformWS( nWheelNum );
	rRotMat = rTrans.getBasis();
	pxOut->_11 = rRotMat[0][0];
	pxOut->_12 = rRotMat[1][0];
	pxOut->_13 = rRotMat[2][0];
	pxOut->_21 = rRotMat[0][1];
	pxOut->_22 = rRotMat[1][1];
	pxOut->_23 = rRotMat[2][1];
	pxOut->_31 = rRotMat[0][2];
	pxOut->_32 = rRotMat[1][2];
	pxOut->_33 = rRotMat[2][2];

	pxOut->_41 = rTrans.getOrigin().getX();
	pxOut->_42 = rTrans.getOrigin().getY();
	pxOut->_43 = rTrans.getOrigin().getZ();
	pxOut->_44 = 1.0f;
}

float	PhysicsVehicle::GetCurrentSpeedMPH( void )
{
	return( m_pRaycastVehicle->getCurrentSpeedKmHour() * 0.621f );
}


void	PhysicsVehicle::GetOrientation( ENGINEQUATERNION* pOut )
{
btTransform trans;
btQuaternion	xQuat;

	m_pRaycastVehicle->getRigidBody()->getMotionState()->getWorldTransform( trans );
	xQuat = trans.getRotation();
	*pOut = *( (ENGINEQUATERNION*)&xQuat );
}


void		PhysicsVehicle::OverridePosition( const VECT* pxPos )
{
btTransform trans;
btQuaternion	xQuat;
btVector3		xPos;

	m_pRaycastVehicle->getRigidBody()->getMotionState()->getWorldTransform( trans );
	xPos.setValue( pxPos->x, pxPos->y, pxPos->z );
	trans.setOrigin( xPos );
	m_pRaycastVehicle->getRigidBody()->proceedToTransform( trans );
}

void		PhysicsVehicle::OverrideOrientation( const ENGINEQUATERNION* pQuat )
{
btTransform trans;
btQuaternion	xQuat;

	m_pRaycastVehicle->getRigidBody()->getMotionState()->getWorldTransform( trans );

	xQuat.setValue( pQuat->x, pQuat->y, pQuat->z, pQuat->w );
	trans.setRotation( xQuat );
	if ( ( !SysIsFinite( pQuat->x ) ) ||
		 ( !SysIsFinite( pQuat->y ) ) ||
		 ( !SysIsFinite( pQuat->z ) ) ||
		 ( !SysIsFinite( pQuat->w ) ) )
	{
		SysPanicIf( TRUE, "PhysicsVehicle Non-finite orientation" );
	}

	m_pRaycastVehicle->getRigidBody()->proceedToTransform( trans );
}

void	PhysicsVehicle::OverridePositionAndOrientation( const VECT* pxPos, const ENGINEQUATERNION* pQuat )
{
btTransform trans;
btQuaternion	xQuat;
btVector3		xPos;

	m_pRaycastVehicle->getRigidBody()->getMotionState()->getWorldTransform( trans );

	xQuat.setValue( pQuat->x, pQuat->y, pQuat->z, pQuat->w );
	xPos.setValue( pxPos->x, pxPos->y, pxPos->z );

	trans.setRotation( xQuat );
	trans.setOrigin( xPos );

	if ( ( !SysIsFinite( pQuat->x ) ) ||
		 ( !SysIsFinite( pQuat->y ) ) ||
		 ( !SysIsFinite( pQuat->z ) ) ||
		 ( !SysIsFinite( pQuat->w ) ) )
	{
		SysPanicIf( TRUE, "PhysicsVehicle Non-finite orientation" );
	}

	m_pRaycastVehicle->getRigidBody()->proceedToTransform( trans );
}


void	PhysicsVehicle::OverrideVelocity( const VECT* pxVel )
{
btVector3		xVel;

	xVel = *( (btVector3*)( pxVel ) );
	m_pRaycastVehicle->getRigidBody()->setLinearVelocity( xVel );
}


void		PhysicsVehicle::SetControls( float fSteeringAngle, float fEngineAmount, float fBrakeAmount )
{
	mfVehicleSteering = fSteeringAngle;

	mfVehicleAcceleratorAmount = fEngineAmount;
	mfVehicleBrakeAmount = fBrakeAmount;
}

typedef struct
{
	float	fBaseForce;
	float	fPeakExtraForce;
	float	fMinSpeed;
	float	fMaxSpeed;

} GEAR_SETTINGS;

GEAR_SETTINGS		maGearSettings[] = 
{
	{ 3000.0f, 2000.0f, 0.0f, 9.0f },
	{ 2000.0f, 1000.0f, 9.0f, 20.0f },
	{ 1500.0f, 700.0f,  20.0f, 35.0f },
	{ 1100.0f, 400.0f,  35.0f, 65.0f },
	{ 900.0f, 250.0f,  70.0f, 100.0f },

	{ 0.0f, 0.0f, -1.0f, -1.0f },
};

void		PhysicsVehicle::UpdateEngine( float fDelta )
{
float	fSpeed = GetCurrentSpeedMPH();
float	fMaxEngineForce;
float	fMaxBreakingForce = 200.f;
float	fEngineForceMod = 1.75f * mPhysicsVehicleSetup.mfEnginePowerRating;
int		nChosenGear = 1;
float	fGearRange;
float	fRangeAngle;
GEAR_SETTINGS*		pxGearSettings;

	pxGearSettings = maGearSettings;
	while( fSpeed > pxGearSettings->fMaxSpeed )
	{
		nChosenGear++;
		pxGearSettings++;
		if ( pxGearSettings->fMaxSpeed == -1.0f )
		{
			nChosenGear--;
			pxGearSettings--;
			break;
		}
	}

	fGearRange = pxGearSettings->fMaxSpeed - pxGearSettings->fMinSpeed;
	fGearRange = (fSpeed - pxGearSettings->fMinSpeed) / fGearRange;

	// TEMP - Set chosen gear immediately..
	mnCurrentGear = nChosenGear;
	mnCurrentRPM = (int)( 1000.0f + (fGearRange * 2500.0f) );

	fRangeAngle = sinf( A45 + (fGearRange * A90) );
	fMaxEngineForce = pxGearSettings->fBaseForce + ( pxGearSettings->fPeakExtraForce * fRangeAngle );
	
	mfVehicleBrakeForce = mfVehicleBrakeAmount * fMaxBreakingForce;
	mfVehicleEngineForce = mfVehicleAcceleratorAmount * fMaxEngineForce * fEngineForceMod;
}

void		PhysicsVehicle::Update( float fDelta )
{
int wheelIndex = 2;

	UpdateEngine( fDelta );

	m_pRaycastVehicle->applyEngineForce(mfVehicleEngineForce,wheelIndex);
	m_pRaycastVehicle->setBrake(mfVehicleBrakeForce,wheelIndex);
	wheelIndex = 3;
	m_pRaycastVehicle->applyEngineForce(mfVehicleEngineForce,wheelIndex);
	m_pRaycastVehicle->setBrake(mfVehicleBrakeForce,wheelIndex);
	
	wheelIndex = 0;
	m_pRaycastVehicle->setSteeringValue(mfVehicleSteering,wheelIndex);
	wheelIndex = 1;
	m_pRaycastVehicle->setSteeringValue(mfVehicleSteering,wheelIndex);

#ifdef PHYSICS_VEHICLE_ONSCREENDEBUG
	char		acString[256];

	sprintf( acString, "Veh: Steer: %.3f   Acc: %.2f   Brake: %.2f  Delta: %.5f", mfVehicleSteering, mfVehicleEngineForce, mfVehicleBrakeForce, fDelta );
	InterfaceText( 1, 100, 10, acString, 0xd0d0d0d0, 0 );
#endif // ifdef PHYSICS_VEHICLE_ONSCREENDEBUG

}



/*

	btCollisionShape* chassisShape = new btBoxShape(btVector3(1.f,0.5f,2.f));
	m_apCollisionShapes.push_back(chassisShape);

	btCompoundShape* compound = new btCompoundShape();
	m_apCollisionShapes.push_back(compound);
	btTransform localTrans;
	localTrans.setIdentity();
	//localTrans effectively shifts the center of mass with respect to the chassis
	localTrans.setOrigin(btVector3(0,1,0));

	compound->addChildShape(localTrans,chassisShape);

	{
		btCollisionShape* suppShape = new btBoxShape(btVector3(0.5f,0.1f,0.5f));
		btTransform suppLocalTrans;
		suppLocalTrans.setIdentity();
		//localTrans effectively shifts the center of mass with respect to the chassis
		suppLocalTrans.setOrigin(btVector3(0,1.0,2.5));
		compound->addChildShape(suppLocalTrans, suppShape);
	}

	tr.setOrigin(btVector3(0,0.f,0));

	m_pCarChassisRigidBody = localCreateRigidBody(800,tr,compound);//chassisShape);
	//m_pCarChassisRigidBody->setDamping(0.2,0.2);
	
	m_pWheelShape = new btCylinderShapeX(btVector3(wheelWidth,wheelRadius,wheelRadius));

	m_guiHelper->createCollisionShapeGraphicsObject(m_pWheelShape);
	int wheelGraphicsIndex = m_pWheelShape->getUserIndex();

	const float position[4]={0,10,10,0};
	const float quaternion[4]={0,0,0,1};
	const float color[4]={0,1,0,1};
	const float scaling[4] = {1,1,1,1};

	for (int i=0;i<4;i++)
	{
		m_wheelInstances[i] = m_guiHelper->registerGraphicsInstance(wheelGraphicsIndex, position, quaternion, color, scaling);
	}

	
	/// create vehicle
	{
		
		m_pVehicleRaycaster = new btDefaultVehicleRaycaster(mpDynamicsWorld);
		m_pRaycastVehicle = new btRaycastVehicle(m_tuning,m_pCarChassisRigidBody,m_pVehicleRaycaster);
		
		///never deactivate the vehicle
		m_pCarChassisRigidBody->setActivationState(DISABLE_DEACTIVATION);

		mpDynamicsWorld->addVehicle(m_pRaycastVehicle);

		float connectionHeight = 1.2f;

	
		bool isFrontWheel=true;

		//choose coordinate system
		m_pRaycastVehicle->setCoordinateSystem(rightIndex,upIndex,forwardIndex);

		btVector3 connectionPointCS0(CUBE_HALF_EXTENTS-(0.3*wheelWidth),connectionHeight,2*CUBE_HALF_EXTENTS-wheelRadius);

		m_pRaycastVehicle->addWheel(connectionPointCS0,wheelDirectionCS0,wheelAxleCS,suspensionRestLength,wheelRadius,m_tuning,isFrontWheel);
		connectionPointCS0 = btVector3(-CUBE_HALF_EXTENTS+(0.3*wheelWidth),connectionHeight,2*CUBE_HALF_EXTENTS-wheelRadius);

		m_pRaycastVehicle->addWheel(connectionPointCS0,wheelDirectionCS0,wheelAxleCS,suspensionRestLength,wheelRadius,m_tuning,isFrontWheel);
		connectionPointCS0 = btVector3(-CUBE_HALF_EXTENTS+(0.3*wheelWidth),connectionHeight,-2*CUBE_HALF_EXTENTS+wheelRadius);
		isFrontWheel = false;
		m_pRaycastVehicle->addWheel(connectionPointCS0,wheelDirectionCS0,wheelAxleCS,suspensionRestLength,wheelRadius,m_tuning,isFrontWheel);
		connectionPointCS0 = btVector3(CUBE_HALF_EXTENTS-(0.3*wheelWidth),connectionHeight,-2*CUBE_HALF_EXTENTS+wheelRadius);
		m_pRaycastVehicle->addWheel(connectionPointCS0,wheelDirectionCS0,wheelAxleCS,suspensionRestLength,wheelRadius,m_tuning,isFrontWheel);
		
		for (int i=0;i<m_pRaycastVehicle->getNumWheels();i++)
		{
			btWheelInfo& wheel = m_pRaycastVehicle->getWheelInfo(i);
			wheel.m_suspensionStiffness = suspensionStiffness;
			wheel.m_wheelsDampingRelaxation = suspensionDamping;
			wheel.m_wheelsDampingCompression = suspensionCompression;
			wheel.m_frictionSlip = wheelFriction;
			wheel.m_rollInfluence = rollInfluence;
		}
	}

*/