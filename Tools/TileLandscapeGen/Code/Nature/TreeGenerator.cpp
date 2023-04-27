
#include <stdio.h>
#include "StandardDef.h"
#include "Interface.h"

#include "../Landscape/LandscapeCoords.h"

#include "../MenuInterface.h"

#include "Trees.h"
#include "TreeLists.h"
#include "TreeGenerator.h"

enum eMapGenType
{
	MAPGEN_CLIFF,
	MAPGEN_NEARCLIFF,
	MAPGEN_SEA,
	MAPGEN_FULLGRASS,
	MAPGEN_PARTIALGRASS,
	MAPGEN_SAND,
	MAPGEN_ROCK,
	MAPGEN_SNOW,
	MAPGEN_EMPTY,
};

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
	
	sprintf( acMapGenBitmapFilename, "%s\\MapGen.bmp", MenuInterfaceGetLevelFolderPath() );
	mhMapGenBitmap = InterfaceLoadTexture( acMapGenBitmapFilename, 0 );
	
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
uint32		ulCol;
float		fR, fG, fB, fA;
int			nMapGenX, nMapGenY;
float	fMapModX = (float)mnMapGenBitmapSizeX / LandscapeGetMapSizeX();
float	fMapModY = (float)mnMapGenBitmapSizeY / LandscapeGetMapSizeY();
float	fWorldToGameMap = LandscapeWorldToMapScale();

	nMapGenX = (int)( (fWorldX * fWorldToGameMap) * fMapModX );
	nMapGenY = (int)( (fWorldY * fWorldToGameMap) * fMapModY );

	InterfaceTextureGetColourAtPoint( mhMapGenBitmap, mpbMapGenLockedBitmap, mnMapGenBitmapPitch, mnMapGenBitmapFormat, nMapGenX, nMapGenY, &fR, &fG, &fB, &fA );
	ulCol = (uint32)( fR * 255.0f ) << 16;
	ulCol |= (uint32)( fG * 255.0f ) << 8;
	ulCol |= (uint32)( fB * 255.0f );
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


void		TreesGenerate( void )
{
int		nLoop;
int		nNumTrees = 30000;
float	fWorldX;
float	fWorldY;
float	fWorldMaxX = LandscapeGetMapSizeX() * LandscapeMapToWorldScale();
float	fWorldMaxY = LandscapeGetMapSizeY() * LandscapeMapToWorldScale();
Tree*	pTree;
int		nRandTries;
int		nMaxRandTries = 10;
eMapGenType		tileType;
BOOL	bSpotOk;

	MapGenSRand( 3482194 );

	MapGenLoadBitmap();

	for ( nLoop = 0; nLoop < nNumTrees; nLoop++ )
	{
		for ( nRandTries = 0; nRandTries < nMaxRandTries; nRandTries++ )
		{
			fWorldX = MapGenFRand( 0.0f, fWorldMaxX );
			fWorldY = MapGenFRand( 0.0f, fWorldMaxY );

			bSpotOk = FALSE;
			tileType = MapGenGetTileType( fWorldX, fWorldY );
			switch( tileType )
			{
			case MAPGEN_CLIFF:
			case MAPGEN_SEA:
			case MAPGEN_SAND:
				break;
			case MAPGEN_FULLGRASS:
				if ( MapGenRand( 10 ) == 0 )
				{
					bSpotOk = TRUE;
				}
				break;
			case MAPGEN_PARTIALGRASS:
			case MAPGEN_ROCK:
			case MAPGEN_EMPTY:
				bSpotOk = TRUE;
				break;
			}

			if ( bSpotOk )
			{
				break;
			}
		}
	
		if ( nRandTries < nMaxRandTries )
		{
			pTree = new Tree;

			pTree->mxPos.x = fWorldX;
			pTree->mxPos.y = fWorldY;
			pTree->mxPos.z = 0.0f;
			pTree->mfRot = MapGenFRand( 0.0f, A360 );
			pTree->mfTreeSpriteSize = MapGenFRand( 0.8f, 1.5f );

			TreeListsAddTree( pTree );
		}

	}

	MapGenReleaseBitmap();
}
