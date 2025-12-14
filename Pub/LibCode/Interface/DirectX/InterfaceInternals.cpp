
#include "InterfaceInternalsDX.h"

#include <stdio.h>

#include <StandardDef.h>
#include <Interface.h>
#include <Engine.h>

#include "DX9Device/DX9Device.h"
#include "../Common/Overlays/Overlays.h"
#include "Jpeg/ShowJpeg.h"

#include "Image.h"

#include "../Win32/Interface-Win32.h"
#include "../LibCode/CodeUtil/Archive.h"
#include "../Common/Overlays/TexturedOverlays.h"
#include "../Common/InterfaceUtil.h"
#include "../Common/InterfaceCommon.h"
#include "../Common/InterfaceInstance.h"
#include "../Common/Font/FontCommon.h"

//------------------------------------------------------------------------------------------------

BOOL	mboInterfaceInitialised = FALSE;
float	mfMipMapBias = -1.0f;//-0.25f;
int		mnOptionTextureFiltering = TRUE;
int		mnOptionBackBuffer = TRUE;
int		mnOptionVsync = TRUE;
int		mnOptionOldStartup = TRUE;
int		mnOptionMinimumSurfaceRes = TRUE;
int		mnOptionFogMode = 0;
int		mnFullscreenAntialias = 0;

int		mnInterfaceDrawWidth = 0;
int		mnInterfaceDrawHeight = 0;
int		mnInterfaceDrawX = 0;
int		mnInterfaceDrawY = 0;
#ifdef TUD11
int		mnMinFilter = 0;
int		mnMagFilter = 0;
int		mnMipFilter = 0;
#else
int		mnMinFilter = D3DTEXF_LINEAR;
int		mnMagFilter = D3DTEXF_LINEAR;
int		mnMipFilter = D3DTEXF_LINEAR;
#endif

BOOL	mboRenderLineAlpha = TRUE;

char*	macInterfaceVersionString = "Intf 1.3";
BOOL	mboSmall = FALSE;

InterfaceDrawCallback mfnInterfaceOnDrawCallback = NULL;


//------------------------------------------------------------------------------------------------
void	InterfaceGetTextureLoadParams(  const char* szFilename, int boReduceFilter, int boMipFilter, int* pnFormat, int* pnFilter, int* pnMipFilter, int* pnMipLevels )
{
#ifdef TUD9
int		nFilter = D3DX_FILTER_NONE;
int		nMipFilter = D3DX_FILTER_NONE;
int		nMipLevels = 1;
int		nFormat = D3DFMT_UNKNOWN;

/*
	if ( ( szFilename ) &&
		 ( strlen( szFilename ) > 4 ) )
	{
		// Any tga's are loaded as 32 bit textures..
		if ( _stricmp( &szFilename[strlen(szFilename)-3], "tga" ) == 0 )
		{
			nFormat = D3DFMT_A8R8G8B8;
		}
		// Bmps are automatically loaded as 16 bit with a 1bit alpha channel
		else if ( _stricmp( &szFilename[strlen(szFilename)-3], "bmp" ) == 0 )
		{
			nFormat = D3DFMT_A1R5G5B5;
		}
	}
*/
	switch ( boReduceFilter )
	{
	case 0:
		break;
	case 2:
		nFilter = D3DX_FILTER_POINT;
		break;
	case 1:
	default:
		nFilter = D3DX_FILTER_LINEAR;
		break;
	}

	switch ( boMipFilter )
	{
	case 0xFF:
		nFormat = D3DFMT_A1R5G5B5;
		nMipLevels = 1;
		break;
	case 0:
     	nMipFilter = D3DX_FILTER_TRIANGLE;
		nMipLevels = D3DX_DEFAULT;
		break;
	case 1:
		nMipLevels = 1;
		break;
	case 2:
		 nMipFilter = D3DX_FILTER_TRIANGLE;
		nMipLevels = D3DX_DEFAULT;
		break;
	case 3:
		 nMipFilter = D3DX_FILTER_LINEAR;
		nMipLevels = D3DX_DEFAULT;
		break;
	case 4:
		 nMipFilter = D3DX_FILTER_POINT;
		nMipLevels = D3DX_DEFAULT;
		break;
	default:
		 nMipFilter = D3DX_FILTER_BOX;
		 nMipLevels = D3DX_DEFAULT;
		 break;
	}

	*pnFormat = nFormat;
	*pnFilter = nFilter;
	*pnMipFilter = nMipFilter;
	*pnMipLevels = nMipLevels;
#else
	*pnFormat = 0;
	*pnFilter = 0;
	*pnMipFilter = 0;
	*pnMipLevels = 0;
#endif
}

