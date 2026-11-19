//------------------------------------------------------------------------------------------------
// UIXTestCore.cpp
//
// The harness framework: test registry, the frame loop, scripted input, traces and reference images.
// UIXTestMain.cpp sets up the device and calls UIXTestRunAll.
//------------------------------------------------------------------------------------------------

#include "UIXTestCore.h"
#include "DirectX/d3dx9.h"

// State inside PlatformWin32.cpp that the harness drives directly, so frames are deterministic
extern float		mfFrameDelta;
// UIXMenu.cpp's record of the open top-level menu
extern UIXMenuItem*	mspExpandedMenuTopLayer;
// UI.cpp's glow timer (UIGetGlowColour), which UIUpdate advances every frame
extern float		msfGlowSin;

#define		UIXTEST_FRAME_DELTA			(1.0f / 60.0f)
#define		UIXTEST_CLEAR_COLOUR		0xFF182028
#define		UIXTEST_BUTTONID_BASE		0x7000
#define		UIXTEST_NUM_BUTTONIDS		16

struct UIXTestEntry
{
	const char*		szName;
	const char*		szFile;
	fnUIXTest		fnTest;
};

struct UIXTestOptions
{
	BOOL			bUpdateRefs;
	BOOL			bVerbose;
	BOOL			bList;
	char			acOnly[128];
	char			acRefsFolder[MAX_PATH];
	char			acResultsFolder[MAX_PATH];
	int				nPixelTolerance;
	int				nMaxDiffPixels;
};

static std::vector<UIXTestEntry>&	UIXTestList( void )
{
static std::vector<UIXTestEntry>	sxList;

	return( sxList );
}

static UIXTestOptions			msxOptions;
static const char*				mszCurrentTest = NULL;
static std::vector<std::string>	msTrace;
static int						msnTestFailures = 0;
static int						msnTotalChecks = 0;
static int						msnTotalImages = 0;
static int						msnTotalRefsUpdated = 0;
static BOOL						msbCaptureNextFrame = FALSE;
static std::vector<uint32>		msCapturedPixels;
static int						msnCaptureW = 0;
static int						msnCaptureH = 0;
static uint32					msulTick = 1000;
static int						msnCursorX = -100;
static int						msnCursorY = -100;
static BOOL						msbLeftDown = FALSE;
static BOOL						msbRightDown = FALSE;
static int						mshIconSheet = NOTFOUND;

//------------------------------------------------------------------------------------------------
UIXTestRegistration::UIXTestRegistration( const char* szName, const char* szFile, fnUIXTest fnTest )
{
UIXTestEntry	xEntry;
const char*		pcSlash = strrchr( szFile, '\\' );

	xEntry.szName = szName;
	xEntry.szFile = pcSlash ? pcSlash + 1 : szFile;
	xEntry.fnTest = fnTest;
	UIXTestList().push_back( xEntry );
}

//------------------------------------------------------------------------------------------------
// Results
//------------------------------------------------------------------------------------------------
void		UIXTestCheck( BOOL bCondition, const char* szDescription, const char* szFile, int nLine )
{
const char*		pcSlash = strrchr( szFile, '\\' );

	msnTotalChecks++;
	if ( !bCondition )
	{
		msnTestFailures++;
		printf( "  FAIL %s: %s (%s:%d)\n", mszCurrentTest, szDescription, pcSlash ? pcSlash + 1 : szFile, nLine );
	}
	else if ( msxOptions.bVerbose )
	{
		printf( "  ok   %s\n", szDescription );
	}
}

void		UIXTestLog( const char* szFormat, ... )
{
char		acLine[1024];
va_list		marker;

	va_start( marker, szFormat );
	vsnprintf( acLine, sizeof( acLine ) - 1, szFormat, marker );
	va_end( marker );
	acLine[sizeof( acLine ) - 1] = 0;
	msTrace.push_back( acLine );
}

InterfaceInstance*	UIXTestInterface( void )
{
	return( InterfaceInstanceMain() );
}

