#ifdef ENGINEDX		// ---- DirectX
#include "EngineDX.h"
#include "ShaderLoaderDX.h"
#endif

#include "StandardDef.h"
#include "Engine.h"
#include "Interface.h"
#include "Rendering.h"

#include "ShadowMapDX.h"

BOOL	mbShadowMapHasLoadedShaders = FALSE;
BOOL	msbShadowMapTexturesActive = FALSE;

LPGRAPHICSPIXELSHADER				mpShadowMapPass1PixelShader = NULL; 
LPGRAPHICSVERTEXSHADER				mpShadowMapPass1VertexShader = NULL; 
LPGRAPHICSCONSTANTBUFFER			mpShadowMapRenderConstantTable = NULL; 
LPGRAPHICSCONSTANTBUFFER			mpShadowMapRenderPSConstantTable = NULL; 

#define			DEFAULT_SHADOW_MAP_SIZE		2048

TEXTURE_HANDLE		mhShadowMapRenderTarget = NOTFOUND;

VECT		mxShadowMapCamTarget;

VECT		mxShadowMapCamPosStore;
VECT		mxShadowMapCamDirStore;
VECT		mxShadowMapCamUpStore;

float	mfShadowMapLightCamTargetDist = 45.0f;
float	mfShadowMapLightNearPlane = 1.0f;
float	mfShadowMapLightFarPlane = 100.0f;
float	mfShadowMapLightFOVMod = 1.0f;
int		mnShadowMapSize = DEFAULT_SHADOW_MAP_SIZE;


void		EngineShadowMapInit( void )
{

}



void		EngineShadowMapLoadShaders( void )
{
	if ( !mbShadowMapHasLoadedShaders )
	{
//		PANIC_IF( TRUE, "Shadow map system being used before its ready?" );
		mpShadowMapPass1VertexShader = EngineLoadVertexShader( "ShadowMapPass1", &mpShadowMapRenderConstantTable, 0 );
		mpShadowMapPass1PixelShader = EngineLoadPixelShader( "ShadowMapPass1", &mpShadowMapRenderPSConstantTable, 0 );

		mhShadowMapRenderTarget = EngineCreateRenderTargetTexture( mnShadowMapSize, mnShadowMapSize, 4 );	

		mbShadowMapHasLoadedShaders = TRUE;
	}
}

void		EngineShadowMapReleaseShaders( void )
{
	EngineReleaseTexture( &mhShadowMapRenderTarget );

	EngineReleaseConstantBuffer( &mpShadowMapRenderConstantTable );
	EngineReleaseConstantBuffer( &mpShadowMapRenderPSConstantTable );
	SAFE_RELEASE( mpShadowMapPass1PixelShader );
	SAFE_RELEASE( mpShadowMapPass1VertexShader );

	mbShadowMapHasLoadedShaders = FALSE;
}



void		EngineShadowMapGetLightAxes( VECT* pxLightPosOut, VECT* pxLightDirOut, VECT* pxLightUp )
{
VECT		xLightPos;
//VECT		xLightTarget;
VECT		xLightDir;
VECT	xUp = { 0.0f, 0.0f, 1.0f };
ENGINE_LIGHT		xPrimaryLight;
VECT	xRight;

	EngineGetPrimaryLight( &xPrimaryLight );
		
	VectScale( &xLightPos, &xPrimaryLight.Direction, -mfShadowMapLightCamTargetDist );
	VectAdd( &xLightPos, &xLightPos, &mxShadowMapCamTarget );

	xLightDir = xPrimaryLight.Direction;
	VectNormalize( &xLightDir );

	if ( ( xLightDir.x == 0.0f ) &&
		 ( xLightDir.y == 0.0f ) )
	{
		xUp.y = 1.0f;
		xUp.z = 0.0f;
	}
	VectNormalize( &xUp);

	VectCross( &xRight, &xLightDir, &xUp );
	VectCross( &xUp, &xRight, &xLightDir );
	VectRotateAboutAxis( &xUp, &xUp, &xLightDir, -A30 );

	*pxLightPosOut = xLightPos;
	*pxLightDirOut = xLightDir;
	*pxLightUp = xUp;
}

float		EngineShadowMapGetFarClipDist( void )
{
	return( mfShadowMapLightFarPlane );
}


