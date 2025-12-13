
#include "EngineDX.h"

#include <stdio.h>
#include <Engine.h>

#include <StandardDef.h>
#include <Rendering.h>
#include <Interface.h>

//#include "../../Universal/GameCode/FrontEnd/Screens/FEVideoOptions.h"		// TEMP - for debug indicators

#include "../LibCode/CodeUtil/Archive.h"
#include "../LibCode/Interface/DirectX/InterfaceInternalsDX.h"
#include "../LibCode/Interface/Common/Overlays/TexturedOverlays.h"


#include "ShadowMapDX.h"
#include "TextureManagerDX.h"

//----------------------------------------------------------------------------------------------------

#define		MAX_TEXTURES_IN_MANAGER		512

typedef enum
{
	TEXTURE_STATE_NONE = 0,
	TEXTURE_STATE_REQUESTED,
	TEXTURE_STATE_LOADING,
	TEXTURE_STATE_LOADED,
	TEXTURE_STATE_LOAD_ERROR,

} TEXTURE_LOADING_STATE;

typedef struct
{
	LPGRAPHICSTEXTURE		pTexture;
	int						nUsed;	
	TEXTURE_LOADING_STATE	nState;	
	char					szFilename[MAX_PATH];
	int						nLoadFlags;
	int						nMode;
	int						nArchiveHandle;
	BOOL					boIsRenderTarget;						
	BOOL					boTextureHasAlpha;		// Effectively means 'Is PNG' atm..
	LPGRAPHICSSURFACE		pDepthStencilSurface;

} TEXTURE_REFERENCES;


TEXTURE_REFERENCES		maTextureReferences[ MAX_TEXTURES_IN_MANAGER ];

IGRAPHICSSURFACE*	mapRenderTargetStack[8] = { NULL };
IGRAPHICSSURFACE*	mapDepthStencilStack[8] = { NULL };
int			mnNumRenderTargetsOnStack = 0;

IGRAPHICSSURFACE*	mpTempDepthSurface = NULL;

BOOL	mboEngineTextureHasCheckCompatability = FALSE;
BOOL	mboEngineTextureCanLoad24bit = FALSE;
BOOL	mboEngineTextureCanLoad32bit = FALSE;
BOOL	mboEngineTextureCanLoadPalettised = FALSE;
ENGINE_OPTIONS	mxEngineOptions = { FALSE };
BOOL	mboTextureManagerKillThread = FALSE;
int		mnTextureManagerAsyncLoadHandle = NOTFOUND;
BOOL	mboPendingLoads = FALSE;
int		mnHighestTextureSlotUsed = 0;

int		msnNumberOfRenderTargetsCreated = 0;
HANDLE mhTextureLoadThread = 0;

volatile bool	mbLoadThreadBufferReady = false;
int		mnLoadThreadBufferMipLevels = 1;
byte*	mpbLoadThreadBuffer = NULL;
int		mnLoadThreadBufferSize = 0;
bool	mbLoadThreadDDSLoad = false;
bool	mbLoadThreadDDSSave = false;
char	mszDDSFilename[256];
TEXTURE_MANAGER_STATS		mxTextureManagerStats;

eSurfaceFormat		mnLoadThreadBufferFormat;
eSurfaceFormat		mnLastSuccessFormat = SURFACEFORMAT_A8R8G8B8;
#ifdef TUD11
int		mnLoadThreadBufferFilter = 0;
int		mnLoadThreadBufferMipFilter = 0;
#else
int		mnLoadThreadBufferFilter = D3DX_FILTER_NONE;
int		mnLoadThreadBufferMipFilter = D3DX_FILTER_NONE;
#endif

int		EngineTextureManagerGetNumRenderTargets( void )
{
	return( msnNumberOfRenderTargetsCreated );
}

void		EngineRenderTargetsTrackingListAllocated( char* acErrorOut )
{
int		nLoop;
int		nCount = 0;
char	acString[256];
int		nExplosionTexturesCount = 0;
int		nValidCount = 0;

	for ( nLoop = 0; nLoop < MAX_TEXTURES_IN_MANAGER; nLoop++ )
	{
		if ( ( maTextureReferences[nLoop].nUsed != 0 ) &&
			 ( maTextureReferences[nLoop].boIsRenderTarget == TRUE ) )
		{
			sprintf( acString, "-- %s\r\n", maTextureReferences[nLoop].szFilename );	
			strcat( acErrorOut, acString );
		}	
	}

}

void		EngineTextureManagerDump( TextureManagerPrintCallback fnPrintCallback )
{
int		nLoop;
int		nCount = 0;
char	acString[256];
int		nExplosionTexturesCount = 0;
int		nValidCount = 0;

	for ( nLoop = 0; nLoop < MAX_TEXTURES_IN_MANAGER; nLoop++ )
	{
		if ( maTextureReferences[nLoop].nUsed != 0 )
		{
			nCount++;
			if ( strnicmp( "Data/Textures/Explosion", maTextureReferences[nLoop].szFilename, 23 ) == 0 )
			{
				nExplosionTexturesCount++;
				if ( maTextureReferences[nLoop].pTexture != NULL )
				{
					nValidCount++;
				}
			}
			else if ( maTextureReferences[nLoop].pTexture != NULL )
			{
				sprintf( acString, "%d.. [TEX] %s", nLoop, maTextureReferences[nLoop].szFilename );
				nValidCount++;
				fnPrintCallback( acString );
			}
			else
			{
				sprintf( acString, "%d.. [NULL] %s", nLoop, maTextureReferences[nLoop].szFilename );
				fnPrintCallback( acString );
			}
		}	
	}
	sprintf( acString, "%d referenced, %d valid, %d explosion textures", nCount, nValidCount, nExplosionTexturesCount );
	fnPrintCallback( acString );
}


//----------------------------------------------------------------------------------------------------
void		EngineTextureManagerGetStats( TEXTURE_MANAGER_STATS* pxOut )
{
int	nLoop = 0;
int	nCount = 0;
int	nValidCount = 0;

	for ( nLoop = 0; nLoop < MAX_TEXTURES_IN_MANAGER; nLoop++ )
	{
		if ( maTextureReferences[nLoop].nUsed != 0 )
		{
			nCount++;
			if ( maTextureReferences[nLoop].pTexture )
			{
				nValidCount++;
			}
		}
	}

	mxTextureManagerStats.nHandleCounter = mnHighestTextureSlotUsed;
	mxTextureManagerStats.nNumTexturesLoaded = nValidCount;

}

void	EngineSetOptions( ENGINE_OPTIONS* pxOptions )
{
	mxEngineOptions = *pxOptions;
}


LPGRAPHICSTEXTURE		EngineGetTextureDirectDX( TEXTURE_HANDLE hTexture )
{
	return( maTextureReferences[ hTexture ].pTexture );
}


IGRAPHICSSURFACE*		EngineGetDepthStencilSurfaceDirectDX( TEXTURE_HANDLE hTexture )
{
	return( maTextureReferences[ hTexture ].pDepthStencilSurface );
}



