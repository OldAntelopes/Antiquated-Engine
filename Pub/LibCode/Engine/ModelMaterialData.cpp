

#ifdef USING_OPENGL

#endif

#ifdef ENGINEDX		// ---- DirectX
#include "DirectX/EngineDX.h"
#include "DirectX/ShaderLoaderDX.h"
#endif
#ifdef IW_SDK

#endif

#include <stdio.h>
#include <StandardDef.h>
#include <Engine.h>
#include <Rendering.h>
#include "Interface.h"

#include "ModelRendering.h"
#include "ModelMaterialData.h"

#ifdef ENGINEDX
#include "DirectX/ShadowMapDX.h"

LPGRAPHICSPIXELSHADER			mpNormalMapRenderPixelShader = NULL; 
LPGRAPHICSVERTEXSHADER			mpNormalMapRenderVertexShader = NULL; 
LPGRAPHICSCONSTANTBUFFER		mpNormalMapRenderConstantTable = NULL; 
LPGRAPHICSCONSTANTBUFFER		mpNormalMapRenderPSConstantTable = NULL; 
#endif

MATERIAL_COLOUR		mxModelMaterialShaderDiffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
MATERIAL_COLOUR		mxModelMaterialShaderAmbient = { 1.0f, 1.0f, 1.0f, 1.0f };
MATERIAL_COLOUR		mxModelMaterialShaderSpecular = { 1.0f, 1.0f, 1.0f, 1.0f };
MATERIAL_COLOUR		mxModelMaterialShaderEmissive = { 1.0f, 1.0f, 1.0f, 1.0f };
float				mModelMaterialShaderSpecularPower = 0.0f;

float	mfShadowEpsilon = 0.00005f;

BOOL		mbModelMaterialsHasLoadedShaders = FALSE;

D3DXVECTOR4		mxEmssiveVecValue( 0.0f, 0.0f, 0.0f, 0.0f );

//----------------------------------------------------------------------------------------------------

void		ModelMaterialShaderSetMaterialProperties( MATERIAL_COLOUR* pxDiffuse, MATERIAL_COLOUR* pxAmbient, MATERIAL_COLOUR* pxSpecular, MATERIAL_COLOUR* pxEmissive, float fSpecularPower )
{
	if ( ( pxDiffuse->fRed != mxModelMaterialShaderDiffuse.fRed ) ||
		 ( pxDiffuse->fGreen != mxModelMaterialShaderDiffuse.fGreen ) ||
		 ( pxDiffuse->fBlue != mxModelMaterialShaderDiffuse.fBlue ) ||
		 ( pxDiffuse->fAlpha != mxModelMaterialShaderDiffuse.fAlpha ) ||
		 ( pxAmbient->fRed != mxModelMaterialShaderAmbient.fRed ) ||
		 ( pxAmbient->fGreen != mxModelMaterialShaderAmbient.fGreen ) ||
		 ( pxAmbient->fBlue != mxModelMaterialShaderAmbient.fBlue ) ||
		 ( pxSpecular->fRed != mxModelMaterialShaderSpecular.fRed ) ||
		 ( pxSpecular->fGreen != mxModelMaterialShaderSpecular.fGreen ) ||
		 ( pxSpecular->fBlue != mxModelMaterialShaderSpecular.fBlue ) ||
		 ( pxEmissive->fRed != mxModelMaterialShaderEmissive.fRed ) ||
		 ( pxEmissive->fGreen != mxModelMaterialShaderEmissive.fGreen ) ||
		 ( pxEmissive->fBlue != mxModelMaterialShaderEmissive.fBlue ) ||
		 ( fSpecularPower != mModelMaterialShaderSpecularPower ) )
	{
		mxModelMaterialShaderDiffuse = *pxDiffuse;
		mxModelMaterialShaderAmbient = *pxAmbient;
		mxModelMaterialShaderSpecular = *pxSpecular;
		mxModelMaterialShaderEmissive = *pxEmissive;

		mxModelMaterialShaderEmissive.fAlpha = 1.0f;
		mxModelMaterialShaderAmbient.fAlpha = 1.0f;
		
		mModelMaterialShaderSpecularPower = fSpecularPower;
#ifdef ENGINEDX
#ifdef TUD9
		D3DXHANDLE handle; 

		if ( mpNormalMapRenderPSConstantTable)
		{
			handle = mpNormalMapRenderPSConstantTable->GetConstantByName(NULL, "gMaterialDiffuse");
			if ( handle )
			{
				mpNormalMapRenderPSConstantTable->SetFloatArray( mpEngineDevice, handle, (float*)&mxModelMaterialShaderDiffuse, 4 );
			}

			handle = mpNormalMapRenderPSConstantTable->GetConstantByName(NULL, "gMaterialSpecular");
			if ( handle )
			{
				mpNormalMapRenderPSConstantTable->SetFloatArray( mpEngineDevice, handle, (float*)&mxModelMaterialShaderSpecular, 4 );
			}

			handle = mpNormalMapRenderPSConstantTable->GetConstantByName(NULL, "gMaterialAmbient");
			if ( handle )
			{
				mpNormalMapRenderPSConstantTable->SetFloatArray( mpEngineDevice, handle, (float*)&mxModelMaterialShaderAmbient, 4 );
			}

			handle = mpNormalMapRenderPSConstantTable->GetConstantByName(NULL, "gMaterialEmissive");
			if ( handle )
			{
				mpNormalMapRenderPSConstantTable->SetFloatArray( mpEngineDevice, handle, (float*)&mxModelMaterialShaderEmissive, 4 );
			}

			handle = mpNormalMapRenderPSConstantTable->GetConstantByName(NULL, "gMaterialSpecularPower");
			if ( handle )
			{
				mpNormalMapRenderPSConstantTable->SetFloat( mpEngineDevice, handle, mModelMaterialShaderSpecularPower );
			}
		
		}
#endif
#endif
	}
}


void		ModelMaterialsFreeShader( void )
{
#ifdef ENGINEDX
#ifdef TUD9
	SAFE_RELEASE( mpNormalMapRenderConstantTable );
	SAFE_RELEASE( mpNormalMapRenderPSConstantTable );
#endif
	SAFE_RELEASE( mpNormalMapRenderPixelShader );
	SAFE_RELEASE( mpNormalMapRenderVertexShader );
#endif 
	mbModelMaterialsHasLoadedShaders = FALSE;
}


void		ModelMaterialsLoadShader( void )
{
	if ( !mbModelMaterialsHasLoadedShaders )
	{
		mbModelMaterialsHasLoadedShaders = TRUE;

#ifdef ENGINEDX
		if ( mpEngineDevice )
		{
#ifdef TUD11
			mpNormalMapRenderVertexShader = EngineLoadVertexShader( "ATMMaterial", NULL, 0 );
			mpNormalMapRenderPixelShader = EngineLoadPixelShader( "ATMMaterial", NULL, 0 );
#else
			mpNormalMapRenderVertexShader = EngineLoadVertexShader( "ATMMaterial", &mpNormalMapRenderConstantTable, 0 );
			mpNormalMapRenderPixelShader = EngineLoadPixelShader( "ATMMaterial", &mpNormalMapRenderPSConstantTable, 0 );
#endif
		}
#endif
	}
}

void		EngineReloadShaders( void )
{
	ModelMaterialsFreeShader();
	ModelMaterialsLoadShader();

#ifdef TUD9
	EngineShadowMapReleaseShaders();
	EngineShadowMapLoadShaders();
#endif
}

void		ModelMaterialsDeactivateNormalShader( void )
{
	EngineSetShadowMultitexture( FALSE );
	EngineSetTexture( 2, NOTFOUND );
	EngineSetTexture( 3, NOTFOUND );
#ifdef ENGINEDX
	EngineSetVertexShader(NULL, NULL );
	EngineSetPixelShader(NULL, NULL );
#endif
}

