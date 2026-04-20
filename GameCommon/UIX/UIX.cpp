
#include <stdarg.h>
#include "StandardDef.h"

#include "InterfaceEx.h"

#include "../Platform/Platform.h"
#include "../UI/UI.h"
#include "UIX.h"
#include "UIXButton.h"
#include "UIXDropdown.h"
#include "UIXListBox.h"
#include "UIXPage.h"
#include "UIXSlider.h"
#include "UIXDropdown.h"
#include "UIXShape.h"
#include "UIXCheckbox.h"
#include "UIXTextBox.h"
#include "UIXText.h"
#include "UIXPopupMenu.h"
#include "UIXCustomRender.h"
#include "UIXCollapsableSection.h"
#include "UIXScrollableSection.h"
#include "UIXMenu.h"
#include "UIXTabBar.h"
#include "UIXModalPopup.h"

uint32						UIX::msulNextObjectID = 2001;
std::vector<UIXPage*>		UIX::msPagesList;
std::map<uint32, UIXObject*>	UIX::msComponentIDMap;
int							UIX::msDragItemType = 0;
UIXObject*					UIX::mspDragDestinationHover = NULL;
UIXObject*					UIX::mspDragSource = NULL;	
UIXObject*					UIX::mspModalObject = NULL;
UIXObject*					UIX::mspMousewheelHoverObject = NULL;
UIXObject*					UIX::mspFocusedSelectionObject = NULL;

UIXObject*					UIX::mspTextEditFocusObject = NULL;
UIXRECT						UIX::mxActivePageRegion;
int							UIX::msSelectionPriority = 0;
int							UIX::msPressedSelectionPriority = 0;
int							UIX::msMouseWheelHoverPriority = 0;
int							UIX::mshUIXIconOverlays[MAX_NUM_UIX_ICONS] = { NOTFOUND };
int							UIX::mshUIXIconsList[MAX_NUM_UIX_ICONS] = { NOTFOUND };
std::string					UIX::msDragText;
std::string					UIX::msTooltipText;
int							UIX::msnTooltipPriority = NOTFOUND;
BOOL						UIX::mbUISelectionModeActive = FALSE;
fnObjectSelectionCallback	UIX::msfnObjectSelectionHandler = NULL;


uint32						UIX::msDragSourceParam = 0;
//--------------------------------------------------------------------------------------
UIXObject::UIXObject( UIXObject* pParent, uint32 uID, UIXRECT rect )
{
	mpParent = pParent;
	mulID = uID;
	mDisplayRect = rect;
	UIX::msComponentIDMap.emplace( uID, this );

}

UIXObject::~UIXObject()
{
	if ( mbDeleteUserObjectOnDestroy )
	{
		SAFE_DELETE( mpUserObject );
	}
	// nOTE THIS ALWAYS SHOULD GET DONE THROUGH uix::dELETEoBJECT
}

void	UIXObject::SelectObject( int nButtonID, uint32 ulParam )
{
	if ( nButtonID == UIX_RIGHT_CLICK_SELECT )
	{
		if ( mfnRightClickSelectedCallback )
		{
			mfnRightClickSelectedCallback( this, mulRightClickSelectParam );
		}	
		else
		{
			if ( UIX::mspFocusedSelectionObject == this )
			{
				UIX::mspFocusedSelectionObject = NULL;
			}
			else
			{
				UIX::mspFocusedSelectionObject = this;
			}
		}
	}
	else
	{
		// HACK: If OnSelected returns false it indicates it may have deleted the object we were using
		//  so we don't do any further callbacks now. This is unpleasant - needs refactoring
		if ( OnSelected( nButtonID, ulParam ) )
		{
			if ( mfnSelectedCallback )
			{
				mfnSelectedCallback( this, mulSelectParam );
			}
		}
	}
}

void	UIXObject::KeyUp(int keyCode)
{
	switch (keyCode)
	{
	case KEY_ESCAPE:
		OnEscape();
		break;
	default:
		break;
	}

	for (UIXObject* pContainedObject : mContainsList)
	{
		pContainedObject->KeyUp(keyCode);
	}
}

void	UIXObject::Update( float delta )
{
	OnUpdate( delta );
	for ( UIXObject* pContainedObject : mContainsList )
	{
		pContainedObject->Update( delta );
	}
}

void	UIXObject::CloseAllDropdowns( uint32 ulExceptID )
{
	OnCloseAllDropdowns( ulExceptID );
	for ( UIXObject* pContainedObject : mContainsList )
	{
		pContainedObject->CloseAllDropdowns(ulExceptID);
	}
}

void	UIXObject::CloseAllMenus()
{
	OnCloseAllMenus();
	for ( UIXObject* pContainedObject : mContainsList )
	{
		pContainedObject->CloseAllMenus();
	}
}

// The actual position something is drawn at is
//   LocalPositionRect x,y  ( Which is set at init time, relative to its parent. e.g. An object's localPosition is 0,0 if it is to be rendered at the top left corner of its parent's display area
//    +
//   ParentRect x,y
//
//  This func also deals with cases where the x or w components are set to negative numbers which indicates that they're right-aligned to the parent rect.
//  (i.e. a localPosition.x of -10 means we draw at 10px from the right hand side, w = -30 means all the width except 30 is used)