int		UIXTestIconSheet( void )
{
	return( mshIconSheet );
}

const char*	UIXTestTypeName( UIXObject* pObject )
{
const char*		pcName = typeid( *pObject ).name();

	if ( strncmp( pcName, "class ", 6 ) == 0 )
	{
		pcName += 6;
	}
	return( pcName );
}

UIXRECT		UIXTestRectOf( UIXObject* pObject )
{
	return( UIXTestActualRect( pObject ) );
}

static int	UIXTestFindIndex( UIXObject* pNode, UIXObject* pTarget, int* pnCounter )
{
int		nFound;

	if ( pNode == pTarget )
	{
		return( *pnCounter );
	}
	(*pnCounter)++;
	for ( UIXObject* pChild : UIXTestChildren( pNode ) )
	{
		nFound = UIXTestFindIndex( pChild, pTarget, pnCounter );
		if ( nFound != NOTFOUND )
		{
			return( nFound );
		}
	}
	return( NOTFOUND );
}

int		UIXTestIndexOf( UIXObject* pObject )
{
int		nCounter = 0;
int		nFound;

	if ( pObject == NULL )
	{
		return( NOTFOUND );
	}
	for ( UIXPage* pPage : UIXTestPages() )
	{
		nFound = UIXTestFindIndex( pPage, pObject, &nCounter );
		if ( nFound != NOTFOUND )
		{
			return( nFound );
		}
	}
	return( NOTFOUND );
}

//------------------------------------------------------------------------------------------------
// Layout dump: renders one frame with every object marked "not drawn", then lists the tree with the rect
// each object drew at. The rect is GetActualRenderRect of the display rect it was last given, which is
// where UIX itself does hit tests and drawing.
//------------------------------------------------------------------------------------------------
static const int	knUnrenderedMarker = -31999;

static void	UIXTestMarkUnrendered( UIXObject* pNode )
{
	UIXTestSetLastDisplayRect( pNode, UIXRECT( knUnrenderedMarker, knUnrenderedMarker, 0, 0 ) );
	for ( UIXObject* pChild : UIXTestChildren( pNode ) )
	{
		UIXTestMarkUnrendered( pChild );
	}
}

static void	UIXTestDumpNode( UIXObject* pNode, int nDepth, int* pnCounter )
{
char		acIndent[64];
int			nLoop;
UIXRECT		xRect;

	for ( nLoop = 0; ( nLoop < nDepth ) && ( nLoop < 30 ); nLoop++ )
	{
		acIndent[nLoop * 2] = ' ';
		acIndent[nLoop * 2 + 1] = ' ';
	}
	acIndent[nLoop * 2] = 0;

	if ( UIXTestLastDisplayRect( pNode ).x == knUnrenderedMarker )
	{
		UIXTestLog( "  #%d %s%s (not drawn)", *pnCounter, acIndent, UIXTestTypeName( pNode ) );
	}
	else
	{
		xRect = UIXTestRectOf( pNode );
		UIXTestLog( "  #%d %s%s at %d,%d size %d,%d contents h %d", *pnCounter, acIndent, UIXTestTypeName( pNode ), xRect.x, xRect.y, xRect.w, xRect.h, UIXTestChildContentsHeight( pNode ) );
	}
	(*pnCounter)++;
	for ( UIXObject* pChild : UIXTestChildren( pNode ) )
	{
		UIXTestDumpNode( pChild, nDepth + 1, pnCounter );
	}
}

void		UIXTestLogLayout( UIXObject* pRoot, const char* szLabel )
{
int		nCounter = 0;

	for ( UIXPage* pPage : UIXTestPages() )
	{
		UIXTestMarkUnrendered( pPage );
	}
	UIXTestFrame( 1 );

	UIXTestLog( "layout %s:", szLabel );
	if ( pRoot == NULL )
	{
		for ( UIXPage* pPage : UIXTestPages() )
		{
			UIXTestDumpNode( pPage, 0, &nCounter );
		}
	}
	else
	{
		nCounter = UIXTestIndexOf( pRoot );
		UIXTestDumpNode( pRoot, 0, &nCounter );
	}
}

