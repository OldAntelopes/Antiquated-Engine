
//-----------------------------------------------------------------------------------------------------------------------
// main.cpp
//
// TODO - Here we'll create a very basic standalone milkdrop player
//-----------------------------------------------------------------------------------------------------------------------

#include "StdWinInclude.h"
#include "StandardDef.h"		// This specifies a minimal set of 'standard' defines like BOOL
#include "Interface.h"			// The 2d graphics interface library
#include "Engine.h"				// The 3d graphics engine library


#include "MilkPlayer.h"		// Sample milkdrop player

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

WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WindowsMsgProc, 0L, 0L,
                  GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
                  "UniversalHelloWorldSample", NULL };

	// Create window
#ifdef USING_OPENGL
	InterfaceInitWindow( "Basic Milkdrop Player sample [OpenGL]", (void*)&wc, TRUE );
#else
	InterfaceInitWindow( "Basic Milkdrop Player sample [DirectX9]", (void*)&wc, TRUE );
#endif
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

	// Now init the engine library
	EngineInitFromInterface();

	MilkPlayerSample::GetSingleton().Initialise();

}



//-------------------------------------------------------------------------
// Function    : MainUpdate
// Params	   : The time elapsed since the last update (in milliseconds)
// Description : 
//-------------------------------------------------------------------------
void	MainUpdate( uint32 ulUpdateTime )
{

	EngineUpdate( TRUE );
	MilkPlayerSample::GetSingleton().Update( ((float)ulUpdateTime) * 0.001f);

	// New frame - clear the back buffer
	InterfaceNewFrame( 0x80808080 );

	// Signal start of rendering pass
	InterfaceBeginRender();

	// Draw stuff here
	MilkPlayerSample::GetSingleton().Render();

	InterfaceText( 0, 10, 10, "Basic Milkdrop Player Sample", 0xFFFFFFFF, 1 );

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
	uint32	ulTimer = SysGetTick();
	uint32	ulLastTimer = SysGetTick();
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
				MainUpdate( ulTimer - ulLastTimer );
				ulLastTimer = ulTimer;
			}
		}
	}

	MainShutdown();

	return 0;
}


 

