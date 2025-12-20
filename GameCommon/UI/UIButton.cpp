
#include <stdio.h>
#include "StandardDef.h"
#include "InterfaceEx.h"

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
		UIInterfaceInstance()->ReleaseTexture(mahUIButtonTextures[nLoop] );
	}
}

void	ButtonStyle::Initialise( int mode, const char* szAssetPath )
{
InterfaceInstance* pInterface = UIInterfaceInstance();
char		acString[256];
FILE*		pFile;
	m_fGlobalAlpha = 1.0f;

	sprintf( acString, "%s\\topleft.bmp", szAssetPath );

	pFile = fopen( acString, "rb");
	if ( pFile )
	{
		fclose( pFile );
		mahUIButtonTextures[0] = pInterface->GetTexture( acString, 0 );
		sprintf( acString, "%s\\top.bmp", szAssetPath );
		mahUIButtonTextures[1] = pInterface->GetTexture( acString, 0 );
		sprintf( acString, "%s\\topright.bmp", szAssetPath );
		mahUIButtonTextures[2] = pInterface->GetTexture( acString, 0 );
		sprintf( acString, "%s\\midleft.bmp", szAssetPath );
		mahUIButtonTextures[3] = pInterface->GetTexture( acString, 0 );
		sprintf( acString, "%s\\mid.bmp", szAssetPath );
		mahUIButtonTextures[4] = pInterface->GetTexture( acString, 0 );
		sprintf( acString, "%s\\midright.bmp", szAssetPath );
		mahUIButtonTextures[5] = pInterface->GetTexture( acString, 0 );
		sprintf( acString, "%s\\botleft.bmp", szAssetPath );
		mahUIButtonTextures[6] = pInterface->GetTexture( acString, 0 );
		sprintf( acString, "%s\\bot.bmp", szAssetPath );
		mahUIButtonTextures[7] = pInterface->GetTexture( acString, 0 );
		sprintf( acString, "%s\\botright.bmp", szAssetPath );
		mahUIButtonTextures[8] = pInterface->GetTexture( acString, 0 );
	}

}

void		ButtonStyle::InitOverlays( void )
{
int		nLoop;

	for ( nLoop = 0; nLoop < 9; nLoop++ )
	{
		mahUIButtonOverlays[nLoop] = UIInterfaceInstance()->CreateNewTexturedOverlay(1, mahUIButtonTextures[nLoop] );
	}
}