//------------------------------------------------------------------------------------------------
// Frames
//------------------------------------------------------------------------------------------------
static void	UIXTestCaptureBackBuffer( void )
{
LPDIRECT3DDEVICE9		pDevice = (LPDIRECT3DDEVICE9)InterfaceGetD3DDevice();
LPDIRECT3DSURFACE9		pBackBuffer = NULL;
LPDIRECT3DSURFACE9		pSysMem = NULL;
D3DSURFACE_DESC			xDesc;
D3DLOCKED_RECT			xLocked;
int						nX, nY;

	msCapturedPixels.clear();
	msnCaptureW = 0;
	msnCaptureH = 0;
	if ( FAILED( pDevice->GetRenderTarget( 0, &pBackBuffer ) ) )
	{
		return;
	}
	pBackBuffer->GetDesc( &xDesc );
	if ( SUCCEEDED( pDevice->CreateOffscreenPlainSurface( xDesc.Width, xDesc.Height, xDesc.Format, D3DPOOL_SYSTEMMEM, &pSysMem, NULL ) ) )
	{
		if ( SUCCEEDED( pDevice->GetRenderTargetData( pBackBuffer, pSysMem ) ) )
		{
			if ( SUCCEEDED( pSysMem->LockRect( &xLocked, NULL, D3DLOCK_READONLY ) ) )
			{
				msnCaptureW = (int)xDesc.Width;
				msnCaptureH = (int)xDesc.Height;
				msCapturedPixels.resize( msnCaptureW * msnCaptureH );
				for ( nY = 0; nY < msnCaptureH; nY++ )
				{
				const uint32*	pulRow = (const uint32*)( (const BYTE*)xLocked.pBits + ( nY * xLocked.Pitch ) );

					for ( nX = 0; nX < msnCaptureW; nX++ )
					{
						// The back buffer is X8R8G8B8: its alpha means nothing, so make it opaque
						msCapturedPixels[nY * msnCaptureW + nX] = pulRow[nX] | 0xFF000000;
					}
				}
				pSysMem->UnlockRect();
			}
		}
		pSysMem->Release();
	}
	pBackBuffer->Release();
}

void		UIXTestFrame( int nFrames )
{
int		nLoop;

	for ( nLoop = 0; nLoop < nFrames; nLoop++ )
	{
		mfFrameDelta = UIXTEST_FRAME_DELTA;
		msulTick += 16;
		SysSetFrameDelta( UIXTEST_FRAME_DELTA, msulTick );

		UISetCurrentCursorPosition( msnCursorX, msnCursorY );
		UIUpdate( UIXTEST_FRAME_DELTA );
		UIX::Update( UIXTEST_FRAME_DELTA );

		InterfaceNewFrame( UIXTEST_CLEAR_COLOUR );
		InterfaceBeginRender();
		UIX::Render( UIXTestInterface() );
		InterfaceDraw();
		UIOnInterfaceDraw();
		InterfaceEndRender();

		if ( ( msbCaptureNextFrame ) &&
			 ( nLoop == nFrames - 1 ) )
		{
			UIXTestCaptureBackBuffer();
			msbCaptureNextFrame = FALSE;
		}
		InterfacePresent();
	}
}

//------------------------------------------------------------------------------------------------
// Input. The order matches a Win32 app: messages arrive between frames, and UIX decides what was pressed
// while it renders. So a click is move, frame, press, frame, release, frame.
//------------------------------------------------------------------------------------------------
void		UIXTestMouseMove( int nX, int nY )
{
	msnCursorX = nX;
	msnCursorY = nY;
	UISetCurrentCursorPosition( nX, nY );
	PlatformSetCurrentCursorPosition( nX, nY );
}

