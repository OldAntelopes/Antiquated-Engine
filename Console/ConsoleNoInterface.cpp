
#include <stdio.h>
#include "StandardDef.h"
#include "Console.h"


RegisteredConsoleCommandList*	mspConsoleCommandList = NULL;

void	RegisteredConsoleCommandList::Shutdown( void )
{
RegisteredConsoleCommandList*		pRegisteredCommands = mspConsoleCommandList;
RegisteredConsoleCommandList*		pNext;

	while( pRegisteredCommands )
	{
		pNext = pRegisteredCommands->mpNext;
		SAFE_FREE( pRegisteredCommands->mszCommandName );
		delete pRegisteredCommands;

		pRegisteredCommands = pNext;
	}

}

BOOL	RegisteredConsoleCommandList::Register( const char* szCommandName, ConsoleCommandHandler fnFunction )
{
RegisteredConsoleCommandList*		pNewCommand = new RegisteredConsoleCommandList;

	pNewCommand->mszCommandName = (char*)( malloc( strlen( szCommandName ) + 1 ) );
	strcpy( pNewCommand->mszCommandName, szCommandName );
	pNewCommand->mfnCommandFunction = fnFunction;

	pNewCommand->mpNext = mspConsoleCommandList;
	mspConsoleCommandList = pNewCommand;

	return( TRUE );
}



void		ConsoleToggleActive( void )
{

}

BOOL		ConsoleIsAvailable( void )
{
	return( TRUE );
}

BOOL		ConsoleIsActive( void )
{
	return( FALSE );
}

void		ConsoleUpdate( float fDelta )
{

}

void		ConsoleRender( void )
{

}

void		ConsoleOnPressEnter( char* szKeyboardInputString )
{

}

void		ConsolePrint( int mode, const char* szText )
{
	printf( szText );
	printf( "\n" );
}
