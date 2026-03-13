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
short		mwUIRightPressX = 0;
short		mwUIRightPressY = 0;
int			mnUISetCursorX = NOTFOUND;
int			mnUISetCursorY = NOTFOUND;

int			mnUIButtonIDPressed = NOTFOUND;
uint32		mulUIButtonIDPressedParam = 0;
uint32		mulUIButtonIDPressedIDParam = 0;
int			mnUIRightButtonIDPressed = NOTFOUND;
uint32		mulUIRightButtonIDPressedParam = 0;
uint32		mulUIRightButtonIDPressedIDParam = 0;

int			mnUIButtonIDHovered = NOTFOUND;
uint32		mulUIButtonIDHoveredParam = 0;
uint32		mulUIButtonIDHoveredID = 0;

int			mnUIButtonIDHeld = NOTFOUND;
uint32		mulUIButtonIDHeldParam = 0;
uint32		mulUIButtonIDHeldID = 0;

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
	mnUIRightButtonIDPressed = NOTFOUND;
	mnUIButtonIDHovered = NOTFOUND;
	UITextBoxNewFrame();
	UIButtonsNewFrame();
	UIDropdownNewFrame();

	UIScrollablePageUpdate( fDelta );

	if ( mnUIButtonIDHeld != NOTFOUND )
	{
		if ( msHoldHandlerList[mnUIButtonIDHeld])
		{			
			msHoldHandlerList[mnUIButtonIDHeld]( mnUIButtonIDHeld, mulUIButtonIDHeldParam, mulUIButtonIDHeldID, TRUE, FALSE );
		}
	}

}


BOOL		UIOnZoom( float fZoomAmount )
{
	UIScrollablePageOnZoom( fZoomAmount );
	return( FALSE );
}

BOOL		UIOnRightButtonPress( int X, int Y )
{
	mwUIRightPressX = X;
	mwUIRightPressY = Y;
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
				msHoldHandlerList[mnUIButtonIDHovered]( mnUIButtonIDHovered, mulUIButtonIDHoveredParam, mulUIButtonIDHoveredID, TRUE, TRUE );
				mnUIButtonIDHeld = mnUIButtonIDHovered;
				mulUIButtonIDHeldParam = mulUIButtonIDHoveredParam;
				mulUIButtonIDHeldID = mulUIButtonIDHoveredID;
			}
		}
	}
	
	UIScrollablePageOnPress( X, Y );

	return( FALSE );
}

BOOL		UIOnReleaseRightButton( int X, int Y )
{
BOOL		bRet = FALSE;

	if ( mnUIRightButtonIDPressed != NOTFOUND )
	{
		if ( msButtonHandlerList[mnUIRightButtonIDPressed] )
		{			
			msButtonHandlerList[mnUIRightButtonIDPressed]( mnUIRightButtonIDPressed, mulUIRightButtonIDPressedParam, mulUIRightButtonIDPressedIDParam );
		}
		mnUIRightButtonIDPressed = NOTFOUND;
		mulUIRightButtonIDPressedParam = 0;
		bRet = TRUE;
	}

	mwUIRightPressX = 0;
	mwUIRightPressY = 0;
	return( bRet );
}

BOOL		UIOnRelease( int X, int Y )
{
BOOL	bRet = FALSE;
bool	bHoldHandlerWasActioned = false;

	if ( mnUIButtonIDHeld != NOTFOUND )
	{
		if ( msHoldHandlerList[mnUIButtonIDHeld])
		{			
			if ( msHoldHandlerList[mnUIButtonIDHeld]( mnUIButtonIDHeld, mulUIButtonIDHeldParam, mulUIButtonIDHeldID, FALSE, FALSE ) )
			{
				mnUIButtonIDHeld = NOTFOUND;
				return( TRUE );
			}
		}
		mnUIButtonIDHeld = NOTFOUND;
		bRet = TRUE;		
	}

	if ( UIScrollablePageOnRelease( X, Y ) == 0 )
	{
		if ( ( UIDropdownOnRelease( X, Y ) == FALSE ) &&
			 ( UISliderOnRelease( X, Y, TRUE ) == FALSE ) &&
			 ( UITextBoxOnRelease( X, Y ) == FALSE ) )
		{
			if ( mnUIButtonIDPressed != NOTFOUND )
			{
				if ( msButtonHandlerList[mnUIButtonIDPressed] )
				{			
					// If we press another button, we should cancel any text box edits in progress first..
					UITextBoxEndCurrentEdit();

					msButtonHandlerList[mnUIButtonIDPressed]( mnUIButtonIDPressed, mulUIButtonIDPressedParam, mulUIButtonIDPressedIDParam );
				}
				else if ( msHoldHandlerList[mnUIButtonIDPressed])
				{			
					msHoldHandlerList[mnUIButtonIDPressed]( mnUIButtonIDPressed, mulUIButtonIDPressedParam, mulUIButtonIDHoveredID, FALSE, FALSE );
				}
				 mnUIButtonIDPressed = NOTFOUND;
				 mulUIButtonIDPressedIDParam = 0;
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

void		UISetActiveInterface( InterfaceInstance* pInterfaceInstance )
{
	mpInterfaceInstance = pInterfaceInstance;
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


uint32	UIGetCurrentHoverIDIndexParam()
{
	return( mulUIButtonIDHoveredID );
}

uint32	UIGetCurrentPressIDIndexParam()
{
	return(mulUIButtonIDPressedIDParam);
}

void		UIHoverIDSet( int nButtonID, uint32 ulParam, uint32 ulID )
{
	mnUIButtonIDHovered = nButtonID;
	mulUIButtonIDHoveredParam = ulParam;
	mulUIButtonIDHoveredID = ulID;
}


void		UIRightPressIDSet( int nButtonID, uint32 ulParam, uint32 ulIDParam )
{
	mnUIRightButtonIDPressed = nButtonID;
	mulUIRightButtonIDPressedParam = ulParam;
	mulUIRightButtonIDPressedIDParam = ulIDParam;
}

void		UIPressIDSet( int nButtonID, uint32 ulParam, uint32 ulIDParam )
{
	mnUIButtonIDPressed = nButtonID;
	mulUIButtonIDPressedParam = ulParam;
	mulUIButtonIDPressedIDParam = ulIDParam;
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

BOOL		UIIsRightPressed( int X, int Y, int W, int H )
{
	if ( ( mwUIRightPressX > X ) &&
		 ( mwUIRightPressX < X + W ) &&
		 ( mwUIRightPressY > Y ) &&
		 ( mwUIRightPressY < Y + H ) )
	{
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
