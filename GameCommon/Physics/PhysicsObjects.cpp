
#include "StandardDef.h"
#include "Engine.h"
#include "Rendering.h"

#include "../../Pub/LibCode/Engine/ModelRendering.h"		// Cheeky

#include "btBulletWorldImporter.h"
#include "PhysicsWorld.h"
#include "PhysicsInternal.h"
#include "PhysicsObjects.h"  


class PhysicsObject
{
public:
	PhysicsObject()
	{
		mpRigidBody = NULL;
		mnHandleID = 0;
		mpNext = NULL;
		mpCollisionShape =  NULL;
	}

	btCollisionShape*	mpCollisionShape;
	btRigidBody*		mpRigidBody;
	int					mnHandleID;
	PhysicsObject*		mpNext;
};


PhysicsObject*		mspPhysicsObjects = NULL;
int					msnNextPhysicsObjectHandle = 100;

PhysicsObject*		PhysicsObjectFind( int hPhysicsObject )
{
PhysicsObject*		pList =	mspPhysicsObjects;

	while( pList )
	{
		if ( pList->mnHandleID == hPhysicsObject )
		{
			return( pList );
		}
		pList = pList->mpNext;
	}
	return( NULL );
}

PhysicsObject*		PhysicsObjectCreateNew( void )
{
PhysicsObject*		pPhysicsObject = new PhysicsObject;

	pPhysicsObject->mnHandleID = msnNextPhysicsObjectHandle;
	msnNextPhysicsObjectHandle++;
	pPhysicsObject->mpNext = mspPhysicsObjects;
	mspPhysicsObjects = pPhysicsObject;

	return( pPhysicsObject );
}

#define USE_BTBVHTRIANGLEMESH_FOR_LANDSCAPE

#ifdef USE_BTBVHTRIANGLEMESH_FOR_LANDSCAPE

btTriangleMesh mHeightfieldTriMesh;

int		PhysicsObjectAddHeightfield( int nHeightfieldWidth, int nHeightfieldHeight, float* pfHeightfield , float fGridScale, float fHeightScale )
{
	if ( mpDynamicsWorld )
	{
	PhysicsObject*		pPhysicsObject = PhysicsObjectCreateNew();
	btDefaultMotionState* pGroundMotionState;
	btBvhTriangleMeshShape*		pTerrainShape;
	btVector3	vecHeightfieldPos( 0.0f, 0.0f, 0.0f );
	int		nLoopX;
	int		nLoopY;
	VECT	xPos1;
	VECT	xPos2;
	VECT	xPos3;
	VECT	xPos4;

		for ( nLoopY = 0; nLoopY < nHeightfieldHeight - 1; nLoopY++ )
		{
			for ( nLoopX = 0; nLoopX < nHeightfieldWidth - 1; nLoopX++ )
			{
				xPos1.x = (FLOAT)( nLoopX * fGridScale );
				xPos1.y = (FLOAT)( nLoopY * fGridScale );
				xPos1.z = pfHeightfield[ (nLoopY * nHeightfieldWidth) + nLoopX ];

				xPos2.x = (FLOAT)( (nLoopX + 1) * fGridScale );
				xPos2.y = (FLOAT)( nLoopY * fGridScale );
				xPos2.z = pfHeightfield[ (nLoopY * nHeightfieldWidth) + nLoopX + 1 ];

				xPos3.x = (FLOAT)( nLoopX * fGridScale );
				xPos3.y = (FLOAT)( (nLoopY + 1) * fGridScale );
				xPos3.z = pfHeightfield[ ((nLoopY+1) * nHeightfieldWidth) + nLoopX ];

				xPos4.x = (FLOAT)( (nLoopX + 1) * fGridScale );
				xPos4.y = (FLOAT)( (nLoopY + 1) * fGridScale );
				xPos4.z = pfHeightfield[ ((nLoopY+1) * nHeightfieldWidth) + (nLoopX+1) ];

				mHeightfieldTriMesh.addTriangle(btVector3(xPos1.x, xPos1.y, xPos1.z), btVector3(xPos3.x, xPos3.y, xPos3.z), btVector3( xPos2.x, xPos2.y, xPos2.z), false );
				mHeightfieldTriMesh.addTriangle(btVector3(xPos3.x, xPos3.y, xPos3.z), btVector3(xPos4.x, xPos4.y, xPos4.z), btVector3( xPos2.x, xPos2.y, xPos2.z), false );
			}
		}
		
		pTerrainShape = new btBvhTriangleMeshShape(&mHeightfieldTriMesh, false);
		pPhysicsObject->mpCollisionShape = pTerrainShape;

		btTriangleInfoMap* pTriangleInfoMap = new btTriangleInfoMap();
		//now you can adjust some thresholds in triangleInfoMap  if needed.
		//btGenerateInternalEdgeInfo fills in the btTriangleInfoMap and stores it as a user pointer of trimeshShape (trimeshShape->setUserPointer(triangleInfoMap))
		btGenerateInternalEdgeInfo(pTerrainShape,pTriangleInfoMap);

		pGroundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), vecHeightfieldPos));
		btRigidBody::btRigidBodyConstructionInfo   groundRigidBodyCI(0, pGroundMotionState, pPhysicsObject->mpCollisionShape, btVector3(0, 0, 0));
		groundRigidBodyCI.m_restitution = 0.1f;

		pPhysicsObject->mpRigidBody = new btRigidBody(groundRigidBodyCI);

		int		f = pPhysicsObject->mpRigidBody->getCollisionFlags();
		pPhysicsObject->mpRigidBody->setCollisionFlags( f | btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK );
