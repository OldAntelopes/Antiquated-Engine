
#include <StandardDef.h>
#include <Interface.h>
#include <stdio.h>
#include <System.h>
#include <Rendering.h>
#include <Engine.h>

#include "Framework/SceneEditor.h"
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

const char*		SceneEditorGetExeDirectory( void )
{
	return( mszExeDirectory );
}
 
//--------------------------------------
// SceneEditorLoadAssets
//	Load the model we're gonna see
//--------------------------------------
void	SceneEditorLoadAssets( void )
{
	mnDemoModelHandle = ModelLoad(mszModelFileName, 0, 1.0f );
	mnDemoTextureHandle = EngineLoadTexture(mszTextureFileName, 0 );
	mxModelPosition.x = mxModelPosition.y = mxModelPosition.z = 0.0f;
	mxModelRotation.x = mxModelRotation.y = mxModelRotation.z = 0.0f;
}

//--------------------------------------
// SceneEditorFreeAssets
//	 Tidyup and release memory used before exiting
//--------------------------------------
void	SceneEditorFreeAssets( void )
{
	ModelFree( mnDemoModelHandle );
}

int		mnLightingMode = 0;

void	SceneEditorUpdateLightingMode( int nMode )
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
// SceneEditorResetCamera
//	 
//--------------------------------------
void	SceneEditorResetCamera( void )
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
// SceneEditorInitScene
//	 
//--------------------------------------
void	SceneEditorInitScene( void )
{
	EngineDefaultState();
	SceneEditorUpdateLightingMode( mnLightingMode );

	MouseCamSetControlMode( CONTROL_MOVE_INVERT );
	MouseCamUpdateCameraMatrices();
	// Initialise the camera
	EngineCameraUpdate();
	EngineCameraSetProjection( PI/4,0.002f,200.0f );
}


#define		SIZE_OF_ONE_TILE		(115.0f/256.0f)
float		mfFloorSize = SIZE_OF_ONE_TILE * 5.0f;
float		mfFloorSteps = SIZE_OF_ONE_TILE;


void	SceneEditorDrawFloor( void )
{
MVECT		xPos1;
MVECT		xPos2;
int			nLoop;
	 
	xPos1.x = -mfFloorSize;
	xPos1.y = -mfFloorSize;
	xPos1.z = 0.0f;

	xPos2.x = -mfFloorSize;
	xPos2.y = mfFloorSize;
	xPos2.z = 0.0f;

	for ( nLoop = 0; nLoop < 11; nLoop++ )
	{
		AddLine( &xPos1, &xPos2, 0xA0000000, 0xA0000000 );
		xPos1.x += mfFloorSteps;
		xPos2.x += mfFloorSteps;
	}

	xPos1.x = -mfFloorSize;
	xPos1.y = -mfFloorSize;
	xPos1.z = 0.0f;
	xPos2.x = mfFloorSize;
	xPos2.y = -mfFloorSize;
	xPos2.z = 0.0f;

	for ( nLoop = 0; nLoop < 11; nLoop++ )
	{
		AddLine( &xPos1, &xPos2, 0xA0404040, 0xA0404040);
		xPos1.y += mfFloorSteps;
		xPos2.y += mfFloorSteps;
	}
}



//--------------------------------------
// SceneEditorUpdateFrame
//	Called every frame to draw all the objects
//--------------------------------------
void	SceneEditorUpdateFrame( void )
{
VECT	xPos;
VECT	xRot;

	memset( &xPos, 0, sizeof( xPos ) );
	memset( &xRot, 0, sizeof( xRot ) );

	EngineUpdate( TRUE );
	SceneMapUpdate( NULL );
	SceneEditorNewFrame();
	InterfaceBeginRender();

	MouseCamUpdateCameraMatrices();
	EngineCameraSetProjection( PI/4,0.002f,200.0f );

	SceneEditorDrawFloor();
	RenderLines( TRUE,FALSE, FALSE, FALSE );

	EngineDefaultState();

	VECT	vecFocus;
	int		nScreenX, nScreenY;

	SceneEditorGetFocusPoint( &vecFocus );
	EngineGetScreenCoordForWorldCoord( &vecFocus, &nScreenX, &nScreenY );
	InterfaceText( 1, nScreenX, nScreenY, "o", 0xD0D0D0D0, 0 ) ;
	SceneMapDisplay(&xPos, &xRot, 0);

	InterfaceDraw();
	InterfaceEndRender();
	InterfacePresent( );

	MouseCamUpdateCameraMatrices();
	EngineCameraSetProjection( PI/4,0.002f,200.0f );
}


//--------------------------------------
// SceneEditorMain
//		Main loop of the demo.. called once the 
//      engine and window have been initialised
//--------------------------------------
void	SceneEditorInit( void )
{
	SysGetCurrentDir(256, mszExeDirectory );
	SceneEditorInitScene();
	SceneEditorResetCamera();
//	SceneMapLoad( "interiortest.uim", NULL, "" );

//	SceneEditorFreeAssets();
}


void		DebugPrint( int flag, const char* szText, ... )
{
	DemoFrameworkDebugPrint( szText );
}
