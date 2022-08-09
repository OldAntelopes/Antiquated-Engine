
#ifndef PATH_FINDING_H
#define	PATH_FINDING_H


extern void	PathDataLoad( void );
extern void PathDataFree( void );

extern BYTE*	PathDataFindPath( float fSourceWorldX, float fSourceWorldY, float fDestWorldX, float fDestWorldY, int* pnNumSteps, int* pnSourcePathMapX, int* pnSourcePathMapY );

extern float		PathDataGetGameWorldX( int nPathMapX );
extern float		PathDataGetGameWorldY( int nPathMapY );

extern void		PathDataGetNextWaypoint( int nCurrentPathMapX, int nCurrentPathMapY, int nDirection, int* pnNextPathMapX, int* pnNextPathMapY );

#endif
