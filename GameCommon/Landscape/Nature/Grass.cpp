
#include "StandardDef.h"
#include "Engine.h"

#include "Grass.h"

#define		NUM_GRASS_VERTEX_BUFFERS		2
#define		SIZE_OF_GRASS_VERTEX_BUFFER		(6*1024)


class GrassTileRenderItem
{
public:
	GrassTileRenderItem()
	{
		mpNext = NULL;
	}

	void		Init( int nMapX, int nMapY, float fGrassLevel )
	{
		mnTileMapX = nMapX;
		mnTileMapY = nMapY;
		mfGrassLevel = fGrassLevel;
	}

	void		Render( void );

	GrassTileRenderItem*		GetNext( void ) { return( mpNext ); }
	void			SetNext( GrassTileRenderItem* pTile ) { mpNext = pTile; }
private:

	int				mnTileMapX;
	int				mnTileMapY;

	float			mfGrassLevel;

	GrassTileRenderItem*		mpNext;
};

GrassTileRenderItem*		mspGrassTilesRenderList = NULL;

	// todo -... use :)
int		mahGrassVertexBuffers[NUM_GRASS_VERTEX_BUFFERS] = { NOTFOUND, NOTFOUND };
int		mnCurrentGrassVertexBufferIndex = 0;

int		mnGrassVertexBufferPos = 0;

int		mhGrassVertexBuffer = NOTFOUND;
int		mnNumGrassBlades = 200;


void		GrassGenerateVertexBuffer( int hVertexBuffer )
{
int		nLoop;
uint32*		pulColStream = NULL;
int			nColStride = 0;
VECT*		pxPosStream = NULL;
int			nPosStride = 0;
float*		pfUVStream = NULL;
int			nUVStride = 0;
VECT*		pxNormStream = NULL;
int			nNormStride = 0;
int			nVertCount = 0;
float		fUBase = 0.0f;
float		fVBase = 0.0f;
float		fUWidth = 1.0f;
float		fVHeight = 1.0f;
float		fHeightMod = 0.3f;
uint32		ulCol = 0xFFFFFFFF;
//uint32		ulCol = 0xFF000000;
float		fGrassSizeX = 0.1f;
float		fGrassSizeZ = 0.7f;

	pulColStream = EngineVertexBufferLockColourStream( hVertexBuffer, &nColStride );
	pxPosStream = EngineVertexBufferLockPositionStream( hVertexBuffer, &nPosStride );
	pxNormStream = EngineVertexBufferLockNormalStream( hVertexBuffer, &nNormStride );
	pfUVStream = EngineVertexBufferLockUVStream( hVertexBuffer, &nUVStride );

	if ( pulColStream )
	{
		for ( nLoop = 0; nLoop < mnNumGrassBlades; nLoop++ )
		{
			// Tri 1 - vert 1
			*pulColStream = ulCol;
			pxPosStream->x = -fGrassSizeX;
			pxPosStream->y = 0.0f;
			pxPosStream->z = fGrassSizeZ;
			pfUVStream[0] = fUBase;
			pfUVStream[1] = fVBase;

			pxPosStream = (VECT*)( ( (byte*)pxPosStream) + nPosStride );
			pxNormStream = (VECT*)( ( (byte*)pxNormStream) + nNormStride );
			pulColStream = (uint32*)( ( (byte*)pulColStream) + nColStride );
			pfUVStream = (float*)( ( (byte*)pfUVStream) + nUVStride );

			// Tri 1 - vert 2
			*pulColStream = ulCol;
			pxPosStream->x = fGrassSizeX;
			pxPosStream->y = 0.0f;
			pxPosStream->z = fGrassSizeZ;
			pfUVStream[0] = fUBase + fUWidth;
			pfUVStream[1] = fVBase;

			pxPosStream = (VECT*)( ( (byte*)pxPosStream) + nPosStride );
			pxNormStream = (VECT*)( ( (byte*)pxNormStream) + nNormStride );
			pulColStream = (uint32*)( ( (byte*)pulColStream) + nColStride );
			pfUVStream = (float*)( ( (byte*)pfUVStream) + nUVStride );

			// Tri 1 - vert 3
			*pulColStream = ulCol;
			pxPosStream->x = -fGrassSizeX;
			pxPosStream->y = 0.0f;
			pxPosStream->z = -fGrassSizeZ;
			pfUVStream[0] = fUBase;
			pfUVStream[1] = fVBase + fVHeight;

			pxPosStream = (VECT*)( ( (byte*)pxPosStream) + nPosStride );
			pxNormStream = (VECT*)( ( (byte*)pxNormStream) + nNormStride );
			pulColStream = (uint32*)( ( (byte*)pulColStream) + nColStride );
			pfUVStream = (float*)( ( (byte*)pfUVStream) + nUVStride );

			//-------------------------

			// Tri 2- vert 1
			*pulColStream = ulCol;
			pxPosStream->x = fGrassSizeX;
			pxPosStream->y = 0.0f;
			pxPosStream->z = fGrassSizeZ;
			pfUVStream[0] = fUBase + fUWidth;
			pfUVStream[1] = fVBase;

			pxPosStream = (VECT*)( ( (byte*)pxPosStream) + nPosStride );
			pxNormStream = (VECT*)( ( (byte*)pxNormStream) + nNormStride );
			pulColStream = (uint32*)( ( (byte*)pulColStream) + nColStride );
			pfUVStream = (float*)( ( (byte*)pfUVStream) + nUVStride );

			// Tri 2 - vert 2
			*pulColStream = ulCol;
			pxPosStream->x = fGrassSizeX;
			pxPosStream->y = 0.0f;
			pxPosStream->z = -fGrassSizeZ;
			pfUVStream[0] = fUBase + fUWidth;
			pfUVStream[1] = fVBase + fVHeight;

			pxPosStream = (VECT*)( ( (byte*)pxPosStream) + nPosStride );
			pxNormStream = (VECT*)( ( (byte*)pxNormStream) + nNormStride );
			pulColStream = (uint32*)( ( (byte*)pulColStream) + nColStride );
			pfUVStream = (float*)( ( (byte*)pfUVStream) + nUVStride );

			// Tri 2 - vert 3
			*pulColStream = ulCol;
			pxPosStream->x = -fGrassSizeX;
			pxPosStream->y = 0.0f;
			pxPosStream->z = -fGrassSizeZ;
			pfUVStream[0] = fUBase;
			pfUVStream[1] = fVBase + fVHeight;

			pxPosStream = (VECT*)( ( (byte*)pxPosStream) + nPosStride );
			pxNormStream = (VECT*)( ( (byte*)pxNormStream) + nNormStride );
			pulColStream = (uint32*)( ( (byte*)pulColStream) + nColStride );
			pfUVStream = (float*)( ( (byte*)pfUVStream) + nUVStride );

			nVertCount += 6;
//				msnCurrentVertexBufferPos += 6;

		}	// loop
		EngineVertexBufferUnlock( hVertexBuffer );
	}
}

