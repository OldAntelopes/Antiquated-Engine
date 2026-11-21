#ifndef PHYSICS_OBJECTS_H
#define PHYSICS_OBJECTS_H

#include "Engine.h"

#ifdef __cplusplus
extern "C"
{
#endif


//-------------------------------------------------

extern int		PhysicsObjectAddPrimitive( int type, const VECT* pxPos, const VECT* pxRot, const VECT* pxSize, float fMass, float fLinearFriction );

extern int		PhysicsObjectAddMesh( int nModelHandle, VECT* pxPos, VECT* pxRot, float fMass );

extern int		PhysicsObjectAddConvexHull( VECT* pxPointsList, int nNumPoints, float fMass );

// A static, concave triangle mesh of a model as placed in the world: its collision sub-model if it has one (else the
// drawn mesh), turned fZRot radians about z. For buildings and scenery a vehicle drives over, into and through
// (ramps, arches). NOTFOUND if there's no physics world or the model has no triangles
extern int		PhysicsObjectAddStaticModelMesh( int nModelHandle, const VECT* pxPos, float fZRot );

// A static box that vehicles collide with (scenery: walls, fences). pxCentre is its middle, pxSize its full size
// before it's turned fZRot radians about z
extern int		PhysicsObjectAddStaticBox( const VECT* pxCentre, const VECT* pxSize, float fZRot );

//----------------------------------------------------------

extern void		PhysicsObjectRemove( int hPhysicsObject );

//----------------------------------------------------------

extern void		PhysicsBeginLoadCachedLandscape(  const char* szFilename );
extern void		PhysicsBeginGenerateLandscapeBVH( int nHeightfieldWidth, int nHeightfieldHeight, float* pfHeightfield, float fGridScale, float fHeightScale, const char* szFilename );
extern BOOL		PhysicsLandscapeGenerationInProgress( void );
extern BOOL		PhysicsLandscapeGenerationComplete( void );
extern void		PhysicsLandscapeGenerationCancel( void );
extern BOOL		PhysicsLandscapeCacheLoadDidError( void );

//----------------------------------------------------------

extern int		PhysicsObjectAddHeightfield( int nHeightfieldWidth, int nHeightfieldHeight, float* pfHeightfield, float fGridScale, float fHeightScale );


#ifdef __cplusplus
}
#endif


#ifdef __cplusplus

class PhysicsObjectTransform
{
public:
	VECT				mxPosition;
	ENGINEQUATERNION	mxQuatRotation;
};

extern BOOL		PhysicsObjectGetTransform( int hPhysicsObject, PhysicsObjectTransform* pxOut );
extern BOOL		PhysicsObjectGetLinearVelocity( int hPhysicsObject, VECT* pxOut );

extern BOOL		PhysicsObjectSetVelocity( int hPhysicsObject, const VECT* pxVel );

extern BOOL		PhysicsObjectOverridePositionAndOrientation( int hPhysicsObject, const VECT* pxPos, const ENGINEQUATERNION* pxOrientation );

extern BOOL		PhysicsObjectApplyLinearForce( int hPhysicsObject, const VECT* pxForce );
extern BOOL		PhysicsObjectApplyAngularForce( int hPhysicsObject, const VECT* pxForce );

#endif
#endif
