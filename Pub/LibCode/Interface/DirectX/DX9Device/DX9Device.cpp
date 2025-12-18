
#include "../InterfaceInternalsDX.h"

#include <stdio.h>
#include <mmsystem.h>

#include <StandardDef.h>
#include <Interface.h>
#include <Engine.h>

#include "../../Win32/Interface-Win32.h"
#include "../../Common/InterfaceCommon.h"
#include "../../Common/InterfaceInstance.h"
#include "../../Common/InterfaceOptions.h"
#include "../../Common/InterfaceUtil.h"
#include "../../Common/InterfaceDevice.h"
#include "../../../Engine/DirectX/OculusDX.h"
#include "../../../Engine/DirectX/EngineDX.h"
#include "../InterfaceInternalsDX.h"
#include "DX9Device.h"

extern "C"
{
#include "../../TempInterfaceResourceList.h"		// Don't even think about asking what this is about.
}

#include "d3dapp.h"


LPGRAPHICSDEVICE       mpLegacyInterfaceD3DDeviceSingleton = NULL; // Our rendering device

LPGRAPHICS        mpD3D       = NULL; // Used to create the D3DDevice

BOOL	mboMinPageSize = TRUE;
BOOL	msbInterfaceGlobalTextureFilteringEnable = TRUE;


u64		mullInterfaceLastPresentTick = 0;

BOOL	mboCurrentlyFullscreen = FALSE;

float	mfInterfaceFPS = 0.0f;

//#ifndef STANDALONE
static CD3DApplication		mcd3dUtilApp;
//#endif
ENGINEMATERIAL mxStandardMat;



INTERFACE_API LPGRAPHICS	InterfaceGetD3D( void )
{
	return( mpD3D );
}




void		OnSetWindowSize( BOOL boFullScreen, int nWidth, int nHeight )
{
//#ifndef STANDALONE
	mcd3dUtilApp.m_bWindowed = !boFullScreen;
//#endif
}





/***************************************************************************
 * Function    : OnSetInitialSize
 ***************************************************************************/
INTERFACE_API void OnSetInitialSize( BOOL boFullScreen, int nFullScreenSizeX, int nFullScreenSizeY , BOOL boSmallFlag )
{
	mboCurrentlyFullscreen = boFullScreen;
	mboFullScreen = boFullScreen;
}


/***************************************************************************
 * Function    : InterfaceGetWidth
 ***************************************************************************/
INTERFACE_API int InterfaceGetWidth( void )
{
	return( InterfaceInstanceMain()->GetWidth() );
}

/***************************************************************************
 * Function    : InterfaceGetHeight
 ***************************************************************************/
INTERFACE_API int InterfaceGetHeight( void )
{
	return( InterfaceInstanceMain()->GetHeight() );
}
INTERFACE_API int InterfaceGetCentreX( void )
{
	return( InterfaceInstanceMain()->GetCentreX() );
}
INTERFACE_API int InterfaceGetCentreY( void )
{
	return( InterfaceInstanceMain()->GetCentreY() );
}

int InterfaceInstance::GetHeight( void )
{
	if ( m_DrawRect.h != 0 )
	{
		return( m_DrawRect.h );
	}
	return( mnRenderSurfaceHeight );
}

int		InterfaceInstance::GetWidth( void )
{
	if ( m_DrawRect.w != 0 )
	{
		return( m_DrawRect.w );
	}
	return( mnRenderSurfaceWidth );
}

int InterfaceInstance::GetCentreX( void )
{
	if ( m_DrawRect.w != 0 )
	{
		return( m_DrawRect.w / 2 );
	}
	return( mnRenderSurfaceWidth / 2 );
}

int InterfaceInstance::GetCentreY( void )
{
	if ( m_DrawRect.h != 0 )
	{
		return( m_DrawRect.h / 2 );
	}
	return( mnRenderSurfaceHeight / 2 );
}










