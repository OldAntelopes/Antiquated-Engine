//---------------------------------------------------------------
// SysUnixConsole.c
//
//  This file is the Unix console implementations of the functions defined in System.h
//---------------------------------------------------------------
#include <stdio.h>

#include <StandardDef.h>
#include "System.h"
#include <time.h>

#ifdef SERVER
#include "../GameCode/Server/Server.h"
#endif

//-----------------------------------------------------------------

BOOL	SysCheckKeyState( int nKey )
{
	return( FALSE );
}

//-------------------------------------------------------
// Function : WidgetSetEnabled				Unix Implementation
//   Set a control enabled or disabled
//--------------------------------------------------------
void WidgetSetEnabled( void* pTargetTopParent, int nTarget, BOOL boNewState )
{
}

//-------------------------------------------------------
// Function : MenuEntrySetEnabled				UnixConsole Implementation
//   Set a menu item enabled or disabled
//--------------------------------------------------------
void MenuEntrySetEnabled( void* pTargetTopParent, int nTarget, BOOL boNewState )
{
}

//-------------------------------------------------------
// Function : SysMessageBox				UnixConsole
//   Given an error msg, dialog box title and flags, this function 
//   should create a popup messagebox window.
//   Returns FALSE if user pressed "NO" or "Cancel", TRUE otherwise.
//--------------------------------------------------------
BOOL	SysMessageBox( char* acError, char* acTitle, int nFlags )
{
	printf( "%s\n", acTitle );
	printf( "%s\n", acError );
	return( TRUE );
}

//-------------------------------------------------------
// Function : SysSetWindowTitle				UnixConsole Implementation
//   This function change the title of a window
//--------------------------------------------------------
void SysSetWindowTitle( void* pxWindow, char* pcNewTitle )
{
}

//-------------------------------------------------------
// Function : SysEmptyList				UnixConsole Implementation
//   This function make a list control empty
//   pxParentWindow is the parent window, pListIdent the list control number (int)
//--------------------------------------------------------
void SysEmptyList( void* pxParentWindow, void* pListIdent )
{
}

//-------------------------------------------------------
// Function : SysAddFilledListLine				UnixConsole Implementation
//--------------------------------------------------------
void SysAddFilledListLine( void* pxParentWindow, void* pListIdent, int nValsQtt, char** ppcValues, int nParam, int nParam2 )
{
}

int		SysRenameFile( char* szSrc, char* szDest )
{
	// TODO
	rename( szSrc, szDest);
	return( 1 );
}
int		SysCopyFile( char* szSrc, char* szDest, BOOL bFailIfExists )
{
	// TODO
	return( 1 );
}

//-------------------------------------------------------
// Function : SysGetSaveFilenameDialog				UnixConsole Implementation
//   This function is used to allow the user to select a filename to save to from the folders on their puter.
//   szOutputFile where the filename will be stocked is expected to be an allocated 256 char string.
//   Returns True if all ok, False if error or user pressed cancel button.
//--------------------------------------------------------
BOOL SysGetSaveFilenameDialog( char* szFileFilter, char* szTitle, char* szDefaultFolder, int nFlags, char* szOutputFile )
{
	return( FALSE );
}


//-------------------------------------------------------
// Function : SysGetOpenFilenameDialog				UnixConsole Implementation
//   This function is used to allow the user to select a file from the folders on their puter
//--------------------------------------------------------
BOOL SysGetOpenFilenameDialog( char* szFileFilter, char* szTitle, char* szDefaultFolder, int nFlags, char* szOutputFile )
{
	return(  FALSE );
}


//--------------------------------------------------------
// Some bits that the game references but dont do anything on the console build....
// (More properly, this stuff should be tidied so that the references to these
// windows-only functions are all kept inside LibWinServer, but for now just providing
// empty implementations here is sufficient)


void ModelSelectorSelectListNum( int nParam ) {}
void ModelSelectorSetFileCommand( short w1, int n1, int n2, char* pcName ) {}
void WindowSetConnectionStatusString( int nStatus ) {}
void WindowServerUpdateMessageScroll( void ) {}
void WindowsServerUpdateChatPlayerList( void ) {}
void WindowSetIslandStatusString( char* pcStatus ) {}
void GatewayPageRefreshServerList( void ) {}
void IndependentListUpdate( void ){}
void GatewaysRefreshList( void ) {}
void SearchPageAddResponse( void ) {}
BOOL IndependentListIsBlocked( int nServerID ) { return( FALSE ); }
void ModelSelectorRefreshDirList( void ) {}
void ServerChangeIcon( void ) {}
void WindowServerShowLoginPopup( void ) {}
void EconomyUpdateTreasuryMessage( void ) {}
void SettingsEditorRefreshFields( void ) {}
void FirstRunPagesRefreshList( BOOL ) {}
void BuildingSelectionInitialise( void ) {}
void BuildingSelectInitialiseDialogValues( void ) {}

