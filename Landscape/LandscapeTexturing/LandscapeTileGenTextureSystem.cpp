
#include <stdio.h>
#include "StandardDef.h"
#include "Engine.h"

#include "../Landscape.h"
#include "../LandscapeHeightmap.h"
#include "../LandscapeTextures.h"

#include "LandscapeTileGenTextureSystem.h"

typedef struct
{
	int		nTileTextureLookupNum;
	int		hTextureHandle;
	ulong	ulLastTouched;

} HIRES_LANDSCAPE_TEXTURES;


//----------------

int			maLoResLandscapeTextures[ NUM_LORES_TEXTURES ];
ulong		mulTextureTouchFrame = 100;
BOOL		mbLandscapeTexturesJpegs = TRUE;

HIRES_LANDSCAPE_TEXTURES		maHiResLandscapeTextures[ MAX_NUM_HIRES_LANDSCAPE_TEXTURES_LOADED ];

int		mahTileTextureLookUp[ TEXTURE_LOOKUP_SLOTS ] = { NOTFOUND };

int		LandscapeTileGenTexturesGetNextFreeHandle( void )
{
int		nLoop;
ulong	ulOldestFrame = mulTextureTouchFrame - 2;
int		nOldestIndex = NOTFOUND;
	
	for ( nLoop = 0; nLoop < MAX_NUM_HIRES_LANDSCAPE_TEXTURES_LOADED; nLoop++ )
	{
		if ( maHiResLandscapeTextures[nLoop].hTextureHandle == NOTFOUND )
		{
			return( nLoop );
		}
		else if ( maHiResLandscapeTextures[nLoop].ulLastTouched < ulOldestFrame )
		{
			ulOldestFrame = maHiResLandscapeTextures[nLoop].ulLastTouched;
			nOldestIndex = nLoop;
		}
	}

	// no empty ones.. find oldest touched
	if ( nOldestIndex != NOTFOUND )
	{
	int		nHeightmapWidth;
	int		nHeightmapHeight;
	int		nTileTexture;

		nHeightmapWidth = LandscapeHeightmapGetSizeX();
		nHeightmapHeight = LandscapeHeightmapGetSizeY();

		nLoop = nOldestIndex;
		EngineReleaseTexture( &maHiResLandscapeTextures[nLoop].hTextureHandle );

		nTileTexture = maHiResLandscapeTextures[nLoop].nTileTextureLookupNum;
		mahTileTextureLookUp[nTileTexture] = NOTFOUND;
//		maHiResLandscapeTextures[nLoop].hTextureHandle = NOTFOUND;
		return( nLoop );
	}

	return( NOTFOUND );
}

// ------------ Get the engine texture handle for a particular LandscapeTextureNum
int			LandscapeTileGenTextureGet( int nLandscapeTextureNum )
{
	return( maHiResLandscapeTextures[nLandscapeTextureNum].hTextureHandle );
}


//---------------------------------------------------------------------------------------------------------------


void		LandscapeTileGenTextureSystem::Initialise( void )
{
int			nLoop;
char		acString[256];

	for ( nLoop = 0; nLoop < MAX_NUM_HIRES_LANDSCAPE_TEXTURES_LOADED; nLoop++ )
	{
		maHiResLandscapeTextures[nLoop].hTextureHandle = NOTFOUND;
		maHiResLandscapeTextures[nLoop].ulLastTouched = 0;
	}

	for ( nLoop = 0; nLoop < TEXTURE_LOOKUP_SLOTS; nLoop++ )
	{
		mahTileTextureLookUp[nLoop] = NOTFOUND;
	}

	int		nCountX = 0;
	int		nCountY = 0;

	for ( nLoop = 0; nLoop < NUM_LORES_TEXTURES; nLoop++ )
	{
		if ( mbLandscapeTexturesJpegs )
		{
			sprintf( acString, "%s\\x4\\TileY%03dX%03d.jpg", LandscapeGetLevelRootPath(), nCountY, nCountX );
		}
		else
		{
			sprintf( acString, "%s\\x4\\TileY%03dX%03d.bmp", LandscapeGetLevelRootPath(), nCountY, nCountX );
		}
		nCountX++;
		if ( nCountX == 4 )
		{
			nCountX = 0;
			nCountY++;
		}
		maLoResLandscapeTextures[ nLoop ] = EngineLoadTexture( acString, 0, 0 );
	}
}


