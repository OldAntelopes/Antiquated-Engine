
#include <stdio.h>
#include "Stdwininclude.h"
#include "StandardDef.h"
#include "Interface.h"
#include "Engine.h"

#include "Landscape/LandscapeHeightmap.h"
#include "Landscape/SourceTextures.h"
#include "Landscape/LandscapeShader.h"
#include "Landscape/LandscapeMesh.h"
#include "Landscape/LandscapeCollision.h"
#include "Landscape/LandscapeCoords.h"
#include "Nature/Trees.h"
#include "Nature/TreeLists.h"

#include "MenuInterface.h"
#include "TileLandscapeGen.h"
#include "MapGen.h"

#include "Generator.h"

enum eGeneratorStage
{
	GENERATOR_INACTIVE,
	GENERATOR_INITIALISING,
	GENERATOR_LOADING,
	GENERATOR_CREATING_COLLISION_MESHES,
	GENERATOR_CREATING_MAPGEN,
	GENERATOR_GENERATING_TREES,
	GENERATOR_CREATING_LORES,
	GENERATOR_CREATING_HIRES,
	GENERATOR_CLEANING_UP,
};

eGeneratorStage		mGeneratorStage = GENERATOR_INACTIVE;

BOOL	mbGeneratorActive = FALSE;
int		mhGeneratorRenderTargetTexture = NOTFOUND;
int		mhShadowTargetTexture = NOTFOUND;
int		mnGeneratorJobNum = 0;

int		mnGeneratedTextureSize = 256;
int		mhGeneratorHeightmapTexture = NOTFOUND;
int		mhGeneratorHeightmapSizeX = 0;
int		mhGeneratorHeightmapSizeY = 0;

int		mnGeneratorLastProcessedX = NOTFOUND;
int		mnGeneratorLastProcessedY = NOTFOUND;


volatile BOOL	mboExitThreads = FALSE;

BOOL	mbExportJPGs = TRUE;
GENERATOR_CONFIG		msGeneratorConfig;

class GeneratorJobGroupDef
{
public:
	int			nGeneratorMapSizeX;
	int			nGeneratorMapSizeY;

	int			nGeneratorTileCurrentX;
	int			nGeneratorTileCurrentY;

	int			nGeneratorTilesPerTextureX;
	int			nGeneratorTilesPerTextureY;

	float		fSourceTextureScale;
	char		szOutPathExtension[32];

	GeneratorJobGroupDef*		mpNext;
};

GeneratorJobGroupDef*	mspGeneratorJobGroupDefs = NULL;



void	GeneratorDisplay( int X, int Y, int W, int H )
{
GeneratorJobGroupDef*	pCurrentTask = mspGeneratorJobGroupDefs;
char	acString[256];
int		nOverlay;
int		nNumJobs = 0;

	nOverlay = EngineTextureCreateInterfaceOverlay( 0, mhGeneratorRenderTargetTexture );

	if ( pCurrentTask )
	{
		nNumJobs = (pCurrentTask->nGeneratorMapSizeX/pCurrentTask->nGeneratorTilesPerTextureX) * (pCurrentTask->nGeneratorMapSizeY/pCurrentTask->nGeneratorTilesPerTextureY);
	}
	 
	switch( mGeneratorStage )
	{
	case GENERATOR_INACTIVE:
		sprintf( acString, "Generator: Inactive" );
		break;
	case GENERATOR_INITIALISING:
		sprintf( acString, "Generator: Initialising" );
		break;
	case GENERATOR_LOADING:
		sprintf( acString, "Generator: Loading heightmap" );
		break;
	case GENERATOR_CREATING_COLLISION_MESHES:
		sprintf( acString, "Generator: Creating collision data" );
		break;
	case GENERATOR_CREATING_MAPGEN:
		sprintf( acString, "Generator: Creating MapData" );
		break;
	case GENERATOR_GENERATING_TREES:
		sprintf( acString, "Generator: Creating Trees" );
		break;
	case GENERATOR_CREATING_LORES:
		sprintf( acString, "Generating lo-res tiles..." );
		break;
	case GENERATOR_CREATING_HIRES:
		sprintf( acString, "Generating hi-res tiles..." );
		break;
	case GENERATOR_CLEANING_UP:
		sprintf( acString, "Generator: Cleaning up.." );
		break;
	}

	InterfaceText( 0, X + 10, Y + 10, acString, 0xd0404040, 0 );

	switch( mGeneratorStage )
	{
	case GENERATOR_CREATING_LORES:
	case GENERATOR_CREATING_HIRES:
		if ( mnGeneratorLastProcessedX != NOTFOUND )
		{
		int	nGridX = pCurrentTask->nGeneratorMapSizeX/pCurrentTask->nGeneratorTilesPerTextureX;
		int	nGridY = pCurrentTask->nGeneratorMapSizeY/pCurrentTask->nGeneratorTilesPerTextureY;
		
			sprintf( acString, "X: %03d  Y:%03d", mnGeneratorLastProcessedX, mnGeneratorLastProcessedY );
			InterfaceText( 0, X + (W - 20), Y + 50, acString, 0xd0404040, 0 );

			sprintf( acString, "Job: %d/%d", mnGeneratorJobNum + 1, nNumJobs );
			InterfaceText( 0, X + (W - 20), Y + 75, acString, 0xd0404040, 0 );

			sprintf( acString, "Grid: %d x %d", nGridX, nGridY );
			InterfaceText( 0, X + (W - 20), Y + 105, acString, 0xd0505050, 0 );
		}
		break;
	}


	InterfaceTexturedRect( nOverlay, X + 10, Y + 40, W - 40, H - 40, 0xFFFFFFFF, 0.0f, 0.0f, 1.0f, 1.0f );

}