TEXTURE_HANDLE	EngineCreateTexture( int nWidth, int nHeight, int format )
{
LPGRAPHICSTEXTURE	pTexture = NULL;
int		nLoop = 1;

	while ( maTextureReferences[nLoop].nUsed != 0 )
	{
		nLoop++;
		if ( nLoop == MAX_TEXTURES_IN_MANAGER ) return( 0 );
	}

#ifdef TUD11
	PANIC_IF( TRUE, "DX11 EngineCreateTexture TBI" );
#else
	D3DFORMAT	 d3dFormat;
	if ( format == 0 )
	{
		d3dFormat = D3DFMT_R5G6B5;
	}
	else
	{
		d3dFormat = D3DFMT_A8R8G8B8;
	}

	mpEngineDevice->CreateTexture(nWidth,nHeight,1,0,d3dFormat,D3DPOOL_MANAGED, &pTexture, NULL );
#endif

	if ( pTexture != NULL ) 
	{
		maTextureReferences[nLoop].pTexture = pTexture;
		maTextureReferences[nLoop].nUsed = 1;
		maTextureReferences[nLoop].nState = TEXTURE_STATE_LOADED;
		strcpy( maTextureReferences[nLoop].szFilename, "[CreateTexture]" );

		if ( nLoop >= mnHighestTextureSlotUsed )
		{
			mnHighestTextureSlotUsed = nLoop + 1;
		}
		return( nLoop );
	}
	return( 0 );
}

BYTE*		EngineLockTexture( TEXTURE_HANDLE handle, int* pnPitch, BOOL bClear )
{
	if ( handle >= 0 ) 
	{
	LPGRAPHICSTEXTURE	pTexture;
#ifdef TUD11
	PANIC_IF( TRUE, "DX11 EngineLockTexture TBI" );

#else
	D3DLOCKED_RECT	xLockedRectDest;

		pTexture = maTextureReferences[handle].pTexture;

		if ( pTexture )
		{
			pTexture->LockRect( 0, &xLockedRectDest, NULL, 0 );//&xDestRect, 0 );

			*pnPitch = xLockedRectDest.Pitch;

			if ( bClear )
			{
			int		nWidth = EngineTextureGetWidth( handle );
			int		nHeight = EngineTextureGetHeight( handle );
			int		nMemSize = xLockedRectDest.Pitch * nHeight;

				memset( xLockedRectDest.pBits, 0, nMemSize );
			}
			return( (BYTE*)xLockedRectDest.pBits );
		}
#endif
	}
	return( NULL );
}

void		EngineUnlockTexture( TEXTURE_HANDLE handle )
{
	if ( handle >= 0 ) 
	{
	LPGRAPHICSTEXTURE	pTexture;

		pTexture = maTextureReferences[handle].pTexture;
#ifdef TUD11
		PANIC_IF(TRUE, "DX11 EngineUnlockTexture TBI" );
#else
		if ( pTexture )
		{
			pTexture->UnlockRect( 0 );
		}
#endif
	}
}

void	EngineCopyTexture( TEXTURE_HANDLE hSrcTexture, TEXTURE_HANDLE hDestTexture )
{
LPGRAPHICSSURFACE	pSourceSurface;
LPGRAPHICSSURFACE	pDestinationSurface;
LPGRAPHICSTEXTURE	pSourceTexture;
LPGRAPHICSTEXTURE	pDestinationTexture;
HRESULT		hRet;

#ifdef TUD11
	PANIC_IF( TRUE, "DX11 EngineCopyTexture TBI" );
#else
	pSourceTexture = EngineGetTextureDirectDX(hSrcTexture);
	pDestinationTexture = EngineGetTextureDirectDX(hDestTexture);

	pSourceTexture->GetSurfaceLevel(0, &pSourceSurface);
	pDestinationTexture->GetSurfaceLevel(0, &pDestinationSurface);

	hRet = D3DXLoadSurfaceFromSurface( pDestinationSurface, NULL, NULL, pSourceSurface, NULL, NULL, D3DX_DEFAULT, 0 );
	
	pSourceSurface->Release();
	pDestinationSurface->Release();
#endif
}

void	EngineCopyTextureToPosition( TEXTURE_HANDLE hSrcTexture, TEXTURE_HANDLE hDestTexture, int nDestPosX, int nDestPosY )
{
IGRAPHICSSURFACE*	pSourceSurface;
IGRAPHICSSURFACE*	pDestinationSurface;
LPGRAPHICSTEXTURE	pSourceTexture;
LPGRAPHICSTEXTURE	pDestinationTexture;
HRESULT		hRet;
RECT		xDestRect;

#ifdef TUD11
	PANIC_IF( TRUE, "DX11 EngineCopyTextureToPosition TBI" );
#else
D3DSURFACE_DESC		xSourceDesc;

	pSourceTexture = EngineGetTextureDirectDX(hSrcTexture);
	pDestinationTexture = EngineGetTextureDirectDX(hDestTexture);

	pSourceTexture->GetSurfaceLevel(0, &pSourceSurface);
	pDestinationTexture->GetSurfaceLevel(0, &pDestinationSurface);

	pSourceSurface->GetDesc( &xSourceDesc );
	xDestRect.left = nDestPosX;
	xDestRect.top = nDestPosY;
	xDestRect.right = nDestPosX + xSourceDesc.Width;
	xDestRect.bottom = nDestPosY + xSourceDesc.Height;

	hRet = D3DXLoadSurfaceFromSurface( pDestinationSurface, NULL, &xDestRect, pSourceSurface, NULL, NULL, D3DX_DEFAULT, 0 );
	
	pSourceSurface->Release();
	pDestinationSurface->Release();
#endif
}


void	EngineSetRenderTargetTexture( TEXTURE_HANDLE hTexture, uint32 ulClearCol, BOOL bClear )
{
#ifdef TUD11
	PANIC_IF( TRUE, "DX11 EngineSetRenderTargetTexture TBI" );
#else
LPGRAPHICSTEXTURE	pRenderTargetTexture;
IGRAPHICSSURFACE*	pTargetSurface;
IGRAPHICSSURFACE*	pTargetDepthStencilSurface;

	// Store the old renderTarget for restoring it later
	mpEngineDevice->GetRenderTarget( 0, &mapRenderTargetStack[mnNumRenderTargetsOnStack] );
	PANIC_IF( mapRenderTargetStack[mnNumRenderTargetsOnStack] == NULL, "No render target?" );

	mpEngineDevice->GetDepthStencilSurface( &mapDepthStencilStack[mnNumRenderTargetsOnStack] );

	PANIC_IF( mnNumRenderTargetsOnStack >= 8, "Too many render targets on stack" );
	mnNumRenderTargetsOnStack++;

	// Get the raw surface etc
	pRenderTargetTexture = EngineGetTextureDirectDX(hTexture);
	if ( pRenderTargetTexture )
	{
	D3DCOLOR	xColor;
	HRESULT		ret;

		xColor = (D3DCOLOR)( ulClearCol );

		pRenderTargetTexture->GetSurfaceLevel(0, &pTargetSurface);
		pTargetDepthStencilSurface = EngineGetDepthStencilSurfaceDirectDX(hTexture);

		mpEngineDevice->SetRenderTarget( 0,pTargetSurface );
		mpEngineDevice->SetDepthStencilSurface( pTargetDepthStencilSurface );

		if ( bClear )
		{
			if ( pTargetDepthStencilSurface )
			{
				ret = mpEngineDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL, xColor, 1.0f, 0 );
			}
			else
			{
				ret = mpEngineDevice->Clear( 0, NULL, D3DCLEAR_TARGET, xColor, 1.0f, 0 );
			}
		}
	
		// Engine now holds the reference, so remove ours
		pTargetSurface->Release();
	}
#endif
}

void				EngineRestoreRenderTarget( void )
{
#ifdef TUD11
	PANIC_IF( TRUE, "DX11 EngineRestoreRenderTarget TBI" );

#else
	PANIC_IF( mnNumRenderTargetsOnStack == 0, "Restoring zero render target?" );
	if ( mnNumRenderTargetsOnStack > 0 )
	{
		mnNumRenderTargetsOnStack--;
	}

	mpEngineDevice->SetDepthStencilSurface( mapDepthStencilStack[mnNumRenderTargetsOnStack]  );
	mpEngineDevice->SetRenderTarget( 0, mapRenderTargetStack[mnNumRenderTargetsOnStack] );

	// Now we've set the old render target and depth stencil on the engine, it holds the reference and 
	// we should release ours
	if ( mapDepthStencilStack[mnNumRenderTargetsOnStack] )
	{
		mapDepthStencilStack[mnNumRenderTargetsOnStack]->Release();
		mapDepthStencilStack[mnNumRenderTargetsOnStack] = NULL;
	}
	if ( mapRenderTargetStack[mnNumRenderTargetsOnStack] )
	{
		mapRenderTargetStack[mnNumRenderTargetsOnStack]->Release();
		mapRenderTargetStack[mnNumRenderTargetsOnStack] = NULL;
	}
#endif
}