BOOL InterfaceShowFullscreenPanic( char* szErrorString )
{	
MSG msg;
uint32		ulThisTickGap = 0;
uint32		ulLastTick = GetTickCount();

	// Enter the message loop
	ZeroMemory( &msg, sizeof(msg) );
	while( msg.message!=WM_QUIT )
	{
		if ( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
		else
		{	
			ulThisTickGap = GetTickCount() - ulLastTick;
			if ( ulThisTickGap > 10 )
			{
				ulLastTick = GetTickCount();
				
				InterfaceShadedBox( 2, 200, 200, 400, 150, 0);
				InterfaceRect( 2, 200, 200, 400, 150, 0xFF000000 );
				InterfaceTextCenter( 2, 200, 600, 220, "TheUniversal Error Message :", COL_F2_COMMS_SPECIAL, 1 );
				InterfaceTextCenter( 2, 200, 600, 260, szErrorString, COL_COMMS_SPECIAL, 0 );
				InterfaceText( 2, 220, 320, "Press Enter to continue, Esc to ignore further errors", COL_WARNING, 0 );

#ifndef INTERFACE_API		// TEMP!
				GameBasicMessagingUpdate();
#endif	
				InterfaceBeginRender();
		
				InterfaceDraw();	
				InterfaceEndRender();
				// Present the backbuffer contents to the display
				InterfacePresent();
				if ( GetKeyState (VK_ESCAPE ) == TRUE )
				{
					return( TRUE );
				}

				if ( GetKeyState( VK_RETURN ) < 0 )
				{
					return( FALSE );
				}
			}
		}
	}
	return( TRUE );
}


#ifdef TUD9
GRAPHICSCAPS	md3dCaps;
BOOL			mbCapsIsSet = FALSE;
#endif


int		InterfaceGetDeviceCaps( int Type )
{
#ifdef TUD11

#else
	switch( Type )
	{
	case MAX_ANISTROPY:
		if ( ( md3dCaps.RasterCaps & D3DPRASTERCAPS_ANISOTROPY ) !=  0 )
		{
			return( md3dCaps.MaxAnisotropy );
		}
		return( 0 );
		break;
	default:
		break;
	}
#endif
	return( 0 );
}


/**************************************************************************
 * Function    : InterfaceInitSmall
 * Params      :
 * Returns     :
 * Description : All the calls to all the stuff that should be done when small.
 ***************************************************************************/
INTERFACE_API void InterfaceInitSmall( void )
{
/*

	if ( InterfaceIsSmall() == FALSE )
	{
		InterfaceSetSmall( TRUE );
		if ( mboCurrentlyFullscreen == TRUE )
		{
			SetCursor( LoadCursor(NULL, IDC_ARROW) );
			InterfaceSetWindowHasChanged( TRUE );
			// When changing from fullscreen to small, we first must set the fullscreen window back to a normal one
			// so that its not retaining exclusive access to the draw screen
			int		nWidth = InterfaceGetWindowWidth();
			int		nHeight = InterfaceGetWindowHeight();

			InterfaceSetWindowSize( FALSE, nWidth, nHeight, FALSE );
			InterfaceSetD3DDevice( InterfaceInitD3D(mboMinPageSize) );

			if ( mpInterfaceD3DDevice != NULL )
			{
			D3DCOLOR xColor;
				// Clear the backbuffer and the zbuffer
				xColor = (D3DCOLOR)( 0 );
				mpInterfaceD3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL, xColor, 1.0f, 0 );
				mpInterfaceD3DDevice->Present( NULL, NULL, NULL, NULL );
			}
			ShowWindow( mhwndInterfaceMain, SW_SHOW );
			UpdateWindow( mhwndInterfaceMain );
			SetCursor( LoadCursor(NULL, IDC_ARROW) );
			mboFullScreen = TRUE;
		}	
	}
	else
	{
		InterfaceSetSmall( FALSE );
		InterfaceSetWindowHasChanged( TRUE );
	}
*/
}

INTERFACE_API void InterfaceSetStandardMaterial( void )
{
	// Set the RGBA for diffuse reflection.
	mxStandardMat.Diffuse.r = 1.0f;
	mxStandardMat.Diffuse.g = 1.0f;
	mxStandardMat.Diffuse.b = 1.0f;
	mxStandardMat.Diffuse.a = 1.0f;
	
	// Set the RGBA for ambient reflection.
	mxStandardMat.Ambient.r = 1.0f;
	mxStandardMat.Ambient.g = 1.0f;
	mxStandardMat.Ambient.b = 1.0f;
	mxStandardMat.Ambient.a = 1.0f;
	
	// Set the color and sharpness of specular highlights.
	mxStandardMat.Specular.r = 0.0f;
	mxStandardMat.Specular.g = 0.0f;
	mxStandardMat.Specular.b = 0.0f;
	mxStandardMat.Specular.a = 1.0f;
	mxStandardMat.Power = 0.0f;
	
	// Set the RGBA for emissive color.
	mxStandardMat.Emissive.r = 0.0f;
	mxStandardMat.Emissive.g = 0.0f;
	mxStandardMat.Emissive.b = 0.0f;
	mxStandardMat.Emissive.a = 1.0f;

	EngineSetMaterial(&mxStandardMat);
}

void InterfaceEnableTextureFiltering( BOOL bFlag )
{
	msbInterfaceGlobalTextureFilteringEnable = bFlag;
}

void InterfaceTurnOffTextureFiltering( void )
{
	InterfaceInstanceMain()->EnableTextureFiltering( FALSE );
}

void InterfaceTurnOnTextureFiltering( int nMode )
{
	InterfaceInstanceMain()->EnableTextureFiltering( TRUE );
}

void InterfaceInstance::EnableTextureFiltering( BOOL bFlag )
{
	if ( mboTextureFilteringCurrentState != bFlag )
	{
		if ( bFlag == FALSE )
		{
			mpLegacyInterfaceD3DDeviceSingleton->SetSamplerState( 0, D3DSAMP_MAGFILTER,   D3DTEXF_POINT );
			mpLegacyInterfaceD3DDeviceSingleton->SetSamplerState( 0, D3DSAMP_MINFILTER,   D3DTEXF_POINT  );
			mpLegacyInterfaceD3DDeviceSingleton->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_POINT );	
			mpLegacyInterfaceD3DDeviceSingleton->SetSamplerState( 0, D3DSAMP_MAXANISOTROPY, 1 );
		}
		else
		{
			if ( msbInterfaceGlobalTextureFilteringEnable != FALSE )
			{
				mpLegacyInterfaceD3DDeviceSingleton->SetSamplerState( 0, D3DSAMP_MAGFILTER, mnMagFilter );
				mpLegacyInterfaceD3DDeviceSingleton->SetSamplerState( 0, D3DSAMP_MINFILTER, mnMinFilter );
				mpLegacyInterfaceD3DDeviceSingleton->SetSamplerState( 0, D3DSAMP_MIPFILTER, mnMipFilter );//D3DTEXF_GAUSSIANQUAD );// mnMipFilter );	
			}	
		}
		mboTextureFilteringCurrentState = bFlag;
	}

}



