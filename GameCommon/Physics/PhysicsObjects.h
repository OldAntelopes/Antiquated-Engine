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
