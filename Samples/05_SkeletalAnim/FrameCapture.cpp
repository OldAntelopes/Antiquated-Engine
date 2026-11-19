
#include <StdWinInclude.h>
#include <d3dx9.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>

#include <StandardDef.h>
#include <Interface.h>

#include "FrameCapture.h"

#pragma comment( lib, "mfplat.lib" )
#pragma comment( lib, "mfreadwrite.lib" )
#pragma comment( lib, "mfuuid.lib" )

static IMFSinkWriter*		mspVideoSinkWriter = NULL;
static DWORD				msdwVideoStreamIndex = 0;
static int					msnVideoWidth = 0;
static int					msnVideoHeight = 0;
static int					msnVideoFPS = 30;
static LONGLONG				msllVideoFrameTime = 0;
static int					msnVideoFrameNum = 0;
static IDirect3DSurface9*	mspCaptureSurface = NULL;

//-------------------------------------------------------------------------
// Grab the current back buffer into a system memory surface (created on demand)
//-------------------------------------------------------------------------
static IDirect3DSurface9*	FrameCaptureGrabBackBuffer( void )
{
LPGRAPHICSDEVICE		pDevice = InterfaceGetD3DDevice();
IDirect3DSurface9*		pBackBuffer = NULL;
D3DSURFACE_DESC			xDesc;

	if ( pDevice == NULL ) return( NULL );

	if ( FAILED( pDevice->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer ) ) )
	{
		return( NULL );
	}
	pBackBuffer->GetDesc( &xDesc );

	if ( mspCaptureSurface )
	{
	D3DSURFACE_DESC		xCaptureDesc;

		mspCaptureSurface->GetDesc( &xCaptureDesc );
		if ( ( xCaptureDesc.Width != xDesc.Width ) ||
			 ( xCaptureDesc.Height != xDesc.Height ) ||
			 ( xCaptureDesc.Format != xDesc.Format ) )
		{
			SAFE_RELEASE( mspCaptureSurface );
		}
	}

	if ( mspCaptureSurface == NULL )
	{
		pDevice->CreateOffscreenPlainSurface( xDesc.Width, xDesc.Height, xDesc.Format, D3DPOOL_SYSTEMMEM, &mspCaptureSurface, NULL );
	}

	if ( mspCaptureSurface )
	{
		if ( FAILED( pDevice->GetRenderTargetData( pBackBuffer, mspCaptureSurface ) ) )
		{
			SAFE_RELEASE( pBackBuffer );
			return( NULL );
		}
	}
	SAFE_RELEASE( pBackBuffer );
	return( mspCaptureSurface );
}

//-------------------------------------------------------------------------
// Function    : FrameCaptureSaveScreenshot
// Description : Saves the back buffer to an image file (format from the extension - .png, .bmp, .jpg)
//-------------------------------------------------------------------------
BOOL		FrameCaptureSaveScreenshot( const char* szFilename )
{
IDirect3DSurface9*		pSurface = FrameCaptureGrabBackBuffer();
D3DXIMAGE_FILEFORMAT	nFormat = D3DXIFF_PNG;
const char*				szExtension = SysGetFileExtension( szFilename );

	if ( pSurface == NULL ) return( FALSE );

	if ( szExtension )
	{
		if ( tinstricmp( szExtension, "bmp" ) == 0 ) nFormat = D3DXIFF_BMP;
		else if ( tinstricmp( szExtension, "jpg" ) == 0 ) nFormat = D3DXIFF_JPG;
	}
	return( SUCCEEDED( D3DXSaveSurfaceToFileA( szFilename, nFormat, pSurface, NULL, NULL ) ) );
}