void		LandscapeTileGenTextureSystem::Shutdown( void )
{
	// todoo...!

}


void		LandscapeTileGenTextureSystem::NewFrame( void )
{
	mulTextureTouchFrame++;
}

void		LandscapeTileGenTextureSystem::ApplyTexture( int nLandscapeTextureNum, int nMapX, int nMapY )
{
int		nHeightmapSizeX;
int		nHeightmapSizeY;
int		nTileBaseX;
int		nTileBaseY;

	nHeightmapSizeX = LandscapeHeightmapGetSizeX( );
	nHeightmapSizeY = LandscapeHeightmapGetSizeY( );

	if ( ( nLandscapeTextureNum >= MAX_NUM_HIRES_LANDSCAPE_TEXTURES_LOADED ) ||
		 ( EngineTextureIsFullyLoaded( maHiResLandscapeTextures[nLandscapeTextureNum].hTextureHandle ) == FALSE ) )
	{
	int		nLoResTextureNum;

		nTileBaseX = (nMapX * 4) / nHeightmapSizeX;
		nTileBaseY = (nMapY * 4) / nHeightmapSizeY;
		nLoResTextureNum = (nTileBaseY * 4) + nTileBaseX;
		if ( nLoResTextureNum > 0 )
		{
			nLoResTextureNum %= 16;

			EngineSetTexture( 0, maLoResLandscapeTextures[nLoResTextureNum] );
		}
	}
	else
	{
		EngineSetTexture( 0, maHiResLandscapeTextures[nLandscapeTextureNum].hTextureHandle );
	}
}

void		LandscapeTileGenTextureSystem::GetUVs( int nLandscapeTextureNum, int nMapX, int nMapY, float* pfUBase, float* pfVBase, float* pfUWidth, float* pfVHeight )
{
int		nHeightmapSizeX;
int		nHeightmapSizeY;
int		nTileBaseX;
int		nTileBaseY;
int		nTextureMapSizeX;
int		nTextureMapSizeY;
int		nHiResTextureMapSize = 32;

	nHeightmapSizeX = LandscapeHeightmapGetSizeX( );
	nHeightmapSizeY = LandscapeHeightmapGetSizeY( );

	if ( ( nLandscapeTextureNum >= MAX_NUM_HIRES_LANDSCAPE_TEXTURES_LOADED ) ||
		 ( EngineTextureIsFullyLoaded( maHiResLandscapeTextures[nLandscapeTextureNum].hTextureHandle ) == FALSE ) )
	{
		nTextureMapSizeX = nHeightmapSizeX / 4;
		nTextureMapSizeY = nHeightmapSizeY / 4;
		nTileBaseX = nMapX % nTextureMapSizeX;
		nTileBaseY = nMapY % nTextureMapSizeY;
		*pfUBase = nTileBaseX * ( 1.0f / (float)( nTextureMapSizeX ) );
		*pfVBase = nTileBaseY * ( 1.0f / (float)( nTextureMapSizeY ) );
		*pfUWidth = *pfUBase + ( 1.0f / (float)( nTextureMapSizeX ) );
		*pfVHeight = *pfVBase + ( 1.0f / (float)( nTextureMapSizeY ) );
	}
	else
	{
		if ( nHeightmapSizeX > nHiResTextureMapSize )
		{
			nTextureMapSizeX = nHeightmapSizeX / nHiResTextureMapSize;
			nTextureMapSizeY = nHeightmapSizeY / nHiResTextureMapSize;
			nTileBaseX = nMapX % nTextureMapSizeX;
			nTileBaseY = nMapY % nTextureMapSizeY;

			*pfUBase = nTileBaseX * ( 1.0f / (float)( nTextureMapSizeX ) );
			*pfVBase = nTileBaseY * ( 1.0f / (float)( nTextureMapSizeY ) );
			*pfUWidth = *pfUBase + ( 1.0f / (float)( nTextureMapSizeX ) );
			*pfVHeight = *pfVBase + ( 1.0f / (float)( nTextureMapSizeY ) );

		}
	}
}

