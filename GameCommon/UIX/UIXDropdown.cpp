
#include "StandardDef.h"
#include "InterfaceEx.h"

#include "../UI/UI.h"
#include "UIXDropdown.h"

UIXDropdownEntry::UIXDropdownEntry( UIXDropdown* pParent, const char* szTitle, uint32 ulUserParam, BOOL bGreyed )
{
	mTitle = szTitle;
	mulUserParam = ulUserParam;
	mpDropdown = pParent;
	mbGreyed = bGreyed;

}
	

//-------------------------------------

UIXDropdown::~UIXDropdown()
{
	UIX::DeleteObject( mpScrollbar );
	for ( UIXDropdownEntry* pListEntry : mDropdownEntries )
	{
		delete pListEntry;
	}
	mDropdownEntries.clear();
}


void	UIXDropdown::Initialise( int mode )
{
	mpScrollbar = new UIXScrollbar(this, UIX::GetNextObjectID(), UIXRECT(0, 0, 0, 0));

}

void		UIXDropdown::OnMouseWheel(float fOffset)
{
	mpScrollbar->OnMouseWheel(fOffset);
}

void	UIXDropdown::ToggleExpanded()
{ 
	mbIsExpanded = !mbIsExpanded;
	if (mbIsExpanded)
	{
		UIX::CloseAllDropdowns(GetID());
	}
}
	


void	UIXDropdown::OnFocusedKeyUp( int keyCode )
{
	switch( keyCode )
	{
	case KEY_UP_ARROW:
		if ( mnSelectedIndex > 0 )
		{
			SetSelectedElementIndex( mnSelectedIndex - 1 );
		}
		else
		{
			SetSelectedElementIndex( (int)mDropdownEntries.size() - 1 );
		}
		break;
	case KEY_DOWN_ARROW:
		if ( mnSelectedIndex < (int)mDropdownEntries.size() - 1 )
		{
			SetSelectedElementIndex( mnSelectedIndex + 1 );
		}
		else
		{
			SetSelectedElementIndex( 0 );
		}
		break;
	}

}

bool	UIXDropdown::OnSelected( int nButtonID, uint32 ulParam)
{
	switch( nButtonID )
	{
	case UIX_DROPDOWN_ENTRY:
		SetSelectedElementIndex( ulParam );
		ToggleExpanded();
		break;
	case UIX_DROPDOWN_HEADER:
		ToggleExpanded();
		break;
	default:
		PANIC_IF(TRUE, "UIXDropdown::OnSelected - Unknown button ID");
		break;
	}
	return( false );
}


void	UIXDropdown::OnEscape()
{
	mbIsExpanded = FALSE;
}



void	UIXDropdown::OnCloseAllDropdowns( uint32 ulExceptID )
{
	if ( ulExceptID != GetID() )
	{
		mbIsExpanded = FALSE;
	}
}


UIXRECT		UIXDropdown::OnRender( InterfaceInstance* pInterface, UIXRECT rect )
{
UIXRECT		renderRect = GetActualRenderRect( rect );
UIXRECT		occupyRect = rect;

	if ( IsFocusedObject() )
	{
		pInterface->Rect( 1, renderRect.x, renderRect.y, renderRect.w, renderRect.h, 0xb0303038 );
	}
	else
	{
		pInterface->Rect( 1, renderRect.x, renderRect.y, renderRect.w, renderRect.h, 0xa0202020 );
	}
	pInterface->OutlineBox( 1, renderRect.x, renderRect.y, renderRect.w, renderRect.h, 0xa0606060 );
	pInterface->Triangle( 1, renderRect.x + renderRect.w - 9, renderRect.y + 3, renderRect.x + renderRect.w - 3, renderRect.y + 3, renderRect.x + renderRect.w - 6, renderRect.y + 9, 0xA0A0A0A0, 0xA0A0A0A0, 0xA0A0A0A0 );

	if ( mpSelectedEntry )
	{
		pInterface->Text( 1, renderRect.x + 3, renderRect.y + 3, 0xD0D0D0D0, 3, mpSelectedEntry->GetText().c_str() );
	}
	
//	if ( ( UIX::GetModalObject() == NULL ) ||
//		 ( UIX::GetModalObject() == this ) ) 
//	{
		UIX::CheckForPress( this, renderRect, UIX_DROPDOWN_HEADER, 0 );
		UIX::CheckForRightButtonPress(this, renderRect, UIX_RIGHT_CLICK_SELECT, 0);

//	}


	occupyRect.h = 0;//GetLocalPositionRect().h + 1;
	occupyRect.y = GetLocalPositionRect().y + GetLocalPositionRect().h + 1;
	return occupyRect;
}

