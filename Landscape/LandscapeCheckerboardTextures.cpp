
#include <stdio.h>
#include "StandardDef.h"
#include "Engine.h"

#include "Landscape.h"
#include "LandscapeTextures.h"
#include "LandscapeCheckerboardTextures.h"

int		mahLandscapeCheckerboardTextures[2] = { NOTFOUND, NOTFOUND };

void		LandscapeCheckerboardTexturesInit( void )
{
	mahLandscapeCheckerboardTextures[0] = EngineLoadTexture( "Data\\Textures\\Land\\CheckerboardDark.png", 0, NULL );
	mahLandscapeCheckerboardTextures[1] = EngineLoadTexture( "Data\\Textures\\Land\\CheckerboardLight.png", 0, NULL );

}

void		LandscapeCheckerboardTexturesNewFrame( void )
{

}

void		LandscapeCheckerboardTexturesShutdown( void )
{

}

// ------------ Gets the LandscapeTextureNum for a tile
int		LandscapeCheckerboardTextureGetTextureNum( int nMapX, int nMapY )
{
	return( ((nMapX + nMapY) & 1)  );

}

// ------------ Sets the appropriate texture for the current landscape texture #
void		LandscapeCheckerboardTextureApply( int nLandscapeTextureNum, int nMapX, int nMapY )
{
	EngineSetTexture( 0, mahLandscapeCheckerboardTextures[ nLandscapeTextureNum ] );

}

// ------------ Get UV coords for a tile
void		LandscapeCheckerboardTextureGetUVs( int nLandscapeTextureNum, int nMapX, int nMapY, float* pfUBase, float* pfVBase, float* pfUWidth, float* pfVWidth )
{
	*pfUBase = 0.0f;
	*pfVBase = 0.0f;
	*pfUWidth = 1.0f;
	*pfVWidth = 1.0f;
}

