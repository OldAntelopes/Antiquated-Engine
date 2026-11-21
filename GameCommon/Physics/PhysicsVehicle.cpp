
#include "StandardDef.h"
#include "Engine.h"
#include "Interface.h"

#include "../RenderUtil/Lines3D.h"

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

void	PhysicsVehicleSetup::SetCentreOfMassOffset( float fCentreOfMassOffsetZ )
{
	mfCentreOfMassOffsetZ = fCentreOfMassOffsetZ;
}


void	PhysicsVehicleSetup::SetSuspensionDampingFactor( float fSuspensionDamping )
{
	mfSuspensionDampingFactor = fSuspensionDamping;
}

void	PhysicsVehicleSetup::SetSuspensionMaxTravel( float fSuspensionMaxTravel )
{
	mfSuspensionMaxTravel = fSuspensionMaxTravel;
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

void	PhysicsVehicleSetup::SetSteeringSpeed( float fSteeringSpeed )
{
	mfSteeringSpeed = fSteeringSpeed;
}

//----------------------------------------------------------------------------

static PhysicsGroundHeightOffsetFunc	mspfnGroundHeightOffset = NULL;

void		PhysicsVehicleSetGroundHeightOffsetFunc( PhysicsGroundHeightOffsetFunc pfnOffset )
{
	mspfnGroundHeightOffset = pfnOffset;
}

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
		mfGroundNormalSmoothing = 0.0f;
	}

	virtual void* castRay(const btVector3& from,const btVector3& to, btVehicleRaycasterResult& result);

	void	SetGroundNormalSmoothing( float fDistance ) { mfGroundNormalSmoothing = fDistance; }

	btDynamicsWorld*	mpDynamicsWorld;
	bool	mUseFilter;
	ushort	mCollisionFilterMask;
	ushort	mCollisionFilterGroup;
	float	mfGroundNormalSmoothing;

private:
	BOOL	LandscapeHeightAt( float fX, float fY, float fNearZ, float* pfHeight );
	BOOL	SmoothLandscapeNormal( const btVector3& xHit, btVector3* pxNormal );
};

//-------------------------------------------------------------
// Function    : LandscapeHeightAt
// Description : The landscape's height straight below (or above) a point, within 10 m of fNearZ
//-------------------------------------------------------------
BOOL	PhysicsVehicleRaycaster::LandscapeHeightAt( float fX, float fY, float fNearZ, float* pfHeight )
{
btVector3	xFrom( fX, fY, fNearZ + 10.0f );
btVector3	xTo( fX, fY, fNearZ - 10.0f );
btCollisionWorld::ClosestRayResultCallback	rayCallback( xFrom, xTo );

	rayCallback.m_collisionFilterMask = COLLISION_FILTER_LANDSCAPE;
	rayCallback.m_collisionFilterGroup = mCollisionFilterGroup;
	mpDynamicsWorld->rayTest( xFrom, xTo, rayCallback );
	if ( rayCallback.hasHit() == false )
	{
		return( FALSE );
	}
	*pfHeight = rayCallback.m_hitPointWorld.getZ();
	if ( mspfnGroundHeightOffset )
	{
		*pfHeight += mspfnGroundHeightOffset( fX, fY );
	}
	return( TRUE );
}

