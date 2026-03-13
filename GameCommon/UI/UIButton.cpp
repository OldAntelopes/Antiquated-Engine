
#include <stdio.h>
#include "StandardDef.h"
#include "InterfaceEx.h"

#include "../Platform/Platform.h"
#include "UI.h"
#include "UIButton.h"

class ButtonStyle
{
public:
	ButtonStyle();

	void	Initialise( int mode, const char* szAssetPath );

	void	NewFrame( void );

	void	Render( int X, int Y, int W, int H, const char* szText, uint32 modeFlags, float fAlpha );

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


void	ButtonStyle::Render( int X, int Y, int W, int H, const char* szText, uint32 modeFlags, float fAlpha )
{
InterfaceInstance* pInterface = UIInterfaceInstance();
int		nButtonImageW = 8;
int		nButtonImageH = 8;
uint32	ulBackgroundCol;
float	fAlphaVal = m_fGlobalAlpha * fAlpha;
int		nFont = 1;

	if ( mahUIButtonOverlays[0] == NOTFOUND )
	{ 
		InitOverlays();
	}

	if ( modeFlags & UIBUTTON_FLAG_DISABLED )
	{
		ulBackgroundCol = GetColWithModifiedAlpha( 0x90D0D0D0, fAlphaVal );
	}
	else if ( modeFlags & UIBUTTON_FLAG_HOVERED )
	{
		ulBackgroundCol = GetColWithModifiedAlpha( 0xFFFFFFFF, fAlphaVal );
	}
	else
	{
		ulBackgroundCol = GetColWithModifiedAlpha( 0xF0E0E0E0, fAlphaVal );	
	}

	if (modeFlags & UIBUTTON_FLAG_SMALL_FONT)
	{
		nFont = 3;
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

	if ( pInterface->GetStringWidth( szText, nFont ) > W )
	{
		nFont = 3;
	}

	if ( !(modeFlags & UIBUTTON_FLAG_NO_LABEL) )
	{
		uint32	ulTextCol = GetColWithModifiedAlpha( 0xD0F0E0C0, fAlphaVal );
		int		nTextH = pInterface->GetStringHeight( szText, nFont );
		int		nTextY;

		if ( modeFlags & UIBUTTON_FLAG_DISABLED )
		{
			ulTextCol = GetColWithModifiedAlpha( 0x90F0E0C0, fAlphaVal );
		}

		nTextY = Y + ( ( H - (nTextH-1) ) / 2 );
		InterfaceSetFontFlags( FONT_FLAG_DROP_SHADOW );

		const char* pcTextToDisplay = szText;

		if ( modeFlags & UIBUTTON_FLAG_LABEL_EDIT )
		{
			pcTextToDisplay = PlatformKeyboardGetInputString( TRUE );
			pInterface->TextLimitWidth(1, X + 3, nTextY, (W - 4), ulTextCol, nFont, pcTextToDisplay);
		}
		else if (pInterface->GetStringWidth(pcTextToDisplay, nFont) < (W - 4))
		{
			pInterface->TextCentre(1, X + (W / 2) + 1, nTextY, ulTextCol, nFont, pcTextToDisplay);
		}
		else
		{
			pInterface->TextLimitWidth(1, X + 3, nTextY, (W - 4), ulTextCol, nFont, pcTextToDisplay);
		}

		InterfaceSetFontFlags( 0 );
	}

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

void		UIButtonDrawBasic( int nButtonID, int nX, int nY, int nWidth, int nHeight, const char* szText, uint32 modeFlags, uint32 ulParam, uint32 ulIDParam, float fAlpha )
{
InterfaceInstance* pInterface = UIInterfaceInstance();
uint32	ulButtonMainCol = 0xB0707070;
uint32	ulTextCol = 0xD0F0F0F0;
int		nTextSize = 15;
int		nTextOffsetY = 2;

	if ( modeFlags & UIBUTTON_FLAG_DISABLED )
	{
		ulButtonMainCol = 0x60606060;
		ulTextCol = 0xE0F0E080;
	}
	else if ( modeFlags & UIBUTTON_FLAG_HOVERED )
	{
		ulButtonMainCol = 0xB0901008;
		ulTextCol = 0xE0F0E080;
	}

	pInterface->Rect( 0, nX, nY, nWidth, nHeight, ulButtonMainCol );
	InterfaceSetFontFlags( FONT_FLAG_DROP_SHADOW );
		
	pInterface->TextCentre( 1, nX + (nWidth/2), nY + nTextOffsetY, ulTextCol, 0, szText ); 
	InterfaceSetFontFlags( 0 );
}


void		UIButtonDrawAlpha( int nButtonID, int nX, int nY, int nWidth, int nHeight, const char* szText, eUIBUTTON_MODE_FLAGS modeFlags, uint32 ulParam, uint32 ulIDParam, float fAlpha )
{
InterfaceInstance*		pInterfaceInstance = UIInterfaceInstance();
BOOL	bEnabled = TRUE;

	if ( (modeFlags & UIBUTTON_FLAG_DISABLED) == 0 )
	{
		if ( UIHoverItem( nX, nY, nWidth, nHeight ) == TRUE )
		{
			modeFlags = (eUIBUTTON_MODE_FLAGS)(modeFlags | UIBUTTON_FLAG_HOVERED);
		}
	}

	if ( modeFlags & UIBUTTON_FLAG_FLAT_STYLE )
	{
		UIButtonDrawBasic( nButtonID, nX, nY, nWidth, nHeight, szText, modeFlags, ulParam, ulIDParam, fAlpha );	
	}
	else
	{
		msButtonStyle.Render( nX, nY, nWidth, nHeight, szText, modeFlags, fAlpha );
	}

	if ( (modeFlags & UIBUTTON_FLAG_DISABLED) == 0 )
	{
		if ( UIIsPressed( nX, nY, nWidth, nHeight ) == TRUE )
		{
			UIPressIDSet( nButtonID, ulParam, ulIDParam );
		}
	}

}


void		UIButtonDraw( int nButtonID, int nX, int nY, int nWidth, int nHeight, const char* szText, eUIBUTTON_MODE_FLAGS modeFlags, uint32 ulParam, uint32 ulIDParam  )
{
	UIButtonDrawAlpha( nButtonID, nX, nY, nWidth, nHeight, szText, modeFlags, ulParam, ulIDParam, 1.0f );
}


BOOL		UIButtonRegion( int nButtonID, int nX, int nY, int nWidth, int nHeight, uint32 ulParam, uint32 ulIDParam )
{
BOOL	bIsHovered = UIHoverItem( nX, nY, nWidth, nHeight );

	if ( UIIsPressed( nX, nY, nWidth, nHeight ) == TRUE )
	{
		UIPressIDSet( nButtonID, ulParam, ulIDParam );
	}
	return( bIsHovered );
}

