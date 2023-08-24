
#include "StandardDef.h"
#include "Interface.h"
#include "Engine.h"

#include "../RenderUtil/Lines3D.h"

#include "PhysicsInternal.h"
#include "PhysicsWorld.h"
#include "BulletCollision/CollisionShapes/btTriangleShape.h"

//#define USE_2D_INTERFACE_FOR_DEBUG_LINES

class PhysicsInterfaceDebugDraw : public btIDebugDraw
{
public:
	void	drawLine(const btVector3& from,const btVector3& to,const btVector3& color)
	{
	VECT	xFrom, xTo;

		xFrom.x = from.getX();
		xFrom.y = from.getY();
		xFrom.z = from.getZ();

		xTo.x = to.getX();
		xTo.y = to.getY();
		xTo.z = to.getZ();

#ifdef USE_2D_INTERFACE_FOR_DEBUG_LINES
		int		nScreenX1, nScreenY1;
		int		nScreenX2, nScreenY2;

		EngineGetScreenCoordForWorldCoord( &xFrom, &nScreenX1, &nScreenY1 );
		EngineGetScreenCoordForWorldCoord( &xTo, &nScreenX2, &nScreenY2 );

		if ( ( nScreenX1 > 0 ) &&
			 ( nScreenX2 > 0 ) &&
			 ( nScreenY1 > 0 ) &&
			 ( nScreenY2 > 0 ) )
		{
		uint32	ulCol;

			ulCol = 0xFF000000 | ( (uint32)(color.getX() * 0xFF) << 16) | ((uint32)( color.getY() * 0xFF) << 8) | ((uint32)( color.getZ() * 0xFF));
			InterfaceLine( 0, nScreenX1, nScreenY1, nScreenX2, nScreenY2, ulCol, ulCol );
		}
#else
		float	fCamDist;
		VECT*	pxCamPos = EngineCameraGetPos();

		fCamDist = VectDistNoZ( &xFrom, pxCamPos );
	
		if ( fCamDist < 250.0f )
		{
		uint32	ulCol;
			ulCol = 0xFF000000 | ( (uint32)(color.getX() * 0xFF) << 16) | ((uint32)( color.getY() * 0xFF) << 8) | ((uint32)( color.getZ() * 0xFF));
			Lines3DAddLine( &xFrom, &xTo, ulCol, ulCol );
		}
#endif
	}

	void	drawContactPoint(const btVector3& PointOnB,const btVector3& normalOnB,btScalar distance,int lifeTime,const btVector3& color)
	{
	VECT	xFrom, xTo;
	int		nScreenX1, nScreenY1;
	int		nScreenX2, nScreenY2;

		xFrom.x = PointOnB.getX();
		xFrom.y = PointOnB.getY();
		xFrom.z = PointOnB.getZ();

		xTo.x = normalOnB.getX() * 10.0f;
		xTo.y = normalOnB.getY() * 10.0f;
		xTo.z = normalOnB.getZ() * 10.0f;

		VectAdd( &xTo, &xTo, &xFrom );

		EngineGetScreenCoordForWorldCoord( &xFrom, &nScreenX1, &nScreenY1 );
		EngineGetScreenCoordForWorldCoord( &xTo, &nScreenX2, &nScreenY2 );

		if ( ( nScreenX1 > 0 ) &&
			 ( nScreenX2 > 0 ) &&
			 ( nScreenY1 > 0 ) &&
			 ( nScreenY2 > 0 ) )
		{
		uint32	ulCol;

			ulCol = 0xFF000000 | ( (uint32)(color.getX() * 0xFF) << 16) | ((uint32)( color.getY() * 0xFF) << 8) | ((uint32)( color.getZ() * 0xFF));
			InterfaceLine( 0, nScreenX1, nScreenY1, nScreenX2, nScreenY2, ulCol, ulCol );
		}

	}

	void	reportErrorWarning(const char* warningString)
	{
		SysDebugPrint( warningString );
	}

	void	draw3dText(const btVector3& location,const char* textString)
	{
	int		nScreenX, nScreenY;
	VECT	xFrom;

		xFrom.x = location.getX();
		xFrom.y = location.getY();
		xFrom.z = location.getZ();
		EngineGetScreenCoordForWorldCoord( &xFrom, &nScreenX, &nScreenY );
		InterfaceText( 0, nScreenX, nScreenY, textString, 0xd0d0d0d, 3 );
	}
	
	void	setDebugMode(int debugMode)
	{
		mnDebugMode = debugMode;
	}
	
	int		getDebugMode() const
	{
		return( mnDebugMode );
	}

	int		mnDebugMode;
};


PhysicsInterfaceDebugDraw		msPhysicsInterfaceDebugDraw;

btBroadphaseInterface*					mpBroadphase = NULL;
btDefaultCollisionConfiguration*		mpCollisionConfiguration = NULL;
btCollisionDispatcher*					mpDispatcher = NULL;
btSequentialImpulseConstraintSolver*	mpSolver = NULL;
btDiscreteDynamicsWorld*				mpDynamicsWorld = NULL;
BOOL				msbPhysicsWorldEnabled = TRUE;