//-------------------------------------------------------------
// Function    : SmoothLandscapeNormal
// Description : The landscape's slope at a point from its heights mfGroundNormalSmoothing either side, in x and y.
//               Unlike the hit triangle's normal it changes smoothly as the point moves, so crossing a triangle edge
//               doesn't kick the suspension. FALSE (keep the triangle's normal) if any sample misses.
//-------------------------------------------------------------
BOOL	PhysicsVehicleRaycaster::SmoothLandscapeNormal( const btVector3& xHit, btVector3* pxNormal )
{
float	fDist = mfGroundNormalSmoothing;
float	fX = xHit.getX();
float	fY = xHit.getY();
float	fZ = xHit.getZ();
float	fXPlus, fXMinus, fYPlus, fYMinus;

	if ( ( LandscapeHeightAt( fX + fDist, fY, fZ, &fXPlus ) == FALSE ) ||
		 ( LandscapeHeightAt( fX - fDist, fY, fZ, &fXMinus ) == FALSE ) ||
		 ( LandscapeHeightAt( fX, fY + fDist, fZ, &fYPlus ) == FALSE ) ||
		 ( LandscapeHeightAt( fX, fY - fDist, fZ, &fYMinus ) == FALSE ) )
	{
		return( FALSE );
	}
	pxNormal->setValue( ( fXMinus - fXPlus ) / ( 2.0f * fDist ), ( fYMinus - fYPlus ) / ( 2.0f * fDist ), 1.0f );
	pxNormal->normalize();
	return( TRUE );
}


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

			BOOL	bLandscape = ( ( rayCallback.m_collisionObject->getBroadphaseHandle() != NULL ) &&
								   ( rayCallback.m_collisionObject->getBroadphaseHandle()->m_collisionFilterGroup & COLLISION_FILTER_LANDSCAPE ) ) ? TRUE : FALSE;

			// The drawn ground, when the game says it differs from the landscape: move the hit along the ray
			if ( ( bLandscape ) &&
				 ( mspfnGroundHeightOffset ) &&
				 ( from.getZ() - to.getZ() > 0.01f ) )
			{
			float	fOffset = mspfnGroundHeightOffset( rayCallback.m_hitPointWorld.getX(), rayCallback.m_hitPointWorld.getY() );
			float	fFraction = rayCallback.m_closestHitFraction - ( fOffset / ( from.getZ() - to.getZ() ) );

				if ( fFraction > 1.0f )
				{
					return 0;		// The drawn ground is below the ray's reach: the wheel's in the air
				}
				if ( fFraction < 0.0f ) fFraction = 0.0f;
				result.m_distFraction = fFraction;
				result.m_hitPointInWorld = from.lerp( to, fFraction );
			}

			if ( ( mfGroundNormalSmoothing > 0.0f ) &&
				 ( bLandscape ) )
			{
			btVector3	xSmoothNormal;

				if ( SmoothLandscapeNormal( result.m_hitPointInWorld, &xSmoothNormal ) )
				{
					result.m_hitNormalInWorld = xSmoothNormal;
				}
			}
			return body;
		}
	}
	return 0;
 }

//-------------------------------------------------------------

