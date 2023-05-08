
#include <windows.h>
#include <StandardDef.h>
#include <Interface.h>

#ifndef TOOL
//#define INCLUDE_OCULUS_SUPPORT
#endif

#ifdef INCLUDE_OCULUS_SUPPORT

#define	OVR_D3D_VERSION		9

#include "OVR.h"
#include "EngineDX.h"

#include "Engine.h"
#include "EngineCameraDX.h"
#include "OculusDX.h"


#include "../../Interface/DirectX/InterfaceInternalsDX.h"

#include "../Src/OVR_CAPI_D3D.h"


typedef struct
{
	int		x;
	int		y;
	int		w;
	int		h;
} Recti;

typedef struct
{
	int		w;
	int		h;
}	 Sizei;

// Rendering parameters used by RenderDevice::CreateDevice.
struct RendererParams
{
    int     Multisample;
    int     Fullscreen;
    // Resolution of the rendering buffer used during creation.
    // Allows buffer of different size then the widow if not zero.
    Recti   Resolution;

    // Windows - Monitor name for fullscreen mode.
    char  MonitorName[256];

    RendererParams(int ms = 1) : Multisample(ms), Fullscreen(0) {}

    bool IsDisplaySet() const
    {
        if ( MonitorName[0] != 0 )
		{
			return( true );
		}
		return( false );
    }
};

RendererParams		m_RendererParams;
ovrHmd		hmd = NULL;
HWND		mOculusHWND;
int		mWindowWidth;
int		mWindowHeight;

bool	m_deviceNameFound = false;
BOOL	mbDirectToRift = FALSE;


void ShutdownOculusVR( void )
{
	if ( hmd )
	{
		ovrHmd_Destroy(hmd);
		hmd = NULL;
		ovr_Shutdown();
	}
}

BOOL	EngineHasOculus( void )
{
	if ( hmd )
	{
		return( TRUE );
	}
	return( FALSE );
}


void InitializeOculusVR( void )
{
	ovr_Initialize();
	hmd = ovrHmd_Create(0);
	if (hmd)
	{
		// Get more details about the HMD.
		ovrSizei resolution = hmd->Resolution;
	}


	// Start the sensor which provides the Rift’s pose and motion.
	ovrHmd_ConfigureTracking(hmd, ovrTrackingCap_Orientation |
									ovrTrackingCap_MagYawCorrection |
									ovrTrackingCap_Position, 0);
	// Query the HMD for the current tracking state.
	ovrTrackingState ts = ovrHmd_GetTrackingState(hmd, ovr_GetTimeInSeconds());
	if (ts.StatusFlags & (ovrStatus_OrientationTracked | ovrStatus_PositionTracked))
	{
//	Posef pose = ts.HeadPose;
	
	
	}

}

BOOL		OculusGetEulerAngles( float* pfYaw, float* pfPitch, float* pfRoll )
{
	ovrTrackingState ts = ovrHmd_GetTrackingState(hmd, ovr_GetTimeInSeconds());
	if (ts.StatusFlags & (ovrStatus_OrientationTracked | ovrStatus_PositionTracked))
	{
	float		yaw, eyePitch, eyeRoll;

		OVR::Posef pose = ts.HeadPose.ThePose;
		pose.Rotation.GetEulerAngles<OVR::Axis_Y, OVR::Axis_X, OVR::Axis_Z, OVR::Rotate_CCW, OVR::Handed_L>(&yaw, &eyePitch, &eyeRoll);
		eyeRoll *= -1.0f;
//		GetEulerAngles<OVR::Axis_Y, OVR::Axis_X, OVR::Axis_Z, OVR::Rotate_CCW, OVR::Handed_L>(&yaw, &, &eyeRoll);

		*pfYaw = yaw;
		*pfPitch = eyePitch;
		*pfRoll = eyeRoll;
		return( TRUE );
	}
	return( FALSE );

}

BOOL		OculusGetQuaternion( float* pfQuaternion )
{
	ovrTrackingState ts = ovrHmd_GetTrackingState(hmd, ovr_GetTimeInSeconds());
	if (ts.StatusFlags & (ovrStatus_OrientationTracked | ovrStatus_PositionTracked))
	{
		OVR::Posef pose = ts.HeadPose.ThePose;

		pfQuaternion[0] = pose.Rotation.x;
		pfQuaternion[1] = pose.Rotation.y;
		pfQuaternion[2] = pose.Rotation.z;
		pfQuaternion[3] = pose.Rotation.w;
		return( TRUE );
	}
	return( FALSE );

}

