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

#include "../TreeSpriteGen.h"
#include "Platform.h"

//-----------------------------------------------------------------------------------------------------------------------
// ------------------------------------------- Windows specific stuff

HWND	ghwndMain;		// Global window handle
short	mwMouseX;
short	mwMouseY;

FILE*	mspSaveFile = NULL;
FILE*	mspLoadFile = NULL;

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
	}
	else
	{
		mspLoadFile = fopen( acFilename, "rb" );
		if ( mspLoadFile == NULL )
		{
			return( -1 );
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


void	PlatformGetCurrentHoldPosition( int* pnX, int* pnY )
{
	*pnX = mwMouseX;
	*pnY = mwMouseY;
}

//-------------------------------------------------------------------------
// Function    : WindowsMsgProc
// Description : 
//-------------------------------------------------------------------------
LRESULT WINAPI WindowsMsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch( msg )
    {
 	case WM_MOUSEMOVE:
		mwMouseX = LOWORD(lParam);  // horizontal position of cursor 
		mwMouseY = HIWORD(lParam);  // vertical position of cursor 
		break;

	case WM_LBUTTONDOWN:
		MainOnPress( mwMouseX, mwMouseY );
		break;
	case WM_LBUTTONUP:
		MainOnRelease( mwMouseX, mwMouseY );
		break;
	case WM_KEYDOWN:
		//Handle a key press event
		switch (wParam)
		{
		case VK_ESCAPE:
			PostQuitMessage( 0 );
			return( DefWindowProc( hWnd, msg, wParam, lParam ) );
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


//-------------------------------------------------------------------------
// Function    : InitialiseWindow
// Description : Creates a window using the InterfaceInitWindow function
//-------------------------------------------------------------------------
void		InitialiseWindow( void )
{
const char*		szWindowTitle = "Tree Sprite Generator v0.1";

WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WindowsMsgProc, 0L, 0L,
                  GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
                  "TreeSpriteGen", NULL };

	// Create window
	InterfaceInitWindow( szWindowTitle, (void*)&wc );
	ghwndMain = InterfaceGetWindow();

}
//-----------------------------------------------------------------------------------------------------------------------


void		PlatformUpdateFrame( void )
{


}



//-------------------------------------------------------------------------
// Function    : PlatformInit
// Description : Creates a window, initialises directX, then initialises the 2d interface system
//-------------------------------------------------------------------------
void	PlatformInit( void )
{
	// Create the window for viewing things in
	InitialiseWindow();
}




//-------------------------------------------------------------------------
// Function    : WinMain
// Description : The application's entry point
//-------------------------------------------------------------------------
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR lpCmdLine, INT )
{
//	ghInstance = hInst;

	MainInitialise();

	MSG msg;
	ZeroMemory( &msg, sizeof(msg) );
		
	// Do the main loop
	while( msg.message != WM_QUIT )
	{
		if ( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
		else
		{	
			PlatformUpdateFrame();
			MainUpdate();
		}
	}

	MainShutdown();

	return 0;
}


 

