#ifndef PLANE_CUTTER_H
#define PLANE_CUTTER_H

class CSceneObject;



extern void		PlaneCutterInit( void );

extern void		PlaneCutterRender( void );

extern BOOL		PlaneCutterIsActive( void );
extern void		PlaneCutterActivate( BOOL bFlag );


extern void		PlaneCutterInitGraphics( void );
extern void		PlaneCutterFreeGraphics( void );

extern void		PlaneCutterRightMouseDownMove( float fScreenXDelta, float fScreenYDelta );
extern void		PlaneCutterLeftMouseDownMove( CSceneObject* pSceneObject, float fScreenXDelta, float fScreenYDelta );

extern void		PlaneCutterMouseDownStore( void );

extern void 	PlaneCutterSlice( CSceneObject* pSceneObject );

extern void		PlaneCutterResetPlane( void );
extern void		PlaneCutterAxisSnap( void );


#endif