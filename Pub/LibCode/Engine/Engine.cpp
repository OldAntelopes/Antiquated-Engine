
#ifdef ENGINEDX
#include "DirectX/EngineDX.h"
#endif

#include <StandardDef.h>
#include <Engine.h>
#include <Rendering.h>
#include <Interface.h>

#ifdef USING_OPENGL
// TODO
#include "OpenGL\EngineGL.h"
#else
#include "DirectX/TextureManagerDX.h"
#include "DirectX/ShadowVolumeDX.h"
#include "DirectX/PostProcessDX.h"
#include "DirectX/OculusDX.h"
#include "DirectX/ShadowMapDX.h"
#endif

#include "EngineCamera.h"
#include "ModelRendering.h"
#include "ModelMaterialData.h"



EngineGameInterface*		m_pEngineGameInterface = NULL;
SceneMapGameInterface*		m_pSceneMapGameInterface = NULL;

BOOL	msbEngineInitialised = FALSE;
BOOL	msbEngineNoTexturesOverride = FALSE;
BOOL	msbEngineIsVRMode = FALSE;
BOOL	msbEngineMaterialBlendOverride = FALSE;
uint32	mulEngineLastFrameTick = 0;

//-------------------------------------------------------------------------------------------------

void		EngineEnableVR( BOOL bEnable )
{
	msbEngineIsVRMode = bEnable;
	InterfaceSetVRMode( bEnable );
	if ( bEnable )
	{
#ifndef USING_OPENGL
		InitializeOculusVR();
#endif
	}
}

BOOL		EngineIsVRMode( void )
{
	return( msbEngineIsVRMode );
}

BOOL		EngineGetMaterialBlendOverride( void )
{
	return( msbEngineMaterialBlendOverride );
}

void		EngineSetMaterialBlendOverride( BOOL bFlag )
{
	msbEngineMaterialBlendOverride = bFlag;
}

void		EngineDebugNoTextures( BOOL bFlag )
{
	msbEngineNoTexturesOverride = bFlag;
}

void					EngineSetGameInterface( EngineGameInterface* pGameInterface )
{
	m_pEngineGameInterface = pGameInterface;
}

EngineGameInterface*	EngineGetGameInterface( void )
{
	return( m_pEngineGameInterface );
}



void					EngineSetSceneMapGameInterface( SceneMapGameInterface* pGameInterface )
{
	m_pSceneMapGameInterface = pGameInterface;
}

SceneMapGameInterface*	EngineGetSceneMapGameInterface( void )
{
	return( m_pSceneMapGameInterface );
}




void		EngineRestart( void )
{
	EngineDefaultState();

#ifdef ENGINEDX
	ModelMaterialsLoadShader();
	ShadowVolumeInitDX();
#endif
}

void		EngineTests( void )
{
int		nCount = 500000;
int		nLoop = 0;
VECT	xVect;
u64		ullTestStartTick = SysGetMicrosecondTick();
u64		ullTestEndTick;
u64		ullResult;
float	afAngles[8] = { A45, A10, A30, 0.1f, A90, 0.1554f, 1.3f, A30 + A10 }; 
	xVect.x = 1.0f;
	xVect.y = 2.0f;
	xVect.z = 0.0f;

	for( nLoop = 0; nLoop < nCount; nLoop++ )
	{
		VectRotateAboutZ( &xVect, afAngles[ nLoop%8 ] );
	}

	ullTestEndTick = SysGetMicrosecondTick();

	ullResult = ullTestEndTick - ullTestStartTick;

	SysDebugPrint( "Test result: %lld", ullResult );
}

void		EngineInit( void )
{
	if (!msbEngineInitialised )
	{
#ifdef ENGINEDX
		EngineTextureManagerInitDX();
		EngineMathsInitSinTable();
#endif
		ModelRenderingInit();

		msbEngineInitialised = TRUE;
	}
	EngineCameraInit();
	EngineRestart();

/*
	EngineTests();
	EngineTests();
	EngineTests();
*/

}

void		EngineRestoreMainShaderState( BOOL bShouldRenderShadows )
{
	if ( EngineSceneShadowMapIsActive() == TRUE )
	{	
		if ( bShouldRenderShadows == FALSE )
		{
			EngineSetVertexShader( NULL, NULL );
			EngineSetPixelShader( NULL, NULL );
		}
		else
		{
			EngineSceneShadowsStartRender( TRUE, FALSE, FALSE );
			EngineEnableFog( 0 );
		}
	}
	else
	{
		EngineSetVertexShader( NULL, NULL );
		EngineSetPixelShader( NULL, NULL );
	}

}