_D3DFORMAT		maPreferredRenderTargetFormats[] = 
{
	D3DFMT_X8R8G8B8,
	D3DFMT_R8G8B8,
	D3DFMT_X8B8G8R8,
	D3DFMT_R5G6B5,
    D3DFMT_X1R5G5B5,

	D3DFMT_UNKNOWN
};

TEXTURE_HANDLE	EngineCreateRenderTargetTexture( int nWidth, int nHeight, int mode, const char* szTrackingName )
{
D3DDISPLAYMODE d3ddm;
HRESULT hr;
LPGRAPHICSTEXTURE	pTexture;
int		nLoop = 1;
int		nFormatsLoop = 0;

	while ( maTextureReferences[nLoop].nUsed != 0 )
	{
		nLoop++;
		if ( nLoop == MAX_TEXTURES_IN_MANAGER ) return( 0 );
	}

#ifdef TUD11
	PANIC_IF( TRUE, "dx11 EngineCreateRenderTargetTexture tbi" );
#else

	LPGRAPHICS pD3D = InterfaceGetD3D();

	switch( mode )
	{
	case 0:
	case 5:
	default:
		mpEngineDevice->CreateTexture(nWidth,nHeight,1,D3DUSAGE_RENDERTARGET,D3DFMT_R5G6B5,D3DPOOL_DEFAULT, &pTexture, NULL );
		break;
	case 1:
	case 2:
	    if( FAILED( pD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d3ddm ) ) )
		{
			PANIC_IF(TRUE,"Couldn't get display mode" );
			return( NULL );
		}

#ifdef INCLUDE_DEBUG_INDICATORS
		FEVideoOptionsSetDebugIndicator( DEBUGIND_DEVICE_FORMAT, (int)d3ddm.Format );
#endif
		nFormatsLoop = 0;
		while( 1 )
		{
			hr = pD3D->CheckDeviceFormat( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3ddm.Format, D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, maPreferredRenderTargetFormats[nFormatsLoop]);
			if ( SUCCEEDED( hr ) )
			{
				mpEngineDevice->CreateTexture(nWidth,nHeight,1,D3DUSAGE_RENDERTARGET,maPreferredRenderTargetFormats[nFormatsLoop],D3DPOOL_DEFAULT, &pTexture, NULL );
				if ( pTexture == NULL )
				{
#ifdef INCLUDE_DEBUG_INDICATORS
					FEVideoOptionsSetDebugIndicator( DEBUGIND_RENDERTARGET_CREATETEXTURE_FAILED, 1 );
#endif
				}
				break;
			}
			else
			{
				if ( nFormatsLoop == 0 )
				{
#ifdef INCLUDE_DEBUG_INDICATORS
					FEVideoOptionsSetDebugIndicator( DEBUGIND_X8R8G8B8_NOT_SUPPORTED, 1 );
#endif
				}
				nFormatsLoop++;

				if ( maPreferredRenderTargetFormats[nFormatsLoop] == D3DFMT_UNKNOWN )
				{
					PANIC_IF(TRUE, "No valid render target formats available");
					return( NULL );
				}
			}
		}
		break;
	case 3:
		mpEngineDevice->CreateTexture(nWidth,nHeight,1,D3DUSAGE_RENDERTARGET,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT, &pTexture, NULL );
		break;
	case 4:
		mpEngineDevice->CreateTexture(nWidth,nHeight,1,D3DUSAGE_RENDERTARGET,D3DFMT_R32F,D3DPOOL_DEFAULT, &pTexture, NULL );
		break;
	}

	if ( pTexture != NULL ) 
	{
	D3DSURFACE_DESC		xDesc;
	IGRAPHICSSURFACE*	pDepthStencilSurface;
	IGRAPHICSSURFACE*	pTargetSurface;

		maTextureReferences[nLoop].pTexture = pTexture;
		maTextureReferences[nLoop].nUsed = 1;
		maTextureReferences[nLoop].nState = TEXTURE_STATE_LOADED;
		maTextureReferences[nLoop].boIsRenderTarget = TRUE;
		if ( szTrackingName )
		{
			strcpy( maTextureReferences[nLoop].szFilename, szTrackingName );
		}
		else
		{
			strcpy( maTextureReferences[nLoop].szFilename, "[CreateRenderTarget]" );
		}
		
		switch( mode )
		{
		case 0:
		case 2:
			break;
		case 5:
		case 4:
		case 3:
		case 1:
		default:
			pTexture->GetSurfaceLevel(0,&pTargetSurface);	
			pTargetSurface->GetDesc( &xDesc );
			
			mpEngineDevice->CreateDepthStencilSurface( xDesc.Width, xDesc.Height, D3DFMT_D24S8, D3DMULTISAMPLE_NONE, 0, TRUE, &pDepthStencilSurface, NULL );
			maTextureReferences[nLoop].pDepthStencilSurface = pDepthStencilSurface;
			pTargetSurface->Release();
			break;
		}

		if ( nLoop >= mnHighestTextureSlotUsed )
		{
			mnHighestTextureSlotUsed = nLoop + 1;
		}
		msnNumberOfRenderTargetsCreated++;
		return( nLoop );
	}
#endif
	return( 0 );
}

int				EngineTextureCreateInterfaceOverlay( int nLayer, TEXTURE_HANDLE hTexture )
{
	if ( hTexture >= 0 )
	{
		if ( maTextureReferences[ hTexture ].nState == TEXTURE_STATE_LOADED )
		{
			maTextureReferences[hTexture].nUsed++;
			return( TexturedOverlayCreateDirect( nLayer, maTextureReferences[ hTexture ].pTexture, hTexture ) );
		}
		else
		{
			return( TexturedOverlayCreate( nLayer, NULL ) );
		}
	}
	else
	{
		return( NOTFOUND );
	}
}

BOOL	EngineTextureDidLoadFail( TEXTURE_HANDLE nTexHandle )
{
	if ( nTexHandle > 0 )
	{
		if ( maTextureReferences[ nTexHandle ].nState == TEXTURE_STATE_LOAD_ERROR )
		{
			return( TRUE );
		}
	}
	return( FALSE );
}

BOOL	EngineTextureIsFullyLoaded( TEXTURE_HANDLE nTexHandle )
{
	if ( nTexHandle > 0 )
	{
		if ( maTextureReferences[ nTexHandle ].nState == TEXTURE_STATE_LOADED )
		{
			return( TRUE );
		}
	}
	return( FALSE );
}

void	EngineLoadTextureGetParams( int nMipMode, int* pnFilter, int* pnMipFilter, int* pnMipLevels )
{
int		boReduceFilter = 1;
int		boMipFilter = 1;

#ifdef TUD11
	*pnFilter = 0;
	*pnMipFilter = 0;
	*pnMipLevels = 0;

#else
	switch( nMipMode )
	{
	case 0:
	default:
		*pnFilter = D3DX_DEFAULT;
		*pnMipFilter = D3DX_DEFAULT;
		*pnMipLevels = D3DX_DEFAULT;
		break;
	case 1:
		*pnFilter = D3DX_FILTER_POINT;
		*pnMipFilter = D3DX_FILTER_POINT;
		*pnMipLevels = D3DX_DEFAULT;
		break;
	case 2:
		*pnFilter = 0;
		*pnMipFilter = 0;
		*pnMipLevels = 0;
		break;
	case 3:
		*pnFilter = D3DX_FILTER_POINT;
		*pnMipFilter = D3DX_FILTER_TRIANGLE;
		*pnMipLevels = D3DX_DEFAULT;
		break;
	case 4:
		*pnFilter = D3DX_FILTER_LINEAR;
		*pnMipFilter = D3DX_FILTER_LINEAR;
		*pnMipLevels = D3DX_DEFAULT;
		break;
	}
#endif
}