void		UIXTestMouseDown( void )
{
	msbLeftDown = TRUE;
	UIOnPress( msnCursorX, msnCursorY );
}

void		UIXTestMouseUp( void )
{
	msbLeftDown = FALSE;
	UIOnRelease( msnCursorX, msnCursorY );
}

void		UIXTestRightDown( void )
{
	msbRightDown = TRUE;
	UIOnRightButtonPress( msnCursorX, msnCursorY );
}

void		UIXTestRightUp( void )
{
	msbRightDown = FALSE;
	UIOnReleaseRightButton( msnCursorX, msnCursorY );
}

void		UIXTestClick( int nX, int nY )
{
	UIXTestMouseMove( nX, nY );
	UIXTestFrame( 1 );
	UIXTestMouseDown();
	UIXTestFrame( 1 );
	UIXTestMouseUp();
	UIXTestFrame( 1 );
}

void		UIXTestRightClick( int nX, int nY )
{
	UIXTestMouseMove( nX, nY );
	UIXTestFrame( 1 );
	UIXTestRightDown();
	UIXTestFrame( 1 );
	UIXTestRightUp();
	UIXTestFrame( 1 );
}

void		UIXTestDrag( int nFromX, int nFromY, int nToX, int nToY, int nSteps )
{
int		nStep;

	if ( nSteps < 1 )
	{
		nSteps = 1;
	}
	UIXTestMouseMove( nFromX, nFromY );
	UIXTestFrame( 1 );
	UIXTestMouseDown();
	UIXTestFrame( 1 );
	for ( nStep = 1; nStep <= nSteps; nStep++ )
	{
		UIXTestMouseMove( nFromX + ( ( nToX - nFromX ) * nStep ) / nSteps, nFromY + ( ( nToY - nFromY ) * nStep ) / nSteps );
		UIXTestFrame( 1 );
	}
	UIXTestMouseUp();
	UIXTestFrame( 1 );
}

void		UIXTestWheel( int nNotches )
{
float	fOffset;

	// As Elemental's control panel: WM_MOUSEWHEEL delta / 8, negated
	fOffset = (float)( nNotches * 120 ) / 8.0f;
	UIX::OnMouseWheel( 0.0f - fOffset );
	UIXTestFrame( 1 );
}

void		UIXTestKey( int nKeyCode )
{
	PlatformKeyboardHandleSpecialKeyDown( nKeyCode );
	UIX::OnKeyUp( nKeyCode );
	PlatformKeyboardHandleSpecialKeyUp( nKeyCode );
	UIXTestFrame( 1 );
}

void		UIXTestType( const char* szText )
{
	while ( *szText )
	{
		PlatformKeyboardHandleChar( (short)*szText );
		szText++;
	}
	UIXTestFrame( 1 );
}

//------------------------------------------------------------------------------------------------
// Button IDs whose presses go into the trace
//------------------------------------------------------------------------------------------------
static int		msnNextButtonID = UIXTEST_BUTTONID_BASE;

void		UIXTestLogPress( int nButtonID, uint32 ulParam, uint32 ulIDParam )
{
	UIXTestLog( "button press id=0x%X param=%u object=#%d", nButtonID, ulParam, UIXTestIndexOf( UIX::FindUIXObjectByID( ulIDParam ) ) );
}

int		UIXTestRegisterIDHandler( void )
{
int		nButtonID = msnNextButtonID;

	msnNextButtonID++;
	if ( msnNextButtonID >= UIXTEST_BUTTONID_BASE + UIXTEST_NUM_BUTTONIDS )
	{
		msnNextButtonID = UIXTEST_BUTTONID_BASE;
	}
	return( nButtonID );
}

static void	UIXTestRegisterButtonHandlers( void )
{
int		nLoop;

	for ( nLoop = 0; nLoop < UIXTEST_NUM_BUTTONIDS; nLoop++ )
	{
		UIRegisterButtonPressHandler( UIXTEST_BUTTONID_BASE + nLoop, UIXTestLogPress );
	}
}

