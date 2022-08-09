
#include <stdio.h>
#include <StandardDef.h>
#include <Engine.h>
#include "Interface.h"

#include "MapData/MapTileData.h"
#include "Nature/Trees.h"
#include "Nature/Grass.h"
#include "Nature/Lakes.h"
#include "LandscapeTextures.h"
#include "LandscapeHeightmap.h"
#include "LandscapeMap.h"
#include "LandscapeCoords.h"
#include "LandscapeRender.h"
#include "LandscapeCollision.h"
#include "Sea.h"

#define		LANDSCAPE_VERTEX_BUFFER_SIZE		512
#define		LANDSCAPE_NUM_VERTEX_BUFFERS		32

typedef struct
{
	int		nMapX;
	int		nMapY;
	int		nResolution;

} RENDER_TILE_ITEM;


class LandscapeTileRenderList
{
public:
	LandscapeTileRenderList()
	{
		mnRenderListSize = 0;
		mnRenderListPos = 0;
		mnLandscapeTextureNum = 0;
		mpTileItemBuffer = NULL;
	}
	~LandscapeTileRenderList()
	{
		if ( mpTileItemBuffer != NULL )
		{
			free( mpTileItemBuffer );
		}
	}

	void	AddTile( int nTextureNum, int nMapX, int nMapY );

	BOOL	RenderList( void );
	void	ResetList( void );

	LandscapeTileRenderList*		GetNext( void ) { return( mpNextRenderList ); }
	void							SetNext( LandscapeTileRenderList* pNext ) { mpNextRenderList = pNext; }

private:
	void		InitRenderList( int nTextureNum );

	int						mnRenderListSize;
	int						mnRenderListPos;
	int						mnLandscapeTextureNum;

	RENDER_TILE_ITEM*		mpTileItemBuffer;

	LandscapeTileRenderList*		mpNextRenderList;
};

class LandscapeTileRenderListManager
{
public:
	LandscapeTileRenderListManager();
	
	void				RenderTileLists( void );
	void				ResetTileLists( void );

	LandscapeTileRenderList*	GetRenderList( int nTextureNum );

	LandscapeTileRenderList*	mpTileRenderLists;
	LandscapeTileRenderList*	mapTileRenderListLookup[LARGEST_LANDSCAPE_TEXTURE_NUM];
};

//--------------------------------------------------------------------------------------

LandscapeTileRenderListManager*		mspLandscapeRenderListManager = NULL;
int			mahVertexBuffers[LANDSCAPE_NUM_VERTEX_BUFFERS] = { NOTFOUND, NOTFOUND };
int			msnCurrentVertexBufferPos = 0;
int			msnCurrentVertexBufferIndex = 0;
BOOL		msbLandscapeRenderHasInitialised = FALSE;
BOOL		msbLandscapeDebugDisplayOn = FALSE;

void		LandscapeShowDebug( BOOL bDebugOn )
{
	msbLandscapeDebugDisplayOn = bDebugOn;
}


int		LandscapeRenderFlushVertexBuffer( void )
{
	EngineEnableBlend( FALSE );
	EngineVertexBufferAddVertsUsed( mahVertexBuffers[ msnCurrentVertexBufferIndex ], msnCurrentVertexBufferPos );
	EngineVertexBufferRender( mahVertexBuffers[ msnCurrentVertexBufferIndex ], TRIANGLE_LIST );
	EngineVertexBufferReset( mahVertexBuffers[ msnCurrentVertexBufferIndex ] );

	msnCurrentVertexBufferIndex++;
	msnCurrentVertexBufferIndex %= LANDSCAPE_NUM_VERTEX_BUFFERS;
	msnCurrentVertexBufferPos = 0;

	return(	 mahVertexBuffers[ msnCurrentVertexBufferIndex ] );
}


//--------------------------------------------------------------------------------------
void	LandscapeTileRenderList::AddTile( int nTextureNum, int nMapX, int nMapY )
{
RENDER_TILE_ITEM*		pNewItem;

	if ( mnRenderListSize == 0 )
	{
		InitRenderList( nTextureNum );
	}
	
	if ( mnRenderListPos >= mnRenderListSize )
	{
	RENDER_TILE_ITEM*		pNewBuffer;
		
		// realloc
		pNewBuffer = (RENDER_TILE_ITEM*)malloc( (mnRenderListSize * 2) * sizeof( RENDER_TILE_ITEM ) ) ;
		memcpy( pNewBuffer, mpTileItemBuffer, mnRenderListSize * sizeof( RENDER_TILE_ITEM ) );
		free( mpTileItemBuffer );
		mpTileItemBuffer = pNewBuffer;

		mnRenderListSize *= 2;
	}
	
	pNewItem = mpTileItemBuffer + mnRenderListPos;
	pNewItem->nMapX = nMapX;
	pNewItem->nMapY = nMapY;

	mnRenderListPos++;
}

