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
float		msfShadowStrength = 1.0f;

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
int		mnShadowMapSize = DEFAULT_SHADOW_MAP_SIZE ;
static BOOL		msbShadowMapTexelSnap = FALSE;

// The width (and height) of the light's orthographic view is this times mfShadowMapLightFOVMod, whatever the map's size
#define			SHADOW_MAP_VIEW_UNITS		2048.0f

// Cascades (opt-in, EngineSceneShadowsSetCascades). With 2, the render target is a 2:1 atlas: the usual ("far") view
// in the left half and a "near" view, the same light and centre but msfShadowMapNearCascadeScale times smaller, in the
// right half. Both are mnShadowMapSize square. Because the near view only differs by a scale in x and y, receivers
// work out its coordinates from their existing light-space position (gShadowCascadeInfo), with no extra matrix.
#define			SHADOW_MAP_MAX_ATLAS_WIDTH	8192
static int		msnShadowMapCascadesWanted = 1;
static int		msnShadowMapCascades = 1;			// in use: what the render target was made for
static float	msfShadowMapNearCascadeScale = 4.0f;
static int		msnShadowMapCurrentCascade = 0;
static float	msfShadowMapCascadeBlend = 0.2f;

void		EngineShadowMapSetSizeOption( int nSizeOption )
{
int		nSize;

	switch( nSizeOption )
	{
	case 0:
	case 1:
	default:
		nSize = DEFAULT_SHADOW_MAP_SIZE;		// 2048
		break;
	case 2:
		nSize = DEFAULT_SHADOW_MAP_SIZE * 2;		// 4096
		break;
	case 3:
		nSize = DEFAULT_SHADOW_MAP_SIZE * 4;		// 8192
		break;
	case 4:
		nSize = DEFAULT_SHADOW_MAP_SIZE / 2;		// 1024
		break;
	}

	if ( nSize != mnShadowMapSize )
	{
		mnShadowMapSize = nSize;
		if ( mbShadowMapHasLoadedShaders )
		{
			EngineShadowMapReleaseShaders();
			EngineShadowMapLoadShaders();
		}
	}
}

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

		msnShadowMapCascades = msnShadowMapCascadesWanted;
		if ( ( msnShadowMapCascades > 1 ) &&
			 ( mnShadowMapSize * msnShadowMapCascades > SHADOW_MAP_MAX_ATLAS_WIDTH ) )
		{
			msnShadowMapCascades = 1;
		}
		mhShadowMapRenderTarget = EngineCreateRenderTargetTexture( mnShadowMapSize * msnShadowMapCascades, mnShadowMapSize, 4, "ShadowMap" );

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

float	EngineShadowMapGetStrength( void )
{
	return( msfShadowStrength );
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
	xPrimaryLight.Direction.z -= 0.02f;
	if ( xPrimaryLight.Direction.z > -0.05f )
	{
	float	fRange = 0.05f - 0.001f;
	float	fShadowStrength = xPrimaryLight.Direction.z + 0.001f;
		fShadowStrength *= -1.0f / fRange;
		fShadowStrength = FClamp( fShadowStrength, 0.0f, 1.0f );
		msfShadowStrength = fShadowStrength;
		if ( xPrimaryLight.Direction.z > -0.005f )
		{
			xPrimaryLight.Direction.z = -0.005f;
		}
	}
	else
	{
		msfShadowStrength = 1.0f;
	}
	VectNormalize( &xPrimaryLight.Direction );

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
	int		nViewMod = (int)SHADOW_MAP_VIEW_UNITS;
//	int		nViewMod = mnShadowMapSize;
	D3DXMatrixOrthoLH(&matProj, (float)nViewMod * mfShadowMapLightFOVMod, (float)nViewMod * mfShadowMapLightFOVMod, mfShadowMapLightNearPlane, mfShadowMapLightFarPlane );
//	EngineSetProjectionMatrix( &Ortho2D);
#endif
	EngineMatrixMultiply( pxMatrix, (ENGINEMATRIX*)&matProj );
}

