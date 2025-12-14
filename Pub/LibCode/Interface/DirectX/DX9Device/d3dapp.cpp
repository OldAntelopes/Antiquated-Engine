//#ifndef STANDALONE

//-----------------------------------------------------------------------------
// File: D3DApp.cpp
//
// Desc: Application class for the Direct3D samples framework library.
//
// Copyright (c) 1998-2000 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <tchar.h>

#include "../InterfaceInternalsDx.h"

#include <StandardDef.h>
#include <Interface.h>


#include "D3DApp.h"
//#include "D3DUtil.h"
#include "DXUtil.h"

#include "../../Common/InterfaceOptions.h"
#include "../../TempInterfaceResourceList.h"
//#include "D3DRes.h"

#include "DX9Device.h"
#include "../../Win32/Interface-Win32.h"


//--------------------------------------------------------------------------------------------------------------

int		mnTextureFilteringFlag;
int		mnBackBufferFlag;
int		mnVsync;
int		mnOldStartup;
int		mnMinimumSurfaceRes;
int		mnFogMode;


//-----------------------------------------------------------------------------
// Global access to the app (needed for the global WndProc())
//-----------------------------------------------------------------------------
static CD3DApplication* g_pD3DApp = NULL;




//-----------------------------------------------------------------------------
// Name: CD3DApplication()
// Desc: Constructor
//-----------------------------------------------------------------------------
CD3DApplication::CD3DApplication()
{
    g_pD3DApp           = this;

    m_dwNumAdapters     = 0;
    m_dwAdapter         = 0L;
    m_pD3D              = NULL;
    m_pd3dDevice        = NULL;
    m_hWnd              = NULL;
    m_hWndFocus         = NULL;
    m_bActive           = FALSE;
    m_bReady            = FALSE;
    m_dwCreateFlags     = 0L;

    m_bFrameMoving      = TRUE;
    m_bSingleStep       = FALSE;
    m_fFPS              = 0.0f;
    m_strDeviceStats[0] = _T('\0');
    m_strFrameStats[0]  = _T('\0');

    m_strWindowTitle    = _T("D3D8 Application");
    m_dwCreationWidth   = 400;
    m_dwCreationHeight  = 300;
    m_bUseDepthBuffer   = TRUE;
    m_dwMinDepthBits    = 16;
    m_dwMinStencilBits  = 0;
    m_bShowCursorWhenFullscreen = TRUE;
}







//-----------------------------------------------------------------------------
// Name: Create()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CD3DApplication::InitD3DMinimal( HWND hwind )
{
HRESULT hr;

    // Create the Direct3D object
#ifdef USE_D3DEX_INTERFACE
	Direct3DCreate9Ex( D3D_SDK_VERSION, &m_pD3D );
#else
	m_pD3D = Direct3DCreate9( D3D_SDK_VERSION );
#endif
    if( m_pD3D == NULL )
	{
		// Try to start with the old vers
#ifdef USE_D3DEX_INTERFACE
		Direct3DCreate9Ex( D3D9b_SDK_VERSION, &m_pD3D );
#else
		m_pD3D = Direct3DCreate9( D3D9b_SDK_VERSION );
#endif
	    if( m_pD3D == NULL )
		{
			return DisplayErrorMsg( D3DAPPERR_NODIRECT3D, MSGERR_APPMUSTEXIT );
		}
	}


    // Build a list of Direct3D adapters, modes and devices. The
    // ConfirmDevice() callback is used to confirm that only devices that
    // meet the app's requirements are considered.
    if( FAILED( hr = BuildDeviceList() ) )
    {
        SAFE_RELEASE( m_pD3D );
        return DisplayErrorMsg( hr, MSGERR_APPMUSTEXIT );
    }

	m_hWnd = hwind;

	return( S_OK );
}

