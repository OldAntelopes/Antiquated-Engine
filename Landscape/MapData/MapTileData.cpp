
#include "StandardDef.h"

#include "../LandscapeCoords.h"
#include "../LandscapeTextures.h"

#include "MapGen.h"
#include "MapData.h"
#include "MapTextureCodes.h"
#include "MapTileData.h"

struct MapTileData
{
	int		nMapX;
	int		nMapY;

	float	fGrassLevel;
	int		nLakeID;

	BYTE	bTextureType;			// e.g. LANDSCAPE_TEXTURETYPE_TILE  or  LANDSCAPE_TEXTURETYPE_TEXMAP
	BYTE	bTextureTypeIndex;		// Tile/Surface/Texmap index #
	BYTE	bPad1;
	BYTE	bPad2;
};



MapTileData*		mspMapTileData = NULL;
int			mnMapTileDataSizeX = 0;
int			mnMapTileDataSizeY = 0;
int			mnNumMapTiles = 0;

MapTileData*		MapTileDataGet( int nMapX, int nMapY )
{
	if ( mspMapTileData )
	{
	int					nIndex = ( abs( (nMapY*mnMapTileDataSizeX) + nMapX) % mnNumMapTiles);
	MapTileData*		pTileData = mspMapTileData + nIndex;

		return( pTileData );
	}
	return( NULL );
}

float	MapTileDataGetGrassLevel( int nMapX, int nMapY )
{
int					nIndex = ( abs( (nMapY*mnMapTileDataSizeX) + nMapX) % mnNumMapTiles);
MapTileData*		pTileData = mspMapTileData + nIndex;
	return( pTileData->fGrassLevel );
}

int		MapTileDataGetLakeID( int nMapX, int nMapY )
{
int					nIndex = ( abs( (nMapY*mnMapTileDataSizeX) + nMapX) % mnNumMapTiles);
MapTileData*		pTileData = mspMapTileData + nIndex;

	return( pTileData->nLakeID );
}

void		MapTileDataSetLakeID( int nMapX, int nMapY, int nLakeID )
{
int					nIndex = ( abs( (nMapY*mnMapTileDataSizeX) + nMapX) % mnNumMapTiles);
MapTileData*		pTileData = mspMapTileData + nIndex;

	pTileData->nLakeID = nLakeID;
}

void		MapTileDataGetTexture( int nMapX, int nMapY, TILE_TEXTURE* pxOut )
{
MapTileData*	pTileData = MapTileDataGet( nMapX, nMapY );

	pxOut->nTextureType = pTileData->bTextureType;
	pxOut->nTextureTypeIndex = pTileData->bTextureTypeIndex;

}

void	MapTileDataBoxTerrainPacketHandler( int nPacketType, void* pxMapPacket )
{
MAP_PACKET_LINE_OF_TERRAIN*		pxTerrainBox = (MAP_PACKET_LINE_OF_TERRAIN*)( pxMapPacket );
MapTileData*			pMapTileData;
int		nLoop;
ONE_LINE_OF_TERRAIN*		pxOneBox;
int		nX1, nY1, nX2, nY2;
int		nIndex1, nIndex2;
int		nMapSize = MapDataGetMapSize();
int		nVal;
int		nLoopX, nLoopY;
int		nSwap;
int		nTextureType;
int		nTextureTypeIndex;

	for( nLoop = 0; nLoop < NUM_TERRAIN_LINES_IN_PACKET; nLoop++ )
	{
		pxOneBox = pxTerrainBox->axPackets + nLoop;

		nIndex1 = pxOneBox->ulTexIndex1 & 0x7FFFFF;
		nIndex2 = pxOneBox->ulTexIndex2 & 0x7FFFFF;
		nVal = (pxOneBox->ulTexIndex1 & 0xFF800000) >> 23;

		nX1 = nIndex1 % nMapSize;
		nY1 = nIndex1 / nMapSize;

		nX2 = nIndex2 % nMapSize;
		nY2 = nIndex2 / nMapSize;

		MapDataConvertMapCoordinate( &nX1, &nY1 );
		MapDataConvertMapCoordinate( &nX2, &nY2 );

		if ( nX1 > nX2 )
		{
			nSwap = nX2;
			nX2 = nX1;
			nX1 = nSwap;
		}
		if ( nY1 > nY2 )
		{
			nSwap = nY2;
			nY2 = nY1;
			nY1 = nSwap;
		}

		for ( nLoopY = nY1; nLoopY <= nY2; nLoopY++ )
		{
			for ( nLoopX = nX1; nLoopX <= nX2; nLoopX++ )
			{
				pMapTileData = MapTileDataGet( nLoopX, nLoopY );

				MapTextureCodesGetTypeAndIndex( nVal, &nTextureType, &nTextureTypeIndex );
				pMapTileData->bTextureType = (BYTE)( nTextureType );
				pMapTileData->bTextureTypeIndex = (BYTE)( nTextureTypeIndex );
			}
		}

	}
}

void		MapTileDataRegisterHandlers( void )
{
	MapDataRegisterPacketHandler( MAP_BOX_OF_TERRAIN, MapTileDataBoxTerrainPacketHandler );
}

void		MapTileDataInit( void )
{
int		nMapSizeX = LandscapeGetMapSizeX();
int		nMapSizeY = LandscapeGetMapSizeY();
int		nMemSize = nMapSizeX * nMapSizeY * sizeof( MapTileData );
int		nLoopX;
int		nLoopY;
MapTileData*		pTileData;

	mnMapTileDataSizeX = nMapSizeX;
	mnMapTileDataSizeY = nMapSizeY;
	mnNumMapTiles = nMapSizeX * nMapSizeY;

	mspMapTileData = (MapTileData*)malloc( nMemSize );
	memset( mspMapTileData, 0, nMemSize );
	
	pTileData = mspMapTileData;
	for( nLoopY = 0; nLoopY < nMapSizeY; nLoopY++ )
	{
		for( nLoopX = 0; nLoopX < nMapSizeX; nLoopX++ )
		{
			pTileData->nMapX = nLoopX;
			pTileData->nMapY = nLoopY;
			pTileData++;
		}
	}

	MapTileDataRegisterHandlers();
}

void		MapTileDataFree( void )
{
	free( mspMapTileData );
	mspMapTileData = NULL;
}


void		MapTileDataParseMapGen( void )
{
int		nMapSizeX = LandscapeGetMapSizeX();
int		nMapSizeY = LandscapeGetMapSizeY();
int		nLoopX;
int		nLoopY;
MapTileData*		pTileData;
eMapGenType		mapType;
VECT	xVect;

	pTileData = mspMapTileData;
	for( nLoopY = 0; nLoopY < nMapSizeY; nLoopY++ )
	{
		for( nLoopX = 0; nLoopX < nMapSizeX; nLoopX++ )
		{
			xVect = LandscapeGetWorldPos( nLoopX, nLoopY );
			mapType = MapGenGetTileType( xVect.x, xVect.y );

			switch( mapType )
			{
			case MAPGEN_FULLGRASS:
				pTileData->fGrassLevel = 1.0f;
				break;
			case MAPGEN_PARTIALGRASS:
				pTileData->fGrassLevel = 0.1f;
				break;
			default:
				break;
			}

			pTileData++;
		}
	}
}


