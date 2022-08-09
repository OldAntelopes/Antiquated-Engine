
#include <math.h>
#include "StandardDef.h"
#include "Engine.h"

#include "../MapData/MapData.h"
#include "../MapData/MapTileData.h"
#include "../LandscapeCoords.h"
#include "../LandscapeHeightmap.h"
#include "../Landscape.h"
#include "../Sea.h"

#include "Lakes.h"

#define		LAKE_NUM_VERTEX_BUFFERS		2
#define		LAKE_VERTEX_BUFFER_SIZE		600

class Lake
{
public:
	
	void		Init( int nLakeID, VECT* pxPos, float fRadius );

	int			GetLakeID( void ) { return( mnLakeID ); }
	float		GetWaterHeight( void ) { return( mfWaterHeight ); }
	Lake*		GetNext( void ) { return( mpNext ); }
	void		SetNext( Lake* pLake ) { mpNext = pLake; }

private:

	float		mfWaterHeight;
	BOOL		mbWantsRender;
	VECT		mxOrigin;
	float		mfRadius;
	int			mnLakeID;
	Lake*		mpNext;
};

class LakeTileRenderList
{
public:
	int		mnMapX;
	int		mnMapY;
	int		mnLakeID;
	float	mfWaterHeight;
	LakeTileRenderList*		mpNext;
};

int						msnNextLakeID = 0x2101;
Lake*					mspLakesList = NULL;
LakeTileRenderList*		mspLakeTileRenderList = NULL;

int			mahLakeVertexBuffers[LAKE_NUM_VERTEX_BUFFERS] = { NOTFOUND, NOTFOUND };
int			msnLakeCurrentVertexBufferPos = 0;
int			msnLakeCurrentVertexBufferIndex = 0;

//--------------------------------------------------------------------------------------


int		LakeRenderFlushVertexBuffer( void )
{
	EngineVertexBufferAddVertsUsed( mahLakeVertexBuffers[ msnLakeCurrentVertexBufferIndex ], msnLakeCurrentVertexBufferPos );
	EngineVertexBufferRender( mahLakeVertexBuffers[ msnLakeCurrentVertexBufferIndex ], TRIANGLE_LIST );
	EngineVertexBufferReset( mahLakeVertexBuffers[ msnLakeCurrentVertexBufferIndex ] );

	msnLakeCurrentVertexBufferIndex++;
	msnLakeCurrentVertexBufferIndex %= LAKE_NUM_VERTEX_BUFFERS;
	msnLakeCurrentVertexBufferPos = 0;

	return(	 mahLakeVertexBuffers[ msnLakeCurrentVertexBufferIndex ] );
}


//--------------------------------------------------------------------------------------