void	LandscapeTileRenderList::ResetList( void )
{
	mnRenderListPos = 0;
}


BOOL	LandscapeTileRenderList::RenderList( void )
{
	if ( mnRenderListPos > 0 )
	{
	int		nLoop;
	int		hCurrentVertexBuffer = mahVertexBuffers[ msnCurrentVertexBufferIndex ];
	ulong*		pulColStream = NULL;
	int			nColStride = 0;
	VECT*		pxPosStream = NULL;
	int			nPosStride = 0;
	VECT*		pxNormalStream = NULL;
	int			nNormalStride = 0;
	float*		pfUVStream = NULL;
	int			nUVStride = 0;
	int			nVertCount = 0;
	RENDER_TILE_ITEM*		pItems = mpTileItemBuffer;
	float		fUBase = 0.0f;
	float		fVBase = 0.0f;
	float		fUWidth = 1.0f;
	float		fVHeight = 1.0f;
	ulong		ulCol = 0xFFFFFFFF;
	float		fMapToWorldSize = LandscapeMapToWorldScale();
	BOOL		bNeedsSeaTile = FALSE;
	float		fSeaHeight = SeaGetHeight();

		pulColStream = EngineVertexBufferLockColourStream( hCurrentVertexBuffer, &nColStride );
		pxPosStream = EngineVertexBufferLockPositionStream( hCurrentVertexBuffer, &nPosStride );
		pfUVStream = EngineVertexBufferLockUVStream( hCurrentVertexBuffer, &nUVStride );
		pxNormalStream = EngineVertexBufferLockNormalStream( hCurrentVertexBuffer, &nNormalStride );

		if ( pulColStream )
		{
			// Set texture..
			LandscapeTextureApply( mnLandscapeTextureNum, pItems->nMapX, pItems->nMapY );

			for ( nLoop = 0; nLoop < mnRenderListPos; nLoop++ )
			{
				// Get UVs for tile
				LandscapeTextureGetUVs( mnLandscapeTextureNum, pItems->nMapX, pItems->nMapY, &fUBase, &fVBase, &fUWidth, &fVHeight );
				// Tri 1 - vert 1
				*pulColStream = ulCol;
				pxPosStream->x = (float)(pItems->nMapX - 0.0f) * fMapToWorldSize;
				pxPosStream->y = (float)(pItems->nMapY - 0.0f) * fMapToWorldSize;
				pxPosStream->z = LandscapeHeightmapGetHeight( pItems->nMapX, pItems->nMapY );
				pfUVStream[0] = fUBase;
				pfUVStream[1] = fVBase;
				*pxNormalStream = LandscapeGetNormal( pItems->nMapX, pItems->nMapY );
				
				if ( ( !bNeedsSeaTile ) &&
					 ( pxPosStream->z < fSeaHeight ) )
				{
					bNeedsSeaTile = TRUE;
				}

				pxPosStream = (VECT*)( ( (byte*)pxPosStream) + nPosStride );
				pulColStream = (ulong*)( ( (byte*)pulColStream) + nColStride );
				pfUVStream = (float*)( ( (byte*)pfUVStream) + nUVStride );
				pxNormalStream = (VECT*)( ( (byte*)pxNormalStream) + nNormalStride );

				// Tri 1 - vert 2
				*pulColStream = ulCol;
				pxPosStream->x = (float)(pItems->nMapX + 1.0f) * fMapToWorldSize;
				pxPosStream->y = (float)(pItems->nMapY - 0.0f) * fMapToWorldSize;
				pxPosStream->z = LandscapeHeightmapGetHeight( pItems->nMapX + 1, pItems->nMapY );
				pfUVStream[0] = fUWidth;
				pfUVStream[1] = fVBase;
				*pxNormalStream = LandscapeGetNormal( pItems->nMapX + 1, pItems->nMapY );
				if ( ( !bNeedsSeaTile ) &&
					 ( pxPosStream->z < fSeaHeight ) )
				{
					bNeedsSeaTile = TRUE;
				}

				pxPosStream = (VECT*)( ( (byte*)pxPosStream) + nPosStride );
				pulColStream = (ulong*)( ( (byte*)pulColStream) + nColStride );
				pfUVStream = (float*)( ( (byte*)pfUVStream) + nUVStride );
				pxNormalStream = (VECT*)( ( (byte*)pxNormalStream) + nNormalStride );

				// Tri 1 - vert 3
				*pulColStream = ulCol;
				pxPosStream->x = (float)(pItems->nMapX - 0.0f) * fMapToWorldSize;
				pxPosStream->y = (float)(pItems->nMapY + 1.0f) * fMapToWorldSize;
				pxPosStream->z = LandscapeHeightmapGetHeight( pItems->nMapX, pItems->nMapY + 1 );
				pfUVStream[0] = fUBase;
				pfUVStream[1] = fVHeight;
				*pxNormalStream = LandscapeGetNormal( pItems->nMapX, pItems->nMapY + 1 );
				if ( ( !bNeedsSeaTile ) &&
					 ( pxPosStream->z < fSeaHeight ) )
				{
					bNeedsSeaTile = TRUE;
				}

				pxPosStream = (VECT*)( ( (byte*)pxPosStream) + nPosStride );
				pulColStream = (ulong*)( ( (byte*)pulColStream) + nColStride );
				pfUVStream = (float*)( ( (byte*)pfUVStream) + nUVStride );
				pxNormalStream = (VECT*)( ( (byte*)pxNormalStream) + nNormalStride );

				//-------------------------

				// Tri 2- vert 1
				*pulColStream = ulCol;
				pxPosStream->x = (float)(pItems->nMapX + 1.0f) * fMapToWorldSize;
				pxPosStream->y = (float)(pItems->nMapY - 0.0f) * fMapToWorldSize;
				pxPosStream->z = LandscapeHeightmapGetHeight( pItems->nMapX + 1, pItems->nMapY );
				pfUVStream[0] = fUWidth;
				pfUVStream[1] = fVBase;
				*pxNormalStream = LandscapeGetNormal( pItems->nMapX + 1, pItems->nMapY );

				pxPosStream = (VECT*)( ( (byte*)pxPosStream) + nPosStride );
				pulColStream = (ulong*)( ( (byte*)pulColStream) + nColStride );
				pfUVStream = (float*)( ( (byte*)pfUVStream) + nUVStride );
				pxNormalStream = (VECT*)( ( (byte*)pxNormalStream) + nNormalStride );

				// Tri 2 - vert 2
				*pulColStream = ulCol;
				pxPosStream->x = (float)(pItems->nMapX + 1.0f) * fMapToWorldSize;
				pxPosStream->y = (float)(pItems->nMapY + 1.0f) * fMapToWorldSize;
				pxPosStream->z = LandscapeHeightmapGetHeight( pItems->nMapX + 1, pItems->nMapY + 1 );
				pfUVStream[0] = fUWidth;
				pfUVStream[1] = fVHeight;
				*pxNormalStream = LandscapeGetNormal( pItems->nMapX + 1, pItems->nMapY + 1 );
				if ( ( !bNeedsSeaTile ) &&
					 ( pxPosStream->z < fSeaHeight ) )
				{
					bNeedsSeaTile = TRUE;
				}

				pxPosStream = (VECT*)( ( (byte*)pxPosStream) + nPosStride );
				pulColStream = (ulong*)( ( (byte*)pulColStream) + nColStride );
				pfUVStream = (float*)( ( (byte*)pfUVStream) + nUVStride );
				pxNormalStream = (VECT*)( ( (byte*)pxNormalStream) + nNormalStride );

				// Tri 2 - vert 3
				*pulColStream = ulCol;
				pxPosStream->x = (float)(pItems->nMapX - 0.0f) * fMapToWorldSize;
				pxPosStream->y = (float)(pItems->nMapY + 1.0f) * fMapToWorldSize;
				pxPosStream->z = LandscapeHeightmapGetHeight( pItems->nMapX, pItems->nMapY + 1 );
				pfUVStream[0] = fUBase;
				pfUVStream[1] = fVHeight;
				*pxNormalStream = LandscapeGetNormal( pItems->nMapX, pItems->nMapY + 1 );

				pxPosStream = (VECT*)( ( (byte*)pxPosStream) + nPosStride );
				pulColStream = (ulong*)( ( (byte*)pulColStream) + nColStride );
				pfUVStream = (float*)( ( (byte*)pfUVStream) + nUVStride );
				pxNormalStream = (VECT*)( ( (byte*)pxNormalStream) + nNormalStride );

				if ( bNeedsSeaTile )
				{
					SeaRenderAddTile( pItems->nMapX, pItems->nMapY );
				}

				pItems++;
				nVertCount += 6;
				msnCurrentVertexBufferPos += 6;
				if ( nVertCount >= LANDSCAPE_VERTEX_BUFFER_SIZE - 6 )
				{
					EngineVertexBufferUnlock( hCurrentVertexBuffer );

					hCurrentVertexBuffer = LandscapeRenderFlushVertexBuffer();

					pulColStream = EngineVertexBufferLockColourStream( hCurrentVertexBuffer, &nColStride );
					pxPosStream = EngineVertexBufferLockPositionStream( hCurrentVertexBuffer, &nPosStride );
					pfUVStream = EngineVertexBufferLockUVStream( hCurrentVertexBuffer, &nUVStride );
					pxNormalStream = EngineVertexBufferLockNormalStream( hCurrentVertexBuffer, &nNormalStride );
					nVertCount = 0;
				}
			}

			EngineVertexBufferUnlock( hCurrentVertexBuffer );

			if ( msnCurrentVertexBufferPos > 0 )
			{
				hCurrentVertexBuffer = LandscapeRenderFlushVertexBuffer();
			}

		}

		return( TRUE );
	}
	return( FALSE );
}



