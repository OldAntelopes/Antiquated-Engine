
#include "StandardDef.h"
#include "InterfaceEx.h"

#include "../UI/UI.h"
#include "UIXTextBox.h"
#include "UIXSlider.h"

void	UIXSlider::ModifySliderRange( float fValue, float fMinRange, float fMaxRange )		// Triggered by (e.g) midi control changing slider values
{
	mfMinVal = fMinRange;
	mfMaxVal = fMaxRange;
	mfCurrentVal = fValue;

	// Expand the handled range if our new value is higher
	if ( mfMinVal < mfInitialMinVal )
	{
		if ( mbAllowRangeExpand )
		{
			mfInitialMinVal = mfMinVal;
		}
		else
		{
			mfMinVal = mfInitialMinVal;
			mfMaxVal = max( mfMinVal, mfMaxVal );
		}
	}
	
	// Expand the handled range if our new value is higher
	if ( mfMaxVal > mfInitialMaxVal )
	{
		if ( mbAllowRangeExpand )
		{
			mfInitialMaxVal = mfMaxVal;
		}
		else
		{
			mfMaxVal = mfInitialMaxVal;
			mfMinVal = min( mfMinVal, mfMaxVal );
		}				
	}
}

float	UIXSlider::OnValueChange( UIXObject* pxSourceObj, float fNewValue, BOOL bFromTextBoxEntry )		// Triggered by (e.g) child text boxes when a new value is entered there directly
{
	if ( pxSourceObj == mpRangeMinTextBox)
	{
		mfMinVal = fNewValue;

		if (mfMinVal > mfMaxVal)
		{
			// When the user has entered a new min value in a text box, we update the max value to fit it
			// if the new min is higher than the max
			if ( bFromTextBoxEntry )
			{
				mfMaxVal = mfMinVal;
				// Expand the handled range if our new value is higher
				if ( mfMaxVal > mfInitialMaxVal )
				{
					if ( mbAllowRangeExpand )
					{
						mfInitialMaxVal = mfMaxVal;
					}
					else
					{
						mfMaxVal = mfInitialMaxVal;
						mfMinVal = min( mfMinVal, mfMaxVal );
					}				
				}
			}
			else 
			{
				mfMinVal = mfMaxVal;
			}
		}
		// Expand the handled range if our new value is higher
		if ( mfMinVal < mfInitialMinVal )
		{
			if ( mbAllowRangeExpand )
			{
				mfInitialMinVal = mfMinVal;
			}
			else
			{
				mfMinVal = mfInitialMinVal;
				mfMaxVal = max( mfMinVal, mfMaxVal );
			}
		}
		return mfMinVal;
	}
	else if ( pxSourceObj == mpRangeMaxTextBox )
	{
		mfMaxVal = fNewValue;
		if (mfMaxVal < mfMinVal)
		{
			// When the user has entered a new max value in a text box, we update the min value to fit it
			// if the new max is lower than the min
			if ( bFromTextBoxEntry )
			{
				mfMinVal = mfMaxVal;
				// Expand the handled range if our new value is higher
				if ( mfMinVal < mfInitialMinVal )
				{
					if ( mbAllowRangeExpand )
					{
						mfInitialMinVal = mfMinVal;
					}
					else
					{
						mfMinVal = mfInitialMinVal;
						mfMaxVal = max( mfMinVal, mfMaxVal );
					}
				}
			}
			else
			{
				mfMaxVal = mfMinVal;
			}
		}
		// Expand the handled range if our new value is higher
		if ( mfMaxVal > mfInitialMaxVal )
		{
			if ( mbAllowRangeExpand )
			{
				mfInitialMaxVal = mfMaxVal;
			}
			else
			{
				mfMaxVal = mfInitialMaxVal;
				mfMinVal = min( mfMinVal, mfMaxVal );
			}
		}
		return mfMaxVal;	
	}
	else if ( pxSourceObj == mpValueTextBox )
	{
		mfCurrentVal = fNewValue;
	}
	return( fNewValue );
}