UIXRECT		UIXObject::GetActualRenderRect( UIXRECT parentRect )
{
UIXRECT		localRect = GetLocalPositionRect();		// This is our local position, relative to 0,0 within whatever container we're in
UIXRECT		renderRect = localRect;

	if ( renderRect.x < 0 )
	{
		renderRect.x = parentRect.x + (parentRect.w + renderRect.x);	
	}
	else
	{
		renderRect.x += parentRect.x;
	}
	renderRect.y += parentRect.y;

	if ( renderRect.w < 0 )
	{
		renderRect.w = (parentRect.w + (renderRect.w+1)) - localRect.x;
	}

	return renderRect;
}

void	UIXObject::PostRender( InterfaceInstance* pInterface )
{
	UIX::msSelectionPriority += GetSelectionPriorityLayer();
	OnPostRender( pInterface, mLastRenderDisplayRect );

	if ( !mContainsList.empty() && ShouldDisplayChildren() )
	{
		for ( UIXObject* pContainedObject : mContainsList )
		{
			pContainedObject->PostRender( pInterface );
		}
	}
	UIX::msSelectionPriority -= GetSelectionPriorityLayer();
}


UIXRECT	UIXObject::Render( InterfaceInstance* pInterface, UIXRECT displayRect )
{
UIXRECT		xUsedRect = displayRect;
UIXRECT		xRect;
UIXRECT		xMaxRect;
bool		bIncludeChildOccupyHeight = IncludeChildrenInOccupyCalc();
int			baseOccupyHeight;
UIXRECT		actualRenderRect = GetActualRenderRect( displayRect );

	xUsedRect.y = 0;
	xUsedRect.h = 0;

	mLastRenderDisplayRect = displayRect;

	UIX::msSelectionPriority += GetSelectionPriorityLayer();

	if ( UIX::IsMouseHover( actualRenderRect, FALSE ) == TRUE )
	{
		// If mouseHover long enough, show tooltip text if we have one
		mfHoverTime += PlatformGetFrameDelta();
		if ( mfHoverTime > 1.0f )
		{
		const char*		pcTooltip = GetTooltipText();

			if ( pcTooltip != NULL )
			{
				UIX::SetActiveTooltip( UIX::msSelectionPriority, pcTooltip );
			}
			else
			{
				// Reset the hover time if we have no tooltip to show
				mfHoverTime = 0.0f;
			}
		}
	}
	else
	{
		mfHoverTime = 0.0f;
	}

	xRect = OnRender( pInterface, displayRect );
		// The y and h values returned from the render function are used

		// If the item has children or siblings, the h value is first used to update
		// the current position (displayRect) the children/siblings start drawing at..
		// so returning an h value effectively blocks out (occupies) some of the 
		// vertical display space.

		// The y value tells the system how much vertical space the render actually used and
		// is returned up to the parent


	xUsedRect.w = xRect.w;		// w is reduced by things like scrollbars occupying space within the page
	xUsedRect.h = xRect.h;
	xUsedRect.y = xRect.y;
	xMaxRect.y = xRect.y;
	baseOccupyHeight = xUsedRect.h;

	if ( !mContainsList.empty() && ShouldDisplayChildren() )
	{
	UIXRECT		xChildDisplayRect = GetActualRenderRect( displayRect );
	int			nCursRelY = xMaxRect.y;
		
		xChildDisplayRect.y += xUsedRect.h;// + mDisplayRect.y;
		if ( xUsedRect.w < 0 ) 
		{
			xChildDisplayRect.w += xUsedRect.w;
		}
		xChildDisplayRect.h -= xUsedRect.h;

		xChildDisplayRect.y -= GetScrollPosition();

		for ( UIXObject* pContainedObject : mContainsList )
		{
			xRect = pContainedObject->Render( pInterface, xChildDisplayRect );

			// MaxRect.y maintains the largest y value drawn to, relative to the DisplayRect, including any space occupied by the main object & children
			if ( xRect.y + nCursRelY > xMaxRect.y )
			{
				xMaxRect.y = xRect.y + nCursRelY;		
			}
			// UsedRect.h contains the cumulative height occupied by all children + the main object
			xUsedRect.h += xRect.h;
			// CursRelY contains the cumulative height occupied by all children
			nCursRelY += xRect.h;
			// childDisplayRect.y is the current draw position for the next child, and is updated as each child occupies space with its returned h value
			xChildDisplayRect.y += xRect.h;
			// childDisplayRect.h is the remaining space in the main objects original render rect, and is updated as each child occupies space with its returned h value
			xChildDisplayRect.h -= xRect.h;
		}

		int presetChildBlockSize = GetLocalPositionRect().h;

		if ( bIncludeChildOccupyHeight )
		{
			if ( xMaxRect.y > xUsedRect.y )
			{
				xUsedRect.y = xMaxRect.y;
			}
			if ( xUsedRect.y > xUsedRect.h )
			{
				xUsedRect.h = xUsedRect.y;
			}

			// MaxRect contains the lowest point that was drawn to by children including those bits that dont change the cursor
			// and will tell us if we need to expand the box
			if ( xMaxRect.y > presetChildBlockSize )
			{
				if ( xMaxRect.y > xUsedRect.h )
				{
					xUsedRect.h = xMaxRect.y;
				}
			}
			mChildContentsHeight = xUsedRect.h;
		}
		else
		{
			xUsedRect.h = baseOccupyHeight;
			mChildContentsHeight = 0;
		}
		OnPostChildrenRender( pInterface );
	}
	UIX::msSelectionPriority -= GetSelectionPriorityLayer();
	return( xUsedRect );
}

