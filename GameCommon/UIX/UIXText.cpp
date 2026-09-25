	

#include "StandardDef.h"
#include "InterfaceEx.h"
#include "UIXText.h"


void	UIXText::Initialise( const char* szTitle, uint32 ulCol, int font, UIX_TEXT_FLAGS fontFlags )
{
	mText = szTitle;
	mulCol = ulCol;
	mFont = font;
	mFontFlags = fontFlags;
}
	

UIXRECT		UIXText::OnRender( InterfaceInstance* pInterface, UIXRECT displayRect )
{
UIXRECT		drawRect = GetActualRenderRect( displayRect );
		// Centre align vertically (should be an option)
int			drawY = drawRect.y + (((drawRect.h - pInterface->GetStringHeight(mText.c_str(), mFont))-1)/2);

	if ( mDynamicTextFunc )
	{
	char	acBuff[512];
		mDynamicTextFunc( this, 0, acBuff );
		mText = acBuff;
	}

	if ( mFontFlags & SMALL )
	{
		pInterface->SetFontFlags( FONT_FLAG_SMALL );
	}
	// TODO - Use mFontFlags if set
// TODO - These are bitflags and some should be combinable
	switch( mFontFlags )
	{
	case ALIGN_RIGHT:
		pInterface->TextRight( 1, drawRect.x + drawRect.w, drawY, mulCol, mFont, mText.c_str() );
		break;
	case ALIGN_CENTRE:
		pInterface->TextCentre( 1, drawRect.x + ((drawRect.w/2)-1), drawY, mulCol, mFont, mText.c_str() );
		break;
	default:
		pInterface->TextLimitWidth( 1, drawRect.x, drawY, drawRect.w, mulCol, mFont, mText.c_str() );
		break;
	}

	if ( mFontFlags & SMALL )
	{
		pInterface->SetFontFlags( 0 );
	}

	displayRect.h = 0;//
	displayRect.y = GetLocalPositionRect().y + drawRect.h + 1;		// displayRect.y returns the lowest point we drew to
	return displayRect;
}


