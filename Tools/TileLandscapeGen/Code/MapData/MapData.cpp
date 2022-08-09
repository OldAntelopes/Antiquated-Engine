
#include <stdio.h>
#include "StandardDef.h"

#include "../Landscape/LandscapeHeightmap.h"
#include "../Landscape/LandscapeCoords.h"
#include "../MenuInterface.h"

#include "MapData.h"

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


FILE*		mspMapSaveFile = NULL;

void		MapDataSaveInit( void )
{
MAP_FILE_HEADER		xMapFileHeader;
char				szOutputFilename[256];

	sprintf( szOutputFilename, "%s\\level.map", MenuInterfaceGetLevelFolderPath() );

	mspMapSaveFile = fopen( szOutputFilename, "wb" );
	xMapFileHeader.nVersion = MAP_FILE_VERSION;
	xMapFileHeader.bMapCoordSystem = 0;
	xMapFileHeader.fMapScale = 1.0f;
	xMapFileHeader.nSizeOfHeader = sizeof( xMapFileHeader );
	xMapFileHeader.wMapTileSize = 512;

	fwrite( &xMapFileHeader, sizeof( xMapFileHeader ), 1, mspMapSaveFile );
}

void		MapDataSaveComplete( void )
{
MAP_MSG_HEADER		xFinalPacket;

	xFinalPacket.bMsgCode = 0xFF;
	xFinalPacket.bPacketType = 0xFF;
	xFinalPacket.wLen = sizeof( MAP_MSG_HEADER );
	fwrite( &xFinalPacket, sizeof( xFinalPacket ), 1, mspMapSaveFile );

	fclose( mspMapSaveFile );

}

void		MapDataSavePacketWrite( int nPacketType, void* pPacket, int nSize )
{
BYTE			abPacketBuff[512];
MAP_MSG_HEADER*		pxPacketHeader = (MAP_MSG_HEADER*)( abPacketBuff );
BYTE*		pbPacket = (BYTE*)( pxPacketHeader + 1 );

	memcpy( pbPacket, pPacket, nSize );
	pxPacketHeader->bMsgCode = MAP_MSGCODE_ID;
	pxPacketHeader->bPacketType = (BYTE)( nPacketType );
	pxPacketHeader->wLen = (short)( nSize + sizeof( MAP_MSG_HEADER ) );

	fwrite( abPacketBuff, pxPacketHeader->wLen, 1, mspMapSaveFile );
}