void	UIXSlider::Initialise( UIX_SLIDER_MODE mode, uint32 ulUserParam, float fMin, float fMax, float fInitialVal, float fMinStep, const char* szText, BOOL bShowTextBoxes )
{
	mfMinVal = fMin;
	mfMaxVal = fMax;
	mfInitialMinVal = fMin;
	mfInitialMaxVal = fMax;
	mfInitialVal = fInitialVal;
	mfCurrentVal = fInitialVal;
	mfMinStep = fMinStep;
	mMode = mode;
	mulUserParam = ulUserParam;
	mbShowValueTextBoxes = bShowTextBoxes;
	if ( szText )
	{
		mText = szText;
	}

	char	acVal[128];

	switch( mMode )
	{
	case SLIDERMODE_VALUERANGE:
		if ( mbShowValueTextBoxes )
		{
		int		nTextSectionW = 120;
		int		nTextBoxW = (nTextSectionW / 2) - 8;
		UIXRECT		textboxRect = UIXRECT(0, 0, nTextBoxW, GetLocalPositionRect().h - 2);
			
			sprintf( acVal, "%.3f", mfMinVal );
			mpRangeMinTextBox = UIX::AddTextBox( this, textboxRect, 0, acVal );

			textboxRect.x += nTextBoxW + 14;
			sprintf( acVal, "%.3f", mfMaxVal );
			mpRangeMaxTextBox = UIX::AddTextBox( this, textboxRect, 0, acVal );
		} 
		break;
	case SLIDERMODE_INTVALUE_WITH_CONSTRAINTS:
		if ( mbShowValueTextBoxes )
		{
		int		nTextSectionW = 180;
		int		nTextBoxW = (nTextSectionW / 3) - 18;
		UIXRECT		textboxRect = UIXRECT(0, 0, nTextBoxW, GetLocalPositionRect().h - 2);
			
			sprintf(acVal, "%d", (int)mfCurrentVal);
			mpValueTextBox = UIX::AddTextBox(this, textboxRect, 0, acVal);
			textboxRect.x += nTextBoxW + 18;

			sprintf( acVal, "%d", (int)mfMinVal );
			mpRangeMinTextBox = UIX::AddTextBox( this, textboxRect, 0, acVal );

			textboxRect.x += nTextBoxW + 14;
			sprintf( acVal, "%d", (int)mfMaxVal );
			mpRangeMaxTextBox = UIX::AddTextBox( this, textboxRect, 0, acVal );
		}
		break;

	case SLIDERMODE_VALUE_WITH_CONSTRAINTS:
	case SLIDERMODE_VALUE:
		if ( mbShowValueTextBoxes )
		{
		int		nTextSectionW = 180;
		int		nTextBoxW = (nTextSectionW / 3) - 18;
		UIXRECT		textboxRect = UIXRECT(0, 0, nTextBoxW, GetLocalPositionRect().h - 2);
			
			sprintf(acVal, "%.2f", mfCurrentVal);
			mpValueTextBox = UIX::AddTextBox(this, textboxRect, 0, acVal);
			textboxRect.x += nTextBoxW + 18;

			// TODO - Should really have something that deals with integers properly here
			// (e.g. Pixellate Pixel size property doesnt need any .00 )
			if ( mfMaxVal >= 10.0f )
			{
				sprintf( acVal, "%.1f", mfMinVal );
				mpRangeMinTextBox = UIX::AddTextBox( this, textboxRect, 0, acVal );

				textboxRect.x += nTextBoxW + 14;
				sprintf( acVal, "%.1f", mfMaxVal );
				mpRangeMaxTextBox = UIX::AddTextBox( this, textboxRect, 0, acVal );
			}
			else
			{
				sprintf( acVal, "%.3f", mfMinVal );
				mpRangeMinTextBox = UIX::AddTextBox( this, textboxRect, 0, acVal );

				textboxRect.x += nTextBoxW + 14;
				sprintf( acVal, "%.3f", mfMaxVal );
				mpRangeMaxTextBox = UIX::AddTextBox( this, textboxRect, 0, acVal );
			}
		}
		break;
	case SLIDERMODE_PLUSMINUS_VALUE:
		if ( mbShowValueTextBoxes )
		{
			UIXRECT		textboxRect = UIXRECT(0, 0, 60, GetLocalPositionRect().h - 2);

			sprintf(acVal, "%.2f", mfCurrentVal);
			mpValueTextBox = UIX::AddTextBox(this, textboxRect, 0, acVal);
		}
		break;
	}
}


float	UIXSlider::GetCursorValue()
{
int		cursX = 0;
int		cursY = 0;
float	fRange = (float)mRenderRect.w;
float		fMinVal = mfInitialMinVal;
float		fMaxVal = mfInitialMaxVal;
float		fVal;

	UIGetCurrentCursorPosition( &cursX, &cursY );
	
	if ( mMode == SLIDERMODE_VERTICAL_VALUE )
	{
		fRange = (float)( mRenderRect.h );
		if ( fRange <= 0.0f ) fRange = 1.0f;
		fVal = (((mRenderRect.y + mRenderRect.h) - cursY) * (fMaxVal-fMinVal)) / fRange;
	}
	else
	{
		if ( fRange <= 0.0f ) fRange = 1.0f;
		fVal = ((cursX - mRenderRect.x) * (fMaxVal-fMinVal)) / fRange;
	}
	fVal += fMinVal;
	if ( fVal < fMinVal ) fVal = fMinVal;
	if ( fVal > fMaxVal ) fVal = fMaxVal;
	return( fVal );
}

