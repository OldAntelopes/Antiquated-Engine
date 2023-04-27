
#include <stdio.h>
#include "StandardDef.h"
#include "Interface.h"

#include "UI.h"
#include "UIButton.h"

class ButtonStyle
{
public:
	ButtonStyle();

	void	Initialise( int mode, const char* szAssetPath );

	void	NewFrame( void );

	void	Render( int X, int Y, int W, int H, const char* szText, int mode, float fAlpha );

	void	Free( void );

private:
	void	InitOverlays( void );
	
	float	m_fGlobalAlpha;
	int		mahUIButtonTextures[9];
	int		mahUIButtonOverlays[9];
};


ButtonStyle::ButtonStyle()
{
int		nLoop;

	for ( nLoop = 0; nLoop < 9; nLoop++ )
	{
		mahUIButtonOverlays[nLoop] = NOTFOUND;
		mahUIButtonTextures[nLoop] = NOTFOUND;
	}

}

void	ButtonStyle::NewFrame( void )
{
int		nLoop;

	for ( nLoop = 0; nLoop < 9; nLoop++ )
	{
		mahUIButtonOverlays[nLoop] = NOTFOUND;
	}
}


void	ButtonStyle::Free( void )
{
int		nLoop;

	for ( nLoop = 0; nLoop < 9; nLoop++ )
	{
		InterfaceReleaseTexture(mahUIButtonTextures[nLoop] );
	}
}

void	ButtonStyle::Initialise( int mode, const char* szAssetPath )
{
char		acString[256];
FILE*		pFile;
	m_fGlobalAlpha = 1.0f;

	sprintf( acString, "%s\\topleft.bmp", szAssetPath );

	pFile = fopen( acString, "rb");
	if ( pFile )
	{
		fclose( pFile );
		mahUIButtonTextures[0] = InterfaceGetTexture( acString, 0 );
		sprintf( acString, "%s\\top.bmp", szAssetPath );
		mahUIButtonTextures[1] = InterfaceGetTexture( acString, 0 );
		sprintf( acString, "%s\\topright.bmp", szAssetPath );
		mahUIButtonTextures[2] = InterfaceGetTexture( acString, 0 );
		sprintf( acString, "%s\\midleft.bmp", szAssetPath );
		mahUIButtonTextures[3] = InterfaceGetTexture( acString, 0 );
		sprintf( acString, "%s\\mid.bmp", szAssetPath );
		mahUIButtonTextures[4] = InterfaceGetTexture( acString, 0 );
		sprintf( acString, "%s\\midright.bmp", szAssetPath );
		mahUIButtonTextures[5] = InterfaceGetTexture( acString, 0 );
		sprintf( acString, "%s\\botleft.bmp", szAssetPath );
		mahUIButtonTextures[6] = InterfaceGetTexture( acString, 0 );
		sprintf( acString, "%s\\bot.bmp", szAssetPath );
		mahUIButtonTextures[7] = InterfaceGetTexture( acString, 0 );
		sprintf( acString, "%s\\botright.bmp", szAssetPath );
		mahUIButtonTextures[8] = InterfaceGetTexture( acString, 0 );
	}

}

void		ButtonStyle::InitOverlays( void )
{
int		nLoop;

	for ( nLoop = 0; nLoop < 9; nLoop++ )
	{
		mahUIButtonOverlays[nLoop] = InterfaceCreateNewTexturedOverlay(1, mahUIButtonTextures[nLoop] );
	}
}


