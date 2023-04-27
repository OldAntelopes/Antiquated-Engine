//---------------------------------------------------------------
// SysWindows.c
//
//  This file is the windows implementations of the functions defined in System.h
//
//  For a cross-platform server (or client), a platform specific version of this
//	file should be created and implemented. (e.g. SysUnix.c)
//---------------------------------------------------------------
#include <stdio.h>

#include <windows.h>
#include <StandardDef.h>
#include "System.h"
#include <direct.h>
#include <commdlg.h>
#include <commctrl.h>
#include <time.h>

#include <Interface.h>

#ifdef SERVER
#include "../LibCode/WinServer/Windows Server.h"
#include "../Gamecode/Server/Server.h"
#endif

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

BOOL	SysCheckKeyState( int nKey )
{
#ifndef SERVER
#ifndef TOOL
	if ( ( InterfaceIsFullscreen() == FALSE ) &&
		 ( GetFocus() != ghwndMain ) )
	{
		return( FALSE );
	}
#endif
#endif

	switch ( nKey )
	{
	case KEY_LEFT_ARROW:
		if ( GetKeyState( VK_LEFT ) < 0 ) return( TRUE );
		break;
	case KEY_UP_ARROW:
		if ( GetKeyState( VK_UP ) < 0 )	return( TRUE );
		break;
	case KEY_DOWN_ARROW:
		if ( GetKeyState( VK_DOWN ) < 0 ) return( TRUE );
		break;
	case KEY_RIGHT_ARROW:
		if ( GetKeyState( VK_RIGHT ) < 0 ) return( TRUE );
		break;
	case KEY_MENU_UP:
		if ( GetKeyState( VK_PRIOR ) < 0 ) 	return( TRUE );
		break;
	case KEY_MENU_DOWN:
		if ( GetKeyState( VK_NEXT ) < 0 ) return( TRUE );
		break;
	case KEY_MENU_SELECT:
		if ( GetKeyState( VK_END ) < 0 ) return( TRUE );
		break;
	case KEY_TAB:
		if ( GetKeyState( VK_TAB ) < 0 ) return( TRUE );
		break;
	case KEY_CTRL:
		if ( GetKeyState( VK_CONTROL ) < 0 ) return( TRUE );
		break;
	case KEY_ESCAPE:
		if ( GetKeyState( VK_ESCAPE ) < 0 ) return( TRUE );
		break;
	case KEY_MENU_END:
		if ( GetKeyState( VK_END ) < 0 ) return( TRUE );
		break;
	case KEY_MENU_HOME:
		if ( GetKeyState( VK_HOME ) < 0 ) return( TRUE );
		break;
	case KEY_RETURN:
		if ( GetKeyState( VK_RETURN ) < 0 ) return( TRUE );
		break;
	}
	return( FALSE );

}

// -- Wrapper for malloc or whateva equivalent mem alloc function
void*	SystemMalloc( int nMemsize )
{
#ifdef MEM_BOUNDS_CHECK
byte*	pbMem;

	nMemsize += 8;
	pbMem = (byte*)malloc(nMemsize);
	pbMem[0] = 0xEE;
	pbMem[1] = 0x01;
	pbMem[2] = 0xCC;
	pbMem[3] = 0x44;
	pbMem[nMemsize-4] = 0xEE;
	pbMem[nMemsize-3] = 0x01;
	pbMem[nMemsize-2] = 0xCC;
	pbMem[nMemsize-1] = 0x44;
	return( (void*)(pbMem + 4) );
#endif
	return( malloc( nMemsize ) );
}

