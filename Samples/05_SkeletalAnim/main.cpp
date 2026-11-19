//-----------------------------------------------------------------------------------------------------------------------
// main.cpp
//
//  SkeletalAnim sample - experimental skeletal animation for .atm models
//
//  Command line options (working directory must be Exe\) :
//		-dump					Write model info + mesh to Data\Samples\SkelOut\ and save some reference screenshots
//		(more modes are added in Scene.cpp)
//-----------------------------------------------------------------------------------------------------------------------

#include <StdWinInclude.h>
#include <d3dx9.h>
#include <StandardDef.h>
#include <Interface.h>
#include <Engine.h>
#include <Rendering.h>

#include "FrameCapture.h"
#include "Scene.h"

HWND	ghwndMain;

//-------------------------------------------------------------------------
LRESULT WINAPI WindowsMsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
	case WM_KEYDOWN:
		switch ( wParam )
		{
		case VK_ESCAPE:
			PostQuitMessage( 0 );
			break;
		default:
			SceneOnKey( (int)wParam );
			break;
		}
		break;
	case WM_DESTROY:
		PostQuitMessage( 0 );
		return( 0 );
	default:
		break;
	}
	return( DefWindowProc( hWnd, msg, wParam, lParam ) );
}

//-------------------------------------------------------------------------
void		InitialiseWindow( void )
{
WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WindowsMsgProc, 0L, 0L,
                  GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
                  "UniversalSkeletalAnimSample", NULL };

	InterfaceSetWindowSize( FALSE, 1280, 720, FALSE );
	InterfaceInitWindow( "Universal - Engine Library - SkeletalAnim sample [DirectX9]", (void*)&wc, TRUE );
	ghwndMain = InterfaceGetWindow();
}

//-------------------------------------------------------------------------
void	MainInitialise( const char* szCommandLine )
{
	InitialiseWindow();
	InterfaceInitDisplayDevice( 0 );
	InterfaceInit( TRUE );
	EngineInitFromInterface();

	SceneInitialise( szCommandLine );
}

//-------------------------------------------------------------------------
// Function    : MainRenderFrame
// Description : Renders one frame. If bPresent is FALSE the back buffer is left for capture
//-------------------------------------------------------------------------
void	MainRenderFrame( uint32 ulUpdateTime )
{
int		nWidth = InterfaceGetWidth();
int		nHeight = InterfaceGetHeight();

	EngineUpdate( TRUE );
	ModelRenderingUpdate( ulUpdateTime );

	InterfaceNewFrame( 0xFF303438 );

	InterfaceBeginRender();
	InterfaceShadedRect( 0, 0, 0, nWidth, nHeight, 0xFF485058, 0xFF485058, 0xFF202428, 0xFF202428 );
	InterfaceDraw();
	InterfaceEndRender();

	InterfaceBeginRender();
	SceneRender();
	SceneDrawOverlay();
	InterfaceDraw();
	InterfaceEndRender();
}

//-------------------------------------------------------------------------
void	MainShutdown( void )
{
	SceneFree();
	FrameCaptureVideoEnd();
	InterfaceFree();
}

//-------------------------------------------------------------------------
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR lpCmdLine, INT )
{
MSG		msg;
uint32	ulLastTick;
uint32	ulTick;

	MainInitialise( lpCmdLine );

	ZeroMemory( &msg, sizeof(msg) );
	ulLastTick = SysGetTick();

	while( msg.message != WM_QUIT )
	{
		if ( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
		else
		{
			ulTick = SysGetTick();
			if ( SceneUpdate( ulTick - ulLastTick ) == FALSE )
			{
				PostQuitMessage( 0 );
			}
			ulLastTick = ulTick;
		}
	}

	MainShutdown();
	return( 0 );
}
