
#include "EngineDX.h"

#include <StandardDef.h>
#include <Engine.h>
#include <Interface.h>

//#include "../../Universal/GameCode/FrontEnd/Screens/FEVideoOptions.h"		// TEMP - for debug indicators

#include "TextureManagerDX.h"
#include "ShaderLoaderDX.h"
#include "PostProcessDX.h"

LPGRAPHICSPIXELSHADER			mpHighPassPixelShader = NULL;
LPGRAPHICSPIXELSHADER			mpBlurHPixelShader = NULL;
LPGRAPHICSPIXELSHADER			mpBlurVPixelShader = NULL;
LPGRAPHICSPIXELSHADER			mpBlurFullPixelShader = NULL;
LPGRAPHICSVERTEXSHADER			mpBasicVertexShader = NULL;

LPGRAPHICSCONSTANTBUFFER		mpHighPassPSConstantTable = NULL;
LPGRAPHICSCONSTANTBUFFER		mpBlurHPSConstantTable = NULL;
LPGRAPHICSCONSTANTBUFFER		mpBlurVPSConstantTable = NULL;
LPGRAPHICSCONSTANTBUFFER		mpBlurFullPSConstantTable = NULL;
LPGRAPHICSCONSTANTBUFFER		mpBasicVSConstantTable = NULL;


BOOL				mboPostProcessShowDebugBlurPassOutput = FALSE;
BOOL				mboPostProcessEnabled = TRUE;
BOOL				mboBloomPassEnabled = TRUE;
TEXTURE_HANDLE		mhPostProcessRenderTarget = NOTFOUND;
TEXTURE_HANDLE		mhPostProcessSmallRenderTarget = NOTFOUND;
TEXTURE_HANDLE		mhPostProcessSmallBlurRenderTarget = NOTFOUND;
int					mhPostProcessOutputVertexBuffer = NOTFOUND;
int					mhPostProcessSmallQuadVertexBuffer = NOTFOUND;
int					mhPostProcessOutputAlphaVertexBuffer = NOTFOUND;
int					mnBlurQualityMod = 1;
int		mnSmallTargetW = 256;
int		mnSmallTargetH = 256;
float	mfHighpassThreshold = 0.3f;
float	mfFullBlurAmount = 0.0f;
int					mhPostProcessGrabScreenTexture = NOTFOUND;
float	msfPostProcessDefaultBloomValue = 0.5f;