void	UIXObject::SetDraggable(int nDragItemType, uint32 ulDragParam)
{
	mDragItemType = nDragItemType;
	mDragItemParam = ulDragParam;
}


bool	UIXObject::DoesContainObjectID( uint32 ulUIXID ) const
{
	if ( ulUIXID == GetID() )
	{
		return true;
	}

	if ( mContainsList.size() > 0 )
	{
		for( auto contained : mContainsList )
		{
			if ( contained->DoesContainObjectID( ulUIXID ) )
			{
				return true;
			}
		}
	}
	return false;
}

void	UIXObject::ActivateDragHold( UIXRECT rect, uint32 ulDragParam )
{
	mDragRectOriginal = rect;
	mbIsBeingDragged = TRUE;
	UIX::SetDragItemType(mDragItemType, this, ulDragParam);
	UIGetCurrentCursorPosition(&mDragRectMouseOriginal.x, &mDragRectMouseOriginal.y);
}


UIXRECT		UIXObject::GetDragOffset()
{
int		nMouseX, nMouseY;

	UIGetCurrentCursorPosition(&nMouseX, &nMouseY);
	
	return(UIXRECT(nMouseX - mDragRectMouseOriginal.x, nMouseY - mDragRectMouseOriginal.y, 0, 0) );
}

BOOL		UIXObject::IsFocusedObject()
{
	return (UIX::GetFocusedObject() == this); 
}

BOOL		UIXObject::DragHasMoved()
{
	int	nMouseX, nMouseY;

	// Check we've moved at least a bit away from the click pos otherwise its not really a drag
	UIGetCurrentCursorPosition(&nMouseX, &nMouseY);
	int nMoveDist = abs(mDragRectMouseOriginal.x - nMouseX) + abs(mDragRectMouseOriginal.y - nMouseY);

	// Arbitrarily, mouse must have moved at least 4px to count as a drag move
	if (nMoveDist > 4)
	{
		return(TRUE);
	}
	return(FALSE);
}

BOOL		UIXObject::HoldHandler(uint32 ulParam, BOOL bIsHeld, BOOL bFirstPress)
{
	BOOL	bCustomHandler = OnDragHoldUpdate(ulParam, bIsHeld, bFirstPress);

	if (bCustomHandler == FALSE)
	{
		if (mDragItemType != 0)
		{
			if (bFirstPress)
			{
				ActivateDragHold( mDraggableRenderRect, mDragItemParam);
			}
			else if (bIsHeld)
			{

			}
			else  // Just released
			{
				if ((mbIsBeingDragged) &&
					(UIX::GetDragDestinationHover() != NULL) &&
					(UIX::GetDragDestinationHover() != this))
				{
					mbIsBeingDragged = FALSE;
					UIX::EndDragItemType(mDragItemType);
					return(TRUE);
				}
				mbIsBeingDragged = FALSE;
				UIX::EndDragItemType(mDragItemType);
			}
		}
	}
	return(bCustomHandler);
}

BOOL		UIXObject::HoldHandlerStatic(int nButtonID, uint32 ulParam, uint32 ulIDParam, BOOL bIsHeld, BOOL bFirstPress)
{
UIXObject* pObject = UIX::FindUIXObjectByID(ulIDParam);

	if (pObject)
	{
		return(pObject->HoldHandler(ulParam, bIsHeld, bFirstPress));
	}
	return(FALSE);
}

const char*		UIXObject::GetTooltipText()
{
	if (mfnCustomTooltipCallback)
	{
		return(mfnCustomTooltipCallback(this, mulCustomTooltipParam));
	}
	return(NULL);
}

void	UIXObject::RegisterDragControlHandler( int nButtonID )
{
	UIRegisterHoldHandler(nButtonID, HoldHandlerStatic);
}

BOOL	UIXObject::CheckDragHoverRegion( UIXRECT dragReceiveRegion )
{
int	type = UIX::GetDragItemType();

	if ( ( type != 0 ) &&
		 ( CanReceiveDragItem(type) ) )
	{
		if ( UIX::IsMouseHover( dragReceiveRegion ) )
		{
			OnHoverDragItem(type);
			return( TRUE );
		}
	}
	return( FALSE );
}

void	UIXObject::Shutdown()
{
	OnShutdown();

	for ( UIXObject* pContainedObject : mContainsList )
	{
		pContainedObject->Shutdown();
		UIX::DeleteObject( pContainedObject );
	}
	mContainsList.clear();
}