//#define		ALWAYS_LO_RES

int		LandscapeTileGenTextureSystem::GetTextureNum( int nMapX, int nMapY )
{
int		nHeightmapWidth;
int		nHeightmapHeight;
int		nTileTexture;
int		nHiResMapSize = 32;
int		nHiResMapping;
int		nHiResLandscapeTextureNum;
BOOL	bTextureIsReady = FALSE;

	if ( ( nMapY < 0 ) || ( nMapX < 0 ) )
	{
		return( 0 );
	}

	nHeightmapWidth = LandscapeHeightmapGetSizeX();
	nHeightmapHeight = LandscapeHeightmapGetSizeY();

#ifdef ALWAYS_LO_RES
	int		nLoResTextureNum;
	int		nTileBaseX;
	int		nTileBaseY;

		nTileBaseX = (nMapX * 4) / nHeightmapWidth;
		nTileBaseY = (nMapY * 4) / nHeightmapHeight;
		nLoResTextureNum = (nTileBaseY * 4) + nTileBaseX;
		if ( nLoResTextureNum > 0 )
		{
			nLoResTextureNum %= 16;
		}
		else
		{
			nLoResTextureNum = 0;
		}
		return( MAX_NUM_HIRES_LANDSCAPE_TEXTURES_LOADED + nLoResTextureNum );
#endif

	nHiResMapping = nHeightmapWidth / nHiResMapSize;

	nTileTexture = ((nMapY/nHiResMapping) * nHiResMapSize) + (nMapX/nHiResMapping);
	if ( nTileTexture < 0 ) nTileTexture = 0;
	nTileTexture %= TEXTURE_LOOKUP_SLOTS;

	nHiResLandscapeTextureNum = mahTileTextureLookUp[nTileTexture];

	if ( nHiResLandscapeTextureNum == NOTFOUND )
	{
		nHiResLandscapeTextureNum = LandscapeTileGenTexturesGetNextFreeHandle();

		if ( nHiResLandscapeTextureNum != NOTFOUND )
		{
		char	acString[256];

			if ( mbLandscapeTexturesJpegs )
			{
				sprintf( acString, "%s\\x32\\TileY%03dX%03d.jpg", LandscapeGetLevelRootPath(), nMapY / nHiResMapping, nMapX / nHiResMapping );
			}
			else
			{
				sprintf( acString, "%s\\x32\\TileY%03dX%03d.bmp", LandscapeGetLevelRootPath(), nMapY / nHiResMapping, nMapX / nHiResMapping );
			}
			maHiResLandscapeTextures[nHiResLandscapeTextureNum].hTextureHandle = EngineLoadTexture( acString, 0, 0 ); 
			maHiResLandscapeTextures[nHiResLandscapeTextureNum].nTileTextureLookupNum = nTileTexture;
			mahTileTextureLookUp[nTileTexture] = nHiResLandscapeTextureNum;
		}
	}

	if ( nHiResLandscapeTextureNum != NOTFOUND )
	{
		maHiResLandscapeTextures[nHiResLandscapeTextureNum].ulLastTouched = mulTextureTouchFrame;
		if ( EngineTextureIsFullyLoaded( maHiResLandscapeTextures[nHiResLandscapeTextureNum].hTextureHandle ) == TRUE )
		{
			bTextureIsReady = TRUE;
		}
	}

	if ( bTextureIsReady )
	{
		return( nHiResLandscapeTextureNum );
	}
	else
	{
		// Get the low-res vers
	int		nLoResTextureNum;
	int		nTileBaseX;
	int		nTileBaseY;

		nTileBaseX = (nMapX * 4) / nHeightmapWidth;
		nTileBaseY = (nMapY * 4) / nHeightmapHeight;
		nLoResTextureNum = (nTileBaseY * 4) + nTileBaseX;
		if ( nLoResTextureNum > 0 )
		{
			nLoResTextureNum %= 16;
		}
		else
		{
			nLoResTextureNum = 0;
		}
		return( MAX_NUM_HIRES_LANDSCAPE_TEXTURES_LOADED + nLoResTextureNum );
	}
 }

