
#include <stdio.h>
#include <float.h>
#include <math.h>

#include "StandardDef.h"
#include "Engine.h"

#include "MapData\MapData.h"

#include "Landscape/LandscapeHeightmap.h"
#include "Landscape/LandscapeCoords.h"
#include "Landscape/LandscapeShader.h"

#include "MenuInterface.h"
#include "MapGen.h"


typedef struct
{
	int		nMapX;
	int		nMapY;

	int		anConnections[8];
	
	BOOL	bIsFlat;
	BOOL	bIsGrassy;
	BOOL	bIsFullGrass;
	BOOL	bIsNearSea;
	BOOL	bIsUnderWater;
	BOOL	bSandy;
	BOOL	bIsMountainPeak;
	BOOL	bIsBeachy;
	BOOL	bIsRocky;
	BOOL	bIsCliff;
	BOOL	bIsSnow;
	BOOL	bIsNearbyCliff;
	
	float	fGridHeight;
	float	fMaxSlope;


} MAPGEN_DATA;

MAPGEN_DATA*		mspMapGenData = NULL;

int		msnNumMountainPeaks = 0;
int		msnNumForestsAdded = 0;

void		MapGenPrePass( void )
{
int		nHeightmapWidth = LandscapeHeightmapGetSizeX();
int		nLoopX;
int		nLoopY;
MAPGEN_DATA*	pMapGenData = mspMapGenData;

	for( nLoopY = 0; nLoopY < nHeightmapWidth; nLoopY++ )
	{
		for( nLoopX = 0; nLoopX < nHeightmapWidth; nLoopX++ )
		{
			pMapGenData->nMapX = nLoopX;
			pMapGenData->nMapY = nLoopY;
			pMapGenData->fGridHeight = LandscapeHeightmapGetHeight( nLoopX, nLoopY );

			pMapGenData++;
		}
	}
}

void		MapGenGetSurroundingDataGrid( int nLoopX, int nLoopY, MAPGEN_DATA* pMapGenData, MAPGEN_DATA** apMapGenData )
{
int		nHeightmapWidth = LandscapeHeightmapGetSizeX();
int		nLeft = -1;
int		nRight = +1;

	if ( nLoopX == 0 )
	{
		nLeft = 0;
	}
	else if ( nLoopX == nHeightmapWidth-1 )
	{
		nRight = 0;
	}

	if ( nLoopY == 0 )
	{
		apMapGenData[0] = pMapGenData + nLeft;
		apMapGenData[1] = pMapGenData;
		apMapGenData[2] = pMapGenData + nRight;
	}
	else
	{
		apMapGenData[0] = pMapGenData - (nHeightmapWidth+nLeft);
		apMapGenData[1] = pMapGenData - nHeightmapWidth;
		apMapGenData[2] = pMapGenData - (nHeightmapWidth+nRight);
	}

	apMapGenData[3] = pMapGenData+nLeft;
	apMapGenData[4] = pMapGenData+nRight;

	if ( nLoopY == nHeightmapWidth-1 )
	{
		apMapGenData[5] = pMapGenData + nLeft;
		apMapGenData[6] = pMapGenData;
		apMapGenData[7] = pMapGenData + nRight;
	}
	else
	{
		apMapGenData[5] = pMapGenData + (nHeightmapWidth+nLeft);
		apMapGenData[6] = pMapGenData + nHeightmapWidth;
		apMapGenData[7] = pMapGenData + (nHeightmapWidth+nRight);
	}
}