void		UIXObject::OnReceiveDragItem( int dragType, UIXObject* pxSourceObject, uint32 ulDragParam, const char* szDragFilename )
{
	if ( mDragMap[dragType] )
	{
		mDragMap[dragType](pxSourceObject, ulDragParam, this, mDragMapParams[dragType], szDragFilename );
	}
}


void		UIXObject::SetDragReceiveCallback( int dragType, fnDragReceiveCallback func, uint32 ulDragDestParam )
{
	mDragMap[dragType] = func;
	mDragMapParams[dragType] = ulDragDestParam;
}


BOOL	UIX::IsRectInActivePageRegion(UIXRECT rect)
{
	if (rect.x + rect.w < mxActivePageRegion.x) return(FALSE);
	if (rect.x > mxActivePageRegion.x + mxActivePageRegion.w) return(FALSE);
	if (rect.y + rect.h < mxActivePageRegion.y) return(FALSE);
	if (rect.y > mxActivePageRegion.y + mxActivePageRegion.h) return(FALSE);
	return(TRUE);
}

BOOL	UIX::IsInActivePageRegion(int x, int y)
{
	if (x < mxActivePageRegion.x) return(FALSE);
	if (x > mxActivePageRegion.x + mxActivePageRegion.w) return(FALSE);
	if (y < mxActivePageRegion.y) return(FALSE);
	if (y > mxActivePageRegion.y + mxActivePageRegion.h) return(FALSE);
	return(TRUE);
}

//--------------------------------------------------------------------------------------------------
// IDParam must always be UIXObject's unique ID ( obj->GetID() )  to use this handler
//
void		UIX::ButtonPressHandler( int nButtonID, uint32 ulParam, uint32 ulIDParam )
{
	auto it = msComponentIDMap.find( ulIDParam );
	if ( it != msComponentIDMap.end() )
	{
	UIXObject* pObject = it->second;

		// This is the mode we use when doing 'Edit midi mapping' and we're just interested 
		if ( nButtonID == UIX_OBJECT_SELECT )
		{
			mspFocusedSelectionObject = pObject;
			if (msfnObjectSelectionHandler)
			{
				msfnObjectSelectionHandler(pObject);
			}
		}
		else
		{
			pObject->SelectObject( nButtonID, ulParam );
		}
	}
}
	
uint32	UIX::GetCurrentPressObjectID() 
{ 
	return( UIGetCurrentPressIDIndexParam() );
}


void	UIX::OnKeyUp(int keyCode)
{
	if ( mspFocusedSelectionObject )
	{
		mspFocusedSelectionObject->OnFocusedKeyUp( keyCode );
	}

	switch (keyCode)
	{
	case KEY_ESCAPE:
		for (UIXPage* pxObjects : msPagesList)
		{
			pxObjects->KeyUp( keyCode );
		}
		break;
	default:
		break;
	}

}

void		UIX::OnMouseWheel( float fOffset )
{
	if ( mspMousewheelHoverObject )
	{
		mspMousewheelHoverObject->OnMouseWheel( fOffset );
	}
}

BOOL		UIX::SliderHoldHandler( int nButtonID, uint32 ulIndex, uint32 ulIDParam, BOOL bIsHeld, BOOL bFirstPress  )
{
UIXSlider*		pSlider;

	switch( nButtonID )
	{
	case UIX_SLIDER_BAR_MAXRANGE:
		pSlider = (UIXSlider*)msComponentIDMap[ulIDParam];
		if ( pSlider )
		{
			pSlider->OnMaxRangeHeldUpdate( bIsHeld, bFirstPress );
		}
		break;
	case UIX_SLIDER_BAR_MINRANGE:
		pSlider = (UIXSlider*)msComponentIDMap[ulIDParam];
		if ( pSlider )
		{
			pSlider->OnMinRangeHeldUpdate( bIsHeld, bFirstPress );
		}
		break;
	case UIX_SLIDER_BAR:
		pSlider = (UIXSlider*)msComponentIDMap[ulIDParam];
		if ( pSlider )
		{
			pSlider->OnHeldUpdate( bIsHeld, bFirstPress );
		}
		break;
	default:
		break;
	}

	return( FALSE );
}

void		UIX::SetMousewheelHoverObject(UIXObject* pObject)
{
	if (msSelectionPriority >= msMouseWheelHoverPriority)
	{
		mspMousewheelHoverObject = pObject;
		msMouseWheelHoverPriority = msSelectionPriority;
	}


}

BOOL		UIX::CheckForRightButtonPress( UIXObject* pxObject, UIXRECT rect, uint32 ulButtonID, uint32 ulButtonParam )
{
	if ( msSelectionPriority >= msPressedSelectionPriority )
	{
		if ( IsRectInActivePageRegion( rect ) )
		{
			if ( UIIsRightPressed( rect.x, rect.y, rect.w, rect.h ) == TRUE )
			{
				UIRightPressIDSet( ulButtonID, ulButtonParam, pxObject->GetID() );
				msPressedSelectionPriority = msSelectionPriority;
			}
			return( IsMouseHover( rect ) );
		}
	}
	return( FALSE );
}

