
#include "s3e.h"
#include "IwDebug.h"
#include "IwGx.h"

#include <StandardDef.h>
#include <System.h>
#include <Interface.h>
#include <System.h>

#include "Platform.h"
#include "../Herd.h"

#define		SAVE_DATA_BUFFER_SIZE	32764

int		mHoldPosX = NOTFOUND;
int		mHoldPosY = NOTFOUND;
BOOL	mbTouchOn = FALSE;
BYTE*	mpSaveDataBuffer = NULL;
BYTE*	mpSaveDataBufferFile = NULL;
int		mnSaveDataBufferPos = 0;
BOOL	mbSaveDataWrite = FALSE;
int		mnSaveDataReadSize;

int		SysSaveDataLoad( byte* pbData, int nSize )
{
s3eResult		ret;

	ret = s3eSecureStorageGet (pbData, (uint16)nSize);

	if ( ret == S3E_RESULT_ERROR )
	{
		switch( s3eSecureStorageGetError() )
		{
		case S3E_SECURESTORAGE_ERR_NONE:
		case S3E_SECURESTORAGE_ERR_PARAM:
			break;
		case S3E_SECURESTORAGE_ERR_NOT_FOUND:
			return( -2 );
		case S3E_SECURESTORAGE_ERR_DEVICE:
		case S3E_SECURESTORAGE_ERR_INSUFF:
		case S3E_SECURESTORAGE_ERR_CORRUPT:
			// TODO
			break;
		}
	}
	else
	{
		return( 1 );
	}
	return( 0 );
}

int		SysSaveDataSave( byte* pbData, int nSize )
{
s3eResult		ret;

	ret = s3eSecureStoragePut (pbData, (uint16)(nSize) );

	if ( ret == S3E_RESULT_ERROR )
	{
		return( 0 );
	}
	return( 1 );
}

int		PlatformSaveDataFileOpen( int slot, BOOL bWrite )
{
	mpSaveDataBuffer = (BYTE*)( malloc( SAVE_DATA_BUFFER_SIZE + 4 ) );
	if ( mpSaveDataBuffer )
	{
		mpSaveDataBufferFile = mpSaveDataBuffer + 4;
		mnSaveDataBufferPos = 0;
		if ( !bWrite )
		{
			if ( SysSaveDataLoad( mpSaveDataBuffer, SAVE_DATA_BUFFER_SIZE - 1 ) < 0 )
			{
				return( -1 );
			}
			mnSaveDataReadSize = *( (int*)mpSaveDataBuffer );
		}
		mbSaveDataWrite = bWrite;
		return( 1 );
	}
	else
	{
		// TODO - ASSERT!
	}
	return( -2 );

}

int		PlatformSaveDataFileWrite( unsigned char* pucData, int nMemSize )
{
	if ( mnSaveDataBufferPos + nMemSize >= SAVE_DATA_BUFFER_SIZE )
	{
		// TODO - ASSERT!
	}
	memcpy( mpSaveDataBufferFile + mnSaveDataBufferPos, pucData, nMemSize );
	mnSaveDataBufferPos += nMemSize;
	return( nMemSize );
}

int		PlatformSaveDataFileRead( unsigned char* pucData, int nMemSize )
{
int		nReadSize = nMemSize;

	if ( mnSaveDataBufferPos + nMemSize > mnSaveDataReadSize )
	{
		nReadSize = mnSaveDataReadSize - mnSaveDataBufferPos;
	}
	memcpy( pucData, mpSaveDataBufferFile + mnSaveDataBufferPos, nReadSize );
	mnSaveDataBufferPos += nReadSize;
	return( nReadSize );
}

int		PlatformSaveDataFileClose( void )
{
	if ( mbSaveDataWrite )
	{
		*( (int*)( mpSaveDataBuffer ) ) = mnSaveDataBufferPos;
		SysSaveDataSave( mpSaveDataBuffer, mnSaveDataBufferPos );
	}
	free( mpSaveDataBuffer );
	mpSaveDataBuffer = NULL;
	mnSaveDataBufferPos = 0;
	return( 1 );
}


void	PlatformGetCurrentHoldPosition( int* pnX, int* pnY )
{
	*pnX = mHoldPosX;
	*pnY = mHoldPosY;
}

