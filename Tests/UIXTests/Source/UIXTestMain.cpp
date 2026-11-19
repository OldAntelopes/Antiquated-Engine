//------------------------------------------------------------------------------------------------
// UIXTestMain.cpp
//
// Entry point for the UIX test harness. GameCommon\Platform\PlatformWin32.cpp owns WinMain, as it does in
// an app, and calls MainInitialise. The harness does all its work there: makes a hidden window and a D3D9
// device at a fixed 640x480, initialises UI and UIX the way an app does, runs every test and exits with the
// number of failed tests (0 = all passed).
//
//   UIXTestsD.exe [--update-refs] [--only <text>] [--list] [--verbose]
//                 [--pixel-tolerance N] [--max-diff-pixels N]
//
// RunUIXTests.ps1 builds and runs it.
//------------------------------------------------------------------------------------------------

#include "UIXTestCore.h"
#include "DirectX/d3dx9.h"

#define		UIXTEST_SCREEN_W		640
#define		UIXTEST_SCREEN_H		480

extern void		UIXTestSetOptions( BOOL bUpdateRefs, BOOL bVerbose, BOOL bList, const char* szOnly, const char* szRefs, const char* szResults, int nPixelTolerance, int nMaxDiffPixels );
extern void		UIXTestSetIconSheet( int hTexture );
extern int		UIXTestRunAll( void );

//------------------------------------------------------------------------------------------------
// A 128x128 icon sheet in UIX's layout (6x6 cells of 19 pixels), drawn here so the harness needs no art
// of its own: each icon is an outlined square with a pattern that differs by icon number.
//------------------------------------------------------------------------------------------------
static BOOL		UIXTestMakeIconSheet( const char* szFilename )
{
LPDIRECT3DDEVICE9		pDevice = (LPDIRECT3DDEVICE9)InterfaceGetD3DDevice();
LPDIRECT3DSURFACE9		pSurface = NULL;
D3DLOCKED_RECT			xLocked;
BOOL					bOK = FALSE;
int						nX, nY;

	if ( FAILED( pDevice->CreateOffscreenPlainSurface( 128, 128, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &pSurface, NULL ) ) )
	{
		return( FALSE );
	}
	if ( SUCCEEDED( pSurface->LockRect( &xLocked, NULL, 0 ) ) )
	{
		for ( nY = 0; nY < 128; nY++ )
		{
		uint32*		pulRow = (uint32*)( (BYTE*)xLocked.pBits + ( nY * xLocked.Pitch ) );

			for ( nX = 0; nX < 128; nX++ )
			{
			int		nCellX = nX / 19;
			int		nCellY = nY / 19;
			int		nInX = nX % 19;
			int		nInY = nY % 19;
			int		nIcon = ( nCellY * 6 ) + nCellX;
			uint32	ulCol = 0x00000000;

				if ( ( nCellX < 6 ) && ( nCellY < 6 ) )
				{
					if ( ( nInX == 1 ) || ( nInX == 17 ) || ( nInY == 1 ) || ( nInY == 17 ) )
					{
						ulCol = 0xFFFFFFFF;
					}
					else if ( ( nInX > 2 ) && ( nInX < 16 ) && ( nInY > 2 ) && ( nInY < 16 ) )
					{
					int		nQuadrant = ( ( nInX < 9 ) ? 0 : 1 ) + ( ( nInY < 9 ) ? 0 : 2 );

						// Icon N fills the quadrants given by the low 4 bits of N+1: coarse enough to survive
						// the texture being loaded reduced, so icons stay distinguishable
						if ( ( ( nIcon + 1 ) >> nQuadrant ) & 1 )
						{
							ulCol = 0xFFFFFFFF;
						}
					}
				}
				pulRow[nX] = ulCol;
			}
		}
		pSurface->UnlockRect();
		bOK = SUCCEEDED( D3DXSaveSurfaceToFile( szFilename, D3DXIFF_PNG, pSurface, NULL, NULL ) );
	}
	pSurface->Release();
	return( bOK );
}

static HWND		UIXTestCreateHiddenWindow( void )
{
WNDCLASSEX	xClass;
RECT		xRect = { 0, 0, UIXTEST_SCREEN_W, UIXTEST_SCREEN_H };
DWORD		dwStyle = WS_OVERLAPPEDWINDOW;

	memset( &xClass, 0, sizeof( xClass ) );
	xClass.cbSize = sizeof( xClass );
	xClass.style = CS_CLASSDC;
	xClass.lpfnWndProc = DefWindowProc;
	xClass.hInstance = GetModuleHandle( NULL );
	xClass.lpszClassName = "UIXTestsWindow";
	RegisterClassEx( &xClass );

	AdjustWindowRect( &xRect, dwStyle, FALSE );
	// Never shown: D3D9 renders into its back buffer, which the tests read back
	return( CreateWindow( "UIXTestsWindow", "UIX tests", dwStyle, 0, 0, xRect.right - xRect.left, xRect.bottom - xRect.top, NULL, NULL, xClass.hInstance, NULL ) );
}

