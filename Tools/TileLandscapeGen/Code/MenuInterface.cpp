
#include <stdio.h>
#include "StdWinInclude.h"
#include "StandardDef.h"	
#include "Interface.h"		
#include "Engine.h"

#include "Landscape/LandscapeShader.h"
#include "Landscape/SourceTextures.h"
#include "UI\UI.h"
#include "Generator.h"
#include "MenuInterface.h"

typedef struct
{
	char	szConsoleLine[256];
} CONSOLE_LINE;

#define		CONSOLE_NUM_LINES	7
CONSOLE_LINE	maConsoleLines[CONSOLE_NUM_LINES];

char	mszDefaultLevelFolder[256] = "d:\\GameDev\\Games\\Herd\\Exe\\Data\\Levels\\TwoIslesJpeg";
int		manRangeConstants[8] = { 255, 240, 180, 160, 135, 25, 15, 0 };
int		mhHeightmapTexture = NOTFOUND;
int		mnNextConsoleLine = 0;

int		mhShadowsCheckbox = NOTFOUND;
int		mhHiResGenCheckbox = NOTFOUND;
int		mhLoResGenCheckbox = NOTFOUND;
int		mhPathGenCheckbox = NOTFOUND;
int		mhTreeShadowsCheckbox = NOTFOUND;

typedef struct
{
	char	szLevelFolder[256];
	int		nShadowsCheckbox;
	int		nHiResCheckbox;
	int		nLoResCheckbox;
	int		nPathGenCheckbox;
	int		nTreeShadowsCheckbox;

	int		anRangeConstants[8];
	int		nWindowLocationX;
	int		nWindowLocationY;

} LANDSCAPEGEN_SAVE_CONFIG;

LANDSCAPEGEN_SAVE_CONFIG		mxSaveConfig;

void		MenuInterfaceLoadConfig( void )
{
FILE*		pFile;
	
	memset( &mxSaveConfig, 0, sizeof( mxSaveConfig ) );
	strcpy( mxSaveConfig.szLevelFolder, mszDefaultLevelFolder );
	mxSaveConfig.nHiResCheckbox = 0;
	mxSaveConfig.nLoResCheckbox = 1;
	mxSaveConfig.nShadowsCheckbox = 1;
	mxSaveConfig.nPathGenCheckbox = 1;
	mxSaveConfig.nTreeShadowsCheckbox = 1;
	memcpy( &mxSaveConfig.anRangeConstants[0], manRangeConstants, 8 * sizeof(int) );

	pFile = fopen( "tlgsettings.dat", "rb" );
	if ( pFile )
	{
		fread( &mxSaveConfig, sizeof( mxSaveConfig ), 1, pFile );
		fclose( pFile );

//		InterfaceSetWindowPosition( mxSaveConfig.nWindowLocationX, mxSaveConfig.nWindowLocationY );

	}
}

void		MenuInterfaceSaveConfig( void )
{
FILE*		pFile;

	
	pFile = fopen( "tlgsettings.dat", "wb" );
	if ( pFile )
	{
		fwrite( &mxSaveConfig, sizeof( mxSaveConfig ), 1, pFile );
		fclose( pFile );
	}
}

const char*		MenuInterfaceGetLevelFolderPath( void )
{
	return( mxSaveConfig.szLevelFolder );
}

void		MenuInterfaceOnLevelFolderPathChanged( void )
{
char		acHeightmapFilename[256];

	InterfaceReleaseTexture( mhHeightmapTexture );

	sprintf( acHeightmapFilename, "%s\\heightmap.bmp", MenuInterfaceGetLevelFolderPath() );
	mhHeightmapTexture = InterfaceLoadTexture( acHeightmapFilename, 0 );


}

