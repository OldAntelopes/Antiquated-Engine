#ifndef LANDSCAPE_COORDS_H
#define LANDSCAPE_COORDS_H

#ifdef __cplusplus
extern "C"
{
#endif

/*
#ifndef INLINE
#ifdef __GNUC__			// for instance, GNU C knows about inline 
#define INLINE __inline__
#endif
#ifndef INLINE
#define INLINE
#endif
#endif
*/
#ifndef INLINE
#define INLINE
#endif

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

extern VECT		LandscapeGetWorldPos( int nMapX, int nMapY );
extern VECT		LandscapeGetNormal( int nMapX, int nMapY );

extern VECT		LandscapeCalculateNormal( int nMapX, int nMapY );

extern float	LandscapeMapToWorldScale( void );
extern float	LandscapeWorldToMapScale( void );

extern int		LandscapeGetMapSizeX( void );
extern int		LandscapeGetMapSizeY( void );

extern void		LandscapeCoordsInit( int nMapSizeX, int nMapSizeY );
extern void		LandscapeSetTileSize( float fTileSize );

extern void		LandscapeCoordsShutdown( void );

const INLINE LANDRENDERVERTEX*		LandscapeGetRenderVertex( int nMapX, int nMapY );
extern void LandscapeCoordsUpdateVertexData( int nMapX, int nMapY );


#ifdef __cplusplus
}
#endif

#endif