void		ModelMaterialsNormalShaderUpdateZBias( float fZBias )
{
#ifdef TUD9
	if ( mpNormalMapRenderConstantTable )
	{
	D3DXHANDLE handle; 
		handle = mpNormalMapRenderConstantTable->GetConstantByName(NULL, "gZBias");
		if ( handle )
		{
			mpNormalMapRenderConstantTable->SetFloat( mpEngineDevice, handle, fZBias );
		}
	}
#endif
}

void		ModelMaterialsNormalShaderUpdateWorldTransform( const ENGINEMATRIX* pMatWorld )
{
#ifdef TUD9
	if ( mpNormalMapRenderConstantTable )
	{
	D3DXHANDLE handle; 
		handle = mpNormalMapRenderConstantTable->GetConstantByName(NULL, "gMatWorld");
		if ( handle )
		{
			if ( pMatWorld )
			{
				mpNormalMapRenderConstantTable->SetMatrix(mpEngineDevice, handle, (D3DXMATRIX*)pMatWorld );		
			}
			else
			{
			D3DXMATRIX Identity; 
				D3DXMatrixIdentity(&Identity);
				mpNormalMapRenderConstantTable->SetMatrix(mpEngineDevice, handle, (D3DXMATRIX*)&Identity );		
			}
		}
	}
#endif
}


void		EngineSceneShadowsSetShaderEpsilon( float fVal )
{
	mfShadowEpsilon = fVal;
}

float		EngineSceneShadowsGetShaderEpsilon( void )
{
	return( mfShadowEpsilon );
}

void		ModelMaterialsEnableLighting( BOOL bLightingEnable )
{
#ifdef ENGINEDX
	if ( mpNormalMapRenderPSConstantTable )
	{
	D3DXHANDLE handle; 

		handle = mpNormalMapRenderPSConstantTable->GetConstantByName(NULL, "gLightingEnabled");
		if ( handle )
		{
			mpNormalMapRenderPSConstantTable->SetBool(mpEngineDevice, handle, bLightingEnable );		
		}
	}
#endif
}

// Public (ModelMaterialData.h): set a float4 constant by name on the model material pixel shader, if it declares one
void		ModelMaterialsShaderSetFloat4( const char* szConstantName, const float* pafValues )
{
	if ( mpNormalMapRenderPSConstantTable )
	{
	D3DXHANDLE handle = mpNormalMapRenderPSConstantTable->GetConstantByName( NULL, szConstantName );

		if ( handle )
		{
			mpNormalMapRenderPSConstantTable->SetFloatArray( mpEngineDevice, handle, pafValues, 4 );
		}
	}
}

// Public (ModelMaterialData.h): set a float4 constant by name on the model material vertex shader, if it declares one
void		ModelMaterialsVertexShaderSetFloat4( const char* szConstantName, const float* pafValues )
{
	if ( mpNormalMapRenderConstantTable )
	{
	D3DXHANDLE handle = mpNormalMapRenderConstantTable->GetConstantByName( NULL, szConstantName );

		if ( handle )
		{
			mpNormalMapRenderConstantTable->SetFloatArray( mpEngineDevice, handle, pafValues, 4 );
		}
	}
}
void		ModelMaterialsShaderEnableOverlay( BOOL bHasOverlay )
{
	if ( mpNormalMapRenderPSConstantTable )
	{
	D3DXHANDLE handle; 

		if ( bHasOverlay == TRUE )
		{
			handle = mpNormalMapRenderPSConstantTable->GetConstantByName(NULL, "gHasNormalMap");
			if ( handle )
			{
				mpNormalMapRenderPSConstantTable->SetFloat( mpEngineDevice, handle, 0.0f );
			}

			handle = mpNormalMapRenderPSConstantTable->GetConstantByName(NULL, "gHasOverlay");
			if ( handle )
			{
				mpNormalMapRenderPSConstantTable->SetFloat( mpEngineDevice, handle, 1.0f );
			}
		}
		else
		{
			handle = mpNormalMapRenderPSConstantTable->GetConstantByName(NULL, "gHasOverlay");
			if ( handle )
			{
				mpNormalMapRenderPSConstantTable->SetFloat( mpEngineDevice, handle, 0.0f );
			}
		}
	}
}

