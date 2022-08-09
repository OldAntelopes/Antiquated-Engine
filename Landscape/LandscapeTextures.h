#ifndef LANDSCAPE_TEXTURES_H
#define LANDSCAPE_TEXTURES_H

#define		LARGEST_LANDSCAPE_TEXTURE_NUM		512

enum
{
	LANDSCAPE_TEXTURETYPE_MAINSURF,
	LANDSCAPE_TEXTURETYPE_TILE,
	LANDSCAPE_TEXTURETYPE_TEXMAP,
};


extern void		LandscapeTexturesInit( int );

extern void		LandscapeTexturesNewFrame( void );

extern void		LandscapeTexturesShutdown( void );

// ------------ Gets the LandscapeTextureNum for a tile
extern int		LandscapeTextureGetTextureNum( int nMapX, int nMapY );

// ------------ Sets the appropriate texture for the current landscape texture #
extern void		LandscapeTextureApply( int nLandscapeTextureNum, int nMapX, int nMapY );

// ------------ Get UV coords for a tile
extern void		LandscapeTextureGetUVs( int nLandscapeTextureNum, int nMapX, int nMapY, float* pfUBase, float* pfVBase, float* pfUWidth, float* pfVWidth );

// ------------ Set texture 
extern void		LandscapeSetTextureResource( int nTextureType, int nTextureTypeIndex, int hTexture );

#endif