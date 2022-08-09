//-----------------------------------------------------------------------------------------------------------------------
// main.cpp
//
//  BasicEngine sample for Engine library
//
// Note for this sample we need to expand the libraries included in the project. 
//  This sample adds..
//
//      Engine.lib   - TheUniversal graphics engine library, containing function for rendering models etc
//
//		d3dx8.lib    - For various DirectX helper stuff that the Engine.lib uses
//      CodeUtil.lib - Access to some standard game utilities and system stuff such as printing text and creating files/directories
//	    zlibstat.lib - Version of zlib compression code which the Engine.lib uses to compress and decompress models
//    {Debug only}
//		dxerr8.lib   - Extra error reporting if a model fails to load etc
//
//-----------------------------------------------------------------------------------------------------------------------

#include <StdWinInclude.h>		// If we need to know about windows stuff like HWNDs, HINSTANCEs etc, then we need to include this first
#include <StandardDef.h>		// This specifies a minimal set of 'standard' defines like BOOL
#include <Interface.h>			// The 2d graphics interface library
#include <Engine.h>				// The 3d graphics engine library
#include <Rendering.h>				// The 3d graphics engine library

#include "OSD.h"
#include "Scene.h"

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
	InterfaceInitWindow( "Universal - Engine Library - BasicEngine sample [OpenGL]", (void*)&wc, TRUE );
#else
	InterfaceInitWindow( "Universal - Engine Library - BasicEngine sample [DirectX9]", (void*)&wc, TRUE );
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

	// Initialise the scene that holds our example
	SceneInitialise();

}


//-------------------------------------------------------------------------
// Function    : MainDrawBackground
// Description : 
//-------------------------------------------------------------------------
void	MainDrawBackground( void )
{
int		width = InterfaceGetWidth();		// Get the width and height of the interface
int		height = InterfaceGetHeight();

	InterfaceBeginRender();

	// Lets draw a shaded box over the entire background to give it a bit of colour
	InterfaceShadedRect( 0, 0, 0, width, height, 0xFF101010, 0xFF101010, 0xFF303030, 0xFF303030 );  // dark red at the top, a slightly less dark red at the bottom

	// Flush the Interface renderer
	InterfaceDraw();
			
	// Signal end of this pass
	InterfaceEndRender();
}


//-------------------------------------------------------------------------
// Function    : MainUpdate
// Params	   : The time elapsed since the last update (in milliseconds)
// Description : 
//-------------------------------------------------------------------------
void	MainUpdate( ulong ulUpdateTime )
{

	EngineUpdate( TRUE );
	ModelRenderingUpdate(ulUpdateTime);

	// New frame - clear the back buffer
	InterfaceNewFrame( 0x80808080 );

	MainDrawBackground();

	// Signal start of rendering pass
	InterfaceBeginRender();

	SceneUpdate();

	// Draw stuff here
	InterfaceText( 0, 10, 10, "Basic Engine Sample", 0xFFFFFFFF, 1 );

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
	SceneFree();
	
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
				MainUpdate( ulTimer - ulLastTimer );
				ulLastTimer = ulTimer;
			}
		}
	}

	MainShutdown();

	return 0;
}


 

