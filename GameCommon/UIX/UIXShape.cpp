	

#include "StandardDef.h"
#include "InterfaceEx.h"
#include "../UI/UI.h"
#include "UIXShape.h"


void	UIXShape::Initialise( eUIXSHAPE_MODE mode, BOOL bBlocks, uint32 ulCol1, uint32 ulCol2, uint32 ulButtonID, uint32 ulButtonParam )
{
	mulCol1 = ulCol1;
	mulCol2 = ulCol2;
	mMode = mode;
	mBlocks = bBlocks;
	mulButtonID = ulButtonID;
	mulButtonParam = ulButtonParam;
}

void		UIXShape::OnUpdate( float delta )
{
	if ( mfAnimationTime > 0.0f )
	{
		mfAnimationPhase += delta;

		if ( mfAnimationPhase > mfAnimationTime )
		{
			// Anim props should determine what to do. For now, just end
			mfAnimationPhase = 0.0f;
			mfAnimationTime = 0.0f;
		}
	}

}
	
UIXRECT		UIXShape::OnRender( InterfaceInstance* pInterface, UIXRECT displayRect )
{
UIXRECT		drawRect = GetActualRenderRect( displayRect );

	if ( CheckDragHoverRegion( drawRect ) )
	{
		pInterface->OutlineBox( 0, drawRect.x, drawRect.y, drawRect.w, drawRect.h, 0x804060d0 );
		UIX::HoverAcceptDragItem(this);	
	}

	switch( mMode )
	{
	case UIXSHAPE_SHADEDRECT:
	default:
		pInterface->ShadedRect( 0, drawRect.x, drawRect.y, drawRect.w, drawRect.h, mulCol1, mulCol1, mulCol2, mulCol2 );

		if ( mulButtonID != 0 )
		{
			if ( UIX::CheckForPress( this, drawRect, mulButtonID, mulButtonParam ) )
			{		
				pInterface->Rect( 1, drawRect.x, drawRect.y, drawRect.w, drawRect.h, 0x20FFFFFF );				
			}
		}
		break;
	case UIXSHAPE_OUTLINEBOX:
		pInterface->OutlineBox( 0, drawRect.x, drawRect.y, drawRect.w, drawRect.h, mulCol1 );
		break;
	case UIXSHAPE_TRANSOUTLINEBOX:
		pInterface->OutlineBox( 0, drawRect.x, drawRect.y, drawRect.w, drawRect.h, 0x40101010 );

		if ( mfAnimationTime > 0.0f )
		{
		float	fAnimPhase = mfAnimationPhase / mfAnimationTime;
		int		drawHeight = (int)( drawRect.h * (1.0f - fAnimPhase) );
		uint32		ulCol = mulCol1;

			switch( mnAnimationMode )
			{
			case 0:
			default:
				drawHeight = (int)( drawRect.h * (1.0f - fAnimPhase) );
				ulCol = GetColWithModifiedAlpha( ulCol, 1.0f - fAnimPhase );
				pInterface->Rect( 1, drawRect.x, drawRect.y + (drawRect.h - drawHeight), 2, drawHeight, ulCol );
				pInterface->Rect( 1, drawRect.x + drawRect.w - 1, drawRect.y + (drawRect.h - drawHeight), 2, drawHeight , ulCol);
				// Always draw the bottom line with full alpha
				pInterface->Rect( 1, drawRect.x, drawRect.y + drawRect.h - 1, drawRect.w, 2, mulCol1 );
				break;
			case 1:
				ulCol = GetColWithModifiedAlpha( ulCol, fAnimPhase );
				drawHeight = (int)( drawRect.h * fAnimPhase );
				pInterface->Rect( 1, drawRect.x, drawRect.y + (drawRect.h - drawHeight), 2, drawHeight, ulCol );
				pInterface->Rect( 1, drawRect.x + drawRect.w - 1, drawRect.y + (drawRect.h - drawHeight), 2, drawHeight , ulCol);
				// Always draw the bottom line with full alpha
				pInterface->Rect( 1, drawRect.x, drawRect.y + drawRect.h - 1, drawRect.w, 2, mulCol1 );
				break;
			}
		}	
		else
		{
			switch( mnAnimationMode )
			{
			case 0:		// Transition out : Only display when the anim is active
			default:
				break;		
			case 1:		// Transition in : Display as normal outline box once anim completes
				pInterface->OutlineBox( 0, drawRect.x, drawRect.y, drawRect.w, drawRect.h, mulCol1 );
				break;
			}
		}
		break;
	}

	if ( mBlocks == FALSE )
	{
		displayRect.h = 0;//pInterface->GetStringHeight( mText.c_str(), mFont );
	}
	else
	{
		displayRect.h = GetLocalPositionRect().h + 1;
	}
	displayRect.y = GetLocalPositionRect().y + GetLocalPositionRect().h + 1;		// displayRect.y returns the lowest point we drew to

	return displayRect;
}