//		pPhysicsObject->mpRigidBody->setCollisionFlags( f | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK );

		mpDynamicsWorld->addRigidBody( pPhysicsObject->mpRigidBody, COLLISION_FILTER_LANDSCAPE, COLLISION_FILTER_LOCAL_VEHICLE | COLLISION_FILTER_REMOTE_VEHICLE | COLLISION_FILTER_REMOTE_PREDICTION_VEHICLE |	COLLISION_FILTER_WEAPON_RAY | COLLISION_FILTER_PHYSICS_OBJECTS );

		return( pPhysicsObject->mnHandleID );
	}
	return( NOTFOUND );
}
#else

int		PhysicsObjectAddHeightfield( int nHeightfieldWidth, int nHeightfieldHeight, float* pfHeightfield , float fGridScale, float fHeightScale )
{
	if ( mpDynamicsWorld )
	{
	PhysicsObject*		pPhysicsObject = PhysicsObjectCreateNew();
	btDefaultMotionState* pGroundMotionState;
	btHeightfieldTerrainShape*		pTerrainShape;
	btVector3	vecHeightfieldPos;

		vecHeightfieldPos.setX( (nHeightfieldWidth-1) * fGridScale * 0.5f );
		vecHeightfieldPos.setY( (nHeightfieldHeight-1) * fGridScale * 0.5f );
		vecHeightfieldPos.setZ( 0.0f );

		// TODO - 0.85f = mfLandscapeHeightScale
		float	fCharHeightScaleMod = 0.0f;		// Note this param is ignored by btHeightfieldTerrainShape when using PHY_FLOAT
		// -256, 256 = min and max height...
		pPhysicsObject->mpCollisionShape = new btHeightfieldTerrainShape( nHeightfieldWidth, nHeightfieldHeight, pfHeightfield, fCharHeightScaleMod, -256.0f, 256.0f, 2, PHY_FLOAT, true );
		pTerrainShape = (btHeightfieldTerrainShape*)( pPhysicsObject->mpCollisionShape );
		pTerrainShape->setLocalScaling( btVector3( fGridScale, fGridScale, fHeightScale ) );
	//	pTerrainShape->setUseDiamondSubdivision( true );
	//	pTerrainShape->setUseZigzagSubdivision( true );

		pGroundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), vecHeightfieldPos));
		btRigidBody::btRigidBodyConstructionInfo   groundRigidBodyCI(0, pGroundMotionState, pPhysicsObject->mpCollisionShape, btVector3(0, 0, 0));
		groundRigidBodyCI.m_restitution = 0.1f;

		pPhysicsObject->mpRigidBody = new btRigidBody(groundRigidBodyCI);

		int		f = pPhysicsObject->mpRigidBody->getCollisionFlags();
		pPhysicsObject->mpRigidBody->setCollisionFlags( f | btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK );
