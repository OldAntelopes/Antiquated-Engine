//-----------------------------------------------------------------------------------------------------------------------
// PlatformWin32.cpp
//
// Windows specific stuff for Base Juliet
//
//-----------------------------------------------------------------------------------------------------------------------

#include <stdio.h>
#include "StdWinInclude.h"
#include "StandardDef.h"	
#include "Interface.h"		

#ifdef _DEBUG
#include "crtdbg.h"
#endif

#include "../Console/Console.h"
#include "Platform.h"


// -------- Forward declarations of gameside C functions

// TODO - Have these passed in on init rather than assumed like this..
extern void		MainInitialise( void );
extern void		MainUpdate( void );
extern void		MainShutdown( void );

extern void		MainOnPress( int X, int Y );
extern void		MainOnRelease( int X, int Y );
extern void		MainOnReleaseRightButton( int X, int Y );


//-----------------------------------------------------------------------------------------------------------------------
// ------------------------------------------- Windows specific stuff

char	mszWindowName[128] = "Default Window Name";

HWND	ghPlatformWin32wndMain;		// Global window handle
HCURSOR		mhPlatformWin32ArrowCursor;
HCURSOR		mhPlatformWin32HandCursor;

BOOL		mbHandCursorSet = FALSE;
short	mwMouseX;
short	mwMouseY;
BOOL	mbThisWindowIsFocused = TRUE;
int		mnPendingTouchRelease = 0;

FILE*	mspSaveFile = NULL;
FILE*	mspLoadFile = NULL;

#define		KEYBOARD_INPUT_BUFFER_SIZE	512

PlatformKeyboardMessageHandler			mfnKeyboardHandler = NULL;
PlatformKeyboardSpecialKeyPressHandler	mfnSpecialKeyUpHandler = NULL;
PlatformControlZoomHandler				mfnZoomControlHandler = NULL;

int		mnKeyboardInputMaxLen = KEYBOARD_INPUT_BUFFER_SIZE;
char	mszKeyboardInputString[KEYBOARD_INPUT_BUFFER_SIZE] = "";
char	mszKeyboardDisplayString[KEYBOARD_INPUT_BUFFER_SIZE] = "";
int		mnKeyboardInputCursor = 0;
BOOL	mbKeyboardShiftOn = FALSE;
BOOL	mbKeyboardCursorFlashOn = FALSE;
float	mfKeyboardCursorFlashTimer = 0.0f;
BOOL	mbActivateMouseOverCursor = FALSE;

u64		mullPlatformLastTick = 0;
float	mfFrameDelta = 0.0f;
int		mnProfileMarker = 0;
uint32	maulProfileTimes[32];
char	maszProfileNames[32][32];

#ifdef _DEBUG
void DetectMemoryLeaks() 
{ 
   _CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF|_CRTDBG_ALLOC_MEM_DF); 
   _CrtSetReportMode(_CRT_ASSERT,_CRTDBG_MODE_FILE); 
   _CrtSetReportFile(_CRT_ASSERT,_CRTDBG_FILE_STDERR); 

//	_CrtSetBreakAlloc( 1844 );
} 
#endif

void		PlatformProfileTextEnd( void )
{
int		nLoop;
uint32	ulTime;

	maulProfileTimes[ mnProfileMarker ] = SysGetTick();

	for( nLoop = 0; nLoop < mnProfileMarker; nLoop++ )
	{
		ulTime = maulProfileTimes[nLoop+1] - maulProfileTimes[nLoop];
		SysDebugPrint("%s  --- %d ms", maszProfileNames[nLoop], (int)ulTime );
	}

	mnProfileMarker = 0;
}



int		SysSaveDataLoad( byte* pbData, int nSize )
{
	// STUB - TODO - Implementation
	return( 0 );
}

int		SysSaveDataSave( byte* pbData, int nSize )
{
	// STUB - TODO - Implementation
	return( 0 );
}



void		PlatformVibrate( int Mode )
{
// stub
}

void		PlatformAdDisplay( BOOL bShow )
{
// stub
}

