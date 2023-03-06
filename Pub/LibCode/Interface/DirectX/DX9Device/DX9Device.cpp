
#include "../InterfaceInternalsDX.h"

#include <stdio.h>
#include <mmsystem.h>

#include <StandardDef.h>
#include <Interface.h>
#include <Engine.h>

#include "../../Win32/Interface-Win32.h"
#include "../../Common/InterfaceDevice.h"
#include "../../Common/InterfaceCommon.h"
#include "../../../Engine/DirectX/OculusDX.h"
#include "../../../Engine/DirectX/EngineDX.h"
#include "DX9Device.h"

extern "C"
{
#include "../../TempInterfaceResourceList.h"		// Don't even think about asking what this is about.
}

#include "d3dapp.h"


LPGRAPHICSDEVICE       mpInterfaceD3DDevice = NULL; // Our rendering device

LPGRAPHICS        mpD3D       = NULL; // Used to create the D3DDevice

BOOL	mboMinPageSize = TRUE;
BOOL	mboAnistropic = TRUE;
BOOL	msbInterfaceGlobalTextureFilteringEnable = TRUE;

int		mnRenderSurfaceWidth = 900;
int		mnRenderSurfaceHeight = 700;  

u64		mullInterfaceLastPresentTick = 0;

BOOL	mboCurrentlyFullscreen = FALSE;
BOOL	mboZBufferLockable = TRUE;
bool	mboDidLimitTo900ByX = false;
bool	mboDidLimitTo900ByY = false;
float	mfInterfaceFPS = 0.0f;

//#ifndef STANDALONE
static CD3DApplication		mcd3dUtilApp;
//#endif
ENGINEMATERIAL mxStandardMat;



INTERFACE_API LPGRAPHICS	InterfaceGetD3D( void )
{
	return( mpD3D );
}

INTERFACE_API BOOL InterfaceIsZBufferLockable( void )
{
	return( mboZBufferLockable);
}

INTERFACE_API void InterfaceSetZBufferLockable( BOOL boFlag )
{
	mboZBufferLockable = boFlag;
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
	if ( mnInterfaceDrawWidth != 0 )
	{
		return( mnInterfaceDrawWidth );
	}
	return( mnRenderSurfaceWidth );
}
/***************************************************************************
 * Function    : InterfaceGetHeight
 ***************************************************************************/
INTERFACE_API int InterfaceGetHeight( void )
{
	if ( mnInterfaceDrawHeight != 0 )
	{
		return( mnInterfaceDrawHeight );
	}
	return( mnRenderSurfaceHeight );
}








BOOL InterfaceShowFullscreenPanic( char* szErrorString )
{	
MSG msg;
ulong		ulThisTickGap = 0;
ulong		ulLastTick = GetTickCount();

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

INTERFACE_API void				InterfaceSetD3DDevice( LPGRAPHICSDEVICE pDevice )
{
	mpInterfaceD3DDevice = pDevice;

#ifdef TUD9
	if ( pDevice )
	{
		mpInterfaceD3DDevice->GetDeviceCaps( &md3dCaps );
		mbCapsIsSet = TRUE;
	}
#endif
}

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

bool	mboTextureFilteringCurrentState = false;

void InterfaceTurnOffTextureFiltering( void )
{
	if ( mboTextureFilteringCurrentState )
	{
		mpInterfaceD3DDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER,   D3DTEXF_POINT );
		mpInterfaceD3DDevice->SetSamplerState( 0, D3DSAMP_MINFILTER,   D3DTEXF_POINT  );
		mpInterfaceD3DDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_POINT );	
		mpInterfaceD3DDevice->SetSamplerState( 0, D3DSAMP_MAXANISOTROPY, 1 );
		mboTextureFilteringCurrentState = false;
	}
}

void InterfaceEnableTextureFiltering( BOOL bFlag )
{
	msbInterfaceGlobalTextureFilteringEnable = bFlag;
}

void InterfaceTurnOnTextureFiltering( int nMode )
{
	// Turning on normally.. may be overriden by graphic options
	if ( nMode == 1 )
	{
		if ( msbInterfaceGlobalTextureFilteringEnable == FALSE )
		{
			return;
		}
	}

	if (! mpInterfaceD3DDevice)
		return;
   
	if ( !mboTextureFilteringCurrentState )
	{
		if ( ( mboAnistropic == TRUE ) &&
			 ( InterfaceGetDeviceCaps( MAX_ANISTROPY ) >= 1 ) )
		{
			mpInterfaceD3DDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC  );
			mpInterfaceD3DDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC  );
			mpInterfaceD3DDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER, mnMipFilter );	
	//		mpInterfaceD3DDevice->SetTextureStageState( 0, D3DTSS_MIPFILTER, D3DTEXF_ANISOTROPIC);	
			mpInterfaceD3DDevice->SetSamplerState( 0, D3DSAMP_MAXANISOTROPY, 10 );
		}
		else
		{ 
			mpInterfaceD3DDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, mnMagFilter );
			mpInterfaceD3DDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, mnMinFilter );
			mpInterfaceD3DDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER, mnMipFilter );//D3DTEXF_GAUSSIANQUAD );// mnMipFilter );	
		}
		mboTextureFilteringCurrentState = true;
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