void HandleSingleTouchMotionCB(s3ePointerMotionEvent* pEvent)
{
	if ( mbTouchOn == TRUE )
	{
//	char	acString[256];

		mHoldPosX = pEvent->m_x;
		mHoldPosY = pEvent->m_y;
	}
}


void HandleSingleTouchButtonCB(s3ePointerEvent* pEvent)
{
//	if ( pEvent->m_TouchID == 0 )
//	{
		if ( pEvent->m_Pressed == 1 )
		{
			if ( mbTouchOn == FALSE )
			{
				MainOnPress( pEvent->m_x, pEvent->m_y );
				mbTouchOn = TRUE;
			}
		}
		else
		{
			mbTouchOn = FALSE;
			MainOnRelease( pEvent->m_x, pEvent->m_y );
		}
//	}
}


void		PlatformUpdateFrame( void )
{
	//Update the input systems
	s3eKeyboardUpdate();
	s3ePointerUpdate();

	int	  touchX = s3ePointerGetX();
	int	  touchY = s3ePointerGetY();

	if ( ( s3eKeyboardGetState( s3eKeyEsc ) & S3E_KEY_STATE_PRESSED ) ||
		 ( s3eKeyboardGetState( s3eKeyBack ) & S3E_KEY_STATE_PRESSED ) )
	{
		s3eDeviceRequestQuit();
	}
}


void		PlatformInit( void )
{
	IwGxInit();

	s3ePointerRegister(S3E_POINTER_BUTTON_EVENT, (s3eCallback)HandleSingleTouchButtonCB, NULL);
	s3ePointerRegister(S3E_POINTER_MOTION_EVENT, (s3eCallback)HandleSingleTouchMotionCB, NULL);
}

void		PlatformSetPrimaryLight( VECT* pxLightDir, float fStrength )
{
CIwColour    s_ColSceneAmb = {0x60, 0x60, 0x60, 0xFF};
CIwColour    s_ColSceneDiff = {0xC0, 0xB8, 0xA0, 0xFF};
 
	IwGxSetLightType(0, IW_GX_LIGHT_AMBIENT);
    IwGxSetLightType(1, IW_GX_LIGHT_DIFFUSE);

	CIwFVec3 dir(pxLightDir->x, pxLightDir->y, pxLightDir->z);
    IwGxSetLightDirn(1, &dir);

	IwGxSetLightCol(0, 0x10, 0x80, 0x10);
	IwGxSetLightCol(1, 0xF0, 0xE8, 0xD0);

//	IwGxSetLightCol(1, 0x10, 0x28, 0x10);
//	IwGxSetLightSpecularCol( 1, 0xF0, 0x00, 0x00, 0xFF );
}

// Callback from S3E when the screen size or rotation changes
int32 ScreenSizeChangeCallback(void* systemData, void* userData)
{
/*
s3eSurfaceOrientation *obj = (s3eSurfaceOrientation *)systemData;
int		nScreenWidth = obj->m_Width;
int		nScreenHeight = obj->m_Height;

	if (obj->m_OrientationChanged)
	{
//        orientation = ;
		IwGxSetScreenOrient( (IwGxScreenOrient)obj->m_DeviceBlitDirection);
    }
	InterfaceSetWindowSize( TRUE, nScreenWidth, nScreenHeight, FALSE ); 
*/
    return 0;
}


// Main entry point for the application
int main()
{
	VECT	xLightDir = { -1.0f, -1.0f, -1.0f };

	s3eDebugTracePrintf("Start");
	MainInitialise();

    // Register a callback so we are informed when the screen is rotated
    s3eSurfaceRegister(S3E_SURFACE_SCREENSIZE, ScreenSizeChangeCallback, NULL);

	s3eDebugTracePrintf("BaseInit done");
    // Loop forever, until the user or the OS performs some action to quit the app
    while (!s3eDeviceCheckQuitRequest())
    {

//		s3eDeviceCheckPauseRequest
		IW_PROFILE_NEWFRAME();

		PlatformUpdateFrame();
		PlatformSetPrimaryLight( &xLightDir, 1.0f );
		MainUpdate();

		IwGxTickUpdate();

        // Sleep for 0ms to allow the OS to process events etc.
        s3eDeviceYield(0);
    }

    s3eSurfaceUnRegister(S3E_SURFACE_SCREENSIZE, ScreenSizeChangeCallback);

	MainShutdown();

	IwGxTerminate();

    // Return
    return 0;
}
