//-----------------------------------------------------------------------------------------------------------------------
// main.cpp
//
//  HelloWorld sample for Interface library
//
//-----------------------------------------------------------------------------------------------------------------------

#include <StdWinInclude.h>		// If we need to know about windows stuff like HWNDs, HINSTANCEs etc, then we need to include this first
#include <StandardDef.h>		// This specifies a minimal set of 'standard' defines like BOOL
#include <Interface.h>			// The 2d graphics interface library

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
// Description : Creates a window using the InterfaceInitWindow function
//-------------------------------------------------------------------------
void		InitialiseWindow( void )
{
#ifdef USING_OPENGL
const char*		szWindowTitle = "Universal - Interface Library - HelloWorld sample [OpenGL]";
#else
const char*		szWindowTitle = "Universal - Interface Library - HelloWorld sample [DirectX9]";
#endif

WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WindowsMsgProc, 0L, 0L,
                  GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
                  "UniversalHelloWorldSample", NULL };

	// Create window
	InterfaceInitWindow( szWindowTitle, (void*)&wc, TRUE );
	ghwndMain = InterfaceGetWindow();

}
//-----------------------------------------------------------------------------------------------------------------------




//-------------------------------------------------------------------------
// Function    : MainInitialise
// Description : Creates a window, initialises directX, then initialises the 2d interface system
//-------------------------------------------------------------------------
void	MainInitialise( void )
{
	// Create the window for viewing things in
	InitialiseWindow();

	// Initialise D3D
	InterfaceInitDisplayDevice( 0 );

	// Initialise the interface 
	InterfaceInit( TRUE );

}



//-------------------------------------------------------------------------
// Function    : MainUpdate
// Description : Called each frame to do all we need to do and draw all we need to draw
//-------------------------------------------------------------------------
void	MainUpdate( void )
{

	// New frame - clear the back buffer
	InterfaceNewFrame( 0 );

	// Signal start of rendering pass
	InterfaceBeginRender();

	// Draw stuff here
	InterfaceText( 0, 10, 10, "Hello World", 0xFFFFFFFF, 1 );

	// Flush the Interface renderer
	InterfaceDraw();
			
	// Signal end of this pass
	InterfaceEndRender();

	// Present the results to the user
	InterfacePresent();

}


//-------------------------------------------------------------------------
// Function    : MainShutdown
// Description : Shutdown the application
//-------------------------------------------------------------------------
void	MainShutdown( void )
{
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
			MainUpdate();
		}
	}

	MainShutdown();

	return 0;
}


 