BOOL		OculusGetHeadOrientation( VECT* pxDir, VECT* pxUp )
{
	ovrTrackingState ts = ovrHmd_GetTrackingState(hmd, ovr_GetTimeInSeconds());
	if (ts.StatusFlags & (ovrStatus_OrientationTracked | ovrStatus_PositionTracked))
	{
	OVR::Vector3f	xDir( 0.0f, 0.0f, -1.0f );
	OVR::Vector3f	xUp( 0.0f, 1.0f, 0.0f );

		OVR::Posef pose = ts.HeadPose.ThePose;
	
		xDir = pose.Rotation.Rotate( xDir );
		pxDir->x = xDir.x;
		pxDir->y = xDir.z;
		pxDir->z = xDir.y;
		xUp = pose.Rotation.Rotate( xUp );
		pxUp->x = xUp.x;
		pxUp->y = xUp.z;
		pxUp->z = xUp.y;
		return( TRUE );
	}
	return( FALSE );
}

BOOL		OculusApplyHeadOrientation( VECT* pxDir, VECT* pxUp )
{
	ovrTrackingState ts = ovrHmd_GetTrackingState(hmd, ovr_GetTimeInSeconds());
	if (ts.StatusFlags & (ovrStatus_OrientationTracked | ovrStatus_PositionTracked))
	{
	OVR::Vector3f	xDir( pxDir->x, pxDir->y, pxDir->z );
	OVR::Vector3f	xUp( pxUp->x, pxUp->y, pxUp->z );

		OVR::Posef pose = ts.HeadPose.ThePose;
	
		xDir = pose.Rotation.Rotate( xDir );
		pxDir->x = xDir.x;
		pxDir->y = xDir.y;
		pxDir->z = xDir.z;
		xUp = pose.Rotation.Rotate( xUp );
		pxUp->x = xUp.x;
		pxUp->y = xUp.y;
		pxUp->z = xUp.z;
		return( TRUE );
	}
	return( FALSE );
}


void*	OculusInitD3DDevice( void* pD3DVoid )
{
LPDIRECT3D9		pD3D = (LPDIRECT3D9)pD3DVoid;
BOOL	FullscreenOutput = FALSE;
IDirect3DDevice9*	pD3DDevice = NULL;
	HRESULT		hr;

    mWindowWidth = m_RendererParams.Resolution.w;
    mWindowHeight= m_RendererParams.Resolution.h;

    int flags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory( &d3dpp, sizeof(d3dpp) );
	
	// Set fullscreen-mode style
/*
	InterfaceSetWindowStyle( true );
	if ( InterfaceGetOption( FSAA ) == 1 )
	{
		if( SUCCEEDED(mpD3D->CheckDeviceMultiSampleType( pAdapter, D3DDEVTYPE_HAL, d3ddm.Format, FALSE, D3DMULTISAMPLE_4_SAMPLES, NULL ) ) )
		{
			d3dpp.MultiSampleType = D3DMULTISAMPLE_4_SAMPLES;
		}
		else if ( SUCCEEDED(mpD3D->CheckDeviceMultiSampleType( pAdapter, D3DDEVTYPE_HAL, d3ddm.Format, FALSE, D3DMULTISAMPLE_2_SAMPLES, NULL ) ) )
		{
			d3dpp.MultiSampleType = D3DMULTISAMPLE_2_SAMPLES;
		}
	}
	*/
	int		nNumAdapters = pD3D->GetAdapterCount();
	D3DDISPLAYMODE		d3ddm;
	pD3D->GetAdapterDisplayMode( nNumAdapters - 1, &d3ddm);

	// TODO - Should be 'did we find an adapter that matched the oculus signature'
	if ( nNumAdapters > 1 )
	{
		d3dpp.Windowed = FALSE;
		d3dpp.BackBufferWidth  = 1920;
		d3dpp.BackBufferHeight = 1080;

	//	d3dpp.SwapEffect = D3DSWAPEFFECT_COPY_VSYNC;
		d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;
		d3dpp.BackBufferFormat = d3ddm.Format;
		d3dpp.EnableAutoDepthStencil = FALSE;
		d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
//	d3dpp.AutoDepthStencilFormat = 0;
		d3dpp.BackBufferCount = 0;
		d3dpp.FullScreen_RefreshRateInHz = d3ddm.RefreshRate;
//	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
		d3dpp.hDeviceWindow          = mOculusHWND;
		d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;//D3DPRESENT_INTERVAL_IMMEDIATE;
//	d3dpp.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
	}
	else
	{
		InterfaceGetDXDeviceCreateParams( FALSE, &d3dpp );
	}
	hr = pD3D->CreateDevice( nNumAdapters-1, D3DDEVTYPE_HAL, mOculusHWND, flags, &d3dpp, &pD3DDevice );
	int i = 0;

	switch ( hr )
	{
	case D3DERR_OUTOFVIDEOMEMORY:
//		PANIC_IF(TRUE,"Not enough video memory to create D3D Device.\n You might be able to fix this by lowering your screen resolution in your desktop display properties." );
		i++;
		break;
	case D3DERR_INVALIDCALL:
//		PANIC_IF(TRUE,"Invalid call for direct3d create device" );
		i++;
		break;
	default:
//		PANIC_IF(TRUE,"Direct3D Create Device failed. There may be problems with your video card drivers or DirectX install.\n Adjusting the DirectX setup options on the Video Options menu may help." );
		i++;
		break;
	}

/*
    hr = D3D11CreateDevice(pAdapter, pAdapter ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE,
                            NULL, flags, NULL, 0, D3D11_SDK_VERSION,
                            &Device.GetRawRef(), NULL, &Context.GetRawRef());
*/

    if (FAILED(hr))
        return( NULL );
#if 0
    if (!RecreateSwapChain())
        return;

    if (Params.Fullscreen)
        SwapChain->SetFullscreenState(1, FullscreenOutput);
#endif 

//	ovrHmd_AttachToWindow(hmd, mOculusHWND, NULL, NULL);

	mpEngineDevice = pD3DDevice;
	
	if ( mbDirectToRift )
	{
		ovrHmd_AttachToWindow(hmd, mOculusHWND, NULL, NULL);
	}

	return( pD3DDevice );
}