//------------------------------------------------------------------------------------------------
// Images
//------------------------------------------------------------------------------------------------
static BOOL	UIXTestSavePNG( const char* szFilename, const std::vector<uint32>& pixels, int nW, int nH )
{
LPDIRECT3DDEVICE9		pDevice = (LPDIRECT3DDEVICE9)InterfaceGetD3DDevice();
LPDIRECT3DSURFACE9		pSurface = NULL;
D3DLOCKED_RECT			xLocked;
int						nY;
BOOL					bOK = FALSE;

	if ( SUCCEEDED( pDevice->CreateOffscreenPlainSurface( nW, nH, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &pSurface, NULL ) ) )
	{
		if ( SUCCEEDED( pSurface->LockRect( &xLocked, NULL, 0 ) ) )
		{
			for ( nY = 0; nY < nH; nY++ )
			{
				memcpy( (BYTE*)xLocked.pBits + ( nY * xLocked.Pitch ), &pixels[nY * nW], nW * 4 );
			}
			pSurface->UnlockRect();
			bOK = SUCCEEDED( D3DXSaveSurfaceToFile( szFilename, D3DXIFF_PNG, pSurface, NULL, NULL ) );
		}
		pSurface->Release();
	}
	return( bOK );
}

static BOOL	UIXTestLoadPNG( const char* szFilename, std::vector<uint32>& pixels, int* pnW, int* pnH )
{
LPDIRECT3DDEVICE9		pDevice = (LPDIRECT3DDEVICE9)InterfaceGetD3DDevice();
LPDIRECT3DSURFACE9		pSurface = NULL;
D3DXIMAGE_INFO			xInfo;
D3DLOCKED_RECT			xLocked;
int						nY;
BOOL					bOK = FALSE;

	if ( FAILED( D3DXGetImageInfoFromFile( szFilename, &xInfo ) ) )
	{
		return( FALSE );
	}
	if ( SUCCEEDED( pDevice->CreateOffscreenPlainSurface( xInfo.Width, xInfo.Height, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &pSurface, NULL ) ) )
	{
		if ( SUCCEEDED( D3DXLoadSurfaceFromFile( pSurface, NULL, NULL, szFilename, NULL, D3DX_FILTER_NONE, 0, NULL ) ) )
		{
			if ( SUCCEEDED( pSurface->LockRect( &xLocked, NULL, D3DLOCK_READONLY ) ) )
			{
				*pnW = (int)xInfo.Width;
				*pnH = (int)xInfo.Height;
				pixels.resize( xInfo.Width * xInfo.Height );
				for ( nY = 0; nY < (int)xInfo.Height; nY++ )
				{
					memcpy( &pixels[nY * xInfo.Width], (BYTE*)xLocked.pBits + ( nY * xLocked.Pitch ), xInfo.Width * 4 );
				}
				pSurface->UnlockRect();
				bOK = TRUE;
			}
		}
		pSurface->Release();
	}
	return( bOK );
}

static int	UIXTestChannelDiff( uint32 ulA, uint32 ulB )
{
int		nMax = 0;
int		nShift;
int		nDiff;

	for ( nShift = 0; nShift < 24; nShift += 8 )
	{
		nDiff = abs( (int)( ( ulA >> nShift ) & 0xFF ) - (int)( ( ulB >> nShift ) & 0xFF ) );
		if ( nDiff > nMax )
		{
			nMax = nDiff;
		}
	}
	return( nMax );
}