void	GrassTileRenderItem::Render( void )
{


//	EngineVertexBufferRender( 

}



void		GrassInit( void )
{
	mhGrassVertexBuffer = EngineCreateVertexBuffer( mnNumGrassBlades*6, 0 );

	GrassGenerateVertexBuffer( mhGrassVertexBuffer );
	
}

void		GrassUpdate( float fDelta )
{


}


void		GrassRender( void )
{
GrassTileRenderItem*	pRenderTile = mspGrassTilesRenderList;
GrassTileRenderItem*	pNext;

	while( pRenderTile )
	{
		pRenderTile->Render();
		pRenderTile = pRenderTile->GetNext();
	}


	pRenderTile = mspGrassTilesRenderList;

	while( pRenderTile )
	{
		pNext = pRenderTile->GetNext();
		delete pRenderTile;
		pRenderTile = pNext;
	}
	mspGrassTilesRenderList = NULL;
}

void		GrassFree( void )
{

}


void		GrassAddRenderTile( int nMapX, int nMapY, float fGrassLevel )
{
	return;

	// TODO - Replace with a pre-allocated buffer which we use to control amount of grass rendered
GrassTileRenderItem*	pNewRenderTile = new GrassTileRenderItem;

	pNewRenderTile->SetNext( mspGrassTilesRenderList );
	mspGrassTilesRenderList = pNewRenderTile;

	pNewRenderTile->Init( nMapX, nMapY, fGrassLevel );	
}

void		GrassAddMapPacket( GRASS_MAP_PACKET* pPacket )
{


}





