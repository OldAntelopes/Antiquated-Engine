
#include "StandardDef.h"
#include "InterfaceEx.h"
#include "../UI/UI.h"
#include "UIXCollapsableSection.h"


void	UIXCollapsableSection::Initialise( UIXRECT headerRect, int mode, const char* szTitle, BOOL bStartCollapsed, int draggableType )
{
	mTitle = szTitle;	
	mbIsCollapsed = bStartCollapsed;
	mMode = mode;
	mHeaderRect = headerRect;
	SetDraggable(draggableType, 0);
}


void	UIXCollapsableSection::ToggleCollapsed()
{
	mbIsCollapsed = !mbIsCollapsed;
}

bool	UIXCollapsableSection::OnSelected( int nButtonID, uint32 ulParam )
{
	ToggleCollapsed();
	return( false );
}

void	UIXCollapsableSection::UpdateUIStateData( UIStateData* pData )
{
	pData->mbTabOpened = !mbIsCollapsed;
}


UIXRECT		UIXCollapsableSection::OnRender( InterfaceInstance* pInterface, UIXRECT displayRect )
{
UIXRECT		renderRect = GetActualRenderRect( displayRect );
int	X = renderRect.x + mHeaderRect.x;
int	Y = renderRect.y;
int	W = renderRect.w - mHeaderRect.w;
int	H = renderRect.h;
uint32		ulCol = 0x90303030;
uint32		ulTextCol = 0xd0d0d0d0;
int		headerH = 20;
int		nHeaderOffsetX = 0;
int			nHeaderW = W;
BOOL	bMouseIsOverSectionHeader = FALSE;

	switch( mMode )
	{
	case 1:
		ulCol = 0x90303040;
		nHeaderOffsetX += 20;
		break;
	case 2:
		ulCol = 0x90404040;
		ulTextCol = 0xd0f0c080;
		nHeaderOffsetX += 5;
//		nHeaderW -= 44;
//		headerH = 20;
		break;
	default:
		break;
	}

	mLastHeaderRender = UIXRECT( X,Y, nHeaderW, headerH );
	pInterface->Rect( 0, X, Y, nHeaderW, headerH, ulCol );
		
	if ( UIX::IsMouseHover( X, Y, nHeaderW, headerH ) == TRUE )
	{
		UIHoverIDSet( UIX_COLLAPSABLE_SECTION_HEADER, 0, GetID() );
		bMouseIsOverSectionHeader = TRUE;
		
		if ( HasRightClickSelectionCallback() )
		{
			UIX::CheckForRightButtonPress(this, UIXRECT(X, Y, nHeaderW, headerH), UIX_RIGHT_CLICK_SELECT, 0);
		}	
	}

//	UIButtonRegion( UIX_COLLAPSABLE_SECTION_HEADER, X, Y, W, headerH, GetID() );

	if ( CheckDragHoverRegion( UIXRECT( X, Y, nHeaderW, headerH ) ) )
	{
		pInterface->Rect( 0, X, Y + headerH, nHeaderW, 2, 0xd08080c0 );	
		UIX::HoverAcceptDragItem( this );
	}
	// TODO - (Optionally) Use the child content height to allow the drag region to cover the whole of the expanded collapsable seciton

	pInterface->Text( 1, X + 20 + nHeaderOffsetX, Y + 4, ulTextCol, 3, mTitle.c_str() );

	if ( mbCollapsable )
	{
		if ( !mbIsCollapsed )
		{
			pInterface->Triangle( 1, X + 4 + nHeaderOffsetX, Y + 8, X + 12 + nHeaderOffsetX, Y + 8, X + 8 + nHeaderOffsetX, Y + 16, 0xa0a0a0a0, 0xa0a0a0a0, 0xa0a0a0a0 );	
		}
		else
		{
			pInterface->Triangle( 1, X + 4 + nHeaderOffsetX, Y + 6, X + 10 + nHeaderOffsetX, Y + 10, X + 4 + nHeaderOffsetX, Y + 14, 0xa0a0a0a0, 0xa0a0a0a0, 0xa0a0a0a0 );	
		}
	}

	displayRect.h = headerH + 1;
	displayRect.y = headerH + 1;		// displayRect.y returns the lowest point we drew to

	if (IsDragHoldActive() )
	{
	UIXRECT	xGrabOffset = GetDragOffset();
	UIXRECT xNewRect = GetInitialDragRect();

		xNewRect.x += xGrabOffset.x;
		xNewRect.y += xGrabOffset.y;

		pInterface->Rect( 1, xNewRect.x, xNewRect.y, xNewRect.w, xNewRect.h, 0x40303030 );
		pInterface->Text( 2, xNewRect.x + 5, xNewRect.y + 2, 0x80a0a0a0, 0, mTitle.c_str() );
	}
	return displayRect;
}

BOOL	UIXCollapsableSection::OnDragHoldUpdate( uint32 ulElementIndex, BOOL bIsHeld, BOOL bFirstPress )
{
	if (IsDraggable() )
	{
		if ( !bFirstPress && !bIsHeld )// Just released
		{
			if ( ( IsDragHoldActive() == FALSE ) ||
				 ( UIX::GetDragDestinationHover() == this ))
			{
				// If cursor has remained within the original section rect, treat it as a press (collapse the tab)
				UIX::CheckForPress( this, mLastHeaderRender, UIX_COLLAPSABLE_SECTION_HEADER, 0 );		
			}
		}
	}
	else
	{
		// Just released
		if ( !bFirstPress && !bIsHeld )// Just released
		{
			UIX::CheckForPress( this, mLastHeaderRender, UIX_COLLAPSABLE_SECTION_HEADER, 0 );
		}		 
	}
	return( FALSE );
}