void*	OculusInitWindow( void* pGamesWCVoid )
{
HWND	hWnd;
WNDCLASSEX*	pGamesWC = (WNDCLASSEX*)( pGamesWCVoid );

	if ( hmd == NULL )
	{
		return( NULL );
	}
    // Window
   WNDCLASS wc;
    memset(&wc, 0, sizeof(wc));
    wc.lpszClassName = "OVRAppWindow";
    wc.style         = CS_OWNDC;
	wc.lpfnWndProc   = pGamesWC->lpfnWndProc;
    wc.cbWndExtra    = NULL;
    RegisterClass(&wc);

    DWORD wsStyle = WS_POPUP;
    DWORD sizeDivisor = 1;
   
	BOOL	UseAppWindowFrame = FALSE;

	Recti	vp = { hmd->WindowsPos.x,hmd->WindowsPos.y, hmd->Resolution.w, hmd->Resolution.h };

	if ( mbDirectToRift )
	{
//		vp.x = 100;
//		vp.y = 0;
	}

    if (UseAppWindowFrame)
    {
        // If using our driver, displaya window frame with a smaller window.
        // Original HMD resolution is still passed into the renderer for proper swap chain.
        wsStyle |= WS_OVERLAPPEDWINDOW;
        m_RendererParams.Resolution.w = vp.w;
        m_RendererParams.Resolution.h = vp.h;
        sizeDivisor = 1;
    }



    RECT winSize = { 0, 0, vp.w / sizeDivisor, vp.h / sizeDivisor};
    AdjustWindowRect(&winSize, wsStyle, false);

    hWnd = CreateWindow( "OVRAppWindow", "UniversalOculus",
                         wsStyle | WS_VISIBLE,
                         vp.x, vp.y,
                         winSize.right-winSize.left, winSize.bottom-winSize.top,
                         NULL, NULL, pGamesWC->hInstance, NULL);

	mOculusHWND = hWnd;

	ShowWindow( hWnd, SW_SHOW );
	UpdateWindow( hWnd );
	return( (void*)hWnd );

}

TEXTURE_HANDLE		mhOculusLeftViewRenderTarget = NOTFOUND;
TEXTURE_HANDLE		mhOculusRightViewRenderTarget = NOTFOUND;

