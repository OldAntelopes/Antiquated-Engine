
//---------------------------------------------------------------
// SysWindows.c
//
//  This file is the windows implementations of the functions defined in System.h
//
//  For a cross-platform server (or client), a platform specific version of this
//	file should be created and implemented. (e.g. SysUnix.c)
//---------------------------------------------------------------

#define STRICT
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <windows.h>

#include <direct.h>
#include <commctrl.h>
#include <time.h>
#include <tchar.h>
#include <stdio.h> 
#include <stdarg.h>
#include <Shlobj.h>
#include <shlwapi.h>
#include <float.h>

#include <StandardDef.h>
#include <System.h>
#include <Interface.h>

HWND		mhSysMainWindow = NULL;
BOOL		mboSysPanicDontShowAnyMoreWarnings = FALSE;

int		SysDeleteFolder( const char* szFolderName )
{
bool            bSubdirectory = false;       // Flag, indicating whether subdirectories have been found
HANDLE          hFile;                       // Handle to directory
char			strFilePath[256];
char		    strPattern[256];                  // Pattern
WIN32_FIND_DATA FileInformation;             // File information
bool			bDeleteSubdirectories = true;

	sprintf( strPattern, "%s\\*.*", szFolderName );
	hFile = ::FindFirstFile(strPattern, &FileInformation);
	if(hFile != INVALID_HANDLE_VALUE)
	{
	    do
		{
			if(FileInformation.cFileName[0] != '.')
			{
				sprintf( strFilePath, "%s\\%s", szFolderName, FileInformation.cFileName );

		        if(FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					if(bDeleteSubdirectories)
					{
					    // Delete subdirectory
						int iRC = SysDeleteFolder( strFilePath );
			            if(iRC)
						  return iRC;
					}
					else
					{
						bSubdirectory = true;
					}
				}
				else
		        {
					// Set file attributes
					if( ::SetFileAttributes(strFilePath, FILE_ATTRIBUTE_NORMAL) == FALSE)
					{
						return ::GetLastError();
					}

		          // Delete file
				  if(::DeleteFile( strFilePath ) == FALSE)
				  {
						return ::GetLastError();
				  }
				}
			}
		 } while(::FindNextFile(hFile, &FileInformation) == TRUE);

	    // Close handle
		::FindClose(hFile);

	    DWORD dwError = ::GetLastError();
		if(dwError != ERROR_NO_MORE_FILES)
		{
			return dwError;
		}
		else
		{
			if(!bSubdirectory)
			{
				// Set directory attributes
				if(::SetFileAttributes(szFolderName, FILE_ATTRIBUTE_NORMAL) == FALSE)
				{
					return ::GetLastError();
				}
				
				// Delete directory
		        if(::RemoveDirectory(szFolderName) == FALSE)
				{
					return ::GetLastError();
				}
			}
		}
	}

	return 0;
}



void*	SysGetMainWindow( void )
{
	return( (void*)( mhSysMainWindow ) );
}

void		SysSetMainWindow( void* pWindowHandle )
{
	mhSysMainWindow = (HWND)(pWindowHandle);
}

char		maszWritableFolderPath[MAX_PATH] = "";

#define USER_PATH_ENABLED

BOOL	SysIsFinite( float fVal )
{
	return( _finite( fVal ) );
}


BOOL	SysCreateWritableDataFolderPath( const char* szGameName )
{
char	acFolderPath[MAX_PATH];

	if( SUCCEEDED( SHGetFolderPath( NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, acFolderPath ) ) )
	{
	int		nStrLen = strlen( acFolderPath );

		if ( ( acFolderPath[nStrLen-1] != '\\' ) &&
			 ( acFolderPath[nStrLen-1] != '/' ) )
		{
			strcat( acFolderPath, "\\" );
		}
		strcat ( acFolderPath, "GamesForMay\\" );
		SysMkDir(acFolderPath);

		if ( szGameName )
		{
			strcat( acFolderPath, szGameName );
		}
		else
		{
			strcat( acFolderPath, "The Universal\\" );
		}
		SysMkDir(acFolderPath);
		return( TRUE );
	}
	return( FALSE );

}