typedef struct
{
	int		nX1;
	int		nY1;
	int		nX2;
	int		nY2;

	GeneratorJobGroupDef		jobDef;

	byte*	pbTexture;
	int		nPitch;

} SHADOW_JOB;

int		msnThreadCompletedCount = 0;

void	LandscapeShadowGetTextureCoordForWorldCoord( SHADOW_JOB* pJob, float fWorldX, float fWorldY, int* pnTextureX, int* pnTextureY )
{
int		nTileX = pJob->jobDef.nGeneratorTileCurrentX;
int		nTileY = pJob->jobDef.nGeneratorTileCurrentY;
float	fWorldXPerPixel = (pJob->jobDef.nGeneratorTilesPerTextureX * LandscapeMapToWorldScale() ) / (float)(mnGeneratedTextureSize);
float	fWorldYPerPixel = (pJob->jobDef.nGeneratorTilesPerTextureY * LandscapeMapToWorldScale() ) / (float)(mnGeneratedTextureSize);
float	fPixelPerWorldX = 1.0f / fWorldXPerPixel;
float	fPixelPerWorldY = 1.0f / fWorldYPerPixel;
float	fTileBaseWorldX = (float)nTileX * LandscapeMapToWorldScale();
float	fTileBaseWorldY = (float)nTileY * LandscapeMapToWorldScale();
float	fTempX;
float	fTempY;
	
	fTempX = (fWorldX - fTileBaseWorldX) * fPixelPerWorldX;
	fTempY = (fWorldY - fTileBaseWorldY) * fPixelPerWorldY;

	*pnTextureX = (int)( fTempX );
	*pnTextureY = (int)( fTempY );
}

void	LandscapeShadowSetShadow( BYTE* pbTexture, int nPitch, int nPixelX, int nPixelY, ulong ulCol )
{
	if ( ( nPixelX > 0 ) &&
		 ( nPixelY > 0 ) &&
		 ( nPixelX < mnGeneratedTextureSize ) &&
		 ( nPixelY < mnGeneratedTextureSize ) )
	{
	ulong*	pulTexture;
	ulong	B;
	ulong	sourceB;
	ulong	ulSource;

		pbTexture += (nPitch * nPixelY);
		pulTexture = (ulong*)pbTexture;
		pulTexture += nPixelX;

		B = ulCol & 0xFF;

		ulSource = pulTexture[0];

		sourceB = ulSource & 0xFF;
		if ( sourceB > B )
		{
			B = sourceB;
		}

		pulTexture[0] = 0xFF000000 | (B << 16) | (B << 8 ) | B;
	}
}

#define		SHADOW_SPRITE_SIZE		16