//#define PHYSICS_DEBUG_DRAW
BOOL		msbPhysicsDebugDraw = FALSE;
BOOL		msbPhysicsDebugHasInitialised = FALSE;

static bool myCustomMaterialCombinerCallback(
    btManifoldPoint& cp,
    const btCollisionObjectWrapper* colObj0Wrap,
    int partId0,
    int index0,
    const btCollisionObjectWrapper* colObj1Wrap,
    int partId1,
    int index1
    )
{
    // one-sided triangles
    if (colObj1Wrap->getCollisionShape()->getShapeType() == TRIANGLE_SHAPE_PROXYTYPE)
    {

		btAdjustInternalEdgeContacts(cp,colObj1Wrap,colObj0Wrap, partId1,index1);

		if ( cp.m_normalWorldOnB.getZ() < 0.0f )
		{
            cp.m_normalWorldOnB *= -1.0f;
		}


/*
        const btTriangleShape* triShape = static_cast<const btTriangleShape*>( colObj1Wrap->getCollisionShape() );
        const btVector3* v = triShape->m_vertices1;
        btVector3 faceNormalLs = btCross(v[1] - v[0], v[2] - v[0]);
        faceNormalLs.normalize();
        btVector3 faceNormalWs = colObj1Wrap->getWorldTransform().getBasis() * faceNormalLs;
        float nDotF = btDot( faceNormalWs, cp.m_normalWorldOnB );
        if ( nDotF <= 0.0f )
        {
            // flip the contact normal to be aligned with the face normal
            cp.m_normalWorldOnB += -2.0f * nDotF * faceNormalWs;
        }		
		else
		{
			cp.m_normalWorldOnB = faceNormalWs;
		}
*/
    }

    //this return value is currently ignored, but to be on the safe side: return false if you don't calculate friction
    return false;
}

BOOL		PhysicsWorldIsEnabled( void )
{
	return( msbPhysicsWorldEnabled );
}

void		PhysicsWorldSetEnabled( BOOL bFlag )
{
	msbPhysicsWorldEnabled = bFlag;
}

BOOL		PhysicsWorldIsDebugRenderEnabled( void )
{
	return( msbPhysicsDebugDraw );
}


void		PhysicsWorldEnableDebugRender( BOOL bFlag )
{
	msbPhysicsDebugDraw = bFlag;

	if ( bFlag )
	{
		if ( msbPhysicsDebugHasInitialised == FALSE )
		{
			Lines3DInitialise( 65536 * 16 );

			if ( mpDynamicsWorld )
			{
				mpDynamicsWorld->setDebugDrawer( &msPhysicsInterfaceDebugDraw );
//				mpDynamicsWorld->getDebugDrawer()->setDebugMode( btIDebugDraw::DBG_DrawAabb | btIDebugDraw::DBG_ProfileTimings | btIDebugDraw::DBG_DrawNormals | btIDebugDraw::DBG_DrawFeaturesText | btIDebugDraw::DBG_DrawContactPoints | btIDebugDraw::DBG_DrawText  );
//				mpDynamicsWorld->getDebugDrawer()->setDebugMode(  );
				mpDynamicsWorld->getDebugDrawer()->setDebugMode( btIDebugDraw::DBG_DrawWireframe | btIDebugDraw::DBG_FastWireframe );
			}
			msbPhysicsDebugHasInitialised = TRUE;
		}
	}
	else
	{
		if ( mpDynamicsWorld->getDebugDrawer() )
		{
			mpDynamicsWorld->getDebugDrawer()->setDebugMode( btIDebugDraw::DBG_NoDebug );
			mpDynamicsWorld->setDebugDrawer( NULL );
		}
		msbPhysicsDebugHasInitialised = FALSE;
	}
}

void		PhysicsWorldInit( void )
{
	gContactAddedCallback = myCustomMaterialCombinerCallback;

	mpBroadphase = new btDbvtBroadphase();

	mpCollisionConfiguration = new btDefaultCollisionConfiguration();
	mpDispatcher = new btCollisionDispatcher(mpCollisionConfiguration);

	mpSolver = new btSequentialImpulseConstraintSolver;

	mpDynamicsWorld = new btDiscreteDynamicsWorld(mpDispatcher, mpBroadphase, mpSolver, mpCollisionConfiguration);
	mpDynamicsWorld->setGravity(btVector3(0, 0, -9.8f));

	if ( msbPhysicsDebugDraw == TRUE )
	{
		Lines3DInitialise( 65536 * 16 );

		mpDynamicsWorld->setDebugDrawer( &msPhysicsInterfaceDebugDraw );
//		mpDynamicsWorld->getDebugDrawer()->setDebugMode( btIDebugDraw::DBG_DrawWireframe );
		mpDynamicsWorld->getDebugDrawer()->setDebugMode( btIDebugDraw::DBG_DrawAabb | btIDebugDraw::DBG_ProfileTimings | btIDebugDraw::DBG_DrawNormals | btIDebugDraw::DBG_DrawFeaturesText | btIDebugDraw::DBG_DrawContactPoints | btIDebugDraw::DBG_DrawText  );

//		DBG_FastWireframe = (1<<13),
	
	}
}