void InterfaceSetSmall( BOOL boSmallFlag )
{
	mboSmall = boSmallFlag;
}

/***************************************************************************
 * Function    : InterfaceSetInitialSmall
 ***************************************************************************/
INTERFACE_API void InterfaceSetInitialSmall( BOOL boSmallFlag )
{
	mboSmall = boSmallFlag;
}

/***************************************************************************
 * Function    : InterfaceIsSmall
 ***************************************************************************/
INTERFACE_API BOOL InterfaceIsSmall ( void )
{
	return( mboSmall );
}



INTERFACE_API void InterfaceSetFilteringModes( int nFilterMode )
{
#ifdef TUD9
	switch( nFilterMode )
	{
	case 0:
	default:
		mnMinFilter = D3DTEXF_LINEAR;
		mnMagFilter = D3DTEXF_LINEAR;
		mnMipFilter = D3DTEXF_POINT;
		break;
	case 1:
		mnMinFilter = D3DTEXF_LINEAR;
		mnMagFilter = D3DTEXF_LINEAR;
		mnMipFilter = D3DTEXF_POINT;
		break;
	case 2:
		mnMinFilter = D3DTEXF_LINEAR;
		mnMagFilter = D3DTEXF_LINEAR;
		mnMipFilter = D3DTEXF_LINEAR;
		break;
	case 3:
		mnMinFilter = D3DTEXF_POINT;
		mnMagFilter = D3DTEXF_POINT;
		mnMipFilter = D3DTEXF_POINT;
		break;
	case 4:
		mnMinFilter = D3DX_FILTER_TRIANGLE;
		mnMagFilter = D3DX_FILTER_TRIANGLE;
		mnMipFilter = D3DTEXF_LINEAR;
		break;
	case 5:
		mnMinFilter = D3DTEXF_LINEAR;
		mnMagFilter = D3DTEXF_LINEAR;
		mnMipFilter = D3DTEXF_LINEAR;
		break;	
	case 6:
		mnMinFilter = D3DX_FILTER_TRIANGLE|D3DX_FILTER_DITHER;
		mnMagFilter = D3DX_FILTER_TRIANGLE|D3DX_FILTER_DITHER;
		mnMipFilter = D3DTEXF_LINEAR;
		break;	
	}
#endif

}

INTERFACE_API void InterfaceSetMipMapBias( float fParam )
{
	mfMipMapBias = fParam;
}




INTERFACE_API void	InterfaceSetGlobalParam( INTF_DRAW_PARAM nParam, int nState )
{
	switch( nParam )
	{
	case INTF_LINES_ALPHA:
		if ( nState == 0 ) mboRenderLineAlpha = FALSE; else mboRenderLineAlpha = TRUE;
		break;
	case INTF_TEXTURE_FILTERING:
		if ( nState == 0 ) InterfaceTurnOffTextureFiltering(); else InterfaceTurnOnTextureFiltering(nState);
		break;
//	case INTF_ANISOTROPIC:
//		if ( nState == 0 ) mboAnistropic = FALSE; else mboAnistropic = TRUE;	
//		break;
	case INTF_TEXTURERECT_LIMIT:
		InterfaceTexturedOverlaysSetLimit( nState );
		break;
	}
}

