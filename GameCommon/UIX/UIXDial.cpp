
#include "StandardDef.h"
#include "InterfaceEx.h"

#include "../UI/UI.h"
#include "UIXDial.h"


void	UIXDial::Initialise( uint32 ulUserParam, float fInitialVal )
{
	mulUserParam = ulUserParam;
	mfCurrentVal = fInitialVal;
}



void	UIXDial::OnHeldUpdate( BOOL bIsHeld, BOOL bFirstPress )
{
	if ( bIsHeld )
	{
//		mfCurrentVal = GetCursorValue();
	}
	mbIsHeld = bIsHeld;
}



UIXRECT		UIXDial::OnRender( InterfaceInstance* pInterface, UIXRECT displayRect )
{
UIXRECT		renderRect = GetActualRenderRect( displayRect );
uint32		ulCol = 0xf0505070;

	mRenderRect = renderRect;

	// TODO - Draw a dial UI here, first, using 'pInterface->Triangle'  to create a smooth render a partial border circle that stretches from the equivalent of 7 on a clock face round to 5
	{
		const float kPi          = 3.14159265f;
		const float kDegToRad    = kPi / 180.0f;
		const float kStartAngle  = 220.0f;   // 7 o'clock, degrees clockwise from 12
		const float kTotalArc    = 280.0f;   // sweeps round to 5 o'clock
		const int   kSegments    = 32;

		const int cx          = renderRect.x + renderRect.w / 2;
		const int cy          = renderRect.y + renderRect.h / 2;
		const int outerRadius = ( renderRect.w < renderRect.h ? renderRect.w : renderRect.h ) / 2 - 1;
		const int innerRadius = outerRadius - 4;

		const uint32 ulBgCol   = 0xf0080808;	
		const uint32 ulFillCol = 0xa0c0c0e0;

		const float fValueAngle = kStartAngle + mfCurrentVal * kTotalArc;

		for ( int i = 0; i < kSegments; i++ )
		{
			const float a0 = ( kStartAngle + kTotalArc * i / kSegments ) * kDegToRad;
			const float a1 = ( kStartAngle + kTotalArc * ( i + 1 ) / kSegments ) * kDegToRad;

			// Colour segment based on whether it falls within the filled value range
			const float segMidAngle = kStartAngle + kTotalArc * ( i + 0.5f ) / kSegments;
			const uint32 ulSegCol   = ( segMidAngle <= fValueAngle ) ? ulFillCol : ulBgCol;

			const int ox0 = (int)( outerRadius * sinf( a0 ) );
			const int oy0 = (int)( -outerRadius * cosf( a0 ) );
			const int ox1 = (int)( outerRadius * sinf( a1 ) );
			const int oy1 = (int)( -outerRadius * cosf( a1 ) );
			const int ix0 = (int)( innerRadius * sinf( a0 ) );
			const int iy0 = (int)( -innerRadius * cosf( a0 ) );
			const int ix1 = (int)( innerRadius * sinf( a1 ) );
			const int iy1 = (int)( -innerRadius * cosf( a1 ) );

			// Two triangles form a quad between inner and outer arc edges
			pInterface->Triangle( 1,
				cx + ox0, cy + oy0,
				cx + ox1, cy + oy1,
				cx + ix0, cy + iy0,
				ulSegCol, ulSegCol, ulSegCol );

			pInterface->Triangle( 1,
				cx + ox1, cy + oy1,
				cx + ix1, cy + iy1,
				cx + ix0, cy + iy0,
				ulSegCol, ulSegCol, ulSegCol );
		}

				// --- Pointer needle ---
		// Diamond shape: wide at pivot, tapering to a forward tip (at innerRadius)
		// and a shorter tail pointing away from the arc.
		{
			const float valRad         = fValueAngle * kDegToRad;

			// Forward/backward unit vectors (clockwise-from-12 convention)
			const float fwdX =  sinf( valRad );
			const float fwdY = -cosf( valRad );

			// Perpendicular unit vector (90deg clockwise from forward)
			const float perpX =  cosf( valRad );
			const float perpY =  sinf( valRad );

			// Key distances
			const int nPointerFwdLen  = innerRadius;            // tip reaches inner edge of arc
			const int nPointerBckLen  = innerRadius / 3;        // tail, opposite direction
			const int nPointerHalfW   = 3;                      // half-width at pivot

			// Computed points
			const int tipFwdX  = cx + (int)( fwdX * nPointerFwdLen );
			const int tipFwdY  = cy + (int)( fwdY * nPointerFwdLen );

			const int tipBckX  = cx - (int)( fwdX * nPointerBckLen );
			const int tipBckY  = cy - (int)( fwdY * nPointerBckLen );

			const int wingLX   = cx + (int)( perpX * nPointerHalfW );
			const int wingLY   = cy + (int)( perpY * nPointerHalfW );

			const int wingRX   = cx - (int)( perpX * nPointerHalfW );
			const int wingRY   = cy - (int)( perpY * nPointerHalfW );

			const uint32 ulPointerTipCol  = 0xffffffff;
			const uint32 ulPointerBaseCol = 0x80ffffff;
			const uint32 ulPointerTailCol = 0xc0808090;

			// Two front triangles (longer, towards arc inner edge)
			pInterface->Triangle( 1,
				cx,      cy,
				wingLX,  wingLY,
				tipFwdX, tipFwdY,
				ulPointerBaseCol, ulPointerBaseCol, ulPointerTipCol );

			pInterface->Triangle( 1,
				cx,      cy,
				tipFwdX, tipFwdY,
				wingRX,  wingRY,
				ulPointerBaseCol, ulPointerTipCol, ulPointerBaseCol );

			// Two back triangles (shorter, tail away from arc)
			pInterface->Triangle( 1,
				cx,      cy,
				tipBckX, tipBckY,
				wingLX,  wingLY,
				ulPointerBaseCol, ulPointerTailCol, ulPointerBaseCol );

			pInterface->Triangle( 1,
				cx,      cy,
				wingRX,  wingRY,
				tipBckX, tipBckY,
				ulPointerBaseCol, ulPointerBaseCol, ulPointerTailCol );
		}
	}


//	pInterface->Triangle(1, mRenderRect.x + (mRenderRect.w / 2), mRenderRect.y, mRenderRect.x + mRenderRect.w - 1, mRenderRect.y + mRenderRect.h - 1, mRenderRect.x + 1, mRenderRect.y + mRenderRect.h - 1, ulCol, ulCol, ulCol);

	// Notches

	displayRect.h = 0;
	displayRect.y = GetLocalPositionRect().y + GetLocalPositionRect().h + 1;		// displayRect.y returns the lowest point we drew to

	return( displayRect );
}


void		UIXDial::OnUpdate( float fDelta )
{
	if ( mValueUpdateFunc )
	{
	float	fNewVal = mValueUpdateFunc( GetID(), mfCurrentVal, 0.0f, 1.0f, mulUserParam, mbIsHeld );
		if ( fNewVal != mfCurrentVal )
		{
			mfCurrentVal = fNewVal;
		}
	}		

}