void	UIXSlider::OnHeldUpdate( BOOL bIsHeld, BOOL bFirstPress )
{
	if ( bIsHeld )
	{
		mfCurrentVal = GetCursorValue();
	}
	mbIsHeld = bIsHeld;
}

void	UIXSlider::OnMinRangeHeldUpdate( BOOL bIsHeld, BOOL bFirstPress )
{
	if ( bIsHeld )
	{
		mfMinVal = GetCursorValue();
		if ( mfMinVal > mfMaxVal )
		{
			mfMinVal = mfMaxVal;
		}
	}
	mbIsHeld = bIsHeld;
}

void	UIXSlider::OnMaxRangeHeldUpdate( BOOL bIsHeld, BOOL bFirstPress )
{
	if ( bIsHeld )
	{
		mfMaxVal = GetCursorValue();
		if ( mfMaxVal < mfMinVal )
		{
			mfMaxVal = mfMinVal;
		}
	}
	mbIsHeld = bIsHeld;
}

UIXRECT		UIXSlider::OnRender( InterfaceInstance* pInterface, UIXRECT displayRect )
{
UIXRECT		renderRect = GetActualRenderRect( displayRect );
uint32		ulCol = 0xf0505070;

	mRenderRect = renderRect;

	switch( mMode )
	{
	case SLIDERMODE_VALUE_WITH_CONSTRAINTS:
	case SLIDERMODE_INTVALUE_WITH_CONSTRAINTS:
		{
		int		nTextAreaW = 180;
		int		nBarMaxW = renderRect.w - nTextAreaW;
		int		nBarW = 0;
	
			if ( mfMaxVal > mfMinVal )
			{
				nBarW = (int)(((mfCurrentVal-mfMinVal) * nBarMaxW) / (mfMaxVal-mfMinVal));
			}
			else
			{
				nBarW = 1;
			}

			// Show the text values to the left
			UIXRECT		drawRect = renderRect;
			int			nTextBoxW = ((nTextAreaW*2) / 3) - 17;
			char		acVal[128];
			uint32		ulTextCol = 0xC0C0C0C0;

			drawRect.w = nTextAreaW;
			drawRect.y += 1;
			drawRect.h -= 2;

			if ( mpRangeMinTextBox )
			{
				if ( mMode == SLIDERMODE_INTVALUE_WITH_CONSTRAINTS )
				{
					sprintf( acVal, "%d", (int)mfMinVal );
				}
				else
				{
					sprintf( acVal, "%.2f", mfMinVal );
				}
				mpRangeMinTextBox->SetText( acVal );
				mpRangeMinTextBox->OnRender( pInterface, drawRect );
				pInterface->Text( 1, drawRect.x + nTextBoxW + 4, drawRect.y + 4, ulTextCol, 3, "-" );
			}

			if ( mpRangeMaxTextBox )
			{
				if ( mMode == SLIDERMODE_INTVALUE_WITH_CONSTRAINTS )
				{
					sprintf( acVal, "%d", (int)mfMaxVal );
				}
				else
				{
					sprintf( acVal, "%.2f", mfMaxVal );
				}
				mpRangeMaxTextBox->SetText( acVal );
				mpRangeMaxTextBox->OnRender( pInterface, drawRect );
			}

			if ( mpValueTextBox )
			{
				if ( mMode == SLIDERMODE_INTVALUE_WITH_CONSTRAINTS )
				{
					sprintf( acVal, "%d", (int)mfCurrentVal );
				}
				else
				{
					sprintf(acVal, "%.2f", mfCurrentVal );
				}
				mpValueTextBox->SetText(acVal);
				mpValueTextBox->OnRender(pInterface, drawRect);
			}

			float		fFullRange = mfInitialMaxVal - mfInitialMinVal;
			int		nMinBarPos = 0;
			int		nMaxBarPos = 0;
			int		nCurrentBarPos = 0;

			if ( fFullRange > 0.0f )
			{
				nMinBarPos = (int)( ((mfMinVal-mfInitialMinVal) * nBarMaxW) / fFullRange );
				nMaxBarPos = (int)( ((mfMaxVal-mfInitialMinVal) * nBarMaxW) / fFullRange );
				nCurrentBarPos = (int)( ((mfCurrentVal-mfInitialMinVal) * nBarMaxW) / fFullRange );
			}

			mRenderRect.x += nTextAreaW;
			mRenderRect.w -= nTextAreaW;
			// Background
			pInterface->Rect( 0, mRenderRect.x, mRenderRect.y, nBarMaxW, mRenderRect.h, 0xf0080808 );
			// INDICATOR (MIN)
			ulCol = 0xf0505070;
			if ( UIX::IsMouseHover( mRenderRect.x + nMinBarPos - 7, mRenderRect.y,  8, mRenderRect.h ) == TRUE )
			{
				UIHoverIDSet( UIX_SLIDER_BAR_MINRANGE, 0, GetID() );
				ulCol = 0xf0a08050;
			}
			pInterface->Triangle( 1, mRenderRect.x + nMinBarPos - 6, mRenderRect.y + 6, mRenderRect.x + nMinBarPos, mRenderRect.y + 6, mRenderRect.x + nMinBarPos, mRenderRect.y, ulCol, ulCol, ulCol );
			pInterface->Rect( 0, mRenderRect.x + nMinBarPos - 6, mRenderRect.y + 6, 6, mRenderRect.h - 6, ulCol );
			// INDICATOR (MAX)
			ulCol = 0xf0505070;
			if ( UIX::IsMouseHover( mRenderRect.x + nMaxBarPos - 1, mRenderRect.y, 8, mRenderRect.h ) == TRUE )
			{
				UIHoverIDSet( UIX_SLIDER_BAR_MAXRANGE, 0, GetID() );
				ulCol = 0xf0a08050;
			}
			pInterface->Rect( 0, mRenderRect.x + nMaxBarPos, mRenderRect.y + 6, 6, mRenderRect.h - 6, ulCol );
			pInterface->Triangle( 1, mRenderRect.x + nMaxBarPos, mRenderRect.y + 6, mRenderRect.x + nMaxBarPos + 6, mRenderRect.y + 6, mRenderRect.x + nMaxBarPos, mRenderRect.y, ulCol, ulCol, ulCol );

			// SLIDERMODE_VALUE_WITH_CONSTRAINTS  Current value indicator
			// 
			// Bar
			pInterface->Rect( 0, mRenderRect.x + nMinBarPos, mRenderRect.y, nCurrentBarPos - nMinBarPos, mRenderRect.h, 0xf0202020 );
			
			ulCol = 0xf0303040;
			if ( UIX::IsMouseHover( mRenderRect.x + nCurrentBarPos - 3, mRenderRect.y, 7, mRenderRect.h ) == TRUE )
			{
				UIHoverIDSet( UIX_SLIDER_BAR, 0, GetID() );
				ulCol = 0xf0a08050;
			}

			// Value/Grab bar
			pInterface->Rect( 0, mRenderRect.x + nCurrentBarPos - 2, mRenderRect.y, 4, mRenderRect.h, ulCol );

			// Notches
			pInterface->Rect( 1, mRenderRect.x, mRenderRect.y + mRenderRect.h - 3, 1, 3, 0xa0909090 );
			pInterface->Rect( 1, mRenderRect.x + (nBarMaxW/2), mRenderRect.y + mRenderRect.h - 5, 1, 5, 0xa0909090 );
			pInterface->Rect( 1, mRenderRect.x + nBarMaxW, mRenderRect.y + mRenderRect.h - 3, 1, 3, 0xa0909090 );
		}
		break;

	case SLIDERMODE_VALUERANGE:
		{
		int		nTextAreaW = 120;

			// TODO - Might want to make this optional or part of a different mode..?
			// Show the text values to the left
			UIXRECT		drawRect = renderRect;
			int			nTextBoxW = (nTextAreaW / 2) - 8;
			char		acVal[128];
			uint32		ulTextCol = 0xC0C0C0C0;

			drawRect.w = nTextAreaW;
			drawRect.y += 1;
			drawRect.h -= 2;

			if ( mpRangeMinTextBox )
			{
				if ( ( mfMinVal <= -100.0f ) || ( mfMinVal >= 100.0f ) )
				{
					sprintf( acVal, "%.1f", mfMinVal );			
				}
				else
				{
					sprintf( acVal, "%.3f", mfMinVal );
				}
				mpRangeMinTextBox->SetText( acVal );
				mpRangeMinTextBox->OnRender( pInterface, drawRect );

				pInterface->Text( 1, drawRect.x + nTextBoxW + 4, drawRect.y + 4, ulTextCol, 3, "-" );
			}

			if ( mpRangeMaxTextBox )
			{
				if ( ( mfMaxVal < -100.0f ) || ( mfMaxVal > 100.0f ) )
				{
					sprintf( acVal, "%.1f", mfMaxVal );
				}
				else
				{
					sprintf( acVal, "%.3f", mfMaxVal );
				}
				mpRangeMaxTextBox->SetText( acVal );
				mpRangeMaxTextBox->OnRender( pInterface, drawRect );
			}

			int		nBarMaxW = renderRect.w - nTextAreaW;
			int		nMinBarPos = (int)(((mfMinVal-mfInitialMinVal) * nBarMaxW) / (mfInitialMaxVal-mfInitialMinVal));
			int		nMaxBarPos = (int)(((mfMaxVal-mfInitialMinVal) * nBarMaxW) / (mfInitialMaxVal-mfInitialMinVal));
			int		nCurrentBarPos = (int)(((mfCurrentVal-mfInitialMinVal) * nBarMaxW) / (mfInitialMaxVal-mfInitialMinVal));

			if ( mfMinVal < mfInitialMinVal ) nMinBarPos = 0;
			if ( mfMaxVal > mfInitialMaxVal ) nMaxBarPos = nBarMaxW;
			if ( nMaxBarPos < 0 ) nMaxBarPos = 0;
			if ( nMinBarPos > nBarMaxW ) nMinBarPos = nBarMaxW;

			mRenderRect.x += nTextAreaW;
			mRenderRect.w -= nTextAreaW;
			// Background
			pInterface->Rect( 0, mRenderRect.x, mRenderRect.y, nBarMaxW, mRenderRect.h, 0xf0080808 );
			// INDICATOR (MIN)
			ulCol = 0xf0505070;
			if ( UIX::IsMouseHover( mRenderRect.x + nMinBarPos - 7, mRenderRect.y, 8, mRenderRect.h ) == TRUE )
			{
				UIHoverIDSet( UIX_SLIDER_BAR_MINRANGE, 0, GetID() );
				ulCol = 0xf0a08050;
			}
			pInterface->Triangle( 1, mRenderRect.x + nMinBarPos - 6, mRenderRect.y + 6, mRenderRect.x + nMinBarPos, mRenderRect.y + 6, mRenderRect.x + nMinBarPos, mRenderRect.y, ulCol, ulCol, ulCol );
			pInterface->Rect( 1, mRenderRect.x + nMinBarPos - 6, mRenderRect.y + 6, 6, mRenderRect.h - 6, ulCol );
			// INDICATOR (MAX)
			ulCol = 0xf0505070;
			if ( UIX::IsMouseHover( mRenderRect.x + nMaxBarPos - 1, mRenderRect.y, 8, mRenderRect.h ) == TRUE )
			{
				UIHoverIDSet( UIX_SLIDER_BAR_MAXRANGE, 0, GetID() );
				ulCol = 0xf0a08050;
			}
			pInterface->Rect( 1, mRenderRect.x + nMaxBarPos, mRenderRect.y + 6, 6, mRenderRect.h - 6, ulCol );
			pInterface->Triangle( 1, mRenderRect.x + nMaxBarPos, mRenderRect.y + 6, mRenderRect.x + nMaxBarPos + 6, mRenderRect.y + 6, mRenderRect.x + nMaxBarPos, mRenderRect.y, ulCol, ulCol, ulCol );

			// Current value indicator		
			if ( mbShowCurrentValueIndicator )
			{
				ulCol = 0xc0b0b0b0;
				pInterface->Rect( 0, mRenderRect.x + nCurrentBarPos - 1, mRenderRect.y + 4, 2, mRenderRect.h - 6, ulCol );
			}

			// between bar
			pInterface->Rect( 0, mRenderRect.x + nMinBarPos + 2, mRenderRect.y + ( mRenderRect.h / 2), (nMaxBarPos - nMinBarPos) - 2, (mRenderRect.h/2), 0xA0404040 );
			// Notches
			pInterface->Rect( 1, mRenderRect.x, mRenderRect.y + mRenderRect.h - 3, 1, 3, 0xa0909090 );
			pInterface->Rect( 1, mRenderRect.x + (nBarMaxW/2), mRenderRect.y + mRenderRect.h - 5, 1, 5, 0xa0909090 );
			pInterface->Rect( 1, mRenderRect.x + nBarMaxW, mRenderRect.y + mRenderRect.h - 3, 1, 3, 0xa0909090 );
		}
		break;
	case SLIDERMODE_ANGLE:
		{
		int		nBarMaxW = mRenderRect.w;
		int		nBarPos;
		float	angleMod = (mfCurrentVal * 360.0f) / A360;
		
			if ( angleMod > 180.0f ) angleMod -= 360.0f;

			nBarPos = (int)( (nBarMaxW / 2) + ( (angleMod * (nBarMaxW / 2)) / 180.0f ) );

//			pInterface->Text( 1, X + 110, lineY + 4, 0xd0e0e0e0, 3, "%.1f", angleMod );
			// BACKGROUND
			pInterface->Rect( 0, mRenderRect.x, mRenderRect.y, nBarMaxW, mRenderRect.h, 0xf0010101 );
			if ( UIX::IsMouseHover( mRenderRect.x, mRenderRect.y, mRenderRect.w, mRenderRect.h ) == TRUE )
			{
				UIHoverIDSet( UIX_SLIDER_BAR, 0, GetID() );
			}
			// INDICATOR
			pInterface->Rect( 0, mRenderRect.x + nBarPos - 2, mRenderRect.y, 4, mRenderRect.h, 0xf0303040 );
			// Notches
			pInterface->Rect( 1, mRenderRect.x, mRenderRect.y + mRenderRect.h - 3, 1, 3, 0xa0909090 );
			pInterface->Rect( 1, mRenderRect.x + (nBarMaxW/2), mRenderRect.y + mRenderRect.h - 5, 1, 5, 0xa0909090 );
			pInterface->Rect( 1, mRenderRect.x + nBarMaxW, mRenderRect.y + mRenderRect.h - 3, 1, 3, 0xa0909090 );
		}
		break;
	case SLIDERMODE_VERTICAL_VALUE:
		{
		int		nKnobH = mRenderRect.w;
		int		nBarMaxH = mRenderRect.h - nKnobH;
		int		nBarH = (int)(((mfCurrentVal-mfMinVal) * nBarMaxH) / (mfMaxVal-mfMinVal));
		uint32	ulKnobCol = 0xf0303040;
			if ( nBarH > nBarMaxH ) nBarH = nBarMaxH;
			if ( nBarH < 0 ) nBarH = 0;

			// TODO - This should be editable text box
//			pInterface->Text( 1, X + 110, lineY + 4, 0xd0e0e0e0, 3, "%d%%", (int)(pProperty->Value(0)*100.0f) );
			// Background
			pInterface->Rect( 0, mRenderRect.x, mRenderRect.y, mRenderRect.w, mRenderRect.h, 0xf0080808 );
				
			if ( UIX::IsMouseHover( mRenderRect.x, mRenderRect.y, mRenderRect.w, mRenderRect.h ) == TRUE )
			{
				UIHoverIDSet( UIX_SLIDER_BAR, 0, GetID() );
				ulKnobCol = 0xf0505070;
			}
			// Notches
			pInterface->Rect( 1, mRenderRect.x, mRenderRect.y, 4, 1, 0xa0909090 );
			pInterface->Rect( 1, mRenderRect.x, mRenderRect.y + (mRenderRect.h/2), 3, 1, 0xa0909090 );
			pInterface->Rect( 1, mRenderRect.x, mRenderRect.y + mRenderRect.h, 4, 1, 0xa0909090 );
			// Bar
			pInterface->Rect( 0, mRenderRect.x, mRenderRect.y + nBarMaxH + nKnobH - nBarH, mRenderRect.w, nBarH, 0xf0202020 );

			int		nKnobTop =  mRenderRect.y + mRenderRect.h - (nBarH + nKnobH);
			pInterface->Rect( 0, mRenderRect.x, nKnobTop, mRenderRect.w, nKnobH, ulKnobCol );

			int			textY = nKnobTop + (((nKnobH - pInterface->GetStringHeight(mText.c_str(), 3))-1)/2);

			pInterface->TextCentre( 1, mRenderRect.x + (mRenderRect.w / 2), textY, 0xD0F0F0F0, 3, mText.c_str() );
		}
		break;
	case SLIDERMODE_PLUSMINUS_VALUE:
		{
			int		nBarMaxW = mRenderRect.w;
			int		nBarPos;
			float	angleMod = (mfCurrentVal * 360.0f) / A360;
			int		nBarHalfW = (nBarMaxW / 2);
			UIXRECT		drawRect = renderRect;
			char		acVal[128];
			if (angleMod > 180.0f) angleMod -= 360.0f;

			drawRect.w = 60;
			drawRect.y += 1;
			drawRect.h -= 2;

			if ( mpValueTextBox )
			{
				sprintf(acVal, "%.3f", mfCurrentVal );
				mpValueTextBox->SetText(acVal);
				mpValueTextBox->OnRender(pInterface, drawRect);
			}
			
			mRenderRect.x += 60;
			mRenderRect.w -= 60;

			// TODO - MinVal might not be -MaxVal 
			// (This currently assumes middle is at 0 and its always, like, -10 -> 10 or something
			nBarPos = (int)(nBarHalfW + ((mfCurrentVal * nBarHalfW) / mfMaxVal));

			//			pInterface->Text( 1, X + 110, lineY + 4, 0xd0e0e0e0, 3, "%.1f", angleMod );
						// BACKGROUND
			pInterface->Rect(0, mRenderRect.x, mRenderRect.y, nBarMaxW, mRenderRect.h, 0xf0010101);
			if (UIX::IsMouseHover(mRenderRect) == TRUE)
			{
				UIHoverIDSet(UIX_SLIDER_BAR, 0, GetID());
			}
			if (nBarPos < nBarHalfW)
			{
				pInterface->Rect(0, mRenderRect.x + nBarPos, mRenderRect.y, nBarHalfW - nBarPos, mRenderRect.h, 0xf0202020);
			}
			else
			{
				pInterface->Rect(0, mRenderRect.x + nBarHalfW, mRenderRect.y, nBarPos - nBarHalfW, mRenderRect.h, 0xf0202020);
			}
			// INDICATOR
			pInterface->Rect(0, mRenderRect.x + nBarPos - 2, mRenderRect.y, 4, mRenderRect.h, 0xf0303040);
			// Notches
			pInterface->Rect(1, mRenderRect.x, mRenderRect.y + mRenderRect.h - 3, 1, 3, 0xa0909090);
			pInterface->Rect(1, mRenderRect.x + (nBarMaxW / 2), mRenderRect.y + mRenderRect.h - 5, 1, 5, 0xa0909090);
			pInterface->Rect(1, mRenderRect.x + nBarMaxW, mRenderRect.y + mRenderRect.h - 3, 1, 3, 0xa0909090);
		}
		break;
	case SLIDERMODE_VALUE:
		{
		UIXRECT		drawRect = renderRect;
		char		acVal[128];
	
			drawRect.y += 1;
			drawRect.h -= 2;

			if ( mpValueTextBox )
			{
				drawRect.w = 60;
				sprintf(acVal, "%.3f", mfCurrentVal );
				mpValueTextBox->SetText(acVal);
				mpValueTextBox->OnRender(pInterface, drawRect);

				mRenderRect.x += drawRect.w + 5;
				mRenderRect.w -= drawRect.w + 5;
				drawRect.x += drawRect.w + 5;
				drawRect.w = mRenderRect.w;
			}

			int		nBarMaxW = drawRect.w;
			int		nBarW = (int)(((mfCurrentVal-mfMinVal) * nBarMaxW) / (mfMaxVal-mfMinVal));
			if ( nBarW > nBarMaxW ) nBarW = nBarMaxW;
			if ( nBarW < 0 ) nBarW = 0;

			// Background
			pInterface->Rect( 0, drawRect.x, drawRect.y, nBarMaxW, drawRect.h, 0xf0080808 );
				
			if ( UIX::IsMouseHover( drawRect ) == TRUE )
			{
				UIHoverIDSet( UIX_SLIDER_BAR, 0, GetID() );
			}
			// Notches
			pInterface->Rect( 1, drawRect.x, drawRect.y + drawRect.h - 5, 1, 5, 0xa0909090 );
			pInterface->Rect( 1, drawRect.x + (nBarMaxW/2), drawRect.y + drawRect.h - 3, 1, 3, 0xa0909090 );
			pInterface->Rect( 1, drawRect.x + nBarMaxW, drawRect.y + drawRect.h - 5, 1, 5, 0xa0909090 );
			// Bar
			pInterface->Rect( 0, drawRect.x, drawRect.y, nBarW, drawRect.h, 0xf0202020 );
			// Value/Grab bar
			pInterface->Rect( 0, drawRect.x + nBarW - 2, drawRect.y, 4, drawRect.h, 0xf0505080 );
		}
		break;
//	case SCALER10:
	default:
		{
		int		nBarMaxW = mRenderRect.w;
		int		nBarW = (int)(((mfCurrentVal-mfMinVal) * nBarMaxW) / (mfMaxVal-mfMinVal));
	
			if ( nBarW > nBarMaxW ) nBarW = nBarMaxW;
			if ( nBarW < 0 ) nBarW = 0;

			// TODO - This should be editable text box
//			pInterface->Text( 1, X + 110, lineY + 4, 0xd0e0e0e0, 3, "%d%%", (int)(pProperty->Value(0)*100.0f) );
			// Background
			pInterface->Rect( 0, mRenderRect.x, mRenderRect.y, nBarMaxW, mRenderRect.h, 0xf0080808 );
				
			if ( UIX::IsMouseHover( mRenderRect ) == TRUE )
			{
				UIHoverIDSet( UIX_SLIDER_BAR, 0, GetID() );
			}
			// Notches
			pInterface->Rect( 1, mRenderRect.x, mRenderRect.y + mRenderRect.h - 5, 1, 5, 0xa0909090 );
			pInterface->Rect( 1, mRenderRect.x + (nBarMaxW/2), mRenderRect.y + mRenderRect.h - 3, 1, 3, 0xa0909090 );
			pInterface->Rect( 1, mRenderRect.x + nBarMaxW, mRenderRect.y + mRenderRect.h - 5, 1, 5, 0xa0909090 );
			// Bar
			pInterface->Rect( 0, mRenderRect.x, mRenderRect.y, nBarW, mRenderRect.h, 0xf0202020 );
			// Value/Grab bar
			pInterface->Rect( 0, mRenderRect.x + nBarW - 2, mRenderRect.y, 4, mRenderRect.h, 0xf0505080 );
		}
		break;
	}
	
	if (UIX::IsUISelectionModeActive() == TRUE )
	{
	uint32		ulShadedCol = 0x40d0c040;
	uint32		ulOutlineCol = 0x80e0d080;

		if ( UIX::GetFocusedObject() == this )
		{
			ulShadedCol = 0x806080d0;
			ulOutlineCol = 0xB0a0b0f0;
		}
		else if ( UIX::IsMouseHover( mRenderRect ) )
		{
			ulShadedCol = 0x60e0d060;
			ulOutlineCol = 0xB0f0e0a0;
			UIX::CheckForPress( this, mRenderRect, UIX_OBJECT_SELECT, 0  );
		}
		else if ( GetObjectSelectionText().empty() == false )
		{
			ulShadedCol = 0x60108020;
			ulOutlineCol = 0xB020c060;
		}

		pInterface->Rect( 0, mRenderRect.x, mRenderRect.y, mRenderRect.w, mRenderRect.h, ulShadedCol );
		pInterface->OutlineBox( 0, mRenderRect.x, mRenderRect.y, mRenderRect.w, mRenderRect.h, ulOutlineCol );	

		auto	selectionText = GetObjectSelectionText();
		int		nFontHeight = 12;
		int		lineY = mRenderRect.y + (mRenderRect.h / 2) - (nFontHeight / 2);
//		pInterface->TextCentre( 1, mRenderRect.x + (mRenderRect.w/2), lineY, 0xd0d0d0d0, 3, selectionText.c_str() );
		pInterface->TextBox(1, mRenderRect.x, lineY, mRenderRect.w, 0xd0d0d0d0, 3, 0, selectionText.c_str());
	}

	displayRect.h = 0;
	displayRect.y = GetLocalPositionRect().y + GetLocalPositionRect().h + 1;		// displayRect.y returns the lowest point we drew to

	return( displayRect );
}


