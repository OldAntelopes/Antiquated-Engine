	

#include "StandardDef.h"
#include "InterfaceEx.h"
#include "UIXShape.h"


void	UIXShape::Initialise( int mode, BOOL bBlocks, uint32 ulCol1, uint32 ulCol2 )
{
	mulCol1 = ulCol1;
	mulCol2 = ulCol2;
	mMode = mode;
	mBlocks = bBlocks;
}
	
UIXRECT		UIXShape::OnRender( InterfaceInstance* pInterface, UIXRECT displayRect )
{
UIXRECT		localRect = GetDisplayRect();

	switch( mMode )
	{
	case 0:
	default:
		pInterface->Rect( 0, displayRect.x + localRect.x, displayRect.y + localRect.y, localRect.w, localRect.h, mulCol1 );
		break;
	}

	if ( mBlocks == FALSE )
	{
		displayRect.h = 0;//pInterface->GetStringHeight( mText.c_str(), mFont );
	}
	else
	{
		displayRect.h = localRect.h;
	}
	displayRect.y = localRect.y + localRect.h;		// displayRect.y returns the lowest point we drew to

	return displayRect;
}