BOOL	LakeRenderRenderLists( void )
{
int		hCurrentVertexBuffer = mahLakeVertexBuffers[ msnLakeCurrentVertexBufferIndex ];
ulong*		pulColStream = NULL;
int			nColStride = 0;
VECT*		pxPosStream = NULL;
int			nPosStride = 0;
float*		pfUVStream = NULL;
int			nUVStride = 0;
int			nVertCount = 0;
LakeTileRenderList*		pItems = mspLakeTileRenderList;
float		fUBase = 0.0f;
float		fVBase = 0.0f;
float		fUWidth = 1.0f;
float		fVHeight = 1.0f;
ulong		ulCol = 0xFFFFFFFF;
float		fMapToWorldSize = LandscapeMapToWorldScale();

	ulCol = EngineGetColValue( 0x50, 0x60, 0xA0, 0xA0 );

	pulColStream = EngineVertexBufferLockColourStream( hCurrentVertexBuffer, &nColStride );
	pxPosStream = EngineVertexBufferLockPositionStream( hCurrentVertexBuffer, &nPosStride );
	pfUVStream = EngineVertexBufferLockUVStream( hCurrentVertexBuffer, &nUVStride );

	if ( pulColStream )
	{
		while( pItems )
		{
//			pItems->mfWaterHeight += 5.0f;
			// Set texture..
//				LandscapeTextureGetUVs( mnLandscapeTextureNum, pItems->nMapX, pItems->nMapY, &fUBase, &fVBase, &fUWidth, &fVHeight );
			// Tri 1 - vert 1
			*pulColStream = ulCol;
			pxPosStream->x = (float)(pItems->mnMapX - 0.0f) * fMapToWorldSize;
			pxPosStream->y = (float)(pItems->mnMapY - 0.0f) * fMapToWorldSize;
			pxPosStream->z = pItems->mfWaterHeight;
			pfUVStream[0] = fUBase;
			pfUVStream[1] = fVBase;

			pxPosStream = (VECT*)( ( (byte*)pxPosStream) + nPosStride );
			pulColStream = (ulong*)( ( (byte*)pulColStream) + nColStride );
			pfUVStream = (float*)( ( (byte*)pfUVStream) + nUVStride );

			// Tri 1 - vert 2
			*pulColStream = ulCol;
			pxPosStream->x = (float)(pItems->mnMapX + 1.0f) * fMapToWorldSize;
			pxPosStream->y = (float)(pItems->mnMapY - 0.0f) * fMapToWorldSize;
			pxPosStream->z = pItems->mfWaterHeight;
			pfUVStream[0] = fUWidth;
			pfUVStream[1] = fVBase;

			pxPosStream = (VECT*)( ( (byte*)pxPosStream) + nPosStride );
			pulColStream = (ulong*)( ( (byte*)pulColStream) + nColStride );
			pfUVStream = (float*)( ( (byte*)pfUVStream) + nUVStride );

			// Tri 1 - vert 3
			*pulColStream = ulCol;
			pxPosStream->x = (float)(pItems->mnMapX - 0.0f) * fMapToWorldSize;
			pxPosStream->y = (float)(pItems->mnMapY + 1.0f) * fMapToWorldSize;
			pxPosStream->z = pItems->mfWaterHeight;
			pfUVStream[0] = fUBase;
			pfUVStream[1] = fVHeight;

			pxPosStream = (VECT*)( ( (byte*)pxPosStream) + nPosStride );
			pulColStream = (ulong*)( ( (byte*)pulColStream) + nColStride );
			pfUVStream = (float*)( ( (byte*)pfUVStream) + nUVStride );

			//-------------------------

			// Tri 2- vert 1
			*pulColStream = ulCol;
			pxPosStream->x = (float)(pItems->mnMapX + 1.0f) * fMapToWorldSize;
			pxPosStream->y = (float)(pItems->mnMapY - 0.0f) * fMapToWorldSize;
			pxPosStream->z = pItems->mfWaterHeight;
			pfUVStream[0] = fUWidth;
			pfUVStream[1] = fVBase;

			pxPosStream = (VECT*)( ( (byte*)pxPosStream) + nPosStride );
			pulColStream = (ulong*)( ( (byte*)pulColStream) + nColStride );
			pfUVStream = (float*)( ( (byte*)pfUVStream) + nUVStride );

			// Tri 2 - vert 2
			*pulColStream = ulCol;
			pxPosStream->x = (float)(pItems->mnMapX + 1.0f) * fMapToWorldSize;
			pxPosStream->y = (float)(pItems->mnMapY + 1.0f) * fMapToWorldSize;
			pxPosStream->z = pItems->mfWaterHeight;
			pfUVStream[0] = fUWidth;
			pfUVStream[1] = fVHeight;

			pxPosStream = (VECT*)( ( (byte*)pxPosStream) + nPosStride );
			pulColStream = (ulong*)( ( (byte*)pulColStream) + nColStride );
			pfUVStream = (float*)( ( (byte*)pfUVStream) + nUVStride );

			// Tri 2 - vert 3
			*pulColStream = ulCol;
			pxPosStream->x = (float)(pItems->mnMapX - 0.0f) * fMapToWorldSize;
			pxPosStream->y = (float)(pItems->mnMapY + 1.0f) * fMapToWorldSize;
			pxPosStream->z = pItems->mfWaterHeight;
			pfUVStream[0] = fUBase;
			pfUVStream[1] = fVHeight;

			pxPosStream = (VECT*)( ( (byte*)pxPosStream) + nPosStride );
			pulColStream = (ulong*)( ( (byte*)pulColStream) + nColStride );
			pfUVStream = (float*)( ( (byte*)pfUVStream) + nUVStride );

			pItems = pItems->mpNext;
			nVertCount += 6;
			msnLakeCurrentVertexBufferPos += 6;
			if ( nVertCount >= LAKE_VERTEX_BUFFER_SIZE - 6 )
			{
				EngineVertexBufferUnlock( hCurrentVertexBuffer );

				hCurrentVertexBuffer = LakeRenderFlushVertexBuffer();

				pulColStream = EngineVertexBufferLockColourStream( hCurrentVertexBuffer, &nColStride );
				pxPosStream = EngineVertexBufferLockPositionStream( hCurrentVertexBuffer, &nPosStride );
				pfUVStream = EngineVertexBufferLockUVStream( hCurrentVertexBuffer, &nUVStride );
				nVertCount = 0;
			}
		}

		EngineVertexBufferUnlock( hCurrentVertexBuffer );

		if ( msnLakeCurrentVertexBufferPos > 0 )
		{
			hCurrentVertexBuffer = LakeRenderFlushVertexBuffer();
		}

	}
	return( TRUE );
}




