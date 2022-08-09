
#include <stdio.h>
#include "StandardDef.h"
#include "Engine.h"
#include "Interface.h"

#include "../LandscapeCoords.h"
#include "../Landscape.h"

#include "MapGen.h"

int		mhMapGenBitmap = NOTFOUND;
BYTE*	mpbMapGenLockedBitmap = NULL;
int		mnMapGenBitmapPitch = 0;
int		mnMapGenBitmapFormat = 0;
int		mnMapGenBitmapSizeX = 0;
int		mnMapGenBitmapSizeY = 0;

int		mnMapGenRand = 234874219;
#define MAPGEN_RAND_MAX ((1U << 31) - 1)

void MapGenSRand(int x)
{
	mnMapGenRand = x;
}

int		MapGenRand( int nMax )
{
int		ret;

//	mnMapGenRand = (mnMapGenRand * 1103515245 + 12345) & MAPGEN_RAND_MAX;
	mnMapGenRand = ( (mnMapGenRand * 214013 + 2531011) & MAPGEN_RAND_MAX);
	ret = mnMapGenRand >> 16;
	return( ret % nMax );
}

float	MapGenFRand( float fLow, float fHigh )
{
int		nRand;
float	fGap = fHigh - fLow;
float	fRand;

	nRand = MapGenRand( 32768 );
	fRand = ((float)(nRand) * fGap) / 32768;
	fRand += fLow;
	return( fRand );
}


void		MapGenLoadBitmap( void )
{
char	acMapGenBitmapFilename[256];
	
	sprintf( acMapGenBitmapFilename, "%s\\MapGen.bmp", LandscapeGetLevelRootPath() );
	mhMapGenBitmap = InterfaceLoadTexture( acMapGenBitmapFilename, 2 );
	
	InterfaceGetTextureSize( mhMapGenBitmap, &mnMapGenBitmapSizeX, &mnMapGenBitmapSizeY );

	mpbMapGenLockedBitmap = InterfaceLockTexture( mhMapGenBitmap, &mnMapGenBitmapPitch, &mnMapGenBitmapFormat, 0 );
	
}

void	MapGenReleaseBitmap( void )
{
	InterfaceUnlockTexture( mhMapGenBitmap );
	mpbMapGenLockedBitmap = NULL;
	InterfaceReleaseTexture( mhMapGenBitmap );
	mhMapGenBitmap = NOTFOUND;
}

eMapGenType		MapGenGetTileType( float fWorldX, float fWorldY )
{
ulong		ulCol;
float		fR, fG, fB, fA;
int			nMapGenX, nMapGenY;
float	fMapModX = (float)mnMapGenBitmapSizeX / LandscapeGetMapSizeX();
float	fMapModY = (float)mnMapGenBitmapSizeY / LandscapeGetMapSizeY();
float	fWorldToGameMap = LandscapeWorldToMapScale();

	nMapGenX = (int)( (fWorldX * fWorldToGameMap) * fMapModX );
	nMapGenY = (int)( (fWorldY * fWorldToGameMap) * fMapModY );

	InterfaceTextureGetColourAtPoint( mhMapGenBitmap, mpbMapGenLockedBitmap, mnMapGenBitmapPitch, mnMapGenBitmapFormat, nMapGenX, nMapGenY, &fR, &fG, &fB, &fA );
	ulCol = (ulong)( fR * 255.0f ) << 16;
	ulCol |= (ulong)( fG * 255.0f ) << 8;
	ulCol |= (ulong)( fB * 255.0f );
	ulCol |= 0xFF000000;

	switch( ulCol )
	{
	case 0xFFf03030:
		return( MAPGEN_NEARCLIFF );
	case 0xFFc01010:
		return( MAPGEN_CLIFF );
	case 0xFF7090e0:
		return( MAPGEN_SEA );
	case 0xFFf0f0f0:
	case 0xFFB0B0B0:
		return( MAPGEN_SNOW );
	case 0xFF606060:
		return( MAPGEN_ROCK );
	case 0xFF009000:
		return( MAPGEN_FULLGRASS );
	case 0xFF00f000:
		return( MAPGEN_PARTIALGRASS );
	case 0xFFC09000:
		return( MAPGEN_SAND );
	}

	return( MAPGEN_EMPTY );
}