void		OculusStartSceneLeftEye( uint32 ulClearCol )
{
	if ( mhOculusLeftViewRenderTarget == NOTFOUND )
	{
//		mhLeftViewRenderTarget = EngineCreateRenderTargetTexture( InterfaceGetWidth()/2, InterfaceGetHeight() );	
//		mhRightViewRenderTarget = EngineCreateRenderTargetTexture( InterfaceGetWidth()/2, InterfaceGetHeight() );	
		mhOculusLeftViewRenderTarget = EngineCreateRenderTargetTexture( 960, 1080, 0, "VR Left Eye" );	
		mhOculusRightViewRenderTarget = EngineCreateRenderTargetTexture( 960, 1080, 0, "VR Right Eye" );	
	}
	
	EngineSetRenderTargetTexture( mhOculusLeftViewRenderTarget, ulClearCol, TRUE );
	EngineCameraSetEyeOffset( FALSE );

	InterfaceSetDrawRegion( 0, 0, EngineTextureGetWidth(mhOculusLeftViewRenderTarget), EngineTextureGetHeight(mhOculusLeftViewRenderTarget) );
}

void		OculusStartSceneRightEye(  uint32 ulClearCol )
{
	EngineRestoreRenderTarget();

	EngineSetRenderTargetTexture( mhOculusRightViewRenderTarget, ulClearCol, TRUE );
	EngineCameraSetEyeOffset( TRUE );
//	InterfaceSetDrawRegion( 0, 0, 960, 1080 );

}


void		OculusStartEndScene( void )
{
int		nLeftViewOverlay = NOTFOUND;
int		nRightViewOverlay = NOTFOUND;

	EngineRestoreRenderTarget();
	InterfaceSetDrawRegion( 0, 0, 0, 0 );

	nLeftViewOverlay = EngineTextureCreateInterfaceOverlay( 0, mhOculusLeftViewRenderTarget );
	nRightViewOverlay = EngineTextureCreateInterfaceOverlay( 0, mhOculusRightViewRenderTarget );

	InterfaceTexturedRect( nLeftViewOverlay, 0, 0, InterfaceGetWidth()/2,InterfaceGetHeight()-1,0xFFFFFFFF, 0.0f, 0.0f, 1.0f, 1.0f );
	InterfaceTexturedRect( nRightViewOverlay, InterfaceGetWidth()/2, 0, InterfaceGetWidth()/2,InterfaceGetHeight()-1,0xFFFFFFFF, 0.0f, 0.0f, 1.0f, 1.0f );
	InterfaceDraw();

} 

void		OculusReleaseGraphics( void )
{
	if ( mhOculusLeftViewRenderTarget != NOTFOUND )
	{
		EngineReleaseTexture( &mhOculusLeftViewRenderTarget );
	}
	if ( mhOculusRightViewRenderTarget != NOTFOUND )
	{
		EngineReleaseTexture( &mhOculusRightViewRenderTarget );
	}

}