// Public (Engine.h): the light's view-projection for the current shadow pass (valid after EngineSceneShadowsInitShadowPass),
// so the app can cull shadow casters against the light's volume
void		EngineSceneShadowsGetLightViewProjection( ENGINEMATRIX* pxMatrixOut )
{
	EngineShadowMapGetLightViewProjMatrix( pxMatrixOut );

	// The near cascade sees the same view scaled up in x and y (EngineSceneShadowsBeginCascade)
	if ( ( msnShadowMapCascades > 1 ) &&
		 ( msnShadowMapCurrentCascade == 1 ) )
	{
	ENGINEMATRIX	xScale;

		EngineMatrixIdentity( &xScale );
		xScale._11 = msfShadowMapNearCascadeScale;
		xScale._22 = msfShadowMapNearCascadeScale;
		EngineMatrixMultiply( pxMatrixOut, &xScale );
	}
}

// Public (Engine.h): opt-in texel snapping, off by default
void		EngineSceneShadowsSetTexelSnap( BOOL bFlag )
{
	msbShadowMapTexelSnap = bFlag;
}

// Public (Engine.h): opt-in cascades, 1 (the default, as before) or 2. fNearScale is how many times smaller the near
// view is than the usual one; a whole number keeps texel snapping exact for both. Changing the count remakes the render
// target at the next shadow pass.
void		EngineSceneShadowsSetCascades( int nCascades, float fNearScale )
{
	if ( nCascades < 1 ) nCascades = 1;
	if ( nCascades > 2 ) nCascades = 2;
	if ( fNearScale < 1.0f ) fNearScale = 1.0f;
	msfShadowMapNearCascadeScale = fNearScale;

	if ( nCascades != msnShadowMapCascadesWanted )
	{
		msnShadowMapCascadesWanted = nCascades;
		if ( mbShadowMapHasLoadedShaders )
		{
			EngineShadowMapReleaseShaders();
			// (reloaded by the next EngineShadowMapInitDepthRenderPass)
		}
	}
}

int			EngineSceneShadowsGetCascades( void )
{
	if ( mbShadowMapHasLoadedShaders ) return( msnShadowMapCascades );
	return( msnShadowMapCascadesWanted );
}

// Sets the shadow pass vertex shader's cascade constant (ShadowMapPass1.vsh, gShadowCascade: x = scale applied to the
// light's x/y, y = atlas offset in clip space, z = atlas scale in x). A shader without the constant ignores it.
static void	EngineShadowMapSetCascadeConstant( void )
{
VECT	xCascade;

	if ( mpShadowMapRenderConstantTable == NULL ) return;

	if ( msnShadowMapCascades <= 1 )
	{
		xCascade.x = 1.0f;
		xCascade.y = 0.0f;
		xCascade.z = 1.0f;
	}
	else if ( msnShadowMapCurrentCascade == 0 )
	{
		xCascade.x = 1.0f;
		xCascade.y = -0.5f;
		xCascade.z = 0.5f;
	}
	else
	{
		xCascade.x = msfShadowMapNearCascadeScale;
		xCascade.y = 0.5f;
		xCascade.z = 0.5f;
	}
	EngineShaderConstantsSetVect( mpShadowMapRenderConstantTable, "gShadowCascade", &xCascade );
}

// Public (Engine.h): between EngineSceneShadowsInitShadowPass and EngineSceneShadowsEndShadowPass, picks which cascade
// the casters drawn next go into. 0 = the usual view (set by InitShadowPass), 1 = the near one.
void		EngineSceneShadowsBeginCascade( int nCascade )
{
	if ( ( nCascade < 0 ) || ( nCascade >= msnShadowMapCascades ) ) nCascade = 0;
	msnShadowMapCurrentCascade = nCascade;
	EngineShadowMapSetCascadeConstant();
}

