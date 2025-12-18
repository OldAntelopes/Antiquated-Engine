
#include <stdio.h>
//----------------------------------------------------- WIN32 Only
#include <windows.h>			// Win32 Specific

#include <StandardDef.h>
#include <Interface.h>
#include <Engine.h>

#include "../../Engine/DirectX/OculusDX.h"
#include "../Common/InterfaceDevice.h"
#include "../Common/InterfaceCommon.h"
#include "../Common/InterfaceInstance.h"
#include "../Common/InterfaceDevice.h"
#include "Interface-Win32.h"

// Some random, probably unused, defines
#define		INITIAL_SCREEN_LEFT			30
#define		INITIAL_SCREEN_RIGHT		780
#define		INITIAL_SCREEN_TOP			100
#define		INITIAL_SCREEN_BOTTOM		600

BOOL		mboHasWindowChanged = FALSE;
BOOL		mboFullScreen = FALSE;

HWND		mhwndInterfaceMain = NULL;

WNDCLASSEX* mpxWindClass;
DWORD		mdwWindowStyle;

int		mnWindowLeft = 50;
int		mnWindowTop = 50;

BOOL	mboMessageBoxActive = FALSE;
BOOL					mboDontShowAnyMoreWarnings = FALSE;

INTERFACE_API BOOL	InterfaceWaitingForMessageBox( void )
{
	return( mboMessageBoxActive );
}


void	PanicImplShowDialog( const char* szErrorString )
{
char	acString[512];
int		nRet;

	sprintf( acString, "**** Error Message ****\n%s\n", szErrorString);
	nRet = MessageBox( mhwndInterfaceMain, acString, "Error message", MB_ABORTRETRYIGNORE | MB_ICONEXCLAMATION);
	if ( nRet == IDIGNORE )
	{
		mboDontShowAnyMoreWarnings = TRUE;
	}
	if ( nRet == IDABORT )
	{
		throw( szErrorString ); 
		PostQuitMessage( 0 );
	}
}

/***************************************************************************
 * Function    : PanicImpl
 * Params      : 
 * Returns     :
 * Description : 
 ***************************************************************************/
void PanicImpl( const char* szErrorString )
{
	if ( ( mboDontShowAnyMoreWarnings == FALSE ) &&
		 ( mboMessageBoxActive == FALSE ) )
	{	 
		mboMessageBoxActive = TRUE;

		if ( InterfaceIsFullscreen() == FALSE )
		{
			PanicImplShowDialog( szErrorString );
			mboMessageBoxActive = FALSE;
		}
		else
		{
//			if ( InterfaceShowFullscreenPanic( szErrorString ) == TRUE )
			{
				mboDontShowAnyMoreWarnings = TRUE;
			}
		}
	}

} 


/***************************************************************************
 * Function    : InterfaceDoesNeedChanging
 ***************************************************************************/
INTERFACE_API BOOL InterfaceDoesNeedChanging ( void )
{
//	if ( InterfaceGetD3DDevice() != NULL )
//	{
#ifndef USING_OPENGL
	if ( InterfaceIsVRMode() == TRUE ) 
//		 && ( EngineHasOculus() == TRUE ) )
	{
		return( FALSE );
	}
#endif
	return( mboHasWindowChanged );
//	}
//	return( FALSE );
}

void	InterfaceSetWindowHasChanged( BOOL bFlag )
{
	mboHasWindowChanged = bFlag;
}

INTERFACE_API void InterfaceInitWindow( const char* pcString, void* pVoidWinClass, BOOL bAllowResize )
{
	InterfaceInstanceMain()->InitWindow( pcString, pVoidWinClass, bAllowResize );
}

//---------------------------------------------------------------------------
// Function    : InterfaceInitWindow
//---------------------------------------------------------------------------
void InterfaceInstance::InitWindow( const char* pcString, void* pVoidWinClass, BOOL bAllowResize )
{
HWND hWnd = 0;

#ifndef USING_OPENGL
	if ( InterfaceIsVRMode() == TRUE )
	{
		hWnd = (HWND)OculusInitWindow(  pVoidWinClass );
	}
#endif

	if ( !hWnd )
	{
	WNDCLASSEX* pWinClass	 = (WNDCLASSEX*)( pVoidWinClass );
	RECT		xWindowRect;

		RegisterClassEx( pWinClass );

		mpxWindClass = pWinClass;

		if ( bAllowResize )
		{
			mdwWindowStyle = WS_CAPTION|WS_SYSMENU|WS_THICKFRAME|WS_MINIMIZEBOX|WS_MAXIMIZEBOX|WS_VISIBLE;
		}
		else
		{
			mdwWindowStyle = WS_CAPTION|WS_SYSMENU|WS_THICKFRAME|WS_MINIMIZEBOX|WS_VISIBLE;
		}
		xWindowRect.left = mnWindowLeft;
		xWindowRect.right = mnWindowLeft + mnWindowWidth;
		xWindowRect.bottom = mnWindowTop + mnWindowHeight;
		xWindowRect.top = mnWindowTop;
		AdjustWindowRect( &xWindowRect, mdwWindowStyle, FALSE );
		// Create the application's window
		hWnd = CreateWindow( pWinClass->lpszClassName, pcString,
								  mdwWindowStyle, xWindowRect.left, xWindowRect.top,
												  xWindowRect.right - xWindowRect.left, xWindowRect.bottom - xWindowRect.top,
								  NULL, NULL, pWinClass->hInstance, NULL );
	}

	mhwndInterfaceMain = hWnd;

}