TEXTURE_HANDLE	EngineLoadTextureFromMem( byte* pbMem, int nMemSize, int width, int height, int format, int nMipMode, int* pnErrorFlag )
{
int	nRet;
int		nLoop = 1;
LPGRAPHICSTEXTURE	pxTexture = NULL;
int		nMipLevels = 1;
//char	acString[256];

	while ( maTextureReferences[nLoop].nUsed != 0 )
	{
		nLoop++;
		if ( nLoop == MAX_TEXTURES_IN_MANAGER ) return( 0 );
	}

#ifdef TUD11
	PANIC_IF( TRUE, "DX11 EngineLoadTextureFromMem TBI" );
#else
	int		nFilter = D3DX_FILTER_NONE;
	int		nMipFilter = D3DX_FILTER_NONE;

	EngineLoadTextureGetParams( nMipMode, &nFilter, &nMipFilter, &nMipLevels );

	if ( nMipMode == 1 )
	{
		nRet = D3DXCreateTexture( mpEngineDevice, width, height, 0, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &pxTexture );
	}
	else
	{
		nRet = D3DXCreateTexture( mpEngineDevice, width, height, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &pxTexture );
	}

	if ( pxTexture )
	{
	D3DLOCKED_RECT	xLockedRectDest;
	//D3DLOCKED_RECT	xLockedRectSrc; //unused ?
	byte*		pbDestination;
	uint32*		pulWrite;
	byte*		pbSrc;
	int			nX;
	int			nY;
	byte			R,G,B;
		pxTexture->LockRect(0,&xLockedRectDest, NULL, 0 );
		pbDestination = (byte*)( xLockedRectDest.pBits );
		pbSrc = (byte*)( pbMem );
	
		for ( nY = 0; nY < height; nY++ )
		{
			pulWrite = (uint32*)pbDestination;
			for ( nX = 0; nX < width; nX++ )
			{
				R = *pbSrc++;
				G = *pbSrc++;
				B = *pbSrc++;
				*pulWrite++ = 0xFF000000|(R<<16)|(G<<8)|(B);
			} // FOR X < MAPSIZE

			pbDestination += xLockedRectDest.Pitch;		
		}
		pxTexture->UnlockRect( 0 );
	}

	if ( nMipMode == 1 )
	{
		D3DXFilterTexture( pxTexture, NULL, 0, D3DX_DEFAULT );
	}
#endif

	maTextureReferences[nLoop].pTexture = pxTexture;
	maTextureReferences[nLoop].nUsed = 1;
	maTextureReferences[nLoop].nState = TEXTURE_STATE_LOADED;
	sprintf( maTextureReferences[nLoop].szFilename, "[LoadFromMem %08x size %d]", (uint32)pbMem, nMemSize );
	if ( nLoop >= mnHighestTextureSlotUsed )
	{
		mnHighestTextureSlotUsed = nLoop + 1;
	}
	return( nLoop );

}

TEXTURE_HANDLE	EngineLoadTextureFromFileInMem( byte* pbMem, int nMipMode, int* pnErrorFlag, int nMemSize, const char* szOptionalFilename )
{
int	nRet;
int		nLoop = 1;
LPGRAPHICSTEXTURE	pxTexture = NULL;
int		nMipLevels = 1;
//char	acString[256];

	while ( maTextureReferences[nLoop].nUsed != 0 )
	{
		nLoop++;
		if ( nLoop == MAX_TEXTURES_IN_MANAGER ) return( 0 );
	}

#ifdef TUD11
	PANIC_IF( TRUE, "DX11 EngineLoadTextureFromFileInMem TBI" );
#else
	int		nFilter = D3DX_FILTER_NONE;
	int		nMipFilter = D3DX_FILTER_NONE;
	EngineLoadTextureGetParams( nMipMode, &nFilter, &nMipFilter, &nMipLevels );

//	nRet = D3DXCreateTextureFromFileInMemory( mpEngineDevice, pbMem, nMemSize, &pxTexture );
	nRet = D3DXCreateTextureFromFileInMemoryEx( mpEngineDevice, pbMem, nMemSize,0,0,nMipLevels,0,D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, nFilter,nMipFilter, 0xFF0000FF, NULL,NULL, &pxTexture );
	if( FAILED( nRet ) )
	{
	D3DFORMAT	dxFormat = EngineDXGetGraphicsFormat( mnLastSuccessFormat );
		nRet = D3DXCreateTextureFromFileInMemoryEx( mpEngineDevice, pbMem, nMemSize,0,0,nMipLevels,0,dxFormat, D3DPOOL_MANAGED, nFilter,nMipFilter, 0xFF0000FF, NULL,NULL, &pxTexture );
		if( FAILED( nRet ) )
		{
			pxTexture = NULL;
			return( 0 );
		}
	}
#endif

	maTextureReferences[nLoop].pTexture = pxTexture;
	maTextureReferences[nLoop].nUsed = 1;
	maTextureReferences[nLoop].nState = TEXTURE_STATE_LOADED;
	maTextureReferences[nLoop].boTextureHasAlpha = FALSE;
	if ( ( szOptionalFilename ) &&
		 ( szOptionalFilename[0] != 0 ) )
	{
	const char*		szExtension = &szOptionalFilename[ strlen(szOptionalFilename)-3 ];
		strcpy( maTextureReferences[nLoop].szFilename, szOptionalFilename );

		if ( ( stricmp( szExtension, "PNG" ) == 0 ) ||
			 ( stricmp( szExtension, "tga" ) == 0 ) )
		{
			maTextureReferences[nLoop].boTextureHasAlpha = TRUE;
		}
	}
	else
	{
		strcpy( maTextureReferences[nLoop].szFilename, "[LoadFileFromMem]" );
	}

	if ( nLoop >= mnHighestTextureSlotUsed )
	{
		mnHighestTextureSlotUsed = nLoop + 1;
	}
	return( nLoop );
}


TEXTURE_HANDLE	EngineLoadTextureInternal( const char* szFilename, int nMode, int* pnErrorFlag, int nArchiveHandle )
{
int		nLoop;

	for ( nLoop = 1; nLoop < mnHighestTextureSlotUsed; nLoop++ )
	{
		if ( ( maTextureReferences[nLoop].szFilename[0] != 0 ) &&
			 ( maTextureReferences[nLoop].szFilename[0] != '[' ) )
		{
			if ( _stricmp( maTextureReferences[nLoop].szFilename, szFilename ) == 0 )
			{
				maTextureReferences[nLoop].nUsed++;
				return( nLoop );
			}
		}
	}

	nLoop = 1;
	while ( maTextureReferences[nLoop].nUsed != 0 )
	{
		nLoop++;
		if ( nLoop == MAX_TEXTURES_IN_MANAGER )
		{
			PANIC_IF( TRUE, "Out of space in engine TextureManager" );
			return( 0 );
		}
	}

	strcpy( maTextureReferences[nLoop].szFilename, szFilename );
	maTextureReferences[nLoop].nState = TEXTURE_STATE_REQUESTED;
	maTextureReferences[nLoop].nMode = nMode;
	maTextureReferences[nLoop].nUsed = 1;
	maTextureReferences[nLoop].pTexture = NULL;
	maTextureReferences[nLoop].nLoadFlags = 0;
	maTextureReferences[nLoop].nArchiveHandle = nArchiveHandle;
	if ( nLoop >= mnHighestTextureSlotUsed )
	{
		mnHighestTextureSlotUsed = nLoop + 1;
	}
	mboPendingLoads = TRUE;
	return( nLoop );
}

