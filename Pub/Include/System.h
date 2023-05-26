#ifndef UNIVERSAL_SYSTEM_H
#define UNIVERSAL_SYSTEM_H

// This is a header intended to collect access functions to as much as the stuff as possible
// thats windows related in the client and server.
//
// Not sure what can get brought here, but the main one (the replacement for messageBox) means a few
// windows.h includes can be removed from the gamecode, which is no bad thing..
//
//  There'll need to be a platform specific implementation of each of the functions declared here.

#ifdef __cplusplus
extern "C"
{
#endif

#include "StandardDef.h"


// ----------------------------  Basic memory allocator
extern void*	SystemMalloc( int nMemsize );
extern void		SystemFree( void* pMem );

// ----------------------------  Debug print & string functions
extern void		SysDebugPrint( const char*, ... );
extern void		SysUserPrint( int mode, const char*, ... );
extern void		SysPanicIf( int condition, const char*, ... );

typedef	void(*fnDebugPrintHandler)( const char* szMessage );
extern void		SysRegisterDebugPrintHandler( fnDebugPrintHandler pDebugPrintHandler );

typedef	void(*fnUserPrintFunction)( int mode, const char* szMessage );
extern void		SysRegisterUserPrintHandler( fnUserPrintFunction pUserPrintHandler );

extern int		SysStricmp( const char* szOut, const char* szIn );
extern BOOL		SysIsFinite( float fVal );

// ---------------------------- Time and tick functions

typedef struct
{
	short	wDay;
	short	wMonth;
	short	wYear;
	short	wHour;
	short	wMinute;
	short	wSecond;
	short	wDayOfWeek;
	
} SYS_LOCALTIME;


extern void				SysGetLocalTime( SYS_LOCALTIME* pMem );		// Date/time
extern void				SysGetUTCTime( SYS_LOCALTIME* pMem );		// (This assumes the local machine as the clock set right.. )
extern uint32			SysGetTimeLong(void ); // returns time in seconds (from 1,jan 1970 or whateva)
extern uint32			SysGetTick( void );		// millisecond tick
extern u64				SysGetMicrosecondTick( void );		
extern void				SysGetLocalTimeFromUnixTimestamp( uint32 ulUnixTime, SYS_LOCALTIME* pTime );
extern uint32			SysGetUnixTimestampFromLocalTime( SYS_LOCALTIME* pTime );
extern float			SysGetFrameDelta();

extern void				SysSetFrameDelta( float fDelta );

// ----------------------------  Keyboard input
enum					// See SysCheckKeyState, SysWasJustPressed 
{
	KEY_LEFT_ARROW = 0,
	KEY_RIGHT_ARROW,
	KEY_UP_ARROW,
	KEY_DOWN_ARROW,
	KEY_WASD_LEFT,
	KEY_WASD_RIGHT,
	KEY_WASD_UP,
	KEY_WASD_DOWN,
	KEY_PAGE_UP,
	KEY_PAGE_DOWN,
	KEY_TAB,
	KEY_CTRL,
	KEY_ESCAPE,
	KEY_MENU_END,
	KEY_MENU_HOME,
	KEY_RETURN,
	KEY_INSERT,
	KEY_DELETE,
	KEY_SHIFT,
	KEY_F1,
	KEY_F2,
	KEY_F3,
	KEY_F4,
	KEY_F5,
	KEY_F6,
	KEY_F7,
	KEY_F8,
	KEY_F9,
	KEY_F10,
	KEY_F11,
	KEY_F12,
	KEY_WASD_ACTION1,
	KEY_WASD_ACTION2,
	KEY_ALT,
	KEY_BACKSPACE,
	KEY_PLUS,
	KEY_MINUS,
	KEY_SPACE,
	KEY_NUMPAD0,
	KEY_NUMPAD1,
	KEY_NUMPAD2,
	KEY_NUMPAD3,
	KEY_NUMPAD4,
	KEY_NUMPAD5,
	KEY_NUMPAD6,
	KEY_NUMPAD7,
	KEY_NUMPAD8,
	KEY_NUMPAD9,
	KEY_CAPSLOCK,
	KEY_CODES_MAX		// EOL
};

extern BOOL		SysCheckKeyState( int nKey );
extern BOOL		SysWasJustPressed( int nKey );
extern void		SysOnKeyEvent( int nVirtKeyCode, int nState );		// The external code must use this to update the keyboard status when OnKeyDown, OnKeyUp events occur
extern int		SysGetKeyCodeForVirtKey( int nVirtKeyCode );

//----------------------------
// Directory/File stuff
typedef	void(*fnDirListingCallback)( const char* szFilename );

extern const char*	SysGetWritableDataFolderPath( const char* szGameName );
extern BOOL			SysCreateWritableDataFolderPath( const char* szGameName );

extern int		SysCopyFile( const char* szSrc, const char* szDest, BOOL bFailIfExists );
extern int		SysRenameFile( const char* szSrc, const char* szDest );
extern BOOL		SysDeleteFile( const char* szFilename );
extern int		SysDeleteFolder( const char* szFolderName );

extern BOOL		SysSetFileNotReadOnly( const char* szFilename );
extern BOOL		SysMkDir( const char* szDirname );
extern void		SysGetCurrentDir( int nStrLen, char* szBuffer );
extern BOOL		SysSetCurrentDir( const char* szDir );
extern BOOL		SysDoesDirExist( const char* szDir );
extern void		SysRecurseCopyFolders( const char* szFolder, const char* szCopyRoot );
extern void		SysCopyFolderContents( const char* szSrcFolder, const char* szDestFolder );
extern void		SysGetAllFilesInFolder( const char* szSrcFolder, fnDirListingCallback callback );

extern int		SysGetFileSize( FILE* );
extern FILE*	SysFileOpen( const char* szFilename, const char* szOpenMode );
extern int		SysFileRead( unsigned char*, int, int, FILE* );
extern int		SysFileWrite( unsigned char*, int, int, FILE* );
extern int		SysFileSeek( FILE*, int, int mode );
extern void		SysFileClose( FILE* );
extern void		SysFileCloseAll( void );

extern int		SysFileGetNumOpenHandles( void );

extern int		SysSaveDataLoad( byte* pbData, int nSize );
extern int		SysSaveDataSave( byte* pbData, int nSize );

//----------------------------
// Dialogs and controls stuff

enum
{
	SYSMESSBOX_EXCLAMATION = 0x1,
	SYSMESSBOX_OK = 0x2,
	SYSMESSBOX_YESNO = 0x4,
};

extern BOOL	SysMessageBox( char* acError, char* acTitle, int nFlags );

extern BOOL	SysGetOpenFilenameDialog( char* szFileFilter, char* szTitle, char* szDefaultFolder, int nFlags, char* szOutputFile );
extern BOOL	SysGetSaveFilenameDialog( char* szFileFilter, char* szTitle, char* szDefaultFolder, int nFlags, char* szOutputFile );
extern BOOL	SysBrowseForFolderDialog( const char* szTitle, const char* szRootPath, char* szOutFolderName, const char* szDefaultPath );
extern void	SysAddFileExtensionIfNeeded( char* szFilename, const char* szExtension );
extern const char*	SysGetFileExtension( const char* szFilename );		// Returns a pointer to the extension within szFilename

//-----------------------------------------------------------------------------

extern BOOL	SysLaunchBrowserWindow( const char* szWeblink );

extern const char*		SysNetworkGetIPAddressText( uint32 ulIP );
extern uint32			SysNetworkGetIPAddress( const char* szIPAddressString );

extern void	SysGetLogDateTime( char* szOutBuffer );

//-----------------------------------------------------------------------------

#ifdef USE_GTK_INTERFACE
extern void WidgetSetEnabled( void* pTargetTopParent, char* pcTarget, BOOL boNewState );
#define MenuEntrySetEnabled( p, s, b ) WidgetSetEnabled( p, s ,b )
#else
extern void WidgetSetEnabled( void* pTargetTopParent, int nTarget, BOOL boNewState );
extern void MenuEntrySetEnabled( void* pTargetTopParent, int nTarget, BOOL boNewState );
#endif
extern void SysSetWindowTitle( void* pxWindow, char* pcNewTitle );
extern void SysEmptyList( void* pxParentWindow, void* pListIdent );
//extern void SysFillListLine( void* pxParentWindow, void* pListIdent, int nLineNum, int nValuesQtt, char** ppcValues );
extern void SysAddFilledListLine( void* pxParentWindow, void* pListIdent, int nValsQtt, char** ppcValues, int nPos, int nParam );

// System utils
extern void		SysSleep( int millisecs );
extern u64		SysStringToU64( const char* szString );

//------------ Platform specific externs

extern void		SysSetMainWindow( void* pWindowHandle );
extern void*	SysGetMainWindow( void );

//------------- Thread routines
typedef	long(*fnThreadFunction)( long );

extern unsigned int		SysCreateThread( fnThreadFunction, void* pThreadPointerParam, uint32 ulThreadParam, int nPriority );
extern void				SysExitThread( int nRetVal );

//------------- System OS processing
extern BOOL		SysOSYieldIfRequired( void );
extern BOOL		SysOSYield( void );

extern BOOL		SysOSQuitApplicationRequested( void );

extern void		SysSetMouseCursor( int mode );

#ifdef WIN32
#ifdef WINUSERAPI	// Only include this if winuser.h has been previously included
extern HINSTANCE ghInstance;
extern HWND	ghwndMain;
extern HWND	ghwndInitPopup;
#endif	// WINUSERAPI
#endif

#ifdef USE_GTK_INTERFACE
#include <gtk/gtk.h>

extern GtkWidget* gpxMainGtkWindow;
#endif

#ifdef __cplusplus
}
#endif


#endif