void	ButtonStyle::Render( int X, int Y, int W, int H, const char* szText, int mode, float fAlpha )
{
InterfaceInstance* pInterface = UIInterfaceInstance();
int		nButtonImageW = 8;
int		nButtonImageH = 8;
uint32	ulBackgroundCol;
float	fAlphaVal = m_fGlobalAlpha * fAlpha;

	if ( mahUIButtonOverlays[0] == NOTFOUND )
	{ 
		InitOverlays();
	}

	switch ( mode )
	{
	case 0:		// enabled/active
	default:
		ulBackgroundCol = GetColWithModifiedAlpha( 0xF0E0E0E0, fAlphaVal );
		break;
	case 1:		// Disabled
		ulBackgroundCol = GetColWithModifiedAlpha( 0x90D0D0D0, fAlphaVal );
		break;
	case 2:		// Hovered
		ulBackgroundCol = GetColWithModifiedAlpha( 0xFFFFFFFF, fAlphaVal );
		break;
	}


	pInterface->TexturedRect( mahUIButtonOverlays[0], X, Y, nButtonImageW, nButtonImageH, ulBackgroundCol, 0.0f, 0.0f, 1.0f, 1.0f );
	pInterface->TexturedRect( mahUIButtonOverlays[1], X+nButtonImageW, Y, W-(nButtonImageW*2), nButtonImageH, ulBackgroundCol, 0.0f, 0.0f, 1.0f, 1.0f );
	pInterface->TexturedRect( mahUIButtonOverlays[2], X+W-nButtonImageW, Y, nButtonImageW, nButtonImageH, ulBackgroundCol, 0.0f, 0.0f, 1.0f, 1.0f );

	pInterface->TexturedRect( mahUIButtonOverlays[3], X, Y+nButtonImageH, nButtonImageW, H-(nButtonImageH*2), ulBackgroundCol, 0.0f, 0.0f, 1.0f, 1.0f );
	pInterface->TexturedRect( mahUIButtonOverlays[4], X+nButtonImageW, Y+nButtonImageH, W-(nButtonImageW*2), H-(nButtonImageH*2), ulBackgroundCol, 0.0f, 0.0f, 1.0f, 1.0f );
	pInterface->TexturedRect( mahUIButtonOverlays[5], X+W-nButtonImageW, Y+nButtonImageH, nButtonImageW, H-(nButtonImageH*2), ulBackgroundCol, 0.0f, 0.0f, 1.0f, 1.0f );

	pInterface->TexturedRect( mahUIButtonOverlays[6], X, Y+H-nButtonImageH, nButtonImageW, nButtonImageH, ulBackgroundCol, 0.0f, 0.0f, 1.0f, 1.0f );
	pInterface->TexturedRect( mahUIButtonOverlays[7], X+nButtonImageW, Y+H-nButtonImageH, W-(nButtonImageW*2), nButtonImageH, ulBackgroundCol, 0.0f, 0.0f, 1.0f, 1.0f );
	pInterface->TexturedRect( mahUIButtonOverlays[8], X+W-nButtonImageW, Y+H-nButtonImageH, nButtonImageW, nButtonImageH, ulBackgroundCol, 0.0f, 0.0f, 1.0f, 1.0f );

	uint32	ulTextCol = GetColWithModifiedAlpha( 0xD0F0E0C0, fAlphaVal );
	int		nTextH = pInterface->GetStringHeight( szText, 1 );
	int		nTextY;

	if ( mode != 0 )
	{
		ulTextCol = GetColWithModifiedAlpha( 0x90F0E0C0, fAlphaVal );
	}

	nTextY = Y + ( ( H - (nTextH-1) ) / 2 );
	InterfaceSetFontFlags( FONT_FLAG_DROP_SHADOW );
	pInterface->TextCentre( 1, X + (W/2) + 1, nTextY, ulTextCol, 1, szText );
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

void		UIButtonDrawBasic( int nButtonID, int nX, int nY, int nWidth, int nHeight, const char* szText, int nMode, uint32 ulParam, float fAlpha )
{
InterfaceInstance* pInterface = UIInterfaceInstance();
uint32	ulButtonMainCol = 0xB0707070;
uint32	ulTextCol = 0xD0F0F0F0;
int		nTextSize = 15;
int		nTextOffsetY = 2;

	switch( nMode )
	{
	case 3:
		ulButtonMainCol = 0x60606060;
		ulTextCol = 0xE0F0E080;
		break;
	case 4:
		ulButtonMainCol = 0xB0901008;
		ulTextCol = 0xE0F0E080;
		break;
	case 0:
	default:
		break;
	}

	InterfaceRect( 0, nX, nY, nWidth, nHeight, ulButtonMainCol );
	InterfaceSetFontFlags( FONT_FLAG_DROP_SHADOW );
		
	InterfaceTextCentre( 1, nX + (nWidth/2), nY + nTextOffsetY, szText, ulTextCol, 0 ); 
	InterfaceSetFontFlags( 0 );
}


void		UIButtonDrawAlpha( int nButtonID, int nX, int nY, int nWidth, int nHeight, const char* szText, int nMode, uint32 ulParam, float fAlpha )
{
InterfaceInstance*		pInterfaceInstance = UIInterfaceInstance();
BOOL	bEnabled = TRUE;

	if ( nMode != 1 )
	{
		if ( UIHoverItem( nX, nY, nWidth, nHeight ) == TRUE )
		{
			nMode += 2;
		}
	}

	if ( nMode > 2 )
	{
		UIButtonDrawBasic( nButtonID, nX, nY, nWidth, nHeight, szText, nMode, ulParam, fAlpha );	
	}
	else
	{
		msButtonStyle.Render( nX, nY, nWidth, nHeight, szText, nMode, fAlpha );
	}

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
BOOL	bIsHovered = UIHoverItem( nX, nY, nWidth, nHeight );

	if ( UIIsPressed( nX, nY, nWidth, nHeight ) == TRUE )
	{
		UIPressIDSet( nButtonID, ulParam );
	}
	return( bIsHovered );
}