void	Lake::Init( int nLakeID, VECT* pxPos, float fRadius )
{
int		nMinX;
int		nMinY;
int		nMaxX;
int		nMaxY;
int		nLoopX;
int		nLoopY;
float	fDist;
VECT	xDist;
float	fMod;
int		nMapRadius;
float	fCurrHeight;
float	fDepthScale = 3.0f;
int		nMapOriginX;
int		nMapOriginY;
float	fMapToWorldScale = LandscapeMapToWorldScale();
float	fRimHeight = 999999.0f;

	mnLakeID = nLakeID;
	mxOrigin = *pxPos;
	mfRadius = fRadius;

	// todo - flatten the landscape around the origin..
	LandscapeGetMapCoord( pxPos, &nMapOriginX, &nMapOriginY );
	nMapRadius = (int)( fRadius * LandscapeWorldToMapScale() ) + 1;

	nMinX = nMapOriginX- nMapRadius;
	nMinY = nMapOriginY - nMapRadius;
	nMaxX = nMapOriginX + nMapRadius;
	nMaxY = nMapOriginY + nMapRadius;

	// Pass 1 - Find the lowest point in the radius.. that will be the highest point of the lake hole
	for ( nLoopX = nMinX; nLoopX < nMaxX; nLoopX++ )
	{
		for ( nLoopY = nMinY; nLoopY < nMaxY; nLoopY++ )
		{
//			if ( fMod > 1.0f ) fMod = 1.0f;
			fCurrHeight = LandscapeHeightmapGetHeight( nLoopX, nLoopY );
			if ( fCurrHeight < fRimHeight )
			{
				fRimHeight = fCurrHeight;
			}
		}
	}


	// Just a simple hole
	for ( nLoopX = nMinX; nLoopX < nMaxX; nLoopX++ )
	{
		for ( nLoopY = nMinY; nLoopY < nMaxY; nLoopY++ )
		{
			xDist.x = (nLoopX - nMapOriginX) * fMapToWorldScale;
			xDist.y = (nLoopY - nMapOriginY) * fMapToWorldScale;
			xDist.z = 0.0f;
			fDist = VectGetLength( &xDist );
			fMod = fDist / fRadius;
//			if ( fMod > 1.0f ) fMod = 1.0f;
			fMod *= A90;
			fMod = cosf( fMod );

			if ( fMod > 0.0f )
			{
				fCurrHeight = fRimHeight - (fMod * fDepthScale);
				LandscapeModifyHeight( nLoopX, nLoopY, fCurrHeight );
			}
			else 
			{
				fCurrHeight = LandscapeHeightmapGetHeight( nLoopX, nLoopY );
				fMod = 1.0f + fMod;
				fCurrHeight = (fMod * fRimHeight) + ((1.0f - fMod) * fCurrHeight );
				LandscapeModifyHeight( nLoopX, nLoopY, fCurrHeight );
			}
			MapTileDataSetLakeID( nLoopX, nLoopY, mnLakeID );
		}
	}

	mfWaterHeight = fRimHeight - (0.1f * fDepthScale);

}
	

