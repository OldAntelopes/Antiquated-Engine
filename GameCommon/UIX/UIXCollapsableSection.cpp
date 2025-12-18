
#include "StandardDef.h"
#include "InterfaceEx.h"
#include "../UI/UI.h"
#include "UIXCollapsableSection.h"


void	UIXCollapsableSection::Initialise( int mode, const char* szTitle, BOOL bStartCollapsed )
{
	mTitle = szTitle;	
	mbIsCollapsed = bStartCollapsed;
	mMode = mode;
}

void	UIXCollapsableSection::ToggleCollapsed()
{
	mbIsCollapsed = !mbIsCollapsed;
}


UIXRECT		UIXCollapsableSection::OnRender( InterfaceInstance* pInterface, UIXRECT displayRect )
{
int	X = displayRect.x + GetDisplayRect().x;
int	Y = displayRect.y + GetDisplayRect().y;
int	W = displayRect.w;
int	H = displayRect.h;
uint32		ulCol = 0x90303030;
int		headerH = 20;

	switch( mMode )
	{
	case 1:
		ulCol = 0x90303040;
		break;
	default:
		break;
	}

	pInterface->Rect( 0, X, Y, W, headerH, ulCol );
				
	UIButtonRegion( UIX_COLLAPSABLE_SECTION_HEADER, X, Y, W, H, GetID() );

	pInterface->Text( 1, X + 20, Y + 4, 0xd0d0d0d0, 3, mTitle.c_str() );

	if ( !mbIsCollapsed )
	{
		pInterface->Triangle( 1, X + 4, Y + 8, X + 12, Y + 8, X + 8, Y + 16, 0xa0a0a0a0, 0xa0a0a0a0, 0xa0a0a0a0 );	
	}
	else
	{
		pInterface->Triangle( 1, X + 4, Y + 6, X + 10, Y + 10, X + 4, Y + 14, 0xa0a0a0a0, 0xa0a0a0a0, 0xa0a0a0a0 );	
	}

	displayRect.h = headerH + 1;
	return displayRect;
}