void		UIXDropdown::OnPostRender( InterfaceInstance* pInterface, UIXRECT rect )
{
UIXRECT		renderRect = GetActualRenderRect( rect );
int			entryIndex = 0;
	
	if ( mbIsExpanded )
	{
	UIXRECT		expandedRect = renderRect;
	int			nMaxDropdownH;
	bool		bScrollbarRequired = false;
	int			nMaxNumEntriesInView = 1;
		
		expandedRect.y += renderRect.h + 1;
		expandedRect.h = (mDropdownEntries.size() * 14) + 4;
		// Show list..
		if ( m_DropListWidth != 0 )
		{
			expandedRect.w = m_DropListWidth;
		}

		nMaxDropdownH = pInterface->GetHeight() - (expandedRect.y + 4);
		nMaxNumEntriesInView = (nMaxDropdownH - 4) / 14;

		if ( nMaxNumEntriesInView < (int)mDropdownEntries.size() )
		{
		UIXRECT	dropUpRect = expandedRect;
		int		nMaxDropUpRectH = nMaxDropdownH;
		int		nDropUpMaxEntryCount;

			dropUpRect.h = (mDropdownEntries.size() * 14) + 4;
			dropUpRect.y = renderRect.y - dropUpRect.h;
			if (dropUpRect.y < 10) dropUpRect.y = 10;
			nMaxDropUpRectH = renderRect.y - dropUpRect.y;
			nDropUpMaxEntryCount = (nMaxDropUpRectH - 4) / 14;

			// Put the dropdown list above the box when appropriate
			if ( nDropUpMaxEntryCount > nMaxNumEntriesInView )
			{
				expandedRect = dropUpRect;
				nMaxDropdownH = nMaxDropUpRectH;
				nMaxNumEntriesInView = nDropUpMaxEntryCount;
			}
		}

		if (nMaxNumEntriesInView < 1) nMaxNumEntriesInView = 1;
		UIXRECT		lineRect = expandedRect;
		lineRect.y += 2;
		lineRect.h = 14;

		if (expandedRect.h > nMaxDropdownH)
		{
			bScrollbarRequired = true;

			UIXRECT		scrollbarRect;

			scrollbarRect.x = expandedRect.x + expandedRect.w - 14;
			scrollbarRect.w = 13;
			scrollbarRect.y = expandedRect.y + 1;
			scrollbarRect.h = nMaxDropdownH - 2;

			mpScrollbar->RenderDirect(pInterface, scrollbarRect, nMaxNumEntriesInView - 1, mDropdownEntries.size() );

			lineRect.w -= 13;
			expandedRect.h = nMaxDropdownH;
		}

		if (UIX::IsMouseHover(expandedRect) == TRUE)
		{
			UIX::SetMousewheelHoverObject(this);
		}
		pInterface->Rect( 0, expandedRect.x, expandedRect.y, expandedRect.w, expandedRect.h, 0xF0202020 );
		pInterface->OutlineBox( 0, expandedRect.x, expandedRect.y, expandedRect.w, expandedRect.h, 0xF0606060 );

		for ( auto listEntry : mDropdownEntries )
		{
			if ((entryIndex >= mpScrollbar->GetScrollPosition()) &&
				(entryIndex < mpScrollbar->GetScrollPosition() + nMaxNumEntriesInView))
			{
				if ( listEntry->IsGreyed() )
				{
					pInterface->TextLimitWidth(2, lineRect.x + 3, lineRect.y, lineRect.w, 0x80606060, 3, listEntry->GetText().c_str());			
				}
				else
				{
					pInterface->TextLimitWidth(2, lineRect.x + 3, lineRect.y, lineRect.w, 0xc0d0d0d0, 3, listEntry->GetText().c_str());

					if (UIX::CheckForPress(this, lineRect, UIX_DROPDOWN_ENTRY, entryIndex))
					{
						pInterface->Rect(1, lineRect.x, lineRect.y, lineRect.w, lineRect.h, 0xa0808080);
					}
				}
				lineRect.y += 14;
			}
			entryIndex++;
		}
	}
}

void	UIXDropdown::SetSelectedElementName( const char* szName )
{
int		index = 0;
	for ( auto listEntry : mDropdownEntries )
	{
		if ( stricmp( szName, listEntry->GetText().c_str() ) == 0 )
		{
			mpSelectedEntry = listEntry;
			mnSelectedIndex = index;
			if ( mSelectionChangedCallback )
			{
				mSelectionChangedCallback( this, mpSelectedEntry->mulUserParam );
			}
			return;
		}
		index++;
	}

}
void		UIXDropdown::OnUpdate( float fDelta )
{
	if ( mValueUpdateFunc )
	{
	int nNewVal = (int)mValueUpdateFunc( this, mnSelectedIndex );

		if ( mnSelectedIndex != nNewVal )
		{
			if ( ( nNewVal >= 0 ) &&
				 ( nNewVal < (int)mDropdownEntries.size() ) )
			{
				mnSelectedIndex = nNewVal;
				mpSelectedEntry = mDropdownEntries[nNewVal];
			}
		}
	}
}


void		UIXDropdown::SetSelectedElementIndex( int index )
{
	if ( index < (int)mDropdownEntries.size() )
	{
		mpSelectedEntry = mDropdownEntries[index];
		mnSelectedIndex = index;

		if ( mSelectionChangedCallback )
		{
			mSelectionChangedCallback( this, mpSelectedEntry->mulUserParam );
		}
	}
	else
	{
		mpSelectedEntry = NULL;
		mnSelectedIndex = NOTFOUND;
	}
}

UIXDropdownEntry*	UIXDropdown::AddElement( const char* szElementName, uint32 ulElementParam, BOOL bGreyed )
{
UIXDropdownEntry*		pNewEntry = new UIXDropdownEntry( this, szElementName, ulElementParam, bGreyed );

	mDropdownEntries.push_back( pNewEntry );
	return( pNewEntry );
}

const UIXDropdownEntry*		UIXDropdown::GetElementByListIndex( int index ) const
{
	if ( index < (int)mDropdownEntries.size() )
	{
		return( mDropdownEntries[index] );
	}
	return( NULL );
}