PhysicsVehicle::PhysicsVehicle()
{
	m_pRaycastVehicle = NULL;
	m_pCarChassisRigidBody = NULL;
	m_pVehicleRaycaster = NULL;
	m_pFrontWheelShape = NULL;
	m_pRearWheelShape = NULL;
	m_pChassisCompoundShape = NULL;
	mfChassisShapeHeight = 0.0f;
	mafWheelRadius[0] = mafWheelRadius[1] = mafWheelRadius[2] = mafWheelRadius[3] = 0.0f;
	mafDefaultWheelRadius[0] = mafDefaultWheelRadius[1] = mafDefaultWheelRadius[2] = mafDefaultWheelRadius[3] = 0.0f;
	memset( maxDefaultWheelPos, 0, sizeof( maxDefaultWheelPos ) );
	mnCurrentRPM = 0;
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

void		PhysicsVehicle::RenderDebug( void )
{
int		nX = 100;
int		nY = 100;
int		nLoop;

	for( nLoop = 0; nLoop < 4; nLoop++ )
	{
		InterfaceTxt( 1, nX, nY, 0, 0, "Wheel %d", nLoop+1 );
		nY += 20;
		InterfaceTxt( 1, nX, nY, 0, 0, "Susp Rel Vel: %f", m_pRaycastVehicle->getWheelInfo(0).m_suspensionRelativeVelocity );
		nY += 20;
	}
}



btRigidBody* PhysicsVehicle::localCreateRigidBody(btScalar mass, const btTransform& startTransform, btCollisionShape* shape, eVehicleCollisionType collisionType, float fCentreOfMassOffsetZ )
{
	btAssert((!shape || shape->getShapeType() != INVALID_SHAPE_PROXYTYPE));

	//rigidbody is dynamic if and only if mass is non zero, otherwise static
	bool isDynamic = (mass != 0.f);

	btVector3 localInertia(0,0,0);
	if (isDynamic)
		shape->calculateLocalInertia(mass,localInertia);
	 
	btTransform		centreOfMassOffset;
	centreOfMassOffset.setIdentity();
	centreOfMassOffset.setOrigin(btVector3(0.0f, 0.0f, 0.0f - fCentreOfMassOffsetZ ));

	//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects

	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform, centreOfMassOffset);

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


void		PhysicsVehicle::CreateVehicle( const VECT* pxPos, eVehicleCollisionType collisionType, const PhysicsVehicleSetup* pSetup )
{
float		fSuspensionRestLength = 0.0f;
btTransform tr;
PhysicsVehicleSetup		xDefaultSetup;
float		wheelFriction = 1000;//BT_LARGE_FLOAT;
//float		fChassisHeightOffGround = 0.2f;
float		wheelWidth = 0.2f;					// this will need to come from the game
float		fChassisHalfWidth = 1.0f;			// This will need to come from the game
float		fChassisHalfLength = 2.1f;			// This will need to come from the game
float		fChassisHalfHeight = 1.2f;			// This will need to come from the game
float		fFrontWheelRadius = 0.55f;			// This will need to come from the game
float		fRearWheelRadius = 0.85f;			// This will need to come from the game
float		fChassisCOMAdjustHeight = fRearWheelRadius + (fChassisHalfHeight - 0.2f);
float		fWheelConnectionHeight = fFrontWheelRadius;// - fSuspensionRestLength;// + 0.2f;

	// if wheelconnectionheight - (fWheelRadius+suspensionrestlength) > fComadjustHeight - fChassisHalfHeight
	// vehicle will be stuck on the ground

	if ( pSetup == NULL )
	{
		pSetup = &xDefaultSetup;
	}
	fSuspensionRestLength = pSetup->mfSuspensionRestLength;

	mPhysicsVehicleSetup = *pSetup;

	// The rigid body's origin is its centre of mass. With mfCentreOfMassHeight 0 that origin sits at ground level
	// under the chassis (the wheels' contact points), so no tyre force can roll or pitch it. A real height moves
	// the chassis box and the wheels down relative to the body by that much.
	float	fCOMHeight = mPhysicsVehicleSetup.mfCentreOfMassHeight;

	mfChassisShapeHeight = fChassisCOMAdjustHeight;
	mafWheelRadius[0] = fFrontWheelRadius;
	mafWheelRadius[1] = fFrontWheelRadius;
	mafWheelRadius[2] = fRearWheelRadius;
	mafWheelRadius[3] = fRearWheelRadius;

	tr.setIdentity();

	btCollisionShape* chassisShape = new btBoxShape(btVector3(fChassisHalfWidth,fChassisHalfLength,fChassisHalfHeight));
	m_apCollisionShapes.push_back(chassisShape);

	btCompoundShape* compound = new btCompoundShape();
	m_apCollisionShapes.push_back(compound);
	m_pChassisCompoundShape = compound;

	btTransform localTrans;
	localTrans.setIdentity();

	localTrans.setOrigin(btVector3(0,0,fChassisCOMAdjustHeight - fCOMHeight));

	compound->addChildShape(localTrans,chassisShape);

	tr.setOrigin(btVector3(pxPos->x, pxPos->y, pxPos->z + 3.0f + fCOMHeight ));

	m_pCarChassisRigidBody = localCreateRigidBody(pSetup->mfMass,tr,compound, collisionType, mPhysicsVehicleSetup.mfCentreOfMassOffsetZ );//chassisShape);

	//m_pCarChassisRigidBody->setDamping(0.2,0.2);
	m_pFrontWheelShape = new btCylinderShapeX(btVector3(wheelWidth,fFrontWheelRadius,fFrontWheelRadius));
	m_pRearWheelShape = new btCylinderShapeX(btVector3(wheelWidth,fRearWheelRadius,fRearWheelRadius));

	m_pVehicleRaycaster = new PhysicsVehicleRaycaster(mpDynamicsWorld, collisionType);
	m_pRaycastVehicle = new btRaycastVehicle(m_tuning, m_pCarChassisRigidBody, (btVehicleRaycaster*)m_pVehicleRaycaster);
		
	// never deactivate the vehicle
	m_pCarChassisRigidBody->setActivationState(DISABLE_DEACTIVATION);

	mpDynamicsWorld->addVehicle(m_pRaycastVehicle);

//	float connectionHeight = 0.28f;
	bool isFrontWheel=true;

	//choose coordinate system
	m_pRaycastVehicle->setCoordinateSystem( 0, 2, 1 );

	float fContactPointOffsetZ = fSuspensionRestLength - fCOMHeight;// * 0.5f;
	// Front Right ( +X, -Y )
	btVector3 connectionPointCS0(fChassisHalfWidth-(0.5f*wheelWidth),fChassisHalfLength-(fFrontWheelRadius*1.8f), fFrontWheelRadius + fContactPointOffsetZ);
	m_pRaycastVehicle->addWheel(connectionPointCS0,wheelDirectionCS0,wheelAxleCS,fSuspensionRestLength,fFrontWheelRadius,m_tuning,isFrontWheel);

	// Front Left ( -X, -Y )
	connectionPointCS0 = btVector3((0.0f - fChassisHalfWidth)+(0.5f*wheelWidth),fChassisHalfLength-(fFrontWheelRadius*1.8f),fFrontWheelRadius + fContactPointOffsetZ);
	m_pRaycastVehicle->addWheel(connectionPointCS0,wheelDirectionCS0,wheelAxleCS,fSuspensionRestLength,fFrontWheelRadius,m_tuning,isFrontWheel);

	isFrontWheel = false;
	// Rear Left ( -X, +Y )
	connectionPointCS0 = btVector3((0.0f - fChassisHalfWidth)+(0.3f*wheelWidth),(0.0f - fChassisHalfLength)+fRearWheelRadius,fRearWheelRadius + fContactPointOffsetZ);
	m_pRaycastVehicle->addWheel(connectionPointCS0,wheelDirectionCS0,wheelAxleCS,fSuspensionRestLength,fRearWheelRadius,m_tuning,isFrontWheel);

	// Rear Right ( +X, +Y )
	connectionPointCS0 = btVector3(fChassisHalfWidth-(0.3f*wheelWidth),(0.0f-fChassisHalfLength)+fRearWheelRadius,fRearWheelRadius + fContactPointOffsetZ);
	m_pRaycastVehicle->addWheel(connectionPointCS0,wheelDirectionCS0,wheelAxleCS,fSuspensionRestLength,fRearWheelRadius,m_tuning,isFrontWheel);

	// Keep the built-in layout's wheel centres (resting on the ground) for when the setup doesn't give its own
	for ( int nLoop = 0; ( nLoop < m_pRaycastVehicle->getNumWheels() ) && ( nLoop < 4 ); nLoop++ )
	{
	btWheelInfo&	wheel = m_pRaycastVehicle->getWheelInfo( nLoop );

		maxDefaultWheelPos[nLoop].x = wheel.m_chassisConnectionPointCS.getX();
		maxDefaultWheelPos[nLoop].y = wheel.m_chassisConnectionPointCS.getY();
		maxDefaultWheelPos[nLoop].z = mafWheelRadius[nLoop];
		mafDefaultWheelRadius[nLoop] = mafWheelRadius[nLoop];
	}
	if ( mPhysicsVehicleSetup.HasWheelGeometry() )
	{
		ApplyGeometry();
	}

	SetAllWheelSettings( pSetup );

}

void		PhysicsVehicle::SetAllWheelSettings( const PhysicsVehicleSetup* pSetup )
{
	for (int i=0;i<m_pRaycastVehicle->getNumWheels();i++)
	{
		btWheelInfo& wheel = m_pRaycastVehicle->getWheelInfo(i);
		// From https://docs.google.com/document/d/18edpOwtGgCwNyvakS78jxMajCuezotCU_0iezcwiFQc/edit ...
		// The stiffness constant for the suspension.  10.0 - Offroad buggy, 50.0 - Sports car, 200.0 - F1 Car
		wheel.m_suspensionStiffness = mPhysicsVehicleSetup.mfSuspensionStiffness;

		wheel.m_maxSuspensionTravelCm = mPhysicsVehicleSetup.mfSuspensionMaxTravel;

		/// The damping coefficient for when the suspension is compressed. Set to k * 2.0 * btSqrt(m_suspensionStiffness) so k is proportional to critical damping.
          /// k = 0.0 undamped & bouncy, k = 1.0 critical damping
          /// k = 0.1 to 0.3 are good values
		wheel.m_wheelsDampingCompression = mPhysicsVehicleSetup.mfSuspensionDampingFactor * 2.0f * btSqrt( wheel.m_suspensionStiffness );

		/// The damping coefficient for when the suspension is expanding.  See the comments for m_wheelsDampingCompression for how to set k.
          /// m_wheelsDampingRelaxation should be slightly larger than m_wheelsDampingCompression, eg k = 0.2 to 0.5
		wheel.m_wheelsDampingRelaxation = (mPhysicsVehicleSetup.mfSuspensionDampingFactor*1.5f) * 2.0f * btSqrt( wheel.m_suspensionStiffness );
		  
		/// The coefficient of friction between the tyre and the ground.
          /// Should be about 0.8 for realistic cars, but can increased for better handling.
          /// Set large (10000.0) for kart racers
		wheel.m_frictionSlip = mPhysicsVehicleSetup.mfFrictionSlip;
		wheel.m_rollInfluence = mPhysicsVehicleSetup.mfRollInfluence;
		wheel.m_maxSuspensionForce = mPhysicsVehicleSetup.mfMaxSuspensionForce;
	}
	if ( m_pVehicleRaycaster )
	{
		m_pVehicleRaycaster->SetGroundNormalSmoothing( mPhysicsVehicleSetup.mfGroundNormalSmoothing );
	}

}

//-------------------------------------------------------------
// Function    : ApplyGeometry
// Description : Puts the chassis box and the wheel mounts where the current setup's rest length and centre of mass
//               height say, relative to the rigid body (whose origin is the centre of mass)
//-------------------------------------------------------------
void		PhysicsVehicle::ApplyGeometry( void )
{
float	fCOMHeight = mPhysicsVehicleSetup.mfCentreOfMassHeight;
float	fRestLength = mPhysicsVehicleSetup.mfSuspensionRestLength;
int		nLoop;

	if ( ( m_pChassisCompoundShape ) &&
		 ( m_pChassisCompoundShape->getNumChildShapes() > 0 ) )
	{
	btTransform		xChildTrans;

		xChildTrans.setIdentity();
		xChildTrans.setOrigin( btVector3( 0.0f, 0.0f, mfChassisShapeHeight - fCOMHeight ) );
		m_pChassisCompoundShape->updateChildTransform( 0, xChildTrans, true );
	}

	for ( nLoop = 0; ( nLoop < m_pRaycastVehicle->getNumWheels() ) && ( nLoop < 4 ); nLoop++ )
	{
	btWheelInfo&	wheel = m_pRaycastVehicle->getWheelInfo( nLoop );
	const VECT*		pxCentre = &maxDefaultWheelPos[nLoop];
	float			fRadius = mafDefaultWheelRadius[nLoop];

		// The setup's own wheel centres (e.g. from the model) when it has them
		if ( ( mPhysicsVehicleSetup.mbWheelGeometrySet ) &&
			 ( mPhysicsVehicleSetup.mafWheelRadius[nLoop] > 0.0f ) )
		{
			pxCentre = &mPhysicsVehicleSetup.maxWheelPos[nLoop];
			fRadius = mPhysicsVehicleSetup.mafWheelRadius[nLoop];
		}
		mafWheelRadius[nLoop] = fRadius;
		wheel.m_wheelsRadius = fRadius;
		wheel.m_suspensionRestLength1 = fRestLength;
		// The mount sits a rest length above the wheel's centre, which is pxCentre->z above the ground under the vehicle
		wheel.m_chassisConnectionPointCS.setValue( pxCentre->x, pxCentre->y, pxCentre->z + fRestLength - fCOMHeight );
	}
}

PhysicsVehicleSetup*		PhysicsVehicle::GetCurrentSetup( void )
{
	return( &mPhysicsVehicleSetup );
}

void		PhysicsVehicle::ModifySettings( const PhysicsVehicleSetup* pSetup )
{
btVector3		inertia( 0.0f, 0.0f, 0.0f );

	if ( mPhysicsVehicleSetup.IsDifferent( pSetup ) )
	{
	float	fOldCOMHeight = mPhysicsVehicleSetup.mfCentreOfMassHeight;
	BOOL	bGeometryChanged = FALSE;

		if ( ( pSetup->mfSuspensionRestLength != mPhysicsVehicleSetup.mfSuspensionRestLength ) ||
			 ( pSetup->mfCentreOfMassHeight != mPhysicsVehicleSetup.mfCentreOfMassHeight ) ||
			 ( mPhysicsVehicleSetup.IsWheelGeometryDifferent( pSetup ) ) )
		{
			bGeometryChanged = TRUE;
		}

		// mfEnginePowerRating is used in the UpdateEngine call every frame
		mPhysicsVehicleSetup = *pSetup;

		// SetAllWheelSettings updates mfWheelsDampingCompression, mfWheelsDampingRelaxation, mfFrictionSlip, mfRollInfluence, mfSuspensionStiffness
		SetAllWheelSettings( pSetup );

		if ( bGeometryChanged )
		{
			ApplyGeometry();
			// Keep the vehicle where it was: the centre of mass moved within it
			if ( pSetup->mfCentreOfMassHeight != fOldCOMHeight )
			{
			btTransform		xTrans = m_pCarChassisRigidBody->getWorldTransform();

				xTrans.setOrigin( xTrans( btVector3( 0.0f, 0.0f, pSetup->mfCentreOfMassHeight - fOldCOMHeight ) ) );
				m_pCarChassisRigidBody->proceedToTransform( xTrans );
			}
		}

		// Update rigid body mass, with the inertia worked out again for the new mass (keeping the old inertia
		// made a heavier vehicle twist as if it were the old weight)
		if ( m_pChassisCompoundShape )
		{
			m_pChassisCompoundShape->calculateLocalInertia( pSetup->mfMass, inertia );
		}
		else
		{
			inertia = m_pCarChassisRigidBody->getLocalInertia();
		}
		m_pCarChassisRigidBody->setMassProps( pSetup->mfMass, inertia );
		m_pCarChassisRigidBody->updateInertiaTensor();
	}
}

int		PhysicsVehicle::GetNumWheelsOnGround( void )
{
int		nLoop;
int		nCount = 0;

	for ( nLoop = 0; nLoop < m_pRaycastVehicle->getNumWheels(); nLoop++ )
	{
		if ( m_pRaycastVehicle->getWheelInfo( nLoop ).m_raycastInfo.m_isInContact )
		{
			nCount++;
		}
	}
	return( nCount );
}

float	PhysicsVehicle::GetWheelSuspensionLength( int nWheelNum )
{
	if ( ( nWheelNum < 0 ) || ( nWheelNum >= m_pRaycastVehicle->getNumWheels() ) ) return( 0.0f );
	return( m_pRaycastVehicle->getWheelInfo( nWheelNum ).m_raycastInfo.m_suspensionLength );
}

BOOL	PhysicsVehicle::IsWheelSliding( int nWheelNum )
{
	if ( ( nWheelNum < 0 ) || ( nWheelNum >= m_pRaycastVehicle->getNumWheels() ) ) return( FALSE );
	return( ( m_pRaycastVehicle->getWheelInfo( nWheelNum ).m_skidInfo < 1.0f ) ? TRUE : FALSE );
}

//-------------------------------------------------------------
// Function    : ApplyDragForces
// Description : Air drag and rolling resistance. Without them nothing but the brake slows the vehicle, and it
//               speeds up for as long as the accelerator is held. Applied before the next physics step.
//-------------------------------------------------------------
void		PhysicsVehicle::ApplyDragForces( void )
{
btVector3	xVel = m_pCarChassisRigidBody->getLinearVelocity();
float		fSpeed = xVel.length();
float		fForce;

	if ( fSpeed < 0.01f ) return;

	fForce = GetDragCoefficientInUse() * fSpeed * fSpeed;
	if ( ( mPhysicsVehicleSetup.mfRollingResistance > 0.0f ) &&
		 ( GetNumWheelsOnGround() > 0 ) )
	{
	float	fRamp = fSpeed / 0.5f;		// Fade out near standstill so it can't push the vehicle backwards

		if ( fRamp > 1.0f ) fRamp = 1.0f;
		fForce += mPhysicsVehicleSetup.mfRollingResistance * mPhysicsVehicleSetup.mfMass * fRamp;
	}

	if ( fForce > 0.0f )
	{
		m_pCarChassisRigidBody->applyCentralForce( xVel * ( 0.0f - ( fForce / fSpeed ) ) );
	}
}

void		PhysicsVehicle::StopMoving( void )
{
	m_pCarChassisRigidBody->setLinearVelocity( btVector3( 0.0f, 0.0f, 0.0f ) );
	m_pCarChassisRigidBody->setAngularVelocity( btVector3( 0.0f, 0.0f, 0.0f ) );
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
	if ( mPhysicsVehicleSetup.mfCentreOfMassHeight != 0.0f )
	{
	btVector3	xGroundPos = trans( btVector3( 0.0f, 0.0f, 0.0f - mPhysicsVehicleSetup.mfCentreOfMassHeight ) );

		pxOut->x = xGroundPos.getX();
		pxOut->y = xGroundPos.getY();
		pxOut->z = xGroundPos.getZ();
		return;
	}
	*pxOut = *( (VECT*)&trans.getOrigin() );
}

void	PhysicsVehicle::GetWheelPosition( int nWheelNum, ENGINEMATRIX* pxOut )
{
btTransform rTrans;
btMatrix3x3 rRotMat;

	memset( pxOut, 0, sizeof( ENGINEMATRIX ) );
	// updateWheelTransform clears the wheel's in-contact flag as a side effect. Keep it, so it still says what the
	// last physics step found until the next one (frames without a step used to read every wheel as airborne)
	bool	bWasInContact = m_pRaycastVehicle->getWheelInfo( nWheelNum ).m_raycastInfo.m_isInContact;
	m_pRaycastVehicle->updateWheelTransform( nWheelNum );
	m_pRaycastVehicle->getWheelInfo( nWheelNum ).m_raycastInfo.m_isInContact = bWasInContact;
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
	// pxPos is the ground-level position GetPosition reports; the body sits mfCentreOfMassHeight above it
	xPos += trans.getBasis() * btVector3( 0.0f, 0.0f, mPhysicsVehicleSetup.mfCentreOfMassHeight );
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
	xPos += trans.getBasis() * btVector3( 0.0f, 0.0f, mPhysicsVehicleSetup.mfCentreOfMassHeight );
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

//-------------------------------------------------------------
// Function    : GetEngineForceAtMPH
// Description : The full-throttle force one driven wheel gets at a speed, from the gear table and the engine power
//-------------------------------------------------------------
float		PhysicsVehicle::GetEngineForceAtMPH( float fSpeed )
{
GEAR_SETTINGS*		pxGearSettings = maGearSettings;
float	fGearRange;

	fSpeed = fabsf( fSpeed );
	while( fSpeed > pxGearSettings->fMaxSpeed )
	{
		pxGearSettings++;
		if ( pxGearSettings->fMaxSpeed == -1.0f )
		{
			pxGearSettings--;
			break;
		}
	}
	fGearRange = ( fSpeed - pxGearSettings->fMinSpeed ) / ( pxGearSettings->fMaxSpeed - pxGearSettings->fMinSpeed );
	return( ( pxGearSettings->fBaseForce + ( pxGearSettings->fPeakExtraForce * sinf( A45 + ( fGearRange * A90 ) ) ) ) * 1.75f * mPhysicsVehicleSetup.mfEnginePowerRating );
}

//-------------------------------------------------------------
// Function    : GetDragCoefficientInUse
// Description : The setup's drag, or with a top speed set, the drag at which the two driven wheels' full-throttle
//               force (less rolling resistance) only just holds that speed on the flat
//-------------------------------------------------------------
float		PhysicsVehicle::GetDragCoefficientInUse( void )
{
float	fTopMPH = mPhysicsVehicleSetup.mfTopSpeedMPH;
float	fTopMetresPerSec;
float	fForce;

	if ( fTopMPH <= 0.0f )
	{
		return( mPhysicsVehicleSetup.mfDragCoefficient );
	}
	fTopMetresPerSec = fTopMPH * 0.44704f;
	fForce = ( 2.0f * GetEngineForceAtMPH( fTopMPH ) ) - ( mPhysicsVehicleSetup.mfRollingResistance * mPhysicsVehicleSetup.mfMass );
	if ( fForce <= 0.0f )
	{
		return( mPhysicsVehicleSetup.mfDragCoefficient );
	}
	return( fForce / ( fTopMetresPerSec * fTopMetresPerSec ) );
}

void		PhysicsVehicle::UpdateEngine( float fDelta )
{
float	fSignedSpeed = GetCurrentSpeedMPH();
// The gear table is for speeds forward or back: with the signed speed, reversing ran the gear maths on negative
// numbers, giving a force that rose and fell erratically (and negative RPMs)
float	fSpeed = fabsf( fSignedSpeed );
float	fMaxEngineForce;
float	fMaxBreakingForce = mPhysicsVehicleSetup.mfBrakeForce;
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

	// Reverse speed limit, when one is set
	if ( ( mfVehicleAcceleratorAmount < 0.0f ) &&
		 ( mPhysicsVehicleSetup.mfReverseMaxMPH > 0.0f ) &&
		 ( ( 0.0f - fSignedSpeed ) >= mPhysicsVehicleSetup.mfReverseMaxMPH ) )
	{
		mfVehicleEngineForce = 0.0f;
	}
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

	ApplyDragForces();

#ifdef PHYSICS_VEHICLE_ONSCREENDEBUG
	char		acString[256];

	sprintf( acString, "Veh: Steer: %.3f   Acc: %.2f   Brake: %.2f  Delta: %.5f", mfVehicleSteering, mfVehicleEngineForce, mfVehicleBrakeForce, fDelta );
	InterfaceText( 1, 100, 10, acString, 0xd0d0d0d0, 0 );
#endif // ifdef PHYSICS_VEHICLE_ONSCREENDEBUG

}