TEXTURE_HANDLE	EngineLoadTextureFromArchive( const char* szFilename, int nMode, int nArchiveHandle, int* pnErrorFlag )
{
	return( EngineLoadTextureInternal( szFilename, nMode, pnErrorFlag, nArchiveHandle ) );
}

TEXTURE_HANDLE	EngineLoadTexture( const char* szFilename, int nMode, int* pnErrorFlag )
{
	return( EngineLoadTextureInternal( szFilename, nMode, pnErrorFlag, NOTFOUND ) );
}

void	EngineReleaseTexture( TEXTURE_HANDLE* pnHandle )
{
int	nHandle = *pnHandle;

	if ( nHandle > 0 )
	{
		if ( maTextureReferences[ nHandle ].nUsed > 1 )
		{
			maTextureReferences[ nHandle ].nUsed--;
		}
		else if ( maTextureReferences[ nHandle ].pTexture != NULL )
		{
		int		nNewRefCount;

			nNewRefCount = maTextureReferences[ nHandle ].pTexture->Release();
			maTextureReferences[ nHandle ].pTexture = NULL;

			if ( maTextureReferences[nHandle].boIsRenderTarget == TRUE )
			{
				msnNumberOfRenderTargetsCreated--;
				maTextureReferences[nHandle].boIsRenderTarget = FALSE;
			}
			if ( maTextureReferences[ nHandle ].pDepthStencilSurface != NULL )
			{
				maTextureReferences[ nHandle ].pDepthStencilSurface->Release();
				maTextureReferences[ nHandle ].pDepthStencilSurface = NULL;
			}
			maTextureReferences[ nHandle ].nUsed = 0;
			maTextureReferences[ nHandle ].szFilename[0] = 0;
			maTextureReferences[ nHandle ].nState = TEXTURE_STATE_NONE;
		}
		else  // We don't have a texture for it yet.. probably freed before it got chance to load (or the load failed?)
		{
			maTextureReferences[ nHandle ].nUsed = 0;
			maTextureReferences[ nHandle ].szFilename[0] = 0;
			maTextureReferences[ nHandle ].nState = TEXTURE_STATE_NONE;
		}
		*pnHandle = -1;
	}
}

void	EngineSetTextureNoDebugOverride( int nTex, TEXTURE_HANDLE nTexHandle )
{
#ifdef TUD11
	PANIC_IF( TRUE, "DX11 EngineSetTextureNoDebugOverride TBI" );
#else
	if ( nTexHandle >= 0 ) 
	{
		mpEngineDevice->SetTexture( nTex, maTextureReferences[ nTexHandle ].pTexture );
	}
	else
	{
		mpEngineDevice->SetTexture( nTex, NULL );
	}
#endif
}

BOOL		mboHasSetShadowMultitexture = FALSE;

