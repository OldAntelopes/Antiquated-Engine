
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
		mnPressPosY = NOTFOUND;

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
			nScreenEpsilon = InterfaceGetHeight() / 100;
			if ( nScreenEpsilon < 5 ) nScreenEpsilon = 5;
			// todo - should be if swipe is in general direction of up-down
			// If moved..
			if ( ( nHoldY < mnPressPosY - nScreenEpsilon ) ||
				 ( nHoldY > mnPressPosY + nScreenEpsilon ) )
			{
			int nDiff = mnPressPosY - nHoldY;
			int	nMaxOffset = mnFullHeight - mnScreenH;

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
	int					mnPressPosY;

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

void		UIScrollablePageRender( int nHandle, int ScreenX, int ScreenY, int ScreenW, int ScreenH, int nFullH )
{
UIScrollablePage*		pPage = UIScrollablePageFind( nHandle );

	if ( pPage )
	{
		pPage->mnScreenX = ScreenX;
		pPage->mnScreenY = ScreenY;
		pPage->mnScreenW = ScreenW;
		pPage->mnScreenH = ScreenH;
		pPage->mnFullHeight = nFullH;
		pPage->mnFullWidth = 0;
		pPage->mnLastRenderFrame = mnScrollablePageCurrentRenderFrame;
	}
}

void		UIScrollablePageRenderHorizontal( int nHandle, int ScreenX, int ScreenY, int ScreenW, int ScreenH, int nFullW )
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
				pPage->mnPressPosY = Y;
				msfScrollablePageHoldTime = 0.0f;
				msbScrollablePageHold = TRUE;
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
		if ( pPage->mnPressPosY != NOTFOUND )
		{
			if ( msfScrollablePageHoldTime > 0.5f )
			{
				nRet = pPage->RecalcPosition( X, Y );
			}
		
			pPage->mnPressPosX = NOTFOUND;
			pPage->mnPressPosY = NOTFOUND;
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

	PlatformGetCurrentHoldPosition( &X, &Y );

	if ( msbScrollablePageHold )
	{
		msfScrollablePageHoldTime += fDelta;
	}
	mnScrollablePageCurrentRenderFrame++;

	while( pPage )
	{
		if ( pPage->mnPressPosY != NOTFOUND )
		{
			if ( msfScrollablePageHoldTime > 0.5f )
			{
				pPage->RecalcPosition( X, Y );
			}
		}
		pPage = pPage->mpNext;
	}

}