int	EnginePostProcessGenerateQuad( int width, int height, ulong ulCol )
{
int			nVertexBufferHandle;
ENGINEBUFFERVERTEX		vertices[6];
ENGINEBUFFERVERTEX*		pxVertexBuffer = &vertices[0];
ENGINEBUFFERVERTEX*		pxBaseVertex = pxVertexBuffer;
int			nLoop;

	memset( vertices, 0, sizeof(ENGINEBUFFERVERTEX) * 6 );

	/** First tri **/
	pxVertexBuffer->position.x = 0.0f;
	pxVertexBuffer->position.y = 0.0f;
	pxVertexBuffer->position.z = 0.0f;
	pxVertexBuffer->normal.z = 1.0f;
	pxVertexBuffer->tu = 0.0f;
	pxVertexBuffer->tv = 0.0f;
	pxVertexBuffer->color    = ulCol;
	pxVertexBuffer++;
	pxVertexBuffer->position.x = (float)( width ) - 1.0f;
	pxVertexBuffer->position.y = 0.0f;
	pxVertexBuffer->position.z = 0.0f;
	pxVertexBuffer->normal.z = 1.0f;
	pxVertexBuffer->tu = 1.0f;
	pxVertexBuffer->tv = 0.0f;
	pxVertexBuffer->color    = ulCol;
	pxVertexBuffer++;
	pxVertexBuffer->position.x = 0.0f;
	pxVertexBuffer->position.y = (float)( height ) - 1.0f;
	pxVertexBuffer->position.z = 0.0f;
	pxVertexBuffer->normal.z = 1.0f;
	pxVertexBuffer->tu = 0.0f;
	pxVertexBuffer->tv = 1.0f;
	pxVertexBuffer->color    = ulCol;
	pxVertexBuffer++;

	// 2ND TRI
	pxVertexBuffer->position.x = (float)( width ) - 1.0f;
	pxVertexBuffer->position.y = 0.0f;
	pxVertexBuffer->position.z = 0.0f;
	pxVertexBuffer->normal.z = 1.0f;
	pxVertexBuffer->color    = ulCol;
	pxVertexBuffer->tu = 1.0f;
	pxVertexBuffer->tv = 0.0f;
	pxVertexBuffer++;
	pxVertexBuffer->position.x = (float)( width ) - 1.0f;
	pxVertexBuffer->position.y = (float)( height ) - 1.0f;
	pxVertexBuffer->position.z = 0.0f;
	pxVertexBuffer->normal.z = 1.0f;
	pxVertexBuffer->color    = ulCol;
	pxVertexBuffer->tu = 1.0f;
	pxVertexBuffer->tv = 1.0f;
	pxVertexBuffer++;
	pxVertexBuffer->position.x = 0.0f;
	pxVertexBuffer->position.y = (float)( height ) - 1.0f;
	pxVertexBuffer->position.z = 0.0f;
	pxVertexBuffer->normal.z = 1.0f;
	pxVertexBuffer->color    = ulCol;
	pxVertexBuffer->tu = 0.0f;
	pxVertexBuffer->tv = 1.0f;
	pxVertexBuffer++;

	nVertexBufferHandle = EngineCreateVertexBuffer( 6, 0 );
	EngineVertexBufferLock( nVertexBufferHandle, TRUE );

	for ( nLoop = 0; nLoop < 6; nLoop++ )
	{
		EngineVertexBufferAdd( nVertexBufferHandle, &vertices[nLoop] );
	}

	EngineVertexBufferUnlock( nVertexBufferHandle );

	return( nVertexBufferHandle );
}



void		EnginePostProcessSetEnabled( BOOL bFlag )
{
	mboPostProcessEnabled = bFlag;
}


BOOL		EnginePostProcessStartScene( ulong ulBackgroundCol )
{
BOOL	bRet = TRUE;

	if ( mboPostProcessEnabled )
	{
		if ( mhPostProcessRenderTarget == NOTFOUND )
		{
			bRet = EnginePostProcessInitGraphics();	
		}

		if ( bRet == TRUE )
		{
			EngineSetRenderTargetTexture( mhPostProcessRenderTarget, ulBackgroundCol, TRUE );
		}
		else
		{
#ifdef INCLUDE_DEBUG_INDICATORS
			FEVideoOptionsSetDebugIndicator( DEBUGIND_NO_RENDER_TARGET, 1 );
#endif
		}
	}
	return( bRet );
}

void		EnginePostProcessGrabScreen( int hDestTexture )
{
	mhPostProcessGrabScreenTexture = hDestTexture;
}

void	EnginePostProcessSetOthorgonalView( int width, int height )
{
ENGINEMATRIX Ortho2D;	
ENGINEMATRIX xMatrix;
	
	EngineMatrixOrtho( &Ortho2D, (float)width, (float)height );
	EngineMatrixIdentity( &xMatrix );

	// Set View to identity
	EngineSetViewMatrix( &xMatrix );

	// Set world matrix to convert Y up and center in screen
	xMatrix._22 = -1.0f;
	xMatrix._41 = (float)( -(width/2) );
	xMatrix._42 = (float)( +(height/2) );
	EngineSetWorldMatrix( &xMatrix );
	EngineSetProjectionMatrix( &Ortho2D);

}


