
#include <math.h>
#include "StandardDef.h"
#include "Engine.h"

#include "MathsUtil.h"

float	WrapAngle360( float fAngle )
{
	if ( fAngle < 0.0f )
	{
		fAngle += A360;
	}
	else if ( fAngle >= A360 )
	{
		fAngle -= A360;
	}
	return( fAngle );
}

float		GetDestinationPitch( const VECT* pxDestination, const VECT* pxPos )
{
VECT	xTravelDir;
float	fDestinationPitch;
float	fHoriz;

	VectSub( &xTravelDir, pxDestination, pxPos );

	fHoriz = VectDistNoZ( pxDestination, pxPos );
	fDestinationPitch = A360 - atan2f( xTravelDir.z, fHoriz );
//	fDestinationPitch = atan2f( fHoriz, xTravelDir.z );

	if ( fDestinationPitch < 0.0f ) fDestinationPitch += A360;
	if ( fDestinationPitch > A360 ) fDestinationPitch -= A360;

	return( fDestinationPitch );
}

float		GetDestinationRotZ( const VECT* pxDestination, const VECT* pxPos )
{
VECT	xTravelDir;
float	fDestinationRotZ;

	VectSub( &xTravelDir, pxDestination, pxPos );
	fDestinationRotZ = (A180 - atan2f( xTravelDir.x, xTravelDir.y )) - A90;

	if ( fDestinationRotZ < 0.0f ) fDestinationRotZ += A360;
	if ( fDestinationRotZ > A360 ) fDestinationRotZ -= A360;

	return( fDestinationRotZ );
}

float	GraphStructGetValue( GRAPH_STRUCT* pxGraphStruct, float fIndex )
{
float		fVal;
float		fLastIndex = pxGraphStruct->fIndex;
float		fLastVal = pxGraphStruct->fValue;

	if ( fIndex > 1.0f ) fIndex = 1.0f;

	while( pxGraphStruct->fIndex < fIndex )
	{
		fLastVal = pxGraphStruct->fValue;
		fLastIndex = pxGraphStruct->fIndex;
		pxGraphStruct++;
	}

	if ( pxGraphStruct->fIndex > fIndex )
	{
	float	fRange = pxGraphStruct->fIndex - fLastIndex;
	float	fOverstep = fIndex - fLastIndex;
	float	fMod = fOverstep / fRange;

		fVal = (fLastVal * (1.0f-fMod) ) + (pxGraphStruct->fValue * fMod);
		return( fVal );
	}
	else
	{
		return( fLastVal );
	}
}


/***************************************************************************
 * Function    : RotateAngleTowardsAngle
 * Params      : Angle to change
				 Target angle
				 Rate to change
 * Description : This function is used to rotate one angle towards another
				 at a constant rate
 ***************************************************************************/
float RotateAngleTowardsAngle( float fAngleToChange, float fTargetAngle, float fTurnRate )
{
float	fDiff;
float	fTurnRateMod;

//	if ( !_finite( fTargetAngle ) ) fTargetAngle = 0.0f;
//	if ( !_finite( fAngleToChange ) ) fAngleToChange = 0.0f;

	fDiff = fAngleToChange - fTargetAngle;

	while ( fDiff < 0 )
	{
		fDiff += A360;
	}
	
	while ( fDiff > A360 )
	{
		fDiff -= A360;
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