void		UIXTestSnapshot( const char* szName, int nX, int nY, int nW, int nH )
{
std::vector<uint32>		crop;
std::vector<uint32>		refPixels;
char					acRefFile[MAX_PATH];
char					acOutFile[MAX_PATH];
char					acDescription[256];
int						nRefW = 0;
int						nRefH = 0;
int						nLoopX, nLoopY;
int						nDiffCount = 0;
int						nMaxDiff = 0;

	msbCaptureNextFrame = TRUE;
	UIXTestFrame( 1 );
	msnTotalImages++;

	if ( msnCaptureW == 0 )
	{
		UIX_CHECK( FALSE, "Snapshot: could not read the back buffer" );
		return;
	}
	if ( ( nW <= 0 ) || ( nH <= 0 ) )
	{
		nX = 0;
		nY = 0;
		nW = msnCaptureW;
		nH = msnCaptureH;
	}
	if ( nX < 0 ) nX = 0;
	if ( nY < 0 ) nY = 0;
	if ( nX + nW > msnCaptureW ) nW = msnCaptureW - nX;
	if ( nY + nH > msnCaptureH ) nH = msnCaptureH - nY;

	crop.resize( nW * nH );
	for ( nLoopY = 0; nLoopY < nH; nLoopY++ )
	{
		memcpy( &crop[nLoopY * nW], &msCapturedPixels[( nY + nLoopY ) * msnCaptureW + nX], nW * 4 );
	}

	sprintf( acRefFile, "%s\\%s_%s.png", msxOptions.acRefsFolder, mszCurrentTest, szName );
	sprintf( acOutFile, "%s\\%s_%s.png", msxOptions.acResultsFolder, mszCurrentTest, szName );
	UIXTestSavePNG( acOutFile, crop, nW, nH );

	if ( msxOptions.bUpdateRefs )
	{
		UIXTestSavePNG( acRefFile, crop, nW, nH );
		msnTotalRefsUpdated++;
		return;
	}

	if ( !UIXTestLoadPNG( acRefFile, refPixels, &nRefW, &nRefH ) )
	{
		sprintf( acDescription, "Snapshot %s: no reference image (run with --update-refs to make one)", szName );
		UIX_CHECK( FALSE, acDescription );
		return;
	}
	if ( ( nRefW != nW ) || ( nRefH != nH ) )
	{
		sprintf( acDescription, "Snapshot %s: reference is %dx%d, frame crop is %dx%d", szName, nRefW, nRefH, nW, nH );
		UIX_CHECK( FALSE, acDescription );
		return;
	}

	for ( nLoopY = 0; nLoopY < nH * nW; nLoopY++ )
	{
	int		nDiff = UIXTestChannelDiff( crop[nLoopY], refPixels[nLoopY] );

		if ( nDiff > nMaxDiff ) nMaxDiff = nDiff;
		if ( nDiff > msxOptions.nPixelTolerance )
		{
			nDiffCount++;
		}
	}

	if ( nDiffCount > msxOptions.nMaxDiffPixels )
	{
	std::vector<uint32>		diffPixels( nW * nH );

		for ( nLoopY = 0; nLoopY < nH; nLoopY++ )
		{
			for ( nLoopX = 0; nLoopX < nW; nLoopX++ )
			{
			int		nIndex = nLoopY * nW + nLoopX;

				if ( UIXTestChannelDiff( crop[nIndex], refPixels[nIndex] ) > msxOptions.nPixelTolerance )
				{
					diffPixels[nIndex] = 0xFFFF0000;
				}
				else
				{
					diffPixels[nIndex] = 0xFF000000 | ( ( crop[nIndex] >> 2 ) & 0x003F3F3F );
				}
			}
		}
		sprintf( acOutFile, "%s\\%s_%s_diff.png", msxOptions.acResultsFolder, mszCurrentTest, szName );
		UIXTestSavePNG( acOutFile, diffPixels, nW, nH );
		sprintf( acDescription, "Snapshot %s: %d pixels differ from the reference (largest channel difference %d); see Out\\Results", szName, nDiffCount, nMaxDiff );
		UIX_CHECK( FALSE, acDescription );
	}
	else
	{
		UIX_CHECK( TRUE, "Snapshot matches" );
	}
}