void		EnginePostProcessDoBloomPass( void )
{
	// *** This stage copies the renderTarget the game has being rendering to into a smaller buffer with a high-pass filter on it (to pick out just the brightest bits)
	// Render the back frame to the blur target with a highpass threshold applied
	EnginePostProcessSetOthorgonalView( mnSmallTargetW, mnSmallTargetH );
	EngineSetRenderTargetTexture( mhPostProcessSmallRenderTarget, 0, TRUE );       // Render target is now smallRender
	EngineSetTextureNoDebugOverride( 0, mhPostProcessRenderTarget );
	EngineSetPixelShader(mpHighPassPixelShader, "HighPass" );
	EngineVertexBufferRender( mhPostProcessSmallQuadVertexBuffer, TRIANGLE_LIST );
	EngineRestoreRenderTarget();    // Render target is now back buffer

	// Now do the blur pass (vertical)
	EngineSetRenderTargetTexture( mhPostProcessSmallBlurRenderTarget, 0, TRUE );  // Render target is now smallBlur
	EngineSetTextureNoDebugOverride( 0, mhPostProcessSmallRenderTarget );
	EngineSetPixelShader( mpBlurVPixelShader, "BlurV" );
	EngineVertexBufferRender( mhPostProcessSmallQuadVertexBuffer, TRIANGLE_LIST );
	EngineRestoreRenderTarget();		// Render target is now back buffer

	// If the FullBlur process is active (Set using .. )
	if ( mfFullBlurAmount > 0.0f )
	{
		if ( mpBlurFullPSConstantTable ) 
		{
			EngineShaderConstantsSetFloat( mpBlurFullPSConstantTable, "fBlurRange", mfFullBlurAmount );
		}
		EnginePostProcessSetOthorgonalView( InterfaceGetWidth(), InterfaceGetHeight() );
		EngineSetPixelShader( mpBlurFullPixelShader, NULL );
		EngineSetTextureNoDebugOverride( 0, mhPostProcessRenderTarget );
		EngineVertexBufferRender( mhPostProcessOutputVertexBuffer, TRIANGLE_LIST );
	}
	// if not showing the debug output
	else if ( !mboPostProcessShowDebugBlurPassOutput )
	{
		// *** This renders the post-process render target onto the backbuffer
		EnginePostProcessSetOthorgonalView( InterfaceGetWidth(), InterfaceGetHeight() );
		EngineSetPixelShader( NULL, NULL );
		EngineSetTextureNoDebugOverride( 0, mhPostProcessRenderTarget );
		if ( EngineVertexBufferRender( mhPostProcessOutputVertexBuffer, TRIANGLE_LIST ) == FALSE )
		{
#ifdef INCLUDE_DEBUG_INDICATORS
			FEVideoOptionsSetDebugIndicator( DEBUGIND_VERTEXBUFFER_RENDERFAIL, 1 );
#endif
		}
	}

	// Complete the blur process (Horizontal blur)
	// *** This stage performs a horizontal blur on the filter target
	EnginePostProcessSetOthorgonalView( mnSmallTargetW, mnSmallTargetH );
	EngineSetRenderTargetTexture( mhPostProcessSmallRenderTarget, 0, TRUE );
	EngineSetTextureNoDebugOverride( 0, mhPostProcessSmallBlurRenderTarget );
	EngineSetPixelShader(mpBlurHPixelShader, "BlurH" );
	EngineVertexBufferRender( mhPostProcessSmallQuadVertexBuffer, TRIANGLE_LIST );
	EngineRestoreRenderTarget();

	EnginePostProcessSetOthorgonalView( InterfaceGetWidth(), InterfaceGetHeight() );
	EngineSetPixelShader( NULL, NULL );
	if ( mboPostProcessShowDebugBlurPassOutput )
	{
		// *** This stage renders the filtered blur view over the whole backbuffer 
		// Render the blur output  to the backbuffer
		EngineSetTextureNoDebugOverride( 0, mhPostProcessSmallRenderTarget );
		EngineVertexBufferRender( mhPostProcessOutputVertexBuffer, TRIANGLE_LIST );
	}
	else
	{
		// *** This stage renders a filtered blur view over the whole backbuffer using additive alpha
		EngineSetTextureFiltering( 3 );
		// Render the smaller quad over the top with some alpha
		EngineSetTextureNoDebugOverride( 0, mhPostProcessSmallRenderTarget );
		EngineEnableBlend( TRUE );
		EngineSetBlendMode( BLEND_MODE_SRCALPHA_ADDITIVE );
		EngineVertexBufferRender( mhPostProcessOutputAlphaVertexBuffer, TRIANGLE_LIST );
		EngineSetTextureFiltering( 1 );
	}
}