//		pPhysicsObject->mpRigidBody->setCollisionFlags( f | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK );

		mpDynamicsWorld->addRigidBody( pPhysicsObject->mpRigidBody, COLLISION_FILTER_LANDSCAPE, COLLISION_FILTER_LOCAL_VEHICLE | COLLISION_FILTER_REMOTE_VEHICLE | COLLISION_FILTER_REMOTE_PREDICTION_VEHICLE |	COLLISION_FILTER_WEAPON_RAY | COLLISION_FILTER_PHYSICS_OBJECTS );

		return( pPhysicsObject->mnHandleID );
	}
	return( NOTFOUND );
}
#endif


int		PhysicsObjectAddPrimitive( int type, const VECT* pxPos, const VECT* pxRot, const VECT* pxSize, float fMass, float fLinearFriction )
{
PhysicsObject*		pPhysicsObject = PhysicsObjectCreateNew();
btDefaultMotionState* pMotionState;
btVector3	inertia(0, 0, 0);
btQuaternion	rotationQuat( pxRot->x, pxRot->y, pxRot->z );

	pMotionState = new btDefaultMotionState(btTransform(rotationQuat, btVector3(pxPos->x, pxPos->y, pxPos->z)));

	switch( type )
	{
	case 0:		// sphere
	default:
        pPhysicsObject->mpCollisionShape = new btSphereShape( pxSize->x );

		break;
	case 1:		// plane
		pPhysicsObject->mpCollisionShape = new btStaticPlaneShape(btVector3(0, 0, 1), 1);
		break;
	case 2:		// box
		pPhysicsObject->mpCollisionShape = new btBoxShape( btVector3( pxSize->x * 0.5f, pxSize->y * 0.5f, pxSize->z * 0.5f ) );
		break;
	}

	pPhysicsObject->mpCollisionShape->calculateLocalInertia(fMass, inertia);

	btRigidBody::btRigidBodyConstructionInfo   rigidBodyCI( fMass, pMotionState, pPhysicsObject->mpCollisionShape, inertia);
	rigidBodyCI.m_restitution = 0.0f;
	rigidBodyCI.m_friction = fLinearFriction;
	rigidBodyCI.m_rollingFriction = 0.2f;
	rigidBodyCI.m_angularDamping = 0.1f;
	pPhysicsObject->mpRigidBody = new btRigidBody(rigidBodyCI);

	ushort		uwCollidesWith = COLLISION_FILTER_LANDSCAPE | COLLISION_FILTER_STATIC_OBJECT | COLLISION_FILTER_PHYSICS_OBJECTS;
	mpDynamicsWorld->addRigidBody( pPhysicsObject->mpRigidBody, COLLISION_FILTER_PHYSICS_OBJECTS, uwCollidesWith );

	return( pPhysicsObject->mnHandleID );
}


int		PhysicsObjectAddConvexHull( VECT* pxPointsList, int nNumPoints, float fMass )
{
PhysicsObject*		pPhysicsObject = PhysicsObjectCreateNew();
btDefaultMotionState* pMotionState;
btVector3	inertia(0, 0, 0);

	pMotionState = new btDefaultMotionState(btTransform( btQuaternion(0.0f,0.0f,0.0f), btVector3(0.0f,0.0f,0.0f)));

	pPhysicsObject->mpCollisionShape = new btConvexHullShape( (btScalar*)pxPointsList, nNumPoints, sizeof( VECT ) );

	pPhysicsObject->mpCollisionShape->calculateLocalInertia(fMass, inertia);

	btRigidBody::btRigidBodyConstructionInfo   rigidBodyCI( fMass, pMotionState, pPhysicsObject->mpCollisionShape, inertia);

	pPhysicsObject->mpRigidBody = new btRigidBody(rigidBodyCI);

	mpDynamicsWorld->addRigidBody( pPhysicsObject->mpRigidBody, COLLISION_FILTER_STATIC_OBJECT, COLLISION_FILTER_LOCAL_VEHICLE | COLLISION_FILTER_REMOTE_VEHICLE | COLLISION_FILTER_REMOTE_PREDICTION_VEHICLE |	COLLISION_FILTER_WEAPON_RAY );

	return( pPhysicsObject->mnHandleID );
}