#ifdef _DEBUG
#define DEBUG_COMPONENT_MAP
#endif

BOOL		UIX::CheckForPress( UIXObject* pxObject, UIXRECT rect, uint32 ulButtonID, uint32 ulButtonParam )
{
	if ( msSelectionPriority >= msPressedSelectionPriority )
	{
		if ( IsRectInActivePageRegion( rect ) )
		{
			if ( UIIsPressed( rect.x, rect.y, rect.w, rect.h ) == TRUE )
			{
#ifdef DEBUG_COMPONENT_MAP
				auto it = msComponentIDMap.find( pxObject->GetID() );
				if ( it == msComponentIDMap.end() )
				{
					SysDebugPrint( "Error - Pressed UIX component ID does not exist in the current componentIDMap. This shouldnt ever happen");
				}
#endif
				UIPressIDSet( ulButtonID, ulButtonParam, pxObject->GetID() );
				msPressedSelectionPriority = msSelectionPriority;
			}

			return( IsMouseHover( rect ) );
		}
	}
	return( FALSE );
}

void		UIX::OnInterfaceDraw()
{
	for (int loop = 0; loop < MAX_NUM_UIX_ICONS; loop++)
	{
		mshUIXIconOverlays[loop] = NOTFOUND;
	}
	UIOnInterfaceDraw();
}

void		UIX::Initialise( int mode )
{
	UIRegisterButtonPressHandler( UIX_COLLAPSABLE_SECTION_HEADER, ButtonPressHandler );
	UIRegisterButtonPressHandler( UIX_DROPDOWN_HEADER, ButtonPressHandler );
	UIRegisterButtonPressHandler( UIX_DROPDOWN_ENTRY, ButtonPressHandler );
	UIRegisterButtonPressHandler( UIX_CHECKBOX, ButtonPressHandler );
	UIRegisterButtonPressHandler( UIX_LISTBOX_SELECT, ButtonPressHandler );
	UIRegisterButtonPressHandler( UIX_TEXTBOX, ButtonPressHandler );
	UIRegisterButtonPressHandler( UIX_MENU_ITEM, ButtonPressHandler );
	UIRegisterButtonPressHandler( UIX_POPUP_MENU_ITEM, ButtonPressHandler );
	UIRegisterButtonPressHandler( UIX_TAB_SELECT, ButtonPressHandler );
	UIRegisterButtonPressHandler( UIX_RIGHT_CLICK_SELECT, ButtonPressHandler );
	UIRegisterButtonPressHandler( UIX_OBJECT_SELECT, ButtonPressHandler );
	
			
	UIRegisterHoldHandler( UIX_SLIDER_BAR, SliderHoldHandler );
	UIRegisterHoldHandler( UIX_SLIDER_BAR_MINRANGE, SliderHoldHandler );
	UIRegisterHoldHandler( UIX_SLIDER_BAR_MAXRANGE, SliderHoldHandler );

	// These two do custom hold handling (e.g. for scrollbars, value scroll)
	UIXTextBox::RegisterControlHandlers();
	UIXScrollableSection::RegisterControlHandlers();

	// -----------------------------------------------
	// ----- Object types implementing drag and drop --
	UIXCollapsableSection::RegisterDragControlHandler(UIX_COLLAPSABLE_SECTION_HEADER);
	UIXListBox::RegisterDragControlHandler(UIX_LISTBOX);
	UIXButton::RegisterDragControlHandler(UIX_BUTTON);
	UIXCustomRender::RegisterDragControlHandler(UIX_CUSTOM_RENDER);
	UIXTextBox::RegisterDragControlHandler(UIX_TEXTBOX);

	InterfaceSetDrawCallback(OnInterfaceDraw );

}

void		UIX::Update( float delta )
{
	for( UIXPage* pxObjects : msPagesList )
	{
		pxObjects->Update( delta );
	}
}

void		UIX::Reset()
{
	for( UIXPage* pxObjects : msPagesList )
	{
		DeleteObject( pxObjects );
	}
	msPagesList.clear();
}


BOOL		UIX::IsMouseHover( int x, int y, int w, int h, BOOL bSetCursor )
{
	if (IsRectInActivePageRegion(UIXRECT(x, y, w, h)))
	{
		if ( bSetCursor )
		{
			return ( UIHoverItem(x, y, w, h) );	
		}
		else
		{
			return ( UIIsMouseHover(x, y, w, h) );
		}
	}
	return(FALSE);
}

BOOL		UIX::IsMouseHover( UIXRECT rect, BOOL bSetCursor )
{
	if ( IsRectInActivePageRegion(rect))
	{
		if ( bSetCursor )
		{
			return ( UIHoverItem( rect.x, rect.y, rect.w, rect.h) );	
		}
		else
		{
			return ( UIIsMouseHover( rect.x, rect.y, rect.w, rect.h ) );
		}
	}
	return( FALSE );
}