//---------------------------------------------------------------------------
// Function    : InterfaceGetWindow
//---------------------------------------------------------------------------
INTERFACE_API HWND InterfaceGetWindow( void )
{
	return( mhwndInterfaceMain );
}

//---------------------------------------------------------------------------
// Function    : InterfaceSetWindow
//---------------------------------------------------------------------------
INTERFACE_API void InterfaceSetWindow( HWND hwndMain )
{
	mhwndInterfaceMain = hwndMain;
}

/***************************************************************************
 * Function    : InterfaceGetWidth
 ***************************************************************************/
INTERFACE_API int InterfaceGetWindowWidth( void )
{
	return( InterfaceInstanceMain()->GetWindowWidth() );
}

/***************************************************************************
 * Function    : InterfaceGetHeight
 ***************************************************************************/
INTERFACE_API int InterfaceGetWindowHeight( void )
{
	return( InterfaceInstanceMain()->GetWindowHeight() );
}

int		InterfaceInstance::GetWindowWidth()
{
	return( mnWindowWidth );
}

int		InterfaceInstance::GetWindowHeight()
{
	return( mnWindowHeight );
}


/***************************************************************************
 * Function    : InterfaceSetWindowPosition
 * Params      : 
 * Returns     :
 * Description : 
 ***************************************************************************/
INTERFACE_API void InterfaceSetWindowPosition( int nLeft, int nTop )
{
	mnWindowLeft = nLeft;
	mnWindowTop = nTop;
}

void	InterfaceWin32SetInitialWindowSize( int sizeX, int sizeY )
{
	InterfaceInstanceMain()->SetInitialWindowSize( sizeX, sizeY );
}

void	InterfaceInstance::SetInitialWindowSize( int sizeX, int sizeY )
{
	mnWindowLeft = INITIAL_SCREEN_LEFT;
	mnWindowTop = INITIAL_SCREEN_TOP;

	if ( sizeX == 0 )
	{
		mnWindowHeight = INITIAL_SCREEN_BOTTOM - INITIAL_SCREEN_TOP;
		mnWindowWidth = INITIAL_SCREEN_RIGHT - INITIAL_SCREEN_LEFT;
	}
	else
	{
	RECT	xRect;
		xRect.left = 0;
		xRect.right = sizeX;
		xRect.top = 0;
		xRect.bottom = sizeY;
//		AdjustWindowRect( &xRect, WS_CAPTION|WS_SYSMENU|WS_THICKFRAME|WS_MINIMIZEBOX|WS_MAXIMIZEBOX, FALSE );
		mnWindowWidth = (xRect.right - xRect.left) + 1;
		mnWindowHeight = (xRect.bottom - xRect.top) + 1;
	}
}


void	InterfaceSetWindowStyle( HWND hWindow, bool bFullscreen )
{
	if ( bFullscreen ) 
	{
        SetWindowLong( hWindow, GWL_STYLE, WS_POPUP|WS_SYSMENU|WS_VISIBLE );
	}
	else
	{
        SetWindowLong( hWindow, GWL_STYLE, mdwWindowStyle );
	}
}


/***************************************************************************
 * Function    : InterfaceIsFullscreen
 ***************************************************************************/
INTERFACE_API BOOL InterfaceIsFullscreen ( void )
{
//	mboFullScreen = !mcd3dUtilApp.m_bWindowed;
	return( mboFullScreen );
}

/***************************************************************************
 * Function    : InterfaceSetWindowSize
 * Params      : 
 * Returns     :
 * Description : 
 ***************************************************************************/
INTERFACE_API void InterfaceSetWindowSize( BOOL boFullScreen, int nWidth, int nHeight, BOOL boAdjust )
{
	InterfaceInstanceMain()->SetWindowSize( boFullScreen, nWidth, nHeight, boAdjust );
}

void	InterfaceInstance::SetWindowSize( BOOL boFullScreen, int nWidth, int nHeight, BOOL boAdjust )
{
	mboHasWindowChanged = TRUE;
	mboFullScreen = boFullScreen;
	if ( mboFullScreen != TRUE )
	{
		if ( boAdjust == TRUE )
		{
		RECT	xRect;

		    if ( mhwndInterfaceMain != NULL )
			{
				GetClientRect( mhwndInterfaceMain, &xRect );
				mnWindowWidth = xRect.right - xRect.left;
				mnWindowHeight = xRect.bottom - xRect.top;
			}
		}
		else
		{
			mnWindowWidth = nWidth;
			mnWindowHeight = nHeight;
		}
	}

	OnSetWindowSize( boFullScreen, mnWindowWidth, mnWindowHeight );
}
//------------------------------------------------------------------------
