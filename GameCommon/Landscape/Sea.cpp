

#include "StandardDef.h"
#include "Engine.h"

#include "LandscapeCoords.h"
#include "Sea.h"

#define MAX_SEA_TILES		2048

typedef struct
{
	int			nMapX;
	int			nMapY;

} SEA_RENDER_TILES;

int			mnMaxSeaTiles = MAX_SEA_TILES;
float		msfSeaHeight = 1.0f;		// NOTE: This is heightmap scale (0 -> 255.0f) x landscape height scale (currently 0.5)

SEA_RENDER_TILES		maSeaRenderTiles[MAX_SEA_TILES];
int			hSeaVertexBuffer = NOTFOUND;
int			mnSeaRenderTilePos = 0;
int			mhSeaTexture = NOTFOUND;

float	SeaGetHeight( void )
{
	return( msfSeaHeight );
}


void		SeaRenderInit( void )
{
	hSeaVertexBuffer = EngineCreateVertexBuffer( mnMaxSeaTiles * 6, 0 );
	mhSeaTexture = EngineLoadTexture( "Data\\Textures\\Sea.png", 0, 0 );
}

int		SeaGetTextureHandle( void )
{
	return( mhSeaTexture );
}


void		SeaRenderAddTile( int nMapX, int nMapY )
{
	if ( hSeaVertexBuffer != NOTFOUND )
	{
		maSeaRenderTiles[mnSeaRenderTilePos].nMapX = nMapX;
		maSeaRenderTiles[mnSeaRenderTilePos].nMapY = nMapY;
		if ( mnSeaRenderTilePos < MAX_SEA_TILES-1 )
		{
			mnSeaRenderTilePos++;
		}
	}
}


void		SeaRenderFlushVertexBuffer( int nVertCount )
{
	EngineVertexBufferRender( hSeaVertexBuffer, TRIANGLE_LIST );
	EngineVertexBufferReset( hSeaVertexBuffer );
	mnSeaRenderTilePos = 0;
}