void	PlatformLogEvent( const char* szEventString, const char* szKey, int nValue )
{
	// STUB - TODO - Flurry or similar?

}
void		PlatformAnalyticsInit( const char* szAnalyticsKey )
{
	// STUB - TODO - Flurry or similar?
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

BOOL				PlatformWindowIsFocused( void )
{
	return( mbThisWindowIsFocused );
}

void*			PlatformGetWindowHandle( void )
{
	return( (void*)( ghPlatformWin32wndMain ) );
}

BOOL				PlatformHasKeyboard( void )
{
	return( TRUE );
}

void		PlatformKeyboardActivate( int mode, const char* szStartText, const char* szPrompt )
{
	strcpy( mszKeyboardInputString, szStartText );
	mnKeyboardInputCursor = strlen( szStartText );
} 

void		PlatformKeyboardDeactivate( void )
{

}

BOOL		PlatformKeyboardOSKIsActive( void )
{
	return( FALSE );
}


void				PlatformKeyboardRegisterZoomControlHandler( PlatformControlZoomHandler fnHandler )
{
	mfnZoomControlHandler = fnHandler;
}

void				PlatformKeyboardRegisterSpecialKeyUpHandler( PlatformKeyboardSpecialKeyPressHandler fnHandler )
{
	mfnSpecialKeyUpHandler = fnHandler;
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
				

void	PlatformPasteFromClipboardToConsole( void )
{
	if ( OpenClipboard( ghPlatformWin32wndMain ) == TRUE )
	{
		if ( IsClipboardFormatAvailable(CF_TEXT) == TRUE )
		{
		HANDLE	ret;
		
			ret = GetClipboardData( CF_TEXT );
			char * pszText = static_cast<char*>( GlobalLock(ret) );
  
			if ( pszText != NULL )
			{
			int		nPasteStrLen = strlen( pszText );
			int		nCurrentStrLen = strlen( mszKeyboardInputString );

				if ( nPasteStrLen + nCurrentStrLen < 250 ) 
				{
					strcat( mszKeyboardInputString, pszText );
					mnKeyboardInputCursor = nPasteStrLen + nCurrentStrLen;
				}
				GlobalUnlock( ret );

			}
		}
		CloseClipboard();
	}

}

void	PlatformKeyboardHandleChar( short wChar )
{
char	cCharToAddToString;
char	cTextType = 0;
//char	acDebug[16];

	cCharToAddToString = (char)( wChar );

	if ( ( cCharToAddToString == '¬' ) &&
		 ( ConsoleIsAvailable() ) )
	{
		ConsoleToggleLarge();
	}
	else if ( ( cCharToAddToString >= 32 ) &&
		 ( cCharToAddToString < 128 ) )
	{
		if ( ( cCharToAddToString == '`' ) &&
			 ( ConsoleIsAvailable() ) )
		{
			ConsoleToggleActive();
		}
		else if ( mnKeyboardInputCursor < KEYBOARD_INPUT_BUFFER_SIZE - 2 )
		{
			mszKeyboardInputString[ mnKeyboardInputCursor ] = cCharToAddToString;
			mnKeyboardInputCursor++;
			mszKeyboardInputString[mnKeyboardInputCursor] = 0;
		}
		else
		{
			// Do a lil ping noise to indicate we've reached the end of the buffer
//			SoundPlay( SOUND_MESSAGE );
		}
	}
}

void	PlatformKeyboardHandleSpecialKeyUp( int nKeyCode )
{
	switch ( nKeyCode )
	{
	case KEY_SHIFT:
		mbKeyboardShiftOn = FALSE;
		return;
	case 'V':
	case 'v':
		if ( SysCheckKeyState( KEY_CTRL ) == TRUE )
		{
			PlatformPasteFromClipboardToConsole();
			return;
		}
		break;
	default:
		if ( mfnSpecialKeyUpHandler )
		{
			mfnSpecialKeyUpHandler( nKeyCode );
		}
		break;
	}

}

void	PlatformKeyboardHandleSpecialKeyDown( int nKeyCode )
{
	switch ( nKeyCode )
	{
	case KEY_SHIFT:
		mbKeyboardShiftOn = TRUE;
		return;
	case KEY_BACKSPACE:
		if ( mnKeyboardInputCursor > 0 ) 
		{
			mnKeyboardInputCursor--;
			mszKeyboardInputString[mnKeyboardInputCursor] = 0;
		}
		break;
	case KEY_RETURN:
		if ( ConsoleIsActive() )
		{
			ConsoleOnPressEnter( mszKeyboardInputString );
			PlatformKeyboardSetInputString( "" );
		}
		else if ( mfnKeyboardHandler )
		{
			mfnKeyboardHandler( KEYBOARD_ON_PRESS_ENTER, mszKeyboardInputString );
		}
		break;
	case KEY_TAB:
		if ( mfnKeyboardHandler )
		{
			mfnKeyboardHandler( KEYBOARD_ON_PRESS_TAB, mszKeyboardInputString );
		}
		break;
	default:
		break;
	}
}

// --------------------------------------------------------------------------

int		PlatformSaveDataFileOpen( int slot, BOOL bWrite )
{
char		acFilename[256];

	if ( ( mspSaveFile != NULL ) ||
		 ( mspLoadFile != NULL ) )
	{
		return( -1 );
	}
	sprintf( acFilename, "SaveFile%02d.dat", slot );
	if ( bWrite )
	{
		mspSaveFile = fopen( acFilename, "wb" );
		// If couldn't write to folder
		if ( mspSaveFile == NULL )
		{
			sprintf( acFilename, "%s\\SaveFile%02d.dat", SysGetWritableDataFolderPath( "TractorCombatSimulator" ), slot );
			mspSaveFile = fopen( acFilename, "wb" );
		}
	}
	else
	{
		mspLoadFile = fopen( acFilename, "rb" );
		if ( mspLoadFile == NULL )
		{
			// Try the writable folder..
			sprintf( acFilename, "%s\\SaveFile%02d.dat", SysGetWritableDataFolderPath( "TractorCombatSimulator" ), slot );
			mspLoadFile = fopen( acFilename, "rb" );
			if ( mspLoadFile == NULL )
			{
				return( -1 );
			}
		}
	}
	return( 1 );
}

int		PlatformSaveDataFileWrite( unsigned char* pucData, int nMemSize )
{
	if ( mspSaveFile )
	{
		fwrite( pucData, nMemSize, 1, mspSaveFile );
		return( nMemSize );
	}
	return( -1 );

}

int		PlatformSaveDataFileRead( unsigned char* pucData, int nMemSize )
{
	if ( mspLoadFile )
	{
	int		nBytesRead = fread( pucData, nMemSize, 1, mspLoadFile );
		return( nBytesRead );
	}
	return( -1 );
}


int		PlatformSaveDataFileClose( void )
{
	if ( mspSaveFile )
	{
		fclose( mspSaveFile );
		mspSaveFile = NULL;
	}
	else if ( mspLoadFile )
	{
		fclose( mspLoadFile );
		mspLoadFile = NULL;
	}

	return( 1 );
}

void				PlatformSetMouseOverCursor( BOOL bFlag )
{
	mbActivateMouseOverCursor = bFlag;
}


void	PlatformGetCurrentCursorPosition( int* pnX, int* pnY )
{
	*pnX = mwMouseX;
	*pnY = mwMouseY;
}

void	PlatformSetCurrentCursorPosition( int X, int Y )
{
	mwMouseX = X;
	mwMouseY = Y;
}

void		PlatformGetFullscreenSize( int* pnWidth, int* pnHeight )
{
RECT DesktopRect;
// Gets the Desktop window
HWND hDesktop = GetDesktopWindow();
		
	// Gets the Desktop window rect or screen resolution in pixels
	GetWindowRect(hDesktop, &DesktopRect);

	*pnWidth = (DesktopRect.right-DesktopRect.left);
	*pnHeight = (DesktopRect.bottom-DesktopRect.top);
}

bool	mbPlatformWin32IsResizingWindow = false;
int		mnPlatformWin32ResizeW = 0;
int		mnPlatformWin32ResizeH = 0;
				
//-------------------------------------------------------------------------
// Function    : WindowsMsgProc
// Description : 
//-------------------------------------------------------------------------
LRESULT WINAPI WindowsMsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch( msg )
    {
	case WM_ACTIVATEAPP:
		// If we're losing focus
		if ( wParam == FALSE )
		{
			mbThisWindowIsFocused = FALSE;
		}
		else
		{
			mbThisWindowIsFocused = TRUE;
		}
		break;
 	case WM_MOUSEMOVE:
		mwMouseX = LOWORD(lParam);  // horizontal position of cursor 
		mwMouseY = HIWORD(lParam);  // vertical position of cursor 
		break;
	case 0x020A:		// WM_MOUSEWHEEL - for some reason i cant get it to include..
		{
		int		nDelta;
		short	wShortParam = HIWORD(wParam);
		float	fOffset;

			nDelta = (int)( wShortParam );
			fOffset = (float)( nDelta ) / 120.0f; 
			if ( mfnZoomControlHandler )
			{
				mfnZoomControlHandler( fOffset );
			}
		}
		break;

	case WM_LBUTTONDOWN:
		mwMouseX = LOWORD(lParam);
		mwMouseY = HIWORD(lParam);
		MainOnPress( mwMouseX, mwMouseY );
		break;
	case WM_RBUTTONUP:
		{
		uint32	ulExtraInfo = GetMessageExtraInfo();
			mwMouseX = LOWORD(lParam);
			mwMouseY = HIWORD(lParam);

			MainOnReleaseRightButton( mwMouseX, mwMouseY );
		}
		break;
	case WM_LBUTTONUP:
		{
		uint32	ulExtraInfo = GetMessageExtraInfo();

			mwMouseX = LOWORD(lParam);
			mwMouseY = HIWORD(lParam);

			// If is touchscreen release
			if ( (ulExtraInfo & 0xFFFFFF00) == 0xFF515700 )
			{
				mnPendingTouchRelease = 2;
			}
			else
			{
				MainOnRelease( mwMouseX, mwMouseY );
			}
		}
		break;
	case WM_KEYUP:
		if ( mbThisWindowIsFocused )
		{
			PlatformKeyboardHandleSpecialKeyUp( SysGetKeyCodeForVirtKey( wParam ) );
		}
		break;
	case WM_KEYDOWN:
		//Handle a key press event
		switch (wParam)
		{
		case VK_ESCAPE:
//			PostQuitMessage( 0 );
			return( DefWindowProc( hWnd, msg, wParam, lParam ) );
			break;
		default:
			if ( mbThisWindowIsFocused )
			{
				PlatformKeyboardHandleSpecialKeyDown( SysGetKeyCodeForVirtKey( wParam ) );
			}
			break;
		}
		break;
	case WM_CHAR:
		/* If not an extended key **/
		if ( !(lParam & (1<<24)) )
		{
			if ( mbThisWindowIsFocused )
			{
				PlatformKeyboardHandleChar( (short)( wParam ) );
			}
		}
		break;
	case WM_EXITSIZEMOVE:
		if ( mbPlatformWin32IsResizingWindow )
		{
		int			nCurrWidth = InterfaceGetWindowWidth();
		int			nCurrHeight = InterfaceGetWindowHeight();
		
			if ( ( mnPlatformWin32ResizeW != nCurrWidth ) ||
				 ( mnPlatformWin32ResizeH != nCurrHeight ) )
			{
				 InterfaceSetWindowSize( FALSE, mnPlatformWin32ResizeW,mnPlatformWin32ResizeH, TRUE );
			}
//			SaveConfig();			
			mbPlatformWin32IsResizingWindow = false;
		}
		break;

	case WM_SIZE:
		switch ( wParam )
		{
		case SIZE_MAXIMIZED:
			InterfaceSetWindowSize( TRUE, 0, 0, FALSE );
//			SaveConfig();			
			break;
		case SIZE_RESTORED:
			if ( ( InterfaceIsFullscreen() == FALSE ) /*&&
				 ( InterfaceDoesNeedChanging() == FALSE )*/ )
			{			
				mbPlatformWin32IsResizingWindow = true;
				mnPlatformWin32ResizeW = LOWORD(lParam);
				mnPlatformWin32ResizeH = HIWORD(lParam);
			}
			break;
		case SIZE_MINIMIZED:
//			mboMinimised = TRUE;
			break;
		}
		break;
	case WM_DESTROY:
		// Handle close & quit
		PostQuitMessage( 0 );
		return 0;
		break;
	default:
		break;
	}

    return( DefWindowProc( hWnd, msg, wParam, lParam ) );
}

void		PlatformSetWindow( int handle )
{
	ghPlatformWin32wndMain = (HWND)handle;
	mhPlatformWin32HandCursor = LoadCursor(NULL,MAKEINTRESOURCE(32649));
	mhPlatformWin32ArrowCursor = LoadCursor(NULL,IDC_ARROW);
	mbHandCursorSet = FALSE;
}


//-------------------------------------------------------------------------
// Function    : InitialiseWindow
// Description : Creates a window using the InterfaceInitWindow function
//-------------------------------------------------------------------------
void		InitialiseWindow( BOOL bAllowResize )
{ 
const char*		szWindowTitle = mszWindowName;

WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WindowsMsgProc, 0L, 0L,
                  GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
                  "GamesForMay Window", NULL };

	// Create window
	InterfaceInitWindow( szWindowTitle, (void*)&wc, bAllowResize );
	ghPlatformWin32wndMain = InterfaceGetWindow();

	mhPlatformWin32HandCursor = LoadCursor(NULL,MAKEINTRESOURCE(32649));
	mhPlatformWin32ArrowCursor = LoadCursor(NULL,IDC_ARROW);
	SetCursor( mhPlatformWin32ArrowCursor );
	mbHandCursorSet = FALSE;

}
//-----------------------------------------------------------------------------------------------------------------------