BOOL TestDepth(D3DFORMAT fmt, D3DDISPLAYMODE d3ddm)
{
    if (D3D_OK!=mpD3D->CheckDeviceFormat(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,d3ddm.Format,
                                         D3DUSAGE_DEPTHSTENCIL,D3DRTYPE_SURFACE,fmt))
        return FALSE;
    if (D3D_OK!=mpD3D->CheckDepthStencilMatch(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,
                                              d3ddm.Format,d3ddm.Format,fmt))
        return FALSE;
    return TRUE;
}

int		msnInterfaceMaxRenderPageWidth = 0;

void		InterfaceSetMaximumFrontBufferWidth( int nMaxWidth )
{
	msnInterfaceMaxRenderPageWidth = nMaxWidth;
}

BOOL		InterfaceInternalsDX::GetDXDeviceCreateParams( HWND hWindow, BOOL boMinPageSize,  D3DPRESENT_PARAMETERS* pD3Dpp )
{
D3DDISPLAYMODE d3ddm;

	if( FAILED( mpD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d3ddm ) ) )
	{
		PANIC_IF(TRUE,"Couldn't get display mode" );
		return( NULL );
	}

	//
	if ( mboFullScreen )
	{
		pD3Dpp->Windowed = FALSE;
		pD3Dpp->BackBufferWidth  = d3ddm.Width;
		pD3Dpp->BackBufferHeight = d3ddm.Height;
		// Set fullscreen-mode style
		InterfaceSetWindowStyle( hWindow, true );
	}
	else
	{
		// Set windowed-mode style
		InterfaceSetWindowStyle( hWindow, false );
		pD3Dpp->Windowed = TRUE;
		
		RECT	xRect;
		GetClientRect( hWindow, &xRect );
		int nWindowWidth = xRect.right - xRect.left;
		int nWindowHeight = xRect.bottom - xRect.top;
		pD3Dpp->BackBufferWidth  = nWindowWidth;
		pD3Dpp->BackBufferHeight = nWindowHeight;
	}

	if ( msnInterfaceMaxRenderPageWidth != 0 )
	{
		if ( pD3Dpp->BackBufferWidth > (uint32)msnInterfaceMaxRenderPageWidth )
		{
		float	fAdjust = ( (float)pD3Dpp->BackBufferWidth ) / msnInterfaceMaxRenderPageWidth;

			 pD3Dpp->BackBufferWidth = msnInterfaceMaxRenderPageWidth;
			 pD3Dpp->BackBufferHeight = (uint32)( pD3Dpp->BackBufferHeight / fAdjust );
		}

		// If Fullscreen.. find a suitable display mode that best matches the requested format
		if ( mboFullScreen )
		{
        DWORD dwNumAdapterModes = mpD3D->GetAdapterModeCount( 0, d3ddm.Format );
		int		nWidthGap;
		int		nBestWidthGap = 99999999;
		int		nBestWidth;
		int		nBestHeight;
		D3DFORMAT		nBestFormat;

			for( UINT iMode = 0; iMode < dwNumAdapterModes; iMode++ )
			{
			D3DDISPLAYMODE DisplayMode;
	            mpD3D->EnumAdapterModes( 0, d3ddm.Format, iMode, &DisplayMode );

				nWidthGap = abs( (int)(DisplayMode.Width - pD3Dpp->BackBufferWidth) );
				if ( nWidthGap < nBestWidthGap )
				{
					nBestWidthGap = nWidthGap;
					nBestWidth = DisplayMode.Width;
					nBestHeight = DisplayMode.Height;
					nBestFormat = DisplayMode.Format;
				}
			}

			if ( nBestWidthGap != 99999999 )
			{
				pD3Dpp->BackBufferWidth = nBestWidth;
				pD3Dpp->BackBufferHeight = nBestHeight;
				pD3Dpp->BackBufferFormat = nBestFormat;
			}
		}
	}

	mpInterfaceInstance->SetRenderSurfaceSize( pD3Dpp->BackBufferWidth, pD3Dpp->BackBufferHeight );

	InterfaceSetZBufferLockable( FALSE );

	// Apply full-scene antialias if available and on in options
	if ( InterfaceGetOption( FSAA ) == 1 )
	{
		if( SUCCEEDED(mpD3D->CheckDeviceMultiSampleType( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3ddm.Format, FALSE, D3DMULTISAMPLE_4_SAMPLES, NULL ) ) )
		{
			pD3Dpp->MultiSampleType = D3DMULTISAMPLE_4_SAMPLES;
		}
		else if ( SUCCEEDED(mpD3D->CheckDeviceMultiSampleType( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3ddm.Format, FALSE, D3DMULTISAMPLE_2_SAMPLES, NULL ) ) )
		{
			pD3Dpp->MultiSampleType = D3DMULTISAMPLE_2_SAMPLES;
		}
	}
	//	    pD3Dpp->SwapEffect = D3DSWAPEFFECT_COPY_VSYNC;
	pD3Dpp->SwapEffect = D3DSWAPEFFECT_DISCARD;
	pD3Dpp->BackBufferFormat = d3ddm.Format;
	pD3Dpp->EnableAutoDepthStencil = TRUE;
	pD3Dpp->AutoDepthStencilFormat = D3DFMT_D24S8;
	pD3Dpp->hDeviceWindow          = hWindow;
	if ( InterfaceGetOption(VSYNC) == 1 )
	{
		pD3Dpp->PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	}
	else
	{
		pD3Dpp->PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	}
	pD3Dpp->Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;

	// If we're creating from scratch (not refreshing existing device)
	if ( mpInterfaceD3DDevice == NULL )
	{
		// Find a decent fmt for the depth buffer
		if ( TestDepth(D3DFMT_D32,d3ddm) == TRUE )
		{
			pD3Dpp->AutoDepthStencilFormat = D3DFMT_D32;
		}
		else if ( TestDepth(D3DFMT_D24S8,d3ddm) == TRUE )
		{
			pD3Dpp->AutoDepthStencilFormat = D3DFMT_D24S8;
		}
		else if ( TestDepth(D3DFMT_D16_LOCKABLE,d3ddm) == TRUE )
		{
			pD3Dpp->AutoDepthStencilFormat = D3DFMT_D16_LOCKABLE;
		}
		else if ( TestDepth(D3DFMT_D24X8,d3ddm) == TRUE )
		{
			pD3Dpp->AutoDepthStencilFormat = D3DFMT_D24X8;
		}
		else if ( TestDepth(D3DFMT_D16,d3ddm) == TRUE )
		{
			pD3Dpp->AutoDepthStencilFormat = D3DFMT_D16;
		}
		else if ( TestDepth(D3DFMT_D15S1,d3ddm) == TRUE )
		{
			pD3Dpp->AutoDepthStencilFormat = D3DFMT_D15S1;
		}
		else
		{
			PANIC_IF(TRUE,"Couldnt find a suitable z-depth format. You may need to change your desktop display format (from 32bit to 16bit, for instance) for this game to work.\n\nAlternatively, check for updates of your video card drivers." );
			// Try with the swapeffect changed
			pD3Dpp->SwapEffect = D3DSWAPEFFECT_DISCARD;
		}
	}

	return( TRUE );
}





