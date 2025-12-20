	

#include "StandardDef.h"
#include "InterfaceEx.h"
#include "UIXCustomRender.h"


void	UIXCustomRender::Initialise( fnCustomRenderCallback renderFunc, uint32 ulUserParam1, uint32 ulUserParam2 )
{
	mfnRenderCallback = renderFunc;
	mulUserParam1 = ulUserParam1;
	mulUserParam2 = ulUserParam2;
	
}
	
UIXRECT		UIXCustomRender::OnRender( InterfaceInstance* pInterface, UIXRECT displayRect )
{
UIXRECT		localRect = GetDisplayRect();
UIXRECT		renderRect = localRect;
UIXRECT		occupyRect;

	renderRect.x += displayRect.x;
	renderRect.y += displayRect.y;

	occupyRect = mfnRenderCallback( pInterface, renderRect, mulUserParam1, mulUserParam2 );

	displayRect.h = 0;//
	displayRect.y = localRect.y + occupyRect.h;	
	return displayRect;
}


