
#include "StandardDef.h"
#include "InterfaceEx.h"
#include "UIX.h"
#include "UIXPage.h"

void	UIXPage::Initialise( const char* szTitle, BOOL bUseClipping )
{
	mbUseClipping = bUseClipping;
	mTitle = szTitle;
}

BOOL		UIXPage::IsWithinPage(int x, int y)
{
	if ((x >= mPageRenderRect.x) &&
		 ( x < mPageRenderRect.x + mPageRenderRect.w ) &&
		 ( y >= mPageRenderRect.y ) &&
		 ( y < mPageRenderRect.y + mPageRenderRect.h ) )
	{
		return( TRUE );
	}
	return(FALSE);
}

void	UIXPage::OnUpdate(float delta)
{
	if ( mfnMouseWheelHandler )
	{
		if ( UIX::IsMouseHover( mPageRenderRect ) == TRUE )
		{
			UIX::SetMousewheelHoverObject( this );
		}
	}
}

void		UIXPage::OnMouseWheel(float fAmount)
{
	if (mfnMouseWheelHandler)
	{
		mfnMouseWheelHandler(fAmount);
	}
}

UIXRECT		UIXPage::OnRender( InterfaceInstance* pInterface, UIXRECT displayRect )
{
UIXRECT		renderRect = GetActualRenderRect( displayRect );

	mPageRenderRect = renderRect;
	UIX::SetActivePageRegion( mPageRenderRect );
	if ( mbUseClipping )
	{
		pInterface->DrawAllElements();
	//	pInterface->SetViewport( rect.x, rect.y, rect.w, rect.h );
		pInterface->SetRenderCanvas();
	}

	displayRect.y = 0;
	displayRect.h = 0;
	return displayRect;
}

void	UIXPage::OnPostChildrenRender( InterfaceInstance* pInterface )
{

	if ( mbUseClipping )
	{
		pInterface->DrawAllElements();
		pInterface->CopyRenderCanvasToBackBuffer( mPageRenderRect.x, mPageRenderRect.y, mPageRenderRect.w, mPageRenderRect.h );
	//	pInterface->SetViewport( 0, 0, pInterface->GetWidth(), pInterface->GetHeight() );
	}
}