void		EnginePostProcessSetConstants( void )
{
VECT	xScreenSize;
int		nDebugIndicators = 0;

	xScreenSize.x = (float)mnSmallTargetW;
	xScreenSize.y = (float)mnSmallTargetH;
	xScreenSize.z = 0.0f;

	if ( mpHighPassPSConstantTable )
	{
		EngineShaderConstantsSetFloat( mpHighPassPSConstantTable, "gThreshold", mfHighpassThreshold );
	}
	else
	{
		nDebugIndicators |= 0x1;
	}

	if ( mpBlurHPSConstantTable )
	{
		EngineShaderConstantsSetVect( mpBlurHPSConstantTable, "xScreenSize", &xScreenSize );
	}
	else
	{
		nDebugIndicators |= 0x2;
	}

	if ( mpBlurVPSConstantTable )
	{
		EngineShaderConstantsSetVect( mpBlurVPSConstantTable, "xScreenSize", &xScreenSize );
	}
	else
	{
		nDebugIndicators |= 0x4;
	}

	if ( mpBlurVPSConstantTable ) 
	{
		EngineShaderConstantsSetVect( mpBlurVPSConstantTable, "xScreenSize", &xScreenSize );
	}
	else
	{
		nDebugIndicators |= 0x8;
	}
	if ( mpBasicVSConstantTable )
	{
	D3DXMATRIX matWorld, matView, matProj;

		mpEngineDevice->GetTransform(D3DTS_WORLD, &matWorld);
		mpEngineDevice->GetTransform(D3DTS_VIEW, &matView);
		mpEngineDevice->GetTransform(D3DTS_PROJECTION, &matProj);

		EngineShaderConstantsSetMatrix( mpBasicVSConstantTable, "gMatWorld", &matWorld );
		EngineShaderConstantsSetMatrix( mpBasicVSConstantTable, "gMatView", &matView );
		EngineShaderConstantsSetMatrix( mpBasicVSConstantTable, "gMatProj", &matProj );
	}
	else
	{
		nDebugIndicators |= 0x10;
	}
#ifdef INCLUDE_DEBUG_INDICATORS
	FEVideoOptionsSetDebugIndicator( DEBUGIND_MISSING_CONSTANT_TABLES, nDebugIndicators );
#endif
}