// Public (Engine.h): how wide the band is where receivers mix the near and usual cascades, as a fraction of the near
// view's half-width (0 = a hard border). Only passed on to the receiver shaders.
void		EngineSceneShadowsSetCascadeBlend( float fBand )
{
	if ( fBand < 0.0f ) fBand = 0.0f;
	if ( fBand > 1.0f ) fBand = 1.0f;
	msfShadowMapCascadeBlend = fBand;
}

// For the receivers' constant gShadowCascadeInfo: x = cascades in the map, y = near scale, z = 1 / atlas width in
// texels, w = the cascade blend band (EngineSceneShadowsSetCascadeBlend)
void		EngineShadowMapGetCascadeInfo( float* pafOut4 )
{
	pafOut4[0] = (float)( msnShadowMapCascades );
	pafOut4[1] = msfShadowMapNearCascadeScale;
	pafOut4[2] = 1.0f / (float)( mnShadowMapSize * msnShadowMapCascades );
	pafOut4[3] = msfShadowMapCascadeBlend;
}

//--------------------------------------------------------------
// EngineShadowMapSnapTargetToTexels
//  With texel snapping on, moves the shadow map's centre (mxShadowMapCamTarget) to the nearest whole shadow-map texel
//  across the light's view. The orthographic light view then only ever moves in whole texels, so a static object's
//  shadow covers the same texels from frame to frame instead of crawling as the camera moves. The light's axes don't
//  depend on the target, so they're the same before and after the snap. Depth along the light isn't snapped: it only
//  moves the stored depths, not where the texels fall.
//--------------------------------------------------------------
static void	EngineShadowMapSnapTargetToTexels( void )
{
VECT	xLightPos;
VECT	xLightDir;
VECT	xUp;
VECT	xRight;
VECT	xSnapped;
VECT	xPart;
float	fTexelSize;
float	fAlongRight;
float	fAlongUp;
float	fAlongDir;

	if ( msbShadowMapTexelSnap == FALSE ) return;

	EngineShadowMapGetLightAxes( &xLightPos, &xLightDir, &xUp );
	VectNormalize( &xLightDir );
	VectCross( &xRight, &xLightDir, &xUp );
	VectNormalize( &xRight );
	VectCross( &xUp, &xRight, &xLightDir );
	VectNormalize( &xUp );

	fTexelSize = ( SHADOW_MAP_VIEW_UNITS * mfShadowMapLightFOVMod ) / (float)( mnShadowMapSize );
	if ( fTexelSize <= 0.0f ) return;

	fAlongRight = VectDot( &mxShadowMapCamTarget, &xRight );
	fAlongUp = VectDot( &mxShadowMapCamTarget, &xUp );
	fAlongDir = VectDot( &mxShadowMapCamTarget, &xLightDir );

	fAlongRight = floorf( ( fAlongRight / fTexelSize ) + 0.5f ) * fTexelSize;
	fAlongUp = floorf( ( fAlongUp / fTexelSize ) + 0.5f ) * fTexelSize;

	VectScale( &xSnapped, &xRight, fAlongRight );
	VectScale( &xPart, &xUp, fAlongUp );
	VectAdd( &xSnapped, &xSnapped, &xPart );
	VectScale( &xPart, &xLightDir, fAlongDir );
	VectAdd( &xSnapped, &xSnapped, &xPart );
	mxShadowMapCamTarget = xSnapped;
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
	EngineShadowMapSnapTargetToTexels();

	ModelRenderingSetShadowPass( TRUE );
	EngineSetRenderTargetTexture( mhShadowMapRenderTarget, ulBackgroundCol, TRUE );
	EngineShadowMapSetConstants();
	msnShadowMapCurrentCascade = 0;
	EngineShadowMapSetCascadeConstant();

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
	EngineShadowMapSnapTargetToTexels();

	ModelRenderingSetShadowPass( TRUE );
	EngineSetRenderTargetTexture( mhShadowMapRenderTarget, ulBackgroundCol, TRUE );
	EngineShadowMapSetConstants();
	msnShadowMapCurrentCascade = 0;
	EngineShadowMapSetCascadeConstant();

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