void		UIX::RenderTooltip( InterfaceInstance* pxInterface )
{
// todo 
	if ( msnTooltipPriority != NOTFOUND )
	{
	int		stringWidth = pxInterface->GetStringWidth( msTooltipText.c_str(), 3 );
	UIXRECT		rect;
	int		mouseX;
	int		mouseY;

		UIGetCurrentCursorPosition( &mouseX, &mouseY );

		rect.x = mouseX - (stringWidth/2);
		if ( rect.x < 5 ) rect.x = 5;

		if ( rect.x + stringWidth > pxInterface->GetWidth() )
		{
		int		overflow = (rect.x + stringWidth) - pxInterface->GetWidth();
			
			rect.x -= overflow + 5;
		}

		rect.y = mouseY + 25;
		if ( rect.y >= pxInterface->GetHeight() ) rect.y = mouseY - 35;

		rect.w = stringWidth + 10;
		rect.h = 20;

		pxInterface->Rect( 2, rect.x, rect.y, rect.w, rect.h, 0xd0101010 );
		pxInterface->OutlineBox( 2, rect.x, rect.y, rect.w, rect.h, 0xc0505050 );

		pxInterface->Text( 2, rect.x + 5, rect.y + 3, 0xe0f0f0f0, 3, msTooltipText.c_str() );
	}
	msnTooltipPriority = NOTFOUND;

}

void		UIX::SetActiveTooltip( int priority, const char* szText )
{
	if ( priority >= msnTooltipPriority )
	{
		msTooltipText = szText;
		msnTooltipPriority = priority;
	}
}

void		UIX::Render( InterfaceInstance* pxInterface )
{
UIXRECT		pageDisplayRect;

	mspDragDestinationHover = NULL;
	msSelectionPriority = 0;
	msPressedSelectionPriority = 0;
	msMouseWheelHoverPriority = 0;
	UIXRECT		rootRect( 0,0,pxInterface->GetWidth(),pxInterface->GetHeight());
	
	for( UIXPage* pxPage : msPagesList )
	{
		mxActivePageRegion = rootRect;
		pxPage->Render( pxInterface, rootRect );
	}

	mxActivePageRegion = UIXRECT( 0,0,pxInterface->GetWidth(),pxInterface->GetHeight());

	pxInterface->Draw();
	for( UIXPage* pxObjects : msPagesList )
	{
		pxObjects->PostRender( pxInterface );
	}
	// Debug stuff
//	pxInterface->Text( 0, 5, 20, 0xd0f02010, 0, "press-pri: %d", msPressedSelectionPriority );
	RenderTooltip(pxInterface);
}

void		UIX::CloseAllDropdowns( uint32 ulExceptID )
{
	for( UIXPage* pxPages : msPagesList )
	{
		pxPages->CloseAllDropdowns(ulExceptID);
	}

}

void		UIX::CloseAllMenus()
{
	for( UIXPage* pxPages : msPagesList )
	{
		pxPages->CloseAllMenus();
	}
}


void		UIX::Shutdown()
{
	for( UIXPage* pObject : msPagesList )
	{
		msComponentIDMap[pObject->GetID()] = NULL;
		msComponentIDMap.erase( pObject->GetID() );
		pObject->Shutdown();
		delete pObject;
	}
	msPagesList.clear();
}

void		UIX::DeleteObject( UIXObject* pObject )
{
		// todo - Get rid of this by using smart(er) pointers for things like the mousewheel hover
	// HACK - Should shift to smart pointers	
	if ( mspDragDestinationHover == pObject ) mspDragDestinationHover = NULL;
	if ( mspTextEditFocusObject == pObject ) mspTextEditFocusObject = NULL;
	if ( mspMousewheelHoverObject == pObject ) mspMousewheelHoverObject = NULL;
	if ( mspFocusedSelectionObject == pObject ) mspFocusedSelectionObject = NULL;

	msPagesList.erase( std::remove(msPagesList.begin(), msPagesList.end(), pObject), msPagesList.end() );
	msComponentIDMap[pObject->GetID()] = NULL;
	msComponentIDMap.erase( pObject->GetID() );
	// Shutdown clears up all the object's children
	pObject->Shutdown();
	delete pObject;
}

UIXObject*		UIX::FindUIXObjectByID( uint32 ulObjectID )
{
	auto it = msComponentIDMap.find( ulObjectID );
	if ( it != msComponentIDMap.end() )
	{
		return( it->second );
	}
	return( NULL );
}

void			UIX::DrawIcon( InterfaceInstance* pInterface, int iconNum, UIXRECT rect, uint32 ulCol )
{
	if ( ( iconNum >= 0 ) &&
		 ( iconNum < MAX_NUM_UIX_ICONS ) )
	{
	int		nIconPageNum = iconNum / 36;
	iconNum %= 36;
	float	fU = ((iconNum%6) * 19.0f) / 128.0f;
	float	fV = ((iconNum/6) * 19.0f) / 128.0f;
	float	fUW = 19.0f / 128.0f;
	float	fVH = 19.0f / 128.0f;
	
		if ( nIconPageNum > 0 )
		{
			fU = 0.0f;
			fV = 0.0f;
			fUW = 1.0f;
			fVH = 1.0f;
		}

		if ( mshUIXIconOverlays[nIconPageNum] == NOTFOUND )
		{
		int		hTexture = mshUIXIconsList[nIconPageNum];

			mshUIXIconOverlays[nIconPageNum] = pInterface->CreateNewTexturedOverlay( 2, hTexture );
		}
		pInterface->TexturedRect( mshUIXIconOverlays[nIconPageNum], rect.x, rect.y, rect.w, rect.h, ulCol, fU, fV, fU + fUW, fV + fVH );
	}	
}

