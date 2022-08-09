#include <stdio.h>
#include <math.h>
#include "StandardDef.h"
#include "Engine.h"
#include "Interface.h"

#include "../Physics/PhysicsObjects.h"

#include "MapData\MapData.h"
#include "Nature\Trees.h"
#include "Nature\Lakes.h"

#include "LandscapeHeightmap.h"
#include "LandscapeCoords.h"
#include "LandscapeRender.h"
#include "LandscapeTextures.h"
#include "Landscape.h"
#include "Sea.h"

extern "C"
{
extern void				MainGameCameraApplyToView( void );
}


//----------------------------------------------------

char	mszLandscapeLevelRootPath[128] = "";

LandscapeInitParams		mxLandscapeInitParams;
BOOL	mbLandscapeHasInitialised = FALSE;

//----------------------------------------------------------------------------------------------------------
// Landscape Internal

LandscapeInitParams*		LandscapeGetInitParams( void )
{
	return( &mxLandscapeInitParams );
}


const char*		LandscapeGetLevelRootPath( void )
{
	return( mszLandscapeLevelRootPath );
}

//----------------------------------------------------------------------------------------------------------

void			LandscapeInit( const char* szLevelRootPath, LandscapeInitParams* pInitParams )
{
int		hHeightmapTexture;
char	acString[256];
LandscapeInitParams		xDefaultInit;

	mbLandscapeHasInitialised = TRUE;
	if ( pInitParams == NULL )
	{
		pInitParams = &xDefaultInit;
	}

	mxLandscapeInitParams = *pInitParams;

	if ( szLevelRootPath )
	{
		strcpy( mszLandscapeLevelRootPath, szLevelRootPath );
	}
	else
	{
		mszLandscapeLevelRootPath[0] = 0;
	}

	LandscapeSetTileSize( mxLandscapeInitParams.fTileSize );
	LandscapeHeightmapSetHeightScale( pInitParams->fLandHeightScale );
	LandscapeHeightmapInit( pInitParams->nHeightmapWidth, pInitParams->nHeightmapHeight );

	if ( pInitParams->bUseHeightmap )
	{
		sprintf( acString, "%s\\heightmap.bmp", mszLandscapeLevelRootPath );

		hHeightmapTexture = InterfaceLoadTexture( acString, 2 );

		LandscapeHeightmapProcessHeightmapTexture( hHeightmapTexture, FALSE );

		InterfaceReleaseTexture( hHeightmapTexture );
	}

	if ( pInitParams->bAddToPhysicsWorld )
	{
		LandscapeHeightmapAddToPhysicsWorld();
	}
	LandscapeCoordsInit( LandscapeHeightmapGetSizeX(), LandscapeHeightmapGetSizeY() );

	LandscapeRenderInit( pInitParams->nLandscapeTextureSystem );

	if ( pInitParams->bUseSea )
	{
		SeaRenderInit();
	}

	MapDataInit();

	if ( pInitParams->bUseNature )
	{
		// **** All this before MapDataLoad to register map packet handlers
		TreesInit();
		LakesInit();
		// ***********
	}

}


void			LandscapeLoadMap( void )
{
	if ( mxLandscapeInitParams.bUseMapData )
	{
		MapDataLoad();
	}

	if ( mxLandscapeInitParams.bUseNature )
	{
		TreesGenerate();
	}
}

void			LandscapeUpdate( float fDelta )
{
	if ( mbLandscapeHasInitialised )
	{
		LandscapeRenderNewFrame();
		LandscapeTexturesNewFrame();

		if ( mxLandscapeInitParams.bUseNature )
		{
			TreesUpdate();
		}
	}
}

void			LandscapeRenderEnvironment( int nMode )
{
	if ( mxLandscapeInitParams.bUseNature )
	{
		TreesRender();
	}
}

void			LandscapeRender( void )
{
	if ( LandscapeRenderHasInitialised() )
	{
		// generate the tile render list
		LandscapeRenderTopdown( 0 );

		// probably want to separate this off later..
		LandscapeRenderFlushTiles();

		if ( mxLandscapeInitParams.bUseNature )
		{
			if ( EngineSceneShadowMapIsActive() == TRUE )
			{
				EngineSceneShadowsEndRender();
			}

			LakesRender();

			MainGameCameraApplyToView();
			TreesRender();

			if ( EngineSceneShadowMapIsActive() == TRUE )
			{
				EngineSceneShadowsStartRender( TRUE, FALSE, FALSE );
			}
		}
	}
}

void			LandscapePreGraphicsDeviceReset( void )
{
	SeaRenderFree();
	LandscapeRenderShutdown();
	TreesFree();
	LakesFree();
}

void			LandscapePostGraphicsDeviceReset( void )
{
	LandscapeRenderInit( mxLandscapeInitParams.nLandscapeTextureSystem );
}

void			LandscapeShutdown( void )
{
	SeaRenderFree();
	LandscapeHeightmapFree();
	LandscapeRenderShutdown();
	LandscapeCoordsShutdown();
	MapDataFree();
	TreesFree();
	LakesFree();
}

