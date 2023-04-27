
#include <d3dx9.h>

#include "StandardDef.h"
#include "Interface.h"
#include "Engine.h"
#include "Rendering.h"

#include "../../../UniversalPub/LibCode/Engine/DirectX/ShaderLoaderDX.h"

#include "UI/UI.h"
#include "Platform\Platform.h"

uint32	mulLastTick = 0;


BOOL	mbTileSpriteGenHasLoadedShaders = FALSE;
LPDIRECT3DPIXELSHADER9			mpTileSpriteGenPixelShader = NULL; 
LPDIRECT3DVERTEXSHADER9			mpTileSpriteGenVertexShader = NULL; 
LPD3DXCONSTANTTABLE				mpTileSpriteGenConstantTable = NULL; 
LPD3DXCONSTANTTABLE				mpTileSpriteGenPSConstantTable = NULL; 

void		TreeSpriteGenLoadShaders( void )
{
	if ( !mbTileSpriteGenHasLoadedShaders )
	{
		mpTileSpriteGenVertexShader = EngineLoadVertexShader( "ShadowMTreeSpriteShaderapPass1", &mpTileSpriteGenConstantTable, 0 );
		mpTileSpriteGenPixelShader = EngineLoadPixelShader( "TreeSpriteShader", &mpTileSpriteGenPSConstantTable, 0 );
		mbTileSpriteGenHasLoadedShaders = TRUE;
	}
}

void		TreeSpriteGenReleaseShaders( void )
{
	SAFE_RELEASE( mpTileSpriteGenConstantTable );
	SAFE_RELEASE( mpTileSpriteGenPSConstantTable );
	SAFE_RELEASE( mpTileSpriteGenPixelShader );
	SAFE_RELEASE( mpTileSpriteGenVertexShader );
	mbTileSpriteGenHasLoadedShaders = FALSE;
}


void		TreeSpriteActivateShaders( void )
{
LPDIRECT3DDEVICE9	pEngineDevice = EngineGetDXDevice();

	EngineShadersStandardVertexDeclaration(0);

	EngineEnableBlend( TRUE );
	EngineEnableZTest( FALSE );
	EngineEnableZWrite( FALSE );
	pEngineDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	pEngineDevice->SetVertexShader(mpTileSpriteGenVertexShader);
	pEngineDevice->SetPixelShader(mpTileSpriteGenPixelShader);
	pEngineDevice->SetRenderState( D3DRS_SEPARATEALPHABLENDENABLE, 1 );
	pEngineDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCCOLOR);
	pEngineDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	pEngineDevice->SetRenderState( D3DRS_SRCBLENDALPHA, D3DBLEND_SRCALPHA);
	pEngineDevice->SetRenderState( D3DRS_DESTBLENDALPHA, D3DBLEND_ONE);
}

void		MainOnPress( int X, int Y )
{
	if ( UIOnPress( X, Y ) == FALSE ) 
	{
//		GameCameraOnPress( X, Y );
	}
	
}

void		MainOnRelease( int X, int Y )
{
	if ( UIOnRelease( X, Y ) == FALSE) 
	{
//		GameCameraOnRelease( X, Y );

	}

}

int		mhTreeModel = NOTFOUND;
int		mhTreeModelTexture = NOTFOUND;
int		mnTreeSpriteRenderTargetTexture = NOTFOUND;

void	TreeSpriteGenRenderTreeModel( void )
{
VECT		xCamPos;
VECT	xCamDir = { 0.0f, -0.5f, -0.7f };
VECT	xCamUp = { 0.0f, -1.0f, 0.0f };		
VECT	xModelPos = { 0.0f, 0.0f, 0.0f };
VECT	xModelRot = { 0.0f, 0.0f, A90 };
float	fZoom = 2.5f;
ENGINE_LIGHT	xLight;

	VectScale( &xCamPos, &xCamDir, -fZoom );
	xCamPos.z += (fZoom*0.3f);

	EngineDefaultState();
	EngineEnableLighting( TRUE );
	xLight.Ambient.a = 1.0f;
	xLight.Ambient.r = 0.8f;
	xLight.Ambient.g = 0.8f;
	xLight.Ambient.b = 0.8f;
	xLight.Diffuse.a = 1.0f;
	xLight.Diffuse.r = 0.2f;
	xLight.Diffuse.g = 0.2f;
	xLight.Diffuse.b = 0.2f;
	xLight.Direction.x = -1.0f;
	xLight.Direction.y = -1.0f;
	xLight.Direction.z = -1.0f;
	xLight.Type = DIRECTIONAL_LIGHT;
	EngineActivateLight( 0, &xLight );


	EngineCameraSetPos( xCamPos.x, xCamPos.y, xCamPos.z );
	EngineCameraSetDirection( xCamDir.x, xCamDir.y, xCamDir.z );
	EngineCameraSetUpVect( xCamUp.x, xCamUp.y, xCamUp.z );
	EngineCameraSetProjection( A45, 1.0f, 200.0f );
	EngineCameraUpdate();

	ModelRender( mhTreeModel, &xModelPos, &xModelRot, 0 );
}

void	TreeSpriteGenRenderSprite( void )
{
	InterfaceBeginRender();
 
	EngineSetRenderTargetTexture( mnTreeSpriteRenderTargetTexture, 0x00000000 );

	TreeSpriteActivateShaders();
	EngineSetMaterialBlendOverride( TRUE );
	TreeSpriteGenRenderTreeModel();
	EngineSetMaterialBlendOverride( FALSE );

	InterfaceEndRender();

	EngineRestoreRenderTarget();

	EngineExportTexture( mnTreeSpriteRenderTargetTexture, "output.png", 3 );

}

void	ToolInitialise( void )
{
	mnTreeSpriteRenderTargetTexture = EngineCreateRenderTargetTexture( 512, 512, 3 );
	mhTreeModel = ModelLoad( "Data\\TreeModel1.atm", 0, 1.0f );

	TreeSpriteGenLoadShaders();

	TreeSpriteGenRenderSprite();

}


void		MainInitialise( void )
{
	srand( SysGetTick() );
	InterfaceSetInitialSize( FALSE, 512, 512, FALSE );

	// Initialise - Create the window for viewing things in
	PlatformInit();

	// Initialise Renderer
	InterfaceInitDisplayDevice( 0 );

	// Initialise the interface 
	InterfaceInit( TRUE );
	EngineInit();

	// ** tool init here
	ToolInitialise();

	mulLastTick = SysGetTimeLong();
}



void	ToolUpdateFrame( float fDelta )
{
	InterfaceNewFrame( 0xFF0000a0 );

	TreeSpriteGenRenderTreeModel();

	InterfaceDraw();

	InterfaceEndRender();
	InterfacePresent();


}


void		MainUpdate( void )
{
uint32	ulThisTick = SysGetTick();
float	fDelta = (float)( ulThisTick - mulLastTick ) * 0.001f;

	if ( fDelta > 0.0f )
	{
		if ( fDelta > 0.5f ) fDelta = 0.5f;

		mulLastTick = ulThisTick;
		EngineUpdate( TRUE );

		// ** Game update
		ToolUpdateFrame( fDelta );
		// GameModeUpdate( fDelta );
		// GameModeRender( );
	}
	else
	{
		SysOSYield();
	}
}


void		MainShutdown( void )
{
	// ** Game shutdown here..
//	GameModeShutdown();

	EngineFree( TRUE );
	InterfaceFree();

}

