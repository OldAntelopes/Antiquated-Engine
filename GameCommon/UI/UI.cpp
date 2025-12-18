#include "DirectX/d3dx9.h"

#include <map>
#include "StandardDef.h"
#include "Interface.h"

#include "../../../Antiquated/Pub/LibCode/Interface/Common/InterfaceInstance.h"
#include "../Platform/Platform.h"


#include "UISlider.h"
#include "UIScrollablePage.h"
#include "UITextBox.h"
#include "UIListBox.h"
#include "UIButton.h"
#include "UIDropdown.h"
#include "UI.h"

InterfaceInstance*	mpInterfaceInstance = NULL;

short		mwUIPressX = 0;
short		mwUIPressY = 0;
int			mnUISetCursorX = NOTFOUND;
int			mnUISetCursorY = NOTFOUND;

int			mnUIButtonIDPressed = NOTFOUND;
uint32		mulUIButtonIDPressedParam = 0;
int			mnUIButtonIDHovered = NOTFOUND;
uint32		mulUIButtonIDHoveredParam = 0;

int			mnUIButtonIDHeld = NOTFOUND;
uint32		mulUIButtonIDHeldParam = 0;

std::map<int, UIButtonHandler>	msButtonHandlerList;
std::map<int, UIHoldHandler>	msHoldHandlerList;

void		UIRegisterHoldHandler( int nButtonID, UIHoldHandler fnHoldHandler )
{
	msHoldHandlerList[nButtonID] = fnHoldHandler;
}

void		UIRegisterButtonPressHandler( int nButtonID, UIButtonHandler fnButtonHandler )
{
	msButtonHandlerList[nButtonID] = fnButtonHandler;
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
	mnUIButtonIDHovered = NOTFOUND;
	UITextBoxNewFrame();
	UIButtonsNewFrame();
	UIDropdownNewFrame();

	UIScrollablePageUpdate( fDelta );

	if ( mnUIButtonIDHeld != NOTFOUND )
	{
		if ( msHoldHandlerList[mnUIButtonIDHeld])
		{			
			msHoldHandlerList[mnUIButtonIDHeld]( mnUIButtonIDHeld, mulUIButtonIDHeldParam, TRUE, FALSE );
		}
	}

}


BOOL		UIOnZoom( float fZoomAmount )
{
	UIScrollablePageOnZoom( fZoomAmount );
	return( FALSE );
}

BOOL		UIOnPress( int X, int Y )
{
	if ( ( UISliderOnPress( X, Y ) == FALSE ) &&
		 ( UIListBoxOnPress( X, Y ) == FALSE ) )

	{
		mwUIPressX = X;
		mwUIPressY = Y;

		UIDropdownOnPress( X, Y );

		if ( mnUIButtonIDHovered != NOTFOUND )
		{
			if ( msHoldHandlerList[mnUIButtonIDHovered] )
			{			
				msHoldHandlerList[mnUIButtonIDHovered]( mnUIButtonIDHovered, mulUIButtonIDHoveredParam, TRUE, TRUE );
				mnUIButtonIDHeld = mnUIButtonIDHovered;
				mulUIButtonIDHeldParam = mulUIButtonIDHoveredParam;
			}
		}
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
				if ( msButtonHandlerList[mnUIButtonIDPressed] )
				{			
					// If we press another button, we should cancel any text box edits in progress first..
					UITextBoxEndCurrentEdit();

					msButtonHandlerList[mnUIButtonIDPressed]( mnUIButtonIDPressed, mulUIButtonIDPressedParam );
				}
				else if ( msHoldHandlerList[mnUIButtonIDPressed])
				{			
					msHoldHandlerList[mnUIButtonIDPressed]( mnUIButtonIDPressed, mulUIButtonIDPressedParam, FALSE, FALSE );
				}
				 mnUIButtonIDPressed = NOTFOUND;
				 bRet = TRUE;
			}
			else if ( mnUIButtonIDHeld != NOTFOUND )
			{
				if ( msHoldHandlerList[mnUIButtonIDHeld])
				{			
					msHoldHandlerList[mnUIButtonIDHeld]( mnUIButtonIDHeld, mulUIButtonIDHeldParam, FALSE, FALSE );
				}
				mnUIButtonIDHeld = NOTFOUND;
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

InterfaceInstance*		UIInterfaceInstance()
{
	return( mpInterfaceInstance );
}

void		UIInitialise( InterfaceInstance* pInterfaceInstance )
{
	if ( pInterfaceInstance == NULL )
	{
		pInterfaceInstance = InterfaceInstanceMain();
	}
	mpInterfaceInstance = pInterfaceInstance;
	UIButtonsInitialise();
}

void		UIShutdown( void )
{
	// TODO - Cleanup msButtonHandlerList

	UIButtonsShutdown();
	UITextboxShutdown();
}

void		UIGetCurrentCursorPosition( int* pnX, int* pnY )
{
	if ( mnUISetCursorX == NOTFOUND )
	{
		PlatformGetCurrentCursorPosition( pnX, pnY );
	}
	else
	{
		*pnX = mnUISetCursorX;
		*pnY = mnUISetCursorY;
	}
}

void		UISetCurrentCursorPosition( int nX, int nY )
{
	mnUISetCursorX = nX;
	mnUISetCursorY = nY;
}


void		UIHoverIDSet( int nButtonID, uint32 ulParam )
{
	mnUIButtonIDHovered = nButtonID;
	mulUIButtonIDHoveredParam = ulParam;
}


void		UIPressIDSet( int nButtonID, uint32 ulParam )
{
	mnUIButtonIDPressed = nButtonID;
	mulUIButtonIDPressedParam = ulParam;
}

BOOL		UIHoverItem( int X, int Y, int W, int H )
{
int		hoverX, hoverY;

	UIGetCurrentCursorPosition( &hoverX, &hoverY );
	if ( ( hoverX > X ) &&
		 ( hoverX < X + W ) &&
		 ( hoverY > Y ) &&
		 ( hoverY < Y + H ) )
	{
		PlatformSetMouseOverCursor( TRUE );
		return( TRUE );
	}
	return( FALSE );
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
