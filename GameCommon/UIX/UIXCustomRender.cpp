	

#include "StandardDef.h"
#include "InterfaceEx.h"
#include "UIXCustomRender.h"


void	UIXCustomRender::Initialise( fnCustomRenderCallback renderFunc, uint32 ulUserParam, fnCustomDragHoldHandlerCallback dragFunc)
{
	mfnRenderCallback = renderFunc;
	mulUserParam = ulUserParam;
	mfnCustomDragHolderHandlerCallback = dragFunc;

}
	
UIXRECT		UIXCustomRender::OnRender( InterfaceInstance* pInterface, UIXRECT displayRect )
{
UIXRECT		renderRect = GetActualRenderRect( displayRect );
UIXRECT		occupyRect;

// Note renderRect is passed as a ref here to allow the customrenderer to modify the region that would subsequently be checked for receiving drag items (e.g. if the custom renderer only draws in a portion of the displayRect, it can shrink the renderRect to that portion, and only that portion will be checked for receiving drag items)
	occupyRect = mfnRenderCallback( this, pInterface, renderRect, mulUserParam );
	
	if (CheckDragHoverRegion(renderRect))
	{
		pInterface->OutlineBox(0, renderRect.x, renderRect.y, renderRect.w, renderRect.h, 0xd08080c0);
		UIX::HoverAcceptDragItem(this);
	}

	displayRect.h = 0;  // This is the position relative to our parent that any children will start drawing at
	displayRect.y = occupyRect.y + GetLocalPositionRect().y;	
	return displayRect;
}



BOOL	UIXCustomRender::OnDragHoldUpdate(uint32 ulIndex, BOOL bIsHeld, BOOL bFirstPress)
{
	if (mfnCustomDragHolderHandlerCallback)
	{
		return(mfnCustomDragHolderHandlerCallback(this, ulIndex, bIsHeld, bFirstPress));
	}
	return(FALSE);
}