ulong		ulTreeShadowMap[] = 
{
	0xFFB0B0B0, 0xff505050,	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x00000000,
	0xff505050, 0xFFB0B0B0, 0xff505050,	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x00000000,
	0x00000000, 0xff505050, 0xFFA0A0A0, 0xff303030,	0x00000000,	0x00000000,	0xFF606060,	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x00000000,
	0x00000000, 0x00000000, 0xff303030, 0xFF909090, 0xff202020,	0xFF606060,	0x00000000,	0xFF707070,	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0xFF505050,	0x00000000,	0x00000000,	0x00000000,

	0x00000000,	0x00000000, 0x00000000, 0xFF606060, 0xFF707070, 0xFF606060,	0xFF606060,	0xFF707070,	0x00000000,	0x00000000,	0xFF686868,	0xFF686868,	0xFF686868,	0x00000000,	0x00000000,	0x00000000,
	0x00000000,	0x00000000,	0x00000000, 0x00000000, 0xFF606060, 0xFF6b6b6b, 0xFF707070,	0xFF606060,	0x00000000,	0xFF707070,	0x00000000,	0xFF686868,	0xFF686868,	0xFF686868,	0xFF686868,	0x00000000,
	0x00000000,	0x00000000,	0x00000000, 0xFF606060, 0xFF606060, 0xFF707070, 0xFF707070, 0x00000000,	0xFF707070,	0xFF686868,	0x00000000,	0xFF686868,	0x00000000,	0x00000000,	0x00000000,	0x00000000,
	0x00000000,	0x00000000,	0x00000000, 0x00000000, 0xFF606060, 0xFF707070, 0x00000000, 0xFF606060,	0xFF686868,	0x00000000,	0xFF686868,	0x00000000,	0xFF505050,	0x00000000,	0x00000000,	0x00000000,

	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0xFF606060,	0xFF606060,	0xFF606060,	0xFF686868, 0xFF606060,	0x00000000,	0xFF686868,	0xFF606060,	0xFF606060,	0x00000000,	0x00000000,	0x00000000,
	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0xFF686868,	0xFF686868,	0xFF606060, 0x00000000,	0xFF606060,	0xFF686868,	0x00000000,	0xFF686868,	0xFF606060,	0x00000000,	0x00000000,
	0x00000000,	0x00000000,	0xFF686868,	0x00000000,	0xFF686868,	0xFF686868,	0x00000000,	0x00000000, 0xFF686868,	0x00000000,	0xFF606060,	0xFF686868,	0xFF686868,	0xFF686868,	0xFF686868,	0x00000000,
	0x00000000,	0x00000000,	0x00000000,	0xFF686868,	0xFF686868,	0x00000000,	0x00000000,	0x00000000, 0xFF686868,	0xFF686868,	0x00000000,	0xFF606060,	0xFF686868, 0xFF505050,	0x00000000,	0x00000000,

	0x00000000,	0x00000000,	0x00000000,	0xFF686868,	0xFF686868,	0x00000000,	0xFF686868,	0x00000000, 0xFF686868,	0x00000000,	0xFF686868,	0xFF686868,	0xFF606060,	0x00000000,	0xFF505050,	0x00000000,
	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0xFF686868, 0x00000000,	0x00000000,	0x00000000,	0xFF505050,	0xFF505050,	0xFF606060,	0xFF505050,	0x00000000,
	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x00000000, 0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0xFF505050,	0xFF606060,	0x00000000,
	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x00000000, 0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0x00000000,	0xFF505050,
};

void	LandscapeShadowTreeProcess( Tree* pTree, ulong ulJob )
{
SHADOW_JOB* pJob = (SHADOW_JOB*)( ulJob );
float	fWorldXPerPixel = (pJob->jobDef.nGeneratorTilesPerTextureX * LandscapeMapToWorldScale() ) / (float)(mnGeneratedTextureSize);
float	fWorldYPerPixel = (pJob->jobDef.nGeneratorTilesPerTextureY * LandscapeMapToWorldScale() ) / (float)(mnGeneratedTextureSize);
int		nTileX = pJob->jobDef.nGeneratorTileCurrentX + (int)( ( pJob->nX1 * fWorldXPerPixel ) * LandscapeWorldToMapScale() );
int		nTileY = pJob->jobDef.nGeneratorTileCurrentY + (int)( ( pJob->nY1 * fWorldYPerPixel ) * LandscapeWorldToMapScale() );
float	fPixelPerWorldX = 1.0f / fWorldXPerPixel;
float	fPixelPerWorldY = 1.0f / fWorldYPerPixel;
float	fWorldBaseX = nTileX * LandscapeMapToWorldScale();
float	fWorldBaseY = nTileY * LandscapeMapToWorldScale();
float	fTexWorldX, fTexWorldY;
int		nPixelX, nPixelY;
float	fTreeShadowSpriteScale = fPixelPerWorldX * 0.7f;
float	fSpriteLoopX;
float	fSpriteLoopY;
float	fSpriteScale;
int		nPixelXBase;
int		nSamplePos;

	fTexWorldX = pTree->mxPos.x - fWorldBaseX;
	fTexWorldY = pTree->mxPos.y - fWorldBaseY;

	nPixelX = pJob->nX1 + (int)( fTexWorldX * fPixelPerWorldX );
	nPixelY = pJob->nY1 + (int)( fTexWorldY * fPixelPerWorldY );

	// add a shadow for each...
	if ( fTreeShadowSpriteScale > 1.0f ) fTreeShadowSpriteScale = 1.0f;
	fSpriteScale = SHADOW_SPRITE_SIZE / (SHADOW_SPRITE_SIZE * fTreeShadowSpriteScale);
	nPixelXBase = nPixelX;
	for ( fSpriteLoopY = 0; fSpriteLoopY < SHADOW_SPRITE_SIZE; fSpriteLoopY += fSpriteScale )
	{
		for ( fSpriteLoopX = 0; fSpriteLoopX < SHADOW_SPRITE_SIZE; fSpriteLoopX += fSpriteScale )
		{
			nSamplePos = (int)(fSpriteLoopY*SHADOW_SPRITE_SIZE) + (int)fSpriteLoopX;
			if ( ulTreeShadowMap[nSamplePos] != 0 )
			{
				LandscapeShadowSetShadow( pJob->pbTexture, pJob->nPitch, nPixelX, nPixelY, ulTreeShadowMap[nSamplePos] );
			}
			nPixelX ++;
		}
		nPixelY++;
		nPixelX = nPixelXBase;
	}

}


