
#include <stdio.h>

#include "StandardDef.h"
#include "Interface.h"
#include "Engine.h"
#include "Rendering.h"

#include "../LandscapeHeightmap.h"
#include "../LandscapeCoords.h"
#include "../Landscape.h"

#include "../MapData/MapData.h"

#include "TreeLists.h"
#include "Trees.h"

#define ENABLE_MAP_FORESTS		
#define ENABLE_GENERATED_FORESTS

#define		NUM_TREE_SPRITE_VERTEX_BUFFERS		2
#define		SIZEOF_TREE_VERTEX_BUFFER			(6*1024)

int			mshTempTreeSprite = NOTFOUND;
int			mshTempTreeTrunkModel = NOTFOUND;
int			mshTempTreeTrunkTexture = NOTFOUND;
Tree*		mspTreeList = NULL;

int			mahTreeSpriteVertexBuffer[NUM_TREE_SPRITE_VERTEX_BUFFERS] = { NOTFOUND, NOTFOUND };
int			mnCurrentTreeSpriteVertexBufferIndex = 0;

uint32*		mpulTreeRenderColStream = NULL;
int			mnTreeRenderColStride = 0;
VECT*		mpxTreeRenderPosStream = NULL;
int			mnTreeRenderPosStride = 0;
float*		mpfTreeRenderUVStream = NULL;
int			mnTreeRenderUVStride = 0;
int			mnTreeRenderVertexBufferPos = 0;


void	Tree::RenderTrunk( void )
{
VECT	xRot = { 0.0f, 0.0f, mfRot };
float	fCamDist;

	if ( mxPos.z == 0.0f )
	{
		mxPos.z = LandscapeHeightmapGetHeightWorld( mxPos.x, mxPos.y );
	}
	fCamDist = VectDist( &mxPos, EngineCameraGetPos() );

	if ( fCamDist < 230.0f )
	{
		ModelRender( mshTempTreeTrunkModel, &mxPos, &xRot, 0 ); // RENDERFLAG_NOFLUSH );
	}

}

void	TreeRenderSpritesFlush( BOOL bEndFrame )
{
	EngineResetColourMode();

	EngineSetColourMode( 0, COLOUR_MODE_TEXTURE_MODULATE );
	EngineSetBlendMode( BLEND_MODE_ALPHABLEND );
	EngineEnableAlphaTest( 1 );
//	EngineEnableZWrite( FALSE );
	EngineVertexBufferUnlock( mahTreeSpriteVertexBuffer[ mnCurrentTreeSpriteVertexBufferIndex ] );

	EngineVertexBufferAddVertsUsed( mahTreeSpriteVertexBuffer[ mnCurrentTreeSpriteVertexBufferIndex ], mnTreeRenderVertexBufferPos );
	EngineVertexBufferRender( mahTreeSpriteVertexBuffer[ mnCurrentTreeSpriteVertexBufferIndex ], TRIANGLE_LIST );

	mnCurrentTreeSpriteVertexBufferIndex++;
	mnCurrentTreeSpriteVertexBufferIndex %= NUM_TREE_SPRITE_VERTEX_BUFFERS;

	EngineVertexBufferReset( mahTreeSpriteVertexBuffer[ mnCurrentTreeSpriteVertexBufferIndex ] );
	mnTreeRenderVertexBufferPos = 0;

	if ( bEndFrame == FALSE )
	{
		mpulTreeRenderColStream = EngineVertexBufferLockColourStream( mahTreeSpriteVertexBuffer[ mnCurrentTreeSpriteVertexBufferIndex ], &mnTreeRenderColStride );
		mpxTreeRenderPosStream = EngineVertexBufferLockPositionStream( mahTreeSpriteVertexBuffer[ mnCurrentTreeSpriteVertexBufferIndex ], &mnTreeRenderPosStride );
		mpfTreeRenderUVStream = EngineVertexBufferLockUVStream( mahTreeSpriteVertexBuffer[ mnCurrentTreeSpriteVertexBufferIndex ], &mnTreeRenderUVStride );
	}

}

