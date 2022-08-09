
#include "StandardDef.h"
#include "Interface.h"

#include "../Platform/Platform.h"

#include "UISlider.h"
#include "UIScrollablePage.h"
#include "UITextBox.h"
#include "UIButton.h"
#include "UIDropdown.h"
#include "UI.h"

short		mwUIPressX = 0;
short		mwUIPressY = 0;

int			mnUIButtonIDPressed = NOTFOUND;
ulong		mulUIButtonIDPressedParam = 0;

class UIButtonHandlerList
{
public:
	int						mnButtonID;
	UIButtonHandler			mpfnButtonHandler;
	
	UIButtonHandlerList*	mpNext;

};

UIButtonHandlerList*	mpButtonHandlerList = NULL;


void		UIRegisterButtonPressHandler( int nButtonID, UIButtonHandler fnButtonHandler )
{
UIButtonHandlerList*		pHandlerList = mpButtonHandlerList;

	while( pHandlerList )
	{
		if ( pHandlerList->mnButtonID == nButtonID )
		{
			pHandlerList->mpfnButtonHandler = fnButtonHandler;
			return;
		}
		pHandlerList = pHandlerList->mpNext;
	}

	pHandlerList = new UIButtonHandlerList;
	pHandlerList->mpNext = mpButtonHandlerList;
	mpButtonHandlerList = pHandlerList;

	pHandlerList->mnButtonID = nButtonID;
	pHandlerList->mpfnButtonHandler = fnButtonHandler;
}




void		UIOnInterfaceDraw( void )
{
	UITextBoxNewFrame();
	UIButtonsNewFrame();
	UIDropdownNewFrame();
}


void		UIUpdate( float fDelta )
{
	mnUIButtonIDPressed = NOTFOUND;
	UITextBoxNewFrame();
	UIButtonsNewFrame();
	UIDropdownNewFrame();

	UIScrollablePageUpdate( fDelta );
}


BOOL		UIOnZoom( float fZoomAmount )
{
	UIScrollablePageOnZoom( fZoomAmount );
	return( FALSE );
}

BOOL		UIOnPress( int X, int Y )
{
	if ( UISliderOnPress( X, Y ) == FALSE )
	{
		mwUIPressX = X;
		mwUIPressY = Y;

		UIDropdownOnPress( X, Y );
	}
	
	UIScrollablePageOnPress( X, Y );

	return( FALSE );
}

BOOL		UIOnRelease( int X, int Y )
{
BOOL	bRet = FALSE;

	if ( UIScrollablePageOnRelease( X, Y ) == 0 )
	{
		if ( ( UISliderOnRelease( X, Y, TRUE ) == FALSE ) &&
			 ( UITextBoxOnRelease( X, Y ) == FALSE ) &&
			 ( UIDropdownOnRelease( X, Y ) == FALSE ) )
		{
			if ( mnUIButtonIDPressed != NOTFOUND )
			{
			UIButtonHandlerList*		pHandlerList = mpButtonHandlerList;

				while( pHandlerList )
				{
					if ( pHandlerList->mnButtonID == mnUIButtonIDPressed )
					{
						// If we press another button, we should cancel any text box edits in progress first..
						UITextBoxEndCurrentEdit();

						pHandlerList->mpfnButtonHandler( mnUIButtonIDPressed, mulUIButtonIDPressedParam );
						break;
					}
					pHandlerList = pHandlerList->mpNext;
				}

				 mnUIButtonIDPressed = NOTFOUND;
				 bRet = TRUE;
			}
		}
		else
		{
			bRet = TRUE;
		}
	}
	else
	{
		UISliderOnRelease( X, Y, FALSE );
		bRet = TRUE;
	}

	mwUIPressX = 0;
	mwUIPressY = 0;

	return( bRet );
}


void		UIInitialise( void )
{
	UIButtonsInitialise();
}

void		UIShutdown( void )
{
UIButtonHandlerList*		pHandlerList = mpButtonHandlerList;
UIButtonHandlerList*		pNext;

	while( pHandlerList )
	{
		pNext = pHandlerList->mpNext;
		delete pHandlerList;
		pHandlerList = pNext;
	}

	mpButtonHandlerList = NULL;

	UIButtonsShutdown();
	UITextboxShutdown();
}


void		UIPressIDSet( int nButtonID, ulong ulParam )
{
	mnUIButtonIDPressed = nButtonID;
	mulUIButtonIDPressedParam = ulParam;
}

void		UIHoverItem( int X, int Y, int W, int H )
{
int		hoverX, hoverY;

	PlatformGetCurrentCursorPosition( &hoverX, &hoverY );
	if ( ( hoverX > X ) &&
		 ( hoverX < X + W ) &&
		 ( hoverY > Y ) &&
		 ( hoverY < Y + H ) )
	{
		PlatformSetMouseOverCursor( TRUE );
	}

}

BOOL		UIIsPressed( int X, int Y, int W, int H )
{
	if ( ( mwUIPressX > X ) &&
		 ( mwUIPressX < X + W ) &&
		 ( mwUIPressY > Y ) &&
		 ( mwUIPressY < Y + H ) )
	{
		return( TRUE );
	}
	return( FALSE );
}


void		UIReleaseGraphicsForDeviceReset( void )
{

}

void		UIInitGraphicsPostDeviceReset( void )
{

}