void	LandscapeShadowJobAddTreeShadows( SHADOW_JOB* pJob )
{
float	fWorldXPerPixel = (pJob->jobDef.nGeneratorTilesPerTextureX * LandscapeMapToWorldScale() ) / (float)(mnGeneratedTextureSize);
float	fWorldYPerPixel = (pJob->jobDef.nGeneratorTilesPerTextureY * LandscapeMapToWorldScale() ) / (float)(mnGeneratedTextureSize);
int		nTileX = pJob->jobDef.nGeneratorTileCurrentX + (int)( ( pJob->nX1 * fWorldXPerPixel ) * LandscapeWorldToMapScale() );
int		nTileY = pJob->jobDef.nGeneratorTileCurrentY + (int)( ( pJob->nY1 * fWorldYPerPixel ) * LandscapeWorldToMapScale() );
int		nTileMaxX = nTileX + (int)( ( ( pJob->nX2 - pJob->nX1 ) * fWorldXPerPixel ) * LandscapeWorldToMapScale() );
int		nTileMaxY = nTileY + (int)( ( ( pJob->nY2 - pJob->nY1 ) * fWorldYPerPixel ) * LandscapeWorldToMapScale() );

	// TODO - Get game map range containing trees that might cast on to this section
	TreeListsSetView( nTileX - 1, nTileY - 1, nTileMaxX + 1, nTileMaxY + 1 );

	// Go through list of trees within that map range
	TreeListsProcessView( LandscapeShadowTreeProcess, (ulong)( pJob ) );


	

}


long WINAPI LandscapeShadowJob(long lParam)
{ 
SHADOW_JOB*		pJob = (SHADOW_JOB*)( lParam );
int		nTileX = pJob->jobDef.nGeneratorTileCurrentX;
int		nTileY = pJob->jobDef.nGeneratorTileCurrentY;
float	fWorldXPerPixel = (pJob->jobDef.nGeneratorTilesPerTextureX * LandscapeMapToWorldScale() ) / (float)(mnGeneratedTextureSize);
float	fWorldYPerPixel = (pJob->jobDef.nGeneratorTilesPerTextureY * LandscapeMapToWorldScale() ) / (float)(mnGeneratedTextureSize);
BYTE*	pbTextureBase;
BYTE*	pbTexture = pJob->pbTexture;
int		nLoopX;
int		nLoopY;
ulong*	pulTexture;
ulong	ulCol;
ulong	ulR;
ulong	ulG;
ulong	ulB;
VECT	xWorldCoord;
VECT	xWorldCoordLineBase;
VECT	xRayStart;
VECT	xRayDir = { -1.0f, -1.0f, 0.5f };
VECT	xRayDir1 = { -1.0f, -1.0f, 0.5f };
VECT	xRayDir2 = { -0.95f, -1.0f, 0.54f };
VECT	xRayDir3 = { -1.0f, -0.95f, 0.54f };
VECT	xRayDir4 = { -1.0f, -1.0f, 0.6f };
VECT	xRayDir5 = { -1.0f, -1.0f, 0.7f };
VECT	xIntersect;
float	fHeightAtCoord;
int		nNumRaysHit = 0;
int		nLoopRays;

	pbTextureBase = pbTexture;
	VectNormalize( &xRayDir1 );
	VectNormalize( &xRayDir2 );
	VectNormalize( &xRayDir3 );
	VectNormalize( &xRayDir4 );
	VectNormalize( &xRayDir5 );

	xWorldCoordLineBase	= LandscapeGetWorldPos( nTileX, nTileY );
	xWorldCoordLineBase.x += pJob->nX1 * fWorldXPerPixel;
	xWorldCoordLineBase.y += pJob->nY1 * fWorldYPerPixel;
	pbTextureBase += (pJob->nPitch * pJob->nY1);
	for ( nLoopY = pJob->nY1; nLoopY < pJob->nY2; nLoopY++ )
	{
		xWorldCoord = xWorldCoordLineBase;
		pulTexture = (ulong*)pbTextureBase;
		pulTexture += pJob->nX1;
		for ( nLoopX = pJob->nX1; nLoopX < pJob->nX2; nLoopX++ )
		{
			if ( mboExitThreads )
			{
				msnThreadCompletedCount++;
				return( 0 );
			}

			ulCol = *pulTexture;
			ulR = (ulCol >> 16) & 0xFF;
			ulG = (ulCol >> 8) & 0xFF;
			ulB = (ulCol & 0xFF);
		
			fHeightAtCoord = LandscapeHeightmapGetHeightWorld( xWorldCoord.x, xWorldCoord.y );
			nNumRaysHit = 0;

			for ( nLoopRays = 0; nLoopRays < 6; nLoopRays++ )
			{
				xRayStart = xWorldCoord;
				switch( nLoopRays )
				{
				case 0:
					xRayStart.z = fHeightAtCoord + 1.0f;
					xRayDir = xRayDir1;
					break;
				case 1:
					xRayStart.z = fHeightAtCoord + 1.6f;
					xRayDir = xRayDir2;
					break;
				case 2:
					xRayStart.z = fHeightAtCoord + 1.6f;
					xRayDir = xRayDir3;
					break;
				case 3:
					xRayStart.z = fHeightAtCoord + 2.0f;
					xRayDir = xRayDir1;
					break;
				case 4:
					xRayStart.z = fHeightAtCoord + 1.1f;
					xRayDir = xRayDir4;
					break;
				case 5:
					xRayStart.z = fHeightAtCoord + 1.2f;
					xRayDir = xRayDir5;
					break;
				}
			
				VectScale( &xRayDir, &xRayDir, 200.0f );
//				VectAdd( &xRayStart, &xRayStart, &xRayDir );
//				VectScale( &xRayDir, &xRayDir, -1.0f );

				if ( LandscapeMeshRayTest( &xRayStart, &xRayDir, &xIntersect ) == TRUE )
//				if ( LandscapeRayTest( &xRayStart, &xRayDir, &xIntersect, FALSE ) == TRUE )
				{
					nNumRaysHit++;
				}
			}

			if ( nNumRaysHit > 0 )
			{
			float	fShadowIntensity = 0.3f;
			float	fRayMod = fShadowIntensity / 6.0f;
			float	fMod = ( nNumRaysHit * fRayMod );

//				ulR = ;
//				ulG = 0xFF;
//				ulB = 0;
				ulR = (ulong)( 255.0f * fMod );
				ulG = (ulong)( 255.0f * fMod );
				ulB = (ulong)( 255.0f * fMod );

				*pulTexture = 0xFF000000 | (ulR << 16) | (ulG << 8) | (ulB);
			}
			else
			{
				*pulTexture = 0xFF000000;
			}
			pulTexture++;
			xWorldCoord.x += fWorldXPerPixel;
		}
		pbTextureBase = pbTextureBase + pJob->nPitch;
		xWorldCoordLineBase.y += fWorldYPerPixel;
	}

	if ( msGeneratorConfig.bGenTreeShadows )
	{
		LandscapeShadowJobAddTreeShadows( pJob );
	}

	msnThreadCompletedCount++;
	return( 0 );
}