static BOOL		UIXTestFindArg( const char* szCmdLine, const char* szArg, char* pcValueOut, int nValueMax )
{
const char*		pcFound = strstr( szCmdLine, szArg );
int				nLen = 0;

	if ( pcFound == NULL )
	{
		return( FALSE );
	}
	if ( pcValueOut )
	{
		pcFound += strlen( szArg );
		while ( *pcFound == ' ' ) pcFound++;
		if ( *pcFound == '"' )
		{
			pcFound++;
			while ( ( *pcFound ) && ( *pcFound != '"' ) && ( nLen < nValueMax - 1 ) ) pcValueOut[nLen++] = *pcFound++;
		}
		else
		{
			while ( ( *pcFound ) && ( *pcFound != ' ' ) && ( nLen < nValueMax - 1 ) ) pcValueOut[nLen++] = *pcFound++;
		}
		pcValueOut[nLen] = 0;
	}
	return( TRUE );
}

void	MainInitialise( void )
{
const char*		szCmdLine = PlatformGetStartupCommandLine();
char			acExePath[MAX_PATH];
char			acRoot[MAX_PATH];
char			acHarness[MAX_PATH];
char			acRefs[MAX_PATH];
char			acResults[MAX_PATH];
char			acExeData[MAX_PATH];
char			acIconSheet[MAX_PATH];
char			acOnly[128] = "";
char			acValue[32];
char*			pcSlash;
int				nPixelTolerance = 24;
int				nMaxDiffPixels = 8;
int				nFailed;
HWND			hWnd;

	if ( szCmdLine == NULL ) szCmdLine = "";

	// The exe is in Antiquated\Tests\UIXTests\Out
	GetModuleFileName( NULL, acExePath, MAX_PATH );
	pcSlash = strrchr( acExePath, '\\' );
	if ( pcSlash ) *pcSlash = 0;
	sprintf( acHarness, "%s\\..", acExePath );
	GetFullPathName( acHarness, MAX_PATH, acHarness, NULL );
	sprintf( acRoot, "%s\\..\\..", acHarness );
	GetFullPathName( acRoot, MAX_PATH, acRoot, NULL );
	sprintf( acRefs, "%s\\Refs", acHarness );
	sprintf( acResults, "%s\\Out\\Results", acHarness );
	sprintf( acExeData, "%s\\Exe", acRoot );
	sprintf( acIconSheet, "%s\\Out\\UIXTestIcons.png", acHarness );
	CreateDirectory( acResults, NULL );

	UIXTestFindArg( szCmdLine, "--only", acOnly, sizeof( acOnly ) );
	if ( UIXTestFindArg( szCmdLine, "--pixel-tolerance", acValue, sizeof( acValue ) ) ) nPixelTolerance = atoi( acValue );
	if ( UIXTestFindArg( szCmdLine, "--max-diff-pixels", acValue, sizeof( acValue ) ) ) nMaxDiffPixels = atoi( acValue );
	UIXTestSetOptions( UIXTestFindArg( szCmdLine, "--update-refs", NULL, 0 ),
					   UIXTestFindArg( szCmdLine, "--verbose", NULL, 0 ),
					   UIXTestFindArg( szCmdLine, "--list", NULL, 0 ),
					   acOnly, acRefs, acResults, nPixelTolerance, nMaxDiffPixels );

	// Fonts and shaders load relative to Antiquated\Exe, as for the samples
	if ( !SetCurrentDirectory( acExeData ) )
	{
		printf( "UIXTests: FAILED (can't find %s)\n", acExeData );
		ExitProcess( 2 );
	}

	PlatformInit( NULL, FALSE, FALSE );
	hWnd = UIXTestCreateHiddenWindow();
	InterfaceSetWindow( hWnd );
	InterfaceInitDisplayDevice( FALSE, UIXTEST_SCREEN_W, UIXTEST_SCREEN_H );
	InterfaceInit( TRUE );
	if ( InterfaceGetD3DDevice() == NULL )
	{
		printf( "UIXTests: FAILED (no D3D9 device)\n" );
		ExitProcess( 2 );
	}
	if ( ( InterfaceGetWidth() != UIXTEST_SCREEN_W ) ||
		 ( InterfaceGetHeight() != UIXTEST_SCREEN_H ) )
	{
		printf( "UIXTests: FAILED (screen is %dx%d, expected %dx%d)\n", InterfaceGetWidth(), InterfaceGetHeight(), UIXTEST_SCREEN_W, UIXTEST_SCREEN_H );
		ExitProcess( 2 );
	}

	UIInitialise( NULL );
	UIX::Initialise( 0 );
	if ( UIXTestMakeIconSheet( acIconSheet ) )
	{
		UIX::LoadIconSheet( UIXTestInterface(), 0, acIconSheet );
		UIXTestSetIconSheet( UIXTestIconSheetHandle( 0 ) );
	}

	nFailed = UIXTestRunAll();

	UIX::Shutdown();
	UIShutdown();
	InterfaceFree();
	fflush( stdout );
	ExitProcess( ( nFailed > 0 ) ? 1 : 0 );
}

void	MainUpdate( void ) {}
void	MainHighFreqUpdate( void ) {}
void	MainShutdown( void ) {}
void	MainOnPress( int X, int Y ) {}
void	MainOnRelease( int X, int Y ) {}
void	MainOnReleaseRightButton( int X, int Y ) {}