void	UIX::LoadIconSheet( InterfaceInstance* pInterface, int sheetNum, const char* szFilename )
{
	mshUIXIconsList[sheetNum] = pInterface->GetTexture( szFilename, 0 );
}

void		UIX::LoadIcon( InterfaceInstance* pInterface, int iconNum, const char* szFilename )
{
	mshUIXIconsList[iconNum] = pInterface->GetTexture( szFilename, 0 );
}


UIXObject*		UIX::AddSubPage( UIXObject* pxContainer, UIXRECT rect, const char* szTitle, BOOL bUseClipping )
{
UIXPage*		pNewPage = new UIXPage( pxContainer, msulNextObjectID++, rect );

	pNewPage->Initialise( szTitle, bUseClipping );
	pxContainer->mContainsList.push_back( pNewPage );
	return( pNewPage );
}

UIXObject*		UIX::AddPage( UIXRECT rect, const char* szTitle, BOOL bUseClipping )
{
UIXPage*		pNewPage = new UIXPage( NULL, msulNextObjectID++, rect );

	pNewPage->Initialise( szTitle, bUseClipping );
	msPagesList.push_back( pNewPage );
	return( pNewPage );
}

void			UIX::SetTextEditFocus( UIXObject* pObject )
{
	// If we're switching to a new text box while another was being edited, save out the old one
	if ( ( mspTextEditFocusObject != NULL ) &&
		 ( pObject != NULL ) )
	{
		mspTextEditFocusObject->EndEdit();
	}
	mspTextEditFocusObject = pObject;
}



UIXText*		UIX::AddText( UIXObject* pxContainer, UIXRECT rect, uint32 ulCol, int font, UIX_TEXT_FLAGS fontFlags, const char* szTitle, ... )
{
UIXText*		pNewText = new UIXText( pxContainer, msulNextObjectID++, rect );
char		acString[1024];
va_list		marker;
uint32*		pArgs;

	pArgs = (uint32*)( &szTitle ) + 1;

    va_start( marker, szTitle );     
	vsprintf( acString, szTitle, marker );
	if ( ulCol == 0 ) ulCol = 0xd0d0d0d0;			// Default col is an offwhite 

	pNewText->Initialise( acString, ulCol, font, fontFlags );
	pxContainer->mContainsList.push_back( pNewText );
	return( pNewText );
}

UIXScrollableSection*		UIX::AddScrollableSection( UIXObject* pxContainer, UIXRECT rect )
{
UIXScrollableSection*		pNewScrollableSection = new UIXScrollableSection( pxContainer, msulNextObjectID++, rect );

	pNewScrollableSection->Initialise();
	pxContainer->mContainsList.push_back( pNewScrollableSection );
	return( pNewScrollableSection );
}

UIXCollapsableSection*		UIX::AddCollapsableSection( UIXObject* pxContainer, UIXRECT rect,  UIXRECT headerRect, int mode, const char* szTitle, BOOL bStartCollapsed, int draggableType )
{
UIXCollapsableSection*		pNewCollapsableSection = new UIXCollapsableSection( pxContainer, msulNextObjectID++, rect );

	pNewCollapsableSection->Initialise( headerRect, mode, szTitle, bStartCollapsed, draggableType );
	pxContainer->mContainsList.push_back( pNewCollapsableSection );
	return( pNewCollapsableSection );
}

UIXButton*			UIX::AddButton( UIXObject* pxContainer, UIXRECT rect, eUIXBUTTON_MODE mode, const char* szTitle, uint32 ulButtonID, uint32 ulButtonParam, BOOL bIsBlocking, uint32 ulCol, int iconNum  )
{
UIXButton*		pNewButton = new UIXButton( pxContainer, msulNextObjectID++, rect );

	pNewButton->Initialise( mode, szTitle, ulButtonID, ulButtonParam, bIsBlocking, ulCol, iconNum );
	pxContainer->mContainsList.push_back( pNewButton );
	return( pNewButton );
}

uint32		UIX::GetNextObjectID()
{
	return(msulNextObjectID++);
}

UIXCustomRender*	UIX::AddCustomRender( UIXObject* pxContainer, UIXRECT rect, fnCustomRenderCallback renderFunc, uint32 ulUserParam, fnCustomDragHoldHandlerCallback dragFunc)
{
UIXCustomRender*		pNewCustomRender = new UIXCustomRender( pxContainer, msulNextObjectID++, rect );

	pNewCustomRender->Initialise( renderFunc, ulUserParam, dragFunc );
	pxContainer->mContainsList.push_back( pNewCustomRender );
	return( pNewCustomRender );
}