const char*	SysGetWritableDataFolderPath( const char* szGameName )
{
char	acPath[MAX_PATH];

#ifdef USER_PATH_ENABLED
	if ( maszWritableFolderPath[0] == 0 )
	{
		if( SUCCEEDED( SHGetFolderPath( NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, acPath ) ) )
		{
		int		nStrLen = strlen( acPath );
			if ( ( acPath[nStrLen-1] != '\\' ) &&
				 ( acPath[nStrLen-1] != '/' ) )
			{
				strcat( acPath, "\\" );
			}
			strcat ( acPath, "GamesForMay\\" );
			if ( szGameName )
			{
				strcat( acPath, szGameName );
			}
			else
			{
				strcat( acPath, "The Universal" );
			}

			SysMakeSubDirectories( "any.txt", acPath );
			strcpy( maszWritableFolderPath, acPath );
			return( maszWritableFolderPath );
		}
		else
		{
			// Couldn't get User local appdata path??
			return( maszWritableFolderPath );
		}
	}
	else
	{
		return( maszWritableFolderPath );
	}
	return( NULL );
#else
	return(maszWritableFolderPath);
#endif
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

ulong				SysGetUnixTimestampFromLocalTime( SYS_LOCALTIME* pTime )
{
time_t	epoch;
tm		xTime;

	xTime.tm_hour = pTime->wHour;
	xTime.tm_mday = pTime->wDay;
	xTime.tm_min = pTime->wMinute;
	xTime.tm_mon = pTime->wMonth;
	xTime.tm_sec = pTime->wSecond;
	xTime.tm_year = pTime->wYear;

	// todo 
	epoch = mktime( &xTime );

	return( *( (ulong*)( &epoch ) ) );
}

void		SysGetLocalTimeFromUnixTimestamp( ulong ulUnixTime, SYS_LOCALTIME* pTime )
{
struct tm *dcp;

	dcp = _localtime32((__time32_t*)(&ulUnixTime) );

	if ( dcp != NULL )
	{
		pTime->wDay = dcp->tm_mday;
		pTime->wHour = dcp->tm_hour;
		pTime->wMinute = dcp->tm_min;
		pTime->wMonth = dcp->tm_mon;
		pTime->wSecond = dcp->tm_sec;
		pTime->wYear = dcp->tm_year;
	}
	else
	{
		memset( pTime, 0, sizeof( SYS_LOCALTIME ) );
	}
}

BOOL	SysLaunchBrowserWindow( const char* szWeblink )
{
	ShellExecute( GetDesktopWindow(), "Open", szWeblink, 0, 0, SW_SHOWNORMAL );
	return( TRUE );
}


void	SysPanicImplShowDialog( const char* szErrorString )
{
	if ( mboSysPanicDontShowAnyMoreWarnings == FALSE )
	{
	char	acString[512];
	int		nRet;

		sprintf( acString, "**** Error Message ****\n%s\n", szErrorString);
		nRet = MessageBox( mhSysMainWindow, acString, "Error message", MB_ABORTRETRYIGNORE | MB_ICONEXCLAMATION);
		if ( nRet == IDIGNORE )
		{
			mboSysPanicDontShowAnyMoreWarnings = TRUE;
		}
		if ( nRet == IDABORT )
		{
			throw( szErrorString ); 
			PostQuitMessage( 0 );
		}
	}
}

void		SysPanicIf( int condition, const char* text, ... )
{
	if ( condition )
	{
	char		acString[1024];
	va_list		marker;
	ulong*		pArgs;
	int			nLen;
		pArgs = (ulong*)( &text ) + 1;

		va_start( marker, text );     
		vsprintf( acString, text, marker );

		nLen = strlen( acString );
		if ( nLen > 0)
		{
			SysPanicImplShowDialog( acString );
		}
	}
}



typedef struct
{
	int				nKeyCode;
	int				nVirtKey;
	
	unsigned int	uKeyState;

} KeyLookupTable;

KeyLookupTable	maKeyLookupTable[KEY_CODES_MAX ] =
{
	{ KEY_LEFT_ARROW,		VK_LEFT, 0 },
	{ KEY_RIGHT_ARROW,		VK_RIGHT, 0 },
	{ KEY_UP_ARROW,			VK_UP, 0 },
	{ KEY_DOWN_ARROW,		VK_DOWN, 0 },
	{ KEY_WASD_LEFT,		'A', 0 },
	{ KEY_WASD_RIGHT,		'D', 0 },
	{ KEY_WASD_UP,			'W', 0 },
	{ KEY_WASD_DOWN,		'S', 0 },
	{ KEY_PAGE_UP,			VK_PRIOR, 0 },
	{ KEY_PAGE_DOWN,		VK_NEXT, 0 },
	{ KEY_TAB,				VK_TAB, 0 },
	{ KEY_CTRL,				VK_CONTROL, 0 },
	{ KEY_ESCAPE,			VK_ESCAPE, 0 },
	{ KEY_MENU_END,			VK_END, 0 },
	{ KEY_MENU_HOME,		VK_HOME, 0 },
	{ KEY_RETURN,			VK_RETURN, 0 },
	{ KEY_INSERT,			VK_INSERT, 0 }, 
	{ KEY_DELETE,			VK_DELETE, 0 },
	{ KEY_SHIFT,			VK_SHIFT, 0 },
	{ KEY_F1,				VK_F1, 0 },
	{ KEY_F2,				VK_F2, 0 },
	{ KEY_F3,				VK_F3, 0 },
	{ KEY_F4,				VK_F4, 0 },
	{ KEY_F5,				VK_F5, 0 },
	{ KEY_F6,				VK_F6, 0 },
	{ KEY_F7,				VK_F7, 0 },
	{ KEY_F8,				VK_F8, 0 },
	{ KEY_F9,				VK_F9, 0 },
	{ KEY_F10,				VK_F10, 0 },
	{ KEY_F11,				VK_F11, 0 },
	{ KEY_F12,				VK_F12, 0 },
	{ KEY_WASD_ACTION1,		'Q', 0 },
	{ KEY_WASD_ACTION2,		'E', 0 },
	{ KEY_ALT,				VK_MENU, 0 },
	{ KEY_BACKSPACE,		VK_BACK, 0 },
	{ KEY_PLUS,				VK_ADD, 0 },
	{ KEY_MINUS,			VK_SUBTRACT, 0 },
	{ KEY_SPACE,			VK_SPACE, 0 },
	{ KEY_NUMPAD0,			VK_NUMPAD0, 0 },
	{ KEY_NUMPAD1,			VK_NUMPAD1, 0 },
	{ KEY_NUMPAD2,			VK_NUMPAD2, 0 },
	{ KEY_NUMPAD3,			VK_NUMPAD3, 0 },
	{ KEY_NUMPAD4,			VK_NUMPAD4, 0 },
	{ KEY_NUMPAD5,			VK_NUMPAD5, 0 },
	{ KEY_NUMPAD6,			VK_NUMPAD6, 0 },
	{ KEY_NUMPAD7,			VK_NUMPAD7, 0 },
	{ KEY_NUMPAD8,			VK_NUMPAD8, 0 },
	{ KEY_NUMPAD9,			VK_NUMPAD9, 0 },
	{ KEY_CAPSLOCK,			VK_CAPITAL, 0 },
	
};

int		SysGetKeyCodeForVirtKey( int nVirtKeyCode )
{
int		loop;

	for ( loop = 0; loop < KEY_CODES_MAX; loop++ )
	{
		if ( maKeyLookupTable[loop].nVirtKey == nVirtKeyCode )
		{
			return( maKeyLookupTable[loop].nKeyCode );
		}
	}
	return( nVirtKeyCode );
}

// The external code must use this to update the keyboard status when OnKeyDown, OnKeyUp events occur
void		SysOnKeyEvent( int nVirtKeyCode, int state )
{
int		loop;

	for ( loop = 0; loop < KEY_CODES_MAX; loop++ )
	{
		if ( maKeyLookupTable[loop].nVirtKey == nVirtKeyCode )
		{
			if ( state == 1 ) // key down
			{
				maKeyLookupTable[loop].uKeyState = 1;
			}
			else if ( state == 0 ) // key up
			{
				maKeyLookupTable[loop].uKeyState = 0;
			}	
			return;
		}
	}
}


const char*		SysNetworkGetIPAddressText( ulong ulIP )
{
	return( inet_ntoa( *((struct in_addr*)(&ulIP))) );
}

ulong			SysNetworkGetIPAddress( const char* szIPAddressString )
{
	return( (ulong)(inet_addr( szIPAddressString )) );
}



BOOL		SysWasJustPressed( int nKey )
{
int		loop;

	for ( loop = 0; loop < KEY_CODES_MAX; loop++ )
	{
		if ( maKeyLookupTable[loop].nKeyCode == nKey )
		{
			if ( maKeyLookupTable[loop].uKeyState == 1  )
			{
				maKeyLookupTable[loop].uKeyState = 0;
				return( TRUE );
			}
			return( FALSE );
		}
	}
	return( FALSE );
}

BOOL	SysCheckKeyState( int nKey )
{
int		loop;

	for ( loop = 0; loop < KEY_CODES_MAX; loop++ )
	{
		if ( maKeyLookupTable[loop].nKeyCode == nKey )
		{
			if ( GetKeyState( maKeyLookupTable[loop].nVirtKey ) < 0 )
			{
				return( TRUE );
			}
			return( FALSE );
		}
	}
	return( FALSE );
}




//-------------------------------------------------------
// Function : WidgetSetEnabled				Windows Implementation
//   Set a control enabled or disabled
//--------------------------------------------------------
void WidgetSetEnabled( void* pTargetTopParent, int nTarget, BOOL boNewState )
{
	EnableWindow( GetDlgItem( (HWND)pTargetTopParent, nTarget ), boNewState );
}

//-------------------------------------------------------
// Function : MenuEntrySetEnabled				Windows Implementation
//   Set a menu item enabled or disabled
//--------------------------------------------------------
void MenuEntrySetEnabled( void* pTargetTopParent, int nTarget, BOOL boNewState )
{
	EnableMenuItem( GetMenu( (HWND)pTargetTopParent ), nTarget, boNewState ? MF_ENABLED : MF_GRAYED );
}

//-------------------------------------------------------
// Function : SysMessageBox				Windows Implementation
//   Given an error msg, dialog box title and flags, this function 
//   should create a popup messagebox window.
//   Returns FALSE if user pressed "NO" or "Cancel", TRUE otherwise.
//--------------------------------------------------------
BOOL	SysMessageBox( char* acError, char* acTitle, int nFlags )
{
int		nRet = 0;
UINT	unWinFlags = 0;

	if ( (nFlags & SYSMESSBOX_EXCLAMATION) )
	{
		unWinFlags |= MB_ICONEXCLAMATION;
	} 

	if ( (nFlags & SYSMESSBOX_OK) )
	{
		unWinFlags |= MB_OK;
	}
	if ( (nFlags & SYSMESSBOX_YESNO) )
	{
		unWinFlags |= (MB_ICONQUESTION|MB_YESNO);
	}
	
	
	nRet = MessageBox( mhSysMainWindow, acError, acTitle, unWinFlags );

	return( (nRet == IDNO) ? FALSE : TRUE );
}

BOOL SysDoesDirExist( const char* szPath)
{
DWORD dwAttrib = GetFileAttributes(szPath);

	if ( ( dwAttrib != INVALID_FILE_ATTRIBUTES ) &&
		 ( dwAttrib & FILE_ATTRIBUTE_DIRECTORY ) )
	{
		return( TRUE );
	}
	return( FALSE );
}

//-------------------------------------------------------
// Function : SysSetWindowTitle				Windows Implementation
//   This function change the title of a window
//--------------------------------------------------------
void SysSetWindowTitle( void* pxWindow, char* pcNewTitle )
{
	SendMessage( (HWND)pxWindow, WM_SETTEXT, 0, (LPARAM)pcNewTitle );
}

//-------------------------------------------------------
// Function : SysEmptyList				Windows Implementation
//   This function make a list control empty
//   pxParentWindow is the parent window, pListIdent the list control number (int)
//--------------------------------------------------------
void SysEmptyList( void* pxParentWindow, void* pListIdent )
{
	SendDlgItemMessage( (HWND)pxParentWindow, *(int*)pListIdent, LVM_DELETEALLITEMS, 0, 0 );
}

//-------------------------------------------------------
// Function : SysAddFilledListLine				Windows Implementation
//   This function fill up a new line of a list control with nValsQtt strings.
//   pxParentWindow is the parent window, pListIdent the list control ident (int)
//--------------------------------------------------------
void SysAddFilledListLine( void* pxParentWindow, void* pListIdent, int nValsQtt, char** ppcValues, int nPosInList, int nParam )
{
LVITEM	xInsert;
int nLoop;
	
	xInsert.state = 0;
	xInsert.stateMask = 0;
	xInsert.iImage = 0;
	xInsert.iItem = nPosInList;
	xInsert.lParam = nParam;
	
	for (nLoop=0; nLoop<nValsQtt; nLoop++)
	{
		xInsert.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
		
		xInsert.iSubItem = nLoop;
		xInsert.pszText = ppcValues[nLoop];

		if (nLoop == 0)
		{
			xInsert.mask |= LVIF_PARAM;
			SendDlgItemMessage( (HWND)pxParentWindow, *(int*)pListIdent, LVM_INSERTITEM, 0, (LPARAM)&xInsert );
		}
		else
		{
			SendDlgItemMessage( (HWND)pxParentWindow, *(int*)pListIdent, LVM_SETITEM, 0, (LPARAM)&xInsert );
		}
	}
}

int		SysRenameFile( const char* szSrc, const char* szDest )
{
	rename( szSrc, szDest);
	return( 1 );
}
int		SysCopyFile( const char* szSrc, const char* szDest, BOOL bFailIfExists )
{
	CopyFile( szSrc, szDest, bFailIfExists );
	return( 1 );
}

const char*	SysGetFileExtension( const char* szFilename )
{
int		nNameLen = strlen( szFilename );
int		nLimit = nNameLen - 8;
const char*		pcRunner = szFilename + nNameLen - 1;
	if ( nLimit < 0 ) nLimit = 0;

	while( nNameLen > nLimit )
	{
		if ( *pcRunner == '.' )
		{
			pcRunner++;
			return( pcRunner );
		}
		pcRunner--;
		nNameLen--;
	}

	return( NULL );
}



void	SysAddFileExtensionIfNeeded( char* szFilename, const char* szExtension )
{
int		nNameLen = strlen( szFilename );
char	acExtensionRequired[64];

	sprintf( acExtensionRequired, ".%s", szExtension );
	if ( stricmp( szFilename + (nNameLen-4), acExtensionRequired) != 0 )
	{
		strcat( szFilename, acExtensionRequired ); 
	}
}

//-------------------------------------------------------
// Function : SysGetSaveFilenameDialog				Windows Implementation
//   This function is used to allow the user to select a filename to save to from the folders on their puter.
//   szOutputFile where the filename will be stocked is expected to be an allocated 256 char string.
//   Returns True if all ok, False if error or user pressed cancel button.
//--------------------------------------------------------
BOOL SysGetSaveFilenameDialog( char* szFileFilter, char* szTitle, char* szDefaultFolder, int nFlags, char* szOutputFile )
{
OPENFILENAME	ofnOpen;

	ZeroMemory( &ofnOpen, sizeof( ofnOpen ) );
	ofnOpen.lStructSize       = sizeof(OPENFILENAME);
	ofnOpen.hwndOwner         = mhSysMainWindow;
    ofnOpen.hInstance         = 0;
	ofnOpen.lpstrFilter       = szFileFilter;
    ofnOpen.lpstrCustomFilter = NULL;
    ofnOpen.nMaxCustFilter    = 0;
    ofnOpen.nFilterIndex      = 1;
    ofnOpen.lpstrFile         = (LPSTR)szOutputFile;
    ofnOpen.nMaxFile          = 256;
    ofnOpen.lpstrInitialDir   = szDefaultFolder;
    ofnOpen.lpstrTitle        = szTitle;
	switch ( nFlags )
	{
	case 0:
	default:
		ofnOpen.Flags = OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR;
		break;
	}

    ofnOpen.nFileOffset       = 0;
    ofnOpen.nFileExtension    = 0;

	return( (GetSaveFileName( &ofnOpen ) != 0) ? TRUE : FALSE );
}


//-------------------------------------------------------
// Function : SysGetOpenFilenameDialog				Windows Implementation
//   This function is used to allow the user to select a file from the folders on their puter
//--------------------------------------------------------
BOOL SysGetOpenFilenameDialog( char* szFileFilter, char* szTitle, char* szDefaultFolder, int nFlags, char* szOutputFile )
{
OPENFILENAME	ofnOpen;

	ZeroMemory( &ofnOpen, sizeof( ofnOpen ) );
	ofnOpen.lStructSize       = sizeof(OPENFILENAME);
	ofnOpen.hwndOwner         = mhSysMainWindow;
	ofnOpen.hInstance         = 0;
	ofnOpen.lpstrFilter       = (LPSTR)szFileFilter;
	ofnOpen.lpstrCustomFilter = NULL;
	ofnOpen.nMaxCustFilter    = 0;
	ofnOpen.nFilterIndex      = 1;
	ofnOpen.lpstrFile         = (LPSTR)szOutputFile;
	ofnOpen.nMaxFile          = 256;
	ofnOpen.lpstrInitialDir   = szDefaultFolder;
	ofnOpen.lpstrTitle        = szTitle;
	switch ( nFlags )
	{
	case 0:
	default:
		ofnOpen.Flags             = OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR;//|OFN_EXPLORER;
		break;
	}
	ofnOpen.nFileOffset       = 0;
	ofnOpen.nFileExtension    = 0;

	int ret =  GetOpenFileName( &ofnOpen );

	if ( ret != 0 )
	{
		return( TRUE );
	}
	else
	{
		DWORD	error = CommDlgExtendedError();
		if ( error == CDERR_DIALOGFAILURE )
		{
		int	i = 0;
			i++;
		}
	}
	return( FALSE );
}

LPITEMIDLIST ConvertPathToLpItemIdList(const char *pszPath)
{
	LPITEMIDLIST  pidl = NULL;
	LPSHELLFOLDER pDesktopFolder = NULL;
	OLECHAR       olePath[MAX_PATH];
	ULONG         chEaten;
	HRESULT       hr;

	if (SUCCEEDED(SHGetDesktopFolder(&pDesktopFolder)))
	{
		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pszPath, -1, 
					olePath, MAX_PATH);
		hr = pDesktopFolder->ParseDisplayName(NULL, NULL, 
					olePath, &chEaten, &pidl, NULL);
		pDesktopFolder->Release();
		return pidl;
	}
	return NULL;
}