// -- Wrapper for free or whateva equivalent mem alloc function
void		SystemFree( void* pMem )
{
#ifdef MEM_BOUNDS_CHECK
byte*	pbMem;
	pbMem = (byte*)(pMem);
	pbMem -= 4;
	pMem = (void*)(pbMem);
	if ( ( pbMem[0] != 0xEE ) ||
		 ( pbMem[1] != 0x01 ) ||
		 ( pbMem[2] != 0xCC ) ||
		 ( pbMem[3] != 0x44 ) )
	{
		// TODO - raise a windows type assertion here to let us jump to debug
		PANIC_IF(TRUE,"pre-mem bounds check error");
	}
#endif
	free( pMem );
}
// Erm, you know, malloc() and free() are kinda universal... --Guy
// yeh but using em means you cant easily slot in your own memchecking stuff..
//   more possible debug functionality than a cross platform thing - M

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
	
	
#ifndef TOOL
	nRet = MessageBox( ghwndMain, acError, acTitle, unWinFlags );
#else
	nRet = MessageBox( gpxMainWindow, acError, acTitle, unWinFlags );
#endif

	return( (nRet == IDNO) ? FALSE : TRUE );
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
//   nPosInList describes where in the list to add it (-1 for the end)
//--------------------------------------------------------
extern void SysAddFilledListLine( void* pxParentWindow, void* pListIdent, int nValsQtt, char** ppcValues, int nPosInList )
{
LVITEM	xInsert;
int nLoop;
	
	xInsert.state = 0;
	xInsert.stateMask = 0;
	xInsert.iImage = 0;
	xInsert.iItem = nPosInList;
	xInsert.lParam = 0;
	
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

int		SysRenameFile( char* szSrc, char* szDest )
{
	rename( szSrc, szDest);
	return( 1 );
}
int		SysCopyFile( char* szSrc, char* szDest, BOOL bFailIfExists )
{
	CopyFile( szSrc, szDest, bFailIfExists );
	return( 1 );
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
#ifndef TOOL
    ofnOpen.hwndOwner         = ghwndMain;
#else
	ofnOpen.hwndOwner         = gpxMainWindow;
#endif
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
#ifndef TOOL
	ofnOpen.hwndOwner         = ghwndMain;
#else
	ofnOpen.hwndOwner         = gpxMainWindow;
#endif
	ofnOpen.hInstance         = 0;
	ofnOpen.lpstrFilter       = (LPSTR)szFileFilter;
	ofnOpen.lpstrCustomFilter = NULL;
	ofnOpen.nMaxCustFilter    = 0;
	ofnOpen.nFilterIndex      = 0;
	ofnOpen.lpstrFile         = (LPSTR)szOutputFile;
	ofnOpen.nMaxFile          = 256;
	ofnOpen.lpstrInitialDir   = szDefaultFolder;
	ofnOpen.lpstrTitle        = szTitle;
	switch ( nFlags )
	{
	case 0:
	default:
		ofnOpen.Flags             = OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR;
		break;
	}
	ofnOpen.nFileOffset       = 0;
	ofnOpen.nFileExtension    = 0;

	if ( GetOpenFileName( &ofnOpen ) == 0 )
	{
		int	nError = CommDlgExtendedError();
		return( FALSE );
	}
	return( TRUE );

}

void		SysGetCurrentDir( int nStrLen, char* szBuffer )
{
	GetCurrentDirectory( nStrLen, szBuffer );
}

BOOL		SysSetCurrentDir( char* szDir )
{
	return( (SetCurrentDirectory( szDir ) != 0) ? TRUE : FALSE );
}



//-------------------------------------------------------
// Function : SysGetTick				Windows Implementation
//   Returns the number of milliseconds since the puter was turned on
//		(Or when the application started.. doesnt matter which as long as it goes up regularly..)
//--------------------------------------------------------
uint32	SysGetTick( void )
{
	return( GetTickCount() );
}



//-------------------------------------------------------
// Function : SysDeleteFile				Windows Implementation
//   Deletes the specified file from the puter
//--------------------------------------------------------
BOOL	SysDeleteFile( char* szFilename )
{
	return( DeleteFile( szFilename ) );
}

//-------------------------------------------------------
// Function : SysMkDir				Windows Implementation
//   
//--------------------------------------------------------
BOOL	SysMkDir( char* szDirname )
{
	return( mkdir( szDirname ) );
}
