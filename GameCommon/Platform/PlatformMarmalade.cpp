
#include "s3e.h"
#include "IwDebug.h"
#include "IwGx.h"

#ifdef USE_FLURRY_ANALYTICS
#include "s3eFlurry.h"
#endif

#include <StandardDef.h>
#include <System.h>
#include <Interface.h>
#include <System.h>

#include "Platform.h"

#define		SAVE_DATA_BUFFER_SIZE	32764

int		mnProfileMarker = 0;
uint32	maulProfileTimes[32];
char	maszProfileNames[32][32];
u64		mullPlatformLastTick = 0;
float	mfFrameDelta = 0.0f;
BOOL	msbAllowSleep = FALSE;

// -------- Forward declarations of gameside C functions

extern void		MainInitialise( void );
extern void		MainUpdate( void );
extern void		MainShutdown( void );

extern void		MainOnPress( int X, int Y );
extern void		MainOnRelease( int X, int Y );
//------------------------------------------------------------

int		mHoldPosX = NOTFOUND;
int		mHoldPosY = NOTFOUND;
BOOL	mbTouchOn = FALSE;
BYTE*	mpSaveDataBuffer = NULL;
BYTE*	mpSaveDataBufferFile = NULL;
int		mnSaveDataBufferPos = 0;
BOOL	mbSaveDataWrite = FALSE;
int		mnSaveDataReadSize;

BOOL	mbFlurryLoggingEnabled = TRUE;

#define		KEYBOARD_INPUT_BUFFER_SIZE	512

PlatformKeyboardMessageHandler		mfnKeyboardHandler = NULL;
int		mnKeyboardInputMaxLen = KEYBOARD_INPUT_BUFFER_SIZE;
char	mszKeyboardInputString[KEYBOARD_INPUT_BUFFER_SIZE] = "";
char	mszKeyboardDisplayString[KEYBOARD_INPUT_BUFFER_SIZE] = "";
int		mnKeyboardInputCursor = 0;
BOOL	mbKeyboardCursorFlashOn = FALSE;
float	mfKeyboardCursorFlashTimer = 0.0f;
BOOL	mbKeyboardOSKIsActive = FALSE;
// Last key that was "pressed" (has just been released)
static s3eKey g_LastKeyPressed = s3eKeyFirst;

void		MarmaladeKeyboardInputUpdate( float fDelta )
{
    // Check for character input being terminated by the platform/user
    int newCharState = s3eKeyboardGetInt(S3E_KEYBOARD_GET_CHAR);

    s3eWChar ch;

	if (mbKeyboardOSKIsActive)
    {
        // Get all chars that were internally buffered since the last update
        while ((ch = s3eKeyboardGetChar()) != S3E_WEOF)
        {
			switch ( ch )
			{
			case 8: // backspace
				if ( mnKeyboardInputCursor > 0 )
				{
					mnKeyboardInputCursor--;
					mszKeyboardInputString[mnKeyboardInputCursor] = 0;
				}
				break;
			default:
				if ( ch >= 32 )
				{
					mszKeyboardInputString[mnKeyboardInputCursor] = (char)ch;
					if (mnKeyboardInputCursor < (KEYBOARD_INPUT_BUFFER_SIZE-1))
					{
						mnKeyboardInputCursor++;
						mszKeyboardInputString[mnKeyboardInputCursor] = 0;
					}
				}
				break;
			}
        }
    }

    // Get the last key that was pressed. Will be return s3eKeyFirst if no key
    // was pressed between the previous calls to s3eKeyboardUpdate().
    g_LastKeyPressed = s3eKeyboardAnyKey();

	if ( g_LastKeyPressed == s3eKeyEnter )
	{
		if ( mfnKeyboardHandler )
		{
			mfnKeyboardHandler( 1, mszKeyboardInputString );
		}
		mnKeyboardInputCursor = 0;
		mszKeyboardInputString[mnKeyboardInputCursor] = 0;
		mbKeyboardOSKIsActive = FALSE;
		s3eKeyboardSetInt( S3E_KEYBOARD_GET_CHAR, false );
	}
}

BOOL		PlatformHasKeyboard( void )
{
	return( FALSE );
}

void		PlatformKeyboardActivate( int mode, const char* szStartText, const char* szPrompt )
{
	strcpy( mszKeyboardInputString, szStartText );
	mnKeyboardInputCursor = strlen( szStartText );

	s3eKeyboardSetInt( S3E_KEYBOARD_GET_CHAR, false );
	s3eKeyboardSetInt( S3E_KEYBOARD_GET_CHAR, true );
	mbKeyboardOSKIsActive = TRUE;
}


void	PlatformGetMACaddress( unsigned char* szOutBuffer )
{
	strcpy( (char*)szOutBuffer, "MOBILE" );
}


void		PlatformKeyboardDeactivate( void )
{

}

BOOL		PlatformKeyboardOSKIsActive( void )
{
	return( mbKeyboardOSKIsActive );
}

void		PlatformKeyboardRegisterHandler( PlatformKeyboardMessageHandler fnHandler )
{
	mfnKeyboardHandler = fnHandler;
}

