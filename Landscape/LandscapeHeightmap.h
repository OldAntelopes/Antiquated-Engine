
#ifndef LANDSCAPE_HEIGHTMAP_H
#define LANDSCAPE_HEIGHTMAP_H


extern BOOL		LandscapeHeightmapInit( int nMapSizeX, int nMapSizeY );
extern void		LandscapeHeightmapFree( void );

extern BOOL		LandscapeHeightmapProcessHeightmapTexture( int hTexture, BOOL bUseTextureSizeForHeightmap );
extern void		LandscapeHeightmapAddToPhysicsWorld( void );

extern float	LandscapeHeightmapGetHeight( int nMapX, int nMapY );
extern float	LandscapeHeightmapGetHeightWorld( float fWorldX, float fWorldY );

extern int		LandscapeHeightmapGetSizeX( void );
extern int		LandscapeHeightmapGetSizeY( void );

extern float	LandscapeHeightmapGetHeightScale( void );
extern void		LandscapeHeightmapSetHeightScale( float fHeightScale );

extern void		LandscapeHeightmapSetHeight( int nMapX, int nMapY, float fHeightInWorldSpace );


#endif