void		ModelMaterialsActivateNormalShader( BOOL bHasDiffuseMap, BOOL bHasNormalMap, BOOL bHasSpecularMap )
{
#ifdef TUD9
D3DXHANDLE handle; 
D3DXMATRIX matWorld, matView, matProj;
D3DXMATRIX	matLightViewProj;

	if ( !mbModelMaterialsHasLoadedShaders )
	{
		ModelMaterialsLoadShader();
	}

	if ( mpNormalMapRenderConstantTable )
	{
		mpEngineDevice->GetTransform(D3DTS_WORLD, &matWorld);
		mpEngineDevice->GetTransform(D3DTS_VIEW, &matView);
		mpEngineDevice->GetTransform(D3DTS_PROJECTION, &matProj);

		// -------- vsh constants
		handle = mpNormalMapRenderConstantTable->GetConstantByName(NULL, "gMatWorld");
		if ( handle )
		{
			mpNormalMapRenderConstantTable->SetMatrix(mpEngineDevice, handle, &matWorld);		
		}

		handle = mpNormalMapRenderConstantTable->GetConstantByName(NULL, "gMatView");
		if ( handle )
		{
			mpNormalMapRenderConstantTable->SetMatrix(mpEngineDevice, handle, &matView);		
		}
		
		
		handle = mpNormalMapRenderConstantTable->GetConstantByName(NULL, "gMatLightViewProj");
		if ( handle )
		{
		ENGINEMATRIX		xMatrix;
			
			EngineShadowMapGetLightViewProjMatrix( &xMatrix );

			mpNormalMapRenderConstantTable->SetMatrix( mpEngineDevice, handle, (D3DXMATRIX*) &xMatrix );

			matLightViewProj = * ( (D3DXMATRIX*) &xMatrix );
		}

		handle = mpNormalMapRenderConstantTable->GetConstantByName(NULL, "gMatProj");
		if ( handle )
		{
			mpNormalMapRenderConstantTable->SetMatrix(mpEngineDevice, handle, &matProj);		
		}
		handle = mpNormalMapRenderConstantTable->GetConstantByName(NULL, "gEyePosition");
		if ( handle )
		{
		VECT	xEyePosition = *EngineCameraGetPos();
			mpNormalMapRenderConstantTable->SetFloatArray( mpEngineDevice, handle, (float*)&xEyePosition, 3 );
		}

		// -------- psh constants
		// float4 AmbientColor;
		// --float AmbientIntensity;

		// -- float3 LightDirection;
		// float4 DiffuseColor;
		// --float DiffuseIntensity;
 
		// float4 SpecularColor;
		// -- float3 EyePosition;
		if ( mpNormalMapRenderPSConstantTable )
		{
		ENGINE_LIGHT		xPrimaryLight;

			PixelShaderSetPointLights( mpNormalMapRenderPSConstantTable );
			PixelShaderSetFog( mpNormalMapRenderPSConstantTable );

			EngineGetPrimaryLight( &xPrimaryLight );
			handle = mpNormalMapRenderPSConstantTable->GetConstantByName(NULL, "gLightDirection");
			if ( handle )
			{
			VECT	xLightDirection;

				xLightDirection = xPrimaryLight.Direction;
				VectNormalize( &xLightDirection );
				mpNormalMapRenderPSConstantTable->SetFloatArray( mpEngineDevice, handle, (float*)&xLightDirection, 3 );
			}

			handle = mpNormalMapRenderPSConstantTable->GetConstantByName(NULL, "gMaterialDiffuse");
			if ( handle )
			{
				mpNormalMapRenderPSConstantTable->SetFloatArray( mpEngineDevice, handle, (float*)&mxModelMaterialShaderDiffuse, 4 );
			}
			handle = mpNormalMapRenderPSConstantTable->GetConstantByName(NULL, "gMaterialAmbient");
			if ( handle )
			{
				mpNormalMapRenderPSConstantTable->SetFloatArray( mpEngineDevice, handle, (float*)&mxModelMaterialShaderAmbient, 4 );
			}

			handle = mpNormalMapRenderPSConstantTable->GetConstantByName(NULL, "xCamPos");
			if ( handle )
			{
			VECT*		pxCamPos = EngineCameraGetPos();
				mpNormalMapRenderPSConstantTable->SetFloatArray( mpEngineDevice, handle, (float*)pxCamPos, 3 );
			}

			handle = mpNormalMapRenderPSConstantTable->GetConstantByName(NULL, "gShadowStrength");
			if ( handle )
			{
			float		fShadowStrength = (float)(EngineShadowMapGetStrength() );
				mpNormalMapRenderPSConstantTable->SetFloat( mpEngineDevice, handle, fShadowStrength );
			}

			handle = mpNormalMapRenderPSConstantTable->GetConstantByName(NULL, "gShadowMapSize");
			if ( handle )
			{
			float		fShadowMapSize = (float)(EngineShadowMapGetSize() );
				mpNormalMapRenderPSConstantTable->SetFloat( mpEngineDevice, handle, fShadowMapSize );
			}
			
			handle = mpNormalMapRenderPSConstantTable->GetConstantByName(NULL, "gShadowEpsilon");
			if ( handle )
			{
			float		fShadowEpsilon = mfShadowEpsilon;
				mpNormalMapRenderPSConstantTable->SetFloat( mpEngineDevice, handle, fShadowEpsilon );
			}

			// Shadow cascades (EngineSceneShadowsSetCascades); only shaders that declare it use it
			handle = mpNormalMapRenderPSConstantTable->GetConstantByName(NULL, "gShadowCascadeInfo");
			if ( handle )
			{
			float		afCascadeInfo[4];
				EngineShadowMapGetCascadeInfo( afCascadeInfo );
				mpNormalMapRenderPSConstantTable->SetFloatArray( mpEngineDevice, handle, afCascadeInfo, 4 );
			}
		
			handle = mpNormalMapRenderPSConstantTable->GetConstantByName(NULL, "gHasNormalMap");
			if ( handle )
			{
			float		fNormalMap = 0.0f;
				if ( bHasNormalMap )
				{
					fNormalMap = 1.0f;
				}
				mpNormalMapRenderPSConstantTable->SetFloat( mpEngineDevice, handle, fNormalMap );
			}

			handle = mpNormalMapRenderPSConstantTable->GetConstantByName(NULL, "gHasSpecularMap");
			if ( handle )
			{
			float		fSpecularMap = 0.0f;
				if ( bHasSpecularMap )
				{
					fSpecularMap = 1.0f;
				}
				mpNormalMapRenderPSConstantTable->SetFloat( mpEngineDevice, handle, fSpecularMap );
			}

			handle = mpNormalMapRenderPSConstantTable->GetConstantByName(NULL, "gHasDiffuseMap");
			if ( handle )
			{
			float		fDiffuseMap = 0.0f;
				
				if ( bHasDiffuseMap )
				{
					fDiffuseMap = 1.0f;
				}
				mpNormalMapRenderPSConstantTable->SetFloat( mpEngineDevice, handle, fDiffuseMap );
			}
			
			handle = mpNormalMapRenderPSConstantTable->GetConstantByName(NULL, "gAmbientColor");
			if ( handle )
			{
			float	afColourRGBA[4];

				afColourRGBA[0] = xPrimaryLight.Ambient.r;
				afColourRGBA[1] = xPrimaryLight.Ambient.g;
				afColourRGBA[2] = xPrimaryLight.Ambient.b;
				afColourRGBA[3] = 1.0f;
				mpNormalMapRenderPSConstantTable->SetFloatArray( mpEngineDevice, handle, afColourRGBA, 4 );
			}

			handle = mpNormalMapRenderPSConstantTable->GetConstantByName(NULL, "gDiffuseColor");
			if ( handle )
			{
			float	afColourRGBA[4];

				afColourRGBA[0] = xPrimaryLight.Diffuse.r;
				afColourRGBA[1] = xPrimaryLight.Diffuse.g;
				afColourRGBA[2] = xPrimaryLight.Diffuse.b;
				afColourRGBA[3] = 1.0f;
				mpNormalMapRenderPSConstantTable->SetFloatArray( mpEngineDevice, handle, afColourRGBA, 4 );
			}

			handle = mpNormalMapRenderPSConstantTable->GetConstantByName(NULL, "gSpecularColor");
			if ( handle )
			{
			float	afColourRGBA[4];

				afColourRGBA[0] = xPrimaryLight.Specular.r;
				afColourRGBA[1] = xPrimaryLight.Specular.g;
				afColourRGBA[2] = xPrimaryLight.Specular.b;
				afColourRGBA[3] = 1.0f;
				mpNormalMapRenderPSConstantTable->SetFloatArray( mpEngineDevice, handle, afColourRGBA, 4 );
			}

			handle = mpNormalMapRenderPSConstantTable->GetConstantByName(NULL, "gDiffuseIntensity");
			if ( handle )
			{
			float		fDiffuseIntensity = 1.0f;
				mpNormalMapRenderPSConstantTable->SetFloat( mpEngineDevice, handle, fDiffuseIntensity );
			}
		}

		EngineShadersStandardVertexDeclaration(0);

		mpEngineDevice->SetVertexShader(mpNormalMapRenderVertexShader);
		EngineSetPixelShader(mpNormalMapRenderPixelShader, "ATMMaterial");
	}
#endif // tud9
}

ModelMaterialData::ModelMaterialData()
{
int		loop;

	for ( loop = 0; loop < MAX_NUM_TEX_CHANNELS; loop++ )
	{
		m_aTexChannels[loop].mszFilename = NULL;
		m_aTexChannels[loop].mhTexture = NOTFOUND;
		m_aTexChannels[loop].m_nSizeOfSourceData = 0;
		m_aTexChannels[loop].m_pSourceData = NULL;
		m_aTexChannels[loop].m_bAllowOverrideTexture = true;
		m_aTexChannels[loop].m_bEmbeddedTexture = false;
	}

	m_szMaterialName = NULL;

	m_BlendType = NONE;
	m_bDoubleSided = false;
	m_bMaterialPropertiesActive = false;
	m_bMaterialPropertiesNightOnly = false;
	m_nChannel = 0;
	m_xChannelEmissive.fRed = m_xChannelEmissive.fGreen = m_xChannelEmissive.fBlue = m_xChannelEmissive.fAlpha = 0.0f;
	m_hReliefTexture = NOTFOUND;
	m_fReliefDepth = 0.0f;
	m_nReliefTextureWidth = 0;
	m_nReliefTextureHeight = 0;
	m_pReliefSource = NULL;
	m_nReliefSourceSize = 0;
	m_nCloneTextureMaterial = NOTFOUND;

	for ( loop = 0; loop < NUM_COLOUR_CHANNELS; loop++ )
	{
		m_aColours[loop].fRed = m_aColours[loop].fGreen = m_aColours[loop].fBlue = m_aColours[loop].fAlpha = 0.0f;
	}

	m_aColours[DIFFUSE].fRed = m_aColours[DIFFUSE].fGreen = m_aColours[DIFFUSE].fBlue = m_aColours[DIFFUSE].fAlpha = 1.0f;
	m_aColours[AMBIENT].fRed = m_aColours[AMBIENT].fGreen = m_aColours[AMBIENT].fBlue = m_aColours[AMBIENT].fAlpha = 1.0f;
	m_aColours[SPECULAR].fAlpha = 1.0f;
	m_aColours[EMISSIVE].fAlpha = 1.0f;
	m_specularPower = 0.0f;
	m_pNext = NULL;
}

