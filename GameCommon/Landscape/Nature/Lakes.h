#ifndef NATURE_LAKES_H
#define NATURE_LAKES_H


extern void		LakesInit( void );

extern void		LakesUpdate( float fDelta );

extern void		LakesAddTile( int nMapX, int nMapY );

extern void		LakesRender( void );

extern void		LakesFree( void );

//------------------------------------------

extern void		LakesAddToScene( VECT* pxOrigin, float fRadius );




#endif
