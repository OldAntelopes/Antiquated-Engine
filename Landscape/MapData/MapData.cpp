
#include <stdio.h>
#include "StandardDef.h"
#include "Engine.h"

#include "../LandscapeHeightmap.h"
#include "../LandscapeCoords.h"
#include "../Landscape.h"

#include "MapTextureCodes.h"
#include "MapTileData.h"
#include "MapData.h"
#include "PathFinding.h"

#define		MAP_FILE_VERSION		5

#define		MAP_MSGCODE_ID			0x50

typedef struct
{
	int		nVersion;
	int		nSizeOfHeader;

	float	fMapScale;
	BYTE	bMapCoordSystem;
	BYTE	bPad1;
	short	wMapTileSize;

} MAP_FILE_HEADER;

typedef struct
{
	BYTE		bMsgCode;
	BYTE		bPacketType;
//	char		cLen;
//	BYTE		bPad1;
	short		wLen;

} MAP_MSG_HEADER;


int		mnNumPacketsInMap = 0;
BYTE*	mpbMapPacketMem = NULL;
int		mnMapPacketMemSize = 0;

//float	fUniMapToWorldScale = (115.0f/256.0f);		// This is the world size of 1 standard universal tile

float	mfMapDataScale = 1.0f;
int		mnMapDataMapSize = 256;
int		mnMapDataCoordSystem = 1;

MapPacketHandler		maMapPacketHandlers[MAX_NUM_VALID_MAP_PACKET_TYPES] = { NULL };


void		MapDataConvertMapCoordinate( int* pnMapX, int* pnMapY )
{
int		nMapX = *pnMapX;
int		nMapY = *pnMapY;

	if ( mnMapDataCoordSystem == 1 )
	{
		*pnMapX = (mnMapDataMapSize-1) - nMapX;
		*pnMapY = (mnMapDataMapSize-1) - nMapY;
	}
	else
	{
		*pnMapX = nMapX;
		*pnMapY = nMapY;
	}

}


VECT	MapDataConvertCoordinate( float fX, float fY )
{
VECT	xPos;
int		nLandscapeHeightmapSize = LandscapeHeightmapGetSizeX();
float	fHeightmapSizeMod;
float	fThisLandscapeScale = LandscapeMapToWorldScale();

	fHeightmapSizeMod = (float) nLandscapeHeightmapSize / mnMapDataMapSize;	
	// Convert the island/universal world coord into island/universal map coord 
	xPos.x = (fX * mfMapDataScale);
	xPos.y = (fY * mfMapDataScale);

	// uni coords - 0.0, 0.0 is bottom right of map, so flip for herd
	if ( mnMapDataCoordSystem == 1 )
	{
		xPos.x = mnMapDataMapSize - xPos.x;
		xPos.y = mnMapDataMapSize - xPos.y;
	}

	// Convert island/universal map coord into herd map coord
	xPos.x *= fHeightmapSizeMod;
	xPos.y *= fHeightmapSizeMod;

	// Convert herd map coord into herd world coord
	xPos.x *= fThisLandscapeScale;
	xPos.y *= fThisLandscapeScale;

	return( xPos );

}

float	MapDataGetScale( void )
{
	return( mfMapDataScale );
}

int		MapDataGetMapSize( void )
{
	return( mnMapDataMapSize );
}

int		MapDataGetCoordinateSystem( void )
{
	return( mnMapDataCoordSystem );
}


void		MapDataAddNewPacket( MAP_MSG_HEADER* pxMapPacket, BOOL bPostLoad )
{
	// todo - copy to mpbMapPacketMem

	if ( maMapPacketHandlers[pxMapPacket->bPacketType] != NULL )
	{
		maMapPacketHandlers[pxMapPacket->bPacketType]( pxMapPacket->bPacketType, (void*)( pxMapPacket + 1 ) );
	}
}


void		MapDataRegisterPacketHandler( int nMapPacketID, MapPacketHandler fnButtonHandler )
{
	maMapPacketHandlers[nMapPacketID] = fnButtonHandler;
}


void MapFileLoad( char* pcFilename )
{
byte				abMapPacket[512];
MAP_MSG_HEADER*		pxMsg = (MAP_MSG_HEADER*)(abMapPacket);
void*				pFile;
MAP_FILE_HEADER		xFileHeader;

	pFile = SysFileOpen( pcFilename, "rb" );

//	mpbEmptyPosInMapMem = ClientMapGetPacketMem();
	mnNumPacketsInMap = 0;

	if ( pFile != NULL )
	{
		SysFileRead( (BYTE*)( &xFileHeader.nVersion ), sizeof(int), 1, pFile );
		if( xFileHeader.nVersion != MAP_FILE_VERSION )
		{
//			ServerPrint( SP_SYSTEM, "Map file version differs. The server will now attempt to update the map file\n");
		}

		SysFileRead( (BYTE*)( &xFileHeader.nSizeOfHeader ), sizeof(int), 1, pFile );
		if( xFileHeader.nSizeOfHeader != sizeof( MAP_FILE_HEADER ) )
		{
//			ServerPrint(SP_SYSTEM, "Map header size differs.\n");
		}

		SysFileRead( (BYTE*)( &xFileHeader.fMapScale ), xFileHeader.nSizeOfHeader - (2*sizeof(int)), 1, pFile );

		if ( xFileHeader.nVersion >= 5 )
		{
			mfMapDataScale = xFileHeader.fMapScale;
			mnMapDataMapSize = xFileHeader.wMapTileSize;
			mnMapDataCoordSystem = xFileHeader.bMapCoordSystem;
		}

		SysFileRead( (BYTE*)( abMapPacket ), sizeof( MAP_MSG_HEADER ), 1, pFile );

//		PANIC_IF( pxMsg->bMsgCode != MAP_MSGCODE_ID, "Invalid data in map file\n" );
		while( pxMsg->bPacketType != 0xFF )
		{
			if ( ( pxMsg->wLen & 0xFF00 ) > 0x200 )
			{
				pxMsg->wLen &= 0xFF;
			}
			// Read the packet 
			SysFileRead( abMapPacket + sizeof( MAP_MSG_HEADER ), pxMsg->wLen - sizeof( MAP_MSG_HEADER ), 1, pFile );		

			MapDataAddNewPacket( pxMsg, FALSE );

			// Read the next header 
			SysFileRead( (BYTE*)( abMapPacket ), sizeof( MAP_MSG_HEADER ), 1, pFile );
		
			if ( pxMsg->bMsgCode != MAP_MSGCODE_ID )
			{
//				PrintConsoleCR( "** Warning - invalid data in map file", COL_WARNING );
				// Abort
				pxMsg->bPacketType = 0xFF;
			}
		}
		SysFileClose( pFile );
	}

}



void		MapDataInit( void )
{
	MapTextureCodesInit();

	mnMapPacketMemSize = 32768;
	mpbMapPacketMem = (BYTE*)( malloc( mnMapPacketMemSize ) );

	MapTileDataInit();
}


void		MapDataLoad( void )
{
char	acMapFileName[256];

	sprintf( acMapFileName, "%s\\level.map", LandscapeGetLevelRootPath() );
	MapFileLoad( acMapFileName );

	PathDataLoad();
}


void		MapDataFree( void )
{
	PathDataFree();

	MapTileDataFree();

	if ( mpbMapPacketMem )
	{
		free( mpbMapPacketMem );
		mpbMapPacketMem = NULL;
		mnMapPacketMemSize = 0;
	}
}
