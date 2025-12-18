	

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
UIXRECT		localRect = GetDisplayRect();

	switch( mFontFlags )
	{
	case ALIGN_RIGHT:
		pInterface->TextRight( 1, displayRect.x + localRect.x + localRect.w, displayRect.y + localRect.y, mulCol, mFont, mText.c_str() );
		break;
	default:
		pInterface->Text( 1, displayRect.x + localRect.x, displayRect.y + localRect.y, mulCol, mFont, mText.c_str() );
		break;
	}
	// TODO - Use mFontFlags if set
	// TODO - Make this TextLimitWidth
	displayRect.h = 0;//pInterface->GetStringHeight( mText.c_str(), mFont );
	return displayRect;
}


