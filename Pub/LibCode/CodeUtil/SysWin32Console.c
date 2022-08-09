//---------------------------------------------------------------
// SysWin32Console.c
//
//  This file is the win32 console implementations of the functions defined in System.h
//---------------------------------------------------------------
#include <stdio.h>
#include <direct.h>	// Needed for "mkdir()"

#include <stdwininclude.h>	// Probably shouldnt use this but need it for "Sleep()"
#include <StandardDef.h>
#include "System.h"
#include <time.h>

#ifdef SERVER
#include "../Gamecode/Server/Server.h"
#endif

//-----------------------------------------------------------------



//#define		MEM_BOUNDS_CHECK

BOOL	SysCheckKeyState( int nKey )
{
	return( FALSE );
}

//-------------------------------------------------------
// Function : WidgetSetEnabled				Windows Implementation
//   Set a control enabled or disabled
//--------------------------------------------------------
void WidgetSetEnabled( void* pTargetTopParent, int nTarget, BOOL boNewState )
{
}

//-------------------------------------------------------
// Function : MenuEntrySetEnabled				Win32Console Implementation
//   Set a menu item enabled or disabled
//--------------------------------------------------------
void MenuEntrySetEnabled( void* pTargetTopParent, int nTarget, BOOL boNewState )
{
}

//-------------------------------------------------------
// Function : SysMessageBox				Win32Console
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
// Function : SysSetWindowTitle				Win32Console Implementation
//   This function change the title of a window
//--------------------------------------------------------
void SysSetWindowTitle( void* pxWindow, char* pcNewTitle )
{
}

//#define		MEM_BOUNDS_CHECK
void		SysGetLocalTime( SYS_LOCALTIME* pTime )
{
SYSTEMTIME	systime;

	GetLocalTime( &systime );
	pTime->wDay = systime.wDay;
	pTime->wHour = systime.wHour;
	pTime->wMinute = systime.wMinute;
	pTime->wMonth = systime.wMonth;
	pTime->wSecond = systime.wSecond;
	pTime->wYear = systime.wYear;
}

//-------------------------------------------------------
// Function : SysEmptyList				Win32Console Implementation
//   This function make a list control empty
//   pxParentWindow is the parent window, pListIdent the list control number (int)
//--------------------------------------------------------
void SysEmptyList( void* pxParentWindow, void* pListIdent )
{
}

void		SysSleep( int millisecs )
{
	Sleep(millisecs);
}

//-------------------------------------------------------
// Function : SysAddFilledListLine				Win32Console Implementation
//--------------------------------------------------------
void SysAddFilledListLine( void* pxParentWindow, void* pListIdent, int nValsQtt, char** ppcValues, int nPos, int nParam )
{
}

int		SysRenameFile( const char* szSrc, const char* szDest )
{
	rename( szSrc, szDest);
	return( 1 );
}
int		SysCopyFile( const char* szSrc, const char* szDest, BOOL bFailIfExists )
{
FILE*	pDestFile;
FILE*	pSrcFile;
byte	bReadBuff[1024];
int		nBytesRead;
int		nBytesLeft;

	if ( bFailIfExists )
	{
		pDestFile = fopen(szDest,"rb");
		if ( pDestFile )
		{
			fclose(pDestFile);
			return( 0 );
		}
	}
	pDestFile = fopen(szDest,"wb");
	pSrcFile = fopen(szSrc,"rb");
	fseek( pSrcFile, 0, SEEK_END );
	nBytesLeft = ftell(pSrcFile);
	rewind( pSrcFile );
	while ( nBytesLeft > 0 )
	{
		if ( nBytesLeft > 1024 )
		{
			nBytesRead = fread( bReadBuff,1,1024,pSrcFile);
		}
		else
		{
			nBytesRead = fread( bReadBuff,1,nBytesLeft,pSrcFile);
		}
		fwrite(bReadBuff,nBytesRead,1,pDestFile);
		nBytesLeft -= nBytesRead;
	}
	fclose(pSrcFile);
	fclose(pDestFile);
	return( 1 );
}

