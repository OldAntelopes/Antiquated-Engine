
#include "StandardDef.h"
#include "Interface.h"

#include "UIScreenClass.h"

void		UIScreen::InitScreen( void )
{
	OnInitScreen();
}

void		UIScreen::ActivateScreen( void )
{
	mbIsActive = true;
	OnActivateScreen();
}


void	UIScreen::RenderScreen( int X, int Y, int W, int H, float fAlpha )
{
int		nScreenX = X;
int		nScreenY = Y;
int		nScreenW = W;
int		nScreenH = H;

	mnScreenButtonHover = NOTFOUND;

	OnRenderScreen( nScreenX, nScreenY, nScreenW, nScreenH, fAlpha );


}


void		UIScreen::UpdateScreen( float fDelta )
{
UIControl*	pControls = mpControlsList;

	while( pControls )
	{
		if ( pControls->IsActive() )
		{
			pControls->Update( fDelta );
		}
		pControls = pControls->GetNext();
	}

	OnUpdateScreen( fDelta );
}


bool		UIScreen::LeftMouseUp()
{
UIControl*	pControls = mpControlsList;

	mbDrawWindowHold = FALSE;

	while( pControls )
	{
		if ( ( pControls->IsActive() ) &&
			 ( pControls->OnLeftMouseUp() == TRUE ) )
		{
			return( true );
		}
		pControls = pControls->GetNext();
	}

	return( OnLeftMouseUp() );
}

bool		UIScreen::LeftMouseDown()
{
UIControl*	pControls = mpControlsList;

	while( pControls )
	{
		if ( ( pControls->IsActive() ) &&
			 ( pControls->OnLeftMouseDown() == TRUE ) )
		{
			return( true );
		}
		pControls = pControls->GetNext();
	}
	return( OnLeftMouseDown() );
}


void		UIScreen::CloseScreen()
{
	mbIsActive = false;
	OnCloseScreen();
}

void		UIScreen::FreeScreen()
{

	OnFreeScreen();
}

void		UIScreen::AddControlToScreen( UIControl* pControl )
{
	pControl->SetNext( mpControlsList );
	mpControlsList = pControl;
}