#ifdef TUD9
D3DFORMAT		InterfaceInternalDX9GetFormat( eInterfaceTextureFormat format )
{
D3DFORMAT dx9Format;

	switch( format )
	{
	case FORMAT_A8R8G8B8:
	default:
		dx9Format = D3DFMT_A8R8G8B8;
		break;
	case FORMAT_A8R3G3B2:
		dx9Format = D3DFMT_A8R3G3B2;
		break;
	case FORMAT_A4R4G4B4:
		dx9Format = D3DFMT_A4R4G4B4;
		break;
	case FORMAT_A8:
		dx9Format = D3DFMT_A8;
		break;
	case FORMAT_DISPLAY:
		{
		D3DDISPLAYMODE	xDisplayMode;
	
			mpInterfaceD3DDevice->GetDisplayMode( 0, &xDisplayMode );
			dx9Format = xDisplayMode.Format;
		}
		break;
	}
	return( dx9Format );
}
#endif

LPGRAPHICSTEXTURE InterfaceInternalsDX::LoadTextureDX( const char* szFilename, int boReduceFilter, int boMipFilter )
{
int	nRet = 0;
LPGRAPHICSTEXTURE	pxTexture = NULL;
int		nFilter = D3DX_FILTER_NONE;
int		nMipFilter = D3DX_FILTER_NONE;
int		nMipLevels = 1;
int		nFormat = D3DFMT_UNKNOWN;

	InterfaceGetTextureLoadParams( szFilename, boReduceFilter, boMipFilter, &nFormat, &nFilter, &nMipFilter, &nMipLevels );
/*	char	acString[256];
	sprintf(acString,"Loading texture %s\n", szFilename );
	PrintConsoleCR(acString,COL_WARNING);
*/
	nRet = D3DXCreateTextureFromFileEx( mpInterfaceD3DDevice, szFilename,
										0,0,nMipLevels,0,(D3DFORMAT)nFormat, D3DPOOL_DEFAULT,nFilter, nMipFilter,
										0xFF0000FF, NULL, NULL,
										&pxTexture );

	if( FAILED( nRet ) )
	{
	int		nNewRet = 0;

		// If 32 bit load failed, try to load em as 16 bit instead
		if ( nFormat == D3DFMT_A8R8G8B8 )
		{
			nNewRet = D3DXCreateTextureFromFileEx( mpInterfaceD3DDevice, szFilename,
											0,0,nMipLevels,0,D3DFMT_A1R5G5B5, D3DPOOL_DEFAULT,nFilter, nMipFilter,
											0xFF0000FF, NULL, NULL,
											&pxTexture );
			if( FAILED( nNewRet ) )
			{
				InterfaceTextureLoadError( nNewRet,szFilename );
				pxTexture = NULL;
			}
		}
		else if ( nFormat == D3DFMT_A1R5G5B5 )
		{
			nNewRet = D3DXCreateTextureFromFileEx( mpInterfaceD3DDevice, szFilename,
											0,0,nMipLevels,0,D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,nFilter, nMipFilter,
											0xFF0000FF, NULL, NULL,
											&pxTexture );
			if( FAILED( nNewRet ) )
			{
				InterfaceTextureLoadError( nNewRet,szFilename );
				pxTexture = NULL;
			}
		}
		else
		{
			InterfaceTextureLoadError( nNewRet,szFilename );
			pxTexture = NULL;
		}
	}
	
	return( pxTexture );


}

 
HRESULT			InterfaceInternalsDX::CreateVertexBuffer(  unsigned int Length, unsigned int Usage, unsigned int FVF, IGRAPHICSVERTEXBUFFER** ppVertexBufferOut )
{
#ifdef TUD11
	PANIC_IF( TRUE, "InterfaceInternalDXCreateVertexBuffer TBI" );
#else
	return( mpInterfaceD3DDevice->CreateVertexBuffer( Length, Usage, FVF, D3DPOOL_DEFAULT, ppVertexBufferOut, NULL ) );
#endif
}

