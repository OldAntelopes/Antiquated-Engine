#ifndef UNIVERSAL_MATHS_LIB_H
#define UNIVERSAL_MATHS_LIB_H

#ifdef __cplusplus
extern "C"
{
#endif

//------------------------------------
//   Maths.h
//
//-----------------------------------

#ifndef FLTPI
#define	FLTPI					3.1415926535f
#endif

#define	A10		(FLTPI/18.0f)
#define	A30		(FLTPI*0.166666667f)
#define	A45		(FLTPI*0.25f)
#define	A60		(FLTPI*0.333333333f)
#define	A90		(FLTPI*0.5f)
#define	A180	(FLTPI)
#define	A270	(FLTPI*1.5f)
#define	A360	(FLTPI*2.0f)

//--------------------------------------------------------------
// Typedefs for a couple of necessary structures (matrices and vectors)
//	 Bit untidy this - they may well get moved to a separate maths lib/header 
//   somepoint later..
//  -----------------------------------------------------------
#ifndef D3DMATRIX_DEFINED
typedef struct _D3DMATRIX {
    union {
        struct {
            float        _11, _12, _13, _14;
            float        _21, _22, _23, _24;
            float        _31, _32, _33, _34;
            float        _41, _42, _43, _44;

        };
        float m[4][4];
    };
} D3DMATRIX;
#define D3DMATRIX_DEFINED
#endif

#define	ENGINEMATRIX	D3DMATRIX

// Matrix functions
extern	void	MatrixCreateScale( ENGINEMATRIX* pMat, float fScale );
extern	void	MatrixIdentity( ENGINEMATRIX* pMat );


#ifndef TRACTOR_STD_DEF_H
typedef struct
{
/*
	union {	float	x;	float fX; };
    union {	float	y;	float fY; };
    union {	float	z;	float fZ; };
*/
 	float	x;
    float	y;	
    float	z;	
} VECT;

typedef VECT            MVECT;

typedef struct
{
	float	x;
	float	y;
	float	z;
	float	w;
} ENGINEQUATERNION;


#endif

extern float	VectDist( const VECT*, const VECT* );
extern float	VectDistNoZ( const VECT*, const VECT* );
extern float	VectGetLength( const VECT* );
extern float	VectNormalize( VECT* );
extern float	VectDot( const VECT*, const VECT* );
extern void		VectCross( VECT*, const VECT*, const VECT* );
extern void		VectScale( VECT* pOut, const VECT* pSrc, float fScale );
extern void		VectAdd( VECT* pOut, const VECT* pSrc, const VECT* pSrc2 );
extern void		VectSub( VECT* pOut, const VECT* pSrc, const VECT* pSrc2 );
extern void		VectTransform( VECT* pOut, const VECT* pSrc, const ENGINEMATRIX* pxMatrix );
extern void		VectLerp( VECT* pOut, const VECT* pPosWhenPhaseIsZero, const VECT* pPosWhenPhaseIsOne, float fPhase );
extern void		VectSlerp( VECT* pOut, const VECT* pPosWhenPhaseIsZero, const VECT* pPosWhenPhaseIsOne, float fPhase );
extern void		VectProject( VECT* pxOut, const VECT* pxIn, ENGINEMATRIX* pxProjMat, ENGINEMATRIX* pxViewMat, ENGINEMATRIX* pxWorldMat );

extern void		VectHermite( VECT* pOut, const VECT* pPos1, const VECT* pTangent1, const VECT* pPos2, const VECT* pTangent2, float fVal );
extern void		VectReflect( VECT* pxOut, const VECT* pxInVect, const VECT* pxPlaneNormal );

extern void		VectRotateAboutX( VECT*, float );
extern void		VectRotateAboutY( VECT*, float );
extern void		VectRotateAboutZ( VECT*, float );
extern void		VectRotateAboutAxis( VECT* pOut, VECT* pSrc, VECT* pxAxis, float fAngle );

extern float	VectGetRotZAngleBetweenVects( const VECT* pxVect1, const VECT* pxVect2 );

extern BOOL		PlaneIntersectLine( VECT* pPlaneOrigin, VECT* pPlaneNormal, VECT* pRayStart, VECT* pRayEnd, VECT* pCollisionOut );

extern void	EngineMatrixIdentity( ENGINEMATRIX* pxMatrix );

extern void EngineSetMatrixFromRotations( const VECT* pxRot, ENGINEMATRIX* pxMatrix );
extern void EngineSetMatrixXYZ( ENGINEMATRIX* pxMatrix, float, float, float );

extern void EngineMatrixMultiply( ENGINEMATRIX* pxMatrix1, const ENGINEMATRIX* pMatrix2 );

extern void	EngineMatrixRotationX( ENGINEMATRIX* pxMatrix, float );
extern void	EngineMatrixRotationY( ENGINEMATRIX* pxMatrix, float );
extern void	EngineMatrixRotationZ( ENGINEMATRIX* pxMatrix, float );
extern void	EngineMatrixRotationAxis( ENGINEMATRIX* pxMatrix, VECT* pxAxis, float );
extern void	EngineMatrixInverse( ENGINEMATRIX* pxMatrix );

extern void	EngineMatrixScaling( ENGINEMATRIX* pxMatrix, float x, float y, float z );

extern void EngineMatrixLookAt( ENGINEMATRIX* pxMatrix, const VECT* pPos, const VECT* pLookAt, const VECT* pUp );
extern void	EngineMatrixOrtho( ENGINEMATRIX* pxOutMatrix, float fWidth, float fHeight );

extern float RotateAngleTowardsAngleWithLag( float fAngleToChange, float fTargetAngle, float fTurnRate, float fLagBounds );

//-------------------------------------- Quaternions

extern void	EngineQuaternionFromMatrix( ENGINEQUATERNION* pOut, const ENGINEMATRIX* pIn );
extern void EngineQuaternionFromAxes( ENGINEQUATERNION* pxQuatOut, const VECT* pxForward, const VECT* pxRight, const VECT* pxUp );
extern void EngineAxesFromQuaternion( const ENGINEQUATERNION* pxQuat, VECT* pxForward, VECT* pxRight, VECT* pxUp );

extern void	EngineMatrixFromQuaternion( ENGINEMATRIX* pOut, const ENGINEQUATERNION* pIn );

extern void	EngineQuaternionSlerp( ENGINEQUATERNION* pOut, ENGINEQUATERNION* pIn1, ENGINEQUATERNION* pIn2, float fBlendAmount);
extern void	EngineQuaternionNormalize( ENGINEQUATERNION* pOut );
extern float	EngineQuaternionDot( ENGINEQUATERNION* pQuat1, ENGINEQUATERNION* pQuat2 );

//-------------------------------------------- Planes
typedef struct
{
	float	a;
	float	b;
	float	c;
	float	d;
} ENGINEPLANE;

extern void		EnginePlaneFromPoints( ENGINEPLANE* pxPlaneOut, const VECT* pxVect1, const VECT* pxVect2, const VECT* pxVect3 );
extern void		EnginePlaneFromPointNormal( ENGINEPLANE* pxOut, const VECT* pxPoint, const VECT* pxNormal );
extern BOOL		EnginePlaneIntersectLine( VECT* pxInteresectPointOut, const ENGINEPLANE* pxPlanet, const VECT* pxLineStart, const VECT* pxLineEnd );
extern float	EnginePlaneDotCoord( const ENGINEPLANE* pxPlane, const VECT* pxPoint );
extern void		EnginePlaneNormalize( ENGINEPLANE* pxPlaneOut, const ENGINEPLANE* pxPlane );

extern void		EngineMatrixShadow( ENGINEMATRIX* pxOutMatrix, const VECT* pxLight, const ENGINEPLANE* pxPlane );

#ifdef __cplusplus
}
#endif

#endif