/***************************************************************************
 * Function    : InterfaceInstance::InitD3D
 ***************************************************************************/
void	 InterfaceInstance::InitD3D( HWND hWindow, BOOL boMinPageSize )
{
LPGRAPHICSDEVICE	pNewGraphicsDevice;

	mhWindow = hWindow;
	if ( InterfaceIsVRMode() == TRUE )
//		 ( EngineHasOculus() == TRUE ) )
	{
		if ( mpD3D == NULL )
		{
#ifdef USE_D3DEX_INTERFACE
			Direct3DCreate9Ex( D3D_SDK_VERSION, &mpD3D );
#else
			mpD3D = Direct3DCreate9( D3D_SDK_VERSION );
#endif
			// Create the D3D object.
			if( mpD3D == NULL )
			{
				// Fallback to 9.0b
#ifdef USE_D3DEX_INTERFACE
				Direct3DCreate9Ex( D3D9b_SDK_VERSION, &mpD3D  );
#else
				mpD3D = Direct3DCreate9( D3D9b_SDK_VERSION  );
#endif
				if( mpD3D == NULL )
				{
					PANIC_IF(TRUE,"Direct3D Create Failed. You may need to update your DirectX runtime and/or video card drivers for this game to work" );
					return;
				}
			}

			pNewGraphicsDevice = (LPGRAPHICSDEVICE) OculusInitD3DDevice( mpD3D );
		}
	}
	else
	{
	D3DXVECTOR3 xVect;
	HRESULT		hr;
	D3DDISPLAYMODE d3ddm;
	bool		bFullScreenAntiAlias = false;

		mboMinPageSize = boMinPageSize;
		if ( mnWindowWidth == 0 ) mnWindowWidth = 800;
		if ( mnWindowHeight == 0 ) mnWindowHeight = 800;

		if ( mpD3D == NULL )
		{
#ifdef USE_D3DEX_INTERFACE
			Direct3DCreate9Ex( D3D_SDK_VERSION, &mpD3D );
#else
			mpD3D = Direct3DCreate9( D3D_SDK_VERSION );
#endif
			// Create the D3D object.
			if( mpD3D == NULL )
			{
				// Fallback to 9.0b
#ifdef USE_D3DEX_INTERFACE
				Direct3DCreate9Ex( D3D9b_SDK_VERSION, &mpD3D );
#else
				mpD3D = Direct3DCreate9( D3D9b_SDK_VERSION );
#endif
				if( mpD3D == NULL )
				{
					PANIC_IF(TRUE,"Direct3D Create Failed. You may need to update your DirectX runtime and/or video card drivers for this game to work" );
					return;
				}
			}
		}

		// Get the current desktop display mode, so we can set up a back
		// buffer of the same format
		if( FAILED( mpD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d3ddm ) ) )
		{
			PANIC_IF(TRUE,"Couldn't get display mode" );
			return;
		}

		// Set up the present parameters - This is generally what odd vid cards have a problem with
		D3DPRESENT_PARAMETERS d3dpp;
		ZeroMemory( &d3dpp, sizeof(d3dpp) );

		mpInterfaceInternals->GetDXDeviceCreateParams( hWindow, boMinPageSize, &d3dpp );

		pNewGraphicsDevice = mpInterfaceInternals->mpInterfaceD3DDevice;
		if ( pNewGraphicsDevice == NULL )
		{
		int		nAdapterToUse = D3DADAPTER_DEFAULT;

//		D3DDISPLAYMODEEX*		pDisplayModeEx;

//			hr = mpD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWindow,D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &pNewGraphicsDevice );

#ifdef USE_D3DEX_INTERFACE
			hr = mpD3D->CreateDeviceEx( nAdapterToUse, D3DDEVTYPE_HAL, hWindow, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, NULL, &pNewGraphicsDevice );								  
#else
			hr = mpD3D->CreateDevice( nAdapterToUse, D3DDEVTYPE_HAL, hWindow, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &pNewGraphicsDevice );								  
#endif

			d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;

			if ( hr == D3DERR_OUTOFVIDEOMEMORY )
			{
#ifdef USE_D3DEX_INTERFACE
				hr = mpD3D->CreateDeviceEx( nAdapterToUse, D3DDEVTYPE_HAL, hWindow,
										  D3DCREATE_HARDWARE_VERTEXPROCESSING/*|D3DCREATE_MULTITHREADED*/,
										  &d3dpp, NULL, &pNewGraphicsDevice );
#else
				hr = mpD3D->CreateDevice( nAdapterToUse, D3DDEVTYPE_HAL, hWindow,
										  D3DCREATE_HARDWARE_VERTEXPROCESSING/*|D3DCREATE_MULTITHREADED*/,
										  &d3dpp, &pNewGraphicsDevice );
#endif
				if ( FAILED(hr) )
				{
					// Using 'Discard' can save us a bunch of vid mem apparently.. 
					// so try to create the screen using this
					d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
#ifdef USE_D3DEX_INTERFACE
					hr = mpD3D->CreateDeviceEx( nAdapterToUse, D3DDEVTYPE_HAL, hWindow,
											  D3DCREATE_SOFTWARE_VERTEXPROCESSING/*|D3DCREATE_MULTITHREADED*/,
											  &d3dpp, NULL, &pNewGraphicsDevice );
#else
					hr = mpD3D->CreateDevice( nAdapterToUse, D3DDEVTYPE_HAL, hWindow,
											  D3DCREATE_SOFTWARE_VERTEXPROCESSING/*|D3DCREATE_MULTITHREADED*/,
											  &d3dpp, &pNewGraphicsDevice );

#endif
				}
			}

			if( FAILED( hr ) )
			{	
				InterfaceSetWindowStyle( hWindow, false );
				switch ( hr )
				{
				case D3DERR_OUTOFVIDEOMEMORY:
					PANIC_IF(TRUE,"Not enough video memory to create D3D Device.\n You might be able to fix this by lowering your screen resolution in your desktop display properties." );
					break;
				case D3DERR_INVALIDCALL:
					PANIC_IF(TRUE,"Invalid call for direct3d create device" );
					break;
				default:
					PANIC_IF(TRUE,"Direct3D Create Device failed. There may be problems with your video card drivers or DirectX install.\n Adjusting the DirectX setup options on the Video Options menu may help." );
					break;
				}
				return;
			}
			hr = pNewGraphicsDevice->Reset( &d3dpp );
		}
		else
		{	
			// Clear the backbuffer and the zbuffer
			pNewGraphicsDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL,0, 1.0f, 0 );
			pNewGraphicsDevice->Present( NULL, NULL, NULL, NULL );
		
			InterfaceSetZBufferLockable( TRUE );
			hr = pNewGraphicsDevice->Reset( &d3dpp );

			if( FAILED( hr ) )
			{
			char	acErrorMsg[1024];
			char	acString[256];

				switch ( hr )
				{
				case D3DERR_OUTOFVIDEOMEMORY:
					strcpy( acErrorMsg,"Out of vid mem\r\n" );
					break;
				case E_OUTOFMEMORY:
					strcpy( acErrorMsg,"Out of sys mem\r\n" );
					break;
				case D3DERR_INVALIDCALL:
					strcpy( acErrorMsg,"Reset failed : Invalid call\r\n" );
					break;
				case D3D_OK:
					strcpy( acErrorMsg,"Reset OK?\r\n" );
					break;
				default:
					strcpy( acErrorMsg,"D3D Reset failed - Unknown\r\n" );
					break;
				}

				sprintf( acString, "%d vertex buffers listed\r\n", EngineGetNumVertexBuffersAllocated() );
				strcat( acErrorMsg, acString );
				if ( EngineGetNumVertexBuffersAllocated() > 0 )
				{
					EngineVertexBufferTrackingListAllocated( acErrorMsg );
				}
				sprintf( acString, "%d render targets\r\n", EngineTextureManagerGetNumRenderTargets() );
				strcat( acErrorMsg, acString );
				if ( EngineTextureManagerGetNumRenderTargets() > 0 )
				{
					EngineRenderTargetsTrackingListAllocated( acErrorMsg );
				}

				
				PANIC_IF( TRUE, acErrorMsg );

				InterfaceSetWindowStyle( hWindow, false );
				InterfaceInitSmall();
				return;
			}		
		}

		// Resize the window if we're in windowed mode and we have just left fullscreen
		if( ( mboFullScreen == FALSE ) &&
			( mboCurrentlyFullscreen == TRUE ) )
		{	
			SetWindowPos( hWindow, HWND_NOTOPMOST,
							mnWindowLeft, mnWindowTop,
							mnWindowWidth, mnWindowHeight,
							SWP_SHOWWINDOW );
		}

		mboCurrentlyFullscreen = mboFullScreen;

		if ( pNewGraphicsDevice != NULL )
		{
		float	fBias = 0.0f;
	/*	D3DCAPS8	d3dCaps;

			mpInterfaceD3DDevice->GetDeviceCaps( &d3dCaps );
	*/
			// Set standard render modes bit
			pNewGraphicsDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
			pNewGraphicsDevice->SetRenderState( D3DRS_ZENABLE, TRUE );
			pNewGraphicsDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
			if ( bFullScreenAntiAlias )
			{
				pNewGraphicsDevice->SetRenderState( D3DRS_MULTISAMPLEANTIALIAS, TRUE );
			}
			InterfaceSetWindowHasChanged( FALSE );

			pNewGraphicsDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL,
									 0, 1.0f, 0 );
			pNewGraphicsDevice->Present( NULL, NULL, NULL, NULL );

			fBias = mfMipMapBias;
			pNewGraphicsDevice->SetSamplerState(0, D3DSAMP_MIPMAPLODBIAS, *((LPDWORD) (&fBias)));

			D3DXMATRIX matTrans;
			D3DXMatrixIdentity( &matTrans );
			// Set-up the matrix for the desired transformation.
			pNewGraphicsDevice->SetTransform( D3DTS_TEXTURE0, &matTrans );
			pNewGraphicsDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
		}

		if ( EngineGetDXDevice() == NULL )
		{
			EngineInitDX( pNewGraphicsDevice );
		}
		InterfaceSetStandardMaterial();
	}
	SetDevice( pNewGraphicsDevice );

