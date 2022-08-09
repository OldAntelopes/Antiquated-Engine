
#include "StandardDef.h"
#include "Interface.h"
#include "../Platform/Platform.h"
#include "UI.h"
#include "UIScrollablePage.h"


class UIScrollablePage
{
public:
	UIScrollablePage()
	{
		mnScreenX = 0;
		mnScreenY = 0;
		mnScreenW = 0;
		mnScreenH = 0;
		mnFullHeight = 0;
		mnFullWidth = 0;
		mnCurrentOffset = 0;
		mnLastRenderFrame = 0;
		mnPressPosX = NOTFOUND;
		mnPressPosScreenY = NOTFOUND;
		mnPressPosOffsetY = NOTFOUND;
		mnScrollbarDisplayX = NOTFOUND;
		mbDidGrabScrollbar = FALSE;
		mnHandle = 0;
		mpNext = NULL;
	}
	~UIScrollablePage()
	{

	}

	int			RecalcPosition( int nHoldX, int nHoldY )
	{
	int		nScreenEpsilon = 5;

		if ( mnFullHeight != 0 )
		{
			nScreenEpsilon = InterfaceGetHeight() / 150;
			if ( nScreenEpsilon < 4 ) nScreenEpsilon = 4;

			if ( mbDidGrabScrollbar )
			{
				nScreenEpsilon = 0;
			}

			// todo - should be if swipe is in general direction of up-down
			// If moved..
			if ( ( nHoldY < mnPressPosScreenY - nScreenEpsilon ) ||
				 ( nHoldY > mnPressPosScreenY + nScreenEpsilon ) )
			{
			int nScreenDiff = (int)( nHoldY - mnPressPosScreenY );
			int	nMaxOffset = mnFullHeight - mnScreenH;
			float	fOffsetScale = (float)( mnFullHeight ) / mnScreenH;
			
				mnCurrentOffset = (int)(nScreenDiff * fOffsetScale) + mnPressPosOffsetY;
				
				if ( mnCurrentOffset < 0 )
				{
					mnCurrentOffset = 0;
				}
				else if ( mnCurrentOffset > nMaxOffset )
				{
					mnCurrentOffset = nMaxOffset;
				}
				return( 1 );
			}
		}
		else  // horizontal
		{
			nScreenEpsilon = InterfaceGetWidth() / 100;
			if ( nScreenEpsilon < 5 ) nScreenEpsilon = 5;

			// todo - should be if swipe is in general direction of left-right
			// If moved..
			if ( ( nHoldX < mnPressPosX - nScreenEpsilon ) ||
				 ( nHoldX > mnPressPosX + nScreenEpsilon ) )
			{
			int nDiff = mnPressPosX - nHoldX;
			int	nMaxOffset = mnFullWidth - mnScreenW;

				mnCurrentOffset += nDiff;
				if ( mnCurrentOffset < 0 )
				{
					mnCurrentOffset = 0;
				}
				else if ( mnCurrentOffset > nMaxOffset )
				{
					mnCurrentOffset = nMaxOffset;
				}
				return( 1 );
			}
		}
		return( 0 );
	}

	int					mnScreenX;
	int					mnScreenY;
	int					mnScreenW;
	int					mnScreenH;
	int					mnFullHeight;
	int					mnFullWidth;

	int					mnCurrentOffset;
	int					mnLastRenderFrame;
	int					mnHandle;

	int					mnPressPosX;
	int					mnPressPosScreenY;
	int					mnPressPosOffsetY;
	BOOL				mbDidGrabScrollbar;
	int					mnScrollbarDisplayX;

	BOOL				mbScrollbarGrabHover;
	BOOL				mbScrollbarClickHover;

	UIScrollablePage*	mpNext;
};
//---------------------------------------------------------------------------------------------

UIScrollablePage*		mspScrollablePageList = NULL;
int			mnScrollablePageCurrentRenderFrame = 0;
int			msnScrollablePageNextHandle = 0x100;
float		msfScrollablePageHoldTime = 0.0f;
BOOL		msbScrollablePageHold = FALSE;

//---------------------------------------------------------------------------------------------

UIScrollablePage*		UIScrollablePageFind( int nHandle )
{
UIScrollablePage*		pPage = mspScrollablePageList;

	while( pPage )
	{
		if ( pPage->mnHandle == nHandle )
		{
			return( pPage );
		}
		pPage = pPage->mpNext;
	}
	return( NULL );	
}