void		EnginePostProcessEndScene( void )
{
	if ( mboPostProcessEnabled )
	{
		// If we've got the textures turned off, we shouldnt do blurring
		if ( msbEngineNoTexturesOverride ) 
		{
			EngineRestoreRenderTarget();
			EngineDefaultState();
			EngineEnableLighting( FALSE );
			EngineSetColourMode( 0, COLOUR_MODE_TEXTURE_MODULATE );
			EngineSetBlendMode( BLEND_MODE_ALPHABLEND );
			EngineEnableCulling( 0 );
			EnginePostProcessSetConstants();
			EngineShadersStandardVertexDeclaration(0);
			EngineEnableBlend( FALSE );
			EngineEnableFog( 0 );

			EnginePostProcessSetOthorgonalView( InterfaceGetWidth(), InterfaceGetHeight() );
			EngineSetPixelShader(NULL, NULL);
			EngineSetTextureNoDebugOverride( 0, mhPostProcessRenderTarget );
			EngineVertexBufferRender( mhPostProcessOutputVertexBuffer, TRIANGLE_LIST );
		}
		else
		{
		int		nPostProcessOverlay = NOTFOUND;

			EngineRestoreRenderTarget();  // Render target is now back buffer
			if ( mhPostProcessGrabScreenTexture != NOTFOUND )
			{
				EngineSetRenderTargetTexture( mhPostProcessGrabScreenTexture, 0, TRUE );
			}
			EngineDefaultState();
			EngineSetVertexShader(mpBasicVertexShader, "Basic");
			EngineSetStandardMaterial();
			EngineEnableLighting( FALSE );
			EngineSetColourMode( 0, COLOUR_MODE_TEXTURE_MODULATE );
			EngineSetBlendMode( BLEND_MODE_ALPHABLEND );
			EngineEnableCulling( 0 );
			EnginePostProcessSetConstants();
			EngineShadersStandardVertexDeclaration(0);
			EngineEnableBlend( FALSE );
			EngineEnableFog( 0 );
			EngineEnableTextureAddressClamp( 1 );

			if ( mboBloomPassEnabled == TRUE )
			{
				EnginePostProcessDoBloomPass();
			}
			else
			{
				// ** This stage copies the game output directly onto the main back buffer
				EnginePostProcessSetOthorgonalView( InterfaceGetWidth(), InterfaceGetHeight() );
				EngineSetPixelShader( NULL, NULL );
				EngineSetTextureNoDebugOverride( 0, mhPostProcessRenderTarget );
				if ( EngineVertexBufferRender( mhPostProcessOutputVertexBuffer, TRIANGLE_LIST ) == FALSE )
				{
#ifdef INCLUDE_DEBUG_INDICATORS
					FEVideoOptionsSetDebugIndicator( DEBUGIND_VERTEXBUFFER_RENDERFAIL, 1 );
#endif
				}
			}

			if ( mhPostProcessGrabScreenTexture != NOTFOUND )
			{
				EngineRestoreRenderTarget();  // Render target is now back buffer
				EnginePostProcessSetOthorgonalView( InterfaceGetWidth(), InterfaceGetHeight() );
				EngineSetPixelShader( NULL, NULL );
				EngineEnableBlend( FALSE );
				EngineSetTextureNoDebugOverride( 0, mhPostProcessGrabScreenTexture );
				// Copy texture copy to back buffer
				EngineVertexBufferRender( mhPostProcessOutputVertexBuffer, TRIANGLE_LIST );
				mhPostProcessGrabScreenTexture = NOTFOUND;
			}
			EngineDefaultState();
			EngineSetVertexShader(NULL, NULL);
			EngineEnableBlend( TRUE );
			EngineSetBlendMode( BLEND_MODE_ALPHABLEND );
			EngineEnableLighting( TRUE );
			mpEngineDevice->SetVertexDeclaration( NULL );
		}

		if ( InterfaceGetDrawRegionWidth() != 0 )
		{
//			InterfaceSetDrawRegion( 0, 0, InterfaceGetDrawRegionWidth(), InterfaceGetDrawRegionHeight() );
		}
	}
	
}

void	EnginePostProcessCreateVertexBuffers( void )
{
ulong		ulCol;

	if ( EngineIsVRMode() )
	{
		if ( EngineHasOculus() )
		{
			InterfaceSetDrawRegion( 0, 0, 960, 1080 );
		}
		else
		{
			InterfaceSetDrawRegion( 0, 0, 0, 0 );
			InterfaceSetDrawRegion( 0, 0, InterfaceGetWidth()/2, InterfaceGetHeight() );
		}
	}
	mhPostProcessOutputVertexBuffer = EnginePostProcessGenerateQuad( InterfaceGetWidth(), InterfaceGetHeight(), 0xFFFFFFFF );
	if ( mhPostProcessOutputVertexBuffer == NOTFOUND )
	{
#ifdef INCLUDE_DEBUG_INDICATORS
		FEVideoOptionsSetDebugIndicator( DEBUGIND_VERTEXBUFFER_CREATEFAIL, 1 );
#endif
	}
	mhPostProcessSmallQuadVertexBuffer = EnginePostProcessGenerateQuad( mnSmallTargetW, mnSmallTargetH, 0xFFFFFFFF ); 
	if ( mhPostProcessSmallQuadVertexBuffer == NOTFOUND )
	{
#ifdef INCLUDE_DEBUG_INDICATORS
		FEVideoOptionsSetDebugIndicator( DEBUGIND_VERTEXBUFFER_CREATEFAIL, 2 );
#endif
	}
	ulCol = GetColWithModifiedAlpha( 0xffffffff, msfPostProcessDefaultBloomValue );
	mhPostProcessOutputAlphaVertexBuffer = EnginePostProcessGenerateQuad( InterfaceGetWidth(), InterfaceGetHeight(), ulCol );
	if ( mhPostProcessOutputAlphaVertexBuffer == NOTFOUND )
	{
#ifdef INCLUDE_DEBUG_INDICATORS
		FEVideoOptionsSetDebugIndicator( DEBUGIND_VERTEXBUFFER_CREATEFAIL, 4 );
#endif
	}
	InterfaceSetDrawRegion( 0, 0, 0, 0 );

}