void				PlatformKeyboardSetInputString( const char* szNewInputString )
{
	if ( !szNewInputString )
	{
		mszKeyboardInputString[0] = 0;
		mnKeyboardInputCursor = 0;
	}
	else
	{
		strcpy( mszKeyboardInputString, szNewInputString );
		mnKeyboardInputCursor = strlen( mszKeyboardInputString );
	}
}

const char*		PlatformKeyboardGetInputString( BOOL bIncludeCursor )
{
	if ( ( bIncludeCursor ) &&
		 ( mbKeyboardCursorFlashOn ) )
	{
		strcpy( mszKeyboardDisplayString, mszKeyboardInputString );
		strcat( mszKeyboardDisplayString, "_" );
		return( mszKeyboardDisplayString );
	}
	else
	{
		return( mszKeyboardInputString );
	}
}

BOOL				PlatformWindowIsFocused( void )
{
	return( TRUE );
}

void		PlatformSetMouseOverCursor( BOOL bFlag )
{
	// nothin to do on marmaladeness

}

 void				PlatformResizeWindow( BOOL bFullScreen, int nWindowWidth, int nWindowHeight )
 {
	 // nothin to do on marmaladeness
 }

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

void		PlatformMoveWindow( int nLeft, int nTop )
{
	// nothing to do for maramalde
}

void				PlatformGetFullscreenSize( int* pnWidth, int* pnHeight )
{
	*pnWidth = s3eSurfaceGetInt( S3E_SURFACE_DEVICE_WIDTH );
	*pnHeight = s3eSurfaceGetInt( S3E_SURFACE_DEVICE_HEIGHT );
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
		SysSaveDataSave( mpSaveDataBuffer, mnSaveDataBufferPos + 4);
	}
	free( mpSaveDataBuffer );
	mpSaveDataBuffer = NULL;
	mnSaveDataBufferPos = 0;
	return( 1 );
}


void	PlatformGetCurrentCursorPosition( int* pnX, int* pnY )
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
		mHoldPosX = pEvent->m_x;
		mHoldPosY = pEvent->m_y;

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

float			PlatformGetFrameDelta( void )
{
	return( mfFrameDelta );
}


BOOL		PlatformUpdateFrame( void )
{
u64	ullCurrentTick = SysGetMicrosecondTick();
float	fDelta = ( (float)(ullCurrentTick - mullPlatformLastTick ) ) * 0.000001f;

	if ( fDelta > 0.0f )
	{
		if ( fDelta > 0.5f ) fDelta = 0.5f;
		mullPlatformLastTick = ullCurrentTick;

		if ( ( fDelta > 0.01650f ) &&
			 ( fDelta < 0.01684f ) )
		{
			fDelta = 0.016667f;
		}
			 
		mfFrameDelta = fDelta;
		
		mfKeyboardCursorFlashTimer += fDelta;
		if ( mfKeyboardCursorFlashTimer > 0.5f )
		{
			mbKeyboardCursorFlashOn = !mbKeyboardCursorFlashOn;
			mfKeyboardCursorFlashTimer = 0.0f;
		}
		//Update the input systems
		s3eKeyboardUpdate();
		s3ePointerUpdate();

		// Keep the screen active..
		if ( msbAllowSleep == FALSE )
		{
			s3eDeviceBacklightOn();
		}

		int	  touchX = s3ePointerGetX();
		int	  touchY = s3ePointerGetY();

		if ( ( s3eKeyboardGetState( s3eKeyEsc ) & S3E_KEY_STATE_PRESSED ) ||
			 ( s3eKeyboardGetState( s3eKeyBack ) & S3E_KEY_STATE_PRESSED ) )
		{
			s3eDeviceRequestQuit();
		}

		MarmaladeKeyboardInputUpdate( 0.1f );
		return( TRUE );
	}
	return( FALSE );
}


void				PlatformInit( const char* szWindowName, BOOL bAllowResize, BOOL bAllowSleep )
{
	msbAllowSleep = bAllowSleep;
	s3ePointerRegister(S3E_POINTER_BUTTON_EVENT, (s3eCallback)HandleSingleTouchButtonCB, NULL);
	s3ePointerRegister(S3E_POINTER_MOTION_EVENT, (s3eCallback)HandleSingleTouchMotionCB, NULL);

}

void		PlatformShutdown( )
{
	s3ePointerUnRegister(S3E_POINTER_BUTTON_EVENT, (s3eCallback)HandleSingleTouchButtonCB);
	s3ePointerUnRegister(S3E_POINTER_MOTION_EVENT, (s3eCallback)HandleSingleTouchMotionCB);
}