void	OculusInitialiseRendering( IDirect3DDevice9* pD3DDevice, IDirect3DSwapChain9* pD3DSwapChain, int backBufferWidth, int backBufferHeight, float eyeFov )
{
#if 0

const bool       FullScreen = true; //Should be true for correct timing.  Use false for debug only.
ovrD3D9Config d3d9cfg;
ovrHmd		HMD = hmd;
//Setup Window and Graphics - use window frame if relying on Oculus driver
const int backBufferMultisample = 1;
bool UseAppWindowFrame = (HMD->HmdCaps & ovrHmdCap_ExtendDesktop) ? false : true;

	HWND window = Util_InitWindowAndGraphics(Recti(HMD->WindowsPos, HMD->Resolution),
                                         FullScreen, backBufferMultisample, UseAppWindowFrame,&pRender);
	if (!window) return 1;

	ovrHmd_AttachToWindow(HMD, window, NULL, NULL);

    //Configure Stereo settings.
    Sizei recommenedTex0Size = ovrHmd_GetFovTextureSize(HMD, ovrEye_Left,  HMD->DefaultEyeFov[0], 1.0f);
    Sizei recommenedTex1Size = ovrHmd_GetFovTextureSize(HMD, ovrEye_Right, HMD->DefaultEyeFov[1], 1.0f);
	Sizei RenderTargetSize;
    RenderTargetSize.w = recommenedTex0Size.w + recommenedTex1Size.w;
    RenderTargetSize.h = max ( recommenedTex0Size.h, recommenedTex1Size.h );

    const int eyeRenderMultisample = 1;
    pRendertargetTexture = EngineCreate pRender->CreateTexture(Texture_RGBA | Texture_RenderTarget |
                                                  eyeRenderMultisample,
                                                  RenderTargetSize.w, RenderTargetSize.h, NULL);
    // The actual RT size may be different due to HW limits.
    RenderTargetSize.w = pRendertargetTexture->GetWidth();
    RenderTargetSize.h = pRendertargetTexture->GetHeight();

    // Initialize eye rendering information.
    // The viewport sizes are re-computed in case RenderTargetSize changed due to HW limitations.
    ovrFovPort eyeFov[2] = { HMD->DefaultEyeFov[0], HMD->DefaultEyeFov[1] } ;

    EyeRenderViewport[0].Pos  = Vector2i(0,0);
    EyeRenderViewport[0].Size = Sizei(RenderTargetSize.w / 2, RenderTargetSize.h);
    EyeRenderViewport[1].Pos  = Vector2i((RenderTargetSize.w + 1) / 2, 0);
    EyeRenderViewport[1].Size = EyeRenderViewport[0].Size;

	// Query D3D texture data.
    EyeTexture[0].D3D9.Header.API            = ovrRenderAPI_D3D9;
    EyeTexture[0].D3D9.Header.TextureSize    = RenderTargetSize;
    EyeTexture[0].D3D9.Header.RenderViewport = EyeRenderViewport[0];
    EyeTexture[0].D3D9.pTexture              = pRendertargetTexture->Tex.GetPtr();
    EyeTexture[0].D3D9.pSRView               = pRendertargetTexture->TexSv.GetPtr();

    // Right eye uses the same texture, but different rendering viewport.
    EyeTexture[1] = EyeTexture[0];
    EyeTexture[1].D3D11.Header.RenderViewport = EyeRenderViewport[1];

	d3d9cfg.D3D9.Header.API = ovrRenderAPI_D3D9;
	d3d9cfg.D3D9.Header.RTSize.w = backBufferWidth;
	d3d9cfg.D3D9.Header.RTSize.h = backBufferHeight;
	d3d9cfg.D3D9.Header.Multisample = backBufferMultisample;
	d3d9cfg.D3D9.pDevice = pD3DDevice;
	d3d9cfg.D3D9.pSwapChain = pD3DSwapChain;
	
	if (!ovrHmd_ConfigureRendering(hmd, &d3d9cfg.Config, ovrDistortionCap_Chromatic | ovrDistortionCap_TimeWarp | ovrDistortionCap_Overdrive, eyeFov, EyeRenderDesc) )
	{

	}

#endif // if 0
}

/*
// Configure Stereo settings.
Sizei recommenedTex0Size = ovrHmd_GetFovTextureSize(hmd, ovrEye_Left,
hmd->DefaultEyeFov[0], 1.0f);
Sizei recommenedTex1Size = ovrHmd_GetFovTextureSize(hmd, ovrEye_Right,
hmd->DefaultEyeFov[1], 1.0f);
27Sizei renderTargetSize;
renderTargetSize.w = recommenedTex0Size.w + recommenedTex1Size.w;
renderTargetSize.h = max ( recommenedTex0Size.h, recommenedTex1Size.h );
const int eyeRenderMultisample = 1;
pRendertargetTexture = pRender->CreateTexture(
Texture_RGBA | Texture_RenderTarget | eyeRenderMultisample,
renderTargetSize.w, renderTargetSize.h, NULL);
// The actual RT size may be different due to HW limits.
renderTargetSize.w = pRendertargetTexture->GetWidth();
renderTargetSize.h = pRendertargetTexture->GetHeight();

*/

#else

#include <Engine.h>
#include "OculusDX.h"

void*	OculusInitWindow( void* pGamesWCVoid )
{
	return( NULL );
}

BOOL	EngineHasOculus( void )
{
	return( FALSE );
}

void InitializeOculusVR( void )
{
}

void*	OculusInitD3DDevice( void* pD3DVoid )
{
	return( NULL );
}

void		OculusReleaseGraphics( void )
{

}

void		OculusStartEndScene( void )
{

}

BOOL		OculusGetHeadOrientation( VECT* pxDir, VECT* pxUp )
{
	return( FALSE );
}

BOOL		OculusGetEulerAngles( float* pfYaw, float* pfPitch, float* pfRoll )
{
	return( FALSE );
}

void		OculusStartSceneLeftEye( uint32 ulClearCol )
{
}

void		OculusStartSceneRightEye(  uint32 ulClearCol )
{

}

void ShutdownOculusVR( void )
{

}

#endif // ifndef TOOL