void			EngineSetShadowMultitexture( BOOL bFlag )
{
	if ( mboHasSetShadowMultitexture != bFlag )
	{
		if ( bFlag )
		{
			EngineSetTexture( 1, EngineShadowMapGetDepthMap() );
			EngineTextureSetBestFiltering( 1, 4 );

			mpEngineDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
			mpEngineDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			mpEngineDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
			mpEngineDevice->SetTextureStageState( 1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
		}
		else
		{
			EngineSetTexture( 1, NOTFOUND );
		}
		mboHasSetShadowMultitexture = bFlag;
	}
}

void*		mapxEngineLastSetTextures[8] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

void	EngineSetTexture( int nTex, TEXTURE_HANDLE nTexHandle )
{
#ifdef TUD11
	PANIC_IF( TRUE, "DX11 EngineSetTexture TBI" );
#else

	if ( ( nTexHandle >= 0 ) &&
		 ( !msbEngineNoTexturesOverride ) )
	{
		if ( nTex == 0 )
		{
			if ( maTextureReferences[nTexHandle].boTextureHasAlpha == TRUE )
			{
				EngineShadowMapActivateTexture(TRUE);
			}
			else
			{
				EngineShadowMapActivateTexture( FALSE );
			}
		}
		else
		{
			mapxEngineLastSetTextures[nTex] = maTextureReferences[ nTexHandle ].pTexture;
		}
		mpEngineDevice->SetTexture( nTex, maTextureReferences[ nTexHandle ].pTexture );
	}
	else if ( nTex > 0 )
	{
		if ( mapxEngineLastSetTextures[nTex] != NULL )
		{
			mapxEngineLastSetTextures[nTex] = NULL;
			mpEngineDevice->SetTextureStageState( nTex, D3DTSS_COLOROP,   D3DTOP_DISABLE );
			mpEngineDevice->SetTextureStageState( nTex, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
		}	
	}
	else
	{
		mpEngineDevice->SetTexture( nTex, NULL );
	}
#endif
}


int	EngineLoadTextureThreadFunction( int nTextureSlotIndex )
{
int		nLoop = 1;
int		boReduceFilter = 1;
int		boMipFilter = 1;
FILE*	pFile;
int		nPreExtensionLength;
TEXTURE_REFERENCES*		pTextureReference = maTextureReferences + nTextureSlotIndex;
int		nFileSize = 0;

#ifdef TUD9
	mnLoadThreadBufferFilter = D3DX_FILTER_LINEAR;
	mnLoadThreadBufferMipFilter = D3DX_FILTER_LINEAR;
#endif
	mnLoadThreadBufferMipLevels = 0;
	mbLoadThreadDDSLoad = false;
	mbLoadThreadDDSSave = false;

	pTextureReference->nState = TEXTURE_STATE_LOADING;
	nPreExtensionLength = strlen(pTextureReference->szFilename)-3;
#ifdef TUD9
	if ( pTextureReference->nLoadFlags == 0xFF )
	{
		mnLoadThreadBufferFormat = EngineDXGetSurfaceFormat( D3DFMT_R8G8B8 );
	}
	else
	{
		mnLoadThreadBufferFormat = EngineDXGetSurfaceFormat( D3DFMT_UNKNOWN );
		if ( _stricmp( &pTextureReference->szFilename[nPreExtensionLength], "dds" ) == 0 )
		{
			mnLoadThreadBufferFormat = EngineDXGetSurfaceFormat( D3DFMT_UNKNOWN );
		}
		// Any tga's are loaded as 32 bit textures..
		else if ( _stricmp( &pTextureReference->szFilename[nPreExtensionLength], "tga" ) == 0 )
		{
			mnLoadThreadBufferFormat = EngineDXGetSurfaceFormat( D3DFMT_UNKNOWN );
		}
		// Bmps are automatically loaded as 16 bit with a 1bit alpha channel
		else if ( _stricmp( &pTextureReference->szFilename[nPreExtensionLength], "bmp" ) == 0 )
		{
			mnLoadThreadBufferFormat = EngineDXGetSurfaceFormat( D3DFMT_UNKNOWN );
			mbLoadThreadDDSSave = true;
		}
		else if ( _stricmp( &pTextureReference->szFilename[nPreExtensionLength], "jpg" ) == 0 )
		{
			mnLoadThreadBufferFormat = EngineDXGetSurfaceFormat( D3DFMT_R8G8B8 );
			mbLoadThreadDDSSave = true;
		}
	}
#endif

	EngineLoadTextureGetParams( pTextureReference->nLoadFlags, &mnLoadThreadBufferFilter, &mnLoadThreadBufferMipFilter, &mnLoadThreadBufferMipLevels );

	if ( (pTextureReference->nMode & NO_MIPMAPPING) != 0 ) 
	{
		mnLoadThreadBufferMipLevels = 1;
#ifdef TUD9
		mnLoadThreadBufferMipFilter = D3DX_FILTER_POINT;
		mnLoadThreadBufferFilter = D3DX_FILTER_POINT;
#endif
	}
	else if ( pTextureReference->nMode == POINT_FILTER )
	{
#ifdef TUD9
		mnLoadThreadBufferMipFilter = D3DX_FILTER_POINT;
		mnLoadThreadBufferFilter = D3DX_FILTER_POINT;
#endif
	}


	// If loading from an archive
	if ( pTextureReference->nArchiveHandle > 0 )
	{
	Archive*		pArchive = ArchiveGetFromHandle( pTextureReference->nArchiveHandle );

		if ( pArchive )
		{
		int		nFileHandle = pArchive->OpenFile( pTextureReference->szFilename );

			if ( nFileHandle > 0 )
			{
				nFileSize = pArchive->GetFileSize( nFileHandle );
				mpbLoadThreadBuffer = (byte*)SystemMalloc( nFileSize );

				if ( mpbLoadThreadBuffer != NULL )
				{
					mnLoadThreadBufferSize = nFileSize;
					pArchive->ReadFile( nFileHandle, mpbLoadThreadBuffer, nFileSize );
					pArchive->CloseFile( nFileHandle );
					return( 1 );
				}
				pArchive->CloseFile( nFileHandle );
			}
		}

		pTextureReference->nState = TEXTURE_STATE_LOAD_ERROR;
		return( -3 );
	}
	else		// Loading from a file using standard fs commands
	{
		pFile = fopen( pTextureReference->szFilename, "rb" );
		if ( !pFile )
		{
			pTextureReference->nState = TEXTURE_STATE_LOAD_ERROR;
			return( -1 );
		}
		nFileSize = SysGetFileSize( pFile );
		fclose( pFile );

		if ( ( mxEngineOptions.bDoCacheImagesToDDS == TRUE ) &&
			 ( pTextureReference->nMode != NO_DDS_CACHE ) )
		{
			if ( mbLoadThreadDDSSave == true )
			{
			FILE*	pOutFile;

				strcpy( mszDDSFilename, pTextureReference->szFilename );
				sprintf( &mszDDSFilename[nPreExtensionLength], "dds" );
				pOutFile = fopen( mszDDSFilename, "rb" );
				// If a dds file exists with this root name
				if ( pOutFile )
				{
					nFileSize = SysGetFileSize( pOutFile );
					fclose( pOutFile );
					mbLoadThreadDDSSave = false;
					mbLoadThreadDDSLoad = true;
#ifdef TUD9
					mnLoadThreadBufferFormat = EngineDXGetSurfaceFormat( D3DFMT_UNKNOWN );
#endif
				}	
				else // no dds file yet, so change the format to dxt1 when we load it (and save the dds out once loaded)
				{
#ifdef TUD9
					mnLoadThreadBufferFormat = EngineDXGetSurfaceFormat( D3DFMT_DXT1 );
#endif
				}
			}
		}
		else
		{
			mbLoadThreadDDSSave = false;
		}

		mpbLoadThreadBuffer = (byte*)SystemMalloc( nFileSize );

		if ( mpbLoadThreadBuffer != NULL )
		{
			mnLoadThreadBufferSize = nFileSize;

			if ( mbLoadThreadDDSLoad )
			{
				pFile = fopen( mszDDSFilename, "rb" );
			}
			else
			{
				pFile = fopen( pTextureReference->szFilename, "rb" );
			}

			if ( pFile )
			{
				fread( mpbLoadThreadBuffer, nFileSize, 1, pFile );
				fclose( pFile );
				return( 1 );
			}

			free( mpbLoadThreadBuffer );
			mpbLoadThreadBuffer = NULL;
		}
		pTextureReference->nState = TEXTURE_STATE_LOAD_ERROR;
		return( -3 );
	}
}


BOOL		EngineIsLoadingTextures( void )
{
	if ( mnTextureManagerAsyncLoadHandle != NOTFOUND )
	{
		return( TRUE );
	}
	return( FALSE );
}

long WINAPI EngineTextureLoadThread(long lParam)
{ 
	while( !mboTextureManagerKillThread )
	{
		if ( ( mnTextureManagerAsyncLoadHandle != NOTFOUND ) &&
			 ( mbLoadThreadBufferReady == false ) )
		{
			EngineLoadTextureThreadFunction(mnTextureManagerAsyncLoadHandle);
			mbLoadThreadBufferReady = true;
		}
		Sleep(10);
	}
	return( 0 );
}


int			msnTextureReductionMode = 0;

void		EngineSetTextureReductionMode( int nMode )
{
	msnTextureReductionMode = nMode;
}

int		EngineTextureLoadFromThreadBuffer( void )
{
LPGRAPHICSTEXTURE	pxTexture = NULL;
TEXTURE_REFERENCES*		pTextureReference = maTextureReferences + mnTextureManagerAsyncLoadHandle;
int	nRet = 0;

	if ( mpbLoadThreadBuffer )
	{
	_D3DPOOL	nPoolType = D3DPOOL_MANAGED;
#ifdef USE_D3DEX_INTERFACE
		nPoolType = D3DPOOL_DEFAULT;
#endif

#ifdef TUD11
		PANIC_IF( TRUE, "DX11 TextureLoadFromThreadBuffer TBI" );
#else
		if ( mbLoadThreadDDSLoad )
		{
			nRet = D3DXCreateTextureFromFileInMemory( mpEngineDevice, mpbLoadThreadBuffer, mnLoadThreadBufferSize, &pxTexture );
		}
		else
		{
		uint32		ulChromaKeyCol = 0xFF0000FF;
		IGRAPHICSFORMAT	d3dtextureFormat = EngineDXGetGraphicsFormat( mnLoadThreadBufferFormat );

			if ( pTextureReference->nMode & NO_CHROMAKEY )
			{
				ulChromaKeyCol = 0;
			}
			if ( pTextureReference->nMode & FORCE_A8R8G8B8)
			{
				d3dtextureFormat = D3DFMT_A8R8G8B8;
			}


			nRet = D3DXCreateTextureFromFileInMemoryEx( mpEngineDevice, mpbLoadThreadBuffer, mnLoadThreadBufferSize,
													0,0,mnLoadThreadBufferMipLevels,0, d3dtextureFormat, nPoolType,mnLoadThreadBufferFilter, mnLoadThreadBufferMipFilter,
													ulChromaKeyCol, NULL, NULL,
													&pxTexture );

			if ( pTextureReference->nMode & REMOVE_ALPHA )
			{
			int			nPitch;
			uint32*		pulColours;
			BYTE*		pbRow;
			int			nImageW;
			int			nImageH;
			int			nLoopY;
			int			nLoopX;
			D3DSURFACE_DESC		xSurface;
			D3DLOCKED_RECT	xLockedRectDest;

				pxTexture->GetLevelDesc( 0, &xSurface );
				nImageW = xSurface.Width;
				nImageH = xSurface.Height;
				pxTexture->LockRect( 0, &xLockedRectDest, NULL, 0 );//&xDestRect, 0 );
				nPitch = xLockedRectDest.Pitch;
				pbRow = (BYTE*)xLockedRectDest.pBits;

				for ( nLoopY = 0; nLoopY < nImageH; nLoopY++ )
				{
					pulColours = (uint32*)pbRow;
					for ( nLoopX = 0; nLoopX < nImageW; nLoopX++ )
					{
						*pulColours = ((*pulColours) & 0x00FFFFFF);
						pulColours++;
					}
					pbRow += nPitch;
				}
				pxTexture->UnlockRect(0);
			}

			if ( msnTextureReductionMode == 1 )
			{
				if (!FAILED(nRet))
				{
					D3DSURFACE_DESC		xDesc;

					pxTexture->GetLevelDesc(0, &xDesc);
					if ((xDesc.Width > 128) ||
						(xDesc.Height > 128))
					{
						LPGRAPHICSTEXTURE	pxTextureResized = NULL;

						nRet = D3DXCreateTextureFromFileInMemoryEx(mpEngineDevice, mpbLoadThreadBuffer, mnLoadThreadBufferSize,
							xDesc.Width / 2, xDesc.Height / 2, mnLoadThreadBufferMipLevels, 0, EngineDXGetGraphicsFormat(mnLoadThreadBufferFormat), nPoolType, mnLoadThreadBufferFilter, mnLoadThreadBufferMipFilter,
							0xFF0000FF, NULL, NULL,
							&pxTextureResized);

						if (!FAILED(nRet))
						{
							pxTexture->Release();
							pxTexture = pxTextureResized;
						}
					}
				}
			}
		}
#endif
		if( FAILED( nRet ) )
		{
		char	acString[256];

			sprintf( acString, "%s -", pTextureReference->szFilename);
			if ( nRet == D3DXERR_INVALIDDATA )
			{
				sprintf( acString, "%sFile data corrupted.", acString );
			}
		}
		
		if( FAILED( nRet ) )
		{
			// if we just tried to load it as a 32 bit and failed.. try again as a 16 bit
			if ( mnLoadThreadBufferFormat != SURFACEFORMAT_A1R5G5B5 )
			{
				mnLoadThreadBufferFormat = SURFACEFORMAT_A1R5G5B5;
			}
			else	// else if format was 1555 and it failed, try 32 bit
			{
				mnLoadThreadBufferFormat = SURFACEFORMAT_A8R8G8B8;
			}
#ifdef TUD11
			PANIC_IF( TRUE, "DX11 TextureLoadFromThreadBuffer TBI" );
#else
			mnLoadThreadBufferMipFilter |= D3DX_FILTER_MIRROR;
			mnLoadThreadBufferFilter |= D3DX_FILTER_MIRROR;

			mnLoadThreadBufferMipFilter |= D3DX_FILTER_DITHER;
			mnLoadThreadBufferFilter |= D3DX_FILTER_DITHER;
		    mpEngineDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_MIRROR );
			mpEngineDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_MIRROR );
			mpEngineDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);	
			mpEngineDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT);	

			nRet = D3DXCreateTextureFromFileInMemoryEx( mpEngineDevice, mpbLoadThreadBuffer,mnLoadThreadBufferSize,
												0,0,mnLoadThreadBufferMipLevels,0, EngineDXGetGraphicsFormat( mnLoadThreadBufferFormat ), nPoolType,mnLoadThreadBufferFilter, mnLoadThreadBufferMipFilter,
												0xFF0000FF, NULL, NULL,
												&pxTexture );
#endif


			if( FAILED( nRet ) )
			{
//#ifdef DISPLAY_TEXTURE_LOAD_ERRORS
#if 1
				char	acString[256];

				sprintf( acString, "%s -", pTextureReference->szFilename);

					switch( nRet )
					{
					case E_OUTOFMEMORY:
						sprintf( acString, "%sNot enough system memory available.", acString );
						break;
					case D3DXERR_INVALIDDATA:
						sprintf( acString, "%sFile data corrupted.", acString );
						break;
					case D3DERR_OUTOFVIDEOMEMORY:
						sprintf( acString, "%sNot enough video memory available.", acString );
						break;
					case D3DERR_NOTAVAILABLE:
						sprintf( acString, "%sMode not available.\nYou may need to update your video card drivers.", acString );
						break;
					default:
						sprintf( acString, "%sCouldnt load file", acString );
						break;
					}
					SysUserPrint( 0, acString );
#endif
				pxTexture = NULL;
				pTextureReference->nState = TEXTURE_STATE_LOAD_ERROR;
				return( -2 );
			}
		}
		else if ( mbLoadThreadDDSSave )
		{
#ifdef TUD11
			PANIC_IF( TRUE, "DX11 TextureLoadFromThreadBuffer TBI" );
#else
			D3DXSaveTextureToFile( mszDDSFilename,D3DXIFF_DDS,pxTexture,NULL );
#endif
		}

		if ( ( pTextureReference->szFilename ) &&
			 ( pTextureReference->szFilename[0] != 0 ) )
		{
		const char*		szExtension = &pTextureReference->szFilename[ strlen(pTextureReference->szFilename)-3 ];
		
			if ( ( stricmp( szExtension, "PNG" ) == 0 ) ||
				 ( stricmp( szExtension, "tga" ) == 0 ) )
			{
				pTextureReference->boTextureHasAlpha = TRUE;
			}
		}
		pTextureReference->pTexture = pxTexture;
		pTextureReference->nState = TEXTURE_STATE_LOADED;
	}
	else
	{
		// Musta been a load error...
	}
	return( 0 );
}