BOOL		InterfaceGetDXDeviceCreateParams( BOOL boMinPageSize, D3DPRESENT_PARAMETERS* pD3Dpp )
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
		InterfaceSetWindowStyle( true );
	}
	else
	{
		// Set windowed-mode style
		InterfaceSetWindowStyle( false );
		pD3Dpp->Windowed = TRUE;
		pD3Dpp->BackBufferWidth  = mnWindowWidth;
		pD3Dpp->BackBufferHeight = mnWindowHeight;

		if ( boMinPageSize == TRUE )
		{
			if ( pD3Dpp->BackBufferWidth < 900 )
			{
				pD3Dpp->BackBufferWidth  = 900;
				mboDidLimitTo900ByX = true;
			}
			if ( pD3Dpp->BackBufferHeight < 700 )
			{
				pD3Dpp->BackBufferHeight = 700;
				mboDidLimitTo900ByY = true;
			}
		}
	}

	if ( msnInterfaceMaxRenderPageWidth != 0 )
	{
		if ( pD3Dpp->BackBufferWidth > (ulong)msnInterfaceMaxRenderPageWidth )
		{
		float	fAdjust = ( (float)pD3Dpp->BackBufferWidth ) / msnInterfaceMaxRenderPageWidth;

			 pD3Dpp->BackBufferWidth = msnInterfaceMaxRenderPageWidth;
			 pD3Dpp->BackBufferHeight = (ulong)( pD3Dpp->BackBufferHeight / fAdjust );
			if ( boMinPageSize == TRUE )
			{
				if ( pD3Dpp->BackBufferWidth < 900 )
				{
					pD3Dpp->BackBufferWidth  = 900;
					mboDidLimitTo900ByX = true;
				}
				if ( pD3Dpp->BackBufferHeight < 700 )
				{
					pD3Dpp->BackBufferHeight = 700;
					mboDidLimitTo900ByY = true;
				}
			}
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

	mnRenderSurfaceWidth  = pD3Dpp->BackBufferWidth;
	mnRenderSurfaceHeight = pD3Dpp->BackBufferHeight;

	mboZBufferLockable = FALSE;

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
	pD3Dpp->hDeviceWindow          = mhwndInterfaceMain;
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
 * Function    : InterfaceInitD3D
 * Params      : 
 * Returns     :
 * Description : 
 ***************************************************************************/
INTERFACE_API LPGRAPHICSDEVICE InterfaceInitD3D( BOOL boMinPageSize )
{
	if ( InterfaceIsVRMode() == TRUE )
//		 ( EngineHasOculus() == TRUE ) )
	{
		if ( mpD3D == NULL )
		{
			mpD3D = Direct3DCreate9( D3D_SDK_VERSION );
			// Create the D3D object.
			if( mpD3D == NULL )
			{
				// Fallback to 9.0b
				mpD3D = Direct3DCreate9( D3D9b_SDK_VERSION );
				if( mpD3D == NULL )
				{
					PANIC_IF(TRUE,"Direct3D Create Failed. You may need to update your DirectX runtime and/or video card drivers for this game to work" );
					return( NULL );
				}
			}

			mpInterfaceD3DDevice = (LPGRAPHICSDEVICE) OculusInitD3DDevice( mpD3D );
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
			mpD3D = Direct3DCreate9( D3D_SDK_VERSION );
			// Create the D3D object.
			if( mpD3D == NULL )
			{
				// Fallback to 9.0b
				mpD3D = Direct3DCreate9( D3D9b_SDK_VERSION );
				if( mpD3D == NULL )
				{
					PANIC_IF(TRUE,"Direct3D Create Failed. You may need to update your DirectX runtime and/or video card drivers for this game to work" );
					return( NULL );
				}
			}
		}

		// Get the current desktop display mode, so we can set up a back
		// buffer of the same format
		if( FAILED( mpD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d3ddm ) ) )
		{
			PANIC_IF(TRUE,"Couldn't get display mode" );
			return( NULL );
		}

		// Set up the present parameters - This is generally what odd vid cards have a problem with
		D3DPRESENT_PARAMETERS d3dpp;
		ZeroMemory( &d3dpp, sizeof(d3dpp) );

		InterfaceGetDXDeviceCreateParams( boMinPageSize, &d3dpp );

		if ( mpInterfaceD3DDevice == NULL )
		{
		int		nAdapterToUse = D3DADAPTER_DEFAULT;

//			hr = mpD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, mhwndInterfaceMain,D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &mpInterfaceD3DDevice );
			hr = mpD3D->CreateDevice( nAdapterToUse, D3DDEVTYPE_HAL, mhwndInterfaceMain,D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &mpInterfaceD3DDevice );								  

			d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;

			if ( hr == D3DERR_OUTOFVIDEOMEMORY )
			{
				if ( mboDidLimitTo900ByX == true )
				{
					d3dpp.BackBufferWidth  = 800;
					mnRenderSurfaceWidth  = d3dpp.BackBufferWidth;
				}
				if ( mboDidLimitTo900ByY == true )
				{
					d3dpp.BackBufferHeight = 600;
					mnRenderSurfaceHeight = d3dpp.BackBufferHeight;
				}

				hr = mpD3D->CreateDevice( nAdapterToUse, D3DDEVTYPE_HAL, mhwndInterfaceMain,
										  D3DCREATE_SOFTWARE_VERTEXPROCESSING/*|D3DCREATE_MULTITHREADED*/,
										  &d3dpp, &mpInterfaceD3DDevice );
				if ( FAILED(hr) )
				{
					// Using 'Discard' can save us a bunch of vid mem apparently.. 
					// so try to create the screen using this
					d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
					hr = mpD3D->CreateDevice( nAdapterToUse, D3DDEVTYPE_HAL, mhwndInterfaceMain,
											  D3DCREATE_SOFTWARE_VERTEXPROCESSING/*|D3DCREATE_MULTITHREADED*/,
											  &d3dpp, &mpInterfaceD3DDevice );
				}
			}

			if( FAILED( hr ) )
			{	
				InterfaceSetWindowStyle( false );
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
				return( NULL );
			}
			hr = mpInterfaceD3DDevice->Reset( &d3dpp );
			InterfaceSetD3DDevice(mpInterfaceD3DDevice);
		}
		else
		{	
			// Clear the backbuffer and the zbuffer
			mpInterfaceD3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL,0, 1.0f, 0 );
			mpInterfaceD3DDevice->Present( NULL, NULL, NULL, NULL );
		
			mboZBufferLockable = TRUE;
			hr = mpInterfaceD3DDevice->Reset( &d3dpp );

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

				
				PANIC_IF( TRUE, acErrorMsg );

				InterfaceSetWindowStyle( false );
				InterfaceInitSmall();
				return( NULL );
			}		
		}

		// Resize the window if we're in windowed mode and we have just left fullscreen
		if( ( mboFullScreen == FALSE ) &&
			( mboCurrentlyFullscreen == TRUE ) )
		{	
			SetWindowPos( mhwndInterfaceMain, HWND_NOTOPMOST,
							mnWindowLeft, mnWindowTop,
							mnWindowWidth, mnWindowHeight,
							SWP_SHOWWINDOW );
		}

		mboCurrentlyFullscreen = mboFullScreen;

		if ( mpInterfaceD3DDevice != NULL )
		{
		float	fBias = 0.0f;
	/*	D3DCAPS8	d3dCaps;

			mpInterfaceD3DDevice->GetDeviceCaps( &d3dCaps );
	*/
			// Set standard render modes bit
			mpInterfaceD3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
			mpInterfaceD3DDevice->SetRenderState( D3DRS_ZENABLE, TRUE );
			mpInterfaceD3DDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
			if ( bFullScreenAntiAlias )
			{
				mpInterfaceD3DDevice->SetRenderState( D3DRS_MULTISAMPLEANTIALIAS, TRUE );
			}
			InterfaceSetWindowHasChanged( FALSE );

			mpInterfaceD3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL,
									 0, 1.0f, 0 );
			mpInterfaceD3DDevice->Present( NULL, NULL, NULL, NULL );

			fBias = mfMipMapBias;
			mpInterfaceD3DDevice->SetSamplerState(0, D3DSAMP_MIPMAPLODBIAS, *((LPDWORD) (&fBias)));

			D3DXMATRIX matTrans;
			D3DXMatrixIdentity( &matTrans );
			// Set-up the matrix for the desired transformation.
			mpInterfaceD3DDevice->SetTransform( D3DTS_TEXTURE0, &matTrans );
			mpInterfaceD3DDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
		}

		EngineInitDX( mpInterfaceD3DDevice );
		InterfaceSetStandardMaterial();
	}
    return( mpInterfaceD3DDevice );
}


