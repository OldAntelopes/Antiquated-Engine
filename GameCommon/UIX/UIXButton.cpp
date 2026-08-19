
#include "StandardDef.h"
#include "InterfaceEx.h"

#include "../Platform/Platform.h"
#include "../UI/UI.h"
#include "UIXButton.h"

UIXButton::~UIXButton()
{
	if (mhImageTexture != NOTFOUND)
	{
		InterfaceReleaseTexture(mhImageTexture);
	}
}

void	UIXButton::EndEdit()
{
	if ( mfnLabelEditCallback )
	{
		mTitle = PlatformKeyboardGetInputString(FALSE);
		PlatformKeyboardSetInputString( "" );
		if ( mfnLabelEditCallback(this, mTitle.c_str() ) == false )
		{
			mTitle = mPreEditTitle;
		}
	}
	UIX::SetTextEditFocus(NULL);
}

int		UIXButton::OnKeyboardMessage( int nResponseCode, const char* szInputText )
{
	switch( nResponseCode )
	{
	case 1:	// press enter
		EndEdit();
		break;
	case 2:		// press TAB

		break;
	}
	return( 0 );
}

int		UIXButton::ButtonEditLabelKeyboardMessageHandlerStatic( int nResponseCode, const char* szInputText, void* pUserObj )
{
	UIXButton*		pButton = (UIXButton*)pUserObj;
	return(pButton->OnKeyboardMessage(nResponseCode, szInputText));
}


void	UIXButton::EnableLabelEdit( fnLabelEditCallback callbackFunc )
{
	mfnLabelEditCallback = callbackFunc; 
	mPreEditTitle = mTitle;
	UIX::SetTextEditFocus( this );
	PlatformKeyboardRegisterHandler( ButtonEditLabelKeyboardMessageHandlerStatic, this );
	PlatformKeyboardActivate( 0, mTitle.c_str(), "" );
}


void	UIXButton::Initialise( eUIXBUTTON_MODE mode, const char* szTitle, uint32 ulButtonID, uint32 ulButtonParam, BOOL bIsBlocking, uint32 ulCol, int iconNum )
{
	if ( szTitle )
	{
		mTitle = szTitle;
	}
	mMode = mode;
	mulButtonID = ulButtonID;
	mulButtonParam = ulButtonParam;
	mbIsBlocking = bIsBlocking;
	mulCol = ulCol;
	mIconNum = iconNum;
	if ( mode == UIXBUTTON_IMAGE )
	{
		mhImageTexture = mIconNum;
	}
}
	