#ifdef RECORD_DELTAS
char*		mpcDeltaBuffer = NULL;
char*		mpcDeltaBufferRunner = NULL;

#define		DELTABUFFERSIZE		100000

void	DeltaBufferFlush( void )
{
	if ( mpcDeltaBuffer != NULL )
	{
	FILE*		pFile = fopen( "DeltaLog.txt", "ab" );
		fwrite( mpcDeltaBuffer, (int)( mpcDeltaBufferRunner - mpcDeltaBuffer ), 1, pFile );
		fclose( pFile );
		mpcDeltaBufferRunner = mpcDeltaBuffer;
	}
}

void	RecordMsgCode( int nCode )
{
char	acString[256];
int		nLen;

	if ( mpcDeltaBuffer == NULL )
	{
		mpcDeltaBuffer = (char*)malloc( DELTABUFFERSIZE );
		mpcDeltaBufferRunner = mpcDeltaBuffer;
	}
	
	sprintf( acString, "Msg: %d", nCode );
	nLen = strlen( acString );
	memcpy( mpcDeltaBufferRunner, acString, nLen );
	mpcDeltaBufferRunner += nLen;
	memcpy( mpcDeltaBufferRunner, "\r\n", 2 );
	mpcDeltaBufferRunner += 2;
	if ( mpcDeltaBufferRunner > mpcDeltaBuffer + (DELTABUFFERSIZE-400) )
	{
		DeltaBufferFlush();
	}

}

