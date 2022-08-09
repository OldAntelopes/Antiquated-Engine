#ifndef LANDSCAPE_CHECKERBOARD_TEXTURES_H
#define LANDSCAPE_CHECKERBOARD_TEXTURES_H


extern void		LandscapeCheckerboardTexturesInit( void );

extern void		LandscapeCheckerboardTexturesNewFrame( void );

extern void		LandscapeCheckerboardTexturesShutdown( void );

// ------------ Gets the LandscapeTextureNum for a tile
extern int		LandscapeCheckerboardTextureGetTextureNum( int nMapX, int nMapY );

// ------------ Sets the appropriate texture for the current landscape texture #
extern void		LandscapeCheckerboardTextureApply( int nLandscapeTextureNum, int nMapX, int nMapY );

// ------------ Get UV coords for a tile
extern void		LandscapeCheckerboardTextureGetUVs( int nLandscapeTextureNum, int nMapX, int nMapY, float* pfUBase, float* pfVBase, float* pfUWidth, float* pfVWidth );




#endif