//------------------------------------------------------------------------------------------------
// Traces
//------------------------------------------------------------------------------------------------
static void	UIXTestCompareTrace( void )
{
char					acRefFile[MAX_PATH];
char					acOutFile[MAX_PATH];
char					acLine[1024];
char					acDescription[1400];
std::vector<std::string>	refLines;
FILE*					pFile;
size_t					nLoop;

	if ( msTrace.empty() )
	{
		return;
	}
	sprintf( acRefFile, "%s\\%s.txt", msxOptions.acRefsFolder, mszCurrentTest );
	sprintf( acOutFile, "%s\\%s.txt", msxOptions.acResultsFolder, mszCurrentTest );

	pFile = fopen( msxOptions.bUpdateRefs ? acRefFile : acOutFile, "wb" );
	if ( pFile )
	{
		for ( nLoop = 0; nLoop < msTrace.size(); nLoop++ )
		{
			fprintf( pFile, "%s\r\n", msTrace[nLoop].c_str() );
		}
		fclose( pFile );
	}
	if ( msxOptions.bUpdateRefs )
	{
		msnTotalRefsUpdated++;
		return;
	}

	pFile = fopen( acRefFile, "rb" );
	if ( pFile == NULL )
	{
		UIX_CHECK( FALSE, "Trace: no reference trace (run with --update-refs to make one)" );
		return;
	}
	while ( fgets( acLine, sizeof( acLine ), pFile ) )
	{
	size_t		nLen = strlen( acLine );

		while ( ( nLen > 0 ) && ( ( acLine[nLen - 1] == '\n' ) || ( acLine[nLen - 1] == '\r' ) ) )
		{
			acLine[--nLen] = 0;
		}
		refLines.push_back( acLine );
	}
	fclose( pFile );

	for ( nLoop = 0; ( nLoop < msTrace.size() ) || ( nLoop < refLines.size() ); nLoop++ )
	{
	const char*		pcGot = ( nLoop < msTrace.size() ) ? msTrace[nLoop].c_str() : "(end of trace)";
	const char*		pcWanted = ( nLoop < refLines.size() ) ? refLines[nLoop].c_str() : "(end of trace)";

		if ( strcmp( pcGot, pcWanted ) != 0 )
		{
			snprintf( acDescription, sizeof( acDescription ) - 1, "Trace line %d differs. Reference: \"%s\"  Now: \"%s\"", (int)nLoop + 1, pcWanted, pcGot );
			acDescription[sizeof( acDescription ) - 1] = 0;
			UIX_CHECK( FALSE, acDescription );
			return;
		}
	}
	UIX_CHECK( TRUE, "Trace matches" );
}

//------------------------------------------------------------------------------------------------
// Between tests: every page gone, UIX's statics and the platform keyboard back to how an app starts
//------------------------------------------------------------------------------------------------
static void	UIXTestResetGlobals( void )
{
	if ( msbLeftDown )
	{
		UIXTestMouseUp();
	}
	if ( msbRightDown )
	{
		UIXTestRightUp();
	}
	UIXTestMouseMove( -100, -100 );
	UIXTestFrame( 1 );

	UIX::Shutdown();
	UIXTestResetUIXStatics();
	mspExpandedMenuTopLayer = NULL;
	msfGlowSin = 0.0f;

	PlatformKeyboardRegisterHandler( NULL, NULL );
	PlatformKeyboardSetInputString( "" );

	// Interface leaves some device state (e.g. whether textured overlays blend with alpha) as the last draw
	// set it, so a test's pixels could depend on the test before it. Draw the same mix of things before
	// every test so each one starts from the same state.
	{
	UIXObject*		pPrimer = UIX::AddPage( UIXRECT( 0, 0, 200, 60 ), "Primer" );

		UIX::AddShape( pPrimer, UIXRECT( 0, 0, 200, 60 ), UIXSHAPE_SHADEDRECT, FALSE, 0xFF000000, 0xFF000000 );
		UIX::AddText( pPrimer, UIXRECT( 4, 4, 150, 16 ), 0xFFFFFFFF, 3, NONE, "Primer" );
		UIX::AddButton( pPrimer, UIXRECT( 4, 24, 120, 20 ), UIXBUTTON_NORMAL, "Primer", 0, 0, FALSE );
		UIX::AddButton( pPrimer, UIXRECT( 130, 24, 24, 24 ), UIXBUTTON_RECT_ICON, "", 0, 0, FALSE, 0xFF303030, 1 );
		UIXTestFrame( 1 );
		UIX::DeleteObject( pPrimer );
	}
	UIXTestFrame( 1 );
}