//-----------------------------------------------------------------------
Lake*	LakesFindID( int nLakeID )
{
Lake*	pLake = mspLakesList;

	while( pLake )
	{
		if ( pLake->GetLakeID() == nLakeID )
		{
			return( pLake );
		}
		pLake = pLake->GetNext();
	}
	return( NULL );
}


void	LakesMapPacketHandler( int nPacketType, void* pxMapPacket )
{
MAP_PACKET_CRATER*		pxLake = (MAP_PACKET_CRATER*)( pxMapPacket );
float	fRadius;
VECT	xPos;

	fRadius = (float)( pxLake->nSize ) / 1000.0f;
	xPos = LandscapeGetWorldPos( pxLake->wMapX, pxLake->wMapY );

	LakesAddToScene( &xPos, fRadius );
}


void		LakesFree( void )
{
int		nLoop;
Lake*	pLakeList = mspLakesList;
Lake*	pNext;

	for( nLoop = 0; nLoop < LAKE_NUM_VERTEX_BUFFERS; nLoop++ )
	{
		EngineVertexBufferFree( mahLakeVertexBuffers[nLoop] );
		mahLakeVertexBuffers[nLoop] = NOTFOUND;
	}

	while( pLakeList )
	{
		pNext = pLakeList->GetNext();
		delete pLakeList;
		pLakeList = pNext;
	}
	mspLakesList = NULL;

}

void		LakesInit( void )
{
int		nLoop;

	MapDataRegisterPacketHandler( MAP_LAKE, LakesMapPacketHandler );

	// Create vertex buffers
	for( nLoop = 0; nLoop < LAKE_NUM_VERTEX_BUFFERS; nLoop++ )
	{
		mahLakeVertexBuffers[nLoop] = EngineCreateVertexBuffer( LAKE_VERTEX_BUFFER_SIZE, 0 );
	}
	msnLakeCurrentVertexBufferPos = 0;
	msnLakeCurrentVertexBufferIndex = 0;

	// Generate a test lake..
	float	fRadius;
	VECT	xPos;

	fRadius = 30.0f;
	xPos.x = 1780.0f;
	xPos.y = 170.0f;
	xPos.z = LandscapeHeightmapGetHeightWorld( xPos.x, xPos.y );

	LakesAddToScene( &xPos, fRadius );
}


void		LakesUpdate( float fDelta )
{


}


void		LakesAddTile( int nMapX, int nMapY )
{
int		nLakeID;

	nLakeID = MapTileDataGetLakeID( nMapX, nMapY );
	if ( nLakeID != 0 )
	{
	Lake*		pLake = LakesFindID( nLakeID );

		if ( pLake )
		{
			// todo - replace.. used fixed buffer size (?)
		LakeTileRenderList*		pRenderList = new LakeTileRenderList;

			pRenderList->mpNext = mspLakeTileRenderList;
			mspLakeTileRenderList = pRenderList;

			pRenderList->mnMapX = nMapX;
			pRenderList->mnMapY = nMapY;
			pRenderList->mfWaterHeight = pLake->GetWaterHeight();
			pRenderList->mnLakeID = nLakeID;
		}
	}

}


void		LakesRender( void )
{
LakeTileRenderList*		pRenderList;
LakeTileRenderList*		pNext;

	EngineEnableBlend( TRUE );
	EngineSetBlendMode( BLEND_MODE_ALPHABLEND );
	EngineEnableLighting( FALSE );
	EngineSetTexture( 0, SeaGetTextureHandle() );

	LakeRenderRenderLists();

	pRenderList = mspLakeTileRenderList;
	while( pRenderList )
	{
		pNext = pRenderList->mpNext;
		delete pRenderList;
		pRenderList = pNext;
	}
	mspLakeTileRenderList = NULL;

}

//------------------------------------------

void		LakesAddToScene( VECT* pxOrigin, float fRadius )
{
Lake*		pNewLake = new Lake;

	pNewLake->SetNext( mspLakesList );
	mspLakesList = pNewLake;

	pNewLake->Init( msnNextLakeID, pxOrigin, fRadius );

}