float		MapGenGetMaxSlope( float fHeight1, float fHeight2, float fHeight3, float fHeight4 )
{
VECT	xEdge1;
VECT	xEdge2;
float	fMapTileSize = LandscapeMapToWorldScale();
float	fHeightValuesToWorldScale = 1.0f;		// TODO - This needs to be defined somewhere and stored in map data
VECT	xCalc;
float	fDot;
float	fDot2;
VECT	xVecUp = { 0.0f, 0.0f, 1.0f };

	xEdge1.x = 0.0f;
	xEdge1.y = fMapTileSize;
	xEdge1.z = fHeight3 - fHeight1;

	xEdge2.x = fMapTileSize;
	xEdge2.y = fMapTileSize;
	xEdge2.z = fHeight4 - fHeight1;

	VectCross( &xCalc, &xEdge2, &xEdge1 );
	VectNormalize( &xCalc );
	fDot = VectDot( &xVecUp, &xCalc );

	xEdge1.x = fMapTileSize;
	xEdge1.y = 0.0f;
	xEdge1.z = fHeight2 - fHeight1;

	VectCross( &xCalc, &xEdge1, &xEdge2 );
	VectNormalize( &xCalc );
	fDot2 = VectDot( &xVecUp, &xCalc );

	if ( fDot2 < fDot )
	{
		return( fDot2 );
	}
	return( fDot );
}


void		MapGenFirstPass( void )
{
int		nHeightmapWidth = LandscapeHeightmapGetSizeX();
int		nLoopX;
int		nLoopY;
MAPGEN_DATA*	pMapGenData = mspMapGenData;
MAPGEN_DATA*	apMapGenData[8];
float	fSnowLineLower;
float	fGrassHeightUpper;
float	fGrassHeightLower;
float	fSeaHeight = 10.0f;			// TODO - This needs to be defined somewhere and stored in map data
int*	pnRangeConstants = LandscapeShaderGetRangeConstants();
float	fHeightValuesToWorldScale = 1.0f;		// TODO - This needs to be defined somewhere and stored in map data
float	fFlatTolerance = 3.0f;

	fSnowLineLower = pnRangeConstants[1] * fHeightValuesToWorldScale;
	fGrassHeightUpper = pnRangeConstants[4] * fHeightValuesToWorldScale;
	fGrassHeightLower = pnRangeConstants[5] * fHeightValuesToWorldScale;

	for( nLoopY = 0; nLoopY < nHeightmapWidth; nLoopY++ )
	{
		for( nLoopX = 0; nLoopX < nHeightmapWidth; nLoopX++ )
		{
			MapGenGetSurroundingDataGrid( nLoopX, nLoopY, pMapGenData, apMapGenData );

			pMapGenData->fMaxSlope = MapGenGetMaxSlope( pMapGenData->fGridHeight, apMapGenData[4]->fGridHeight, apMapGenData[6]->fGridHeight, apMapGenData[7]->fGridHeight );

			if ( pMapGenData->fMaxSlope < 0.921f )
			{
				pMapGenData->bIsCliff = TRUE;
			}

			pMapGenData++;
		}
	}
}