int		PhysicsObjectAddMesh( int nModelHandle, VECT* pxPos, VECT* pxRot, float fMass )
{
MODEL_RENDER_DATA*		pxModelData;
PhysicsObject*		pPhysicsObject = PhysicsObjectCreateNew();
btDefaultMotionState* pMotionState;
btVector3	inertia(0, 0, 0);
CUSTOMVERTEX*	pxVertices = NULL;
btQuaternion	rotationQuat( pxRot->x, pxRot->y, pxRot->z );

	pMotionState = new btDefaultMotionState(btTransform( rotationQuat, btVector3(pxPos->x, pxPos->y, pxPos->z)));

	pxModelData = &maxModelRenderData[ nModelHandle ];
	// TODO - Triangle mesh or convex hull...
	pxModelData->pxBaseMesh->LockVertexBuffer( 0, (BYTE**)&pxVertices );
	
	pPhysicsObject->mpCollisionShape = new btConvexHullShape( (btScalar*)&pxVertices->position, pxModelData->xStats.nNumVertices, sizeof( CUSTOMVERTEX ) );

	pxModelData->pxBaseMesh->UnlockVertexBuffer();

	pPhysicsObject->mpCollisionShape->calculateLocalInertia(fMass, inertia);

	btRigidBody::btRigidBodyConstructionInfo   rigidBodyCI( fMass, pMotionState, pPhysicsObject->mpCollisionShape, inertia);

	pPhysicsObject->mpRigidBody = new btRigidBody(rigidBodyCI);

	mpDynamicsWorld->addRigidBody( pPhysicsObject->mpRigidBody, COLLISION_FILTER_STATIC_OBJECT, COLLISION_FILTER_LOCAL_VEHICLE | COLLISION_FILTER_REMOTE_VEHICLE | COLLISION_FILTER_REMOTE_PREDICTION_VEHICLE |	COLLISION_FILTER_WEAPON_RAY );

	return( pPhysicsObject->mnHandleID );
}

void		PhysicsObjectRemove( int hPhysicsObject )
{
PhysicsObject*		pObject = PhysicsObjectFind( hPhysicsObject );

	if ( pObject )
	{
		mpDynamicsWorld->removeRigidBody( pObject->mpRigidBody );
	}
}

BOOL		PhysicsObjectSetVelocity( int hPhysicsObject, const VECT* pxVel )
{
PhysicsObject*		pObject = PhysicsObjectFind( hPhysicsObject );

	if ( pObject )
	{
	btVector3		xVel;
	
		xVel = *( (btVector3*)( pxVel ) );
		pObject->mpRigidBody->setLinearVelocity( xVel );
		return( TRUE );
	}
	return( FALSE );
}

BOOL		PhysicsObjectOverridePositionAndOrientation( int hPhysicsObject, const VECT* pxPos, const ENGINEQUATERNION* pQuat )
{
PhysicsObject*		pObject = PhysicsObjectFind( hPhysicsObject );

	if ( pObject )
	{
	btTransform trans;
	btQuaternion	xQuat;
	btVector3		xPos;
	btVector3		xVel;

		xVel.setValue( 0.0f, 0.0f, 0.0f );

		pObject->mpRigidBody->getMotionState()->getWorldTransform( trans );

		xQuat.setValue( pQuat->x, pQuat->y, pQuat->z, pQuat->w );
		xPos.setValue( pxPos->x, pxPos->y, pxPos->z );

		trans.setRotation( xQuat );
		trans.setOrigin( xPos );

//		pObject->mpRigidBody->proceedToTransform( trans );	
//		xNewMotionState.setWorldTransform( trans );
		
		pObject->mpRigidBody->setCenterOfMassTransform(trans);
//		pObject->mpRigidBody->getMotionState()->setWorldTransform( trans );
		pObject->mpRigidBody->setLinearVelocity( xVel );
		return( TRUE );
	}
	return( FALSE );
}