void	MenuInterfaceButtonPressHandler( int nButtonID, uint32 ulParam )
{
	switch( ulParam )
	{
	case 0:		// Start generator
		{
		GENERATOR_CONFIG	xConfig;

			mxSaveConfig.nHiResCheckbox = UICheckboxIsChecked( mhHiResGenCheckbox );
			mxSaveConfig.nLoResCheckbox = UICheckboxIsChecked( mhLoResGenCheckbox );
			mxSaveConfig.nPathGenCheckbox = UICheckboxIsChecked( mhPathGenCheckbox );
			mxSaveConfig.nShadowsCheckbox = UICheckboxIsChecked( mhShadowsCheckbox );
			mxSaveConfig.nTreeShadowsCheckbox = UICheckboxIsChecked( mhTreeShadowsCheckbox );
			
			xConfig.bGenLoRes = mxSaveConfig.nLoResCheckbox;
			xConfig.bGenHiRes = mxSaveConfig.nHiResCheckbox;
			xConfig.bGenShadows = mxSaveConfig.nShadowsCheckbox;
			xConfig.bGenPathfinding = mxSaveConfig.nPathGenCheckbox;
			xConfig.bGenTreeShadows = mxSaveConfig.nTreeShadowsCheckbox;
		
			LandscapeShaderSetRangeConstants( manRangeConstants );
			GeneratorInitPreset( &xConfig );
		}
		break;
	case 1:		// Change level folder
		{
		char	acPreviousPath[256];
			strcpy( acPreviousPath, mxSaveConfig.szLevelFolder );
			mxSaveConfig.szLevelFolder[0] = 0;
			SysBrowseForFolderDialog( "Level folder", "", mxSaveConfig.szLevelFolder, acPreviousPath );

			// If changed
			if ( stricmp( mxSaveConfig.szLevelFolder, acPreviousPath ) != 0 )
			{
				MenuInterfaceOnLevelFolderPathChanged();
			}

		}
		break;
	}

}


void		MenuInterfaceInit( void )
{
char	acHeightmapFilename[256];
FILE*	pFile;

	memset( &maConsoleLines[0], 0, sizeof( maConsoleLines[0] ) * CONSOLE_NUM_LINES );
	MenuInterfaceLoadConfig();

	sprintf( acHeightmapFilename, "%s\\heightmap.bmp", MenuInterfaceGetLevelFolderPath() );
		
	pFile = fopen( acHeightmapFilename, "rb" );

	if ( pFile )
	{
		fclose( pFile );

		mhHeightmapTexture = InterfaceLoadTexture( acHeightmapFilename, 0 );
	}
	else
	{
		MenuInterfacePrint( "WARNING: No Heightmap file: %s", acHeightmapFilename );
	}

	UIRegisterButtonPressHandler( UIBUTTONID_MENUINTERFACE, MenuInterfaceButtonPressHandler );

	mhShadowsCheckbox = UICheckboxCreate( "Generate Shadows", mxSaveConfig.nShadowsCheckbox );
	mhHiResGenCheckbox = UICheckboxCreate( "Generate HiRes", mxSaveConfig.nHiResCheckbox );
	mhLoResGenCheckbox = UICheckboxCreate( "Generate LoRes", mxSaveConfig.nLoResCheckbox );
	mhPathGenCheckbox = UICheckboxCreate( "Generate Pathfinding", mxSaveConfig.nPathGenCheckbox );
	mhTreeShadowsCheckbox = UICheckboxCreate( "Embed Tree Shadows", mxSaveConfig.nTreeShadowsCheckbox );

	MenuInterfacePrint( " *** Tile landscape generator initialised" );
}

void		MenuInterfaceUpdate( float fDelta )
{

}

void		MenuInterfaceShowSourceDetails( int X, int Y, int W, int H, int nIndex )
{
char	acString[256];
int		nTileSize = W;
int		nOverlay;

	sprintf( acString, "%d", nIndex + 1 );
	InterfaceTextRight( 1, X - 5, Y + 10, acString, 0xd0d0d0d0, 1  );
	nOverlay = EngineTextureCreateInterfaceOverlay( 0, SourceTexturesGetTextureHandle(nIndex) );
	InterfaceTexturedRect( nOverlay, X, Y, nTileSize, nTileSize, 0xFFFFFFFF, 0.0f, 0.0f, 1.0f, 1.0f );

	sprintf( acString, "%d", manRangeConstants[(nIndex*2)] );
	InterfaceText( 1, X + W + 5, Y + 5, acString, 0xe0f0f0f0, 3 );

	sprintf( acString, "%d", manRangeConstants[(nIndex*2)+1] );
	InterfaceText( 1, X + W + 5, Y + 30, acString, 0xe0d0d0d0, 3 );
}

void		MenuInterfaceShowSourceTextures( int X, int Y, int W, int H )
{
int		nLineY = Y;
int		nTileSize = W;

	MenuInterfaceShowSourceDetails( X, nLineY, W, H, 0 );
	nLineY += nTileSize + 2;

	MenuInterfaceShowSourceDetails( X, nLineY, W, H, 1 );
	nLineY += nTileSize + 2;

	MenuInterfaceShowSourceDetails( X, nLineY, W, H, 2 );
	nLineY += nTileSize + 2;

	MenuInterfaceShowSourceDetails( X, nLineY, W, H, 3 );
	nLineY += nTileSize + 2;


}