void		UIXSlider::OnUpdate( float fDelta )
{
	if ( mValueUpdateFunc )
	{
		switch( mMode )
		{
		case SLIDERMODE_VERTICAL_VALUE:
		case SLIDERMODE_VALUE:
		case SLIDERMODE_ANGLE:
		default:
			{	
			float	fNewVal = mValueUpdateFunc( GetID(), mfCurrentVal, 0.0f, 1.0f, mulUserParam, mbIsHeld );
				if ( fNewVal != mfCurrentVal )
				{
					mfCurrentVal = fNewVal;
				}
			}		
			break;
		case SLIDERMODE_INTVALUE_WITH_CONSTRAINTS:
		case SLIDERMODE_VALUE_WITH_CONSTRAINTS:
			if ( mfCurrentVal < mfMinVal ) mfCurrentVal = mfMinVal;
			if ( mfCurrentVal > mfMaxVal ) mfCurrentVal = mfMaxVal;
			mfCurrentVal = mValueUpdateFunc( GetID(), mfCurrentVal, mfMinVal, mfMaxVal, mulUserParam, mbIsHeld );
			break;
		case SLIDERMODE_VALUERANGE:
			mfCurrentVal = mValueUpdateFunc( GetID(), mfCurrentVal, mfMinVal, mfMaxVal, mulUserParam, mbIsHeld );
			break;
		}
	}

}