void	EngineTextureManagerUpdateDX( void )
{
int		nLoop;

	if ( mbLoadThreadBufferReady == true )
	{
		EngineTextureLoadFromThreadBuffer();
		if ( mpbLoadThreadBuffer )
		{
			free( mpbLoadThreadBuffer );
			mpbLoadThreadBuffer = NULL;
		}
		mbLoadThreadBufferReady = false;
		mnTextureManagerAsyncLoadHandle = NOTFOUND;
	}

	// if not loading something, scan for things to load
	if ( mnTextureManagerAsyncLoadHandle == NOTFOUND )
	{
		if ( mboPendingLoads )
		{
			for ( nLoop = 1; nLoop < mnHighestTextureSlotUsed; nLoop++ )
			{
				if ( maTextureReferences[nLoop].nState == TEXTURE_STATE_REQUESTED )
				{
					mnTextureManagerAsyncLoadHandle = nLoop;
					return;
				}
			}
			mboPendingLoads	= FALSE;
		}
	}

}

void	EngineTextureManagerInitDX( void )
{
uint32 iID;

	if ( mhTextureLoadThread == 0 )
	{
		ZeroMemory( maTextureReferences, sizeof( TEXTURE_REFERENCES ) * MAX_TEXTURES_IN_MANAGER );
		// Start a thread that will load the textures
		mhTextureLoadThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)EngineTextureLoadThread,0,0,(LPDWORD)&iID);
	}
}


void	EngineTextureManagerFreeDX( BOOL boCompletely )
{
	if ( boCompletely )
	{
		mboTextureManagerKillThread = TRUE;
		if ( mnTextureManagerAsyncLoadHandle != NOTFOUND )
		{
			Sleep(100);
		}
		// TODO(?) - Free all textures we've got managed
		CloseHandle( mhTextureLoadThread );
		mhTextureLoadThread = 0;
	}
}