void		PlatformSetPrimaryLight( VECT* pxLightDir, float fStrength )
{
CIwColour    s_ColSceneAmb = {0x60, 0x60, 0x60, 0xFF};
CIwColour    s_ColSceneDiff = {0xC0, 0xB8, 0xA0, 0xFF};
 
	IwGxSetLightType(0, IW_GX_LIGHT_AMBIENT);
    IwGxSetLightType(1, IW_GX_LIGHT_DIFFUSE);

	CIwFVec3 dir(pxLightDir->x, pxLightDir->y, pxLightDir->z);
    IwGxSetLightDirn(1, &dir);

	IwGxSetLightCol(0, 0x80, 0x80, 0x80);
	IwGxSetLightCol(1, 0xF0, 0xE8, 0xD0);

//	IwGxSetLightCol(1, 0x10, 0x28, 0x10);
//	IwGxSetLightSpecularCol( 1, 0xF0, 0x00, 0x00, 0xFF );
}

void		PlatformAnalyticsInit( const char* szAnalyticsKey )
{
#ifdef USE_FLURRY_ANALYTICS
    if ( s3eFlurryAvailable() )
	{
		s3eFlurryStartSession( szAnalyticsKey );
		mbFlurryLoggingEnabled = TRUE;
	}
#endif
}

void	PlatformLogEvent( const char* szEventString, const char* szKey, int nValue )
{

    if ( mbFlurryLoggingEnabled )
	{
#ifdef USE_FLURRY_ANALYTICS
	char	acString[256];

	    if ( s3eFlurryAvailable() )
		{
			if ( szKey )
			{
				sprintf( acString, "%s|%d", szKey, nValue );
				s3eFlurryLogEventParams( szEventString, acString );
			}
			else
			{
				s3eFlurryLogEvent(szEventString);
			}
		}
#endif
	}
/*
	s3eFlurryLogEventTimed("whileTimedEvent");
    s3eFlurryLogEventParams("whileParamsEvent", "key|value");
    s3eFlurryLogEventParamsTimed("whileTimedParamsEvent", "key1|value1");
*/
}


void		PlatformProfileTextEnd( void )
{
int		nLoop;
uint32	ulTime;

	maulProfileTimes[ mnProfileMarker ] = SysGetTick();

	for( nLoop = 0; nLoop < mnProfileMarker; nLoop++ )
	{
		ulTime = maulProfileTimes[nLoop+1] - maulProfileTimes[nLoop];
		s3eDebugTracePrintf("%s  --- %d ms", maszProfileNames[nLoop], (int)ulTime );
	}

	mnProfileMarker = 0;
}



void		PlatformProfileTextMark( const char* szName )
{
int		nMarker = mnProfileMarker % 32;

	maulProfileTimes[mnProfileMarker] = SysGetTick();
	strcpy( maszProfileNames[mnProfileMarker], szName );
	mnProfileMarker++;
}

void		PlatformProfileRender( void )
{
#ifdef PROFILE_ENABLED
int		nLoop;
uint32	ulTime;
char	acString[256];
int		nLineY = 150;

	maulProfileTimes[ mnProfileMarker ] = SysGetTick();

	for( nLoop = 0; nLoop < mnProfileMarker; nLoop++ )
	{
		ulTime = maulProfileTimes[nLoop+1] - maulProfileTimes[nLoop];
		InterfaceTextRight( 0, 250, nLineY, maszProfileNames[nLoop], 0xD0C0C0C0, 0 );
		sprintf( acString, "%dms", ulTime );
		InterfaceRect( 0, 252, nLineY, ulTime*5, 18, 0xc020a030 );
		InterfaceText( 1, 255, nLineY, acString, 0xD0E0E0E0, 0 );
		nLineY += 20;
	}

	mnProfileMarker = 0;
#endif
}

void		PlatformProfileMark( const char* szName )
{
#ifdef PROFILE_ENABLED
	PlatformProfileTextMark( szName );
#endif
}

void		PlatformProfileStart( const char* szName )
{
}

void		PlatformProfileStop( void )
{
}

void		PlatformVibrate( int Mode )
{
	switch( Mode )
	{
	case 0:
	default:
		s3eVibraVibrate(255, 20);
		break;
	case 1:
		s3eVibraVibrate(255, 50);
		break;
	}
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

	s3eDebugTracePrintf("------------------------------------------------------------------------------------");
	s3eDebugTracePrintf("-- MAIN start  ---------------------------------------------------------------------");
	s3eDebugTracePrintf("------------------------------------------------------------------------------------");

	MainInitialise();

    // Register a callback so we are informed when the screen is rotated
    s3eSurfaceRegister(S3E_SURFACE_SCREENSIZE, ScreenSizeChangeCallback, NULL);

	s3eDebugTracePrintf("BaseInit done");
    // Loop forever, until the user or the OS performs some action to quit the app
    while (!s3eDeviceCheckQuitRequest())
    {
//		s3eDeviceCheckPauseRequest
		if ( PlatformUpdateFrame() == TRUE )
		{
			IW_PROFILE_NEWFRAME();
			PlatformSetPrimaryLight( &xLightDir, 1.0f );
			MainUpdate();

			IwGxTickUpdate();
		}

        // Sleep for 0ms to allow the OS to process events etc.
        s3eDeviceYield(0);
    }

    s3eSurfaceUnRegister(S3E_SURFACE_SCREENSIZE, ScreenSizeChangeCallback);

	MainShutdown();

	PlatformShutdown();
	
    // Return
    return 0;
}