void	LandscapeCopyShadowOnToTile( void )
{
int		hVertexBuffer;
ENGINEBUFFERVERTEX		xVertex;
float	fPosW = 512.0f;
float	fPosH = 512.0f;

	EngineSetRenderTargetTexture( mhGeneratorRenderTargetTexture, 0, FALSE );

	EngineCameraSetOthorgonalView( 512, 512 );

	hVertexBuffer = EngineCreateVertexBuffer( 6, 0 );
	EngineVertexBufferLock( hVertexBuffer, TRUE );

	xVertex.color = 0xFFFFFFFF;
	xVertex.position.x = 0.0f;
	xVertex.position.y = 0.0f;
	xVertex.position.z = 0.0f;
	xVertex.tu = 0.0f;
	xVertex.tv = 0.0f;
	EngineVertexBufferAdd( hVertexBuffer, &xVertex );

	xVertex.position.x = fPosW;
	xVertex.position.y = 0.0f;
	xVertex.tu = 1.0f;
	xVertex.tv = 0.0f;
	EngineVertexBufferAdd( hVertexBuffer, &xVertex );

	xVertex.position.x = 0.0f;
	xVertex.position.y = fPosH;
	xVertex.tu = 0.0f;
	xVertex.tv = 1.0f;
	EngineVertexBufferAdd( hVertexBuffer, &xVertex );

	// tri 2
	xVertex.position.x = fPosW;
	xVertex.position.y = 0.0f;
	xVertex.tu = 1.0f;
	xVertex.tv = 0.0f;
	EngineVertexBufferAdd( hVertexBuffer, &xVertex );

	xVertex.position.x = fPosW;
	xVertex.position.y = fPosH;
	xVertex.tu = 1.0f;
	xVertex.tv = 1.0f;
	EngineVertexBufferAdd( hVertexBuffer, &xVertex );

	xVertex.position.x = 0.0f;
	xVertex.position.y = fPosH;
	xVertex.tu = 0.0f;
	xVertex.tv = 1.0f;
	EngineVertexBufferAdd( hVertexBuffer, &xVertex );

	EngineVertexBufferUnlock( hVertexBuffer );

	EngineDefaultState();
	EngineSetStandardMaterial();
	EngineSetBlendMode( BLEND_MODE_COLOUR_SUBTRACTIVE );
	EngineSetColourMode( 0, COLOUR_MODE_TEXTURE_MODULATE );
	EngineEnableBlend( TRUE );
	EngineEnableLighting( FALSE );
	EngineEnableFog( FALSE );
	EngineSetTexture( 0, mhShadowTargetTexture );
	EngineVertexBufferRender( hVertexBuffer, TRIANGLE_LIST );

	EngineVertexBufferFree( hVertexBuffer );
	EngineRestoreRenderTarget();
	EngineDefaultState();
}