void			LandscapeModifyHeight( int nMapX, int nMapY, float fNewHeightInWorldSpace )
{
	LandscapeHeightmapSetHeight( nMapX, nMapY, fNewHeightInWorldSpace );

	LandscapeCoordsUpdateVertexData( nMapX, nMapY );

}


eLANDTYPE		LandscapeGetTileType( int nMapX, int nMapY )
{
float	fSeaHeight = SeaGetHeight();
float	fLandHeight;

	fLandHeight = LandscapeHeightmapGetHeight( nMapX, nMapY );

	if ( fLandHeight <= fSeaHeight )
	{
		return( LANDTYPE_SEA );
	}

	return( LANDTYPE_GRASS );

/*
	LANDTYPE_BEACH,
	LANDTYPE_ROCK,
	LANDTYPE_SNOW,
*/
}


MVECT		maxBoundingOffsets[4] =
{
	{  0.0f,  0.6f, 0.0f },		/** Left **/	
	{  0.0f, -0.6f, 0.0f },		/** Right **/	
	{  1.0f,  0.0f, 0.0f },		/** Front **/	
	{ -1.0f,  0.0f, 0.0f },		/** Back **/	
};



void			LandscapeGetYawPitchRoll( const VECT* pxPos, float fRotZ, float fScale, VECT* pxRotOut )
{
ENGINEMATRIX matLocalRot;
FLOAT	fHeight;
//int		nIndex;
int		nLoop;
VECT		xBasePos;
MVECT*	pxBoundingOffsets;
float	fRollAngle;
float	fRollSca;
float	fPitchAngle;
int		nStepAmount = 10;
//VECT	xVect;
//float	fAngle;
VECT	axBoundingPositions[4];

	// Generate local rotation matrix 
	EngineMatrixRotationZ( &matLocalRot, fRotZ ); 
	pxBoundingOffsets = maxBoundingOffsets;

	// Get height positions at front, back, left and right
	for ( nLoop = 0; nLoop < 4; nLoop++ )
	{
		VectScale( &xBasePos, pxBoundingOffsets, fScale );
	    VectTransform( &xBasePos, &xBasePos, &matLocalRot );
		VectAdd( &xBasePos, &xBasePos, pxPos );

		fHeight = LandscapeHeightmapGetHeightWorld( xBasePos.x, xBasePos.y );
		
		xBasePos.z = fHeight;
		axBoundingPositions[ nLoop ] = xBasePos;
		pxBoundingOffsets++;
	}

	fRollSca = ( (axBoundingPositions[0].z - axBoundingPositions[1].z) / (1.0f * fScale ) );
	fRollAngle = atanf( fRollSca );

	fPitchAngle = ( (axBoundingPositions[3].z - axBoundingPositions[2].z) / (1.0f * fScale) );
	fPitchAngle = atanf( fPitchAngle );
	
	pxRotOut->x = fRollAngle;
	pxRotOut->y = fPitchAngle;
	pxRotOut->z = fRotZ;

/*
	// Calculate and store the vehicles orientation vector
	ENGINEMATRIX	xOrientationMatrix;
	xVect = pxPlayerDetails->xVehicleRotations;
	xVect.x = 0.0f;
	EngineSetMatrixFromRotations( &xVect, &xOrientationMatrix );
//	EngineSetMatrixFromRotations( &pxPlayerDetails->xVehicleRotations, &xOrientationMatrix );
	pxPlayerDetails->xVehicleOrientationVector.x = 1.0f;
	pxPlayerDetails->xVehicleOrientationVector.y = 0.0f;
	pxPlayerDetails->xVehicleOrientationVector.z = 0.0f;
	VectTransform( &pxPlayerDetails->xVehicleOrientationVector, &pxPlayerDetails->xVehicleOrientationVector, &xOrientationMatrix );

	xVect = pxPlayerDetails->xVehicleRotations;
	EngineSetMatrixFromRotations( &xVect, &xOrientationMatrix );

	pxPlayerDetails->xVehicleUpVector.x = 0.0f;
	pxPlayerDetails->xVehicleUpVector.y = 0.0f;
	pxPlayerDetails->xVehicleUpVector.z = 1.0f;
	VectTransform( &pxPlayerDetails->xVehicleUpVector, &pxPlayerDetails->xVehicleUpVector, &xOrientationMatrix );
*/

} 

VECT			LandscapeGetAlignedForwardVector( const VECT* pxPos, float fRotZ, float fRadius, VECT* pxRotOut )
{
VECT	xRot;
VECT	xVect = { 1.0f, 0.0f, 0.0f };
ENGINEMATRIX	xOrientationMatrix;

	LandscapeGetYawPitchRoll( pxPos, fRotZ, fRadius, &xRot );

	if ( pxRotOut )
	{
		*pxRotOut = xRot;
	}
	// We don't need to worry about the roll
	xRot.x = 0.0f;
	EngineSetMatrixFromRotations( &xRot, &xOrientationMatrix );
	VectTransform( &xVect, &xVect, &xOrientationMatrix );
	return( xVect );
}
