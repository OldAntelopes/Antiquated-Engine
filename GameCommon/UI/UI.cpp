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

UIInstance		msTempSingleton;
UIInstance*		mspTempSingleton = &msTempSingleton;

void		UIInstance::RegisterHoldHandler( int nButtonID, UIHoldHandler fnHoldHandler )
{
	msHoldHandlerList[nButtonID] = fnHoldHandler;
}

void		UIRegisterHoldHandler( int nButtonID, UIHoldHandler fnHoldHandler )
{
	msTempSingleton.RegisterHoldHandler( nButtonID, fnHoldHandler );
}

void		UIInstance::RegisterButtonPressHandler( int nButtonID, UIButtonHandler fnButtonHandler )
{
	msButtonHandlerList[nButtonID] = fnButtonHandler;
}
void		UIRegisterButtonPressHandler( int nButtonID, UIButtonHandler fnButtonHandler )
{
	msTempSingleton.RegisterButtonPressHandler( nButtonID, fnButtonHandler );
}




void		UIOnInterfaceDraw( void )
{
//	UITextBoxNewFrame();
//	UIButtonsNewFrame();
//	UIDropdownNewFrame();
}

void		UIInstance::Update( float fDelta )
{
	mnUIButtonIDPressed = NOTFOUND;
	mnUIRightButtonIDPressed = NOTFOUND;
	mnUIButtonIDHovered = NOTFOUND;
	mpUITextBoxImpl->NewFrame();
	mpUIButtonImpl->NewFrame();
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
void		UIUpdate( float fDelta )
{
	msTempSingleton.Update(fDelta);
}


BOOL		UIInstance::OnZoom( float fZoomAmount )
{
	UIScrollablePageOnZoom( fZoomAmount );
	return( FALSE );
}
BOOL		UIOnZoom( float fZoomAmount )
{
	return( msTempSingleton.OnZoom( fZoomAmount ) );
}

BOOL		UIInstance::OnRightButtonPress( int X, int Y )
{
	mwUIRightPressX = X;
	mwUIRightPressY = Y;
	return( FALSE );
}
BOOL		UIOnRightButtonPress( int X, int Y )
{
	return( msTempSingleton.OnRightButtonPress( X, Y ) );
}
BOOL		UIInstance::OnPress( int X, int Y )
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

BOOL		UIOnPress( int X, int Y )
{
	return( msTempSingleton.OnPress(X,Y) );
}

void		UIInstance::ButtonDrawAlpha( int nButtonID, int nX, int nY, int nWidth, int nHeight, const char* szText, eUIBUTTON_MODE_FLAGS modeFlags, uint32 ulParam, uint32 ulIDParam, float fAlpha )
{
	mpUIButtonImpl->Draw( this, nButtonID, nX, nY, nWidth, nHeight, szText, modeFlags, ulParam, ulIDParam, fAlpha);
}

void		UIInstance::ButtonDraw( int nButtonID, int nX, int nY, int nWidth, int nHeight, const char* szText, eUIBUTTON_MODE_FLAGS modeFlags, uint32 ulParam, uint32 ulIDParam )
{
	ButtonDrawAlpha( nButtonID, nX, nY, nWidth, nHeight, szText, modeFlags, ulParam, ulIDParam, 1.0f );
}

BOOL		UIInstance::OnReleaseRightButton( int X, int Y )
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

BOOL		UIOnReleaseRightButton( int X, int Y )
{
	return( msTempSingleton.OnReleaseRightButton( X, Y ) );
}

BOOL		UIInstance::OnRelease( int X, int Y )
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
			 ( mpUITextBoxImpl->OnRelease( this, X, Y ) == FALSE ) )
		{
			if ( mnUIButtonIDPressed != NOTFOUND )
			{
				if ( msButtonHandlerList[mnUIButtonIDPressed] )
				{			
					// If we press another button, we should cancel any text box edits in progress first..
					mpUITextBoxImpl->EndCurrentEdit();

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


BOOL		UIOnRelease( int X, int Y )
{
	return( msTempSingleton.OnRelease( X, Y ) );
}

InterfaceInstance*		UIInstance::GetInterfaceInstance()
{
	return( mpInterfaceInstance );
}

InterfaceInstance*		UIInterfaceInstance()
{
	return( msTempSingleton.GetInterfaceInstance() );
}

void		UIInstance::Initialise( InterfaceInstance* pInterfaceInstance )
{
	if ( pInterfaceInstance == NULL )
	{
		pInterfaceInstance = InterfaceInstanceMain();
	}
	mpInterfaceInstance = pInterfaceInstance;

	mpUIButtonImpl = new UIButtonImpl( this );
	mpUITextBoxImpl = new UITextBoxImpl( this );
}

void		UISetActiveInterface( InterfaceInstance* pInterfaceInstance )
{
	mpInterfaceInstance = pInterfaceInstance;
}

void	UIInitialise(InterfaceInstance* pInterfaceInstance)
{
	msTempSingleton.Initialise( pInterfaceInstance );
}

void		UIInstance::Shutdown( void )
{
	// TODO - Cleanup msButtonHandlerList
	mpUIButtonImpl->Shutdown();
	mpUITextBoxImpl->Shutdown();
}

void	UIShutdown()
{
	msTempSingleton.Shutdown();
}

void		UIInstance::GetCurrentCursorPosition( int* pnX, int* pnY )
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

void		UIGetCurrentCursorPosition( int* pnX, int* pnY )
{
	msTempSingleton.GetCurrentCursorPosition( pnX, pnY );
}

void		UIInstance::SetCurrentCursorPosition( int nX, int nY )
{
	mnUISetCursorX = nX;
	mnUISetCursorY = nY;
}

void		UISetCurrentCursorPosition( int nX, int nY )
{
	msTempSingleton.SetCurrentCursorPosition( nX, nY );
}

uint32	UIInstance::GetCurrentHoverIDIndexParam()
{
	return( mulUIButtonIDHoveredID );
}

uint32	UIGetCurrentHoverIDIndexParam()
{
	return( msTempSingleton.GetCurrentHoverIDIndexParam() );
}

uint32	UIInstance::GetCurrentPressIDIndexParam()
{
	return(mulUIButtonIDPressedIDParam);
}

uint32	UIGetCurrentPressIDIndexParam()
{
	return( msTempSingleton.GetCurrentPressIDIndexParam() );
}


void		UIInstance::HoverIDSet( int nButtonID, uint32 ulParam, uint32 ulID )
{
	mnUIButtonIDHovered = nButtonID;
	mulUIButtonIDHoveredParam = ulParam;
	mulUIButtonIDHoveredID = ulID;
}

void		UIHoverIDSet( int nButtonID, uint32 ulParam, uint32 ulID )
{
	msTempSingleton.HoverIDSet( nButtonID, ulParam, ulID );
}

void		UIInstance::RightPressIDSet( int nButtonID, uint32 ulParam, uint32 ulIDParam )
{
	mnUIRightButtonIDPressed = nButtonID;
	mulUIRightButtonIDPressedParam = ulParam;
	mulUIRightButtonIDPressedIDParam = ulIDParam;
}

void		UIRightPressIDSet( int nButtonID, uint32 ulParam, uint32 ulIDParam )
{
	msTempSingleton.RightPressIDSet( nButtonID, ulParam, ulIDParam );
}

void		UIInstance::PressIDSet( int nButtonID, uint32 ulParam, uint32 ulIDParam )
{
	mnUIButtonIDPressed = nButtonID;
	mulUIButtonIDPressedParam = ulParam;
	mulUIButtonIDPressedIDParam = ulIDParam;
}

void		UIPressIDSet( int nButtonID, uint32 ulParam, uint32 ulIDParam )
{
	msTempSingleton.PressIDSet( nButtonID, ulParam, ulIDParam );
}

BOOL		UIInstance::HoverItem( int X, int Y, int W, int H )
{
int		hoverX, hoverY;

	GetCurrentCursorPosition( &hoverX, &hoverY );
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

BOOL		UIHoverItem( int X, int Y, int W, int H )
{
	return( msTempSingleton.HoverItem( X, Y, W, H ) );
}

BOOL		UIInstance::IsRightPressed( int X, int Y, int W, int H )
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

BOOL		UIIsRightPressed( int X, int Y, int W, int H )
{
	return( msTempSingleton.IsRightPressed( X, Y, W, H ) );
}
	
BOOL		UIInstance::IsPressed( int X, int Y, int W, int H )
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

BOOL		UIIsPressed( int X, int Y, int W, int H )
{
	return( msTempSingleton.IsPressed( X, Y, W, H ) );
}

void		UIReleaseGraphicsForDeviceReset( void )
{

}

void		UIInitGraphicsPostDeviceReset( void )
{

}


//-------------------------------

int		UIInstance::TextBoxCreate( int nMode, const char* szInitialText, int nMaxTextLen )
{
	return( mpUITextBoxImpl->Create( nMode, szInitialText, nMaxTextLen ) );
}


void	UIInstance::TextBoxRender( int nHandle, int nScreenX, int nScreenY, int nScreenW, int nScreenH )
{
	mpUITextBoxImpl->Render( this, nHandle, nScreenX, nScreenY, nScreenW, nScreenH );
}


const char*		UIInstance::TextBoxGetText( int nHandle )
{
	return( mpUITextBoxImpl->GetText( nHandle ) );
}

void		UIInstance::TextBoxEndEdit( int nHandle )
{
	mpUITextBoxImpl->EndEdit( nHandle );
}

void		UIInstance::TextBoxDestroy( int nHandle )
{
	mpUITextBoxImpl->Destroy( nHandle );
}