void	LandscapeApplyShadowsThreaded( GeneratorJobGroupDef* pJobDef )
{
ulong iID;
HANDLE hThread;
SHADOW_JOB			xJob1;
SHADOW_JOB			xJob2;
SHADOW_JOB			xJob3;
SHADOW_JOB			xJob4;
int				nPitch;
BYTE*	pbTexture;

	pbTexture = EngineLockTexture( mhShadowTargetTexture, &nPitch, TRUE );

	xJob1.jobDef = *pJobDef;
	xJob1.nX1 = 0;
	xJob1.nY1 = 0;
	xJob1.nX2 = 128;
	xJob1.nY2 = 128;
	xJob1.pbTexture = pbTexture;
	xJob1.nPitch = nPitch;
	xJob2.jobDef = *pJobDef;
	xJob2.nX1 = 128;
	xJob2.nY1 = 0;
	xJob2.nX2 = 256;
	xJob2.nY2 = 128;
	xJob2.pbTexture = pbTexture;
	xJob2.nPitch = nPitch;
	xJob3.jobDef = *pJobDef;
	xJob3.nX1 = 0;
	xJob3.nY1 = 128;
	xJob3.nX2 = 128;
	xJob3.nY2 = 256;
	xJob3.pbTexture = pbTexture;
	xJob3.nPitch = nPitch;
	xJob4.jobDef = *pJobDef;
	xJob4.nX1 = 128;
	xJob4.nY1 = 128;
	xJob4.nX2 = 256;
	xJob4.nY2 = 256;
	xJob4.pbTexture = pbTexture;
	xJob4.nPitch = nPitch;

	msnThreadCompletedCount = 0;

	hThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)LandscapeShadowJob,(LPVOID)(&xJob1),0,&iID);
	hThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)LandscapeShadowJob,(LPVOID)(&xJob2),0,&iID);
	hThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)LandscapeShadowJob,(LPVOID)(&xJob3),0,&iID);
	hThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)LandscapeShadowJob,(LPVOID)(&xJob4),0,&iID);

	// todo - rather than just looping here we should drop out and do regular windows updates instead
	// until the threads have completed..
	while( msnThreadCompletedCount < 4 )
	{
		Sleep(10);
		if ( MainProcessUpdate() == FALSE )
		{
			mboExitThreads = TRUE;
			Sleep(10);
			msnThreadCompletedCount = 4;
		}
//		EngineUpdate( FALSE );
//		GeneratorDisplay();

		// todo - acknowledge quit in here...!
	}

	EngineUnlockTexture( mhShadowTargetTexture );

	// Copy shadow texture on to rendertarget texture
	LandscapeCopyShadowOnToTile();

}

void		GeneratorShutdown( void )
{

}