void		MapGenSecondPass( void )
{
int		nHeightmapWidth = LandscapeHeightmapGetSizeX();
int		nLoopX;
int		nLoopY;
MAPGEN_DATA*	pMapGenData = mspMapGenData;
MAPGEN_DATA*	apMapGenData[8];
int		nGridLoop;
BOOL	bFullGrass;
BOOL	bIsBeachy;
float	fSnowLineLower;
float	fGrassHeightUpper;
float	fGrassHeightLower;
float	fSeaHeight = 10.0f;			// TODO - This needs to be defined somewhere and stored in map data
int*	pnRangeConstants = LandscapeShaderGetRangeConstants();
float	fHeightValuesToWorldScale = 1.0f;		// TODO - This needs to be defined somewhere and stored in map data
float	fFlatTolerance = 3.0f;
float	fHeightDiff;
float	fTemp;

	fSnowLineLower = pnRangeConstants[1] * fHeightValuesToWorldScale;
	fGrassHeightUpper = pnRangeConstants[4] * fHeightValuesToWorldScale;
	fGrassHeightLower = pnRangeConstants[5] * fHeightValuesToWorldScale;

	for( nLoopY = 0; nLoopY < nHeightmapWidth; nLoopY++ )
	{
		for( nLoopX = 0; nLoopX < nHeightmapWidth; nLoopX++ )
		{
			MapGenGetSurroundingDataGrid( nLoopX, nLoopY, pMapGenData, apMapGenData );

			if ( ( pMapGenData->fGridHeight >= fGrassHeightLower ) &&
				 ( pMapGenData->fGridHeight <= fGrassHeightUpper ) )
			{
				pMapGenData->bIsGrassy = TRUE;

				bIsBeachy = FALSE;
				if ( pMapGenData->fMaxSlope < 0.95f )
				{
					bFullGrass = FALSE;
				}
				else
				{
					bFullGrass = TRUE;
					for ( nGridLoop = 0; nGridLoop < 8; nGridLoop++ )
					{
						if ( ( apMapGenData[nGridLoop]->fGridHeight < fGrassHeightLower ) ||
							 ( apMapGenData[nGridLoop]->fGridHeight > fGrassHeightUpper ) ||
							 ( apMapGenData[nGridLoop]->fMaxSlope < 0.96f ) )
						{
							if ( apMapGenData[nGridLoop]->fGridHeight < fGrassHeightLower )
							{
								bIsBeachy = TRUE;
							}
							bFullGrass = FALSE;
						}
					}
				}

				pMapGenData->bIsFullGrass = bFullGrass;
				pMapGenData->bIsBeachy = bIsBeachy;

				fHeightDiff = fabs( pMapGenData->fGridHeight - apMapGenData[4]->fGridHeight );
				fTemp = fabs( pMapGenData->fGridHeight - apMapGenData[6]->fGridHeight );
				if ( fTemp > fHeightDiff )
				{
					fHeightDiff = fTemp;
				}
				fTemp = fabs( pMapGenData->fGridHeight - apMapGenData[7]->fGridHeight );
				if ( fTemp > fHeightDiff )
				{
					fHeightDiff = fTemp;
				}
				if ( fHeightDiff < fFlatTolerance )
				{
					pMapGenData->bIsFlat = TRUE;
				}
				else
				{
					pMapGenData->bIsFlat = FALSE;
				}

			}
			else
			{
				pMapGenData->bIsGrassy = FALSE;
				pMapGenData->bIsFullGrass = FALSE;

				if ( ( pMapGenData->fGridHeight < fGrassHeightLower ) &&
					 ( pMapGenData->fGridHeight > fSeaHeight ) )
				{
					pMapGenData->bIsBeachy = TRUE;
				}
				else if ( pMapGenData->fGridHeight > fGrassHeightUpper )
				{
					pMapGenData->bIsRocky = TRUE;
				}
			}

			if ( apMapGenData[1]->bIsCliff )
			{
				pMapGenData->bIsNearbyCliff = TRUE;
			}
			else if ( apMapGenData[3]->bIsCliff )
			{
				pMapGenData->bIsNearbyCliff = TRUE;
			}
			else if ( apMapGenData[4]->bIsCliff )
			{
				pMapGenData->bIsNearbyCliff = TRUE;
			}
			else if ( apMapGenData[6]->bIsCliff )
			{
				pMapGenData->bIsNearbyCliff = TRUE;
			}

			if ( pMapGenData->fGridHeight <= fSeaHeight * 2.0f )
			{
				pMapGenData->bIsUnderWater = TRUE;
			}

			if ( pMapGenData->fGridHeight < fGrassHeightLower )
			{
				pMapGenData->bSandy = TRUE;			
			}
			else if ( pMapGenData->fGridHeight > fSnowLineLower )
			{
			BOOL	bIsPeak = TRUE;

				pMapGenData->bIsSnow = TRUE;
				for ( nGridLoop = 0; nGridLoop < 8; nGridLoop++ )
				{
					if ( apMapGenData[nGridLoop]->fGridHeight > pMapGenData->fGridHeight )
					{
						bIsPeak = FALSE;
					}
				}
				pMapGenData->bIsMountainPeak = bIsPeak;
				if ( bIsPeak )
				{
					msnNumMountainPeaks++;
				}
			}

			pMapGenData++;
		}
	}
}