void	MenuInterfaceRenderLog( int X, int Y, int W, int H )
{
int		nLineY;
int		nNextLineToDraw;
int		nCount = CONSOLE_NUM_LINES;

	InterfaceRect( 0, X, Y, W, H, 0xFFC0C0C0 );
	InterfaceOutlineBox( 0, X, Y, W, H, 0xC0202020 );

	nLineY = Y + H - 16;

	nNextLineToDraw = mnNextConsoleLine - 1;
	if ( nNextLineToDraw < 0 ) nNextLineToDraw = CONSOLE_NUM_LINES - 1;

	while( nCount > 0 )
	{
		InterfaceText( 1, X + 5, nLineY, maConsoleLines[nNextLineToDraw].szConsoleLine, 0xD0303030, 3 );
		nNextLineToDraw--;
		if ( nNextLineToDraw < 0 ) nNextLineToDraw = CONSOLE_NUM_LINES - 1;
		nLineY -= 16;
		nCount--;
	}
}

void		MenuInterfaceRender( void )
{
int		nButtonX;
int		nButtonY;
int		nButtonW;
int		nButtonH;
//char	acString[256];
char	acWorkingDir[256];
int		nLineY = 35;
int		nTabX = 130;

	EngineDefaultState();

	InterfaceNewFrame( 0xFFA0A0A0 );

	InterfaceBeginRender();

	InterfaceTextCenter( 0, 0, InterfaceGetWidth(), 5, "Tile Landscape Generator", 0xd0404040, 1 );

	MenuInterfaceShowSourceTextures( InterfaceGetWidth() - 90, 70, 48, 48 );

	SysGetCurrentDir( 256, acWorkingDir );

	UIButtonDraw( UIBUTTONID_MENUINTERFACE, 10, nLineY - 5, 110, 25, "Level Folder:", 0, 1 );

//	InterfaceTextRight( 1, nTabX, nLineY, "Level Folder: ", 0xf0C0C0C0, 0 );
	InterfaceText( 1, nTabX, nLineY, MenuInterfaceGetLevelFolderPath(), 0xf0f0f0f0, 0 );
	nLineY += 25;

	if ( mhHeightmapTexture != NOTFOUND )
	{
	int		nOverlay = InterfaceCreateNewTexturedOverlay( 0, mhHeightmapTexture );
		InterfaceTexturedRect( nOverlay, nTabX - 100, nLineY, 200, 200, 0xffffffff, 0.0f, 0.0f, 1.0f, 1.0f );
	}

	MenuInterfaceRenderLog( 10, InterfaceGetHeight() - 122, InterfaceGetWidth() - 20, 114 );

	if ( GeneratorIsActive() == TRUE )
	{
		GeneratorDisplay( nTabX + 120, nLineY, 200, 200 );
	}
	else
	{
		nTabX = nTabX + 160;

		UICheckboxRender( mhShadowsCheckbox, nTabX, nLineY, 200, 22 );
		nLineY += 30;
		UICheckboxRender( mhLoResGenCheckbox, nTabX, nLineY, 200, 22 );
		nLineY += 30;
		UICheckboxRender( mhHiResGenCheckbox, nTabX, nLineY, 200, 22 );
		nLineY += 30;
		UICheckboxRender( mhPathGenCheckbox, nTabX, nLineY, 200, 22 );
		nLineY += 30;
		UICheckboxRender( mhTreeShadowsCheckbox, nTabX, nLineY, 200, 22 );
		nLineY += 30;
			

		nButtonX = nTabX - 25;
		nButtonY = InterfaceGetHeight() - 180;
		nButtonW = 300;
		nButtonH = 50;
	
		UIButtonDraw( UIBUTTONID_MENUINTERFACE, nButtonX, nButtonY, nButtonW, nButtonH, "Start Generator", 0, 0 );
	}
	
	InterfaceDraw();

	InterfaceEndRender();
	InterfacePresent();


}

void		MenuInterfaceShutdown( void )
{
	MenuInterfaceSaveConfig();
}

void		MenuInterfaceProcessRender( void )
{

}

void		MenuInterfacePrint( const char* text, ... )
{
char		acString[1024];
va_list		marker;
//uint32*		pArgs;
int			nLen;

//	pArgs = (uint32*)( &text ) + 1;

	va_start( marker, text );     
	vsprintf( acString, text, marker );

	nLen = strlen( acString );
	if ( nLen > 0)
	{
		strcpy( maConsoleLines[mnNextConsoleLine].szConsoleLine, acString );
		mnNextConsoleLine++;
		mnNextConsoleLine %= CONSOLE_NUM_LINES;
//		mfnSysUserPrintFunction( mode, acString );
	}

}