/*
        btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0, 1, 0), 1);

        btCollisionShape* fallShape = new btSphereShape(1);


        btDefaultMotionState* groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, -1, 0)));
        btRigidBody::btRigidBodyConstructionInfo
                groundRigidBodyCI(0, groundMotionState, groundShape, btVector3(0, 0, 0));
        btRigidBody* groundRigidBody = new btRigidBody(groundRigidBodyCI);
        dynamicsWorld->addRigidBody(groundRigidBody);


        btDefaultMotionState* fallMotionState =
                new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 50, 0)));
        btScalar mass = 1;
        btVector3 fallInertia(0, 0, 0);
        fallShape->calculateLocalInertia(mass, fallInertia);
        btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass, fallMotionState, fallShape, fallInertia);
        btRigidBody* fallRigidBody = new btRigidBody(fallRigidBodyCI);
        dynamicsWorld->addRigidBody(fallRigidBody);
*/


void		PhysicsWorldUpdate( float fDelta )
{
	if ( mpDynamicsWorld )
	{
		mpDynamicsWorld->stepSimulation( fDelta, 10);
	}

}

BOOL		PhysicsWorldRayTest( const VECT* pxRayStart, const VECT* pxRayEnd, VECT* pxCollisionPoint )
{
btVector3	vecRayStart( pxRayStart->x, pxRayStart->y, pxRayStart->z );
btVector3	vecRayEnd( pxRayEnd->x, pxRayEnd->y, pxRayEnd->z );
btCollisionWorld::ClosestRayResultCallback rayCallback(vecRayStart, vecRayEnd);

	rayCallback.m_collisionFilterMask = COLLISION_FILTER_LANDSCAPE | COLLISION_FILTER_STATIC_OBJECT | COLLISION_FILTER_PHYSICS_OBJECTS;
	rayCallback.m_collisionFilterGroup = COLLISION_FILTER_WEAPON_RAY;
	mpDynamicsWorld->rayTest( vecRayStart, vecRayEnd, rayCallback );

	if ( rayCallback.hasHit() )
	{
		pxCollisionPoint->x = rayCallback.m_hitPointWorld.getX();
		pxCollisionPoint->y = rayCallback.m_hitPointWorld.getY();
		pxCollisionPoint->z = rayCallback.m_hitPointWorld.getZ();
		return( TRUE );
	}
	return( FALSE );
}


void		PhysicsWorldRender( void )
{

	if ( msbPhysicsDebugDraw == TRUE )
	{
		Lines3DFlush( NULL );

		if ( mpDynamicsWorld )
		{
			mpDynamicsWorld->debugDrawWorld();
		}

		Lines3DFlush( NULL );
	}
}

void		PhysicsWorldShutdown( void )
{
	if ( msbPhysicsDebugDraw == TRUE )
	{
		Lines3DShutdown();
	}

	SAFE_DELETE( mpDynamicsWorld );
	SAFE_DELETE( mpSolver );
	SAFE_DELETE( mpCollisionConfiguration );
	SAFE_DELETE( mpDispatcher );
	SAFE_DELETE( mpBroadphase );

}




//--------------------------------------------------------------------------------


/*

void	PhysicsTest(void)
{
        btBroadphaseInterface* broadphase = new btDbvtBroadphase();

        btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
        btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);

        btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;

        btDiscreteDynamicsWorld* dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);

        dynamicsWorld->setGravity(btVector3(0, -10, 0));


        btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0, 1, 0), 1);

        btCollisionShape* fallShape = new btSphereShape(1);


        btDefaultMotionState* groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, -1, 0)));
        btRigidBody::btRigidBodyConstructionInfo
                groundRigidBodyCI(0, groundMotionState, groundShape, btVector3(0, 0, 0));
        btRigidBody* groundRigidBody = new btRigidBody(groundRigidBodyCI);
        dynamicsWorld->addRigidBody(groundRigidBody);


        btDefaultMotionState* fallMotionState =
                new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 50, 0)));
        btScalar mass = 1;
        btVector3 fallInertia(0, 0, 0);
        fallShape->calculateLocalInertia(mass, fallInertia);
        btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass, fallMotionState, fallShape, fallInertia);
        btRigidBody* fallRigidBody = new btRigidBody(fallRigidBodyCI);
        dynamicsWorld->addRigidBody(fallRigidBody);


        for (int i = 0; i < 300; i++) 
		{
                dynamicsWorld->stepSimulation(1 / 60.f, 10);

                btTransform trans;
                fallRigidBody->getMotionState()->getWorldTransform(trans);

                std::cout << "sphere height: " << trans.getOrigin().getY() << std::endl;
        }

        dynamicsWorld->removeRigidBody(fallRigidBody);
        delete fallRigidBody->getMotionState();
        delete fallRigidBody;

        dynamicsWorld->removeRigidBody(groundRigidBody);
        delete groundRigidBody->getMotionState();
        delete groundRigidBody;


        delete fallShape;

        delete groundShape;


        delete dynamicsWorld;
        delete solver;
        delete collisionConfiguration;
        delete dispatcher;
        delete broadphase;

}
*/