
#include <stdarg.h>

#include "StandardDef.h"
#include "InterfaceEx.h"
#include "../../../Antiquated/GameCommon/Platform/Platform.h"
#include "../UI/UI.h"
#include "UIXTextBox.h"

void		UIXTextBox::RegisterControlHandlers()
{

}

void	UIXTextBox::Initialise( int mode, const char* szTitle )
{
	mText = szTitle;
	mMode = mode;
}


void	UIXTextBox::SetText( const char* text, ... )
{
char		acString[1024];
va_list		marker;
uint32*		pArgs;

	pArgs = (uint32*)( &text ) + 1;

	va_start( marker, text );     
	vsprintf( acString, text, marker );
	mText = acString;

}

void	UIXTextBox::EndEdit()
{
	mText = PlatformKeyboardGetInputString(FALSE);

	if ( GetParent() != NULL )
	{
		float	fValue = (float)atof( mText.c_str() );
		float	fAcceptedValue = GetParent()->OnValueChange( this, fValue, TRUE );

		if ( fAcceptedValue != fValue )
		{
			char		acVal[128];
			sprintf( acVal, "%.3f", fAcceptedValue );
			mText = acVal;
		}
	}

	if ( mfnEndEditCallback )
	{
		mfnEndEditCallback( this, mText.c_str() );
	}

	UIX::SetTextEditFocus( NULL );
	PlatformKeyboardSetInputString( "" );
	PlatformKeyboardRegisterHandler( NULL, NULL );
}

int		UIXTextBox::OnKeyboardMessage( int nResponseCode, const char* szInputText )
{

	switch( nResponseCode )
	{
	case KEYBOARD_ON_PRESS_ENTER:	// press enter
		EndEdit();
		break;
	case KEYBOARD_ON_PRESS_TAB:		// press TAB
		EndEdit();
		// TODO - Select next textbox
		break;
	}
	return( 0 );
}

int		UIXTextBox::TextBoxKeyboardMessageHandlerStatic( int nResponseCode, const char* szInputText, void* pUserObj )
{
	UIXTextBox*		pTextBox = (UIXTextBox*)pUserObj;
	return(pTextBox->OnKeyboardMessage(nResponseCode, szInputText));
}

BOOL	UIXTextBox::OnSelect()
{
const char*		pcInput = PlatformKeyboardGetInputString(FALSE);

	mPreEditText = mText;
	UIX::SetTextEditFocus( this );
	PlatformKeyboardRegisterHandler( TextBoxKeyboardMessageHandlerStatic, this );
	PlatformKeyboardActivate( 0, mText.c_str(), "" );
	return( FALSE );
}

bool		UIXTextBox::OnSelected( int nButtonID, uint32 ulParam )
{
	OnSelect();
	return( false );
}


UIXRECT		UIXTextBox::OnRender( InterfaceInstance* pInterface, UIXRECT displayRect )
{
UIXRECT		drawRect = GetActualRenderRect( displayRect );
uint32		ulOutlineBoxCol = 0x40404040;
uint32		ulTextCol = 0xd0d0d0d0;
char		acBuff[256];

	SetDraggableRenderRect(drawRect);

	if ( UIX::IsMouseHover( drawRect ) == TRUE )
	{
		ulOutlineBoxCol = 0x60808080;
		UIHoverIDSet( UIX_TEXTBOX, 0, GetID() );

		UIX::CheckForPress( this, drawRect, UIX_TEXTBOX, 0 );
	}

	// Render semi-trans drag item if we've got something in progress
	if ( IsDragHoldActive() && DragHasMoved())
	{
	UIXRECT	xGrabOffset = GetDragOffset();
	UIXRECT xNewRect = GetInitialDragRect();

		xNewRect.x += xGrabOffset.x;
		xNewRect.y += xGrabOffset.y;
		pInterface->Rect( 2, xNewRect.x, xNewRect.y, xNewRect.w, xNewRect.h, 0x40303030 );
		pInterface->Text( 2, xNewRect.x + 5, xNewRect.y + 2, 0x80a0a0a0, 0, mText.c_str() );	
	}


	pInterface->OutlineBox( 0, drawRect.x, drawRect.y, drawRect.w, drawRect.h, ulOutlineBoxCol );

	// TODO - Use mFontFlags if set
	// TODO - Make this TextLimitWidth

	strcpy( acBuff, mText.c_str() );
	if ( UIX::GetTextEditFocus() == this )
	{
		mText = PlatformKeyboardGetInputString( FALSE );

		if ( mText == mPreEditText)
		{
			// Higlight text
			pInterface->Rect( 0, drawRect.x + 2, drawRect.y + 2, drawRect.w - 4, drawRect.h - 4, 0x804040d0 );		
		}
		ulTextCol = 0xe0e0e0e0;
		strcpy( acBuff, PlatformKeyboardGetInputString(TRUE));
	}
	
	int		maxTextWidth = drawRect.w - 4;
	int stringWidth = pInterface->GetStringWidth(acBuff, 3);
	if (stringWidth >= maxTextWidth )
	{
		char* pcRunner = acBuff;
		int	   stringLen = strlen(acBuff);

		pInterface->Text(1, drawRect.x + 4, drawRect.y + (drawRect.h - 10) / 2, ulTextCol, 3, "..");

		maxTextWidth -= 8;
		while ((stringLen > 5) &&
			(stringWidth >= maxTextWidth))
		{
			pcRunner += 3;
			stringLen -= 3;
			stringWidth = pInterface->GetStringWidth(pcRunner, 3);
		}
		pInterface->Text(1, drawRect.x + 12, drawRect.y + (drawRect.h - 10) / 2, ulTextCol, 3, pcRunner);
	}
	else
	{
		pInterface->Text(1, drawRect.x + 4, drawRect.y + (drawRect.h - 10) / 2, ulTextCol, 3, acBuff);
	}

	displayRect.h = 0;//
	displayRect.y = GetLocalPositionRect().y + drawRect.h;		// displayRect.y returns the lowest point we drew to
	return displayRect;


}