ModelMaterialData::~ModelMaterialData()
{
int		loop;

	for ( loop = 0; loop < MAX_NUM_TEX_CHANNELS; loop++ )
	{
		if ( m_aTexChannels[loop].mszFilename )
		{
			SystemFree( m_aTexChannels[loop].mszFilename );
			m_aTexChannels[loop].mszFilename = NULL;
		}

		if ( m_aTexChannels[loop].mhTexture != NOTFOUND )
		{
			EngineReleaseTexture( &m_aTexChannels[loop].mhTexture );
		}

		if ( m_aTexChannels[loop].m_pSourceData != NULL )
		{
			SystemFree( m_aTexChannels[loop].m_pSourceData );
			m_aTexChannels[loop].m_pSourceData = NULL;
		}
	}

	if ( m_hReliefTexture != NOTFOUND )
	{
		EngineReleaseTexture( &m_hReliefTexture );
	}
	if ( m_pReliefSource != NULL )
	{
		SystemFree( m_pReliefSource );
		m_pReliefSource = NULL;
	}
}

void	ModelMaterialData::LoadReliefFromMem( byte* pbMem, int nMemSize, float fDepth, int nTextureWidth, int nTextureHeight )
{
	if ( m_hReliefTexture != NOTFOUND )
	{
		EngineReleaseTexture( &m_hReliefTexture );
	}
	m_hReliefTexture = EngineLoadTextureFromFileInMem( pbMem, 0, NULL, nMemSize, NULL );
	m_fReliefDepth = fDepth;
	m_nReliefTextureWidth = nTextureWidth;
	m_nReliefTextureHeight = nTextureHeight;
}

void	ModelMaterialData::SetReliefSource( byte* pbMem, int nMemSize )
{
	if ( m_pReliefSource != NULL )
	{
		SystemFree( m_pReliefSource );
	}
	m_pReliefSource = (byte*)( SystemMalloc( nMemSize ) );
	m_nReliefSourceSize = nMemSize;
	memcpy( m_pReliefSource, pbMem, nMemSize );
}

//----------------------------------------------------------------------------------------------------
// Relief on model materials (opt-in, EngineSetModelRelief). It needs the app's ATMMaterial pixel shader to have the
// relief path: The Universal's does. It shares gLandRelief and sampler s6 with that shader's landscape relief, with
// gLandRelief.w = 2 saying it's a model, gLandReliefEdge = fade start, fade end, then texel sizes.

static BOOL		msbModelReliefEnabled = FALSE;
static float	msfModelReliefDepthScale = 1.0f;
static float	msfModelReliefMaxSteps = 16.0f;
static float	msfModelReliefNormalStrength = 1.0f;
static float	msfModelReliefFadeStart = 40.0f;
static float	msfModelReliefFadeEnd = 90.0f;

void	EngineSetModelRelief( BOOL bEnable, float fDepthScale, float fMaxSteps, float fNormalStrength, float fFadeStart, float fFadeEnd )
{
	msbModelReliefEnabled = bEnable;
	msfModelReliefDepthScale = fDepthScale;
	msfModelReliefMaxSteps = fMaxSteps;
	msfModelReliefNormalStrength = fNormalStrength;
	msfModelReliefFadeStart = fFadeStart;
	msfModelReliefFadeEnd = fFadeEnd;
}

//----------------------------------------------------------------------------------------------------
// State channels (ATMFormat.md, chunk 0x20). A material tagged with a channel gets its lit colour while the app has set
// that channel's strength above 0. The strengths belong to the draw, not the model, so clones sharing these materials can
// differ: the app sets them before drawing a model and clears them after. All 0 unless an app sets them.

static float	msafModelMaterialChannels[MODEL_MATERIAL_NUM_CHANNELS] = { 0.0f };
static BOOL		msabModelMaterialChannelHasColour[MODEL_MATERIAL_NUM_CHANNELS] = { FALSE };
static MATERIAL_COLOUR	msaxModelMaterialChannelColours[MODEL_MATERIAL_NUM_CHANNELS];

void	EngineSetModelMaterialChannel( int nChannel, float fStrength )
{
	if ( ( nChannel >= 1 ) && ( nChannel <= MODEL_MATERIAL_NUM_CHANNELS ) )
	{
		msafModelMaterialChannels[nChannel - 1] = fStrength;
		msabModelMaterialChannelHasColour[nChannel - 1] = FALSE;
	}
}

void	EngineSetModelMaterialChannelColour( int nChannel, float fStrength, float fRed, float fGreen, float fBlue )
{
	if ( ( nChannel >= 1 ) && ( nChannel <= MODEL_MATERIAL_NUM_CHANNELS ) )
	{
		msafModelMaterialChannels[nChannel - 1] = fStrength;
		msabModelMaterialChannelHasColour[nChannel - 1] = TRUE;
		msaxModelMaterialChannelColours[nChannel - 1].fRed = fRed;
		msaxModelMaterialChannelColours[nChannel - 1].fGreen = fGreen;
		msaxModelMaterialChannelColours[nChannel - 1].fBlue = fBlue;
		msaxModelMaterialChannelColours[nChannel - 1].fAlpha = 1.0f;
	}
}

void	EngineClearModelMaterialChannels( void )
{
int		nLoop;

	for ( nLoop = 0; nLoop < MODEL_MATERIAL_NUM_CHANNELS; nLoop++ )
	{
		msafModelMaterialChannels[nLoop] = 0.0f;
		msabModelMaterialChannelHasColour[nLoop] = FALSE;
	}
}

void	ModelMaterialData::SetChannel( int nChannel, float fRed, float fGreen, float fBlue )
{
	if ( ( nChannel >= 1 ) && ( nChannel <= MODEL_MATERIAL_NUM_CHANNELS ) )
	{
		m_nChannel = nChannel;
		m_xChannelEmissive.fRed = fRed;
		m_xChannelEmissive.fGreen = fGreen;
		m_xChannelEmissive.fBlue = fBlue;
		m_xChannelEmissive.fAlpha = 1.0f;
	}
}

/***************************************************************************
 * Function    : ModelGetMaterialChannelLamps
 * Params      : pxPositions, pxNormals - nMax of each, filled in with the middle and the facing of each lamp, in model space,
 *				 after any scale or rotation the model was loaded with. A front face has cross( b - a, c - a ) along its normal
 * Returns     : How many lamps. A lamp is a set of the channel's faces touching at a corner
 * Description : Reads the mesh, so call it once per model and keep the answer, not per frame
 ***************************************************************************/
