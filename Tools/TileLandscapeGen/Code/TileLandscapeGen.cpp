//-----------------------------------------------------------------------------------------------------------------------
// TileLandscapeGen.cpp
//
//-----------------------------------------------------------------------------------------------------------------------

#include <stdio.h>
#include "StdWinInclude.h"
#include "StandardDef.h"	
#include "Interface.h"		
#include "Engine.h"

#include "Landscape/SourceTextures.h"
#include "UI\UI.h"
#include "Platform/Platform.h"
#include "Generator.h"

#include "TileLandscapeGen.h"
#include "MenuInterface.h"

ulong	mulLastProcessRenderTick = 0;



void	MainOnPress( int X, int Y )
{
	if ( UIOnPress( X, Y ) == FALSE )
	{

	}

}

void	MainOnRelease( int X, int Y )
{
	if ( UIOnRelease( X, Y ) == FALSE )
	{

	}
}



BOOL	MainInitialise( void )
{
	InterfaceSetInitialSize( FALSE, 700, 400, FALSE );

	PlatformInit();

	// Initialise Renderer
	InterfaceInitDisplayDevice( 0 );

	// Initialise the interface 
	InterfaceInit( TRUE );
	EngineInit();

	if ( SourceTexturesLoad() == FALSE )
	{
		return( FALSE );
	}

	MenuInterfaceInit();

	return( TRUE );

}

void	MainShutdown( void )
{
	GeneratorShutdown();
	MenuInterfaceShutdown();
	SourceTexturesFree();
}

void	MainUpdateFrame( void )
{
	EngineUpdate( TRUE );
	UIUpdate( 0.1f );
	MenuInterfaceUpdate( 0.1f );
	GeneratorUpdate();

	MenuInterfaceRender();

}


BOOL	MainProcessUpdate( void )
{
MSG msg;
ZeroMemory( &msg, sizeof(msg) );

	while ( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
	{
		TranslateMessage( &msg );
		DispatchMessage( &msg );

		if ( msg.message == WM_QUIT )
		{
			PlatformExit();
			return( FALSE );
		}
	}

	if ( SysGetTick() - mulLastProcessRenderTick > 1000 )
	{
		mulLastProcessRenderTick = SysGetTick();
		//MenuInterfaceRender();
	}
	return( TRUE );
}



 