UIXRECT		UIXButton::OnRender( InterfaceInstance* pInterface, UIXRECT displayRect )
{
UIXRECT		localRect = GetLocalPositionRect();
UIXRECT		drawRect = GetActualRenderRect( displayRect );
uint32		ulDefaultCol = 0x90404040;
		
	if ( mulCol != 0 )
	{
		ulDefaultCol = mulCol;
	}

	SetDraggableRenderRect(drawRect);
	
	if ( UIX::IsMouseHover( drawRect ) == TRUE )
	{
		UIHoverIDSet( UIX_BUTTON, 0, GetID() );
	}
	// Receive dragged item
	if ( CheckDragHoverRegion( UIXRECT( drawRect.x, drawRect.y, drawRect.w, drawRect.h ) ) )
	{
		pInterface->OutlineBox( 1, drawRect.x, drawRect.y, drawRect.w, drawRect.h, 0x808080c0 );	
		UIX::HoverAcceptDragItem( this );
	}

	// Render semi-trans drag item if we've got something in progress
	if ( IsDragHoldActive() && DragHasMoved())
	{
	UIXRECT	xGrabOffset = GetDragOffset();
	UIXRECT xNewRect = GetInitialDragRect();

		xNewRect.x += xGrabOffset.x;
		xNewRect.y += xGrabOffset.y;
		pInterface->Rect( 2, xNewRect.x, xNewRect.y, xNewRect.w, xNewRect.h, 0x40303030 );
		pInterface->Text( 2, xNewRect.x + 5, xNewRect.y + 2, 0x80a0a0a0, 0, mTitle.c_str() );	
	}

	switch( mMode)
	{
	case UIXBUTTON_NORMAL:
	case UIXBUTTON_DISABLED:
	default:
		{
		eUIBUTTON_MODE_FLAGS		modeFlags = UIBUTTON_FLAG_SMALL_FONT;

			if (mfnLabelEditCallback != NULL)
			{
				modeFlags = (eUIBUTTON_MODE_FLAGS)(modeFlags | UIBUTTON_FLAG_LABEL_EDIT);
			}
			if ( mMode == UIXBUTTON_DISABLED )
			{
				modeFlags = (eUIBUTTON_MODE_FLAGS)(modeFlags | UIBUTTON_FLAG_DISABLED);
			}

			UIButtonDraw( mulButtonID, drawRect.x, drawRect.y, drawRect.w, drawRect.h, mTitle.c_str(), modeFlags, mulButtonParam, GetID() );
			if ( mulRightPressButtonID )
			{
				UIX::CheckForRightButtonPress( this, drawRect, mulRightPressButtonID, mulRightPressButtonParam );	
			}
			if ( HasRightClickSelectionCallback() )
			{
				UIX::CheckForRightButtonPress(this, drawRect, UIX_RIGHT_CLICK_SELECT, 0);
			}	
		}
		break;
	case UIXBUTTON_IMAGE_PATH:
		if ( mbHasAttemptedLoadTexture == FALSE )
		{
			// TODO - Switch to async
			mhImageTexture = pInterface->GetTexture( mTitle.c_str(), 0 );
			mbHasAttemptedLoadTexture = TRUE;
		}

		if ( mhImageTexture != NOTFOUND )
		{
			int		nOverlay = pInterface->CreateNewTexturedOverlay( 0, mhImageTexture );
			pInterface->TexturedRect( nOverlay, drawRect.x, drawRect.y, drawRect.w, drawRect.h, mulCol, 0.0f, 0.0f, 1.0f, 1.0f );
		}
		else
		{
			pInterface->Rect( 0, drawRect.x, drawRect.y, drawRect.w, drawRect.h, ulDefaultCol );
		}

		if ( mulButtonID != 0 )
		{	
			if ( UIX::CheckForPress( this, drawRect, mulButtonID, mulButtonParam ) )
			{		
				pInterface->Rect( 1, drawRect.x, drawRect.y, drawRect.w, drawRect.h, 0x20FFFFFF );				
			}
		}
		if ( HasRightClickSelectionCallback() )
		{
			UIX::CheckForRightButtonPress(this, drawRect, UIX_RIGHT_CLICK_SELECT, 0);
		}	
		break;
	case UIXBUTTON_IMAGE:
		if ( mfnDynamicButtonImageHandler != NULL )
		{
		int		hTexture = mfnDynamicButtonImageHandler( this, mpImageHandlerParam );

			if ( hTexture != NOTFOUND )
			{
			int		nOverlay = pInterface->CreateNewTexturedOverlay( 0, hTexture );
			uint32	ulCol = 0xD0FFFFFF;

				if ( mulCol != 0 )
				{
					ulCol = mulCol;
				}
				pInterface->TexturedRect( nOverlay, drawRect.x, drawRect.y, drawRect.w, drawRect.h, ulCol, 0.0f, 0.0f, 1.0f, 1.0f );
			}
			else
			{
				pInterface->Rect( 0, drawRect.x, drawRect.y, drawRect.w, drawRect.h, ulDefaultCol );
			}		
		}
		else if ( mhImageTexture != NOTFOUND )
		{
			int		nOverlay = pInterface->CreateNewTexturedOverlay( 0, mhImageTexture );
			pInterface->TexturedRect( nOverlay, drawRect.x, drawRect.y, drawRect.w, drawRect.h, mulCol, 0.0f, 0.0f, 1.0f, 1.0f );
		}
		else
		{
			pInterface->Rect( 0, drawRect.x, drawRect.y, drawRect.w, drawRect.h, ulDefaultCol );
		}

		if ( mulButtonID != 0 )
		{	
			if ( UIX::CheckForPress( this, drawRect, mulButtonID, mulButtonParam ) )
			{		
				pInterface->Rect( 1, drawRect.x, drawRect.y, drawRect.w, drawRect.h, 0x30FFe0c0 );				
			}
		}
		if ( mulRightPressButtonID )
		{
			UIX::CheckForRightButtonPress( this, drawRect, mulRightPressButtonID, mulRightPressButtonParam );	
		}
		break;
	case UIXBUTTON_ICON:
		{
			UIButtonDraw( mulButtonID, drawRect.x, drawRect.y, drawRect.w, drawRect.h, "", UIBUTTON_FLAG_NONE, mulButtonParam, GetID() );

			UIXRECT	 iconRect = drawRect;

			// TEMP assumes icons are all 19x19
			iconRect.x += ((drawRect.w-19)/2);
			iconRect.y += ((drawRect.h-19)/2);
			iconRect.w = 19;
			iconRect.h = 19;

			UIX::DrawIcon( pInterface, mIconNum, iconRect, 0xE0D0D0D0 );
		}
		break;

	case UIXBUTTON_RECT_ICON:
		if ( mbIsGlowing )
		{
		uint32		ulGlowCol = UIGetGlowColour();

			pInterface->Rect( 0, drawRect.x, drawRect.y, drawRect.w, drawRect.h, ulGlowCol );
		}
		else
		{
			pInterface->Rect( 0, drawRect.x, drawRect.y, drawRect.w, drawRect.h, mulCol );
		}
		if ( mulButtonID != 0 )
		{
			if ( UIX::CheckForPress( this, drawRect, mulButtonID, mulButtonParam ) )
			{		
				pInterface->Rect( 1, drawRect.x, drawRect.y, drawRect.w, drawRect.h, 0x20FFFFFF );				
			}
		}
		drawRect.x += ((drawRect.w - 19)/2);
		drawRect.y += ((drawRect.h - 19)/2);
		drawRect.w = 19;
		drawRect.h = 19;
		UIX::DrawIcon( pInterface, mIconNum, drawRect, 0xE0D0D0D0 );
		break;		
	case UIXBUTTON_PLAIN_RECT_BOLD_TEXT:
		pInterface->Rect( 0, drawRect.x, drawRect.y, drawRect.w, drawRect.h, mulCol );

		if ( mulButtonID != 0 )
		{
			if ( UIX::CheckForPress( this, drawRect, mulButtonID, mulButtonParam ) )
			{		
				pInterface->Rect( 1, drawRect.x, drawRect.y, drawRect.w, drawRect.h, 0x20FFFFFF );				
			}
		}
		// TODO - Properly centre the title
		pInterface->Text( 1, drawRect.x + 6, drawRect.y + 1, 0xd0a0a0a0, 1, mTitle.c_str() );
		break;
	case UIXBUTTON_PLAIN_RECT:
		pInterface->Rect( 0, drawRect.x, drawRect.y, drawRect.w, drawRect.h, mulCol );

		if ( mulButtonID != 0 )
		{
			if ( UIX::CheckForPress( this, drawRect, mulButtonID, mulButtonParam ) )
			{		
				pInterface->Rect( 1, drawRect.x, drawRect.y, drawRect.w, drawRect.h, 0x20FFFFFF );				
			}
		}
		if ( mulRightPressButtonID )
		{
			UIX::CheckForRightButtonPress( this, drawRect, mulRightPressButtonID, mulRightPressButtonParam );	
		}
		// TODO - Properly centre the title
		pInterface->Text( 1, drawRect.x + 8, drawRect.y + 3, 0xd0a0a0a0, 3, mTitle.c_str() );
		break;
	case UIXBUTTON_TEXT_WITH_COLLAPSABLE:
		{
		int		nFont = 3;
		int		stringHeight = pInterface->GetStringHeight( mTitle.c_str(), nFont );
		
			pInterface->TextRight( 0, drawRect.x + drawRect.w, drawRect.y + ((drawRect.h-stringHeight)/2), 0xD0c0c0d0, 3, mTitle.c_str() );

			if ( UIX::CheckForPress( this, drawRect, mulButtonID, mulButtonParam ) )
			{			
			UIXRECT		cogRect = drawRect;
				cogRect.x += 2;
				cogRect.y += 6;
				cogRect.h -= 4;
				cogRect.w = cogRect.h;
				if ( !mbIsCollapsed )
				{
					pInterface->Triangle( 1, cogRect.x, cogRect.y, cogRect.x + 8, cogRect.y, cogRect.x + 4, cogRect.y + 8, 0xa0a0a0a0, 0xa0a0a0a0, 0xa0a0a0a0 );	
				}
				else
				{
					pInterface->Triangle( 1, cogRect.x, cogRect.y, cogRect.x + 6, cogRect.y + 4, cogRect.x, cogRect.y + 8, 0xa0a0a0a0, 0xa0a0a0a0, 0xa0a0a0a0 );	
				}
			}
		}
		break;
	case UIXBUTTON_TEXT_WITH_SETTINGS:
		{
		int		nFont = 3;
		int		stringHeight = pInterface->GetStringHeight( mTitle.c_str(), nFont );
		
			pInterface->TextRight( 0, drawRect.x + drawRect.w, drawRect.y + ((drawRect.h-stringHeight)/2), 0xD0c0c0d0, 3, mTitle.c_str() );

			if ( UIX::CheckForPress( this, drawRect, mulButtonID, mulButtonParam ) )
			{
			UIXRECT		cogRect = drawRect;

				cogRect.x += 2;
				cogRect.y += 2;
				cogRect.h -= 4;
				cogRect.w = cogRect.h;
				// Draw  
				UIX::DrawIcon( pInterface, 2, cogRect, 0xa0a0a0a0 );
			}
		}

		break;

	}

	if ( (GetUIDParam() != 0 ) &&
		 (UIX::IsUISelectionModeActive() == TRUE) )
	{
		uint32		ulShadedCol = 0x40d0c040;
		uint32		ulOutlineCol = 0x80e0d080;

		if (UIX::GetFocusedObject() == this)
		{
			ulShadedCol = 0x806080d0;
			ulOutlineCol = 0xB0a0b0f0;
		}
		else if (UIX::IsMouseHover(drawRect))
		{
			ulShadedCol = 0x60e0d060;
			ulOutlineCol = 0xB0f0e0a0;
			UIX::CheckForPress(this, drawRect, UIX_OBJECT_SELECT, 0);
		}
		else if (GetObjectSelectionText().empty() == false)
		{
			ulShadedCol = 0x60108020;
			ulOutlineCol = 0xB020c060;
		}

		pInterface->Rect(0, drawRect.x, drawRect.y, drawRect.w, drawRect.h, ulShadedCol);
		pInterface->OutlineBox(0, drawRect.x, drawRect.y, drawRect.w, drawRect.h, ulOutlineCol);

		auto	selectionText = GetObjectSelectionText();
		int		nFontHeight = 12;
		int		lineY = drawRect.y + (drawRect.h / 2) - (nFontHeight / 2);
		//		pInterface->TextCentre( 1, mRenderRect.x + (mRenderRect.w/2), lineY, 0xd0d0d0d0, 3, selectionText.c_str() );
		pInterface->TextBox(1, drawRect.x, lineY, drawRect.w, 0xd0d0d0d0, 3, 0, selectionText.c_str());
	}


	displayRect.h = localRect.h + 1;
	displayRect.y = localRect.h + 1; //drawRect.y + drawRect.h + 1;		// displayRect.y returns the lowest point we drew to

	if ( !mbIsBlocking )
	{
		displayRect.h = 0;
	}
	return( displayRect );
}