void	Tree::Render( void )
{
int			nColStride = mnTreeRenderColStride;
int			nPosStride = mnTreeRenderPosStride;
int			nUVStride = mnTreeRenderUVStride;
uint32*		pulColStream = (uint32*)( ( (byte*)mpulTreeRenderColStream) + (nColStride*mnTreeRenderVertexBufferPos) );
VECT*		pxPosStream = (VECT*)( ( (byte*)mpxTreeRenderPosStream) + (nPosStride*mnTreeRenderVertexBufferPos) );
float*		pfUVStream = (float*)( ( (byte*)mpfTreeRenderUVStream) + (nUVStride*mnTreeRenderVertexBufferPos));
uint32		ulCol = 0xFFFFFFFF;
float		fBasePosX;
float		fBasePosY;
float		fWidthX;
float		fWidthY;
float		fUBase = 0.0f;
float		fVBase = 0.0f;
float		fUWidth = 1.0f;
float		fVHeight = 1.0f;
float		fRot = mfRot;
float		fSpriteHeight = 3.5f * mfTreeSpriteSize;

	if ( mxPos.z == 0.0f )
	{
		mxPos.z = LandscapeHeightmapGetHeightWorld( mxPos.x, mxPos.y );
	}

	fBasePosX = -5.0f * mfTreeSpriteSize;
	fBasePosY = -5.0f * mfTreeSpriteSize;
	fWidthX = 10.0f * mfTreeSpriteSize;
	fWidthY = 10.0f * mfTreeSpriteSize;

	*pulColStream = ulCol;
	pxPosStream->x = fBasePosX;
	pxPosStream->y = fBasePosY;
	pxPosStream->z = fSpriteHeight;
	VectRotateAboutZ( pxPosStream, fRot );
	VectAdd( pxPosStream, &mxPos, pxPosStream );
	pfUVStream[0] = fUBase;
	pfUVStream[1] = fVBase;

	pxPosStream = (VECT*)( ( (byte*)pxPosStream) + nPosStride );
	pulColStream = (uint32*)( ( (byte*)pulColStream) + nColStride );
	pfUVStream = (float*)( ( (byte*)pfUVStream) + nUVStride );

	// Tri 1 - vert 2
	*pulColStream = ulCol;
	pxPosStream->x = fBasePosX + fWidthX;
	pxPosStream->y = fBasePosY;
	pxPosStream->z = fSpriteHeight;
	VectRotateAboutZ( pxPosStream, fRot );
	VectAdd( pxPosStream, &mxPos, pxPosStream );
	pfUVStream[0] = fUBase + fUWidth;
	pfUVStream[1] = fVBase;

	pxPosStream = (VECT*)( ( (byte*)pxPosStream) + nPosStride );
	pulColStream = (uint32*)( ( (byte*)pulColStream) + nColStride );
	pfUVStream = (float*)( ( (byte*)pfUVStream) + nUVStride );

	// Tri 1 - vert 3
	*pulColStream = ulCol;
	pxPosStream->x = fBasePosX;
	pxPosStream->y = fBasePosY + fWidthY;
	pxPosStream->z = fSpriteHeight;
	VectRotateAboutZ( pxPosStream, fRot );
	VectAdd( pxPosStream, &mxPos, pxPosStream );
	pfUVStream[0] = fUBase;
	pfUVStream[1] = fVBase + fVHeight;

	pxPosStream = (VECT*)( ( (byte*)pxPosStream) + nPosStride );
	pulColStream = (uint32*)( ( (byte*)pulColStream) + nColStride );
	pfUVStream = (float*)( ( (byte*)pfUVStream) + nUVStride );

	//-------------------------

	// Tri 2- vert 1
	*pulColStream = ulCol;
	pxPosStream->x = fBasePosX + fWidthX;
	pxPosStream->y = fBasePosY;
	pxPosStream->z = fSpriteHeight;
	VectRotateAboutZ( pxPosStream, fRot );
	VectAdd( pxPosStream, &mxPos, pxPosStream );
	pfUVStream[0] = fUBase + fUWidth;
	pfUVStream[1] = fVBase;

	pxPosStream = (VECT*)( ( (byte*)pxPosStream) + nPosStride );
	pulColStream = (uint32*)( ( (byte*)pulColStream) + nColStride );
	pfUVStream = (float*)( ( (byte*)pfUVStream) + nUVStride );

	// Tri 2 - vert 2
	*pulColStream = ulCol;
	pxPosStream->x = fBasePosX + fWidthX;
	pxPosStream->y = fBasePosY + fWidthY;
	pxPosStream->z = fSpriteHeight;
	VectRotateAboutZ( pxPosStream, fRot );
	VectAdd( pxPosStream, &mxPos, pxPosStream );
	pfUVStream[0] = fUBase + fUWidth;
	pfUVStream[1] = fVBase + fVHeight;

	pxPosStream = (VECT*)( ( (byte*)pxPosStream) + nPosStride );
	pulColStream = (uint32*)( ( (byte*)pulColStream) + nColStride );
	pfUVStream = (float*)( ( (byte*)pfUVStream) + nUVStride );

	// Tri 2 - vert 3
	*pulColStream = ulCol;
	pxPosStream->x = fBasePosX;
	pxPosStream->y = fBasePosY + fWidthY;
	pxPosStream->z = fSpriteHeight;
	VectRotateAboutZ( pxPosStream, fRot );
	VectAdd( pxPosStream, &mxPos, pxPosStream );
	pfUVStream[0] = fUBase;
	pfUVStream[1] = fVBase + fVHeight;

	pxPosStream = (VECT*)( ( (byte*)pxPosStream) + nPosStride );
	pulColStream = (uint32*)( ( (byte*)pulColStream) + nColStride );
	pfUVStream = (float*)( ( (byte*)pfUVStream) + nUVStride );

	mnTreeRenderVertexBufferPos += 6;
	if ( mnTreeRenderVertexBufferPos >= (SIZEOF_TREE_VERTEX_BUFFER-6) )
	{
		TreeRenderSpritesFlush( FALSE );
	}

}