void	ButtonStyle::Render( int X, int Y, int W, int H, const char* szText, int mode, float fAlpha )
{
int		nButtonImageW = 8;
int		nButtonImageH = 8;
uint32	ulBackgroundCol;
float	fAlphaVal = m_fGlobalAlpha * fAlpha;

	if ( mahUIButtonOverlays[0] == NOTFOUND )
	{ 
		InitOverlays();
	}

	if ( mode == 0 )
	{
		ulBackgroundCol = GetColWithModifiedAlpha( 0xF0FFFFFF, fAlphaVal );
	}
	else
	{
		ulBackgroundCol = GetColWithModifiedAlpha( 0x90FFFFFF, fAlphaVal );
	}

	InterfaceTexturedRect( mahUIButtonOverlays[0], X, Y, nButtonImageW, nButtonImageH, ulBackgroundCol, 0.0f, 0.0f, 1.0f, 1.0f );
	InterfaceTexturedRect( mahUIButtonOverlays[1], X+nButtonImageW, Y, W-(nButtonImageW*2), nButtonImageH, ulBackgroundCol, 0.0f, 0.0f, 1.0f, 1.0f );
	InterfaceTexturedRect( mahUIButtonOverlays[2], X+W-nButtonImageW, Y, nButtonImageW, nButtonImageH, ulBackgroundCol, 0.0f, 0.0f, 1.0f, 1.0f );

	InterfaceTexturedRect( mahUIButtonOverlays[3], X, Y+nButtonImageH, nButtonImageW, H-(nButtonImageH*2), ulBackgroundCol, 0.0f, 0.0f, 1.0f, 1.0f );
	InterfaceTexturedRect( mahUIButtonOverlays[4], X+nButtonImageW, Y+nButtonImageH, W-(nButtonImageW*2), H-(nButtonImageH*2), ulBackgroundCol, 0.0f, 0.0f, 1.0f, 1.0f );
	InterfaceTexturedRect( mahUIButtonOverlays[5], X+W-nButtonImageW, Y+nButtonImageH, nButtonImageW, H-(nButtonImageH*2), ulBackgroundCol, 0.0f, 0.0f, 1.0f, 1.0f );

	InterfaceTexturedRect( mahUIButtonOverlays[6], X, Y+H-nButtonImageH, nButtonImageW, nButtonImageH, ulBackgroundCol, 0.0f, 0.0f, 1.0f, 1.0f );
	InterfaceTexturedRect( mahUIButtonOverlays[7], X+nButtonImageW, Y+H-nButtonImageH, W-(nButtonImageW*2), nButtonImageH, ulBackgroundCol, 0.0f, 0.0f, 1.0f, 1.0f );
	InterfaceTexturedRect( mahUIButtonOverlays[8], X+W-nButtonImageW, Y+H-nButtonImageH, nButtonImageW, nButtonImageH, ulBackgroundCol, 0.0f, 0.0f, 1.0f, 1.0f );

	uint32	ulTextCol = GetColWithModifiedAlpha( 0xD0F0E0C0, fAlphaVal );
	int		nTextH = GetStringHeight( szText, 1 );
	int		nTextY;

	if ( mode != 0 )
	{
		ulTextCol = GetColWithModifiedAlpha( 0x90F0E0C0, fAlphaVal );
	}

	nTextY = Y + ( ( H - nTextH ) / 2 );
	InterfaceSetFontFlags( FONT_FLAG_DROP_SHADOW );
	InterfaceTextCenter( 1, X, X + W, nTextY, szText, ulTextCol, 1 );
	InterfaceSetFontFlags( 0 );

}


//---------------------------------------------------

ButtonStyle			msButtonStyle;

void		UIButtonsInitialise( void )
{
	msButtonStyle.Initialise( 1, "Data\\UI\\Button1" );
}

void		UIButtonsNewFrame( void )
{
	msButtonStyle.NewFrame();
}

void		UIButtonsShutdown( void )
{
	msButtonStyle.Free();
}

void		UIButtonDrawAlpha( int nButtonID, int nX, int nY, int nWidth, int nHeight, const char* szText, int nMode, uint32 ulParam, float fAlpha )
{
#ifdef BASIC_RENDER
int		nTextSize;
int		nTextOffsetY;
#endif
uint32	ulButtonMainCol = 0xB0707070;
uint32	ulTextCol = 0xD0F0F0F0;
BOOL	bEnabled = TRUE;

#ifdef BASIC_RENDER
	nTextSize = 14;
	nTextOffsetY = (nHeight - nTextSize) / 2;

	switch( nMode )
	{
	case 1:
		ulButtonMainCol = 0x60606060;
		ulTextCol = 0x60F0F0F0;
		bEnabled = FALSE;
		break;
	case 2:
		ulButtonMainCol = 0xB0901008;
		ulTextCol = 0xE0F0E080;
		break;
	case 0:
	default:
		break;
	}

	InterfaceRect( 0, nX, nY, nWidth, nHeight, ulButtonMainCol );

	if ( bEnabled )
	{
		InterfaceRect( 0, nX, nY, nWidth, 1, 0x60D0D0D0 );
		InterfaceRect( 0, nX, nY, 1, nHeight, 0x60D0D0D0 );
		InterfaceRect( 0, nX, nY + nHeight, nWidth, 1, 0x60202020 );
		InterfaceRect( 0, nX + nWidth, nY, 1, nHeight, 0x60202020 );
	}

	InterfaceSetFontFlags( FONT_FLAG_DROP_SHADOW );
	InterfaceTextCentre( 1, nX + (nWidth/2), nY + nTextOffsetY, szText, ulTextCol, 0 ); 
	InterfaceSetFontFlags( 0 );
#endif

	if ( nMode != 1 )
	{
		UIHoverItem( nX, nY, nWidth, nHeight );
	}

	msButtonStyle.Render( nX, nY, nWidth, nHeight, szText, nMode, fAlpha );

	if ( nMode != 1 )
	{
		if ( UIIsPressed( nX, nY, nWidth, nHeight ) == TRUE )
		{
			UIPressIDSet( nButtonID, ulParam );
		}
	}

}


void		UIButtonDraw( int nButtonID, int nX, int nY, int nWidth, int nHeight, const char* szText, int nMode, uint32 ulParam )
{
	UIButtonDrawAlpha( nButtonID, nX, nY, nWidth, nHeight, szText, nMode, ulParam, 1.0f );
}


BOOL		UIButtonRegion( int nButtonID, int nX, int nY, int nWidth, int nHeight, uint32 ulParam )
{
	UIHoverItem( nX, nY, nWidth, nHeight );

	if ( UIIsPressed( nX, nY, nWidth, nHeight ) == TRUE )
	{
		UIPressIDSet( nButtonID, ulParam );
		return( TRUE );
	}
	return( FALSE );
}