void		EngineShadowMapGetLightViewProjMatrix( ENGINEMATRIX* pxMatrix )
{
VECT		xLightPos;
VECT		xLightTarget;
VECT		xLightDir;
VECT		xUp;
D3DXMATRIX	matProj;

	EngineShadowMapGetLightAxes( &xLightPos, &xLightDir, &xUp );

	xLightTarget = mxShadowMapCamTarget;

	EngineMatrixLookAt( pxMatrix, &xLightPos, &xLightTarget, &xUp );
	// near far need to be configurable...

#ifdef TUD11
	PANIC_IF( TRUE, "DX11 EngineShadowMapGetLightViewProjMatrix TBI" );
#else
//	mfShadowMapLightFOVMod = 0.15f;
//	D3DXMatrixPerspectiveFovLH( &matProj, fFOV * mfShadowMapLightFOVMod, 1.0f, mfShadowMapLightNearPlane, mfShadowMapLightFarPlane );
	D3DXMatrixOrthoLH(&matProj, (float)mnShadowMapSize * mfShadowMapLightFOVMod, (float)mnShadowMapSize * mfShadowMapLightFOVMod, mfShadowMapLightNearPlane, mfShadowMapLightFarPlane );
//	EngineSetProjectionMatrix( &Ortho2D);
#endif
	EngineMatrixMultiply( pxMatrix, (ENGINEMATRIX*)&matProj );
}

void		EngineSceneShadowsSetRenderParams( float fLightCamTargetDist, float fLightNearPlane, float fLightFarPlane, float fFOVMod )
{
	mfShadowMapLightCamTargetDist = fLightCamTargetDist;
	mfShadowMapLightNearPlane = fLightNearPlane;
	mfShadowMapLightFarPlane = fLightFarPlane;
	mfShadowMapLightFOVMod = fFOVMod;
}

BOOL		EngineShadowMapTexturesAreActive( void )
{
	return( msbShadowMapTexturesActive );
}

void	EngineShadowMapActivateTexture( BOOL bFlag )
{
	if ( ModelRenderingIsShadowPass() == TRUE )
	{
		msbShadowMapTexturesActive = bFlag;
		if ( bFlag )
		{
			EngineShaderConstantsSetFloat( mpShadowMapRenderPSConstantTable, "gHasDiffuseMap", 1.0 );
		}
		else
		{
			EngineShaderConstantsSetFloat( mpShadowMapRenderPSConstantTable, "gHasDiffuseMap", 0.0 );
		}
	}
}


void	EngineShadowMapSetWorldMatrix( const ENGINEMATRIX*	pxWorldMatrix )
{
	if ( mpShadowMapRenderConstantTable )
	{
		if ( pxWorldMatrix )
		{
			EngineShaderConstantsSetMatrix( mpShadowMapRenderConstantTable, "gMatWorld", pxWorldMatrix );
		}
		else
		{
		ENGINEMATRIX		xIdentity;

			EngineMatrixIdentity( &xIdentity );
			EngineShaderConstantsSetMatrix( mpShadowMapRenderConstantTable, "gMatWorld", &xIdentity );
		}
	}
}


int		EngineShadowMapGetSize( void )
{
	return( mnShadowMapSize );
}

//--------------------------------------------------------------
// EngineShadowMapSetConstants
//  Set constants for the ShadowMapPass1 shaders
//--------------------------------------------------------------
void		EngineShadowMapSetConstants( void )
{
D3DXMATRIX	xLightViewProj;

	if ( mpShadowMapRenderConstantTable )
	{
	D3DXMATRIX		xMatrix;
	VECT			xLightPos;
	VECT			xLightTarget;
	VECT			xUp;
	VECT			xLightDir;

		// Set lightViewProj matrix in shader
		EngineShadowMapGetLightViewProjMatrix( &xLightViewProj );
		EngineShaderConstantsSetMatrix( mpShadowMapRenderConstantTable, "gMatLightViewProjection", &xLightViewProj );

		EngineShadowMapGetLightAxes( &xLightPos, &xLightDir, &xUp );
		xLightTarget = mxShadowMapCamTarget;

		// ------- Set camera for viewing from the light position (so things like engine culling results will pass appropriately)
		EngineCameraSetPos( xLightPos.x, xLightPos.y, xLightPos.z );
		EngineCameraSetDirection( xLightDir.x, xLightDir.y, xLightDir.z );
		EngineCameraSetUpVect( xUp.x, xUp.y, xUp.z );
		EngineCameraUpdate();

		EngineCameraSetViewAspectOverride( 1.0f );
//		EngineCameraSetProjection( fFOV * mfShadowMapLightFOVMod, mfShadowMapLightNearPlane, mfShadowMapLightFarPlane );
#ifdef TUD11
		PANIC_IF( TRUE, "DX11 EngineShadowMapSetConstants TBI" );
#else
		D3DXMatrixOrthoLH(&xMatrix, (float)mnShadowMapSize * mfShadowMapLightFOVMod, (float)mnShadowMapSize * mfShadowMapLightFOVMod, mfShadowMapLightNearPlane, mfShadowMapLightFarPlane );
		EngineSetProjectionMatrix( &xMatrix);
#endif
	}
}