void	LandscapeTileRenderList::InitRenderList( int nTextureNum )
{
	mnRenderListSize = 32;
	mnRenderListPos = 0;
	mnLandscapeTextureNum = nTextureNum;

	mpTileItemBuffer = (RENDER_TILE_ITEM*)( malloc( mnRenderListSize * sizeof( RENDER_TILE_ITEM ) ) );
	if ( mpTileItemBuffer == NULL )
	{
		mnRenderListSize = 0;
	}
}


//--------------------------------------------------------------------------------------
LandscapeTileRenderListManager::LandscapeTileRenderListManager()
{
	mpTileRenderLists = NULL;
	memset( mapTileRenderListLookup, 0, sizeof( LandscapeTileRenderList* ) * LARGEST_LANDSCAPE_TEXTURE_NUM );
}


void	LandscapeTileRenderListManager::ResetTileLists( void )
{
LandscapeTileRenderList*		pTileRenderList = mpTileRenderLists;
LandscapeTileRenderList*		pNext;

	while( pTileRenderList )
	{
		pNext = pTileRenderList->GetNext();
		delete pTileRenderList;
		pTileRenderList = pNext;
	}
	mpTileRenderLists = NULL;

	memset( mapTileRenderListLookup, 0, sizeof( LandscapeTileRenderList* ) * LARGEST_LANDSCAPE_TEXTURE_NUM );
}