int			UIScrollablePageCreate( void )
{
UIScrollablePage*		pNewPage = new UIScrollablePage;

	pNewPage->mpNext = mspScrollablePageList;
	mspScrollablePageList = pNewPage;
	
	pNewPage->mnHandle = msnScrollablePageNextHandle++;

	return( pNewPage->mnHandle );
}

BOOL		UIScrollablePageRender( int nHandle, int ScreenX, int ScreenY, int ScreenW, int ScreenH, int nFullH )
{
UIScrollablePage*		pPage = UIScrollablePageFind( nHandle );
BOOL	bHasScrollbar = FALSE;

	if ( pPage )
	{
	int		nCursorX, nCursorY;

		PlatformGetCurrentCursorPosition( &nCursorX, &nCursorY );

		pPage->mnScrollbarDisplayX = NOTFOUND;
		pPage->mbScrollbarGrabHover = FALSE;
		pPage->mbScrollbarClickHover = FALSE;

		if ( nFullH > ScreenH ) 
		{
			UIHoverItem( ScreenX, ScreenY, ScreenW, ScreenH );
		}
		pPage->mnScreenX = ScreenX;
		pPage->mnScreenY = ScreenY;
		pPage->mnScreenW = ScreenW;
		pPage->mnScreenH = ScreenH;
		pPage->mnFullHeight = nFullH;
		pPage->mnFullWidth = 0;
		pPage->mnLastRenderFrame = mnScrollablePageCurrentRenderFrame;

		if ( ScreenH < nFullH )
		{
		int		nScrollbarBoxX;
		int		nScrollbarBoxY;
		int		nScrollbarBoxW;
		int		nScrollbarBoxH;
		int		nBarX, nBarY, nBarW, nBarH;
		int		nMaxBarH;
		float	fBarScale;

			nScrollbarBoxX = ScreenX + ScreenW - 18;
			nScrollbarBoxY = ScreenY + 3;
			nScrollbarBoxW = 15;
			nScrollbarBoxH = ScreenH - 6;

			pPage->mnScrollbarDisplayX = nScrollbarBoxX;
			nBarX = nScrollbarBoxX + 2;
			nBarY = nScrollbarBoxY + 2;
			nBarW = nScrollbarBoxW - 4;
			nMaxBarH = nScrollbarBoxH - 4;

			fBarScale = (float)( ScreenH ) / (float)( nFullH );

			nBarY += (int)( pPage->mnCurrentOffset * fBarScale );

			if ( fBarScale > 1.0f ) fBarScale = 1.0f;
			nBarH = (int)( nMaxBarH * fBarScale );

			if ( nBarH < 10 ) nBarH = 10;

			InterfaceOutlineBox( 0, nScrollbarBoxX, nScrollbarBoxY, nScrollbarBoxW, nScrollbarBoxH, 0xd0a0a0a0 );

			InterfaceRect( 1, nBarX, nBarY, nBarW, nBarH, 0xc0b0b0b0 );
			bHasScrollbar = TRUE;

			if ( ( nCursorX >= nBarX ) &&
				 ( nCursorX <= nBarX + nBarW ) &&
				 ( nCursorY >= nBarY ) &&
				 ( nCursorY <= nBarY + nBarH ) )
			{
				pPage->mbScrollbarGrabHover = TRUE;
			}

		}
	}
	return( bHasScrollbar );
}

BOOL		UIScrollablePageRenderHorizontal( int nHandle, int ScreenX, int ScreenY, int ScreenW, int ScreenH, int nFullW )
{
UIScrollablePage*		pPage = UIScrollablePageFind( nHandle );

	if ( pPage )
	{
		pPage->mnScreenX = ScreenX;
		pPage->mnScreenY = ScreenY;
		pPage->mnScreenW = ScreenW;
		pPage->mnScreenH = ScreenH;
		pPage->mnFullHeight = 0;
		pPage->mnFullWidth = nFullW;
		pPage->mnLastRenderFrame = mnScrollablePageCurrentRenderFrame;
	}
	return( FALSE );
}

int			UIScrollablePageGetPosition( int nHandle )
{
UIScrollablePage*		pPage = UIScrollablePageFind( nHandle );
	
	if ( pPage )
	{
		if ( pPage->mnFullHeight <= pPage->mnScreenH )
		{
			pPage->mnCurrentOffset = 0;
		}
		return( pPage->mnCurrentOffset );
	}
	return( 0 );
}

