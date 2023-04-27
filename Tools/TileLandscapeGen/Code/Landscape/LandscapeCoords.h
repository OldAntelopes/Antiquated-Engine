#ifndef LANDSCAPE_COORDS_H
#define LANDSCAPE_COORDS_H


typedef struct 
{
	VECT	 position; // The position
	VECT	 normal; // The position
	uint32    color;    // The color
	FLOAT    tu, tv;   // The texture coordinates
	FLOAT    tu2, tv2;   // The texture coordinates
	VECT	tangent;

} LANDRENDERVERTEX;

extern void		LandscapeGetMapCoord( const VECT* pxPos, int* pnMapX, int* pnMapY );

extern inline VECT		LandscapeGetWorldPos( int nMapX, int nMapY );
extern inline VECT		LandscapeGetNormal( int nMapX, int nMapY );

extern VECT		LandscapeCalculateNormal( int nMapX, int nMapY );

extern float	LandscapeMapToWorldScale( void );
extern float	LandscapeWorldToMapScale( void );

extern void	LandscapeCoordsInit( int nMapSizeX, int nMapSizeY );

extern int		LandscapeGetMapSizeX( void );
extern int		LandscapeGetMapSizeY( void );

extern const inline LANDRENDERVERTEX*		LandscapeGetRenderVertex( int nMapX, int nMapY );

#endif