#ifdef TUD9
	if ( pNewGraphicsDevice )
	{
		pNewGraphicsDevice->GetDeviceCaps( &md3dCaps );
		mbCapsIsSet = TRUE;
	}
#endif
//    return( mpInterfaceD3DDevice );
}


INTERFACE_API void				InterfaceInitDisplayDevice( BOOL boMinRenderPageSize )
{
	InterfaceInstanceMain()->InitD3D( mhwndInterfaceMain, boMinRenderPageSize );
	mpLegacyInterfaceD3DDeviceSingleton = InterfaceInstanceMain()->mpInterfaceInternals->mpInterfaceD3DDevice;
}


INTERFACE_API INT_PTR CALLBACK InterfaceVidOptionsDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
//#ifndef STANDALONE
	return mcd3dUtilApp.SelectDeviceProc( hDlg, msg, wParam, lParam );
//#else
//	return( 0 );
//#endif
}


uint32	mulRefCount;
INTERFACE_API void InterfaceFreeAllD3D( void )
{
//uint32	ulRefCount;
    if( mpLegacyInterfaceD3DDeviceSingleton != NULL )
	{
		mpLegacyInterfaceD3DDeviceSingleton->SetTexture(0, NULL);
		mpLegacyInterfaceD3DDeviceSingleton->SetTexture(1, NULL);
		mpLegacyInterfaceD3DDeviceSingleton->EvictManagedResources();
		mulRefCount = mpLegacyInterfaceD3DDeviceSingleton->Release();
		mpLegacyInterfaceD3DDeviceSingleton = NULL;
//#ifndef STANDALONE
		mcd3dUtilApp.m_pd3dDevice = NULL;
//endif
	}

	if( mpD3D != NULL )
	{
		mulRefCount = mpD3D->Release();
		mpD3D = NULL;
//#ifndef STANDALONE
		mcd3dUtilApp.m_pD3D = NULL;
//#endif
	}

}