void	RecordDelta( float fDelta, uint32 ulThisTick )
{
char	acString[256];
int		nLen;

	if ( mpcDeltaBuffer == NULL )
	{
		mpcDeltaBuffer = (char*)malloc( DELTABUFFERSIZE );
		mpcDeltaBufferRunner = mpcDeltaBuffer;
	}
	
	sprintf( acString, "Delta %.4f. Tick: %ld", fDelta, ulThisTick );
	nLen = strlen( acString );
	memcpy( mpcDeltaBufferRunner, acString, nLen );
	mpcDeltaBufferRunner += nLen;
	memcpy( mpcDeltaBufferRunner, "\r\n", 2 );
	mpcDeltaBufferRunner += 2;
	if ( mpcDeltaBufferRunner > mpcDeltaBuffer + (DELTABUFFERSIZE-400) )
	{
		DeltaBufferFlush();
	}
}
#endif

float			PlatformGetFrameDelta( void )
{
	return( mfFrameDelta );
}

BOOL		PlatformUpdateFrame( void )
{
u64	ullCurrentTick = SysGetMicrosecondTick();
float	fDelta = ( (float)(ullCurrentTick - mullPlatformLastTick ) ) * 0.000001f;

	if ( fDelta > 0.001f )
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
			mfKeyboardCursorFlashTimer = 0.0f;
			mbKeyboardCursorFlashOn = !mbKeyboardCursorFlashOn;
		}

		if ( mbActivateMouseOverCursor )
		{
			if ( mbHandCursorSet == FALSE )
			{
				SetCursor( mhPlatformWin32HandCursor );
				mbHandCursorSet = TRUE;
			}
		}
		else
		{
			if ( mbHandCursorSet == TRUE )
			{
				SetCursor( mhPlatformWin32ArrowCursor );
				mbHandCursorSet = FALSE;
			}
		}
		mbActivateMouseOverCursor = FALSE;

		if ( mnPendingTouchRelease > 0 )
		{
			mnPendingTouchRelease--;
			if ( mnPendingTouchRelease == 0 )
			{
				MainOnRelease( mwMouseX, mwMouseY );
			}
		}
		return( TRUE );
	}
	return( FALSE );
}