UIXTabBar*		UIX::AddTabBar( UIXObject* pxContainer, UIXRECT rect )
{
UIXTabBar*		pNewTabBar = new UIXTabBar( pxContainer, msulNextObjectID++, rect );

	pNewTabBar->Initialise(0);
	pxContainer->mContainsList.push_back( pNewTabBar );
	return( pNewTabBar );

}

UIXPopupMenu*	UIX::AddPopupMenu( UIXObject* pxContainer, UIXRECT rect )
{
UIXPopupMenu*		pNewMenu = new UIXPopupMenu( pxContainer, msulNextObjectID++, rect );

	pNewMenu->Initialise(  );
	pxContainer->mContainsList.push_back( pNewMenu );
	return( pNewMenu );
}

UIXMenu*		UIX::AddMenuBar( UIXObject* pxContainer, UIXRECT rect )
{
UIXMenu*		pNewMenu = new UIXMenu( pxContainer, msulNextObjectID++, rect );

	pNewMenu->Initialise(  );
	pxContainer->mContainsList.push_back( pNewMenu );
	return( pNewMenu );
}

UIXModalPopup*		UIX::AddModalPopup( UIXObject* pxContainer, UIXRECT rect )
{
UIXModalPopup*		pNewModalPopup = new UIXModalPopup( pxContainer, msulNextObjectID++, rect );

	pNewModalPopup->Initialise(  );
	pxContainer->mContainsList.push_back( pNewModalPopup );
	return( pNewModalPopup );
}


UIXCheckbox*		UIX::AddCheckbox( UIXObject* pxContainer, UIXRECT rect, UIX_CHECKBOX_MODE mode, BOOL bIsChecked, const char* szText, fnSelectedCallback selectedFunc, uint32 ulSelectParam )
{
UIXCheckbox*		pNewCheckbox = new UIXCheckbox( pxContainer, msulNextObjectID++, rect );

	pNewCheckbox->Initialise( mode, bIsChecked, szText, selectedFunc, ulSelectParam );
	pxContainer->mContainsList.push_back( pNewCheckbox );
	return( pNewCheckbox );
}


UIXShape*			UIX::AddShape( UIXObject* pxContainer, UIXRECT rect, eUIXSHAPE_MODE mode, BOOL bBlocks, uint32 ulCol1, uint32 ulCol2, uint32 ulButtonID, uint32 ulButtonParam )
{
UIXShape*		pNewShape = new UIXShape( pxContainer, msulNextObjectID++, rect );

	pNewShape->Initialise( mode, bBlocks, ulCol1, ulCol2, ulButtonID, ulButtonParam );
	pxContainer->mContainsList.push_back( pNewShape );
	return( pNewShape );
}


UIXTextBox*			UIX::AddTextBox( UIXObject* pxContainer, UIXRECT rect, int mode, const char* szDefaultText )
{
UIXTextBox*		pNewTextBox = new UIXTextBox( pxContainer, msulNextObjectID++, rect );

	pNewTextBox->Initialise( mode, szDefaultText );
	pxContainer->mContainsList.push_back( pNewTextBox );
	return( pNewTextBox );
}

UIXListBox*			UIX::AddListBox( UIXObject* pxContainer, UIXRECT rect, int mode, BOOL bContentsDraggable, int dragItemType )
{
UIXListBox*		pNewListbox = new UIXListBox( pxContainer, msulNextObjectID++, rect );

	pNewListbox->Initialise( mode, bContentsDraggable, dragItemType );
	pxContainer->mContainsList.push_back( pNewListbox );
	return( pNewListbox );
}

UIXSlider*			UIX::AddSlider( UIXObject* pxContainer, UIXRECT rect, UIX_SLIDER_MODE mode, uint32 ulUserParam, float fMin, float fMax, float fInitial, float fMinStep, const char* szText, BOOL bShowTextBoxes )
{
UIXSlider*		pNewSlider = new UIXSlider( pxContainer, msulNextObjectID++, rect );

	pNewSlider->Initialise( mode, ulUserParam, fMin, fMax, fInitial, fMinStep, szText, bShowTextBoxes );
	pxContainer->mContainsList.push_back( pNewSlider );
	return( pNewSlider );
}

UIXDropdown*		UIX::AddDropdown( UIXObject* pxContainer, UIXRECT rect )
{
UIXDropdown*		pNewDropdown = new UIXDropdown( pxContainer, msulNextObjectID++, rect );

	pNewDropdown->Initialise( 0 );
	pxContainer->mContainsList.push_back( pNewDropdown );
	return( pNewDropdown );
}


void	UIX::EndDragItemType( int type ) 
{
	if ( msDragItemType == type )
	{
		if ( ( mspDragDestinationHover != NULL ) &&
			 ( mspDragSource != mspDragDestinationHover ) )
		{
			mspDragDestinationHover->OnReceiveDragItem( type, mspDragSource, msDragSourceParam);
			mspDragDestinationHover = NULL;
		}
		msDragItemType = 0; 
	}

}

void	UIStateData::OnUpdate( float fDelta )
{
	if ( mpAssociatedUIXObj ) mpAssociatedUIXObj->UpdateUIStateData( this );
}