int		EngineShadowMapGetDepthMap( void )
{
	return( mhShadowMapRenderTarget );
}


void		EngineShadowMapInitDepthRenderPassAutoTarget( float fTargetDist )
{
//uint32	ulBackgroundCol = 0;//0x001f0000;//0xff000000;
uint32	ulBackgroundCol = 0xffffffff;
float	fVal = 1.0f;
VECT	xCamDir = *EngineCameraGetDirection();
VECT	xCamPos = *EngineCameraGetPos();

//	ulBackgroundCol = *( (uint32*)( &fVal ) );

	mxShadowMapCamPosStore = *EngineCameraGetPos();
	mxShadowMapCamDirStore = *EngineCameraGetDirection();
	mxShadowMapCamUpStore = *EngineCameraGetUpVect();

	if ( !mbShadowMapHasLoadedShaders )
	{
		EngineShadowMapLoadShaders();
	}

	VectScale( &xCamDir, &xCamDir, fTargetDist );
	VectAdd( &mxShadowMapCamTarget, &xCamPos, &xCamDir );

	ModelRenderingSetShadowPass( TRUE );
	EngineSetRenderTargetTexture( mhShadowMapRenderTarget, ulBackgroundCol, TRUE );
	EngineShadowMapSetConstants();

	EngineShadersStandardVertexDeclaration(0);

	EngineSetVertexShader(mpShadowMapPass1VertexShader, "ShadowMap" );
	EngineSetPixelShader(mpShadowMapPass1PixelShader, "ShadowMap" );


}

void		EngineShadowMapInitDepthRenderPass( const VECT* pxCamTarget )
{
//uint32	ulBackgroundCol = 0;//0x001f0000;//0xff000000;
uint32	ulBackgroundCol = 0xffffffff;
float	fVal = 1.0f;

//	ulBackgroundCol = *( (uint32*)( &fVal ) );

	mxShadowMapCamPosStore = *EngineCameraGetPos();
	mxShadowMapCamDirStore = *EngineCameraGetDirection();
	mxShadowMapCamUpStore = *EngineCameraGetUpVect();

	if ( !mbShadowMapHasLoadedShaders )
	{
		EngineShadowMapLoadShaders();
	}

	mxShadowMapCamTarget = *pxCamTarget;

	ModelRenderingSetShadowPass( TRUE );
	EngineSetRenderTargetTexture( mhShadowMapRenderTarget, ulBackgroundCol, TRUE );
	EngineShadowMapSetConstants();

	EngineShadersStandardVertexDeclaration(0);

	EngineSetVertexShader(mpShadowMapPass1VertexShader, "ShadowMapPass1" );
	EngineSetPixelShader(mpShadowMapPass1PixelShader, "ShadowMapPass1" );

}

void		EngineShadowMapEndDepthRenderPass( void )
{
	EngineCameraSetViewAspectOverride( 0.0f );
	EngineRestoreRenderTarget();
	EngineSetPixelShader( NULL, NULL );
	EngineSetVertexShader( NULL, NULL );

	ModelRenderingSetShadowPass( FALSE );

	EngineCameraSetPos( mxShadowMapCamPosStore.x, mxShadowMapCamPosStore.y, mxShadowMapCamPosStore.z );
	EngineCameraSetDirection( mxShadowMapCamDirStore.x, mxShadowMapCamDirStore.y, mxShadowMapCamDirStore.z );
	EngineCameraSetUpVect( mxShadowMapCamUpStore.x, mxShadowMapCamUpStore.y, mxShadowMapCamUpStore.z );

//	int	nOverlay = EngineTextureCreateInterfaceOverlay( 1, mhShadowMapRenderTarget );
//	InterfaceTexturedRect( nOverlay, 10, 10, 400, 400, 0xf0A08060, 0.0f, 0.0f, 1.0f, 1.0f );
}