void		LandscapeTileRenderListManager::RenderTileLists( void )
{
LandscapeTileRenderList*		pTileRenderList = mpTileRenderLists;
int		nTextureCount = 0;

	while( pTileRenderList )
	{
		if ( pTileRenderList->RenderList() == TRUE )
		{
			nTextureCount++;
		}
		pTileRenderList = pTileRenderList->GetNext();
	}

	if ( msbLandscapeDebugDisplayOn )
	{
	char	acString[256];
		sprintf( acString, "%d landscape textures used", nTextureCount );
		InterfaceTextRight( 0, InterfaceGetWidth() - 10, 130, acString, 0xd0d0d0d0, 0 );
	}
}

LandscapeTileRenderList*	LandscapeTileRenderListManager::GetRenderList( int nTextureNum )
{
	if ( mapTileRenderListLookup[nTextureNum] ) 
	{
		return( mapTileRenderListLookup[nTextureNum] );
	}
	else
	{
	LandscapeTileRenderList*		pNewLandscapeTileRenderList = new LandscapeTileRenderList;

		mapTileRenderListLookup[nTextureNum] = pNewLandscapeTileRenderList;

		pNewLandscapeTileRenderList->SetNext( mpTileRenderLists );
		mpTileRenderLists = pNewLandscapeTileRenderList;

		return( pNewLandscapeTileRenderList );
	}
}