INTERFACE_API void InterfaceInitVidOptions( HINSTANCE hInst, HWND hDialogWind )
{
//#ifndef STANDALONE
HWND	hWnd;
	if ( mpD3D == NULL )
	{
		mcd3dUtilApp.m_bWindowed = !mboFullScreen;
		mcd3dUtilApp.InitD3DMinimal( GetDesktopWindow() );
	}

	hWnd = CreateDialogParam(hInst, (LPCTSTR)IDD_VID_OPTIONS, hDialogWind, (DLGPROC)InterfaceVidOptionsDlgProc,(LPARAM)(&mcd3dUtilApp) );
	ShowWindow( hWnd, SW_SHOW);
//#endif
}



/***************************************************************************
 * Function    : InterfaceEndRender
 ***************************************************************************/
INTERFACE_API void InterfaceEndRender( void)
{
	InterfaceInstanceMain()->EndRender();

}
/***************************************************************************
 * Function    : InterfaceEndRender
 ***************************************************************************/
void InterfaceInstance::EndRender( void)
{
	if ( mpInterfaceInternals->mpInterfaceD3DDevice )
	{
	    // End the scene
		mpInterfaceInternals->mpInterfaceD3DDevice->EndScene();
	}
	mbIsInScene = false;

} 

/***************************************************************************
 * Function    : InterfaceBeginRender
 ***************************************************************************/
