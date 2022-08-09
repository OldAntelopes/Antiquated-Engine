
#ifndef LANDSCAPE_HEIGHTMAP_H
#define LANDSCAPE_HEIGHTMAP_H


extern BOOL LandscapeHeightmapInit( int nMapSizeX, int nMapSizeY );
extern void LandscapeHeightmapFree( void );


extern BOOL LandscapeHeightmapProcessHeightmapTexture( int hTexture, BOOL bUseTextureSizeForHeightmap );

extern float	LandscapeHeightmapGetHeight( int nMapX, int nMapY );
extern float	LandscapeHeightmapGetHeightWorld( float fWorldX, float fWorldY );

extern int		LandscapeHeightmapGetSizeX( void );
extern int		LandscapeHeightmapGetSizeY( void );

extern float	LandscapeHeightmapGetHeightScale( void );

#endif