void	InterfaceInternalsDX::CreateTexture( int width, int height, int levels, int mode, eInterfaceTextureFormat format, LPGRAPHICSTEXTURE* ppTexture )
{
#ifdef TUD11
	PANIC_IF( TRUE, "InterfaceInternalDXCreateTexture TBI" );
#else
	D3DFORMAT dx9Format = InterfaceInternalDX9GetFormat( format );
	*ppTexture = NULL;
#ifdef USE_D3DEX_INTERFACE
	mpInterfaceD3DDevice->CreateTexture( width, height, levels, mode, dx9Format, D3DPOOL_DEFAULT, ppTexture, NULL );
#else
	mpInterfaceD3DDevice->CreateTexture( width, height, levels, mode, dx9Format, D3DPOOL_MANAGED, ppTexture, NULL );
#endif
#endif
}


void	InterfaceInternalsDX::SetStreamSource( unsigned int StreamNumber, IGRAPHICSVERTEXBUFFER *pStreamData, unsigned int OffsetInBytes, unsigned int Stride )
{
#ifdef TUD11
	PANIC_IF( TRUE, "InterfaceInternalDXCreateVertexBuffer TBI" );
#else
	mpInterfaceD3DDevice->SetStreamSource( StreamNumber, pStreamData, OffsetInBytes, Stride );
#endif

};

//	LPGRAPHICSTEXTURE	LoadTextureFromArchive( const char* szFilename, int boReduceFilter, int boMipFilter, int nArchiveHandle );

void	InterfaceInternalDXCreateTexture( int width, int height, int levels, int mode, eInterfaceTextureFormat format, LPGRAPHICSTEXTURE* ppTexture )
{
	InterfaceInstanceMain()->mpInterfaceInternals->CreateTexture( width, height, levels, mode, format, ppTexture );
}


void	InterfaceInternalDXSetStreamSource( unsigned int StreamNumber, IGRAPHICSVERTEXBUFFER *pStreamData, unsigned int OffsetInBytes, unsigned int Stride )
{
	InterfaceInstanceMain()->mpInterfaceInternals->SetStreamSource( StreamNumber, pStreamData, OffsetInBytes, Stride );
}


HRESULT	InterfaceInternalDXCreateVertexBuffer( unsigned int Length, unsigned int Usage, unsigned int FVF, IGRAPHICSVERTEXBUFFER** ppVertexBufferOut )
{
	return( InterfaceInstanceMain()->mpInterfaceInternals->CreateVertexBuffer( Length, Usage, FVF, ppVertexBufferOut ) );
}

HRESULT	InterfaceInternalDXCreateImageSurface( unsigned int width, unsigned int height, eInterfaceTextureFormat format, IGRAPHICSSURFACE** ppSurface )
{
#ifdef TUD11
	PANIC_IF( TRUE, "InterfaceInternalDXCreateImageSurface TBI" );

#else
	D3DFORMAT dx9Format = InterfaceInternalDX9GetFormat( format );

	return( mpInterfaceD3DDevice->CreateOffscreenPlainSurface( width, height, dx9Format, D3DPOOL_DEFAULT, ppSurface, NULL ) );
#endif
}





INTERFACE_API LPGRAPHICSDEVICE	InterfaceGetD3DDevice( void )
{
	return( mpInterfaceD3DDevice );
} 

int		InterfaceGetDrawRegionWidth( void )
{
	return(	mnInterfaceDrawWidth );

}

int		InterfaceGetDrawRegionHeight( void )
{
	return(	mnInterfaceDrawHeight );

}
	 
INTERFACE_API void	InterfaceSetDrawRegion( int nX, int nY, int nWidth, int nHeight )
{
	mnInterfaceDrawX = nX;
	mnInterfaceDrawY = nY;

	mnInterfaceDrawWidth = nWidth;
	mnInterfaceDrawHeight = nHeight;
}