INTERFACE_API void InterfaceBeginRender( void)
{
	InterfaceInstanceMain()->BeginRender();
}

/***************************************************************************
 * Function    : InterfaceBeginRender
 ***************************************************************************/
void InterfaceInstance::BeginRender( void)
{
	if ( mpInterfaceInternals->mpInterfaceD3DDevice )
	{
	    // Begin the scene
		mpInterfaceInternals->mpInterfaceD3DDevice->BeginScene();
	}
	mbIsInScene = true;
 } 

INTERFACE_API BOOL	InterfaceIsInRender( void )
{
	return( InterfaceInstanceMain()->IsInRender() );
}


BOOL	InterfaceInstance::IsInRender( void )
{
	if ( mbIsInScene == true )
	{
		return( TRUE );
	}
	return( FALSE );
}

BOOL		mbInterfaceIsUsingDefaultFonts = TRUE;

void		InterfaceSetIsUsingDefaultFonts( BOOL bUsingDefaultFonts )
{
	mbInterfaceIsUsingDefaultFonts = bUsingDefaultFonts;
}

/***************************************************************************
 * Function    : InterfaceNewFrame
 ***************************************************************************/
INTERFACE_API int InterfaceNewFrame( uint32 ulCol )
{
	return( InterfaceInstanceMain()->NewFrame( ulCol ) );
}