void		EngineSceneShadowMapActivate( BOOL bFlag )
{
	ModelRenderingActivateSceneShadowMap( bFlag );
}

BOOL		EngineSceneShadowMapIsActive( void )
{
	return( ModelRenderingIsSceneShadowMapActive() );
}

void		EngineSceneShadowsInitShadowPass( const VECT* pxCamTarget )
{
#ifndef USING_OPENGL
	EngineShadowMapInitDepthRenderPass( pxCamTarget );
#endif
}

void		EngineSceneShadowsInitShadowPassAutoTarget( float fTargetDist )
{
#ifndef USING_OPENGL
VECT		xTarget;

	VectScale( &xTarget, EngineCameraGetDirection(), fTargetDist );
	VectAdd( &xTarget, &xTarget, EngineCameraGetPos() );
	EngineShadowMapInitDepthRenderPass( &xTarget );
#endif
}

void		EngineSceneShadowsEndShadowPass( void )
{
#ifndef USING_OPENGL
	EngineShadowMapEndDepthRenderPass();
#endif
}


uint32	EngineGetColValue( int R, int G, int B, int A )
{
uint32		ulCol;

	ulCol = (R << 16 ) | ( G << 8 ) | ( B ) | (A << 24);
	return( ulCol );
}


void		EngineSceneShadowsStartRender( BOOL bHasDiffuseMap, BOOL bHasNormalMap, BOOL bHasSpecularMap )
{
	if ( ModelRenderingIsShadowPass() == FALSE )
	{
		EngineSetShadowMultitexture( TRUE );
		ModelMaterialsActivateNormalShader( bHasDiffuseMap, bHasNormalMap, bHasSpecularMap );
	}
}

void		EngineSceneShadowsEndRender( void )
{
	if ( ModelRenderingIsShadowPass() == FALSE )
	{
		ModelMaterialsDeactivateNormalShader();
	}
}




void	EngineResetLighting( void )
{
#ifdef ENGINEDX
	EngineLightingDXNewFrame();
#endif
}

void	EngineUpdate( BOOL bNewFrame )
{
uint32	ulNewTick = SysGetTick();

#ifdef ENGINEDX
	EngineTextureManagerUpdateDX();
	if ( bNewFrame )
	{
		ShadowVolumeUpdate();
		EngineLightingDXNewFrame();
	}
#endif
	if ( bNewFrame )
	{
		if ( mulEngineLastFrameTick == 0 )
		{
			mulEngineLastFrameTick = ulNewTick;
		}
		ModelRenderingUpdate( ulNewTick - mulEngineLastFrameTick );
		mulEngineLastFrameTick = ulNewTick;

	}

#ifdef IW_SDK			// Marmalade
	EngineTextureManagerUpdateMarmalade();
#endif
}


void		EngineFree( BOOL boCompletely )
{
	if ( msbEngineInitialised )
	{
		ModelMaterialsFreeShader();
#ifdef ENGINEDX
		EngineTextureManagerFreeDX(boCompletely);		// should be EngineFreePlatform
		EngineShadowMapReleaseShaders();
		ShadowVolumeFree();
		EnginePostProcessFreeGraphics();
#endif

		if ( boCompletely )
		{
#ifdef ENGINEDX
			EngineMathsFreeSinTable();
			OculusReleaseGraphics();
#endif
			ModelRenderingFree();
			msbEngineInitialised = FALSE;
		}
	}
}