//--------------------------------------------------------------------------------------

void	LandscapeRenderListAddTile( int nMapX, int nMapY, int nResolution )
{
//LANDSCAPE_MAP_DATA*		pxMapData = LandscapeMapGetData( nMapX, nMapY );
//int		nLandscapeTextureNum = pxMapData->nLandscapeTextureNum;
int		nLandscapeTextureNum = LandscapeTextureGetTextureNum( nMapX, nMapY );
LandscapeTileRenderList*		pRenderList;

	if ( ( nLandscapeTextureNum >= 0 ) &&
		 ( nLandscapeTextureNum < LARGEST_LANDSCAPE_TEXTURE_NUM ) )
	{
	float	fGrassLevel = MapTileDataGetGrassLevel( nMapX, nMapY );

		if ( fGrassLevel > 0.0f )
		{
			GrassAddRenderTile( nMapX, nMapY, fGrassLevel );
		}

		LakesAddTile( nMapX, nMapY );

		pRenderList = mspLandscapeRenderListManager->GetRenderList( nLandscapeTextureNum );
		pRenderList->AddTile( nLandscapeTextureNum, nMapX, nMapY );
	}
	else
	{
		// ERROR - Invalid LandscapeTextureNum
	}
}

BOOL	LandscapeRenderHasInitialised( void )
{
	return( msbLandscapeRenderHasInitialised );
}

void	LandscapeRenderInit( int nLandscapeTextureSystem )
{
int		nLoop;

	if ( !msbLandscapeRenderHasInitialised )
	{
		msbLandscapeRenderHasInitialised = TRUE;

		mspLandscapeRenderListManager = new LandscapeTileRenderListManager;

		for( nLoop = 0; nLoop < LANDSCAPE_NUM_VERTEX_BUFFERS; nLoop++ )
		{
			mahVertexBuffers[nLoop] = EngineCreateVertexBuffer( LANDSCAPE_VERTEX_BUFFER_SIZE, 0 );
		}
		msnCurrentVertexBufferPos = 0;
		msnCurrentVertexBufferIndex = 0;

		LandscapeTexturesInit( nLandscapeTextureSystem );
	}
}

void	LandscapeRenderShutdown( void )
{
int		nLoop;

	for( nLoop = 0; nLoop < LANDSCAPE_NUM_VERTEX_BUFFERS; nLoop++ )
	{
		EngineVertexBufferFree( mahVertexBuffers[nLoop] );
	}

	LandscapeTexturesShutdown();

	SAFE_DELETE( mspLandscapeRenderListManager );
	msbLandscapeRenderHasInitialised = FALSE;

}


void		GetMapCoordForWorldCoord( const VECT* pxWorldPos, int* pnMapX, int* pnMapY )
{
float	fWorldToMapSize = LandscapeWorldToMapScale();

	*pnMapX = (int)( ( pxWorldPos->x * fWorldToMapSize ) + 0.5f );
	*pnMapY = (int)( ( pxWorldPos->y * fWorldToMapSize ) + 0.5f );

}




BOOL	LandscapeRenderGetViewGridCoord( VECT* pxRayStart, VECT* pxRayDir, int* pnMapX, int* pnMapY )
{
ENGINEPLANE		xPlane;
VECT	xFloorPos1 = { -1000.0f, -1000.0f, 0.0f };
VECT	xFloorPos2 = { 1000.0f, -1000.0f, 0.0f };
VECT	xFloorPos3 = { -1000.0f, 1000.0f, 0.0f };
VECT	xRayEnd;
VECT	xRay = *pxRayDir;
VECT	xIntersectPoint;

	VectScale( &xRay, &xRay, 10000.0f );
	VectAdd( &xRayEnd, pxRayStart, &xRay );
	
	EnginePlaneFromPoints( &xPlane, &xFloorPos1, &xFloorPos2, &xFloorPos3 );

	// find where on the zero-floor plane the ray hits
	if ( EnginePlaneIntersectLine( &xIntersectPoint, &xPlane, pxRayStart, &xRayEnd ) == TRUE )
	{
		LandscapeGetMapCoord( &xIntersectPoint, pnMapX, pnMapY );
		return( TRUE );
	}
	return( FALSE );
}