//-----------------------------------------------------------------------------
// Name: Create()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CD3DApplication::Create( HWND hwind )
{
HRESULT hr;

	InitD3DMinimal( hwind );

    // The focus window can be a specified to be a different window than the
    // device window.  If not, use the device window as the focus window.
    if( m_hWndFocus == NULL )
        m_hWndFocus = m_hWnd;

    // Save window properties
	m_dwWindowStyle = WS_CAPTION|WS_SYSMENU|WS_THICKFRAME|WS_MINIMIZEBOX|WS_MAXIMIZEBOX|WS_VISIBLE;
    GetWindowRect( m_hWnd, &m_rcWindowBounds );
    GetClientRect( m_hWnd, &m_rcWindowClient );

    // Initialize the 3D environment for the app
    if( FAILED( hr = Initialize3DEnvironment() ) )
    {
        SAFE_RELEASE( m_pD3D );
        return DisplayErrorMsg( hr, MSGERR_APPMUSTEXIT );
    }

    // The app is ready to go
    m_bReady = TRUE;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: SortModesCallback()
// Desc: Callback function for sorting display modes (used by BuildDeviceList).
//-----------------------------------------------------------------------------
int SortModesCallback( const VOID* arg1, const VOID* arg2 )
{
    D3DDISPLAYMODE* p1 = (D3DDISPLAYMODE*)arg1;
    D3DDISPLAYMODE* p2 = (D3DDISPLAYMODE*)arg2;

    if( p1->Format > p2->Format )   return -1;
    if( p1->Format < p2->Format )   return +1;
    if( p1->Width  < p2->Width )    return -1;
    if( p1->Width  > p2->Width )    return +1;
    if( p1->Height < p2->Height )   return -1;
    if( p1->Height > p2->Height )   return +1;

    return 0;
}




//-----------------------------------------------------------------------------
// Name: BuildDeviceList()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CD3DApplication::BuildDeviceList()
{
    const DWORD dwNumDeviceTypes = 2;
    const TCHAR* strDeviceDescs[] = { _T("HAL"), _T("REF") };
    const D3DDEVTYPE DeviceTypes[] = { D3DDEVTYPE_HAL, D3DDEVTYPE_REF };

    BOOL bHALExists = FALSE;
    BOOL bHALIsWindowedCompatible = FALSE;
    BOOL bHALIsDesktopCompatible = FALSE;
    BOOL bHALIsSampleCompatible = FALSE;

	m_bWindowed = !mboFullScreen;

    // Loop through all the adapters on the system (usually, there's just one
    // unless more than one graphics card is present).
    for( UINT iAdapter = 0; iAdapter < m_pD3D->GetAdapterCount(); iAdapter++ )
    {
        // Fill in adapter info
        D3DAdapterInfo* pAdapter  = &m_Adapters[m_dwNumAdapters];
        m_pD3D->GetAdapterIdentifier( iAdapter, 0, &pAdapter->d3dAdapterIdentifier );
        m_pD3D->GetAdapterDisplayMode( iAdapter, &pAdapter->d3ddmDesktop );
        pAdapter->dwNumDevices    = 0;
        pAdapter->dwCurrentDevice = 0;

        // Enumerate all display modes on this adapter
        D3DDISPLAYMODE modes[100];
        D3DFORMAT      formats[20];
        DWORD dwNumFormats      = 0;
        DWORD dwNumModes        = 0;
#ifdef TUD9
        DWORD dwNumAdapterModes = m_pD3D->GetAdapterModeCount( iAdapter, pAdapter->d3ddmDesktop.Format );
#else
		DWORD dwNumAdapterModes = m_pD3D->GetAdapterModeCount( iAdapter );
#endif
        // Add the adapter's current desktop format to the list of formats
        formats[dwNumFormats++] = pAdapter->d3ddmDesktop.Format;

        for( UINT iMode = 0; iMode < dwNumAdapterModes; iMode++ )
        {
		DWORD m;
            // Get the display mode attributes
            D3DDISPLAYMODE DisplayMode;
#ifdef TUD9
            m_pD3D->EnumAdapterModes( iAdapter,pAdapter->d3ddmDesktop.Format, iMode, &DisplayMode );
#else
            m_pD3D->EnumAdapterModes( iAdapter, iMode, &DisplayMode );
#endif
            // Filter out low-resolution modes
            if( DisplayMode.Width  < 800 || DisplayMode.Height < 600 )
                continue;

            // Check if the mode already exists (to filter out refresh rates)
            for( m=0L; m<dwNumModes; m++ )
            {
                if( ( modes[m].Width  == DisplayMode.Width  ) &&
                    ( modes[m].Height == DisplayMode.Height ) &&
                    ( modes[m].Format == DisplayMode.Format ) )
                    break;
            }

            // If we found a new mode, add it to the list of modes
            if( m == dwNumModes )
            {
			DWORD f;

                modes[dwNumModes].Width       = DisplayMode.Width;
                modes[dwNumModes].Height      = DisplayMode.Height;
                modes[dwNumModes].Format      = DisplayMode.Format;
                modes[dwNumModes].RefreshRate = 0;
                dwNumModes++;

                // Check if the mode's format already exists
                for(  f=0; f<dwNumFormats; f++ )
                {
                    if( DisplayMode.Format == formats[f] )
                        break;
                }

                // If the format is new, add it to the list
                if( f== dwNumFormats )
                    formats[dwNumFormats++] = DisplayMode.Format;
            }
        }

        // Sort the list of display modes (by format, then width, then height)
        qsort( modes, dwNumModes, sizeof(D3DDISPLAYMODE), SortModesCallback );

        // Add devices to adapter
        for( UINT iDevice = 0; iDevice < dwNumDeviceTypes; iDevice++ )
        {
            // Fill in device info
            D3DDeviceInfo* pDevice;
            pDevice                 = &pAdapter->devices[pAdapter->dwNumDevices];
            pDevice->DeviceType     = DeviceTypes[iDevice];
            m_pD3D->GetDeviceCaps( iAdapter, DeviceTypes[iDevice], &pDevice->d3dCaps );
            pDevice->strDesc        = strDeviceDescs[iDevice];
            pDevice->dwNumModes     = 0;
            pDevice->dwCurrentMode  = 0;
            pDevice->bCanDoWindowed = FALSE;
            pDevice->bWindowed      = FALSE;
            pDevice->MultiSampleType = D3DMULTISAMPLE_NONE;

            // Examine each format supported by the adapter to see if it will
            // work with this device and meets the needs of the application.
            BOOL  bFormatConfirmed[20];
            DWORD dwBehavior[20];
            D3DFORMAT fmtDepthStencil[20];
			DWORD f;
			DWORD m;

            for( f=0; f<dwNumFormats; f++ )
            {
                bFormatConfirmed[f] = FALSE;
                fmtDepthStencil[f] = D3DFMT_UNKNOWN;

                // Skip formats that cannot be used as render targets on this device
                if( FAILED( m_pD3D->CheckDeviceType( iAdapter, pDevice->DeviceType,
                                                     formats[f], formats[f], FALSE ) ) )
                    continue;

                if( pDevice->DeviceType == D3DDEVTYPE_HAL )
                {
                    // This system has a HAL device
                    bHALExists = TRUE;

#ifdef TUD9
                    if( 1 )
#else
                    if( pDevice->d3dCaps.Caps2 & D3DCAPS2_CANRENDERWINDOWED )
#endif
                    {
                        // HAL can run in a window for some mode
                        bHALIsWindowedCompatible = TRUE;

                        if( f == 0 )
                        {
                            // HAL can run in a window for the current desktop mode
                            bHALIsDesktopCompatible = TRUE;
                        }
                    }
                }

                // Confirm the device/format for HW vertex processing
                if( pDevice->d3dCaps.DevCaps&D3DDEVCAPS_HWTRANSFORMANDLIGHT )
                {
                    if( pDevice->d3dCaps.DevCaps&D3DDEVCAPS_PUREDEVICE )
                    {
                        dwBehavior[f] = D3DCREATE_HARDWARE_VERTEXPROCESSING |
                                        D3DCREATE_PUREDEVICE;

                        if( SUCCEEDED( ConfirmDevice( &pDevice->d3dCaps, dwBehavior[f],
                                                      formats[f] ) ) )
                            bFormatConfirmed[f] = TRUE;
                    }

                    if ( FALSE == bFormatConfirmed[f] )
                    {
                        dwBehavior[f] = D3DCREATE_HARDWARE_VERTEXPROCESSING;

                        if( SUCCEEDED( ConfirmDevice( &pDevice->d3dCaps, dwBehavior[f],
                                                      formats[f] ) ) )
                            bFormatConfirmed[f] = TRUE;
                    }

                    if ( FALSE == bFormatConfirmed[f] )
                    {
                        dwBehavior[f] = D3DCREATE_MIXED_VERTEXPROCESSING;

                        if( SUCCEEDED( ConfirmDevice( &pDevice->d3dCaps, dwBehavior[f],
                                                      formats[f] ) ) )
                            bFormatConfirmed[f] = TRUE;
                    }
                }

                // Confirm the device/format for SW vertex processing
                if( FALSE == bFormatConfirmed[f] )
                {
                    dwBehavior[f] = D3DCREATE_SOFTWARE_VERTEXPROCESSING;

                    if( SUCCEEDED( ConfirmDevice( &pDevice->d3dCaps, dwBehavior[f],
                                                  formats[f] ) ) )
                        bFormatConfirmed[f] = TRUE;
                }

                // Find a suitable depth/stencil buffer format for this device/format
                if( bFormatConfirmed[f] && m_bUseDepthBuffer )
                {
                    if( !FindDepthStencilFormat( iAdapter, pDevice->DeviceType,
                        formats[f], &fmtDepthStencil[f] ) )
                    {
                        bFormatConfirmed[f] = FALSE;
                    }
                }
            }
            // Add all enumerated display modes with confirmed formats to the
            // device's list of valid modes
            for( m=0L; m<dwNumModes; m++ )
            {
                for( f=0; f<dwNumFormats; f++ )
                {
                    if( modes[m].Format == formats[f] )
                    {
                        if( bFormatConfirmed[f] == TRUE )
                        {
                            // Add this mode to the device's list of valid modes
                            pDevice->modes[pDevice->dwNumModes].Width      = modes[m].Width;
                            pDevice->modes[pDevice->dwNumModes].Height     = modes[m].Height;
                            pDevice->modes[pDevice->dwNumModes].Format     = modes[m].Format;
                            pDevice->modes[pDevice->dwNumModes].dwBehavior = dwBehavior[f];
                            pDevice->modes[pDevice->dwNumModes].DepthStencilFormat = fmtDepthStencil[f];
                            pDevice->dwNumModes++;

                            if( pDevice->DeviceType == D3DDEVTYPE_HAL )
                                bHALIsSampleCompatible = TRUE;
                        }
                    }
                }
            }

            // Select any 640x480 mode for default (but prefer a 16-bit mode)
            for( m=0; m<pDevice->dwNumModes; m++ )
            {
			int		nInitialWidth, nInitialHeight;

				InterfaceGetInitialScreenSize( &nInitialWidth, &nInitialHeight );
                if( ( (int)(pDevice->modes[m].Width) == nInitialWidth) && ( (int)(pDevice->modes[m].Height) == nInitialHeight) )
                {
                    pDevice->dwCurrentMode = m;
                    if( pDevice->modes[m].Format == D3DFMT_R5G6B5 ||
                        pDevice->modes[m].Format == D3DFMT_X1R5G5B5 ||
                        pDevice->modes[m].Format == D3DFMT_A1R5G5B5 )
                    {
                        break;
                    }
                }
            }
			if ( m == pDevice->dwNumModes )
			{
				m = 0;
			}

            // Check if the device is compatible with the desktop display mode
           // (which was added initially as formats[0])
#ifdef TUD9
			pDevice->bCanDoWindowed = TRUE;
#else
            if( bFormatConfirmed[m] && (pDevice->d3dCaps.Caps2 & D3DCAPS2_CANRENDERWINDOWED) )
            {
                pDevice->bCanDoWindowed = TRUE;
            }
#endif
            pDevice->bWindowed      = m_bWindowed;

            // If valid modes were found, keep this device
            if( pDevice->dwNumModes > 0 )
                pAdapter->dwNumDevices++;
        }

        // If valid devices were found, keep this adapter
        if( pAdapter->dwNumDevices > 0 )
            m_dwNumAdapters++;
    }

    // Return an error if no compatible devices were found
    if( 0L == m_dwNumAdapters )
        return D3DAPPERR_NOCOMPATIBLEDEVICES;

    // Pick a default device that can render into a window
    // (This code assumes that the HAL device comes before the REF
    // device in the device array).
    for( DWORD a=0; a<m_dwNumAdapters; a++ )
    {
        for( DWORD d=0; d < m_Adapters[a].dwNumDevices; d++ )
        {
			if ( ( mboFullScreen ) || m_Adapters[a].devices[d].bWindowed )
            {
                m_Adapters[a].dwCurrentDevice = d;
                m_dwAdapter = a;

                // Display a warning message
                if( m_Adapters[a].devices[d].DeviceType == D3DDEVTYPE_REF )
                {
                    if( !bHALExists )
                        DisplayErrorMsg( D3DAPPERR_NOHARDWAREDEVICE, MSGWARN_SWITCHEDTOREF );
                    else if( !bHALIsSampleCompatible )
                        DisplayErrorMsg( D3DAPPERR_HALNOTCOMPATIBLE, MSGWARN_SWITCHEDTOREF );
                    else if( !bHALIsWindowedCompatible )
                        DisplayErrorMsg( D3DAPPERR_NOWINDOWEDHAL, MSGWARN_SWITCHEDTOREF );
                    else if( !bHALIsDesktopCompatible )
                        DisplayErrorMsg( D3DAPPERR_NODESKTOPHAL, MSGWARN_SWITCHEDTOREF );
                    else // HAL is desktop compatible, but not sample compatible
                        DisplayErrorMsg( D3DAPPERR_NOHALTHISMODE, MSGWARN_SWITCHEDTOREF );
                }

                return S_OK;
            }
        }
    }

    return D3DAPPERR_NOWINDOWABLEDEVICES;
}




//-----------------------------------------------------------------------------
// Name: FindDepthStencilFormat()
// Desc: Finds a depth/stencil format for the given device that is compatible
//       with the render target format and meets the needs of the app.
//-----------------------------------------------------------------------------
BOOL CD3DApplication::FindDepthStencilFormat( UINT iAdapter, D3DDEVTYPE DeviceType,
    D3DFORMAT TargetFormat, D3DFORMAT* pDepthStencilFormat )
{
    if( m_dwMinDepthBits <= 16 && m_dwMinStencilBits == 0 )
    {
        if( SUCCEEDED( m_pD3D->CheckDeviceFormat( iAdapter, DeviceType,
            TargetFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D16 ) ) )
        {
            if( SUCCEEDED( m_pD3D->CheckDepthStencilMatch( iAdapter, DeviceType,
                TargetFormat, TargetFormat, D3DFMT_D16 ) ) )
            {
                *pDepthStencilFormat = D3DFMT_D16;
                return TRUE;
            }
        }
    }

    if( m_dwMinDepthBits <= 15 && m_dwMinStencilBits <= 1 )
    {
        if( SUCCEEDED( m_pD3D->CheckDeviceFormat( iAdapter, DeviceType,
            TargetFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D15S1 ) ) )
        {
            if( SUCCEEDED( m_pD3D->CheckDepthStencilMatch( iAdapter, DeviceType,
                TargetFormat, TargetFormat, D3DFMT_D15S1 ) ) )
            {
                *pDepthStencilFormat = D3DFMT_D15S1;
                return TRUE;
            }
        }
    }

    if( m_dwMinDepthBits <= 24 && m_dwMinStencilBits == 0 )
    {
        if( SUCCEEDED( m_pD3D->CheckDeviceFormat( iAdapter, DeviceType,
            TargetFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24X8 ) ) )
        {
            if( SUCCEEDED( m_pD3D->CheckDepthStencilMatch( iAdapter, DeviceType,
                TargetFormat, TargetFormat, D3DFMT_D24X8 ) ) )
            {
                *pDepthStencilFormat = D3DFMT_D24X8;
                return TRUE;
            }
        }
    }

    if( m_dwMinDepthBits <= 24 && m_dwMinStencilBits <= 8 )
    {
        if( SUCCEEDED( m_pD3D->CheckDeviceFormat( iAdapter, DeviceType,
            TargetFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24S8 ) ) )
        {
            if( SUCCEEDED( m_pD3D->CheckDepthStencilMatch( iAdapter, DeviceType,
                TargetFormat, TargetFormat, D3DFMT_D24S8 ) ) )
            {
                *pDepthStencilFormat = D3DFMT_D24S8;
                return TRUE;
            }
        }
    }

    if( m_dwMinDepthBits <= 24 && m_dwMinStencilBits <= 4 )
    {
        if( SUCCEEDED( m_pD3D->CheckDeviceFormat( iAdapter, DeviceType,
            TargetFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24X4S4 ) ) )
        {
            if( SUCCEEDED( m_pD3D->CheckDepthStencilMatch( iAdapter, DeviceType,
                TargetFormat, TargetFormat, D3DFMT_D24X4S4 ) ) )
            {
                *pDepthStencilFormat = D3DFMT_D24X4S4;
                return TRUE;
            }
        }
    }

    if( m_dwMinDepthBits <= 32 && m_dwMinStencilBits == 0 )
    {
        if( SUCCEEDED( m_pD3D->CheckDeviceFormat( iAdapter, DeviceType,
            TargetFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D32 ) ) )
        {
            if( SUCCEEDED( m_pD3D->CheckDepthStencilMatch( iAdapter, DeviceType,
                TargetFormat, TargetFormat, D3DFMT_D32 ) ) )
            {
                *pDepthStencilFormat = D3DFMT_D32;
                return TRUE;
            }
        }
    }

    return FALSE;
}



//-----------------------------------------------------------------------------
// Name: Initialize3DEnvironment()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CD3DApplication::Initialize3DEnvironment()
{
    HRESULT hr;

    D3DAdapterInfo* pAdapterInfo = &m_Adapters[m_dwAdapter];
    D3DDeviceInfo*  pDeviceInfo  = &pAdapterInfo->devices[pAdapterInfo->dwCurrentDevice];
    D3DModeInfo*    pModeInfo    = &pDeviceInfo->modes[pDeviceInfo->dwCurrentMode];

    // Prepare window for possible windowed/fullscreen change
    AdjustWindowForChange();

    // Set up the presentation parameters
    ZeroMemory( &m_d3dpp, sizeof(m_d3dpp) );
    m_d3dpp.Windowed               = pDeviceInfo->bWindowed;
	
	if ( InterfaceGetOption( BACK_BUFFER ) == TRUE )
	{
		m_d3dpp.BackBufferCount        = 1;
	}
		
//    m_d3dpp.MultiSampleType        = pDeviceInfo->MultiSampleType;
	if ( InterfaceGetOption( VSYNC ) == TRUE )
	{
#ifdef TUD9
		m_d3dpp.SwapEffect             = D3DSWAPEFFECT_COPY;
		m_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

#else
		m_d3dpp.SwapEffect             = D3DSWAPEFFECT_COPY_VSYNC;
#endif
	}
	else
	{
		m_d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
	}

    m_d3dpp.EnableAutoDepthStencil = m_bUseDepthBuffer;
    m_d3dpp.AutoDepthStencilFormat = pModeInfo->DepthStencilFormat;
    m_d3dpp.hDeviceWindow          = m_hWnd;
	m_d3dpp.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
    if( m_bWindowed )
    {
        m_d3dpp.BackBufferWidth  = m_rcWindowClient.right - m_rcWindowClient.left;
        m_d3dpp.BackBufferHeight = m_rcWindowClient.bottom - m_rcWindowClient.top;
        m_d3dpp.BackBufferFormat = pAdapterInfo->d3ddmDesktop.Format;
    }
    else
    {
        m_d3dpp.BackBufferWidth  = pModeInfo->Width;
        m_d3dpp.BackBufferHeight = pModeInfo->Height;
        m_d3dpp.BackBufferFormat = pModeInfo->Format;
    }
	if ( InterfaceGetOption( MINIMUM_SURFACE_RES ) == TRUE )
	{
		if ( m_d3dpp.BackBufferWidth < 900 )
		{
			m_d3dpp.BackBufferWidth = 900;
		}
		if (  m_d3dpp.BackBufferHeight < 700 )
		{
			 m_d3dpp.BackBufferHeight = 700;
		}
	}


	if ( m_pd3dDevice == NULL )
	{
	    // Create the device
#ifdef USE_D3DEX_INTERFACE
		hr = m_pD3D->CreateDeviceEx( m_dwAdapter, pDeviceInfo->DeviceType,
                               m_hWndFocus, pModeInfo->dwBehavior, &m_d3dpp, NULL,
                               &m_pd3dDevice );
#else
		hr = m_pD3D->CreateDevice( m_dwAdapter, pDeviceInfo->DeviceType,
                               m_hWndFocus, pModeInfo->dwBehavior, &m_d3dpp, &m_pd3dDevice );
#endif
	}
	else
	{		
		hr = m_pd3dDevice->Reset( &m_d3dpp );
	}

    if( SUCCEEDED(hr) )
    {
        // When moving from fullscreen to windowed mode, it is important to
        // adjust the window size after recreating the device rather than
        // beforehand to ensure that you get the window size you want.  For
        // example, when switching from 640x480 fullscreen to windowed with
        // a 1000x600 window on a 1024x768 desktop, it is impossible to set
        // the window size to 1000x600 until after the display mode has
        // changed to 1024x768, because windows cannot be larger than the
        // desktop.
        if( m_bWindowed )
        {
            SetWindowPos( m_hWnd, HWND_NOTOPMOST,
                          m_rcWindowBounds.left, m_rcWindowBounds.top,
                          ( m_rcWindowBounds.right - m_rcWindowBounds.left ),
                          ( m_rcWindowBounds.bottom - m_rcWindowBounds.top ),
                          SWP_SHOWWINDOW );
        }

        // Store device Caps
        m_pd3dDevice->GetDeviceCaps( &m_d3dCaps );
        m_dwCreateFlags = pModeInfo->dwBehavior;

        // Store device description
        if( pDeviceInfo->DeviceType == D3DDEVTYPE_REF )
            lstrcpy( m_strDeviceStats, TEXT("REF") );
        else if( pDeviceInfo->DeviceType == D3DDEVTYPE_HAL )
            lstrcpy( m_strDeviceStats, TEXT("HAL") );
        else if( pDeviceInfo->DeviceType == D3DDEVTYPE_SW )
            lstrcpy( m_strDeviceStats, TEXT("SW") );

        if( pModeInfo->dwBehavior & D3DCREATE_HARDWARE_VERTEXPROCESSING &&
            pModeInfo->dwBehavior & D3DCREATE_PUREDEVICE )
        {
            if( pDeviceInfo->DeviceType == D3DDEVTYPE_HAL )
                lstrcat( m_strDeviceStats, TEXT(" (pure hw vp)") );
            else
                lstrcat( m_strDeviceStats, TEXT(" (simulated pure hw vp)") );
        }
        else if( pModeInfo->dwBehavior & D3DCREATE_HARDWARE_VERTEXPROCESSING )
        {
            if( pDeviceInfo->DeviceType == D3DDEVTYPE_HAL )
                lstrcat( m_strDeviceStats, TEXT(" (hw vp)") );
            else
                lstrcat( m_strDeviceStats, TEXT(" (simulated hw vp)") );
        }
        else if( pModeInfo->dwBehavior & D3DCREATE_MIXED_VERTEXPROCESSING )
        {
            if( pDeviceInfo->DeviceType == D3DDEVTYPE_HAL )
                lstrcat( m_strDeviceStats, TEXT(" (mixed vp)") );
            else
                lstrcat( m_strDeviceStats, TEXT(" (simulated mixed vp)") );
        }
        else if( pModeInfo->dwBehavior & D3DCREATE_SOFTWARE_VERTEXPROCESSING )
        {
            lstrcat( m_strDeviceStats, TEXT(" (sw vp)") );
        }

        if( pDeviceInfo->DeviceType == D3DDEVTYPE_HAL )
        {
            lstrcat( m_strDeviceStats, TEXT(": ") );
            lstrcat( m_strDeviceStats, pAdapterInfo->d3dAdapterIdentifier.Description );
        }

		if ( InterfaceGetOption( BACK_BUFFER ) == TRUE )
		{
	        // Store render target surface desc
		    IGRAPHICSSURFACE* pBackBuffer;
#ifdef TUD9
			m_pd3dDevice->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );
#else
			m_pd3dDevice->GetBackBuffer( 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );
#endif
	        pBackBuffer->GetDesc( &m_d3dsdBackBuffer );
		    pBackBuffer->Release();
		}

        // Set up the fullscreen cursor
        if( m_bShowCursorWhenFullscreen && !m_bWindowed )
        {
            HCURSOR hCursor;
#ifdef _WIN64
            hCursor = (HCURSOR)GetClassLongPtr( m_hWnd, GCLP_HCURSOR );
#else
            hCursor = (HCURSOR)GetClassLong( m_hWnd, GCL_HCURSOR );
#endif
//            D3DUtil_SetDeviceCursor( m_pd3dDevice, hCursor );
            m_pd3dDevice->ShowCursor( TRUE );
        }

		return( S_OK );
    }

    // If that failed, fall back to the reference rasterizer
    if( pDeviceInfo->DeviceType == D3DDEVTYPE_HAL )
    {
        // Let the user know we are switching from HAL to the reference rasterizer
        DisplayErrorMsg( hr, MSGWARN_SWITCHEDTOREF );

        // Select the default adapter
        m_dwAdapter = 0L;
        pAdapterInfo = &m_Adapters[m_dwAdapter];

        // Look for a software device
        for( UINT i=0L; i<pAdapterInfo->dwNumDevices; i++ )
        {
            if( pAdapterInfo->devices[i].DeviceType == D3DDEVTYPE_REF )
            {
                pAdapterInfo->dwCurrentDevice = i;
                pDeviceInfo = &pAdapterInfo->devices[i];
                m_bWindowed = pDeviceInfo->bWindowed;
                break;
            }
        }

        // Try again, this time with the reference rasterizer
        if( pAdapterInfo->devices[pAdapterInfo->dwCurrentDevice].DeviceType ==
            D3DDEVTYPE_REF )
        {
            hr = Initialize3DEnvironment();
        }
    }

    return hr;
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CD3DApplication::Resize3DEnvironment()
{
    HRESULT hr;

    // Reset the device
    if( FAILED( hr = m_pd3dDevice->Reset( &m_d3dpp ) ) )
        return hr;

    // Store render target surface desc
    IGRAPHICSSURFACE* pBackBuffer;
#ifdef TUD9
	m_pd3dDevice->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );
#else
    m_pd3dDevice->GetBackBuffer( 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );
#endif
    pBackBuffer->GetDesc( &m_d3dsdBackBuffer );
    pBackBuffer->Release();

    // Set up the fullscreen cursor
    if( m_bShowCursorWhenFullscreen && !m_bWindowed )
    {
        HCURSOR hCursor;
#ifdef _WIN64
        hCursor = (HCURSOR)GetClassLongPtr( m_hWnd, GCLP_HCURSOR );
#else
        hCursor = (HCURSOR)GetClassLong( m_hWnd, GCL_HCURSOR );
#endif
//        D3DUtil_SetDeviceCursor( m_pd3dDevice, hCursor );
        m_pd3dDevice->ShowCursor( TRUE );
    } 


    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: ToggleFullScreen()
// Desc: Called when user toggles between fullscreen mode and windowed mode
//-----------------------------------------------------------------------------
HRESULT CD3DApplication::ToggleFullscreen()
{
    // Get access to current adapter, device, and mode
    D3DAdapterInfo* pAdapterInfo = &m_Adapters[m_dwAdapter];
    D3DDeviceInfo*  pDeviceInfo  = &pAdapterInfo->devices[pAdapterInfo->dwCurrentDevice];
    D3DModeInfo*    pModeInfo    = &pDeviceInfo->modes[pDeviceInfo->dwCurrentMode];

    // Need device change if going windowed and the current device
    // can only be fullscreen
    if( !m_bWindowed && !pDeviceInfo->bCanDoWindowed )
        return ForceWindowed();

    m_bReady = FALSE;

    // Toggle the windowed state
    m_bWindowed = !m_bWindowed;
    pDeviceInfo->bWindowed = m_bWindowed;

    // Prepare window for windowed/fullscreen change
    AdjustWindowForChange();

    // Set up the presentation parameters
    m_d3dpp.Windowed               = pDeviceInfo->bWindowed;
    m_d3dpp.MultiSampleType        = pDeviceInfo->MultiSampleType;
    m_d3dpp.AutoDepthStencilFormat = pModeInfo->DepthStencilFormat;
    m_d3dpp.hDeviceWindow          = m_hWnd;
    if( m_bWindowed )
    {
        m_d3dpp.BackBufferWidth  = m_rcWindowClient.right - m_rcWindowClient.left;
	    m_d3dpp.BackBufferHeight = m_rcWindowClient.bottom - m_rcWindowClient.top;
		m_d3dpp.BackBufferFormat = pAdapterInfo->d3ddmDesktop.Format;
    }
    else
    {
        m_d3dpp.BackBufferWidth  = pModeInfo->Width;
        m_d3dpp.BackBufferHeight = pModeInfo->Height;
        m_d3dpp.BackBufferFormat = pModeInfo->Format;
    }

	if ( InterfaceGetOption( MINIMUM_SURFACE_RES ) == TRUE )
	{
		if ( m_d3dpp.BackBufferWidth < 900 )
		{
			m_d3dpp.BackBufferWidth = 900;
		}
		if (  m_d3dpp.BackBufferHeight < 700 )
		{
			 m_d3dpp.BackBufferHeight = 700;
		}
	}

    // Resize the 3D device
    if( FAILED( Resize3DEnvironment() ) )
    {
        if( m_bWindowed )
            return ForceWindowed();
        else
            return E_FAIL;
    }

    // When moving from fullscreen to windowed mode, it is important to
    // adjust the window size after resetting the device rather than
    // beforehand to ensure that you get the window size you want.  For
    // example, when switching from 640x480 fullscreen to windowed with
    // a 1000x600 window on a 1024x768 desktop, it is impossible to set
    // the window size to 1000x600 until after the display mode has
    // changed to 1024x768, because windows cannot be larger than the
    // desktop.
    if( m_bWindowed )
    {
        SetWindowPos( m_hWnd, HWND_NOTOPMOST,
                      m_rcWindowBounds.left, m_rcWindowBounds.top,
                      ( m_rcWindowBounds.right - m_rcWindowBounds.left ),
                      ( m_rcWindowBounds.bottom - m_rcWindowBounds.top ),
                      SWP_SHOWWINDOW );
    }

    m_bReady = TRUE;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: ForceWindowed()
// Desc: Switch to a windowed mode, even if that means picking a new device
//       and/or adapter
//-----------------------------------------------------------------------------
HRESULT CD3DApplication::ForceWindowed()
{
    HRESULT hr;
    D3DAdapterInfo* pAdapterInfoCur = &m_Adapters[m_dwAdapter];
    D3DDeviceInfo*  pDeviceInfoCur  = &pAdapterInfoCur->devices[pAdapterInfoCur->dwCurrentDevice];
    BOOL bFoundDevice = FALSE;

    if( pDeviceInfoCur->bCanDoWindowed )
    {
        bFoundDevice = TRUE;
    }
    else
    {
        // Look for a windowable device on any adapter
        D3DAdapterInfo* pAdapterInfo;
        DWORD dwAdapter;
        D3DDeviceInfo* pDeviceInfo;
        DWORD dwDevice;
        for( dwAdapter = 0; dwAdapter < m_dwNumAdapters; dwAdapter++ )
        {
            pAdapterInfo = &m_Adapters[dwAdapter];
            for( dwDevice = 0; dwDevice < pAdapterInfo->dwNumDevices; dwDevice++ )
            {
                pDeviceInfo = &pAdapterInfo->devices[dwDevice];
                if( pDeviceInfo->bCanDoWindowed )
                {
                    m_dwAdapter = dwAdapter;
                    pDeviceInfoCur = pDeviceInfo;
                    pAdapterInfo->dwCurrentDevice = dwDevice;
                    bFoundDevice = TRUE;
                    break;
                }
            }
            if( bFoundDevice )
                break;
        }
    }

    if( !bFoundDevice )
        return E_FAIL;

    pDeviceInfoCur->bWindowed = TRUE;
    m_bWindowed = TRUE;

    // Now destroy the current 3D device objects, then reinitialize

    m_bReady = FALSE;

    // Release all scene objects that will be re-created for the new device
    InvalidateDeviceObjects();
    DeleteDeviceObjects();

    // Release display objects, so a new device can be created
    if( m_pd3dDevice->Release() > 0L )
        return DisplayErrorMsg( D3DAPPERR_NONZEROREFCOUNT, MSGERR_APPMUSTEXIT );

    // Create the new device
    if( FAILED( hr = Initialize3DEnvironment() ) )
        return DisplayErrorMsg( hr, MSGERR_APPMUSTEXIT );
    m_bReady = TRUE;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: AdjustWindowForChange()
// Desc: Prepare the window for a possible change between windowed mode and
//       fullscreen mode.  This function is virtual and thus can be overridden
//       to provide different behavior, such as switching to an entirely
//       different window for fullscreen mode (as in the MFC sample apps).
//-----------------------------------------------------------------------------
HRESULT CD3DApplication::AdjustWindowForChange()
{
    if( m_bWindowed )
    {
        // Set windowed-mode style
        SetWindowLong( m_hWnd, GWL_STYLE, m_dwWindowStyle );
    }
    else
    {
        // Set fullscreen-mode style
        SetWindowLong( m_hWnd, GWL_STYLE, WS_POPUP|WS_SYSMENU|WS_VISIBLE );
    }
    return S_OK;
}



 
//-----------------------------------------------------------------------------
// Name: UserSelectNewDevice()
// Desc: Displays a dialog so the user can select a new adapter, device, or
//       display mode, and then recreates the 3D environment if needed
//-----------------------------------------------------------------------------
HRESULT CD3DApplication::UserSelectNewDevice()
{
//HRESULT hr;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: SelectDeviceProc()
// Desc: Windows message handling function for the device select dialog
//-----------------------------------------------------------------------------
INT_PTR CALLBACK CD3DApplication::SelectDeviceProc( HWND hDlg, UINT msg,
                                                    WPARAM wParam, LPARAM lParam )
{
    // Get access to the UI controls
    HWND hwndAdapterList        = GetDlgItem( hDlg, IDC_ADAPTER_COMBO );
    HWND hwndDeviceList         = GetDlgItem( hDlg, IDC_DEVICE_COMBO );
    HWND hwndFullscreenModeList = GetDlgItem( hDlg, IDC_FULLSCREENMODES_COMBO );
    HWND hwndWindowedRadio      = GetDlgItem( hDlg, IDC_WINDOW );
    HWND hwndFullscreenRadio    = GetDlgItem( hDlg, IDC_FULLSCREEN );
    HWND hwndDoubleBuffer		= GetDlgItem( hDlg, IDC_FRAME_BUFFER );
    HWND hwndVsync				= GetDlgItem( hDlg, IDC_VSYNC );
    HWND hwndOldStartup			= GetDlgItem( hDlg, IDC_STARTUP_OLD );
    HWND hwndMinimumSurfaceRes	= GetDlgItem( hDlg, IDC_MINIMUM_SURFACE_RES );
    HWND hwndAdvancedRadio		= GetDlgItem( hDlg, IDC_ADVANCED );
    BOOL bUpdateDlgControls     = FALSE;

    // Static state for adapter/device/mode selection
    static CD3DApplication* pd3dApp;
    static DWORD  dwOldAdapter, dwNewAdapter;
    static DWORD  dwOldDevice,  dwNewDevice;
    static DWORD  dwOldMode,    dwNewMode;
    static BOOL   bOldWindowed, bNewWindowed;
    static D3DMULTISAMPLE_TYPE OldMultiSampleType, NewMultiSampleType;

    // Working variables
    D3DAdapterInfo* pAdapter;
    D3DDeviceInfo*  pDevice;

    // Handle the initialization message
    if( WM_INITDIALOG == msg )
    {
        // Old state
         pd3dApp      = (CD3DApplication*)lParam;
        dwOldAdapter = pd3dApp->m_dwAdapter;
        pAdapter     = &pd3dApp->m_Adapters[dwOldAdapter];

        dwOldDevice  = pAdapter->dwCurrentDevice;
        pDevice      = &pAdapter->devices[dwOldDevice];

        dwOldMode    = pDevice->dwCurrentMode;
        bOldWindowed = pDevice->bWindowed;
        OldMultiSampleType = pDevice->MultiSampleType;

		mnTextureFilteringFlag = InterfaceGetOption( TEXTURE_FILTERING );
		mnBackBufferFlag = InterfaceGetOption( BACK_BUFFER );
		mnVsync = InterfaceGetOption( VSYNC );
		mnOldStartup = InterfaceGetOption( OLD_STARTUP );
		mnMinimumSurfaceRes = InterfaceGetOption( MINIMUM_SURFACE_RES );
		mnFogMode = InterfaceGetOption( FOG_MODE );

        // New state is initially the same as the old state
        dwNewAdapter = dwOldAdapter;
        dwNewDevice  = dwOldDevice;
        dwNewMode    = dwOldMode;
        bNewWindowed = bOldWindowed;
        NewMultiSampleType = OldMultiSampleType;

        // Set flag to update dialog controls below
        bUpdateDlgControls = TRUE;
    }

    if( WM_COMMAND == msg )
    {
        // Get current UI state
        bNewWindowed  = Button_GetCheck( hwndWindowedRadio );
		mnBackBufferFlag = Button_GetCheck(hwndDoubleBuffer );
		mnVsync = Button_GetCheck(hwndVsync );
		mnOldStartup = Button_GetCheck( hwndOldStartup );
		mnMinimumSurfaceRes = Button_GetCheck( hwndMinimumSurfaceRes );
        if( IDOK == LOWORD(wParam) )
        {
			InterfaceSetOption( BACK_BUFFER, Button_GetCheck( hwndDoubleBuffer ) );
			InterfaceSetOption( VSYNC, Button_GetCheck( hwndVsync ) );
			InterfaceSetOption( OLD_STARTUP, Button_GetCheck( hwndOldStartup ) );
			InterfaceSetOption( MINIMUM_SURFACE_RES, Button_GetCheck( hwndMinimumSurfaceRes ) );

            // Handle the case when the user hits the OK button. Check if any
            // of the options were changed
            if( dwNewAdapter != dwOldAdapter || dwNewDevice  != dwOldDevice  ||
                dwNewMode    != dwOldMode    || bNewWindowed != bOldWindowed ||
                NewMultiSampleType != OldMultiSampleType )
            {
                pd3dApp->m_dwAdapter = dwNewAdapter;

                pAdapter = &pd3dApp->m_Adapters[dwNewAdapter];
                pAdapter->dwCurrentDevice = dwNewDevice;

                pAdapter->devices[dwNewDevice].dwCurrentMode = dwNewMode;
                pAdapter->devices[dwNewDevice].bWindowed     = bNewWindowed;

				if ( bNewWindowed == FALSE )
				{
				int		nScreenWidth = pAdapter->devices[dwNewDevice].modes[dwNewMode].Width;
				int		nScreenHeight = pAdapter->devices[dwNewDevice].modes[dwNewMode].Height;

					InterfaceSetInitialSize( TRUE, nScreenWidth, nScreenHeight, FALSE );
				}
			
                pAdapter->devices[dwNewDevice].MultiSampleType = NewMultiSampleType;

                EndDialog( hDlg, IDOK );
				mboFullScreen = !bNewWindowed;
	        }
            else
                EndDialog( hDlg, IDCANCEL );
    
            return TRUE;
        }
        else if( IDCANCEL == LOWORD(wParam) )
        {
            // Handle the case when the user hits the Cancel button
            EndDialog( hDlg, IDCANCEL );
            return TRUE;
        }
        else if( CBN_SELENDOK == HIWORD(wParam) )
        {
            if( LOWORD(wParam) == IDC_ADAPTER_COMBO )
            {
                dwNewAdapter = ComboBox_GetCurSel( hwndAdapterList );
                pAdapter     = &pd3dApp->m_Adapters[dwNewAdapter];

                dwNewDevice  = pAdapter->dwCurrentDevice;
                dwNewMode    = pAdapter->devices[dwNewDevice].dwCurrentMode;
                bNewWindowed = pAdapter->devices[dwNewDevice].bWindowed;
            }
            else if( LOWORD(wParam) == IDC_DEVICE_COMBO )
            {
                pAdapter     = &pd3dApp->m_Adapters[dwNewAdapter];

                dwNewDevice  = ComboBox_GetCurSel( hwndDeviceList );
                dwNewMode    = pAdapter->devices[dwNewDevice].dwCurrentMode;
                bNewWindowed = pAdapter->devices[dwNewDevice].bWindowed;
            }
            else if( LOWORD(wParam) == IDC_FULLSCREENMODES_COMBO )
            {
                dwNewMode = ComboBox_GetCurSel( hwndFullscreenModeList );
            }
        }
        // Keep the UI current
        bUpdateDlgControls = TRUE;
    }

    // Update the dialog controls
    if( bUpdateDlgControls )
    {

        // Reset the content in each of the combo boxes
        ComboBox_ResetContent( hwndAdapterList );
        ComboBox_ResetContent( hwndDeviceList );
        ComboBox_ResetContent( hwndFullscreenModeList );

        pAdapter = &pd3dApp->m_Adapters[dwNewAdapter];
        pDevice  = &pAdapter->devices[dwNewDevice];

        // Add a list of adapters to the adapter combo box
        for( DWORD a=0; a < pd3dApp->m_dwNumAdapters; a++ )
        {
            // Add device name to the combo box
            DWORD dwItem = ComboBox_AddString( hwndAdapterList,
                             pd3dApp->m_Adapters[a].d3dAdapterIdentifier.Description );

            // Set the item data to identify this adapter
            ComboBox_SetItemData( hwndAdapterList, dwItem, a );

            // Set the combobox selection on the current adapater
            if( a == dwNewAdapter )
                ComboBox_SetCurSel( hwndAdapterList, dwItem );
        }

        // Add a list of devices to the device combo box
        for( DWORD d=0; d < pAdapter->dwNumDevices; d++ )
        {
            // Add device name to the combo box
            DWORD dwItem = ComboBox_AddString( hwndDeviceList,
                                               pAdapter->devices[d].strDesc );

            // Set the item data to identify this device
            ComboBox_SetItemData( hwndDeviceList, dwItem, d );

            // Set the combobox selection on the current device
            if( d == dwNewDevice )
                ComboBox_SetCurSel( hwndDeviceList, dwItem );
        }

        // Add a list of modes to the mode combo box
        for( DWORD m=0; m < pDevice->dwNumModes; m++ )
        {
            DWORD BitDepth = 16;
            if( pDevice->modes[m].Format == D3DFMT_X8R8G8B8 ||
                pDevice->modes[m].Format == D3DFMT_A8R8G8B8 ||
                pDevice->modes[m].Format == D3DFMT_R8G8B8 )
            {
                BitDepth = 32;
            }

            // Add mode desc to the combo box
            TCHAR strMode[80];
            _stprintf( strMode, _T("%ld x %ld x %ld"), pDevice->modes[m].Width,
                                                       pDevice->modes[m].Height,
                                                       BitDepth );
            DWORD dwItem = ComboBox_AddString( hwndFullscreenModeList, strMode );

            // Set the item data to identify this mode
            ComboBox_SetItemData( hwndFullscreenModeList, dwItem, m );

            // Set the combobox selection on the current mode
             if( m == dwNewMode )
                ComboBox_SetCurSel( hwndFullscreenModeList, dwItem );
        }
        EnableWindow( hwndWindowedRadio, pDevice->bCanDoWindowed );

		Button_SetCheck( hwndDoubleBuffer, mnBackBufferFlag );
		Button_SetCheck( hwndVsync, mnVsync );
		Button_SetCheck( hwndOldStartup, mnOldStartup );
		Button_SetCheck( hwndMinimumSurfaceRes, mnMinimumSurfaceRes );

	   if( bNewWindowed )
        {
            Button_SetCheck( hwndWindowedRadio,   TRUE );
            Button_SetCheck( hwndFullscreenRadio, FALSE );
            EnableWindow( hwndFullscreenModeList, FALSE );
        }
        else
        {
            Button_SetCheck( hwndWindowedRadio,   FALSE );
            Button_SetCheck( hwndFullscreenRadio, TRUE );
            EnableWindow( hwndFullscreenModeList, TRUE );
        }

		if ( mnOldStartup == FALSE )
		{
            EnableWindow( hwndWindowedRadio, TRUE );
            EnableWindow( hwndVsync, TRUE );
            EnableWindow( hwndMinimumSurfaceRes, TRUE );
            EnableWindow( hwndDoubleBuffer, TRUE );
            EnableWindow( hwndFullscreenRadio, TRUE );
            EnableWindow( hwndAdapterList, TRUE );
            EnableWindow( hwndDeviceList, TRUE );
            Button_SetCheck( hwndAdvancedRadio, TRUE );
		}
		else
		{
            EnableWindow( hwndFullscreenModeList, FALSE );
            EnableWindow( hwndWindowedRadio, FALSE );
            EnableWindow( hwndVsync, FALSE );
            EnableWindow( hwndMinimumSurfaceRes, FALSE );
            EnableWindow( hwndDoubleBuffer, FALSE );
            EnableWindow( hwndFullscreenRadio, FALSE );
            EnableWindow( hwndAdapterList, FALSE );
            EnableWindow( hwndDeviceList, FALSE );
            Button_SetCheck( hwndAdvancedRadio, FALSE );
		}

        
		return TRUE;
    }

    return FALSE;
}




//-----------------------------------------------------------------------------
// Name: Run()
// Desc:
//-----------------------------------------------------------------------------
INT CD3DApplication::Run()
{
	return( 0 );
}






//-----------------------------------------------------------------------------
// Name: Pause()
// Desc: Called in to toggle the pause state of the app.
//-----------------------------------------------------------------------------
VOID CD3DApplication::Pause( BOOL bPause )
{
}



//-----------------------------------------------------------------------------
// Name: Cleanup3DEnvironment()
// Desc: Cleanup scene objects
//-----------------------------------------------------------------------------
VOID CD3DApplication::Cleanup3DEnvironment()
{
    m_bActive = FALSE;
    m_bReady  = FALSE;

    if( m_pd3dDevice )
    {
        InvalidateDeviceObjects();
        DeleteDeviceObjects();

        m_pd3dDevice->Release();
        m_pD3D->Release();

        m_pd3dDevice = NULL;
        m_pD3D       = NULL;
    }

    FinalCleanup();
}




//-----------------------------------------------------------------------------
// Name: DisplayErrorMsg()
// Desc: Displays error messages in a message box
//-----------------------------------------------------------------------------
HRESULT CD3DApplication::DisplayErrorMsg( HRESULT hr, DWORD dwType )
{
    TCHAR strMsg[512];

    switch( hr )
    {
        case D3DAPPERR_NODIRECT3D:
            _tcscpy( strMsg, _T("Could not initialize Direct3D. You may\n")
                             _T("want to check that the latest version of\n")
                             _T("DirectX is correctly installed on your\n")
                             _T("system. ") );
            break;

        case D3DAPPERR_NOCOMPATIBLEDEVICES:
            _tcscpy( strMsg, _T("Could not find any compatible Direct3D\n")
                             _T("devices.") );
            break;

        case D3DAPPERR_NOWINDOWABLEDEVICES:
            _tcscpy( strMsg, _T("A Tractor cannot run in a desktop\n")
                             _T("window with the current display settings.\n")
                             _T("Please change your desktop settings to a\n")
                             _T("16- or 32-bit display mode and re-run the game") );
            break;

        case D3DAPPERR_NOHARDWAREDEVICE:
            _tcscpy( strMsg, _T("No hardware-accelerated Direct3D devices\n")
                             _T("were found.") );
            break;

        case D3DAPPERR_HALNOTCOMPATIBLE:
            _tcscpy( strMsg, _T("A Tractor requires functionality that is\n")
                             _T("not available on your Direct3D hardware\n")
                             _T("accelerator.") );
            break;

        case D3DAPPERR_NOWINDOWEDHAL:
            _tcscpy( strMsg, _T("Your Direct3D hardware accelerator cannot\n")
                             _T("render into a window.\n") );
            break;

        case D3DAPPERR_NODESKTOPHAL:
            _tcscpy( strMsg, _T("Your Direct3D hardware accelerator cannot\n")
                             _T("render into a window with the current\n")
                             _T("desktop display settings.\n") );
            break;

        case D3DAPPERR_NOHALTHISMODE:
            _tcscpy( strMsg, _T("A Tractir requires functionality that is\n")
                             _T("not available on your Direct3D hardware\n")
                             _T("accelerator with the current desktop display\n")
                             _T("settings.\n") );
            break;

        case D3DAPPERR_MEDIANOTFOUND:
            _tcscpy( strMsg, _T("Could not load required media." ) );
            break;

        case D3DAPPERR_RESIZEFAILED:
            _tcscpy( strMsg, _T("Could not reset the Direct3D device." ) );
            break;

        case D3DAPPERR_NONZEROREFCOUNT:
            _tcscpy( strMsg, _T("A D3D object has a non-zero reference\n")
                             _T("count (meaning things were not properly\n")
                             _T("cleaned up).") );
            break;

        case E_OUTOFMEMORY:
            _tcscpy( strMsg, _T("Not enough memory.") );
            break;

        case D3DERR_OUTOFVIDEOMEMORY:
            _tcscpy( strMsg, _T("Not enough video memory.") );
            break;

        default:
            _tcscpy( strMsg, _T("A Tractor DirectX Error. Your Video\n")
                             _T("Options may need adjusting.") );
    }

    if( MSGERR_APPMUSTEXIT == dwType )
    {
        _tcscat( strMsg, _T("\n\nTractor is about to die.") );
        MessageBox( NULL, strMsg, m_strWindowTitle, MB_ICONERROR|MB_OK );

        // Close the window, which shuts down the app
        if( m_hWnd )
            SendMessage( m_hWnd, WM_CLOSE, 0, 0 );
    }
    else
    {
        if( MSGWARN_SWITCHEDTOREF == dwType )
            _tcscat( strMsg, _T("\n\nSwitching to the reference rasterizer,\n")
                             _T("a software device that implements the entire\n")
                             _T("Direct3D feature set, but runs very slowly.") );
        MessageBox( NULL, strMsg, m_strWindowTitle, MB_ICONWARNING|MB_OK );
    }

    return hr;
}



//#endif	// ndef STANDALONE

