
#include <stdio.h>
#include <float.h>
#include <math.h>

#include <StandardDef.h>
#include <Engine.h>
#include <Interface.h>



//------------------------------------------------------------------------
FLOAT RotateAngleTowardsAngleWithLag( float fAngleToChange, float fTargetAngle, float fTurnRate, float fLagBounds )
{
float	fDiff;
float	fTurnRateMod;
float	fLag = 1.0f;
float	fAbsDiff;
float	fDirDiff;
//char	acString[256];

/*
	if ( !_finite( fTargetAngle ) ) fTargetAngle = 0.0f;
	if ( !_finite( fAngleToChange ) ) fAngleToChange = 0.0f;
*/
	fDiff = fAngleToChange - fTargetAngle;

//	sprintf( acString, "target %.3f, angle %.3f, diff %.3f", fTargetAngle, fAngleToChange, fDiff );
//	InterfaceText(0, 200,200,acString, 0xFFFFFFFF, 0 );
	while ( fDiff < 0 )
	{
		fDiff += A360;
	}
	
	while ( fDiff > A360 )
	{
		fDiff -= A360;
	}
	fAbsDiff = fDiff;
	fDirDiff = fDiff;

	if ( fDiff > A180 )
	{
		fAbsDiff = A360 - fDiff;
		fDirDiff = fDiff - A360;
	}
//	sprintf( acString, "moddiff %.3f, absdiff %.3f, lagbounds %.3f, dirdiff %.3f", fDiff, fAbsDiff, fLagBounds, fDirDiff );
//	InterfaceText(0, 200,220,acString, 0xFFFFFFFF, 0 );

	if ( fAbsDiff < fLagBounds )
	{
		fLag = (fAbsDiff / fLagBounds) + 0.1f;
		if ( fLag > 1.0f ) fLag = 1.0f;
		fTurnRate *= fLag;
	}
	else
	{
		fLag = (fAbsDiff / fLagBounds);
		fLag *= fLag;
		fTurnRate *= fLag;
	}

	if ( fTurnRate < 0.0f )
	{
		fTurnRateMod = 0 - fTurnRate;
	}
	else
	{
		fTurnRateMod = fTurnRate;
	}

	if ( (fDiff < fTurnRateMod) || (fTurnRateMod > A360-fDiff) )
	{
		fAngleToChange = fTargetAngle;
		return ( fAngleToChange );
	}

	/** If the turn rotation is too big, but not more than 180 degrees to big **/
	if ( fDiff < A180 )
	{
		fAngleToChange -= fTurnRate;
		if ( fAngleToChange < 0.0f )
		{
			fAngleToChange += A360;
		}
	}	
	else
	{
		fAngleToChange += fTurnRate;
		if ( fAngleToChange > A360 )
		{
			fAngleToChange -= A360;
		}
	}

	return( fAngleToChange );
} 
