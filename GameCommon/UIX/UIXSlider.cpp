
#include "StandardDef.h"
#include "InterfaceEx.h"

#include "../UI/UI.h"
#include "UIXSlider.h"


void	UIXSlider::Initialise( UIX_SLIDER_MODE mode, uint32 ulUserParam, float fMin, float fMax, float fInitialVal, float fMinStep )
{
	mfMinVal = fMin;
	mfMaxVal = fMax;
	mfInitialVal = fInitialVal;
	mfCurrentVal = fInitialVal;
	mfMinStep = fMinStep;
	mMode = mode;
	mulUserParam = ulUserParam;
}

void	UIXSlider::OnHeldUpdate( BOOL bIsHeld, BOOL bFirstPress )
{
	if ( bIsHeld )
	{
	int		cursX = 0;
	int		cursY = 0;
	float	fRange = (float)mRenderRect.w;

		UIGetCurrentCursorPosition( &cursX, &cursY );
	
		if ( fRange <= 0.0f ) fRange = 1.0f;
		float	fVal = ((cursX - mRenderRect.x) * mfMaxVal) / fRange;
		fVal += mfMinVal;
		if ( fVal < mfMinVal ) fVal = mfMinVal;
		if ( fVal > mfMaxVal ) fVal = mfMaxVal;
		mfCurrentVal = fVal;
		mbIsHeld = TRUE;
	}
	else
	{
		mbIsHeld = FALSE;
	}

}

UIXRECT		UIXSlider::OnRender( InterfaceInstance* pInterface, UIXRECT displayRect )
{
UIXRECT		localRect = GetDisplayRect();

	switch( mMode )
	{
	case ANGLE:
		{
		int		nBarMaxW = localRect.w;
		int		nBarPos;
		float	angleMod = (mfCurrentVal * 360.0f) / A360;
		
			if ( angleMod > 180.0f ) angleMod -= 360.0f;

			nBarPos = (int)( (nBarMaxW / 2) + ( (angleMod * (nBarMaxW / 2)) / 180.0f ) );

//			pInterface->Text( 1, X + 110, lineY + 4, 0xd0e0e0e0, 3, "%.1f", angleMod );
			pInterface->Rect( 0, localRect.x + displayRect.x, localRect.y + displayRect.y, nBarMaxW, localRect.h, 0xf0010101 );
			pInterface->Rect( 0, localRect.x + displayRect.x + nBarPos - 2, localRect.y + displayRect.y, 4, localRect.h, 0xf0303040 );
		}
		break;
//	case SCALER10:
	case VALUE:
	default:
		{
		int		nBarMaxW = localRect.w;
		int		nBarW = (int)(((mfCurrentVal-mfMinVal) * nBarMaxW) / mfMaxVal);
	
			if ( nBarW > nBarMaxW ) nBarW = nBarMaxW;
			if ( nBarW < 0 ) nBarW = 0;

			mRenderRect = localRect;
			mRenderRect.x += displayRect.x;
			mRenderRect.y += displayRect.y;

			// TODO - This should be editable text box
//			pInterface->Text( 1, X + 110, lineY + 4, 0xd0e0e0e0, 3, "%d%%", (int)(pProperty->Value(0)*100.0f) );
			// Background
			pInterface->Rect( 0, mRenderRect.x, mRenderRect.y, nBarMaxW, localRect.h, 0xf0080808 );
				
			if ( UIHoverItem( mRenderRect.x, mRenderRect.y, mRenderRect.w, mRenderRect.h ) == TRUE )
			{
				UIHoverIDSet( UIX_SLIDER_BAR, GetID() );
			}
			// Notches
			pInterface->Rect( 2, mRenderRect.x, mRenderRect.y + mRenderRect.h - 5, 1, 5, 0xa0909090 );
			pInterface->Rect( 2, mRenderRect.x + (nBarMaxW/2), mRenderRect.y + mRenderRect.h - 3, 1, 3, 0xa0909090 );
			pInterface->Rect( 2, mRenderRect.x + nBarMaxW, mRenderRect.y + mRenderRect.h - 5, 1, 5, 0xa0909090 );
			// Bar
			pInterface->Rect( 1, mRenderRect.x, mRenderRect.y, nBarW, mRenderRect.h, 0xf0202020 );
			pInterface->Rect( 1, mRenderRect.x + nBarW - 2, mRenderRect.y, 4, mRenderRect.h, 0xf0303040 );
		}
		break;
	}
	
	displayRect.h = 0;
	displayRect.y = localRect.y + localRect.h + 1;		// displayRect.y returns the lowest point we drew to

	return( displayRect );
}


void		UIXSlider::OnUpdate( float fDelta )
{
	if ( mValueUpdateFunc )
	{
	float	fNewVal = mValueUpdateFunc( GetID(), mfCurrentVal, mulUserParam, mbIsHeld );

		if ( fNewVal != mfCurrentVal )
		{
			mfCurrentVal = fNewVal;
		}
	}

}
