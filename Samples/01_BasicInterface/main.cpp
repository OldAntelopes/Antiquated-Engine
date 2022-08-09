//-----------------------------------------------------------------------------------------------------------------------
// main.cpp
//
//  BasicInterface sample for Interface library
//
//-----------------------------------------------------------------------------------------------------------------------

#include <StdWinInclude.h>		// If we need to know about windows stuff like HWNDs, HINSTANCEs etc, then we need to include this first
#include <StandardDef.h>		// This specifies a minimal set of 'standard' defines like BOOL
#include <Interface.h>			// The 2d graphics interface library

#include "OSD.h"

//-----------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------
// ------------------------------------------- Windows specific stuff

HWND	ghwndMain;		// Global window handle

//-------------------------------------------------------------------------
// Function    : WindowsMsgProc
// Description : 
//-------------------------------------------------------------------------
LRESULT WINAPI WindowsMsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch( msg )
    {
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
// Description : 
//-------------------------------------------------------------------------
void		InitialiseWindow( void )
{
#ifdef USING_OPENGL
const char*		szWindowTitle = "Universal - Interface Library - BasicInterface sample [OpenGL]";
#else
const char*		szWindowTitle = "Universal - Interface Library - BasicInterface sample [DirectX9]";
#endif

WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WindowsMsgProc, 0L, 0L,
                  GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
                  "UniversalHelloWorldSample", NULL };

// Create window
	InterfaceSetWindowSize( FALSE, 800, 600, FALSE );
	InterfaceInitWindow( szWindowTitle, (void*)&wc, TRUE );
	ghwndMain = InterfaceGetWindow();
}
//-----------------------------------------------------------------------------------------------------------------------




//-------------------------------------------------------------------------
// Function    : MainInitialise
// Description : 
//-------------------------------------------------------------------------
void	MainInitialise( void )
{
	// Create the window for viewing things in
	InitialiseWindow();

	// Initialise display device
	InterfaceInitDisplayDevice( 0 );

	// Initialise the interface 
	InterfaceInit( TRUE );

	// Initialise our own OSD stuff (loading images etc)
	OSDInitialise();
}



//-------------------------------------------------------------------------
// Function    : MainUpdate
// Description : 
//-------------------------------------------------------------------------
void	MainUpdate( void )
{

	// New frame - clear the back buffer
	InterfaceNewFrame( 0 );

	// Signal start of rendering pass
	InterfaceBeginRender();

	// Draw stuff here
	OSDRender();

	// Flush the Interface renderer
	InterfaceDraw();
			
	// Signal end of this pass
	InterfaceEndRender();

	// Present the results to the user
	InterfacePresent();

}


//-------------------------------------------------------------------------
// Function    : MainShutdown
// Description : 
//-------------------------------------------------------------------------
void	MainShutdown( void )
{
	OSDFree();

	InterfaceFree();
}



//-------------------------------------------------------------------------
// Function    : WinMain
// Description : The application's entry point
//-------------------------------------------------------------------------
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR lpCmdLine, INT )
{
//	ghInstance = hInst;

	MainInitialise();

	MSG		msg;
	ulong	ulTimer = SysGetTick();
	ulong	ulLastTimer = SysGetTick();
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
			ulTimer = SysGetTick();

			if ( ulTimer != ulLastTimer )
			{
				MainUpdate();
				ulLastTimer = ulTimer;
			}
		}
	}

	MainShutdown();

	return 0;
}


 

