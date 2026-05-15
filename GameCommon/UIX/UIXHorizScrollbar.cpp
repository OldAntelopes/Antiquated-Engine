
#include "StandardDef.h"
#include "InterfaceEx.h"

#include "../UI/UI.h"
#include "UIXHorizScrollbar.h"
//extern void		UIScrollablePageRender( int nHandle, int ScreenX, int ScreenY, int ScreenW, int ScreenH, int nFullH );
//extern void		UIScrollablePageRenderHorizontal( int nHandle, int ScreenX, int ScreenY, int ScreenW, int ScreenH, int nFullW );
//extern int		UIScrollablePageGetPosition( int nHandle );
//extern void		UIScrollablePageDestroy( int nHandle );

void	UIXHorizScrollbar::Initialise( fnControlCallback controlCallback, int nVal, int nViewRange, int nMaxRange )
{
	mfnControlCallback = controlCallback;
	mnViewRange = nViewRange;
	mnMaxRange = nMaxRange;
	mnScrollPosition = nVal;
}


UIXHorizScrollbar::~UIXHorizScrollbar()
{
}


void	UIXHorizScrollbar::RenderScrollbar( InterfaceInstance* pInterface, UIXRECT rect)
{
int		nScrollbarBoxX;
int		nScrollbarBoxY;
int		nScrollbarBoxW;
int		nScrollbarBoxH;
int		nBarX, nBarY, nBarW, nBarH;
int		nMaxBarW;
float	fBarScale;


	nScrollbarBoxX = rect.x + 1;
	nScrollbarBoxY = rect.y + 1;
	nScrollbarBoxW = rect.w - 2;
	nScrollbarBoxH = rect.h - 2;
	pInterface->OutlineBox( 0, nScrollbarBoxX, nScrollbarBoxY, nScrollbarBoxW, nScrollbarBoxH, 0xd0202020 );

	nBarX = nScrollbarBoxX + 2;
	nBarY = nScrollbarBoxY + 3;

	nBarH = nScrollbarBoxH - 5;
	nMaxBarW = nScrollbarBoxW - 4;

	fBarScale = (float)( mnViewRange ) / (float)( mnMaxRange );

	if ( fBarScale > 1.0f ) fBarScale = 1.0f;
	nBarW = (int)( nMaxBarW * fBarScale );

	int		nMaxMinusView = mnMaxRange - mnViewRange;
	float	fScrollPos = 0.0f;

	if ( nMaxMinusView > 0 )
	{
		fScrollPos = (float)mnScrollPosition / (float)nMaxMinusView;
		fScrollPos = FClamp( fScrollPos, 0.0f, 1.0f );
	}
	int nBarOffsetX = (int)( fScrollPos * (nMaxBarW-nBarW) );
	nBarX += nBarOffsetX;

	if ( nBarW < 10 ) nBarW = 10;
	pInterface->Rect( 1, nBarX, nBarY, nBarW, nBarH, 0xc0202020 );

	mScrollbarLastRender = UIXRECT( nBarX, nBarY, nBarW, nBarH );

	if ( UIX::IsMouseHover( nBarX, nBarY, nBarW, nBarH ) == TRUE )
	{
		int		nMouseX, nMouseY;
		UIGetCurrentCursorPosition( &nMouseX, &nMouseY );
		UIHoverIDSet( UIX_HORIZ_SCROLLBAR, 0, GetID() );
		mnHoverOffsetX = nMouseX - nBarX;
	}
}

UIXRECT		UIXHorizScrollbar::OnRender( InterfaceInstance* pInstance, UIXRECT displayRect )
{
UIXRECT		renderRect = GetActualRenderRect( displayRect );
UIXRECT		occupyRect = displayRect;

	RenderScrollbar( pInstance, renderRect );

	occupyRect.h = 0;//GetLocalPositionRect().h + 1;
	occupyRect.y = GetLocalPositionRect().y + GetLocalPositionRect().h + 1;
	return occupyRect;
}

void		UIXHorizScrollbar::OnUpdate( float fDelta )
{
	if ( mfnControlCallback )
	{
		mnScrollPosition = mfnControlCallback( this, FALSE, mnScrollPosition, &mnMaxRange, &mnViewRange );
	}
}

BOOL		UIXHorizScrollbar::HoldHandler( uint32 ulElementIndex, BOOL bIsHeld, BOOL bFirstPress )
{
	int		nMouseX, nMouseY;
	UIGetCurrentCursorPosition( &nMouseX, &nMouseY );

	UIXRECT	barRect = mScrollbarLastRender;
	int		nMaxBarW = barRect.w;

	if ( bFirstPress )
	{
		mbDidGrabScrollbar = TRUE;
		mnPressPosOffsetX = mnHoverOffsetX;
		mnPressPosScreenX = nMouseX;
		mnHoldStartScrollPosScreen = barRect.x;
		mnHoldStartScrollPos = mnScrollPosition;
	}
	else if ( bIsHeld )
	{
		mnHoldMoveDistanceX = nMouseX - mnPressPosScreenX;

		// Compute the track width and bar width from stored render rect extents
		// mScrollbarLastRender holds the bar; track starts at (box origin+2)
		// We need to derive nMaxBarW (track width) = scrollbar box width - 4
		// Since we don't store track separately, compute from callback or derive from bar scale
		float fBarScale = ( mnMaxRange > 0 ) ? (float)mnViewRange / (float)mnMaxRange : 1.0f;
		if ( fBarScale > 1.0f ) fBarScale = 1.0f;
		int nBarW = barRect.w;
		int nTrackW = ( fBarScale > 0.0f ) ? (int)( nBarW / fBarScale ) : nBarW;

		int nNewBarX = mnHoldStartScrollPosScreen + mnHoldMoveDistanceX;
		int nTrackStartX = mnHoldStartScrollPosScreen - (int)( ( (float)mnHoldStartScrollPos / (float)( mnMaxRange - mnViewRange ) ) * ( nTrackW - nBarW ) );

		float fScrollFrac = 0.0f;
		int nSlideRange = nTrackW - nBarW;
		if ( nSlideRange > 0 )
		{
			fScrollFrac = (float)( nNewBarX - nTrackStartX ) / (float)nSlideRange;
			fScrollFrac = FClamp( fScrollFrac, 0.0f, 1.0f );
		}

		int nMaxMinusView = mnMaxRange - mnViewRange;
		mnScrollPosition = (int)( fScrollFrac * nMaxMinusView );

		if ( mfnControlCallback )
		{
			mfnControlCallback( this, TRUE, mnScrollPosition, &mnMaxRange, &mnViewRange );
		}
	}
	else
	{
		mbDidGrabScrollbar = FALSE;
	}
	return( FALSE );
}

BOOL		UIXHorizScrollbar::HoldHandlerStatic( int nButtonID, uint32 ulParam, uint32 ulIndex, BOOL bIsHeld, BOOL bFirstPress )
{
	UIXHorizScrollbar* pScrollbar = (UIXHorizScrollbar*)UIX::FindUIXObjectByID( ulIndex );
	if ( pScrollbar )
	{
		return( pScrollbar->HoldHandler( ulParam, bIsHeld, bFirstPress ) );
	}
	return( FALSE );
}

void		UIXHorizScrollbar::RegisterControlHandlers()
{
	UIRegisterHoldHandler( UIX_HORIZ_SCROLLBAR, HoldHandlerStatic );
}