//------------------------------------------------------------------------------------------------
// Running
//------------------------------------------------------------------------------------------------
void		UIXTestSetIconSheet( int hTexture )
{
	mshIconSheet = hTexture;
}

void		UIXTestSetOptions( BOOL bUpdateRefs, BOOL bVerbose, BOOL bList, const char* szOnly, const char* szRefs, const char* szResults, int nPixelTolerance, int nMaxDiffPixels )
{
	msxOptions.bUpdateRefs = bUpdateRefs;
	msxOptions.bVerbose = bVerbose;
	msxOptions.bList = bList;
	strncpy( msxOptions.acOnly, szOnly ? szOnly : "", sizeof( msxOptions.acOnly ) - 1 );
	strncpy( msxOptions.acRefsFolder, szRefs, MAX_PATH - 1 );
	strncpy( msxOptions.acResultsFolder, szResults, MAX_PATH - 1 );
	msxOptions.nPixelTolerance = nPixelTolerance;
	msxOptions.nMaxDiffPixels = nMaxDiffPixels;
}

// Returns the number of failed tests
int			UIXTestRunAll( void )
{
std::vector<UIXTestEntry>&	tests = UIXTestList();
size_t		nLoop;
int			nRun = 0;
int			nFailedTests = 0;

	UIXTestRegisterButtonHandlers();

	for ( nLoop = 0; nLoop < tests.size(); nLoop++ )
	{
	int		nFailuresBefore;

		if ( ( msxOptions.acOnly[0] != 0 ) &&
			 ( strstr( tests[nLoop].szName, msxOptions.acOnly ) == NULL ) )
		{
			continue;
		}
		if ( msxOptions.bList )
		{
			printf( "%s (%s)\n", tests[nLoop].szName, tests[nLoop].szFile );
			continue;
		}

		mszCurrentTest = tests[nLoop].szName;
		msTrace.clear();
		nFailuresBefore = msnTestFailures;
		msnTestFailures = 0;

		UIXTestResetGlobals();
		msnNextButtonID = UIXTEST_BUTTONID_BASE;
		tests[nLoop].fnTest();
		UIXTestCompareTrace();
		nRun++;

		if ( msnTestFailures > 0 )
		{
			nFailedTests++;
			printf( "FAIL %s\n", mszCurrentTest );
		}
		else if ( msxOptions.bVerbose )
		{
			printf( "pass %s\n", mszCurrentTest );
		}
		msnTestFailures += nFailuresBefore;
	}
	UIXTestResetGlobals();

	if ( !msxOptions.bList )
	{
		if ( nRun == 0 )
		{
			printf( "UIXTests: FAILED (no tests matched)\n" );
			return( 1 );
		}
		else if ( ( msxOptions.bUpdateRefs ) && ( nFailedTests == 0 ) )
		{
			printf( "UIXTests: REFS UPDATED (%d tests, %d reference files written, %d checks passed). Run again without --update-refs to confirm they're stable\n", nRun, msnTotalRefsUpdated, msnTotalChecks );
		}
		else if ( nFailedTests == 0 )
		{
			printf( "UIXTests: ALL PASSED (%d tests, %d checks, %d images)\n", nRun, msnTotalChecks, msnTotalImages );
		}
		else
		{
			printf( "UIXTests: FAILED (%d of %d tests)\n", nFailedTests, nRun );
		}
	}
	return( nFailedTests );
}