int		ModelGetMaterialChannelLamps( int nModelHandle, int nChannel, VECT* pxPositions, VECT* pxNormals, int nMax )
{
#define		MAX_LAMP_FACES		64
MODEL_RENDER_DATA*		pxModelData;
ModelMaterialData*		pMaterial;
const ENGINEMESH_COLLISION_TRIS*	pxTris;
DWORD*	pulAttributes = NULL;
int		anFaces[MAX_LAMP_FACES];
int		anGroup[MAX_LAMP_FACES];
int		nNumFaces = 0;
int		nNumLamps = 0;
int		nFace;
int		nOther;
int		nCorner;
int		nOtherCorner;
int		nLoop;
BOOL	bAnyMaterial = FALSE;
BOOL	abHasChannel[256];

	if ( nModelHandle == NOTFOUND ) return( 0 );
	pxModelData = &maxModelRenderData[ nModelHandle ];
	if ( pxModelData->pxBaseMesh == NULL ) return( 0 );

	memset( abHasChannel, 0, sizeof( abHasChannel ) );
	pMaterial = pxModelData->pMaterialData;
	while ( pMaterial )
	{
		if ( ( pMaterial->GetChannel() == nChannel ) &&
			 ( pMaterial->GetAttrib() >= 0 ) &&
			 ( pMaterial->GetAttrib() < 256 ) )
		{
			abHasChannel[ pMaterial->GetAttrib() ] = TRUE;
			bAnyMaterial = TRUE;
		}
		pMaterial = pMaterial->GetNext();
	}
	if ( bAnyMaterial == FALSE ) return( 0 );

	pxTris = pxModelData->pxBaseMesh->GetCollisionTris( pxModelData->xStats.nNumIndices );
	if ( pxTris == NULL ) return( 0 );

	pxModelData->pxBaseMesh->LockAttributeBuffer( kLock_ReadOnly, (BYTE**)&pulAttributes );
	if ( pulAttributes == NULL ) return( 0 );
	for ( nFace = 0; ( nFace < pxTris->nNumTris ) && ( nNumFaces < MAX_LAMP_FACES ); nFace++ )
	{
		if ( ( pulAttributes[nFace] < 256 ) &&
			 ( abHasChannel[ pulAttributes[nFace] ] ) )
		{
			anFaces[nNumFaces] = nFace;
			anGroup[nNumFaces] = nNumFaces;
			nNumFaces++;
		}
	}
	pxModelData->pxBaseMesh->UnlockAttributeBuffer();

	// Faces with a corner in the same place are the same lamp
	for ( nFace = 0; nFace < nNumFaces; nFace++ )
	{
		for ( nOther = 0; nOther < nFace; nOther++ )
		{
		BOOL	bTouching = FALSE;

			for ( nCorner = 0; nCorner < 3; nCorner++ )
			{
				for ( nOtherCorner = 0; nOtherCorner < 3; nOtherCorner++ )
				{
					if ( VectDist( &pxTris->pxVerts[ (anFaces[nFace]*3) + nCorner ], &pxTris->pxVerts[ (anFaces[nOther]*3) + nOtherCorner ] ) < 0.001f )
					{
						bTouching = TRUE;
					}
				}
			}
			if ( ( bTouching ) && ( anGroup[nFace] != anGroup[nOther] ) )
			{
			int		nOldGroup = anGroup[nFace];

				for ( nLoop = 0; nLoop < nNumFaces; nLoop++ )
				{
					if ( anGroup[nLoop] == nOldGroup ) anGroup[nLoop] = anGroup[nOther];
				}
			}
		}
	}

	for ( nLoop = 0; ( nLoop < nNumFaces ) && ( nNumLamps < nMax ); nLoop++ )
	{
		if ( anGroup[nLoop] == nLoop )		// (Each group is named after its first face)
		{
		VECT	xSum = { 0.0f, 0.0f, 0.0f };
		VECT	xNormal = { 0.0f, 0.0f, 0.0f };
		int		nCorners = 0;

			for ( nFace = 0; nFace < nNumFaces; nFace++ )
			{
				if ( anGroup[nFace] == nLoop )
				{
				const VECT*		pxCorners = &pxTris->pxVerts[ anFaces[nFace]*3 ];
				VECT	xEdge1;
				VECT	xEdge2;
				VECT	xCross;

					for ( nCorner = 0; nCorner < 3; nCorner++ )
					{
						xSum.x += pxCorners[nCorner].x;
						xSum.y += pxCorners[nCorner].y;
						xSum.z += pxCorners[nCorner].z;
						nCorners++;
					}
					VectSub( &xEdge1, &pxCorners[1], &pxCorners[0] );
					VectSub( &xEdge2, &pxCorners[2], &pxCorners[0] );
					VectCross( &xCross, &xEdge1, &xEdge2 );
					xNormal.x += xCross.x;
					xNormal.y += xCross.y;
					xNormal.z += xCross.z;
				}
			}
			VectNormalize( &xNormal );
			pxPositions[nNumLamps].x = xSum.x / nCorners;
			pxPositions[nNumLamps].y = xSum.y / nCorners;
			pxPositions[nNumLamps].z = xSum.z / nCorners;
			pxNormals[nNumLamps] = xNormal;
			nNumLamps++;
		}
	}
	return( nNumLamps );
}

