
#include <stdio.h>
#include "StandardDef.h"
#include "InterfaceEx.h"

#include "../Platform/Platform.h"
#include "UI.h"
#include "UIInternal.h"
#include "UIButton.h"

class ButtonStyle
{
public:
	ButtonStyle();

	void	Initialise( InterfaceInstance* pInterface, int mode, const char* szAssetPath );

	void	NewFrame( void );

	void	Render( InterfaceInstance* pInterface, int X, int Y, int W, int H, const char* szText, uint32 modeFlags, float fAlpha );

	void	Free( InterfaceInstance* pInterface);

private:
	void	InitOverlays( InterfaceInstance* pInterface );
	
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


void	ButtonStyle::Free(InterfaceInstance* pInterface)
{
int		nLoop;

	for ( nLoop = 0; nLoop < 9; nLoop++ )
	{
		UIInterfaceInstance()->ReleaseTexture(mahUIButtonTextures[nLoop] );
	}
}

void	ButtonStyle::Initialise( InterfaceInstance* pInterface, int mode, const char* szAssetPath )
{
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

void		ButtonStyle::InitOverlays( InterfaceInstance* pInterface )
{
int		nLoop;

	for ( nLoop = 0; nLoop < 9; nLoop++ )
	{
		mahUIButtonOverlays[nLoop] = pInterface->CreateNewTexturedOverlay(1, mahUIButtonTextures[nLoop] );
	}
}


void	ButtonStyle::Render( InterfaceInstance* pInterface, int X, int Y, int W, int H, const char* szText, uint32 modeFlags, float fAlpha )
{
int		nButtonImageW = 8;
int		nButtonImageH = 8;
uint32	ulBackgroundCol;
float	fAlphaVal = m_fGlobalAlpha * fAlpha;
int		nFont = 1;

	if ( mahUIButtonOverlays[0] == NOTFOUND )
	{ 
		InitOverlays( pInterface );
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

UIButtonImpl::UIButtonImpl( UIInstance* pUIInstance )
{
	mpButtonStyle = new ButtonStyle;
	mpButtonStyle->Initialise( pUIInstance->GetInterfaceInstance(), 1, "Data\\UI\\Button1");

}
	
void		UIButtonImpl::Shutdown(UIInstance* pUIInstance)
{
	mpButtonStyle->Free( pUIInstance->GetInterfaceInstance());

}

void		UIButtonImpl::NewFrame( void )
{
	mpButtonStyle->NewFrame();
}



void		UIButtonDrawBasic( InterfaceInstance* pInterface, int nButtonID, int nX, int nY, int nWidth, int nHeight, const char* szText, uint32 modeFlags, uint32 ulParam, uint32 ulIDParam, float fAlpha )
{
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

void	UIButtonImpl::DrawCheckbox(UIInstance* pUIInstance, int nButtonID, int nX, int nY, int nWidth, int nHeight, const char* szText, BOOL bChecked, uint32 ulParam, uint32 ulIDParam, float fAlpha)
{
	InterfaceInstance* pInterface = pUIInstance->GetInterfaceInstance();

	uint32		ulTextCol = 0xc0c0c0c0;
	pInterface->OutlineBox(0, nX, nY, nHeight, nHeight, 0xC0505050);

	if (bChecked)
	{
		pInterface->Rect(0, nX + 1, nY + 2, nHeight - 3, nHeight - 3, 0xd0808080);
		ulTextCol = 0xd0e0e0e0;
	}

	if (pUIInstance->HoverItem(nX, nY, nWidth, nHeight) == TRUE)
	{
		ulTextCol = 0xe0f0f0f0;
	}
	pInterface->Text(1, nX + nHeight + 2, nY + 3, ulTextCol, 3, szText);

	if ( pUIInstance->IsPressed(nX, nY, nWidth, nHeight) == TRUE )
	{
		pUIInstance->PressIDSet(nButtonID, ulParam, ulIDParam);
	}
}

void UIButtonImpl::Draw( UIInstance* pUIInstance, int nButtonID, int nX, int nY, int nWidth, int nHeight, const char* szText, eUIBUTTON_MODE_FLAGS modeFlags, uint32 ulParam, uint32 ulIDParam, float fAlpha )
{
InterfaceInstance*		pInterfaceInstance = pUIInstance->GetInterfaceInstance();
BOOL	bEnabled = TRUE;

	if ( (modeFlags & UIBUTTON_FLAG_DISABLED) == 0 )
	{
		if ( pUIInstance->HoverItem( nX, nY, nWidth, nHeight ) == TRUE )
		{
			modeFlags = (eUIBUTTON_MODE_FLAGS)(modeFlags | UIBUTTON_FLAG_HOVERED);
		}
	}

	if ( modeFlags & UIBUTTON_FLAG_FLAT_STYLE )
	{
		UIButtonDrawBasic( pInterfaceInstance, nButtonID, nX, nY, nWidth, nHeight, szText, modeFlags, ulParam, ulIDParam, fAlpha );	
	}
	else
	{
		mpButtonStyle->Render( pInterfaceInstance, nX, nY, nWidth, nHeight, szText, modeFlags, fAlpha );
	}

	if ( (modeFlags & UIBUTTON_FLAG_DISABLED) == 0 )
	{
		if ( pUIInstance->IsPressed( nX, nY, nWidth, nHeight ) == TRUE )
		{
			pUIInstance->PressIDSet( nButtonID, ulParam, ulIDParam );
		}
	}

}

void		UIButtonDrawAlpha( int nButtonID, int nX, int nY, int nWidth, int nHeight, const char* szText, eUIBUTTON_MODE_FLAGS modeFlags, uint32 ulParam, uint32 ulIDParam, float fAlpha )
{
	mspTempSingleton->ButtonDrawAlpha( nButtonID, nX, nY, nWidth, nHeight, szText, modeFlags, ulParam, ulIDParam, fAlpha);
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



//--------------------------------
// Legacy singleton mode

void		UIButtonDraw(int nButtonID, int nX, int nY, int nWidth, int nHeight, const char* szText, eUIBUTTON_MODE_FLAGS modeFlags, uint32 ulParam, uint32 ulIDParam)
{
	mspTempSingleton->ButtonDrawAlpha(nButtonID, nX, nY, nWidth, nHeight, szText, modeFlags, ulParam, ulIDParam, 1.0f);
}

void		UIButtonsNewFrame(void)
{
	mspTempSingleton->mpUIButtonImpl->NewFrame();
}

void		UIButtonsShutdown(void)
{
	mspTempSingleton->mpUIButtonImpl->Shutdown(mspTempSingleton);
}