void	MapGenPickRandomForestLocation( int* pnMapX, int* pnMapY )
{
int		nHeightmapWidth = LandscapeHeightmapGetSizeX();
int		random = rand() % ( nHeightmapWidth * nHeightmapWidth );
MAPGEN_DATA*	pMapGenData = mspMapGenData + random;
int		nCount = 500;

	while( nCount > 0 )
	{
		if ( ( pMapGenData->bIsFullGrass == FALSE ) &&
			 ( pMapGenData->bIsBeachy == FALSE ) &&
			 ( pMapGenData->bIsGrassy == TRUE ) )
		{
			*pnMapX = pMapGenData->nMapX;
			*pnMapY = pMapGenData->nMapY;
			return;
		}
		random = rand() % ( nHeightmapWidth*nHeightmapWidth);
		pMapGenData = mspMapGenData + random;
		nCount--;
	}
	
	*pnMapX = -1;
	*pnMapY = -1;

}

void	MapGenFindMountainPeak( int index, int* pnMapX, int* pnMapY )
{
int		nHeightmapWidth = LandscapeHeightmapGetSizeX();
MAPGEN_DATA*	pMapGenData = mspMapGenData;
int				nLoop;
int			nMapSize = (nHeightmapWidth * nHeightmapWidth );
int			nCount = 0;

	for ( nLoop = 0; nLoop < nMapSize; nLoop++ )
	{
		if ( pMapGenData->bIsMountainPeak )
		{
			if ( nCount == index )
			{
				*pnMapX = pMapGenData->nMapX;
				*pnMapY = pMapGenData->nMapY;
				return;
			}
			nCount++;
		}
		pMapGenData++;
	}
}

void	MapGenMountainForestRings( void )
{
int		nLoop;
int		nMapX;
int		nMapY;

	for ( nLoop = 0; nLoop < msnNumMountainPeaks; nLoop++ )
	{
		MapGenFindMountainPeak( nLoop, &nMapX, &nMapY );

		// todo - 

	}
}

void	MapGenSaveForests( void )
{
int		nLoop;
MAP_PACKET_FOREST		xForestPacket;
int		nMapX;
int		nMapY;

	for ( nLoop = 0; nLoop < 50; nLoop++ )
	{
		MapGenPickRandomForestLocation( &nMapX, &nMapY );
		if ( nMapX > 0 )
		{
			xForestPacket.fX = (float)( nMapX );
			xForestPacket.fY = (float)( nMapY );
			xForestPacket.fSpread = 2.0f;
			xForestPacket.wNumTrees = 5;
			xForestPacket.wTreeType = 1;
			MapDataSavePacketWrite( MAP_FOREST, &xForestPacket, sizeof( xForestPacket ) );
			msnNumForestsAdded++;
		}
	}

	MapGenMountainForestRings();
}

void	MapGenMakeMapFile( void )
{

	MapDataSaveInit();

	MapGenSaveForests();


	MapDataSaveComplete();

}

void		MapGenExportKeyTexture( void )
{
int		nHeightmapWidth = LandscapeHeightmapGetSizeX();
int		hTexture = EngineCreateTexture( nHeightmapWidth, nHeightmapWidth, 1 );
int		nLoopX;
int		nLoopY;
BYTE*	pbLockedTexture;
int		nPitch;
uint32*	pulTextureARGB;
uint32	ulCol;
MAPGEN_DATA*	pMapGenData = mspMapGenData;
 
	pbLockedTexture = EngineLockTexture( hTexture, &nPitch, TRUE );
	pulTextureARGB = (uint32*)( pbLockedTexture );
	for ( nLoopY = 0; nLoopY < nHeightmapWidth; nLoopY++ )
	{
		for ( nLoopX = 0; nLoopX < nHeightmapWidth; nLoopX++ )
		{
			if ( pMapGenData->bIsCliff )
			{
				ulCol = 0xFFc01010;
			}
			else if ( pMapGenData->bIsNearbyCliff )
			{
				ulCol = 0xFFf03030;
			}
			else if ( pMapGenData->bIsMountainPeak )
			{
				ulCol = 0xFFf0f0f0;
			}
			else if ( pMapGenData->bIsUnderWater )
			{
				ulCol = 0xFF7090e0;
			}
			else if ( pMapGenData->bIsSnow ) 
			{
				ulCol = 0xFFB0B0B0;
			}
			else if ( pMapGenData->bIsRocky ) 
			{
				ulCol = 0xFF606060;
			}
			else if ( pMapGenData->bIsGrassy )
			{
				ulCol = 0xFF00f000;
				if ( pMapGenData->bIsFullGrass )
				{
					ulCol = 0xFF009000;
				}
			}
			else if ( pMapGenData->bIsBeachy )
			{
				ulCol = 0xFFC09000;
			}
			else
			{
				ulCol = 0xff101010;
			}
			pulTextureARGB[ (nLoopY * (nPitch/4)) + nLoopX ] = ulCol;
			pMapGenData++;
		}
	}
	EngineUnlockTexture( hTexture );

	char	szOutputFilename[256];

	sprintf( szOutputFilename, "%s\\MapGen.bmp", MenuInterfaceGetLevelFolderPath() );
	EngineExportTexture( hTexture, szOutputFilename, 0 );
	MenuInterfacePrint( "Created MapGen: %s", szOutputFilename );
}