void	EngineExportTexture( TEXTURE_HANDLE nTexHandle, const char* szFilename, int nMode )
{
#ifdef TUD11
	PANIC_IF( TRUE, "DX11 EngineExportTexture TBI" );
#else
LPGRAPHICSTEXTURE	pTexture = maTextureReferences[ nTexHandle ].pTexture;
HRESULT		ret;
//	D3DXIFF_BMP
//	ret = D3DXSaveTextureToFile( (LPCSTR)szFilename, D3DXIFF_JPG, pTexture, NULL );

	switch( nMode )
	{
	case 3:
		ret = D3DXSaveTextureToFile( (LPCSTR)szFilename,D3DXIFF_PNG,pTexture,NULL );
		break;
	case 2:
		ret = D3DXSaveTextureToFile( (LPCSTR)szFilename,D3DXIFF_JPG,pTexture,NULL );
		break;
	case 1:
		ret = D3DXSaveTextureToFile( (LPCSTR)szFilename,D3DXIFF_DDS,pTexture,NULL );
		break;
	case 0:
	default:
		ret = D3DXSaveTextureToFile( (LPCSTR)szFilename, D3DXIFF_BMP, pTexture, NULL );
		break;
	}
#endif

#ifndef TOOL
/*	if ( ret != D3D_OK )
	{
	char	acString[256];
		sprintf( acString, "Error exporting image file %s", szFilename );
		PrintConsoleCR(acString, COL_WARNING );
	}
*/
#endif
}

int		EngineTextureGetWidth( TEXTURE_HANDLE nTexHandle )
{
#ifdef TUD11
	PANIC_IF( TRUE, "DX11 EngineTextureGetWidth TBI" );
#else	
LPGRAPHICSTEXTURE pxBaseTexture;
D3DSURFACE_DESC		xSurface;

	if ( nTexHandle >= 0 )
	{
		pxBaseTexture = maTextureReferences[ nTexHandle ].pTexture;
		if ( pxBaseTexture )
		{
			pxBaseTexture->GetLevelDesc( 0, &xSurface );
			return( xSurface.Width );
		}
	}
#endif
	return( 0 );
}

int		EngineTextureGetHeight( TEXTURE_HANDLE nTexHandle )
{
#ifdef TUD11
	PANIC_IF( TRUE, "DX11 EngineTextureGetHeight TBI" );
#else	
LPGRAPHICSTEXTURE pxBaseTexture;
D3DSURFACE_DESC		xSurface;

	if ( nTexHandle >= 0 )
	{
		pxBaseTexture = maTextureReferences[ nTexHandle ].pTexture;
		if ( pxBaseTexture )
		{
			pxBaseTexture->GetLevelDesc( 0, &xSurface );
			return( xSurface.Height );
		}
	}
#endif
	return( 0 );
}

void	EngineProcessTexture( TEXTURE_HANDLE nTexHandle, int nProcessMode )
{
#ifdef TUD11
	PANIC_IF( TRUE, "DX11 EngineProcessTexture TBI" );
#else	
LPGRAPHICSTEXTURE pxBaseTexture;
LPGRAPHICSTEXTURE pxNewTexture;
D3DLOCKED_RECT	xLockedRectDest;
D3DLOCKED_RECT	xLockedRectSrc;
D3DSURFACE_DESC		xSurface;
byte*		pbDestination;
byte*		pbSrc;
ushort*		puwSrc;
uint32*		pulSrc;
uint32*		pulDest;
ushort*		puwDest;
byte*		pbDest;
int			nX;
int			nY;
int			nWidth;
int			nHeight;
byte		bR, bG, bB;
int			nAlpha;
int			nFormat;
int			nSourceFormat;

	if ( nTexHandle >= 0 )
	{
		pxBaseTexture = maTextureReferences[ nTexHandle ].pTexture;

		if ( maTextureReferences[nTexHandle].nState != TEXTURE_STATE_LOADED )
		{
			PANIC_IF( TRUE, "Error Code #4316 - Cannot process texture as it is not loaded yet" );
			return;
		}

		pxBaseTexture->GetLevelDesc( 0, &xSurface );
		nWidth = xSurface.Width;
		nHeight = xSurface.Height;
		nSourceFormat = xSurface.Format;

		nFormat = D3DFMT_A8R8G8B8;
		mpEngineDevice->CreateTexture( nWidth, nHeight, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &pxNewTexture, NULL );
		if ( pxNewTexture == NULL )
		{
			nFormat = D3DFMT_A8R3G3B2;
			mpEngineDevice->CreateTexture( nWidth, nHeight, 1, 0, D3DFMT_A8R3G3B2, D3DPOOL_MANAGED, &pxNewTexture, NULL );
		
			if ( pxNewTexture == NULL )
			{
				nFormat = D3DFMT_A4R4G4B4;
				mpEngineDevice->CreateTexture( nWidth, nHeight, 1, 0, D3DFMT_A4R4G4B4, D3DPOOL_MANAGED, &pxNewTexture, NULL );
			
				if ( pxNewTexture == NULL )
				{
					nFormat = D3DFMT_A8;
					mpEngineDevice->CreateTexture( nWidth, nHeight, 1, 0, D3DFMT_A8, D3DPOOL_MANAGED, &pxNewTexture, NULL );
			
					if ( pxNewTexture == NULL )
					{
						PANIC_IF( TRUE, "Error Code #4315 - Alphablended smoke texture cannot be created. Please report this to the bug report forums." );
						return;
					}
				}
			}
		}
		pxBaseTexture->LockRect( 0, &xLockedRectSrc, NULL, 0 );//&xDestRect, 0 );
		pxNewTexture->LockRect( 0, &xLockedRectDest, NULL, 0 );//&xDestRect, 0 );

		if ( ( xLockedRectDest.pBits != NULL ) &&
			 ( xLockedRectSrc.pBits != NULL ) )
		{	
			pbSrc = (byte*)( xLockedRectSrc.pBits );
			pbDestination = (byte*)( xLockedRectDest.pBits );

			for ( nY = 0; nY < nHeight; nY++ )
			{
				puwSrc = (ushort*)( pbSrc );
				pulSrc = (uint32*)( pbSrc );
				pulDest = (uint32*)( pbDestination );
				puwDest = (ushort*)( pbDestination );
				pbDest = pbDestination;
				for ( nX = 0; nX < nWidth; nX++ )
				{
					if ( nSourceFormat == D3DFMT_A1R5G5B5 )
					{
						bR = ( ( *(puwSrc) ) >> 10 ) & 0x1F;
						bG = ( ( *(puwSrc) ) >> 5 ) & 0x1F;
						bB = ( ( *(puwSrc) ) ) & 0x1F;
						bR <<= 3;
						bG <<= 3;
						bB <<= 3;
					}
					else if ( nSourceFormat == D3DFMT_A8R8G8B8 )
					{
						bR = ( *pulSrc >> 16 ) & 0xFF;
						bG = ( *pulSrc >> 8 ) & 0xFF;
						bB = ( *pulSrc ) & 0xFF;
					}
					else
					{
						PANIC_IF(TRUE,"Cannot process texture - unsupported format for processing" );
						return;
					}

					nAlpha = (bR + bG + bB) / 3;

					switch( nFormat )
					{
					case D3DFMT_A8R8G8B8:
						*(pulDest) = (nAlpha << 24) | (bR << 16 ) | (bG << 8) | (bB);
						break;
					case D3DFMT_A8:
						*(pbDest) = (byte)(nAlpha);
						break;
					case D3DFMT_A4R4G4B4:
						*(puwDest) = ((nAlpha>>4) << 12) | ((bR>>4) << 8 ) | ((bG>>4) << 4) | (bB>>4);
						break;
					case D3DFMT_A8R3G3B2:
						*(puwDest) = (nAlpha << 8) | ((bR>>5) << 5 ) | ((bG>>5) << 2) | (bB>>6);
						break;
					}

					puwSrc++;
					pulSrc++;
					pulDest++;
					puwDest++;
					pbDest++;

				} // FOR X < MAPSIZE

				pbDestination += xLockedRectDest.Pitch;
				pbSrc += xLockedRectSrc.Pitch;		
			}

			pxBaseTexture->UnlockRect( 0 );
			pxNewTexture->UnlockRect( 0);
		}

		pxBaseTexture->Release();

		maTextureReferences[ nTexHandle ].pTexture = pxNewTexture;
	}
#endif
}