BOOL		EnginePostProcessInitGraphics( void )
{
	if ( mhPostProcessRenderTarget == NOTFOUND )
	{
		mnSmallTargetW = InterfaceGetWidth() / mnBlurQualityMod;
		mnSmallTargetH = InterfaceGetHeight() / mnBlurQualityMod;

		mhPostProcessRenderTarget = EngineCreateRenderTargetTexture( InterfaceGetWidth(), InterfaceGetHeight(), 1 );	
		if ( mhPostProcessRenderTarget == NOTFOUND )
		{
			mboPostProcessEnabled = FALSE;
			return( FALSE );		
		}
		mhPostProcessSmallRenderTarget = EngineCreateRenderTargetTexture( mnSmallTargetW, mnSmallTargetH, 1 );
		mhPostProcessSmallBlurRenderTarget = EngineCreateRenderTargetTexture( mnSmallTargetW, mnSmallTargetH, 1 );

		EnginePostProcessCreateVertexBuffers();

		mpHighPassPixelShader = EngineLoadPixelShader( "Highpass", &mpHighPassPSConstantTable, 0 );
		mpBlurHPixelShader = EngineLoadPixelShader( "BlurH", &mpBlurHPSConstantTable, 0 );
		mpBlurVPixelShader = EngineLoadPixelShader( "BlurV", &mpBlurVPSConstantTable, 0 );
		mpBasicVertexShader = EngineLoadVertexShader("Basic", &mpBasicVSConstantTable, 0 );
		mpBlurFullPixelShader = EngineLoadPixelShader( "BlurFull", &mpBlurFullPSConstantTable, 0 );
		if ( mpHighPassPixelShader == NULL )
		{
			mboPostProcessEnabled = FALSE;
			return( FALSE );
		}
	}
	return( TRUE );
}

void		EnginePostProcessFreeGraphics( void )
{
	EngineReleaseTexture( &mhPostProcessRenderTarget );
	EngineReleaseTexture( &mhPostProcessSmallRenderTarget );
	EngineReleaseTexture( &mhPostProcessSmallBlurRenderTarget );
	EngineVertexBufferFree( mhPostProcessOutputVertexBuffer );
	EngineVertexBufferFree( mhPostProcessSmallQuadVertexBuffer );
	EngineVertexBufferFree( mhPostProcessOutputAlphaVertexBuffer );
	
	mhPostProcessOutputVertexBuffer = NOTFOUND;
	mhPostProcessSmallQuadVertexBuffer = NOTFOUND;
	mhPostProcessOutputAlphaVertexBuffer = NOTFOUND;

	EngineReleaseConstantBuffer( &mpHighPassPSConstantTable );
	EngineReleaseConstantBuffer( &mpBlurHPSConstantTable );
	EngineReleaseConstantBuffer( &mpBlurVPSConstantTable );
	EngineReleaseConstantBuffer( &mpBasicVSConstantTable );
	EngineReleaseConstantBuffer( &mpBlurFullPSConstantTable );
	
	
	SAFE_RELEASE( mpHighPassPixelShader );
	mpHighPassPixelShader = NULL;
	SAFE_RELEASE( mpBlurHPixelShader )
	mpBlurHPixelShader = NULL;
	SAFE_RELEASE( mpBlurVPixelShader )
	mpBlurVPixelShader = NULL;
	SAFE_RELEASE( mpBlurFullPixelShader )
	mpBlurFullPixelShader = NULL;
	SAFE_RELEASE( mpBasicVertexShader )
	mpBasicVertexShader = NULL;

}


