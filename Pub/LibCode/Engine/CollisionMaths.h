
#ifndef COLLISION_MATHS_H
#define	COLLISION_MATHS_H

#ifdef __cplusplus
extern "C"				// All interfaces use a C-linkage
{
#endif
	
extern BOOL	SweepSphereTriBoundsCheck( VECT* pTri1,VECT* pTri2,VECT* pTri3,VECT* pSphereStart, VECT* pSphereEnd, float fSphereRadius );
extern BOOL	SweepSphereTriCollision( VECT* pTri1,VECT* pTri2,VECT* pTri3,VECT* pSphereStart, VECT* pSphereEnd, float fSphereRadius, VECT* pxHit, VECT* pxNormal, float* );



//----------------------------------------------------------------
extern VECT		mxCollResultNormal;
extern VECT		mxCollResultPos;
extern VECT		mxCollResultImpulse;
extern float	mfCollResultDist;
//----------------------------------------------------------------


#ifdef __cplusplus
}
#endif

#endif