int InterfaceInstance::NewFrame( uint32 ulCol )
{
D3DCOLOR xColor;
BOOL	boSmallQuit = FALSE;
BOOL	boIsSmall = InterfaceIsSmall();

	// If we're not in small mode
	if ( InterfaceIsSmall() != TRUE )
	{
		// If directX needs reinitialising (e.g resize, go to small.. etc. etc)
		if ( InterfaceDoesNeedChanging() == TRUE )
		{
			InterfaceReleaseForDeviceReset();
			InitD3D( mhWindow, mboMinPageSize);

			if ( mpInterfaceInternals->mpInterfaceD3DDevice != NULL )
			{
				// Clear the backbuffer and the zbuffer
				xColor = (D3DCOLOR)( ulCol );
				mpInterfaceInternals->mpInterfaceD3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL, xColor, 1.0f, 0 );
				mpInterfaceInternals->mpInterfaceD3DDevice->Present( NULL, NULL, NULL, NULL );
			}
			else
			{
				// Return a quit code
				return( -1 );
			}
			ShowWindow( mhWindow, SW_SHOW );
			UpdateWindow( mhWindow );
			SetCursor( LoadCursor(NULL, IDC_ARROW) );

			if ( mpInterfaceInternals->mpInterfaceD3DDevice != NULL )
			{
				// Clear the backbuffer and the zbuffer
				xColor = (D3DCOLOR)( ulCol );
				mpInterfaceInternals->mpInterfaceD3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL, xColor, 1.0f, 0 );
			}
			// Indicate that we have changed the interface

//			InterfaceSetGlobalParam( INTF_ANISOTROPIC, 0 );
//			InterfaceSetGlobalParam( INTF_TEXTURE_FILTERING, 0 );

			InterfaceRestorePostDeviceReset();
//			InterfaceInit( TRUE );
			return( 1 );
		}
	}

	if ( mpInterfaceInternals->mpInterfaceD3DDevice != NULL )
	{
	HRESULT	ret;
		
		// Clear the backbuffer and the zbuffer
		xColor = (D3DCOLOR)( ulCol );
		ret = mpInterfaceInternals->mpInterfaceD3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL, xColor, 1.0f, 0 );
	}
	return( 0 );
	
}

/***************************************************************************
 * Function    : InterfaceGetFPS
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
INTERFACE_API float	InterfaceGetFPS( void )
{
	return( mfInterfaceFPS );
}

#define		NUM_FRAME_TIMES		16
uint32 maulInterfaceLastFrameTimes[NUM_FRAME_TIMES] = { 0 };
int		mnInterfaceNextFrameTimeStore = 0;

/***************************************************************************
 * Function    : InterfacePresent
 * Description : DX9 Implementation
 ***************************************************************************/
INTERFACE_API void InterfacePresent( void )
{
	InterfaceInstanceMain()->Present();

}

void InterfaceInstance::Present( void )
{
HRESULT	hr;

	if ( mpInterfaceInternals->mpInterfaceD3DDevice )
	{
		hr = mpInterfaceInternals->mpInterfaceD3DDevice->Present( NULL, NULL, NULL, NULL );
		// Show the results
		if ( hr == D3DERR_DEVICELOST )
		{
			if ( mpInterfaceInternals->mpInterfaceD3DDevice->TestCooperativeLevel() != D3DERR_DEVICELOST )
			{
				InterfaceSetWindowHasChanged( TRUE );
			}
		}
	}

	// update the timer
	u64		ullThisTick = SysGetMicrosecondTick();
	if ( mullInterfaceLastPresentTick == 0 )
	{
		mullInterfaceLastPresentTick = ullThisTick;
	}
	else
	{
	uint32	ulFrameTime = (uint32)(ullThisTick - mullInterfaceLastPresentTick);

		if ( ulFrameTime > 0 )
		{
		int		nLoop;
		uint32	ulAverageFrameTime = 0;

			maulInterfaceLastFrameTimes[mnInterfaceNextFrameTimeStore] = ulFrameTime;
			mnInterfaceNextFrameTimeStore = (mnInterfaceNextFrameTimeStore+1) % NUM_FRAME_TIMES;
			for ( nLoop = 0; nLoop < NUM_FRAME_TIMES; nLoop++ )
			{
				ulAverageFrameTime += maulInterfaceLastFrameTimes[nLoop];
			}
			ulAverageFrameTime /= NUM_FRAME_TIMES;
			mfInterfaceFPS = 1000000.0f / (float)( ulAverageFrameTime );		// convert from ms per frame to frames per second
		}
		mullInterfaceLastPresentTick = ullThisTick;
	}
}