void		EnginePostProcessSetValue( const char* szName, float fValue )
{
	if ( stricmp( szName, "BLOOM" ) == 0 )
	{
		if ( mhPostProcessOutputAlphaVertexBuffer != NOTFOUND )
		{
		ulong		ulCol;

			if ( fValue < 0.0f ) fValue = 0.0f;
			if ( fValue > 1.0f ) fValue = 1.0f;
			ulCol = (ulong)( fValue * 255.0f );
			ulCol <<= 24;
			ulCol |= 0xFFFFFF;

			if ( EngineIsVRMode() ) 
			{
				if ( EngineHasOculus() )
				{
					InterfaceSetDrawRegion( 0, 0, 960, 1080 );
				}
				else
				{	InterfaceSetDrawRegion( 0, 0, 0, 0 );
					InterfaceSetDrawRegion( 0, 0, InterfaceGetWidth()/2, InterfaceGetHeight() );
				}
			}
			EngineVertexBufferFree( mhPostProcessOutputAlphaVertexBuffer );
			mhPostProcessOutputAlphaVertexBuffer = EnginePostProcessGenerateQuad( InterfaceGetWidth(), InterfaceGetHeight(), ulCol );
			InterfaceSetDrawRegion( 0, 0, 0, 0 );
		}
		else
		{
			msfPostProcessDefaultBloomValue = fValue;
		}

	}
	else if ( stricmp( szName, "FULLBLUR" ) == 0 )
	{
		mfFullBlurAmount = fValue;
		if ( mfFullBlurAmount >= 50.0f )
		{
			mfFullBlurAmount = 50.0f;
		}
	}
	else if ( stricmp( szName, "BLOOMTHRESHOLD" ) == 0 )
	{
		mfHighpassThreshold = fValue;
		if ( mfHighpassThreshold >= 1.0f )
		{
			mfHighpassThreshold = 0.99f;
		}
	}
	else if ( stricmp( szName, "SHOWBLOOM") == 0 )
	{
		if ( fValue == 0.0f )
		{	
			mboBloomPassEnabled = FALSE;
		}
		else
		{
			mboBloomPassEnabled = TRUE;
		}
  	}
	else if ( stricmp( szName, "SHOWBLURPASS" ) == 0 )
	{
		if ( fValue == 0.0f )
		{
			mboPostProcessShowDebugBlurPassOutput = FALSE;
		}
		else
		{
			mboPostProcessShowDebugBlurPassOutput = TRUE;
		}
	}
	else if ( stricmp( szName, "BLURQUALITY" ) == 0 )
	{
	int	nOldValue = mnBlurQualityMod;

		if ( fValue <= 0.0f )
		{
			mnBlurQualityMod = 1;
		}
		else if ( fValue <= 1.0f )
		{
			mnBlurQualityMod = 4;
		}
		else if ( fValue <= 2.0f )
		{
			mnBlurQualityMod = 3;
		}
		else if ( fValue <= 3.0f )
		{
			mnBlurQualityMod = 2;
		}
		else 
		{
			mnBlurQualityMod = 1;
		}

		if ( mnBlurQualityMod != nOldValue )
		{
			EnginePostProcessFreeGraphics();
			EnginePostProcessInitGraphics();
		}
	}
	else if ( stricmp( szName, "POSTPROCESS" ) == 0 )
	{
		if ( fValue == 0.0f )
		{
			mboPostProcessEnabled = FALSE;
		}
		else
		{
			mboPostProcessEnabled = TRUE;
		}
	}
	else if ( stricmp( szName, "RELOAD" ) == 0 )
	{
		EnginePostProcessFreeGraphics();
		EnginePostProcessInitGraphics();
	}
}