void	TreesMapPacketHandler( int nPacketType, void* pxMapPacket )
{
#ifdef ENABLE_MAP_FORESTS		
MAP_PACKET_FOREST*		pForest = (MAP_PACKET_FOREST*)( pxMapPacket );
int		nLoop;
Tree*	pTree;
float	fMapScaleValue = LandscapeMapToWorldScale() / MapDataGetScale();
float	fRadius = pForest->fSpread * fMapScaleValue;
VECT	xBasePos;
LandscapeInitParams*		pxLandscapeInitParams = LandscapeGetInitParams();

	xBasePos = MapDataConvertCoordinate( pForest->fX, pForest->fY );

	for ( nLoop = 0; nLoop < pForest->wNumTrees; nLoop++ )
	{
		pTree = new Tree;

		pTree->mxPos = xBasePos;
		pTree->mxPos.x += FRand( -fRadius, fRadius );
		pTree->mxPos.y += FRand( -fRadius, fRadius );
		pTree->mxPos.z = 0.0f;
		pTree->mfRot = FRand( 0.0f, A360 );
		pTree->mfTreeSpriteSize = FRand( pxLandscapeInitParams->fTreeScaleMin, pxLandscapeInitParams->fTreeScaleMax );

		TreeListsAddTree( pTree );
}
#endif
}


void		TreesInit( void )
{
int	nLoop;
LandscapeInitParams*		pxInitParams = LandscapeGetInitParams();
float		fTreeTrunkScale = pxInitParams->fTreeScaleMin / 0.8f;

	mshTempTreeSprite = EngineLoadTexture( "Data\\Textures\\Trees\\TreeSprite2.png", 0, 0 );
	mshTempTreeTrunkModel = ModelLoad( "Data\\Models\\Trees\\TreeTrunk1.atm", 0, fTreeTrunkScale );
	mshTempTreeTrunkTexture = EngineLoadTexture( "Data\\Models\\Trees\\TreeTrunk1.jpg", 0, 0 );

	for ( nLoop = 0; nLoop < NUM_TREE_SPRITE_VERTEX_BUFFERS; nLoop++ )
	{
		mahTreeSpriteVertexBuffer[nLoop] = EngineCreateVertexBuffer( SIZEOF_TREE_VERTEX_BUFFER, 0 );
	}
	
	MapDataRegisterPacketHandler( MAP_FOREST, TreesMapPacketHandler );

	TreeListsInit();
}


void		TreesUpdate( void )
{
	TreeListsUpdate( 0.1f );

}


void		TreesFree( void )
{
int		nLoop;

	TreeListsFree();

	EngineReleaseTexture( &mshTempTreeSprite );

	for ( nLoop = 0; nLoop < NUM_TREE_SPRITE_VERTEX_BUFFERS; nLoop++ )
	{
		EngineVertexBufferFree( mahTreeSpriteVertexBuffer[nLoop] );
		mahTreeSpriteVertexBuffer[nLoop] = NOTFOUND;
	}

	ModelFree( mshTempTreeTrunkModel );
	mshTempTreeTrunkModel = NOTFOUND;
	EngineReleaseTexture( &mshTempTreeTrunkTexture );
}




void		TreesRender( void )
{

	EngineSetTexture( 0, mshTempTreeTrunkTexture );

	TreeListsRenderPass1();

	EngineCameraUpdate();

	mpulTreeRenderColStream = EngineVertexBufferLockColourStream( mahTreeSpriteVertexBuffer[ mnCurrentTreeSpriteVertexBufferIndex ], &mnTreeRenderColStride );
	mpxTreeRenderPosStream = EngineVertexBufferLockPositionStream( mahTreeSpriteVertexBuffer[ mnCurrentTreeSpriteVertexBufferIndex ], &mnTreeRenderPosStride );
	mpfTreeRenderUVStream = EngineVertexBufferLockUVStream( mahTreeSpriteVertexBuffer[ mnCurrentTreeSpriteVertexBufferIndex ], &mnTreeRenderUVStride );

	EngineDefaultState();
	EngineEnableLighting( FALSE );
	EngineEnableZTest( TRUE );
	EngineEnableBlend( TRUE );
	EngineSetBlendMode( BLEND_MODE_ALPHABLEND );
	EngineSetTexture( 0, mshTempTreeSprite );

	TreeListsRenderPass2();

	TreeRenderSpritesFlush( TRUE );
}

void		TreesSetView( int nGameMapMinX, int nGameMapMinY, int nGameMapMaxX, int nGameMapMaxY )
{
	TreeListsSetView( nGameMapMinX, nGameMapMinY, nGameMapMaxX, nGameMapMaxY );
}