INTERFACE_API void				InterfaceInitDisplayDevice( BOOL boMinRenderPageSize )
{
	InterfaceInitD3D( boMinRenderPageSize );
}


INTERFACE_API INT_PTR CALLBACK InterfaceVidOptionsDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
//#ifndef STANDALONE
	return mcd3dUtilApp.SelectDeviceProc( hDlg, msg, wParam, lParam );
//#else
//	return( 0 );
//#endif
}


ulong	mulRefCount;
INTERFACE_API void InterfaceFreeAllD3D( void )
{
//ulong	ulRefCount;
    if( mpInterfaceD3DDevice != NULL )
	{
		mpInterfaceD3DDevice->SetTexture(0, NULL);
		mpInterfaceD3DDevice->SetTexture(1, NULL);
		mpInterfaceD3DDevice->EvictManagedResources();
		mulRefCount = mpInterfaceD3DDevice->Release();
		mpInterfaceD3DDevice = NULL;
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



bool	mbIsInScene = false;

/***************************************************************************
 * Function    : InterfaceEndRender
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
INTERFACE_API void InterfaceEndRender( void)
{
	if ( mpInterfaceD3DDevice )
	{
	    // End the scene
		mpInterfaceD3DDevice->EndScene();
	}
	mbIsInScene = false;

} 


/***************************************************************************
 * Function    : InterfaceBeginRender
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
INTERFACE_API void InterfaceBeginRender( void)
{
	if ( mpInterfaceD3DDevice )
	{
		if ( mbIsInScene == true )
		{
		int		nBreak = 0;
			nBreak++;
		}
	    // Begin the scene
		mpInterfaceD3DDevice->BeginScene();
	}
	mbIsInScene = true;
 } 

INTERFACE_API BOOL	InterfaceIsInRender( void )
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
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
INTERFACE_API int InterfaceNewFrame( ulong ulCol )
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
			InterfaceSetD3DDevice( InterfaceInitD3D(mboMinPageSize) );

			if ( mpInterfaceD3DDevice != NULL )
			{
				// Clear the backbuffer and the zbuffer
				xColor = (D3DCOLOR)( ulCol );
				mpInterfaceD3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL, xColor, 1.0f, 0 );
				mpInterfaceD3DDevice->Present( NULL, NULL, NULL, NULL );
			}
			else
			{
				// Return a quit code
				return( -1 );
			}
			ShowWindow( mhwndInterfaceMain, SW_SHOW );
			UpdateWindow( mhwndInterfaceMain );
			SetCursor( LoadCursor(NULL, IDC_ARROW) );

			if ( mpInterfaceD3DDevice != NULL )
			{
				// Clear the backbuffer and the zbuffer
				xColor = (D3DCOLOR)( ulCol );
				mpInterfaceD3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL, xColor, 1.0f, 0 );
			}
			// Indicate that we have changed the interface

//			InterfaceSetGlobalParam( INTF_ANISOTROPIC, 0 );
//			InterfaceSetGlobalParam( INTF_TEXTURE_FILTERING, 0 );

			InterfaceRestorePostDeviceReset();
//			InterfaceInit( TRUE );
			return( 1 );
		}
	}

	if ( mpInterfaceD3DDevice != NULL )
	{
	HRESULT	ret;
		
		// Clear the backbuffer and the zbuffer
		xColor = (D3DCOLOR)( ulCol );
		ret = mpInterfaceD3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL, xColor, 1.0f, 0 );
		if ( ret != D3D_OK )
		{
		int	nBreak = 0;
			nBreak++;
		}
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
ulong maulInterfaceLastFrameTimes[NUM_FRAME_TIMES] = { 0 };
int		mnInterfaceNextFrameTimeStore = 0;

/***************************************************************************
 * Function    : InterfacePresent
 * Description : DX9 Implementation
 ***************************************************************************/
INTERFACE_API void InterfacePresent( void )
{
HRESULT	hr;

	if ( mpInterfaceD3DDevice )
	{
		hr = mpInterfaceD3DDevice->Present( NULL, NULL, NULL, NULL );
		// Show the results
		if ( hr == D3DERR_DEVICELOST )
		{
			if ( mpInterfaceD3DDevice->TestCooperativeLevel() != D3DERR_DEVICELOST )
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
	ulong	ulFrameTime = (ulong)(ullThisTick - mullInterfaceLastPresentTick);

		if ( ulFrameTime > 0 )
		{
		int		nLoop;
		ulong	ulAverageFrameTime = 0;

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
