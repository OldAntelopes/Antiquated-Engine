
#include <StandardDef.h>
#include <Interface.h>
#include <stdio.h>
#include <System.h>
#include <Rendering.h>
#include <Engine.h>

#include "Framework/InteriorMapEditor.h"
#include "Util/HttpDownload.h"
#include "Util/Line Renderer.h"
#include "Util/MouseCam.h"
#include "Main.h"
#include "../LibCode/Engine/Scene/SceneMap.h"		// cheeky


int		mnDemoModelHandle = NOTFOUND;
int		mnDemoTextureHandle = NOTFOUND;
VECT	mxModelPosition;
VECT	mxModelRotation;

char*	mszModelFileName = "TestRoomPart1.atm";
char*	mszTextureFileName = "TestRoomPart1.bmp";

char	mszExeDirectory[256] = "";

const char*		InteriorMapEditorGetExeDirectory( void )
{
	return( mszExeDirectory );
}
 
//--------------------------------------
// InteriorMapEditorLoadAssets
//	Load the model we're gonna see
//--------------------------------------
void	InteriorMapEditorLoadAssets( void )
{
	mnDemoModelHandle = ModelLoad(mszModelFileName, 0, 1.0f );
	mnDemoTextureHandle = EngineLoadTexture(mszTextureFileName, 0 );
	mxModelPosition.x = mxModelPosition.y = mxModelPosition.z = 0.0f;
	mxModelRotation.x = mxModelRotation.y = mxModelRotation.z = 0.0f;
}

//--------------------------------------
// InteriorMapEditorFreeAssets
//	 Tidyup and release memory used before exiting
//--------------------------------------
void	InteriorMapEditorFreeAssets( void )
{
	ModelFree( mnDemoModelHandle );
}

int		mnLightingMode = 1;

void	InteriorMapEditorUpdateLightingMode( int nMode )
{
ENGINE_LIGHT	xLight;
	mnLightingMode = nMode;
	// Add a light to the scene
	memset( &xLight, 0, sizeof( xLight ) );
	xLight.Type = DIRECTIONAL_LIGHT;
	switch( mnLightingMode )
	{
	case 0:
	default:
		xLight.Ambient.r = xLight.Ambient.g = xLight.Ambient.b = 0.5f;
		xLight.Diffuse.r = xLight.Diffuse.g = xLight.Diffuse.b = 0.5f;
		break;
	case 1:
		xLight.Ambient.r = xLight.Ambient.g = xLight.Ambient.b = 1.0f;
		xLight.Diffuse.r = xLight.Diffuse.g = xLight.Diffuse.b = 0.0f;
		break;
	case 2:
		xLight.Ambient.r = xLight.Ambient.g = xLight.Ambient.b = 0.0f;
		xLight.Diffuse.r = xLight.Diffuse.g = xLight.Diffuse.b = 1.0f;
		break;
	}
	xLight.Direction.x = 0.5f;
	xLight.Direction.y = -0.5f;
	xLight.Direction.z = -0.5f;
	VectNormalize( &xLight.Direction );
	EngineActivateLight( 0, &xLight );

}


//--------------------------------------
// InteriorMapEditorResetCamera
//	 
//--------------------------------------
void	InteriorMapEditorResetCamera( void )
{
VECT	xCamPos = { 1.0f, 1.0f, 1.0f };
VECT	xFocus = { 0.0f, 0.0f, 0.0f };

	MouseCamSet( &xCamPos, &xFocus );
	MouseCamUpdateCameraMatrices();

	// Initialise the camera
	EngineCameraSetPos(1.0f,1.0f,1.0f);
	EngineCameraSetDirection(-0.5f,-0.5f,-0.5f);
	EngineCameraUpdate();
	EngineCameraSetProjection( PI/4,0.002f,200.0f );
	
}

//--------------------------------------
// InteriorMapEditorInitScene
//	 
//--------------------------------------
void	InteriorMapEditorInitScene( void )
{
	EngineDefaultState();
	InteriorMapEditorUpdateLightingMode( mnLightingMode );

	MouseCamUpdateCameraMatrices();

	// Initialise the camera
	EngineCameraUpdate();
	EngineCameraSetProjection( PI/4,0.002f,200.0f );
}


#define		SIZE_OF_ONE_TILE		(115.0f/256.0f)
float		mfFloorSize = SIZE_OF_ONE_TILE * 5.0f;
float		mfFloorSteps = SIZE_OF_ONE_TILE;


void	InteriorMapEditorDrawFloor( void )
{
MVECT		xPos1;
MVECT		xPos2;
int			nLoop;
	 
	xPos1.fX = -mfFloorSize;
	xPos1.fY = -mfFloorSize;
	xPos1.fZ = 0.0f;

	xPos2.fX = -mfFloorSize;
	xPos2.fY = mfFloorSize;
	xPos2.fZ = 0.0f;

	for ( nLoop = 0; nLoop < 11; nLoop++ )
	{
		AddLine( &xPos1, &xPos2, 0xA0000000, 0xA0000000 );
		xPos1.fX += mfFloorSteps;
		xPos2.fX += mfFloorSteps;
	}

	xPos1.fX = -mfFloorSize;
	xPos1.fY = -mfFloorSize;
	xPos1.fZ = 0.0f;
	xPos2.fX = mfFloorSize;
	xPos2.fY = -mfFloorSize;
	xPos2.fZ = 0.0f;

	for ( nLoop = 0; nLoop < 11; nLoop++ )
	{
		AddLine( &xPos1, &xPos2, 0xA0404040, 0xA0404040);
		xPos1.fY += mfFloorSteps;
		xPos2.fY += mfFloorSteps;
	}
}



//--------------------------------------
// InteriorMapEditorUpdateFrame
//	Called every frame to draw all the objects
//--------------------------------------
void	InteriorMapEditorUpdateFrame( void )
{
VECT	xPos;
VECT	xRot;

	memset( &xPos, 0, sizeof( xPos ) );
	memset( &xRot, 0, sizeof( xRot ) );

	EngineUpdate( TRUE );
	SceneMapUpdate( NULL );
	InteriorMapEditorNewFrame();
	InterfaceBeginRender();

	InteriorMapEditorDrawFloor();
	RenderLines( TRUE,FALSE, FALSE, FALSE );

	EngineDefaultState();
	SceneMapDisplay(&xPos, &xRot, 0);

	InterfaceEndRender();
	InterfacePresent( );
}


//--------------------------------------
// InteriorMapEditorMain
//		Main loop of the demo.. called once the 
//      engine and window have been initialised
//--------------------------------------
void	InteriorMapEditorInit( void )
{
	SysGetCurrentDir(256, mszExeDirectory );
	InteriorMapEditorInitScene();
	InteriorMapEditorResetCamera();
	SceneMapLoad( "interiortest.uim", NULL, "" );

//	InteriorMapEditorFreeAssets();
}


void		DebugPrint( int flag, const char* szText, ... )
{
	DemoFrameworkDebugPrint( szText );
}