BOOL	mboDontShowAnyMoreLoadErrors = FALSE;

INTERFACE_API void InterfaceTextureLoadError( int nRet, const char* szFilename )
{
char	acString[512];

//return;
	if ( mboDontShowAnyMoreLoadErrors == FALSE )
	{
		sprintf( acString, "**** Texture load error ****\n%s\n", szFilename);
		switch( nRet )
		{
		case E_OUTOFMEMORY:
			sprintf( acString, "%sNot enough system memory available.", acString );
			break;
		case D3DXERR_INVALIDDATA:
			sprintf( acString, "%sFile does not exist or is corrupted.", acString );
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
		
		if ( ( InterfaceIsFullscreen() == TRUE ) &&
		     ( mpInterfaceD3DDevice != NULL ) )
		{
			if ( InterfaceShowFullscreenPanic( acString ) == TRUE )
			{
				mboDontShowAnyMoreLoadErrors = TRUE;
			}
		}
		else
		{
			nRet = MessageBox( mhwndInterfaceMain, acString, "Error message", MB_ABORTRETRYIGNORE | MB_ICONEXCLAMATION);
			if ( nRet == IDIGNORE )
			{
				mboDontShowAnyMoreLoadErrors = TRUE;
			}
			if ( nRet == IDABORT )
			{
				PostQuitMessage( 0 );
			}
		}
	}
}



INTERFACE_API LPGRAPHICSTEXTURE InterfaceLoadTextureDXFromFileInMem( const char* szFilename, byte* pbMem, int nMemSize, int boReduceFilter, int boMipFilter )
{
int	nRet;
LPGRAPHICSTEXTURE	pxTexture = NULL;
int		nFilter = D3DX_FILTER_NONE;
int		nMipFilter = D3DX_FILTER_NONE;
int		nMipLevels = 1;
int		nFormat = D3DFMT_R8G8B8;

	InterfaceGetTextureLoadParams( szFilename, boReduceFilter, boMipFilter, &nFormat, &nFilter, &nMipFilter, &nMipLevels );

#ifdef USE_D3DEX_INTERFACE
	nRet = D3DXCreateTextureFromFileInMemoryEx( mpInterfaceD3DDevice, pbMem, nMemSize,0,0,nMipLevels,0,(D3DFORMAT)nFormat, D3DPOOL_DEFAULT, nFilter,nMipFilter, 0xFF0000FF, NULL,NULL, &pxTexture );
#else
	nRet = D3DXCreateTextureFromFileInMemoryEx( mpInterfaceD3DDevice, pbMem, nMemSize,0,0,nMipLevels,0,(D3DFORMAT)nFormat, D3DPOOL_MANAGED, nFilter,nMipFilter, 0xFF0000FF, NULL,NULL, &pxTexture );
#endif
	if( FAILED( nRet ) )
	{
		pxTexture = NULL;
	}
	return( pxTexture );
}

INTERFACE_API LPGRAPHICSTEXTURE InterfaceLoadTextureFromArchiveDX( const char* szFilename, int boReduceFilter, int boMipFilter, int nArchiveHandle )
{
Archive*		pArchive = ArchiveGetFromHandle( nArchiveHandle );
byte*	pbFileInMem;
int		nFileHandle;
int		nFileSize;

	nFileHandle = pArchive->OpenFile( szFilename );

	if ( nFileHandle > 0 )
	{
		// Get the size of the tex
		nFileSize = pArchive->GetFileSize( nFileHandle );

		pbFileInMem = (byte*)( SystemMalloc( nFileSize ) );
		if ( pbFileInMem != NULL )
		{
			pArchive->ReadFile( nFileHandle, pbFileInMem, nFileSize );
			pArchive->CloseFile( nFileHandle );

			return( InterfaceLoadTextureDXFromFileInMem( szFilename, pbFileInMem, nFileSize, boReduceFilter, boMipFilter ) );
		}
	}
	return( NULL );
}

INTERFACE_API LPGRAPHICSTEXTURE InterfaceLoadTextureDX( const char* szFilename, int boReduceFilter, int boMipFilter )
{
	return( InterfaceInstanceMain()->mpInterfaceInternals->LoadTextureDX( szFilename, boReduceFilter, boMipFilter));
}







/***************************************************************************
 * Function    : InterfaceGetOption
 * Params      : 
 * Returns     :
 * Description : 
 ***************************************************************************/
INTERFACE_API int InterfaceGetOption( int nOptionNum )
{
	switch( nOptionNum )
	{
	case TEXTURE_FILTERING:
		return( mnOptionTextureFiltering );
		break;
	case BACK_BUFFER:
		return( mnOptionBackBuffer );
		break;
	case VSYNC:
		return( mnOptionVsync );
		break;
	case OLD_STARTUP:
		return( mnOptionOldStartup );
		break;
	case MINIMUM_SURFACE_RES:
		return( mnOptionMinimumSurfaceRes );
		break;
	case FOG_MODE:
		return( mnOptionFogMode );
		break;
	case FSAA:
		return( mnFullscreenAntialias );
		break;
	}

	return( 0 );
}

/***************************************************************************
 * Function    : InterfaceSetOption
 * Params      : 
 * Returns     :
 * Description : 
 ***************************************************************************/
INTERFACE_API void InterfaceSetOption( int nOptionNum, int nValue )
{
	if ( nOptionNum >= MAX_OPTIONS )
	{
		return;
	}

	switch( nOptionNum )
	{
	case TEXTURE_FILTERING:
		mnOptionTextureFiltering = nValue;
		break;
	case BACK_BUFFER:
		mnOptionBackBuffer = nValue;
		break;
	case VSYNC:
		mnOptionVsync = nValue;
		break;
	case OLD_STARTUP:
		mnOptionOldStartup = nValue;
		break;
	case MINIMUM_SURFACE_RES:
		mnOptionMinimumSurfaceRes = nValue;
		break;
	case FOG_MODE:
		mnOptionFogMode = nValue;
		break;
	case FSAA:
		mnFullscreenAntialias = nValue;
		break;
	}

}

/***************************************************************************
 * Function    : InterfaceInit
 * Params      : 
 * Returns     :
 * Description : 
 ***************************************************************************/
INTERFACE_API void InterfaceInit( BOOL bUseDefaultFonts )
{
	if ( mboInterfaceInitialised == FALSE )
	{
		InterfaceSetIsUsingDefaultFonts( bUseDefaultFonts );
		InitialiseFont( bUseDefaultFonts );
		InitialiseOverlays();

		InitTexturedOverlays();

		InterfaceImagesInit();
	}

	mboInterfaceInitialised = TRUE;
}


/***************************************************************************
 * Function    : InterfaceBeginScene
 * Params      : 
 * Returns     :
 * Description : 
 ***************************************************************************/
void InterfaceBeginScene( void )
{
	LockOverlays();
}

void	InterfaceSetDrawCallback( InterfaceDrawCallback fnOnDrawCallback )
{
	mfnInterfaceOnDrawCallback = fnOnDrawCallback;
}

/***************************************************************************
 * Function    : InterfaceEndScene
 * Params      : 
 * Returns     :
 * Description : 
 ***************************************************************************/
void InterfaceEndScene( void )
{
	UnlockOverlays();
}


/***************************************************************************
 * Function    : InterfaceDrawAllElements
 * Params      : 
 * Returns     :
 * Description : 
 ***************************************************************************/
INTERFACE_API void InterfaceDrawAllElements( void )
{
	DrawTexturedOverlays(0);
	RenderOverlays( 0 );
	RenderStrings( 0 );

	DrawPrimaryJpeg();
	DrawBufferedJpegs(0);
	DrawTexturedOverlays(1);
	RenderOverlays( 1 );
	RenderStrings( 1 );
	DrawBufferedJpegs(1);

	DrawTexturedOverlays(2);
	RenderOverlays( 2 );
	DrawBufferedJpegs( 2 );
	RenderStrings( 2 );

	RenderOverlays( 3 );
	RenderStrings( 3 );

	// Clear buffered interface items
	ClearStrings();

	if ( mfnInterfaceOnDrawCallback )
	{
		mfnInterfaceOnDrawCallback();
	}
}



/***************************************************************************
 * Function    : InterfaceDrawNoMatrix
 * Params      : 
 * Returns     :
 * Description : 
 ***************************************************************************/
INTERFACE_API void InterfaceDrawNoMatrix( void )
{
	if (!mpInterfaceD3DDevice) return;

	mpInterfaceD3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	InterfaceDrawAllElements();
}


void	InterfaceScreenSetWorldMatrix( VECT* pTopLeft, VECT* pBotLeft, VECT* pBotRight, int nScreenWidth, int nScreenHeight )
{
VECT	vecCross;
VECT	midXPos;
VECT	vecBottomEdge;
VECT	midYPos;
VECT	vecLeftEdge;
float	fLenLeft;
float	fLenTop;
float pixelWidth = (float)nScreenWidth;//InterfaceGetWidth();
float pixelHeight = (float)nScreenHeight;//InterfaceGetHeight();

	// Get length and normalized vec of bottom edge of screen
	VectSub( &midXPos, pBotRight, pBotLeft );
	fLenTop = VectGetLength( &midXPos );
	vecBottomEdge = midXPos;
	VectNormalize( &vecBottomEdge );

	// Get length and normalized vec of left edge of screen
	VectSub( &midYPos, pBotLeft, pTopLeft );
	fLenLeft = VectGetLength( &midYPos );
	vecLeftEdge = midYPos;
	VectNormalize( &vecLeftEdge );

	VectScale( &vecBottomEdge, &vecBottomEdge,fLenTop / (float)pixelWidth );
	VectScale( &vecLeftEdge, &vecLeftEdge, fLenLeft / (float)pixelHeight );
	VectCross( &vecCross, &vecBottomEdge, &vecLeftEdge );

	ENGINEMATRIX worldMatrix;
	worldMatrix._11 = vecBottomEdge.x;
	worldMatrix._12 = vecBottomEdge.y;
	worldMatrix._13 = vecBottomEdge.z;
	worldMatrix._14 = 0.0f;

	worldMatrix._21 = vecLeftEdge.x;
	worldMatrix._22 = vecLeftEdge.y;
	worldMatrix._23 = vecLeftEdge.z;
	worldMatrix._24 = 0.0f;

	worldMatrix._31 = vecCross.x;
	worldMatrix._32 = vecCross.y;
	worldMatrix._33 = vecCross.z;
	worldMatrix._34 = 0.0f;

	worldMatrix._41 = pTopLeft->x;
	worldMatrix._42 = pTopLeft->y;
	worldMatrix._43 = pTopLeft->z;
	worldMatrix._44 = 1.0f;

	EngineSetWorldMatrix( &worldMatrix);

}

INTERFACE_API void	InterfaceDrawUI( float fScreenAngle )
{
	if (!mpInterfaceD3DDevice) return;

	if ( InterfaceIsVRMode() == TRUE )
	{
	VECT*	pxCamPos = EngineCameraGetPos();
	VECT	xCamDir = { 0.0f, -1.0f, 0.0f };
//	VECT*	pxCamDir = EngineCameraGetDirection();
	VECT*	pxCamDir = &xCamDir;
	VECT	xUp = { 0.0f, 0.0f, 1.0f };
	VECT	xRight;
	VECT	xScreenPosTL;
	VECT	xScreenPosBL;
	VECT	xScreenPosBR;
	VECT	xOffset;
	float	fOffsetDepth = 2.5f;

		VectRotateAboutZ( &xCamDir, fScreenAngle );
		xOffset = *pxCamDir;

		VectNormalize( &xOffset );
		VectScale( &xOffset, &xOffset, fOffsetDepth );
		VectAdd( &xScreenPosTL, pxCamPos, &xOffset );

		VectCross( &xRight, &xUp, pxCamDir );
		VectNormalize( &xRight );
		VectAdd( &xScreenPosTL, &xScreenPosTL, &xUp );
		VectScale( &xOffset, &xUp, -2.0f );
		VectAdd(  &xScreenPosBL, &xScreenPosTL, &xOffset );
		VectAdd(  &xScreenPosBR, &xScreenPosBL, &xRight );

		EngineCameraUpdate();
		InterfaceScreenSetWorldMatrix( &xScreenPosTL, &xScreenPosBL, &xScreenPosBR, 960, 1080 );
	}
	else
	{
	//------------
		D3DXMATRIX Ortho2D;	
		ENGINEMATRIX xMatrix;
	
		EngineMatrixIdentity( &xMatrix );
		EngineSetViewMatrix( &xMatrix );
		
		xMatrix._22 = -1.0f;
		xMatrix._41 = (float)( -(InterfaceGetWidth()/2) );
		xMatrix._42 = (float)( +(InterfaceGetHeight()/2) );
		EngineSetWorldMatrix( &xMatrix );

		D3DXMatrixOrthoLH(&Ortho2D, (float)InterfaceGetWidth(), (float)InterfaceGetHeight(), 0.0f, 1.0f);
		EngineSetProjectionMatrix( &Ortho2D);
	//------------
	}
	mpInterfaceD3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

	InterfaceDrawAllElements();
}


/***************************************************************************
 * Function    : InterfaceDraw
 * Params      : 
 * Returns     :
 * Description : 
 ***************************************************************************/
INTERFACE_API void InterfaceDraw( void )
{
	if (!mpInterfaceD3DDevice) return;

//------------
	D3DXMATRIX Ortho2D;	
	ENGINEMATRIX xMatrix;
	
	D3DXMatrixOrthoLH(&Ortho2D, (float)InterfaceGetWidth(), (float)InterfaceGetHeight(), 0.0f, 1.0f);
	EngineMatrixIdentity(&xMatrix);

	EngineSetViewMatrix( &xMatrix);
	xMatrix._22 = -1.0f;
	xMatrix._41 = (float)( InterfaceGetWidth() ) * -0.5f;
	xMatrix._42 = (float)( InterfaceGetHeight() ) * 0.5f;
	EngineSetWorldMatrix( &xMatrix );
	EngineSetProjectionMatrix( &Ortho2D);
//------------

	mpInterfaceD3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );

	InterfaceDrawAllElements();

	// Reset the blend mode as it may have been left in a funky state..
	mpInterfaceD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	mpInterfaceD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

}



/***************************************************************************
 * Function    : InterfaceFree
 * Params      : 
 * Returns     :
 * Description : 
 ***************************************************************************/
INTERFACE_API void InterfaceFree( void )
{
	if ( mboInterfaceInitialised == TRUE )
	{
		mpInterfaceD3DDevice->SetTexture(0, NULL);

		InterfaceImagesFree();
		ClearBufferedJpegs();
		FreeFont( TRUE );
		FreeTexturedOverlays();
		FreeOverlays();
		mboInterfaceInitialised = FALSE;
	}
}

INTERFACE_API void InterfaceReleaseForDeviceReset( void )
{
	mpInterfaceD3DDevice->SetTexture(0, NULL);
	InterfaceImagesFree();
	ClearBufferedJpegs();
	FreeFont( FALSE );
	FreeTexturedOverlays();
	FreeOverlays();
}

INTERFACE_API void InterfaceRestorePostDeviceReset( void )
{
	InitialiseFontBuffers();
	InitialiseOverlays();
	InitTexturedOverlays();
	InterfaceImagesInit();
}