//-------------------------------------------------------------------------
// Function    : PlatformInit
// Description : Creates a window, initialises directX, then initialises the 2d interface system
//-------------------------------------------------------------------------
void	PlatformInit( const char* szWindowName,  BOOL bAllowResize, BOOL bAllowSleep )
{
	if ( szWindowName )
	{
		strcpy( mszWindowName, szWindowName );

		// Create the window for viewing things in
		InitialiseWindow(bAllowResize);
	}
	mullPlatformLastTick = SysGetMicrosecondTick();
}


void				PlatformResizeWindow( BOOL bFullScreen, int nWindowWidth, int nWindowHeight )
{
DWORD	dwWindowStyle = WS_CAPTION|WS_SYSMENU|WS_THICKFRAME|WS_MINIMIZEBOX|WS_MAXIMIZEBOX|WS_VISIBLE;
RECT		xWindowRect;

	xWindowRect.left = 0;
	xWindowRect.right = nWindowWidth;
	xWindowRect.bottom =nWindowHeight;
	xWindowRect.top = 0;
	AdjustWindowRect( &xWindowRect, dwWindowStyle, FALSE );
	SetWindowPos( ghPlatformWin32wndMain, NULL, 0, 0, xWindowRect.right - xWindowRect.left, xWindowRect.bottom - xWindowRect.top, SWP_NOZORDER | SWP_NOMOVE );	
}