//-------------------------------------------------------------------------
// Function    : FrameCaptureVideoStart
// Description : Opens an H.264 .mp4 for writing. Frames are added with FrameCaptureVideoAddFrame
//-------------------------------------------------------------------------
BOOL		FrameCaptureVideoStart( const char* szFilename, int nWidth, int nHeight, int nFPS )
{
IMFMediaType*	pOutputType = NULL;
IMFMediaType*	pInputType = NULL;
WCHAR			wszFilename[MAX_PATH];
HRESULT			hr;

	msnVideoWidth = nWidth & ~1;
	msnVideoHeight = nHeight & ~1;
	msnVideoFPS = nFPS;
	msllVideoFrameTime = 0;
	msnVideoFrameNum = 0;

	MultiByteToWideChar( CP_ACP, 0, szFilename, -1, wszFilename, MAX_PATH );

	hr = MFStartup( MF_VERSION );
	if ( SUCCEEDED( hr ) ) hr = MFCreateSinkWriterFromURL( wszFilename, NULL, NULL, &mspVideoSinkWriter );

	// Output : H.264
	if ( SUCCEEDED( hr ) ) hr = MFCreateMediaType( &pOutputType );
	if ( SUCCEEDED( hr ) ) hr = pOutputType->SetGUID( MF_MT_MAJOR_TYPE, MFMediaType_Video );
	if ( SUCCEEDED( hr ) ) hr = pOutputType->SetGUID( MF_MT_SUBTYPE, MFVideoFormat_H264 );
	if ( SUCCEEDED( hr ) ) hr = pOutputType->SetUINT32( MF_MT_AVG_BITRATE, 8000000 );
	if ( SUCCEEDED( hr ) ) hr = pOutputType->SetUINT32( MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive );
	if ( SUCCEEDED( hr ) ) hr = MFSetAttributeSize( pOutputType, MF_MT_FRAME_SIZE, msnVideoWidth, msnVideoHeight );
	if ( SUCCEEDED( hr ) ) hr = MFSetAttributeRatio( pOutputType, MF_MT_FRAME_RATE, nFPS, 1 );
	if ( SUCCEEDED( hr ) ) hr = MFSetAttributeRatio( pOutputType, MF_MT_PIXEL_ASPECT_RATIO, 1, 1 );
	if ( SUCCEEDED( hr ) ) hr = mspVideoSinkWriter->AddStream( pOutputType, &msdwVideoStreamIndex );

	// Input : RGB32 top-down
	if ( SUCCEEDED( hr ) ) hr = MFCreateMediaType( &pInputType );
	if ( SUCCEEDED( hr ) ) hr = pInputType->SetGUID( MF_MT_MAJOR_TYPE, MFMediaType_Video );
	if ( SUCCEEDED( hr ) ) hr = pInputType->SetGUID( MF_MT_SUBTYPE, MFVideoFormat_RGB32 );
	if ( SUCCEEDED( hr ) ) hr = pInputType->SetUINT32( MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive );
	if ( SUCCEEDED( hr ) ) hr = pInputType->SetUINT32( MF_MT_DEFAULT_STRIDE, (UINT32)( msnVideoWidth * 4 ) );
	if ( SUCCEEDED( hr ) ) hr = MFSetAttributeSize( pInputType, MF_MT_FRAME_SIZE, msnVideoWidth, msnVideoHeight );
	if ( SUCCEEDED( hr ) ) hr = MFSetAttributeRatio( pInputType, MF_MT_FRAME_RATE, nFPS, 1 );
	if ( SUCCEEDED( hr ) ) hr = MFSetAttributeRatio( pInputType, MF_MT_PIXEL_ASPECT_RATIO, 1, 1 );
	if ( SUCCEEDED( hr ) ) hr = mspVideoSinkWriter->SetInputMediaType( msdwVideoStreamIndex, pInputType, NULL );

	if ( SUCCEEDED( hr ) ) hr = mspVideoSinkWriter->BeginWriting();

	SAFE_RELEASE( pOutputType );
	SAFE_RELEASE( pInputType );

	if ( FAILED( hr ) )
	{
		SysDebugPrint( "FrameCaptureVideoStart failed (0x%08x)\n", hr );
		SAFE_RELEASE( mspVideoSinkWriter );
		return( FALSE );
	}
	return( TRUE );
}

//-------------------------------------------------------------------------
// Function    : FrameCaptureVideoAddFrame
// Description : Appends the current back buffer as the next video frame
//-------------------------------------------------------------------------
BOOL		FrameCaptureVideoAddFrame( void )
{
IDirect3DSurface9*	pSurface;
D3DLOCKED_RECT		xLocked;
IMFSample*			pSample = NULL;
IMFMediaBuffer*		pBuffer = NULL;
BYTE*				pbDest = NULL;
LONGLONG			llDuration = 10000000 / msnVideoFPS;
DWORD				dwBufferSize = msnVideoWidth * msnVideoHeight * 4;
HRESULT				hr;
int					nRow;

	if ( mspVideoSinkWriter == NULL ) return( FALSE );

	pSurface = FrameCaptureGrabBackBuffer();
	if ( pSurface == NULL ) return( FALSE );

	hr = MFCreateMemoryBuffer( dwBufferSize, &pBuffer );
	if ( SUCCEEDED( hr ) ) hr = pBuffer->Lock( &pbDest, NULL, NULL );
	if ( SUCCEEDED( hr ) )
	{
		if ( SUCCEEDED( pSurface->LockRect( &xLocked, NULL, D3DLOCK_READONLY ) ) )
		{
			for ( nRow = 0; nRow < msnVideoHeight; nRow++ )
			{
				memcpy( pbDest + (nRow * msnVideoWidth * 4), (BYTE*)xLocked.pBits + (nRow * xLocked.Pitch), msnVideoWidth * 4 );
			}
			pSurface->UnlockRect();
		}
		pBuffer->Unlock();
	}
	if ( SUCCEEDED( hr ) ) hr = pBuffer->SetCurrentLength( dwBufferSize );
	if ( SUCCEEDED( hr ) ) hr = MFCreateSample( &pSample );
	if ( SUCCEEDED( hr ) ) hr = pSample->AddBuffer( pBuffer );
	if ( SUCCEEDED( hr ) ) hr = pSample->SetSampleTime( msllVideoFrameTime );
	if ( SUCCEEDED( hr ) ) hr = pSample->SetSampleDuration( llDuration );
	if ( SUCCEEDED( hr ) ) hr = mspVideoSinkWriter->WriteSample( msdwVideoStreamIndex, pSample );

	msllVideoFrameTime += llDuration;
	msnVideoFrameNum++;

	SAFE_RELEASE( pSample );
	SAFE_RELEASE( pBuffer );
	return( SUCCEEDED( hr ) );
}

//-------------------------------------------------------------------------
// Function    : FrameCaptureVideoEnd
//-------------------------------------------------------------------------
void		FrameCaptureVideoEnd( void )
{
	if ( mspVideoSinkWriter )
	{
		mspVideoSinkWriter->Finalize();
		SAFE_RELEASE( mspVideoSinkWriter );
		MFShutdown();
	}
	SAFE_RELEASE( mspCaptureSurface );
}