char	mszBrowseDefaultPath[256] = "";

int	WINAPI	BrowseCallbackProc( HWND   hwnd,  UINT   uMsg,  LPARAM lParam,  LPARAM lpData )
{

	if ( uMsg == BFFM_INITIALIZED )
	{
		if ( mszBrowseDefaultPath[0] != 0 )
		{
			SendMessage( hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)mszBrowseDefaultPath );
		}
	}

	return( 0 );
}


BOOL	SysBrowseForFolderDialog( const char* szTitle, const char* szRootPath, char* szOutFolderName, const char* szDefaultPath )
{
BROWSEINFO	xBrowseInfo;
LPITEMIDLIST		pItemList;
char	acFolderPath[512];

	if ( szDefaultPath )
	{
		strcpy( mszBrowseDefaultPath, szDefaultPath );
	}
	else
	{
		mszBrowseDefaultPath[0] = 0;
	}
	ZeroMemory( &xBrowseInfo, sizeof( BROWSEINFO ) );

	xBrowseInfo.hwndOwner = mhSysMainWindow;
	xBrowseInfo.lpszTitle = szTitle;
	xBrowseInfo.pidlRoot = ConvertPathToLpItemIdList(szRootPath);
	xBrowseInfo.pszDisplayName = (LPSTR)acFolderPath;
	xBrowseInfo.lpfn = BrowseCallbackProc;
	xBrowseInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;

	pItemList = SHBrowseForFolder( &xBrowseInfo );

	if ( pItemList != NULL )
	{
		SHGetPathFromIDList( pItemList, acFolderPath );
		strcat( szOutFolderName, acFolderPath );
		return( TRUE );
	}	
	return( FALSE );
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
u64		SysGetMicrosecondTick( void )
{
__int64		u64CurrentTick;
	
	if ( QueryPerformanceCounter ( (LARGE_INTEGER*)&u64CurrentTick ) )
	{
	__int64		u64ticksPerSecond;
			
		QueryPerformanceFrequency( (LARGE_INTEGER*)&u64ticksPerSecond );

		u64CurrentTick = (u64CurrentTick * 1000000) / u64ticksPerSecond;
		return( (u64)( u64CurrentTick ) );
	}
	return( 0 );
}

//-------------------------------------------------------
// Function : SysGetTick				Windows Implementation
//   Returns the number of milliseconds since the puter was turned on
//		(Or when the application started.. doesnt matter which as long as it goes up regularly..)
//--------------------------------------------------------
ulong	SysGetTick( void )
{
__int64		u64CurrentTick;
	
	if ( QueryPerformanceCounter ( (LARGE_INTEGER*)&u64CurrentTick ) )
	{
	__int64		u64ticksPerSecond;
			
		QueryPerformanceFrequency( (LARGE_INTEGER*)&u64ticksPerSecond );

		u64CurrentTick = (u64CurrentTick * 1000) / u64ticksPerSecond;
		return( (ulong)( u64CurrentTick ) );
	}

	return( GetTickCount() );
/*#ifdef SERVER
	return( GetTickCount() );
#else
#ifdef TOOL
	return( GetTickCount() );
#else
float	fTime = DXUtil_Timer(TIMER_GETABSOLUTETIME);
	if ( fTime == 0.0f )
	{
		return( GetTickCount() );
	}
	return( (ulong)(fTime*1000.0f) );
#endif
#endif
*/
}


BOOL	SysSetFileNotReadOnly( const char* szFilename )
{
	return( SetFileAttributes( szFilename, FILE_ATTRIBUTE_NORMAL ) );
}

//-------------------------------------------------------
// Function : SysDeleteFile				Windows Implementation
//   Deletes the specified file from the puter
//--------------------------------------------------------
BOOL	SysDeleteFile( const char* szFilename )
{
	return( DeleteFile( szFilename ) );
}

/***************************************************************************
 * Function    : SysGetAllFilesInFolder
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
void		SysGetAllFilesInFolder( const char* szSrcFolder, fnDirListingCallback callback )
{
WIN32_FIND_DATA FileData; 
char	acString[256];
HANDLE hSearch; 
BOOL fFinished = FALSE; 

	sprintf( acString, "%s/*.*", szSrcFolder );	  
	hSearch = FindFirstFile(acString, &FileData); 

	if (hSearch != INVALID_HANDLE_VALUE) 
	{ 
		while ( !fFinished )
		{ 
			if( ( !lstrcmp(FileData.cFileName, ".")  ) ||
				( !lstrcmp(FileData.cFileName, "..") ) )
			{
				// Jus skip these
			}
			else
			{
				sprintf( acString, "%s\\%s", szSrcFolder,  FileData.cFileName );
				callback( acString );
			}
	 
		    if (!FindNextFile(hSearch, &FileData)) 
		    {
	            fFinished = TRUE; 
		    }
		} 
		// Close the search handle.  
		FindClose(hSearch);
	}
}


/***************************************************************************
 * Function    : SysCopyFolderContents
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
void SysCopyFolderContents( const char* szSrcFolder, const char* szDestFolder )
{
WIN32_FIND_DATA FileData; 
char	acString[256];
char	acString2[256];
HANDLE hSearch; 
BOOL fFinished = FALSE; 

	sprintf( acString, "%s/*.*", szSrcFolder );	 
	 
	hSearch = FindFirstFile(acString, &FileData); 

	if (hSearch != INVALID_HANDLE_VALUE) 
	{ 
		while ( !fFinished )
		{ 
			if( ( !lstrcmp(FileData.cFileName, ".")  ) ||
				( !lstrcmp(FileData.cFileName, "..") ) )
			{
				// Jus skip these
			}
			else
			{
				sprintf( acString, "%s\\%s", szSrcFolder,  FileData.cFileName );
				sprintf( acString2, "%s\\%s", szDestFolder, FileData.cFileName );
				CopyFile( acString, acString2, FALSE );
			}
	 
		    if (!FindNextFile(hSearch, &FileData)) 
		    {
	            fFinished = TRUE; 
		    }
		} 
		// Close the search handle.  
		if (!FindClose(hSearch)) 
		{ 
//		   PANIC_IF(TRUE,"File system error");
		} 
	}
}


/***************************************************************************
 * Function    : SysRecurseCopyFolders
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
void SysRecurseCopyFolders( const char* szFolder, const char* szCopyRoot )
{
WIN32_FIND_DATA FileData; 
HANDLE hSearch; 
BOOL fFinished = FALSE; 
char	acString[256];
char	acString2[256];

	// create a copy of the directory in the main folder
	sprintf( acString, "%s\\%s", szCopyRoot, szFolder );
	SysMkDir( acString );
	
	// Copy all the files into it
	SysCopyFolderContents( szFolder, acString );
	sprintf( acString, "%s\\*", szFolder );	 
	 
	hSearch = FindFirstFile(acString, &FileData); 
	// Search for subfolders and recurse them too
	if (hSearch != INVALID_HANDLE_VALUE) 
	{ 
		while ( (!fFinished) )
		{ 
			if( ( !lstrcmp(FileData.cFileName, ".")  ) ||
				( !lstrcmp(FileData.cFileName, "..") ) )
			{
				// Jus skip these for the moment
			}
			else
			{
				if(FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					sprintf( acString2,"%s\\%s", szFolder, FileData.cFileName );
					sprintf( acString, "%s\\%s", szCopyRoot, acString2 );
					SysMkDir( acString );
					SysRecurseCopyFolders( acString2, szCopyRoot );
				}
			}

		    if (!FindNextFile(hSearch, &FileData)) 
		    {
	            fFinished = TRUE; 
		    }
		}
	}
}

//-------------------------------------------------------
// Function : SysMkDir				Windows Implementation
//   
//  returns 0 if directory created ok (or if it already exists)
//   otherwise returns 1
//--------------------------------------------------------
BOOL	SysMkDir( const char* acPath )
{
char	acCurrentDir[256];
char	acFullPath[256];
const char*	pcRunner = acPath;
const char*	pcPathToCreate = acPath;

	GetCurrentDirectory( 256, acCurrentDir );

	if ( SetCurrentDirectory( acPath ) == FALSE )
	{
		while( ( *pcRunner != ':' ) &&
			   ( *pcRunner != 0 ) )
		{
			pcRunner++;
		}
		// If we weren't provided with a full absolute path
		if ( *pcRunner == 0 )
		{
			strcpy( acFullPath, acCurrentDir );
			strcat( acFullPath, "/" );
			strcat( acFullPath, acPath );
			pcPathToCreate = acFullPath;
		}

		if ( ERROR_SUCCESS != SHCreateDirectoryEx( NULL, pcPathToCreate, NULL ) )
		{
			SetCurrentDirectory( acCurrentDir );
			return( 1 );
		}
	}

	SetCurrentDirectory( acCurrentDir );
	return( 0 );
}


void		SysSleep( int millisecs )
{
	Sleep(millisecs);
}

void				SysExitThread( int nRetVal )
{
	// nothing to do on windows.. just a marmalade thing.
}


unsigned int		SysCreateThread( fnThreadFunction pfnThreadFunction, void* pThreadPointerParam, ulong ulThreadParam, int nPriority )
{
ulong iID;
HANDLE hThread;

	hThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)pfnThreadFunction,(LPVOID)(pThreadPointerParam),ulThreadParam,&iID);
	if ( nPriority < 0 )
	{
		SetThreadPriority( hThread, THREAD_PRIORITY_BELOW_NORMAL );
	}
	return( (unsigned int)( hThread ) );
}

void		SysSetMouseCursor( int mode )
{
	if ( mode == 0 )
	{
		SetCursor( LoadCursor(NULL, IDC_ARROW) );
	}
}


BOOL	mboApplicationExitRequested = FALSE;

BOOL		SysOSYield( void )
{
	SysSleep( 0 );
	return( TRUE );
}

BOOL		SysOSYieldIfRequired( void )
{
MSG		windowsMsg;

	ZeroMemory( &windowsMsg, sizeof(windowsMsg) );

	if ( PeekMessage( &windowsMsg, NULL, 0U, 0U, PM_REMOVE ) )
	{ 
		if ( windowsMsg.message == WM_QUIT )
		{
			mboApplicationExitRequested = TRUE;
		}

		TranslateMessage( &windowsMsg );
		DispatchMessage( &windowsMsg );
		return( TRUE );
	}
	return( FALSE );
}

BOOL		SysOSQuitApplicationRequested( void )
{
	return( mboApplicationExitRequested );
}