void		UIScrollablePageOnZoom( float fZoomAmount )
{
UIScrollablePage*		pPage = mspScrollablePageList;

	while( pPage )
	{
		if ( pPage->mnLastRenderFrame > mnScrollablePageCurrentRenderFrame - 2 )
		{
		int		X;
		int		Y;

			PlatformGetCurrentCursorPosition( &X, &Y );

			if ( ( X >= pPage->mnScreenX ) &&
				 ( X <= pPage->mnScreenX + pPage->mnScreenW ) &&
				 ( Y >= pPage->mnScreenY ) &&
				 ( Y <= pPage->mnScreenY + pPage->mnScreenH ) )
			{
			int	nMaxOffset = pPage->mnFullHeight - pPage->mnScreenH;

				pPage->mnCurrentOffset += (int)( fZoomAmount * -32.0f );
				
				if ( pPage->mnCurrentOffset < 0 )
				{
					pPage->mnCurrentOffset = 0;
				}
				else if ( pPage->mnCurrentOffset > nMaxOffset )
				{
					pPage->mnCurrentOffset = nMaxOffset;
				}
			}
		}
		pPage = pPage->mpNext;
	}
}

int			UIScrollablePageOnPress( int X, int Y )
{
UIScrollablePage*		pPage = mspScrollablePageList;

	while( pPage )
	{
		if ( pPage->mnLastRenderFrame > mnScrollablePageCurrentRenderFrame - 2 )
		{
			if ( ( X >= pPage->mnScreenX ) &&
				 ( X <= pPage->mnScreenX + pPage->mnScreenW ) &&
				 ( Y >= pPage->mnScreenY ) &&
				 ( Y <= pPage->mnScreenY + pPage->mnScreenH ) )
			{
				pPage->mnPressPosX = X;
				pPage->mnPressPosScreenY = Y;
				pPage->mnPressPosOffsetY = pPage->mnCurrentOffset;
				msfScrollablePageHoldTime = 0.0f;
				msbScrollablePageHold = TRUE;

				if ( pPage->mbScrollbarGrabHover )
				{
					pPage->mbDidGrabScrollbar = TRUE;
				}
			}
		}
		pPage = pPage->mpNext;
	}
	return( 0 );
}

int			UIScrollablePageOnRelease( int X, int Y )
{
UIScrollablePage*		pPage = mspScrollablePageList;
int			nRet = 0;

	while( pPage )
	{
		if ( pPage->mnPressPosScreenY != NOTFOUND )
		{
			if ( msfScrollablePageHoldTime > 0.5f )
			{
				nRet = pPage->RecalcPosition( X, Y );
			}
		
			pPage->mnPressPosX = NOTFOUND;
			pPage->mnPressPosScreenY = NOTFOUND;
			pPage->mnPressPosOffsetY = NOTFOUND;
		}
		pPage = pPage->mpNext;
	}

	msfScrollablePageHoldTime = 0.0f;
	msbScrollablePageHold = FALSE;

	return( nRet );
}


void		UIScrollablePageDestroy( int nHandle )
{
UIScrollablePage*		pPage = mspScrollablePageList;
UIScrollablePage*		pLast = NULL;

	while( pPage )
	{
		if ( pPage->mnHandle == nHandle )
		{
			if ( pLast )
			{
				pLast->mpNext = pPage->mpNext;
			}
			else
			{
				mspScrollablePageList = pPage->mpNext;
			}
			delete pPage;
			return;
		}
		pPage = pPage->mpNext;
	}
}

void		UIScrollablePageUpdate( float fDelta )
{
UIScrollablePage*		pPage = mspScrollablePageList;
int	X, Y;

	PlatformGetCurrentCursorPosition( &X, &Y );

	if ( msbScrollablePageHold )
	{
		msfScrollablePageHoldTime += fDelta;
	}
	mnScrollablePageCurrentRenderFrame++;

	while( pPage )
	{
		if ( pPage->mnPressPosScreenY != NOTFOUND )
		{
			if ( ( msfScrollablePageHoldTime > 0.5f ) ||
				 ( pPage->mbDidGrabScrollbar ) )
			{
				pPage->RecalcPosition( X, Y );
			}
		}
		pPage = pPage->mpNext;
	}

}