void	GeneratorJob( GeneratorJobGroupDef* pJobDef )
{
char	acString[256];
RENDER_TILE_ITEM		xRenderTile;
int		nDrawX = 0;
int		nDrawY = 0;
int		nDrawW = mnGeneratedTextureSize / pJobDef->nGeneratorTilesPerTextureX;
int		nDrawH = mnGeneratedTextureSize / pJobDef->nGeneratorTilesPerTextureY;
int		nLoopX;
int		nLoopY;
int		nTileX = pJobDef->nGeneratorTileCurrentX;
int		nTileY = pJobDef->nGeneratorTileCurrentY;
int		hExportTexture;

	EngineDefaultState();

	EngineSetRenderTargetTexture( mhGeneratorRenderTargetTexture, 0, TRUE );

	LandscapeShaderSetOthorgonalView( mnGeneratedTextureSize, mnGeneratedTextureSize );
   	EngineEnableLighting( FALSE );
	EngineEnableZTest( FALSE );
	EngineEnableZWrite( FALSE );
	EngineEnableCulling( 0 );
	LandscapeShaderBeginRender();

	for ( nLoopY = 0; nLoopY < pJobDef->nGeneratorTilesPerTextureY; nLoopY++ )
	{
		nDrawX = 0;
		for ( nLoopX = 0; nLoopX < pJobDef->nGeneratorTilesPerTextureX; nLoopX++ )
		{
			xRenderTile.nMapX = nTileX + nLoopX;
			xRenderTile.nMapY = nTileY + nLoopY;
			xRenderTile.nResolution = 1;
			LandscapeShaderDrawSingleTile( &xRenderTile, nDrawX, nDrawY, nDrawX + nDrawW, nDrawY + nDrawH, pJobDef->fSourceTextureScale );
			nDrawX += nDrawW;
		}
		nDrawY += nDrawH;
	}

	EngineRestoreRenderTarget();
	LandscapeShaderCleanup();

	if ( msGeneratorConfig.bGenShadows == TRUE )
	{
//		LandscapeApplyShadows(pJobDef);
		LandscapeApplyShadowsThreaded( pJobDef );
	}

	hExportTexture = mhGeneratorRenderTargetTexture;

	mnGeneratorLastProcessedX = nTileX/pJobDef->nGeneratorTilesPerTextureX;
	mnGeneratorLastProcessedY = nTileY/pJobDef->nGeneratorTilesPerTextureY;
	if ( mbExportJPGs )
	{
		sprintf( acString, "%s\\%s\\TileY%03dX%03d.jpg", MenuInterfaceGetLevelFolderPath(), pJobDef->szOutPathExtension, mnGeneratorLastProcessedY, mnGeneratorLastProcessedX );
		EngineExportTexture( hExportTexture, acString, 2 );
	}
	else
	{
		sprintf( acString, "%s\\%s\\TileY%03dX%03d.bmp", MenuInterfaceGetLevelFolderPath(), pJobDef->szOutPathExtension, mnGeneratorLastProcessedY, mnGeneratorLastProcessedX );
		EngineExportTexture( hExportTexture, acString, 0 );
	}

}


BOOL		GeneratorUpdateJobs( void )
{
GeneratorJobGroupDef*	pJobGroup = mspGeneratorJobGroupDefs;

	if ( pJobGroup )
	{
		if ( stricmp ( pJobGroup->szOutPathExtension, "x4" ) == 0 )
		{
			mGeneratorStage = GENERATOR_CREATING_LORES;
		}
		else
		{
			mGeneratorStage = GENERATOR_CREATING_HIRES;
		}

		GeneratorJob( pJobGroup );
		mnGeneratorJobNum++;

		pJobGroup->nGeneratorTileCurrentX += pJobGroup->nGeneratorTilesPerTextureX;
		if ( pJobGroup->nGeneratorTileCurrentX >= pJobGroup->nGeneratorMapSizeX )
		{
			pJobGroup->nGeneratorTileCurrentX = 0;
			pJobGroup->nGeneratorTileCurrentY += pJobGroup->nGeneratorTilesPerTextureY;
			if ( pJobGroup->nGeneratorTileCurrentY >= pJobGroup->nGeneratorMapSizeY )
			{
			GeneratorJobGroupDef*	pNext = pJobGroup->mpNext;
			
				delete pJobGroup;
				mnGeneratorJobNum = 0;
				mspGeneratorJobGroupDefs = pNext;
				if ( pNext == NULL )
				{
					return( FALSE );
				}
			}
		}
		return( TRUE );
	}
	return( FALSE );
}

BOOL		GeneratorIsActive( void )
{
	return( mbGeneratorActive );
}




void		GeneratorAddJobGroup( int nMapSizeX, int nMapSizeY, int nTilesPerTextureX, int nTilesPerTextureY, float fSourceTextureScale, const char* szOutPathExtension )
{
GeneratorJobGroupDef*	pJobGroup = new GeneratorJobGroupDef;

	pJobGroup->mpNext = mspGeneratorJobGroupDefs;
	mspGeneratorJobGroupDefs = pJobGroup;

	pJobGroup->nGeneratorMapSizeX = nMapSizeX;
	pJobGroup->nGeneratorMapSizeY = nMapSizeY;

	pJobGroup->nGeneratorTilesPerTextureX = nTilesPerTextureX;
	pJobGroup->nGeneratorTilesPerTextureY = nTilesPerTextureY;
	
	pJobGroup->fSourceTextureScale = fSourceTextureScale;

//	pJobGroup->nGeneratorTileCurrentX = 2 * nTilesPerTextureX;
//	pJobGroup->nGeneratorTileCurrentY = 4 * nTilesPerTextureY;

	pJobGroup->nGeneratorTileCurrentX = 0;
	pJobGroup->nGeneratorTileCurrentY = 0;

	strcpy( pJobGroup->szOutPathExtension, szOutPathExtension );
}