void	ModelMaterialsEndRelief( void )
{
#ifdef TUD9
static const float	safOff[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	ModelMaterialsShaderSetFloat4( "gLandRelief", safOff );
	EngineSetTexture( 6, NOTFOUND );
#endif
}

int		ModelMaterialData::Apply( void )
{
int	ret = NO_CHANGES;

	if ( ( m_aTexChannels[DIFFUSEMAP].mhTexture != NOTFOUND ) ||
		 ( m_aTexChannels[DIFFUSEMAP].m_bAllowOverrideTexture == FALSE ) )
	{
		EngineSetTexture( 0, m_aTexChannels[DIFFUSEMAP].mhTexture );
	}

#ifdef TUD9

	// If this is the shadowmap first pass, we only care about the main diffuse texture
	if ( ModelRenderingIsShadowPass() == TRUE )
	{
		return ret;
	}

	// Relief, only when this draw is using the shader that has it (the reflection pass's statics are fixed function)
	if ( ( m_hReliefTexture != NOTFOUND ) &&
		 ( msbModelReliefEnabled ) &&
		 ( m_fReliefDepth > 0.0f ) &&
		 ( m_nReliefTextureWidth > 0 ) &&
		 ( m_nReliefTextureHeight > 0 ) &&
		 ( mpNormalMapRenderPixelShader != NULL ) )
	{
	LPGRAPHICSPIXELSHADER	pCurrentShader = NULL;

		mpEngineDevice->GetPixelShader( &pCurrentShader );
		if ( pCurrentShader == mpNormalMapRenderPixelShader )
		{
		float	afRelief[4];
		float	afReliefEdge[4];

			afRelief[0] = m_fReliefDepth * msfModelReliefDepthScale;
			afRelief[1] = msfModelReliefMaxSteps;
			afRelief[2] = msfModelReliefNormalStrength;
			afRelief[3] = 2.0f;
			afReliefEdge[0] = msfModelReliefFadeStart;
			afReliefEdge[1] = msfModelReliefFadeEnd;
			afReliefEdge[2] = 1.0f / (float)m_nReliefTextureWidth;
			afReliefEdge[3] = 1.0f / (float)m_nReliefTextureHeight;
			EngineSetTexture( 6, m_hReliefTexture );
			mpEngineDevice->SetSamplerState( 6, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP );
			mpEngineDevice->SetSamplerState( 6, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP );
			mpEngineDevice->SetSamplerState( 6, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
			mpEngineDevice->SetSamplerState( 6, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
			mpEngineDevice->SetSamplerState( 6, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );
			ModelMaterialsShaderSetFloat4( "gLandRelief", afRelief );
			ModelMaterialsShaderSetFloat4( "gLandReliefEdge", afReliefEdge );
			ret |= RELIEF_ACTIVATED;
		}
		if ( pCurrentShader != NULL )
		{
			pCurrentShader->Release();
		}
	}

	// Probably not needed?
//	EngineSetShadowMultitexture( TRUE );

	//  Set normal map texture if present and activate shader system
	if ( m_aTexChannels[NORMALMAP].mhTexture != NOTFOUND )
	{
		EngineSetTexture( 2, m_aTexChannels[NORMALMAP].mhTexture );
		if ( m_aTexChannels[SPECULARMAP].mhTexture != NOTFOUND )
		{
			mpEngineDevice->SetTextureStageState( 2, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
			mpEngineDevice->SetTextureStageState( 2, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			mpEngineDevice->SetTextureStageState( 2, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
			mpEngineDevice->SetTextureStageState( 2, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );

			EngineSetTexture( 3, m_aTexChannels[SPECULARMAP].mhTexture );
			ModelMaterialsActivateNormalShader(TRUE, TRUE, TRUE);
		}
		else
		{
			ModelMaterialsActivateNormalShader(TRUE, TRUE, FALSE);
		}
		ret |= NORMALMAP_ACTIVATED;
		EngineEnableSpecular(TRUE);
		ret |= SPECULAR_ACTIVATED;		
	}
	else if ( m_aTexChannels[SPECULARMAP].mhTexture != NOTFOUND )
	{
		mpEngineDevice->SetTextureStageState( 2, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
		mpEngineDevice->SetTextureStageState( 2, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		mpEngineDevice->SetTextureStageState( 2, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
		mpEngineDevice->SetTextureStageState( 2, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );

		EngineSetTexture( 3, m_aTexChannels[SPECULARMAP].mhTexture );
		ModelMaterialsActivateNormalShader( TRUE, FALSE, TRUE );
		EngineEnableSpecular(TRUE);
//		ret |= NORMALMAP_ACTIVATED;
		ret |= SPECULAR_ACTIVATED;		
	}
	else
	{
//		EngineSceneShadowsStartRender( TRUE, FALSE, FALSE );
	}
#endif // tud9

	if ( m_bDoubleSided )
	{
		EngineEnableCulling( 0 );
		ret |= DOUBLESIDED_ACTIVATED;		
	}
	else
	{
		EngineEnableCulling( 1 );
	}
	
	float	fChannelStrength = ( m_nChannel > 0 ) ? msafModelMaterialChannels[m_nChannel - 1] : 0.0f;

	if ( ( fChannelStrength > 0.0f ) ||
		 ( ( m_bMaterialPropertiesActive ) &&
		   ( ( m_bMaterialPropertiesNightOnly == false ) || ( EngineGetGameInterface()->IsDaytime() == false ) ) ) )
	{
		if ( m_BlendType != NONE )
		{
	#ifdef USING_OPENGL
			// TODO 
	#endif

	#ifdef TUD9
			switch( m_BlendType )
			{
			case ALPHABLEND_NOCUTOFF:
				EngineEnableBlend( TRUE );
				mpEngineDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
				mpEngineDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
				mpEngineDevice->SetRenderState( D3DRS_ALPHAREF, 1 );
				break;
			case ALPHABLEND:
				EngineEnableBlend( TRUE );
				mpEngineDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
				mpEngineDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
				mpEngineDevice->SetRenderState( D3DRS_ALPHAREF, 0x80 );
	//			EngineEnableZWrite( FALSE );
				break;
			case ADDITIVE_ALPHA:
				EngineEnableBlend( TRUE );
				mpEngineDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
				mpEngineDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
	//			EngineEnableZWrite( FALSE );
				break;
			case SUBTRACTIVE_ALPHA:
				EngineEnableBlend( TRUE );
				mpEngineDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
				mpEngineDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
				break;
			case COLOURBLEND:
				EngineEnableBlend( TRUE );
				mpEngineDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCCOLOR);
				mpEngineDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);
				break;
			case ADDITIVE_COLOUR:
				EngineEnableBlend( TRUE );
				mpEngineDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCCOLOR);
				mpEngineDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
	//			EngineEnableZTest( FALSE );
				break;
			case SUBTRACTIVE_COLOUR:
				EngineEnableBlend( TRUE );
				mpEngineDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
				mpEngineDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);
				break;
			}
	#endif
			ret |= BLEND_RENDER_STATES_CHANGED;
		}
		else
		{
			if ( EngineGetMaterialBlendOverride() == FALSE )
			{
				EngineEnableBlend( FALSE );
			}
		}
#ifdef USING_OPENGL
		// TODO 
#endif

#ifdef TUD9
	ENGINEMATERIAL xDXMaterial;
		// Set the RGBA for diffuse reflection.
		xDXMaterial.Diffuse.r = m_aColours[DIFFUSE].fRed;
		xDXMaterial.Diffuse.g = m_aColours[DIFFUSE].fGreen;
		xDXMaterial.Diffuse.b = m_aColours[DIFFUSE].fBlue;
		xDXMaterial.Diffuse.a = m_aColours[DIFFUSE].fAlpha;
		
		// Set the RGBA for ambient reflection.
		xDXMaterial.Ambient.r = m_aColours[AMBIENT].fRed;
		xDXMaterial.Ambient.g = m_aColours[AMBIENT].fGreen;
		xDXMaterial.Ambient.b = m_aColours[AMBIENT].fBlue;
		xDXMaterial.Ambient.a = m_aColours[AMBIENT].fAlpha;
		
		// Set the color and sharpness of specular highlights.
		xDXMaterial.Specular.r = m_aColours[SPECULAR].fRed;
		xDXMaterial.Specular.g = m_aColours[SPECULAR].fGreen;
		xDXMaterial.Specular.b = m_aColours[SPECULAR].fBlue;
		xDXMaterial.Specular.a = m_aColours[SPECULAR].fAlpha;
		if ( ( xDXMaterial.Specular.r == 0.0f ) &&
			 ( xDXMaterial.Specular.g == 0.0f ) &&
			 ( xDXMaterial.Specular.b == 0.0f ) )
		{
			xDXMaterial.Power = 0.0f;
		}
		else
		{
			xDXMaterial.Power = m_specularPower;
		}
		
		// Set the RGBA for emissive color.
		xDXMaterial.Emissive.r = m_aColours[EMISSIVE].fRed;
		xDXMaterial.Emissive.g = m_aColours[EMISSIVE].fGreen;
		xDXMaterial.Emissive.b = m_aColours[EMISSIVE].fBlue;
		xDXMaterial.Emissive.a = m_aColours[EMISSIVE].fAlpha;

		if ( fChannelStrength > 0.0f )
		{
			const MATERIAL_COLOUR*	pxLit = msabModelMaterialChannelHasColour[m_nChannel - 1] ? &msaxModelMaterialChannelColours[m_nChannel - 1] : &m_xChannelEmissive;

			xDXMaterial.Emissive.r = pxLit->fRed * fChannelStrength;
			xDXMaterial.Emissive.g = pxLit->fGreen * fChannelStrength;
			xDXMaterial.Emissive.b = pxLit->fBlue * fChannelStrength;
			xDXMaterial.Emissive.a = 1.0f;
		}

		EngineSetMaterial(&xDXMaterial);
		ret |= MATERIAL_CHANGED;

		if ( ( xDXMaterial.Specular.r > 0.0f ) ||
			 ( xDXMaterial.Specular.g > 0.0f ) ||
			 ( xDXMaterial.Specular.b > 0.0f ) )
		{
			EngineEnableSpecular(TRUE);
			ret |= SPECULAR_ACTIVATED;		
		}

		mpEngineDevice->SetRenderState(	D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL );
		mpEngineDevice->SetRenderState(	D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL );
#endif

	}
	return( ret );
}

byte*	ModelMaterialData::GetSourceData( int texnum, int* pnSize )
{
	*pnSize = m_aTexChannels[texnum].m_nSizeOfSourceData;
	return( m_aTexChannels[texnum].m_pSourceData );
}

void	ModelMaterialData::SetSourceData(int texnum, byte* pbData, int nDataSize )
{
	m_aTexChannels[texnum].m_pSourceData = (byte*)( SystemMalloc( nDataSize ) );
	m_aTexChannels[texnum].m_nSizeOfSourceData = nDataSize;
	memcpy( m_aTexChannels[texnum].m_pSourceData, pbData, nDataSize );
}


void	ModelMaterialData::SetFilename( int texnum, const char* szFilename )
{
	SAFE_FREE( m_aTexChannels[texnum].mszFilename );
	m_aTexChannels[texnum].mszFilename = (char*)( SystemMalloc( strlen(szFilename) + 1 ) );
	strcpy( m_aTexChannels[texnum].mszFilename, szFilename );
}

void	ModelMaterialData::LoadTextureFilename( int texnum, const char* szFilename )
{
	// if not a clone
	if ( m_nCloneTextureMaterial == NOTFOUND )
	{
		if ( m_aTexChannels[texnum].mhTexture != NOTFOUND )
		{
			EngineReleaseTexture( &m_aTexChannels[texnum].mhTexture );
		}
	}
	else		// If was a clone, clear the clone status
	{
		m_nCloneTextureMaterial = NOTFOUND;
	}

	if ( szFilename )
	{
		m_aTexChannels[texnum].mhTexture = EngineLoadTexture( szFilename, 0 );
		m_aTexChannels[texnum].mszFilename = (char*)( SystemMalloc( strlen(szFilename) + 1 ) );
		strcpy( m_aTexChannels[texnum].mszFilename, szFilename );
	}
}

void	ModelMaterialData::RemoveTexture( int texnum )
{
	if ( m_nCloneTextureMaterial == NOTFOUND )
	{		
		if ( m_aTexChannels[texnum].mhTexture != NOTFOUND )
		{
			EngineReleaseTexture( &m_aTexChannels[texnum].mhTexture );
		}
	}
	else		// If was a clone, clear the clone status
	{
		m_nCloneTextureMaterial = NOTFOUND;
	}
	SystemFree( m_aTexChannels[texnum].mszFilename );
	m_aTexChannels[texnum].mszFilename = NULL;
	SetIsEmbeddedTexture( texnum, FALSE );
}

void	ModelMaterialData::LoadTextureFromMem( int texnum, byte* pbMem, int nMemSize, const char* szOriginalFilename )
{
	// if not a clone
	if ( m_nCloneTextureMaterial == NOTFOUND )
	{
		if ( m_aTexChannels[texnum].mhTexture != NOTFOUND )
		{
			EngineReleaseTexture( &m_aTexChannels[texnum].mhTexture );
		}
	}
	else		// If was a clone, clear the clone status
	{
		m_nCloneTextureMaterial = NOTFOUND;
	}

	m_aTexChannels[texnum].mhTexture = EngineLoadTextureFromFileInMem( pbMem, 0, NULL, nMemSize, szOriginalFilename );
	if ( szOriginalFilename )
	{
		m_aTexChannels[texnum].mszFilename = (char*)( SystemMalloc( strlen(szOriginalFilename) + 1 ) );
		strcpy( m_aTexChannels[texnum].mszFilename, szOriginalFilename );
	}
}

void	ModelMaterialData::SetMaterialName( const char* szMaterialName )
{
	SAFE_FREE( m_szMaterialName );

	if ( szMaterialName )
	{
		m_szMaterialName = (char*)( SystemMalloc( strlen(szMaterialName) + 1 ) );
		strcpy( m_szMaterialName, szMaterialName );
	}
}

void	ModelMaterialData::CloneTextureFromMaterial( int texnum, ModelMaterialData* pSource )
{
	// If not already a clone
	if ( m_nCloneTextureMaterial == NOTFOUND )
	{
		if ( m_aTexChannels[texnum].mhTexture != NOTFOUND )
		{
			EngineReleaseTexture( &m_aTexChannels[texnum].mhTexture );
		}
	}

	if ( pSource )
	{
		m_aTexChannels[texnum].mhTexture = pSource->m_aTexChannels[texnum].mhTexture;
		m_nCloneTextureMaterial = pSource->m_nAttrib;
	}
	else
	{
		m_aTexChannels[texnum].mhTexture = NOTFOUND;
		m_nCloneTextureMaterial = NOTFOUND;
	}

}


MATERIAL_COLOUR		ModelMaterialData::GetColour( eCOLOUR_CHANNELS channel )
{
	return( m_aColours[ channel ] );
}

void	ModelMaterialData::SetColour( eCOLOUR_CHANNELS channel, const MATERIAL_COLOUR* pCol )
{
	m_aColours[ channel ] = *pCol;
}


ModelMaterialData*		FindMaterial( MODEL_RENDER_DATA* pxModelData, int nAttrib )
{
ModelMaterialData*		pMaterial = pxModelData->pMaterialData;

	if ( pMaterial )
	{
		if ( ( pMaterial->GetNext() == NULL ) &&
			 ( pMaterial->HasActiveMaterialProperties() == FALSE ) &&
			 ( pMaterial->HasRelief() == false ) )
		{
			return( NULL );
		}

		while ( pMaterial )
		{
			if ( pMaterial->GetAttrib() == nAttrib )
			{
				return( pMaterial );
			}
			pMaterial = pMaterial->GetNext();
		}
	}
	return( NULL );
}

ModelMaterialData*		FindMaterialByMaterialName( MODEL_RENDER_DATA* pxModelData, const char* szMaterialName )
{
ModelMaterialData*		pMaterial;

	pMaterial = pxModelData->pMaterialData;

	while ( pMaterial )
	{
		if ( stricmp( pMaterial->GetMaterialName(), szMaterialName ) == 0 )
		{
			return( pMaterial );
		}
		pMaterial = pMaterial->GetNext();
	}
	return( NULL );
}

ModelMaterialData*		FindMaterialFromHandle( int nModelHandle, int nAttrib )
{
MODEL_RENDER_DATA*		pxModelData;

	if ( nModelHandle == NOTFOUND ) return( NULL );
	pxModelData = &maxModelRenderData[ nModelHandle ];

	return( FindMaterial( pxModelData, nAttrib ) );
}

void	ModelDeleteAllMaterials( int nModelHandle )
{
MODEL_RENDER_DATA* pxModelData;
ModelMaterialData*	pMaterialData = NULL;
ModelMaterialData*	pNext = NULL;
DWORD*			pxAttributes;
int				nIndexLoop;
int				nNumFaces;

	if ( nModelHandle == NOTFOUND ) return;
	pxModelData = &maxModelRenderData[ nModelHandle ];

	pMaterialData = pxModelData->pMaterialData;
	
	while ( pMaterialData )
	{
		pNext = pMaterialData->GetNext();

		delete pMaterialData;

		pMaterialData = pNext;
	}
	pxModelData->pMaterialData = NULL;
	pxModelData->xStats.nNumMaterials = 0;

	pxModelData->pxBaseMesh->LockAttributeBuffer( kLock_ReadOnly, (BYTE**)( &pxAttributes ) );
	nNumFaces = pxModelData->xStats.nNumIndices / 3;

	for( nIndexLoop = 0; nIndexLoop < nNumFaces; nIndexLoop++ )
	{
		*pxAttributes++ = 0;
	}
	pxModelData->pxBaseMesh->UnlockAttributeBuffer();

}

//-------------------------------------
ModelMaterialData*	ModelCreateDefaultMaterialsData( MODEL_RENDER_DATA* pxModelData, const char* acTexture )
{
	if ( !pxModelData->pMaterialData )
	{
	ModelMaterialData*	pMaterialData;
	MATERIAL_COLOUR		xCol;

		pMaterialData = new ModelMaterialData;

		pMaterialData->SetAttrib( 0 );
		xCol.fAlpha = 1.0f;
		xCol.fRed = 1.0f;
		xCol.fGreen = 1.0f;
		xCol.fBlue = 1.0f;
		pMaterialData->SetColour( ModelMaterialData::DIFFUSE, &xCol );
		pMaterialData->SetColour( ModelMaterialData::AMBIENT, &xCol );
		xCol.fRed = 0.5f;
		xCol.fGreen = 0.5f;
		xCol.fBlue = 0.5f;
		pMaterialData->SetColour( ModelMaterialData::SPECULAR, &xCol );
		xCol.fRed = 0.0f;
		xCol.fGreen = 0.0f;
		xCol.fBlue = 0.0f;
		pMaterialData->SetColour( ModelMaterialData::EMISSIVE, &xCol );

		pMaterialData->LoadTextureFilename( 0, acTexture );

		// Add to the modeldata linked list of materials
		pMaterialData->SetNext(pxModelData->pMaterialData);
		pxModelData->pMaterialData = pMaterialData;

		pxModelData->xStats.nNumMaterials = 1;
		return( pMaterialData );
	}
	return( NULL );

}

void	ModelCreateDefaultMaterials( int nModelHandle, const char* acTexture )
{
MODEL_RENDER_DATA* pxModelData;

	if ( nModelHandle == NOTFOUND ) return;
	pxModelData = &maxModelRenderData[ nModelHandle ];

	ModelCreateDefaultMaterialsData(pxModelData, acTexture );

}

ModelMaterialData*	ModelAddNewMaterialData( MODEL_RENDER_DATA* pxModelData, const char* acTexture )
{
	if ( !pxModelData->pMaterialData )
	{
		return( ModelCreateDefaultMaterialsData( pxModelData, acTexture ) );
	}
	else
	{
	ModelMaterialData*	pMaterialData;
	int		nMax = 0;

		// Find the highest materialID currently in use
		pMaterialData = pxModelData->pMaterialData;
		while ( pMaterialData )
		{
			if ( pMaterialData->GetAttrib() > nMax )
			{
				nMax = pMaterialData->GetAttrib();
			}
			pMaterialData = pMaterialData->GetNext();
		}

		pMaterialData = new ModelMaterialData;

		pMaterialData->SetAttrib( nMax + 1 );
		pMaterialData->LoadTextureFilename( 0, acTexture );

		// Add to the modeldata linked list of materials
		pMaterialData->SetNext(pxModelData->pMaterialData);
		pxModelData->pMaterialData = pMaterialData;

		pxModelData->xStats.nNumMaterials++;
		return( pMaterialData );
	}
}

void	ModelAddNewMaterial( int nModelHandle, const char* acTexture )
{
MODEL_RENDER_DATA* pxModelData;

	if ( nModelHandle == NOTFOUND ) return;
	pxModelData = &maxModelRenderData[ nModelHandle ];

	ModelAddNewMaterialData( pxModelData, acTexture );
}


BOOL	ModelMaterialOptimiseCompareTextureData( byte* pbThisData, int nThisSize, byte* pbSourceData, int nSourceSize )
{
int		nLoop;

	if ( nThisSize != nSourceSize ) return( FALSE );

	for ( nLoop = 0; nLoop < nThisSize; nLoop++ )
	{
		if ( *pbThisData != *pbSourceData )
		{
			return( FALSE );
		}
		pbThisData++;
		pbSourceData++;
	}
	return( true );
}



void	ModelMaterialCountEmbeddedTextures( MODEL_RENDER_DATA* pModel )
{
int		nCount = 0;
int		nNumMaterials = pModel->xStats.nNumMaterials;
ModelMaterialData*			pThisMaterial = NULL;
int		nLoop;
int		texchanloop;

	// Loop through all the materials starting at the second
	for ( nLoop = 0; nLoop < nNumMaterials; nLoop++ )
	{
		pThisMaterial = FindMaterial( pModel, nLoop );
		if ( pThisMaterial )
		{
			for ( texchanloop = 0; texchanloop < MAX_NUM_TEX_CHANNELS; texchanloop++ )
			{
				if ( pThisMaterial->IsEmbeddedTexture(texchanloop) )
				{	
					nCount++;
				}
			}
		}
	}

	pModel->xStats.nNumEmbeddedTextures = nCount;
}


void	ModelMaterialOptimiseTextureClones( MODEL_RENDER_DATA* pModel )
{
int		nNumMaterials = pModel->xStats.nNumMaterials;
ModelMaterialData*			pThisMaterial = NULL;
ModelMaterialData*			pSourceMaterial = NULL;
byte*		pbThisMaterialData = NULL;
int			nThisMaterialDataSize = 0;
byte*		pbSourceMaterialData = NULL;
int			nSourceMaterialDataSize = 0;

	// No point doing anything if just 1 material
	if ( nNumMaterials > 1 )
	{
	int		Loop1;
	int		Loop2;

		// Loop through all the materials starting at the second
		for ( Loop1 = 1; Loop1 < nNumMaterials; Loop1++ )
		{
			pThisMaterial = FindMaterial( pModel, Loop1 );
			if ( pThisMaterial )
			{
				// Ignore if this is already a clone
				if ( pThisMaterial->GetCloneTextureMaterialIndex(0) == NOTFOUND )
				{
					// Get the texture data for the 'this' material
					if ( pThisMaterial->IsEmbeddedTexture(0) )
					{
						pbThisMaterialData = pThisMaterial->GetSourceData( DIFFUSEMAP, &nThisMaterialDataSize );
					}
						
					// Loop through all the earlier materials
					for ( Loop2 = 0; Loop2 < Loop1; Loop2++ )
					{
						pSourceMaterial = FindMaterial( pModel, Loop2 );

						// Ignore if this is already a clone
						if ( ( pSourceMaterial ) &&
							 ( pSourceMaterial->GetCloneTextureMaterialIndex(DIFFUSEMAP) == NOTFOUND ) )
						{
						char	acString[256];

							// Get the texture data for the 'source' material
							if ( pSourceMaterial->IsEmbeddedTexture(DIFFUSEMAP) )
							{
								pbSourceMaterialData = pSourceMaterial->GetSourceData( DIFFUSEMAP, &nSourceMaterialDataSize );

								// Now compare the two..
								if ( ModelMaterialOptimiseCompareTextureData( pbThisMaterialData, nThisMaterialDataSize, pbSourceMaterialData, nSourceMaterialDataSize ) == TRUE )
								{
									// Data matches..
									// If good.. get rid of the 'this' texture data and identify it as a clone of 'source'
									sprintf( acString, "Clone found - Material %d with %d\n", Loop1, Loop2 );
									pThisMaterial->CloneTextureFromMaterial( DIFFUSEMAP, pSourceMaterial );
								}
								else
								{
									sprintf( acString, "No material-texture match found - creating a new one (%d)\n", Loop1 );	
								}
							}
							else
							{
							const char*		szSourceFileName = pSourceMaterial->GetFilename(DIFFUSEMAP );

								if ( ( szSourceFileName ) &&
									 ( szSourceFileName[0] != 0 ) )
								{
								const char*		szThisFileName = pThisMaterial->GetFilename(DIFFUSEMAP);

									if ( ( szThisFileName ) &&
										 ( szThisFileName[0] != 0 ) )
									{
										if ( stricmp( szSourceFileName, szThisFileName ) == 0 )
										{
											sprintf( acString, "Clone found - Material %d with %d\n", Loop1, Loop2 );
											pThisMaterial->CloneTextureFromMaterial( DIFFUSEMAP, pSourceMaterial );
										}
									}
								}


							}
						}
	
					}
				}
			}	
		}
	}

	ModelMaterialCountEmbeddedTextures( pModel );
}