BOOL		PhysicsObjectApplyLinearForce( int hPhysicsObject, const VECT* pxForce )
{
PhysicsObject*		pObject = PhysicsObjectFind( hPhysicsObject );

	if ( pObject )
	{
	btVector3		xForce;

		xForce.setValue( pxForce->x, pxForce->y, pxForce->z );
		pObject->mpRigidBody->applyCentralForce( xForce );
		pObject->mpRigidBody->activate( true );
		return( TRUE );
	}
	return( FALSE );
}

BOOL		PhysicsObjectApplyAngularForce( int hPhysicsObject, const VECT* pxForce )
{
PhysicsObject*		pObject = PhysicsObjectFind( hPhysicsObject );

	if ( pObject )
	{
	btVector3		xForce;

		xForce.setValue( pxForce->x, pxForce->y, pxForce->z );
		pObject->mpRigidBody->applyTorque( xForce );
		pObject->mpRigidBody->activate( true );
		return( TRUE );
	}
	return( FALSE );
}

BOOL		PhysicsObjectGetLinearVelocity( int hPhysicsObject, VECT* pxOut )
{
PhysicsObject*		pObject = PhysicsObjectFind( hPhysicsObject );

	if ( pObject )
	{
	btVector3	vecVel = pObject->mpRigidBody->getLinearVelocity();

		pxOut->x = vecVel.getX();
		pxOut->y = vecVel.getY();
		pxOut->z = vecVel.getZ();
		return( TRUE );
	}
	return( FALSE );
}

BOOL		PhysicsObjectGetTransform( int hPhysicsObject, PhysicsObjectTransform* pxOut )
{
PhysicsObject*		pObject = PhysicsObjectFind( hPhysicsObject );

	if ( pObject )
	{
	btTransform trans;
	btQuaternion	xQuat;

		pObject->mpRigidBody->getMotionState()->getWorldTransform(trans);
	
		pxOut->mxPosition = *( (VECT*)&trans.getOrigin() );
		xQuat = trans.getRotation();
		pxOut->mxQuatRotation = *( (ENGINEQUATERNION*)&xQuat );
		return( TRUE );
	}
	return( FALSE );
}


unsigned int	mhPhysicsBVHGenerationThread = 0;
PhysicsObject*	mpLandscapePhysicsObject = NULL;
char*			mszPhysicsLandscapeOutputFilename = NULL;
BOOL			mboLandscapeCacheLoadDidError = FALSE;

BOOL		PhysicsLandscapeCacheLoadDidError( void )
{
	return( mboLandscapeCacheLoadDidError );
}

void		PhysicsGenerateLandscapeTrimesh( int nHeightfieldWidth, int nHeightfieldHeight, float* pfHeightfield, float fGridScale, float fHeightScale )
{
int		nLoopX;
int		nLoopY;
VECT	xPos1;
VECT	xPos2;
VECT	xPos3;
VECT	xPos4;

	for ( nLoopY = 0; nLoopY < nHeightfieldHeight - 1; nLoopY++ )
	{
		for ( nLoopX = 0; nLoopX < nHeightfieldWidth - 1; nLoopX++ )
		{
			xPos1.x = (FLOAT)( nLoopX * fGridScale );
			xPos1.y = (FLOAT)( nLoopY * fGridScale );
			xPos1.z = pfHeightfield[ (nLoopY * nHeightfieldWidth) + nLoopX ];

			xPos2.x = (FLOAT)( (nLoopX + 1) * fGridScale );
			xPos2.y = (FLOAT)( nLoopY * fGridScale );
			xPos2.z = pfHeightfield[ (nLoopY * nHeightfieldWidth) + nLoopX + 1 ];

			xPos3.x = (FLOAT)( nLoopX * fGridScale );
			xPos3.y = (FLOAT)( (nLoopY + 1) * fGridScale );
			xPos3.z = pfHeightfield[ ((nLoopY+1) * nHeightfieldWidth) + nLoopX ];

			xPos4.x = (FLOAT)( (nLoopX + 1) * fGridScale );
			xPos4.y = (FLOAT)( (nLoopY + 1) * fGridScale );
			xPos4.z = pfHeightfield[ ((nLoopY+1) * nHeightfieldWidth) + (nLoopX+1) ];

			mHeightfieldTriMesh.addTriangle(btVector3(xPos1.x, xPos1.y, xPos1.z), btVector3(xPos3.x, xPos3.y, xPos3.z), btVector3( xPos2.x, xPos2.y, xPos2.z), false );
			mHeightfieldTriMesh.addTriangle(btVector3(xPos3.x, xPos3.y, xPos3.z), btVector3(xPos4.x, xPos4.y, xPos4.z), btVector3( xPos2.x, xPos2.y, xPos2.z), false );
		}
	}
}


