#ifndef GAMECOMMON_PHYSICS_WORLD_H
#define GAMECOMMON_PHYSICS_WORLD_H

#ifdef __cplusplus
extern "C"
{
#endif

enum
{
	COLLISION_FILTER_LANDSCAPE = 0x1,
	COLLISION_FILTER_STATIC_OBJECT = 0x2,
	COLLISION_FILTER_LOCAL_VEHICLE = 0x4,
	COLLISION_FILTER_REMOTE_VEHICLE = 0x8,
	COLLISION_FILTER_REMOTE_PREDICTION_VEHICLE = 0x10,
	COLLISION_FILTER_WEAPON_RAY = 0x20,
	COLLISION_FILTER_PHYSICS_OBJECTS = 0x40,
};


extern void		PhysicsWorldInit( void );

extern void		PhysicsWorldUpdate( float fDelta );

extern void		PhysicsWorldRender( void );

extern BOOL		PhysicsWorldRayTest( const VECT* pxRayStart, const VECT* pxRayEnd, VECT* pxCollisionPoint );


extern void		PhysicsWorldShutdown( void );

extern void		PhysicsWorldEnableDebugRender( BOOL bFlag );
extern BOOL		PhysicsWorldIsDebugRenderEnabled( void );

#ifdef __cplusplus
}
#endif

#endif
