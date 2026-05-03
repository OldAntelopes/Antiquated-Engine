
#include "StandardDef.h"
#include "InterfaceEx.h"
#include "UIX.h"
#include "UIXMenu.h"

UIXMenuItem*		mspExpandedMenuTopLayer = NULL;


void	UIXMenuItem::Initialise( const char* szText, fnSelectedCallback selectCallback, uint32 ulSelectParam )
{
	mText = szText;
	SetSelectedCallback( selectCallback, ulSelectParam );
}

UIXMenuItem*		UIXMenuItem::AddMenuItem( const char* szMenuItemName, fnSelectedCallback selectCallback, uint32 ulSelectParam )
{
UIXMenuItem*		pNewMenuItem = new UIXMenuItem(this, UIX::GetNextObjectID(), UIXRECT(0, 0, 0, 0));

	pNewMenuItem->Initialise(szMenuItemName, selectCallback, ulSelectParam );
	GetChildObjectList().push_back( pNewMenuItem );
	return( pNewMenuItem );
}

void		UIXMenuItem::OnCloseAllMenus()
{
	mbIsExpanded = false;
	mspExpandedMenuTopLayer = NULL;
}

bool		UIXMenuItem::OnSelected( int nButtonID, uint32 ulParam )
{
	if (GetChildObjectList().size() > 0)
	{
		if ( mbIsExpanded )
		{
			mbIsExpanded = false;
			mspExpandedMenuTopLayer = NULL;	
		}
		else
		{
			if ( mspExpandedMenuTopLayer != NULL )
			{
				mspExpandedMenuTopLayer->SetExpanded( false );
				mspExpandedMenuTopLayer = NULL;
			}
			mbIsExpanded = true;
			mspExpandedMenuTopLayer = this;
		}
	}
	return( true );
}

void		UIXMenuItem::OnEscape()
{
	mbIsExpanded = false;
	mspExpandedMenuTopLayer = NULL;
}


void		UIXMenuItem::DoRender( InterfaceInstance* pInterface, UIXRECT rect )
{
	int		nNumChildren = GetChildObjectList().size();
	UIXMenuItem* pMenuItem;
	int		stringWidth;

	for ( UIXObject* pItem : GetChildObjectList() )
	{
		pMenuItem = (UIXMenuItem*)pItem;
		stringWidth = pInterface->GetStringWidth( pMenuItem->GetText(), 0 ) + 1;
		if ( rect.w < stringWidth + 25 )
		{
			rect.w = stringWidth + 25;
		}
	}

	rect.h = nNumChildren * 20 + 4;
	pInterface->ShadedRect(0, rect.x, rect.y, rect.w, rect.h, 0xFF202020, 0xFF202020, 0xFF181818, 0xFF181818);
	rect.y += 4;
	for ( UIXObject* pItem : GetChildObjectList() )
	{
		pMenuItem = (UIXMenuItem*)pItem;
		rect.h = 16;
		uint32		ulTextCol = 0xd0d0d0d0;
		uint32		ulHilightCol = 0x80808080;

		// If no sub-items and no selection callback, this menu item is greyed out
		if ( ( pMenuItem->GetChildObjectList().size() == 0 ) &&
			 ( pMenuItem->HasSelectionCallback() == false ) )
		{
			ulTextCol = 0x80808080;
		}
		else if (UIX::IsMouseHover(rect))
		{
			pInterface->Rect(1, rect.x, rect.y, rect.w, rect.h, ulHilightCol);
			ulTextCol = 0xe0e0e0e0;
			UIX::CheckForPress(pMenuItem, rect, UIX_MENU_ITEM, 0);
		}

		if ( pMenuItem->mbIsChecked )
		{
			UIXRECT iconRect(rect.x, rect.y - 2, 19, 19 );//rect.h - 4);
			
			UIX::DrawIcon( pInterface, 11, iconRect, 0xd0d0d0d0 );
//			pInterface->Rect(1, rect.x + 3, rect.y + 2, 12, rect.h - 4, 0xd02030b0 );	
		}
		pInterface->Text(0, rect.x + 20, rect.y + 1, ulTextCol, 0, pMenuItem->GetText());
		rect.y += 18;
	}
}

//---------------------------------------------------------------------

UIXMenuItem*	UIXMenu::AddMenuItem( const char* szMenuItemName, int nSubMenuWidth )
{
UIXMenuItem*		pNewMenuItem = new UIXMenuItem(this, UIX::GetNextObjectID(), UIXRECT(0, 0, 0, 0));

	pNewMenuItem->Initialise(szMenuItemName);
	pNewMenuItem->SetSubMenuWidth( nSubMenuWidth );
	GetChildObjectList().push_back( pNewMenuItem );
	return( pNewMenuItem );
}

void	UIXMenu::Initialise( )
{
}

UIXRECT		UIXMenu::OnRender( InterfaceInstance* pInterface, UIXRECT displayRect )
{
UIXRECT		renderRect = GetActualRenderRect( displayRect );
int		stringWidth;
UIXRECT		selectRect;
uint32		ulTextCol = 0xD0C0C0C0;
uint32		ulHilightCol = 0x80808080;

	pInterface->ShadedRect( 0, renderRect.x, renderRect.y, renderRect.w, renderRect.h, 0x90303030,0x90303030, 0x90282828, 0x90282828 );

	renderRect.x += 5;
	renderRect.w -= 5;
	renderRect.y += 7;
	renderRect.h -= 7;

	for ( UIXObject* pChild : GetChildObjectList() )
	{
		UIXMenuItem* pMenuItem = (UIXMenuItem*)pChild;
		stringWidth = pInterface->GetStringWidth( pMenuItem->GetText(), 3 ) + 1;

		selectRect = renderRect;
		selectRect.x -= 3;
		selectRect.w = stringWidth + 6;
		selectRect.y -= 1;

		pMenuItem->SetLastRenderRect(selectRect);
		if ( UIX::IsMouseHover( selectRect ) )
		{
			pInterface->Rect( 0, selectRect.x, selectRect.y, selectRect.w, selectRect.h, ulHilightCol );	
			ulTextCol = 0xe0e0e0e0;

			UIX::CheckForPress( pMenuItem, selectRect, UIX_MENU_ITEM, 0 );
		}
		pInterface->Text(0, renderRect.x, renderRect.y, ulTextCol, 3, pMenuItem->GetText() );

		renderRect.x += stringWidth + 15;
		renderRect.w -= stringWidth + 15;
	}

	displayRect.y = 0;
	displayRect.h = 0;
	return displayRect;
}

void		UIXMenu::OnPostRender( InterfaceInstance* pInterface, UIXRECT rect )
{
	for ( UIXObject* pChild : GetChildObjectList() )
	{
		UIXMenuItem* pMenuItem = (UIXMenuItem*)pChild;
		if (pMenuItem->IsExpanded())
		{
			UIXRECT		itemRect = pMenuItem->GetLastRenderRect();
			itemRect.y += itemRect.h;
			if ( itemRect.w < 150 ) itemRect.w = 150;
			itemRect.h = pInterface->GetHeight() - (itemRect.y + 10);

			itemRect.w += 20;
			pMenuItem->DoRender( pInterface, itemRect );
		}
	}

}