void	PhysicsGenerateLandscapeBVH( void )
{ 
PhysicsObject*		pPhysicsObject;
btDefaultMotionState* pGroundMotionState;
btBvhTriangleMeshShape*		pTerrainShape;
btVector3	vecHeightfieldPos( 0.0f, 0.0f, 0.0f );
char		acOutputFilename[256];

	mpLandscapePhysicsObject = PhysicsObjectCreateNew();
	pPhysicsObject = mpLandscapePhysicsObject;
	pTerrainShape = new btBvhTriangleMeshShape(&mHeightfieldTriMesh, false);
	pPhysicsObject->mpCollisionShape = pTerrainShape;

	btTriangleInfoMap* pTriangleInfoMap = new btTriangleInfoMap();
	//now you can adjust some thresholds in triangleInfoMap  if needed.
	//btGenerateInternalEdgeInfo fills in the btTriangleInfoMap and stores it as a user pointer of trimeshShape (trimeshShape->setUserPointer(triangleInfoMap))
	btGenerateInternalEdgeInfo(pTerrainShape,pTriangleInfoMap);

	if ( mszPhysicsLandscapeOutputFilename )
	{
	btDefaultSerializer* pSerializer = new btDefaultSerializer();
	FILE* file;

		// start the serialization and serialize the trimeshShape
		pSerializer->startSerialization();
		pTerrainShape->serializeSingleShape(pSerializer);
		pSerializer->finishSerialization();

		strcpy( acOutputFilename, mszPhysicsLandscapeOutputFilename );
		strcat( acOutputFilename, ".bvh" );
		// create a file and write the serialized content to file
		file = fopen(	acOutputFilename,"wb");
		fwrite(pSerializer->getBufferPointer(),pSerializer->getCurrentBufferSize(),1, file);
		fclose(file);

		delete pSerializer;
	}

	pGroundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), vecHeightfieldPos));
	btRigidBody::btRigidBodyConstructionInfo   groundRigidBodyCI(0, pGroundMotionState, pPhysicsObject->mpCollisionShape, btVector3(0, 0, 0));
	groundRigidBodyCI.m_restitution = 0.1f;

	pPhysicsObject->mpRigidBody = new btRigidBody(groundRigidBodyCI);

	int		f = pPhysicsObject->mpRigidBody->getCollisionFlags();
	pPhysicsObject->mpRigidBody->setCollisionFlags( f | btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK );
//	pPhysicsObject->mpRigidBody->setCollisionFlags( f | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK );

	mpDynamicsWorld->addRigidBody( pPhysicsObject->mpRigidBody, COLLISION_FILTER_LANDSCAPE, COLLISION_FILTER_LOCAL_VEHICLE | COLLISION_FILTER_REMOTE_VEHICLE | COLLISION_FILTER_REMOTE_PREDICTION_VEHICLE |	COLLISION_FILTER_WEAPON_RAY | COLLISION_FILTER_PHYSICS_OBJECTS );


}

