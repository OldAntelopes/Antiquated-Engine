
#include "StandardDef.h"
#include "Interface.h"
#include "Engine.h"

#include "EntitySystem\EntityManager.h"
#include "ParticleSystem\ParticleManager.h"
#include "UI/UI.h"
#include "GameMode.h"

#include "Console\Console.h"
 
GameMode*		mspCurrentGameMode = NULL;


void	Module::InitModule( void )
{
	OnInitModule();
}

void	Module::Update( float fDelta )
{
Module*		pSubModules = mpSubModulesList;

	OnUpdate( fDelta );
	while ( pSubModules )
	{
		pSubModules->Update( fDelta );
		pSubModules = pSubModules->GetNext();
	}
}
	
void	Module::Render( int nRenderPass )
{
Module*		pSubModules = mpSubModulesList;
	
	OnRender( nRenderPass );
	while ( pSubModules )
	{
		pSubModules->Render( nRenderPass );
		pSubModules = pSubModules->GetNext();
	}
	
}

void	Module::Shutdown( void )
{
Module*		pSubModules = mpSubModulesList;
	
	OnShutdown( );
	while ( pSubModules )
	{
		pSubModules->Shutdown( );
		pSubModules = pSubModules->GetNext();
	}
}

Module::~Module()
{
Module*		pSubModules = mpSubModulesList;
Module*		pNextSubModule;

	while ( pSubModules )
	{
		pNextSubModule = pSubModules->GetNext();
		delete pSubModules;
		pSubModules = pNextSubModule;
	}
}

void	Module::ReleaseGraphicsForDeviceReset( void )
{
Module*		pSubModules = mpSubModulesList;
	
	OnReleaseGraphicsForDeviceReset( );
	while ( pSubModules )
	{
		pSubModules->ReleaseGraphicsForDeviceReset( );
		pSubModules = pSubModules->GetNext();
	}

}

void	Module::InitGraphicsPostDeviceReset( void )
{
Module*		pSubModules = mpSubModulesList;
	
	OnInitGraphicsPostDeviceReset( );
	while ( pSubModules )
	{
		pSubModules->InitGraphicsPostDeviceReset( );
		pSubModules = pSubModules->GetNext();
	}

}
void	Module::AddSubModule( Module* pModule )
{
	pModule->SetNext( mpSubModulesList );
	mpSubModulesList = pModule;

	pModule->InitModule();


}


//------------------------------------------------------------------

void	GameMode::AddModule( Module* pModule )
{
	pModule->SetNext( mpModulesList );
	mpModulesList = pModule;

	pModule->InitModule();
}

void	GameMode::InitGameMode( void )
{
	OnInitGameMode();
}


void	GameMode::Update( float fDelta )
{
Module*		pModules = mpModulesList;

	OnUpdate( fDelta );

	while( pModules )
	{
		pModules->Update( fDelta );
		pModules = pModules->GetNext();
	}
}

void	GameMode::ReleaseGraphicsForDeviceReset( void )
{
Module*		pModules = mpModulesList;

	while( pModules )
	{
		pModules->ReleaseGraphicsForDeviceReset();
		pModules = pModules->GetNext();
	}
	OnReleaseGraphicsForDeviceReset();
}

void	GameMode::InitGraphicsPostDeviceReset( void )
{
Module*		pModules = mpModulesList;

	while( pModules )
	{
		pModules->InitGraphicsPostDeviceReset();
		pModules = pModules->GetNext();
	}
	OnInitGraphicsPostDeviceReset();
}

void	GameMode::RenderGameModeModules( int nRenderPass )
{
Module*		pModules = mpModulesList;

	while( pModules )
	{
		pModules->Render( nRenderPass );
		pModules = pModules->GetNext();
	}
}

void	GameMode::Render( void )
{

	OnRender();
}

void	GameMode::Shutdown( void )
{
Module*		pModules = mpModulesList;
Module*		pNext;

	while( pModules )
	{
		pNext = pModules->GetNext();
		pModules->Shutdown();
		delete pModules;
		pModules = pNext;
	}

	OnShutdown();
}


void		GameModeSet( GameMode* pNewGameMode )
{
	if ( mspCurrentGameMode ) 
	{
		mspCurrentGameMode->Shutdown();
	}
	delete mspCurrentGameMode;

	mspCurrentGameMode = pNewGameMode;

	mspCurrentGameMode->InitGameMode();
}


void		GameModeInit( GameMode* pInitialMode )
{
	mspCurrentGameMode = pInitialMode;
	mspCurrentGameMode->InitGameMode();
}

void		GameModeUpdate( float fDelta )
{
	UIUpdate( fDelta );

	mspCurrentGameMode->Update( fDelta );

}

void	GameModeReleaseGraphicsForDeviceReset( void )
{
	
	UIReleaseGraphicsForDeviceReset();

	mspCurrentGameMode->ReleaseGraphicsForDeviceReset();

	EngineFree( FALSE );

}

void	GameModeInitGraphicsPostDeviceReset( void )
{
	UIInitGraphicsPostDeviceReset();

	mspCurrentGameMode->InitGraphicsPostDeviceReset();
}

GameMode*		GameModeGetCurrent( void )
{
	return( mspCurrentGameMode );
}

void		GameModeNewFrame( uint32 ulBackgroundClearCol )
{
	// If we need to change and we currently have a d3d device
	if ( InterfaceDoesNeedChanging() == TRUE ) 
	{
		// Free all graphics
		GameModeReleaseGraphicsForDeviceReset();
	}
	
	switch( InterfaceNewFrame( ulBackgroundClearCol ) )
	{
	case 0:
		break;
	case 1:
		EngineRestart();
		GameModeInitGraphicsPostDeviceReset();
		break;
	default:
		// ERROR!!
		break;
	}
}

void		GameModeRender( uint32 ulBackgroundClearCol )
{
	GameModeNewFrame( ulBackgroundClearCol );

	InterfaceBeginRender();

	mspCurrentGameMode->Render();

	InterfaceDraw();

	if ( ConsoleIsActive() )
	{
		ConsoleRender();
		InterfaceDraw();
	}
	InterfaceEndRender();
	InterfacePresent();

}


void		GameModeShutdown( void )
{
	mspCurrentGameMode->Shutdown();

	delete mspCurrentGameMode;
	mspCurrentGameMode = NULL;
}


/*
// Kill all the stuff that was created from static initialisers
void		GameModeShutdownSystem( void )
{
	ConsoleShutdown();
	EntityManagerShutdown();
	ParticleManagerShutdown();
}
*/

BOOL		GameModeOnPress( int X, int Y )
{
	if ( mspCurrentGameMode )
	{
		return( mspCurrentGameMode->OnPress( X, Y ) );
	}
	return( FALSE );
}

BOOL		GameModeOnRelease( int X, int Y )
{
	if ( mspCurrentGameMode )
	{
		return( mspCurrentGameMode->OnRelease( X, Y ) );
	}
	return( FALSE );


}