typedef struct
{
	uint32	ulPathFileHeaderCode;
	BYTE	bVersion;
	BYTE	bSizeOfHeader;
	BYTE	bPad1;
	BYTE	bPad2;
	
	int		nMapSizeX;
	int		nMapSizeY;

} PATH_FILE_HEADER;

void	MapGenMakePathData( void )
{
int		nHeightmapWidth = LandscapeHeightmapGetSizeX();
int		nHeightmapHeight = LandscapeHeightmapGetSizeY();
int		nLoopX;
int		nLoopY;
MAPGEN_DATA*	pMapGenData = mspMapGenData;
float	fSeaHeight = 10.0f;			// TODO - This needs to be defined somewhere and stored in map data
int*	pnRangeConstants = LandscapeShaderGetRangeConstants();
float	fHeightValuesToWorldScale = 1.0f;		// TODO - This needs to be defined somewhere and stored in map data
float	fFlatTolerance = 3.0f;
FILE*	pFile;
BYTE	bPath;
PATH_FILE_HEADER		xFileHeader;
char	szOutputFilename[256];
	
	sprintf( szOutputFilename, "%s\\pathdata.pth", MenuInterfaceGetLevelFolderPath() );
	pFile = fopen( szOutputFilename, "wb" );

	memset( &xFileHeader, 0, sizeof( xFileHeader ) );
	xFileHeader.ulPathFileHeaderCode = 0x60a49e12;
	xFileHeader.bVersion = 1;
	xFileHeader.bSizeOfHeader = sizeof( xFileHeader );
	xFileHeader.nMapSizeX = nHeightmapWidth;
	xFileHeader.nMapSizeY = nHeightmapHeight;
	fwrite( &xFileHeader, sizeof( xFileHeader ), 1, pFile );

	for( nLoopY = 0; nLoopY < nHeightmapHeight; nLoopY++ )
	{
		for( nLoopX = 0; nLoopX < nHeightmapWidth; nLoopX++ )
		{
			if ( ( !pMapGenData->bIsCliff ) &&
				 ( !pMapGenData->bIsUnderWater ) &&
				 ( !pMapGenData->bIsSnow ) &&
				 ( !pMapGenData->bIsNearbyCliff ) )
			{
				bPath = 1;
			}
			else
			{
				bPath = 0;
			}
						
			fwrite( &bPath, 1, 1, pFile );
			
			pMapGenData++;
		}
	}
	fclose( pFile );

	MenuInterfacePrint( "Created pathdata: %s", szOutputFilename );

}


void		MapGenInit( void )
{
int		nHeightmapWidth = LandscapeHeightmapGetSizeX();
int		nMemSize = nHeightmapWidth * nHeightmapWidth * sizeof( MAPGEN_DATA );

	mspMapGenData = (MAPGEN_DATA*)( malloc( nMemSize ) );
	memset( mspMapGenData, 0, nMemSize );

	msnNumMountainPeaks = 0;

	MapGenPrePass();

	MapGenFirstPass();
	MapGenSecondPass();

	MapGenMakeMapFile();

	MapGenMakePathData();

	MapGenExportKeyTexture();
}