void				PlatformMoveWindow( int nLeft, int nTop )
{
	SetWindowPos( ghPlatformWin32wndMain, NULL, nLeft, nTop, 0, 0, SWP_NOZORDER | SWP_NOSIZE );	
}

HINSTANCE	mhInstance = NULL;
const char*		mszCmdLine = NULL;

void*			PlatformGetInstanceHandle( void )
{
	return( (void*)mhInstance );
}

const char*		PlatformGetStartupCommandLine( void )
{
	return( mszCmdLine );
}


void				PlatformRequestApplicationShutdown( void )
{
	PostQuitMessage( 0 );
}

//-------------------------------------------------------------------------
// Function    : WinMain
// Description : The application's entry point
//-------------------------------------------------------------------------
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR lpCmdLine, INT )
{
	mhInstance = hInst;
	mszCmdLine = (const char*)( lpCmdLine );
#ifdef _DEBUG
	DetectMemoryLeaks();
#endif

	MainInitialise();

	MSG msg;
	ZeroMemory( &msg, sizeof(msg) );
		
	// Do the main loop
	while( msg.message != WM_QUIT )
	{
		if ( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
		{
#ifdef RECORD_DELTAS
			RecordMsgCode( (int)msg.message );
#endif
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
		else
		{	
			if ( PlatformUpdateFrame() == TRUE )
			{
				MainUpdate();
			}
			else
			{
				SysSleep(1);
			}
		}
	}

#ifdef RECORD_DELTAS
	DeltaBufferFlush();
#endif

	MainShutdown();

	return 0;
}


 