//-------------------------------------------------------
// Function : SysGetSaveFilenameDialog				Win32Console Implementation
//   This function is used to allow the user to select a filename to save to from the folders on their puter.
//   szOutputFile where the filename will be stocked is expected to be an allocated 256 char string.
//   Returns True if all ok, False if error or user pressed cancel button.
//--------------------------------------------------------
BOOL SysGetSaveFilenameDialog( char* szFileFilter, char* szTitle, char* szDefaultFolder, int nFlags, char* szOutputFile )
{
	return( FALSE );
}


//-------------------------------------------------------
// Function : SysGetOpenFilenameDialog				Win32Console Implementation
//   This function is used to allow the user to select a file from the folders on their puter
//--------------------------------------------------------
BOOL SysGetOpenFilenameDialog( char* szFileFilter, char* szTitle, char* szDefaultFolder, int nFlags, char* szOutputFile )
{
	return(  FALSE );
}

void		SysGetCurrentDir( int nStrLen, char* szBuffer )
{
	GetCurrentDirectory( nStrLen, szBuffer );
}

BOOL		SysSetCurrentDir( const char* szDir )
{
	return( (SetCurrentDirectory( szDir ) != 0) ? TRUE : FALSE );
}


//-------------------------------------------------------
// Function : SysGetMicrosecondTick				Win32Console Implementation
//   Returns the number of microseconds (1/1,000,000ths of a second, 1000 microseconds = 1ms)
//--------------------------------------------------------
ulong	SysGetMicrosecondTick( void )
{
__int64		u64CurrentTick;
	
	if ( QueryPerformanceCounter ( (LARGE_INTEGER*)&u64CurrentTick ) )
	{
	__int64		u64ticksPerSecond;
			
		QueryPerformanceFrequency( (LARGE_INTEGER*)&u64ticksPerSecond );

		u64CurrentTick = (u64CurrentTick * 1000000) / u64ticksPerSecond;
		return( (ulong)( u64CurrentTick ) );
	}
	return( 0 );
	/*
LARGE_INTEGER		u64CurrentTick;
	
	if ( QueryPerformanceCounter ( &u64CurrentTick ) )
	{
	LARGE_INTEGER		u64ticksPerSecond;
			
		QueryPerformanceFrequency( &u64ticksPerSecond );

		if ( u64ticksPerSecond > 0 )
		{
			u64CurrentTick = (u64CurrentTick * 1000000) / u64ticksPerSecond;
			return( (ulong)( u64CurrentTick );
		}
	}
	return( 0 );
	*/
}

//-------------------------------------------------------
// Function : SysGetTick				Win32Console Implementation
//   Returns the number of milliseconds since the puter was turned on
//		(Or when the application started.. doesnt matter which as long as it goes up regularly..)
//--------------------------------------------------------
ulong	SysGetTick( void )
{
	return( GetTickCount() );

/* Previous method..
clock_t clocktime;
float	fClockTicks;
	clocktime = clock();
	// Convert clock ticks to seconds
	fClockTicks = (float)( clocktime ) / CLK_TCK;
	// Convert seconds to ms
	fClockTicks *= 1000.0f;
	return( (ulong)(fClockTicks) + (1000*60) );	// the + (60*1000) is a lil bodge so the gulLastTick doesnt start
												// at 0, which makes a few calculations about time go a bit whacky sometimes..
												// (e.g. when doing if (EventTime < gulLastTick - 4000) )
*/
}



//-------------------------------------------------------
// Function : SysDeleteFile				Win32Console Implementation
//   Deletes the specified file from the puter
//--------------------------------------------------------
BOOL	SysDeleteFile( const char* szFilename )
{
	remove( szFilename );
	return( FALSE );
}

//-------------------------------------------------------
// Function : SysMkDir				Win32Console Implementation
//   
//--------------------------------------------------------
BOOL	SysMkDir( const char* szDirname )
{
	mkdir(szDirname);
	return( FALSE );
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

void ServerChangeIcon( void ) {}
void WindowServerShowLoginPopup( void ) {}
void EconomyUpdateTreasuryMessage( void ) {}
void SettingsEditorRefreshFields( void ) {}
void FirstRunPagesRefreshList( BOOL bParam ) {}
void BuildingSelectionInitialise( void ) {}
void BuildingSelectInitialiseDialogValues( void ) {}