long WINAPI PhysicsLoadCachedBVHThread(long lParam)
{ 
PhysicsObject*		pPhysicsObject;
btBulletWorldImporter import(0);//don't store info into the world
char		acFilename[256];
btBvhTriangleMeshShape*		pTerrainShape;

	mboLandscapeCacheLoadDidError = FALSE;
	strcpy( acFilename, mszPhysicsLandscapeOutputFilename );
	strcat( acFilename, ".bvh" );
	
	if (import.loadFile(acFilename))
	{
	int numShp = import.getNumCollisionShapes();
			
		if ( numShp == 1 )
		{
		btDefaultMotionState* pGroundMotionState;
		btVector3	vecHeightfieldPos( 0.0f, 0.0f, 0.0f );

			pTerrainShape = (btBvhTriangleMeshShape*)import.getCollisionShapeByIndex(0);

			mpLandscapePhysicsObject = PhysicsObjectCreateNew();
			pPhysicsObject = mpLandscapePhysicsObject;
			pPhysicsObject->mpCollisionShape = pTerrainShape;

			pGroundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), vecHeightfieldPos));
			btRigidBody::btRigidBodyConstructionInfo   groundRigidBodyCI(0, pGroundMotionState, pPhysicsObject->mpCollisionShape, btVector3(0, 0, 0));
			groundRigidBodyCI.m_restitution = 0.1f;

			pPhysicsObject->mpRigidBody = new btRigidBody(groundRigidBodyCI);

			int		f = pPhysicsObject->mpRigidBody->getCollisionFlags();
			pPhysicsObject->mpRigidBody->setCollisionFlags( f | btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK );
			//	pPhysicsObject->mpRigidBody->setCollisionFlags( f | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK );

			mpDynamicsWorld->addRigidBody( pPhysicsObject->mpRigidBody, COLLISION_FILTER_LANDSCAPE, COLLISION_FILTER_LOCAL_VEHICLE | COLLISION_FILTER_REMOTE_VEHICLE | COLLISION_FILTER_REMOTE_PREDICTION_VEHICLE |	COLLISION_FILTER_WEAPON_RAY | COLLISION_FILTER_PHYSICS_OBJECTS );

			mhPhysicsBVHGenerationThread = 0;
			SysExitThread( 0 );
			return( 0 );
		}
	}

	mboLandscapeCacheLoadDidError = TRUE;

	mhPhysicsBVHGenerationThread = 0;
	SysExitThread( 0 );
	return( 0 );
}




long WINAPI PhysicsGenerateBVHThread(long lParam)
{ 
	PhysicsGenerateLandscapeBVH();

	mhPhysicsBVHGenerationThread = 0;
	SysExitThread( 0 );
	return( 0 );
}


void		PhysicsBeginLoadCachedLandscape(  const char* szFilename )
{
	mboLandscapeCacheLoadDidError = FALSE;
	if ( szFilename )
	{
		mszPhysicsLandscapeOutputFilename = (char*)( SystemMalloc( strlen( szFilename ) + 1 ) );
		strcpy( mszPhysicsLandscapeOutputFilename, szFilename );
	}

	mhPhysicsBVHGenerationThread = SysCreateThread( (fnThreadFunction)PhysicsLoadCachedBVHThread, (void*)NULL, 0, 0 );

}


void		PhysicsBeginGenerateLandscapeBVH( int nHeightfieldWidth, int nHeightfieldHeight, float* pfHeightfield, float fGridScale, float fHeightScale, const char* szFilename )
{
	mboLandscapeCacheLoadDidError = FALSE;
	PhysicsGenerateLandscapeTrimesh( nHeightfieldWidth, nHeightfieldHeight, pfHeightfield, fGridScale, fHeightScale );

	if ( szFilename )
	{
		mszPhysicsLandscapeOutputFilename = (char*)( SystemMalloc( strlen( szFilename ) + 1 ) );
		strcpy( mszPhysicsLandscapeOutputFilename, szFilename );
	}

	mhPhysicsBVHGenerationThread = SysCreateThread( (fnThreadFunction)PhysicsGenerateBVHThread, (void*)NULL, 0, 0 );

} 

BOOL		PhysicsLandscapeGenerationInProgress( void )
{
	if ( mhPhysicsBVHGenerationThread != 0 )
	{
		return( TRUE );
	}
	return( FALSE );
}

BOOL		PhysicsLandscapeGenerationComplete( void )
{
	if ( ( mhPhysicsBVHGenerationThread == 0 ) &&
		 ( mpLandscapePhysicsObject != NULL ) )
	{
		return( TRUE );
	}
	return( FALSE );
}

extern void		PhysicsLandscapeGenerationCancel( void );