void		EngineActivateSimpleLight(int lightNum, int mode, unsigned int ulCol, VECT* pxVect )
{
ENGINE_LIGHT		xLight;
float		fColA, fColR, fColG, fColB;

	fColA = (float)( ulCol >> 24 );
	fColA /= 255.0f;
	fColR = (float)( ( ulCol >> 16 ) & 0xFF );
	fColR /= 255.0f;
	fColG = (float)( ( ulCol >> 8 ) & 0xFF );
	fColG /= 255.0f;
	fColB = (float)( ulCol & 0xFF );
	fColB /= 255.0f;

	memset( &xLight, 0, sizeof( xLight ) );

	switch( mode )
	{
	case 0:		// Basic directional with some ambient
		xLight.Type = DIRECTIONAL_LIGHT;
		xLight.Diffuse.a = 1.0f;
		xLight.Diffuse.r = fColR * 0.75f;
		xLight.Diffuse.g = fColG * 0.75f;
		xLight.Diffuse.b = fColB * 0.75f;
		xLight.Ambient.a = 1.0f;
		xLight.Ambient.r = fColR * 0.4f;
		xLight.Ambient.g = fColG * 0.4f;
		xLight.Ambient.b = fColB * 0.4f;

		if ( pxVect )
		{
			xLight.Direction = *pxVect;
		}
		else
		{
			xLight.Direction.x = 1.0f;
			xLight.Direction.y = -1.0f;
			xLight.Direction.z = -1.0f;
		}
		break;
	case 1:		// Basic directional with no ambient
		xLight.Type = DIRECTIONAL_LIGHT;
		xLight.Diffuse.a = fColA;
		xLight.Diffuse.r = fColR;
		xLight.Diffuse.g = fColG;
		xLight.Diffuse.b = fColB;
		if ( pxVect )
		{
			xLight.Direction = *pxVect;
		}
		else
		{
			xLight.Direction.x = -1.0f;
			xLight.Direction.y = -1.0f;
			xLight.Direction.z = -1.0f;
		}
		break;
	case 2:		// Basic directional with low ambient
		xLight.Type = DIRECTIONAL_LIGHT;
		xLight.Diffuse.a = 1.0f;
		xLight.Diffuse.r = fColR * 0.8f;
		xLight.Diffuse.g = fColG * 0.8f;
		xLight.Diffuse.b = fColB * 0.8f;
		xLight.Ambient.a = 1.0f;
		xLight.Ambient.r = fColR * 0.2f;
		xLight.Ambient.g = fColG * 0.2f;
		xLight.Ambient.b = fColB * 0.2f;

		if ( pxVect )
		{
			xLight.Direction = *pxVect;
		}
		else
		{
			xLight.Direction.x = 1.0f;
			xLight.Direction.y = -1.0f;
			xLight.Direction.z = -1.0f;
		}
		break;
	}

	VectNormalize( &xLight.Direction );

	EngineActivateLight( lightNum, &xLight );
}


void EngineAxesFromQuaternion( const ENGINEQUATERNION* pxQuat, VECT* pxForward, VECT* pxRight, VECT* pxUp )
{
ENGINEMATRIX		xMat;

	// NOTE - assuming Y is the normal forward direction, which happens to work for 'squadron' but probably needs
	// to be configurable for other arrangements..
	EngineMatrixFromQuaternion( &xMat, pxQuat );
	pxRight->x = xMat._11;
	pxRight->y = xMat._12;
	pxRight->z = xMat._13;
	pxForward->x = xMat._21;
	pxForward->y = xMat._22;
	pxForward->z = xMat._23;
	pxUp->x = xMat._31;
	pxUp->y = xMat._32;
	pxUp->z = xMat._33;

}

void	EngineQuaternionFromAxes( ENGINEQUATERNION* pxQuatOut, const VECT* pxForward, const VECT* pxRight, const VECT* pxUp )
{
ENGINEMATRIX		xMat;
ENGINEMATRIX*		pMatrix = &xMat;
	
	// NOTE - assuming Y is the normal forward direction, which happens to work for 'squadron' but probably needs
	// to be configurable for other arrangements..

	pMatrix->_11 = pxRight->x;
	pMatrix->_12 = pxRight->y;
	pMatrix->_13 = pxRight->z;
	pMatrix->_14 = 0.0f;
	pMatrix->_21 = pxForward->x;
	pMatrix->_22 = pxForward->y;
	pMatrix->_23 = pxForward->z;
	pMatrix->_24 = 0.0f;
	pMatrix->_31 = pxUp->x;
	pMatrix->_32 = pxUp->y;
	pMatrix->_33 = pxUp->z;
	pMatrix->_34 = 0.0f;
	pMatrix->_41 = 0.0f;
	pMatrix->_42 = 0.0f;
	pMatrix->_43 = 0.0f;
	pMatrix->_44 = 1.0f;

	EngineQuaternionFromMatrix( pxQuatOut, &xMat );
	EngineQuaternionNormalize( pxQuatOut );
}