void		GeneratorInitialLoad( void )
{
char	acHeightmapFilename[256];
FILE*	pFile;

	mhGeneratorHeightmapSizeX = 512;
	mhGeneratorHeightmapSizeY = 512;
	LandscapeHeightmapInit( mhGeneratorHeightmapSizeX, mhGeneratorHeightmapSizeY );
		
	sprintf( acHeightmapFilename, "%s\\heightmap.bmp", MenuInterfaceGetLevelFolderPath() );

	pFile = fopen( acHeightmapFilename, "rb" );

	if ( pFile )
	{
		fclose( pFile );

		mhGeneratorHeightmapTexture = InterfaceLoadTexture( acHeightmapFilename, 0 );

		if ( mhGeneratorHeightmapTexture != NOTFOUND )
		{
			MenuInterfacePrint( "Heightmap loaded: %s", acHeightmapFilename );
		
			LandscapeHeightmapProcessHeightmapTexture( mhGeneratorHeightmapTexture, TRUE );
	
			mhGeneratorHeightmapSizeX = LandscapeHeightmapGetSizeX();
			mhGeneratorHeightmapSizeY = LandscapeHeightmapGetSizeY();
		}
	}
	else
	{
		MenuInterfacePrint( "WARNING: No Heightmap file: %s", acHeightmapFilename );
	}

}

void		GeneratorCreateCollision( void )
{
	LandscapeRenderGenerateBlocks();
//	LandscapeRenderGenerateFullLandscapeMeshDX();
}

void		GeneratorCreateMapGen( void )
{
	if ( msGeneratorConfig.bGenPathfinding )
	{
		MapGenInit();
	}
}




void	GeneratorCreateTexGenJobs( void )
{
	LandscapeShaderInitShaders();
	
	mhGeneratorRenderTargetTexture = EngineCreateRenderTargetTexture( mnGeneratedTextureSize, mnGeneratedTextureSize, 2 );
	mhShadowTargetTexture = EngineCreateTexture( mnGeneratedTextureSize, mnGeneratedTextureSize, 1 );


	if ( msGeneratorConfig.bGenHiRes == TRUE )
	{
		GeneratorAddJobGroup( mhGeneratorHeightmapSizeX, mhGeneratorHeightmapSizeY, mhGeneratorHeightmapSizeX/32, mhGeneratorHeightmapSizeX/32, 0.0625f, "x32" );
	}
	
//	GeneratorAddJobGroup( nHeightmapSize, nHeightmapSize, nHeightmapSize/8, nHeightmapSize/8, 0.125f, "x8" );
	if ( msGeneratorConfig.bGenLoRes == TRUE )
	{
		GeneratorAddJobGroup( mhGeneratorHeightmapSizeX, mhGeneratorHeightmapSizeY, mhGeneratorHeightmapSizeX/4, mhGeneratorHeightmapSizeX/4, 0.25f, "x4" );
	}

}

void		GeneratorInitPreset( GENERATOR_CONFIG* pConfig )
{
	mnGeneratorLastProcessedX = NOTFOUND;
	mnGeneratorLastProcessedY = NOTFOUND;

	srand( SysGetTick() );
	msGeneratorConfig = *pConfig;

	mGeneratorStage = GENERATOR_INITIALISING;
	mbGeneratorActive = TRUE;

	MenuInterfacePrint( " ------- Generator Starting -------" );

}

void		GeneratorCleanup( void )
{
	InterfaceReleaseTexture( mhGeneratorHeightmapTexture );
}

BOOL		GeneratorUpdate( void )
{
	if ( mbGeneratorActive )
	{
	BOOL	bStuffToDo = TRUE;

		switch( mGeneratorStage )
		{
		case GENERATOR_INACTIVE:
			break;
		case GENERATOR_INITIALISING:
			mGeneratorStage = GENERATOR_LOADING;
			break;
		case GENERATOR_LOADING:
			GeneratorInitialLoad();
			mGeneratorStage = GENERATOR_CREATING_COLLISION_MESHES;
			break;
		case GENERATOR_CREATING_COLLISION_MESHES:
			GeneratorCreateCollision();
			mGeneratorStage = GENERATOR_CREATING_MAPGEN;
			break;
		case GENERATOR_CREATING_MAPGEN:
			GeneratorCreateMapGen();
			GeneratorCreateTexGenJobs();
			mGeneratorStage = GENERATOR_GENERATING_TREES;
			break;
		case GENERATOR_GENERATING_TREES:
			TreesGenerate();
			mGeneratorStage = GENERATOR_CREATING_LORES;
			break;
		case GENERATOR_CREATING_LORES:
		case GENERATOR_CREATING_HIRES:
			bStuffToDo = GeneratorUpdateJobs();
			if ( !bStuffToDo )
			{
				mGeneratorStage = GENERATOR_CLEANING_UP;
			}
			break;
		case GENERATOR_CLEANING_UP:
			GeneratorCleanup();
			mbGeneratorActive = FALSE;
			mGeneratorStage = GENERATOR_INACTIVE;
			MenuInterfacePrint( " ** Generator complete" );
			break;
		}

		if ( mbGeneratorActive == FALSE )
		{
			LandscapeShaderFreeShaders();
		}
	}
	return( mbGeneratorActive );
}