// new version
BOOL	 LandscapeRenderTopdown( int nFocusLoc )
{
VECT	xRayStart;
VECT	xRayDir;
//	VECT	xRayEnd;
//VECT	xIntersectPoint;
BOOL	bDidIntersectLandscape;
int		anGridX[4];
int		anGridY[4];
int		nLoop;
int		nMinX = 0;
int		nMaxX = 0;
int		nMinY = 0;
int		nMaxY = 0;
int		nX, nY;
int		nOffscreenBoundarySize = 2;
int		nExtraTiles = 5;

	for ( nLoop = 0; nLoop < 4; nLoop++ ) 
	{
		switch( nLoop )
		{
		case 0:
			EngineGetRayForScreenCoord( 0 - nOffscreenBoundarySize, 0 - nOffscreenBoundarySize, &xRayStart, &xRayDir );
			break;
		case 1:
			EngineGetRayForScreenCoord( InterfaceGetWidth() + nOffscreenBoundarySize, 0 - nOffscreenBoundarySize, &xRayStart, &xRayDir );
			break;
		case 2:
			EngineGetRayForScreenCoord( 0 - nOffscreenBoundarySize, InterfaceGetHeight() + nOffscreenBoundarySize, &xRayStart, &xRayDir );
			break;
		case 3:
			EngineGetRayForScreenCoord( InterfaceGetWidth() + nOffscreenBoundarySize, InterfaceGetHeight() + nOffscreenBoundarySize, &xRayStart, &xRayDir );
			break;
		}

		bDidIntersectLandscape = LandscapeRenderGetViewGridCoord( &xRayStart, &xRayDir, &anGridX[nLoop], &anGridY[nLoop] );
		if ( !bDidIntersectLandscape )
		{
			// Camera isnt pointing fully into the ground, so can't use the routine here..
			return( FALSE );
		}
/*
		bDidIntersectLandscape = LandscapeRayTest( &xRayStart, &xRayDir, &xIntersectPoint, FALSE );
		if ( !bDidIntersectLandscape )
		{
			// Camera isnt pointing fully into the ground, so can't use the routine here..
			return( FALSE );
		}

		GetMapCoordForWorldCoord( &xIntersectPoint, &anGridX[nLoop], &anGridY[nLoop] );
*/
	}

	// If we get here, all corner rays hit the landscape so we can proceed..

	// Find smallest and largest
	nMinX = anGridX[0];
	nMaxX = nMinX;
	nMinY = anGridY[0];
	nMaxY = nMinY;
	for ( nLoop = 1; nLoop < 4; nLoop++ ) 
	{
		if ( anGridX[nLoop] < nMinX )
		{
			nMinX = anGridX[nLoop];
		}
		else if ( anGridX[nLoop] > nMaxX )
		{
			nMaxX = anGridX[nLoop];
		}
		if ( anGridY[nLoop] < nMinY )
		{
			nMinY = anGridY[nLoop];
		}
		else if ( anGridY[nLoop] > nMaxY )
		{
			nMaxY = anGridY[nLoop];
		}
	}

	if ( msbLandscapeDebugDisplayOn )
	{
	char	acString[256];
		sprintf( acString, "Land render %dx%d to %dx%d (%d tiles)", nMinX, nMinY, nMaxX, nMaxY, (nMaxX-nMinX)*(nMaxY-nMinY) );
		InterfaceTextRight( 0, InterfaceGetWidth()-10, 110, acString, 0xd0d0d0d0, 0 );
	}

/*
	nMinY = 10;
	nMaxY = 55;
	nMinX = 10;
	nMaxX = 55;
*/
	TreesSetView( nMinX, nMinY, nMaxX, nMaxY );

	for ( nY = nMinY - nExtraTiles ; nY < nMaxY + nExtraTiles; nY++ )
	{
		for ( nX = nMinX - nExtraTiles; nX < (nMaxX + nExtraTiles); nX++ )
		{
			LandscapeRenderListAddTile( nX, nY, 1 );			
		}
	}
	return( TRUE );
}


void	 LandscapeRenderFlushTiles( void )
{
	EngineCameraUpdate();
//	EngineSceneShadowsStartRender( TRUE, FALSE, FALSE );

	mspLandscapeRenderListManager->RenderTileLists();
	SeaRenderFlush();

//	EngineSceneShadowsEndRender();

}

void		LandscapeRenderNewFrame( void )
{
	if ( mspLandscapeRenderListManager )
	{
		mspLandscapeRenderListManager->ResetTileLists();
	}
	msnCurrentVertexBufferIndex = 0;

}
