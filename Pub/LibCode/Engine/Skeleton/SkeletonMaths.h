#ifndef ENGINE_SKELETON_MATHS_H
#define ENGINE_SKELETON_MATHS_H

//-----------------------------------------------------------------------------------------------------------
// SkeletonMaths.h
//
//  Small self-contained quaternion helpers for the skeletal animation code.
//  Conventions : Hamilton product, column vectors ( v' = q v q* ), so QuatMul( a, b ) applies b first, then a.
//  (Deliberately not using the D3DX quaternion functions, which use the opposite multiplication order)
//-----------------------------------------------------------------------------------------------------------

#include <math.h>

UNI_INLINE ENGINEQUATERNION	QuatIdentity( void )
{
ENGINEQUATERNION	xQuat = { 0.0f, 0.0f, 0.0f, 1.0f };
	return( xQuat );
}

UNI_INLINE ENGINEQUATERNION	QuatMul( const ENGINEQUATERNION* pA, const ENGINEQUATERNION* pB )
{
ENGINEQUATERNION	xOut;

	xOut.w = (pA->w * pB->w) - (pA->x * pB->x) - (pA->y * pB->y) - (pA->z * pB->z);
	xOut.x = (pA->w * pB->x) + (pA->x * pB->w) + (pA->y * pB->z) - (pA->z * pB->y);
	xOut.y = (pA->w * pB->y) - (pA->x * pB->z) + (pA->y * pB->w) + (pA->z * pB->x);
	xOut.z = (pA->w * pB->z) + (pA->x * pB->y) - (pA->y * pB->x) + (pA->z * pB->w);
	return( xOut );
}

UNI_INLINE ENGINEQUATERNION	QuatConjugate( const ENGINEQUATERNION* pA )
{
ENGINEQUATERNION	xOut;

	xOut.x = -pA->x;
	xOut.y = -pA->y;
	xOut.z = -pA->z;
	xOut.w = pA->w;
	return( xOut );
}

UNI_INLINE ENGINEQUATERNION	QuatFromAxisAngle( float fAxisX, float fAxisY, float fAxisZ, float fAngle )
{
ENGINEQUATERNION	xOut;
float	fSin = sinf( fAngle * 0.5f );

	xOut.x = fAxisX * fSin;
	xOut.y = fAxisY * fSin;
	xOut.z = fAxisZ * fSin;
	xOut.w = cosf( fAngle * 0.5f );
	return( xOut );
}

UNI_INLINE void		QuatNormalise( ENGINEQUATERNION* pA )
{
float	fLen = sqrtf( (pA->x*pA->x) + (pA->y*pA->y) + (pA->z*pA->z) + (pA->w*pA->w) );

	if ( fLen > 0.0000001f )
	{
		fLen = 1.0f / fLen;
		pA->x *= fLen;
		pA->y *= fLen;
		pA->z *= fLen;
		pA->w *= fLen;
	}
	else
	{
		*pA = QuatIdentity();
	}
}

UNI_INLINE VECT		QuatRotate( const ENGINEQUATERNION* pQ, const VECT* pV )
{
VECT	xOut;
float	fTx, fTy, fTz;

	// t = 2 * cross( q.xyz, v ) ; v' = v + w*t + cross( q.xyz, t )
	fTx = 2.0f * ( (pQ->y * pV->z) - (pQ->z * pV->y) );
	fTy = 2.0f * ( (pQ->z * pV->x) - (pQ->x * pV->z) );
	fTz = 2.0f * ( (pQ->x * pV->y) - (pQ->y * pV->x) );
	xOut.x = pV->x + (pQ->w * fTx) + ( (pQ->y * fTz) - (pQ->z * fTy) );
	xOut.y = pV->y + (pQ->w * fTy) + ( (pQ->z * fTx) - (pQ->x * fTz) );
	xOut.z = pV->z + (pQ->w * fTz) + ( (pQ->x * fTy) - (pQ->y * fTx) );
	return( xOut );
}

UNI_INLINE ENGINEQUATERNION	QuatSlerp( const ENGINEQUATERNION* pA, const ENGINEQUATERNION* pB, float fPhase )
{
ENGINEQUATERNION	xB = *pB;
ENGINEQUATERNION	xOut;
float	fDot = (pA->x*pB->x) + (pA->y*pB->y) + (pA->z*pB->z) + (pA->w*pB->w);
float	fScaleA;
float	fScaleB;

	if ( fDot < 0.0f )
	{
		fDot = -fDot;
		xB.x = -xB.x; xB.y = -xB.y; xB.z = -xB.z; xB.w = -xB.w;
	}

	if ( fDot > 0.9995f )
	{
		fScaleA = 1.0f - fPhase;
		fScaleB = fPhase;
	}
	else
	{
	float	fTheta = acosf( fDot );
	float	fSinTheta = sinf( fTheta );

		fScaleA = sinf( (1.0f - fPhase) * fTheta ) / fSinTheta;
		fScaleB = sinf( fPhase * fTheta ) / fSinTheta;
	}
	xOut.x = (pA->x * fScaleA) + (xB.x * fScaleB);
	xOut.y = (pA->y * fScaleA) + (xB.y * fScaleB);
	xOut.z = (pA->z * fScaleA) + (xB.z * fScaleB);
	xOut.w = (pA->w * fScaleA) + (xB.w * fScaleB);
	QuatNormalise( &xOut );
	return( xOut );
}

// Shortest-arc rotation that takes direction pFrom onto direction pTo
UNI_INLINE ENGINEQUATERNION	QuatFromTo( const VECT* pFrom, const VECT* pTo )
{
VECT	xFrom = *pFrom;
VECT	xTo = *pTo;
ENGINEQUATERNION	xOut;
float	fLenFrom = sqrtf( (xFrom.x*xFrom.x) + (xFrom.y*xFrom.y) + (xFrom.z*xFrom.z) );
float	fLenTo = sqrtf( (xTo.x*xTo.x) + (xTo.y*xTo.y) + (xTo.z*xTo.z) );
float	fDot;

	if ( ( fLenFrom < 0.000001f ) || ( fLenTo < 0.000001f ) ) return( QuatIdentity() );

	xFrom.x /= fLenFrom; xFrom.y /= fLenFrom; xFrom.z /= fLenFrom;
	xTo.x /= fLenTo; xTo.y /= fLenTo; xTo.z /= fLenTo;
	fDot = (xFrom.x*xTo.x) + (xFrom.y*xTo.y) + (xFrom.z*xTo.z);

	if ( fDot < -0.99999f )
	{
	VECT	xAxis;
		// 180 degrees - pick any perpendicular axis
		xAxis.x = 0.0f; xAxis.y = -xFrom.z; xAxis.z = xFrom.y;
		if ( (xAxis.y*xAxis.y) + (xAxis.z*xAxis.z) < 0.0001f )
		{
			xAxis.x = xFrom.z; xAxis.y = 0.0f; xAxis.z = -xFrom.x;
		}
		VectNormalize( &xAxis );
		return( QuatFromAxisAngle( xAxis.x, xAxis.y, xAxis.z, PI ) );
	}
	xOut.x = (xFrom.y * xTo.z) - (xFrom.z * xTo.y);
	xOut.y = (xFrom.z * xTo.x) - (xFrom.x * xTo.z);
	xOut.z = (xFrom.x * xTo.y) - (xFrom.y * xTo.x);
	xOut.w = 1.0f + fDot;
	QuatNormalise( &xOut );
	return( xOut );
}

#endif