void		SeaRenderFlush( void )
{
//	mnSeaRenderTilePos = 0;
	if ( mnSeaRenderTilePos > 0 )
	{
	int			nLoop;
	ulong*		pulColStream = NULL;
	int			nColStride = 0;
	VECT*		pxPosStream = NULL;
	int			nPosStride = 0;
	float*		pfUVStream = NULL;
	int			nUVStride = 0;
	ulong		ulCol = 0xC0507090;
	float		fUBase = 0.0f;
	float		fVBase = 0.0f;
	float		fUWidth = 1.0f;
	float		fVHeight = 1.0f;
	float		fSeaHeight = SeaGetHeight();
	SEA_RENDER_TILES*		pItems = maSeaRenderTiles;
	int			nVertCount = 0;
	float		fMapToWorldSize = LandscapeMapToWorldScale();

		ulCol = EngineGetColValue( 0x50, 0x70, 0xE0, 0x80 );
		pulColStream = EngineVertexBufferLockColourStream( hSeaVertexBuffer, &nColStride );
		pxPosStream = EngineVertexBufferLockPositionStream( hSeaVertexBuffer, &nPosStride );
		pfUVStream = EngineVertexBufferLockUVStream( hSeaVertexBuffer, &nUVStride );

		EngineEnableBlend( TRUE );
		EngineSetBlendMode( BLEND_MODE_ALPHABLEND );
		EngineSetTexture( 0, mhSeaTexture );
		EngineEnableLighting( FALSE );
		for( nLoop = 0; nLoop < mnSeaRenderTilePos; nLoop++ )
		{
			// Set texture..
	//		LandscapeTextureGetUVs( mnLandscapeTextureNum, pItems->nMapX, pItems->nMapY, &fUBase, &fVBase, &fUWidth, &fVHeight );
			// Tri 1 - vert 1
			*pulColStream = ulCol;
			pxPosStream->x = (float)(pItems->nMapX - 0.0f) * fMapToWorldSize;
			pxPosStream->y = (float)(pItems->nMapY - 0.0f) * fMapToWorldSize;
			pxPosStream->z = fSeaHeight;
			pfUVStream[0] = fUBase;
			pfUVStream[1] = fVBase;

			pxPosStream = (VECT*)( ( (byte*)pxPosStream) + nPosStride );
			pulColStream = (ulong*)( ( (byte*)pulColStream) + nColStride );
			pfUVStream = (float*)( ( (byte*)pfUVStream) + nUVStride );

			// Tri 1 - vert 2
			*pulColStream = ulCol;
			pxPosStream->x = (float)(pItems->nMapX + 1.0f) * fMapToWorldSize;
			pxPosStream->y = (float)(pItems->nMapY - 0.0f) * fMapToWorldSize;
			pxPosStream->z = fSeaHeight;
			pfUVStream[0] = fUWidth;
			pfUVStream[1] = fVBase;

			pxPosStream = (VECT*)( ( (byte*)pxPosStream) + nPosStride );
			pulColStream = (ulong*)( ( (byte*)pulColStream) + nColStride );
			pfUVStream = (float*)( ( (byte*)pfUVStream) + nUVStride );

			// Tri 1 - vert 3
			*pulColStream = ulCol;
			pxPosStream->x = (float)(pItems->nMapX - 0.0f) * fMapToWorldSize;
			pxPosStream->y = (float)(pItems->nMapY + 1.0f) * fMapToWorldSize;
			pxPosStream->z = fSeaHeight;
			pfUVStream[0] = fUBase;
			pfUVStream[1] = fVHeight;

			pxPosStream = (VECT*)( ( (byte*)pxPosStream) + nPosStride );
			pulColStream = (ulong*)( ( (byte*)pulColStream) + nColStride );
			pfUVStream = (float*)( ( (byte*)pfUVStream) + nUVStride );

			//-------------------------

			// Tri 2- vert 1
			*pulColStream = ulCol;
			pxPosStream->x = (float)(pItems->nMapX + 1.0f) * fMapToWorldSize;
			pxPosStream->y = (float)(pItems->nMapY - 0.0f) * fMapToWorldSize;
			pxPosStream->z = fSeaHeight;
			pfUVStream[0] = fUWidth;
			pfUVStream[1] = fVBase;

			pxPosStream = (VECT*)( ( (byte*)pxPosStream) + nPosStride );
			pulColStream = (ulong*)( ( (byte*)pulColStream) + nColStride );
			pfUVStream = (float*)( ( (byte*)pfUVStream) + nUVStride );

			// Tri 2 - vert 2
			*pulColStream = ulCol;
			pxPosStream->x = (float)(pItems->nMapX + 1.0f) * fMapToWorldSize;
			pxPosStream->y = (float)(pItems->nMapY + 1.0f) * fMapToWorldSize;
			pxPosStream->z = fSeaHeight;
			pfUVStream[0] = fUWidth;
			pfUVStream[1] = fVHeight;

			pxPosStream = (VECT*)( ( (byte*)pxPosStream) + nPosStride );
			pulColStream = (ulong*)( ( (byte*)pulColStream) + nColStride );
			pfUVStream = (float*)( ( (byte*)pfUVStream) + nUVStride );

			// Tri 2 - vert 3
			*pulColStream = ulCol;
			pxPosStream->x = (float)(pItems->nMapX - 0.0f) * fMapToWorldSize;
			pxPosStream->y = (float)(pItems->nMapY + 1.0f) * fMapToWorldSize;
			pxPosStream->z = fSeaHeight;
			pfUVStream[0] = fUBase;
			pfUVStream[1] = fVHeight;

			pxPosStream = (VECT*)( ( (byte*)pxPosStream) + nPosStride );
			pulColStream = (ulong*)( ( (byte*)pulColStream) + nColStride );
			pfUVStream = (float*)( ( (byte*)pfUVStream) + nUVStride );

			pItems++;
			nVertCount += 6;
		}
		EngineVertexBufferAddVertsUsed( hSeaVertexBuffer, nVertCount );
		EngineVertexBufferUnlock( hSeaVertexBuffer );
		SeaRenderFlushVertexBuffer(nVertCount);
	}
	EngineEnableBlend( FALSE );

}

void		SeaRenderFree( void )
{
	EngineVertexBufferFree( hSeaVertexBuffer );
	hSeaVertexBuffer = NOTFOUND;
	EngineReleaseTexture( &mhSeaTexture );


}
