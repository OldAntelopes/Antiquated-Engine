
#include <math.h>
#include <float.h>

#include <windows.h>		// For OpenGL
#include <gl/gl.h>

#include <StandardDef.h>
#include <Engine.h>

#define T0 0
#define T1 4
#define T2 8
#define T3 12
#define T4 1
#define T5 5
#define T6 9
#define T7 13
#define T8 2
#define T9 6
#define T10 10
#define T11 14
#define T12 3
#define T13 7
#define T14 11
#define T15 15

class MathHelper {
public:
	// This is a column-order matrix multiply of matrices m1 and m2.
	static void matrixMult4x4 (GLfloat* result, GLfloat* m1, GLfloat* m2);
	static void matrixMult4x4Column (GLfloat* result, GLfloat* m1, GLfloat* m2);
	// Makes an identity matrix
	static void makeIdentity (GLfloat* result);
	
	// Returns a matrix that translates by x, y and z amount
	static void makeTranslate (GLfloat* result, GLfloat x, GLfloat y, GLfloat z);
	// Updates result to include a translate
	static void translateMatrixBy (GLfloat* result, GLfloat x, GLfloat y, GLfloat z);

	// Returns matrices that rotate about the X, Y and Z axes by a rotation amount (radians)
	static void makeRotateX (GLfloat* result, GLfloat rotation);
	static void makeRotateY (GLfloat* result, GLfloat rotation);
	static void makeRotateZ (GLfloat* result, GLfloat rotation);

	// 1) Fill in the "makeScale" prototype. Note: this should scale across all three axes

	
	// Copies the src matrix into the dest matrix
	static void copyMatrix (GLfloat* src, GLfloat* dest);
	static void print4x4Matrix(GLfloat* mat);

private:
};

// This is a column-order matrix multiply of matrices m1 and m2.
void MathHelper::matrixMult4x4Column (GLfloat* result, GLfloat* m1, GLfloat* m2)
{
	for (int i = 0; i < 16; i++) {
		result[i] = 0.0f;
	}
	result[T0] = m1[T0]*m2[T0]+m1[T1]*m2[T4]+m1[T2]*m2[T8]+m1[T3]*m2[T12];
	result[T1] = m1[T0]*m2[T1]+m1[T1]*m2[T5]+m1[T2]*m2[T9]+m1[T3]*m2[T13];
	result[T2] = m1[T0]*m2[T2]+m1[T1]*m2[T6]+m1[T2]*m2[T10]+m1[T3]*m2[T14];
	result[T3] = m1[T0]*m2[T3]+m1[T1]*m2[T7]+m1[T2]*m2[T11]+m1[T3]*m2[T15];

	result[T4] = m1[T4]*m2[T0]+m1[T5]*m2[T4]+m1[T6]*m2[T8]+m1[T7]*m2[T12];
	result[T5] = m1[T4]*m2[T1]+m1[T5]*m2[T5]+m1[T6]*m2[T9]+m1[T7]*m2[T13];
	result[T6] = m1[T4]*m2[T2]+m1[T5]*m2[T6]+m1[T6]*m2[T10]+m1[T7]*m2[T14];
	result[T7] = m1[T4]*m2[T3]+m1[T5]*m2[T7]+m1[T6]*m2[T11]+m1[T7]*m2[T15];

	result[T8] = m1[T8]*m2[T0]+m1[T9]*m2[T4]+m1[T10]*m2[T8]+m1[T11]*m2[T12];
	result[T9] = m1[T8]*m2[T1]+m1[T9]*m2[T5]+m1[T10]*m2[T9]+m1[T11]*m2[T13];
	result[T10] = m1[T8]*m2[T2]+m1[T9]*m2[T6]+m1[T10]*m2[T10]+m1[T11]*m2[T14];
	result[T11] = m1[T8]*m2[T3]+m1[T9]*m2[T7]+m1[T10]*m2[T11]+m1[T11]*m2[T15];

	result[T12] = m1[T12]*m2[T0]+m1[T13]*m2[T4]+m1[T14]*m2[T8]+m1[T15]*m2[T12];
	result[T13] = m1[T12]*m2[T1]+m1[T13]*m2[T5]+m1[T14]*m2[T9]+m1[T15]*m2[T13];
	result[T14] = m1[T12]*m2[T2]+m1[T13]*m2[T6]+m1[T14]*m2[T10]+m1[T15]*m2[T14];
	result[T15] = m1[T12]*m2[T3]+m1[T13]*m2[T7]+m1[T14]*m2[T11]+m1[T15]*m2[T15];
}
void MathHelper::matrixMult4x4 (GLfloat* result, GLfloat* m1, GLfloat* m2)
{
	for (int i = 0; i < 16; i++) {
		result[i] = 0.0f;
	}
	result[0] = m1[0]*m2[0]+m1[1]*m2[4]+m1[2]*m2[8]+m1[3]*m2[12];
	result[1] = m1[0]*m2[1]+m1[1]*m2[5]+m1[2]*m2[9]+m1[3]*m2[13];
	result[2] = m1[0]*m2[2]+m1[1]*m2[6]+m1[2]*m2[10]+m1[3]*m2[14];
	result[3] = m1[0]*m2[3]+m1[1]*m2[7]+m1[2]*m2[11]+m1[3]*m2[15];

	result[4] = m1[4]*m2[0]+m1[5]*m2[4]+m1[6]*m2[8]+m1[7]*m2[12];
	result[5] = m1[4]*m2[1]+m1[5]*m2[5]+m1[6]*m2[9]+m1[7]*m2[13];
	result[6] = m1[4]*m2[2]+m1[5]*m2[6]+m1[6]*m2[10]+m1[7]*m2[14];
	result[7] = m1[4]*m2[3]+m1[5]*m2[7]+m1[6]*m2[11]+m1[7]*m2[15];

	result[8] = m1[8]*m2[0]+m1[9]*m2[4]+m1[10]*m2[8]+m1[11]*m2[12];
	result[9] = m1[8]*m2[1]+m1[9]*m2[5]+m1[10]*m2[9]+m1[11]*m2[13];
	result[10] = m1[8]*m2[2]+m1[9]*m2[6]+m1[10]*m2[10]+m1[11]*m2[14];
	result[11] = m1[8]*m2[3]+m1[9]*m2[7]+m1[10]*m2[11]+m1[11]*m2[15];

	result[12] = m1[12]*m2[0]+m1[13]*m2[4]+m1[14]*m2[8]+m1[15]*m2[12];
	result[13] = m1[12]*m2[1]+m1[13]*m2[5]+m1[14]*m2[9]+m1[15]*m2[13];
	result[14] = m1[12]*m2[2]+m1[13]*m2[6]+m1[14]*m2[10]+m1[15]*m2[14];
	result[15] = m1[12]*m2[3]+m1[13]*m2[7]+m1[14]*m2[11]+m1[15]*m2[15];
}


// Makes an identity matrix
void MathHelper::makeIdentity (GLfloat* result)
{
	for (int i = 0; i < 16; i++) {
		result[i] = 0.0f;
	}
	result[0] = result[5] = result[10] = result[15]= 1.0f;
}

// Returns a matrix that translates by x, y and z amount
void MathHelper::makeTranslate (GLfloat* result, GLfloat x, GLfloat y, GLfloat z)
{
	makeIdentity(result);
	result[3] = x;
	result[7] = y;
	result[11] = z;
}

// Returns matrices that rotate about the X, Y and Z axes by a rotation amount (radians)
void MathHelper::makeRotateX (GLfloat* result, GLfloat rotation)
{
	makeIdentity(result);
	result[5] = result[10] = cos(rotation);
	result[6] = sin(rotation);
	result[9] = -result[6];
}
void MathHelper::makeRotateY (GLfloat* result, GLfloat rotation)
{
	makeIdentity(result);
	result[0] = result[10] = cos(rotation);
	result[2] = -sin(rotation);
	result[8] = -result[2];
}
void MathHelper::makeRotateZ (GLfloat* result, GLfloat rotation)
{
	makeIdentity(result);
	result[0] = result[5] = cos(rotation);
	result[1] = sin(rotation);
	result[4] = -result[1];
}


void MathHelper::translateMatrixBy (GLfloat* result, GLfloat x, GLfloat y, GLfloat z)
{
	result[3] += x;
	result[7] += y;
	result[11] += z;
}

void MathHelper::copyMatrix (GLfloat* src, GLfloat* dest)
{
	for (int i = 0; i < 16; i++) {
		dest[i] = src[i];
	}
}

void		VectSlerp( VECT* pOut, const VECT* pPosWhenPhaseIsZero, const VECT* pPosWhenPhaseIsOne, float fPhase )
{
float	fSlerpPhase;

	fSlerpPhase = ( fPhase * A180 ) - A90;
	fSlerpPhase = ((sinf( fSlerpPhase )) + 1.0f) * 0.5f;
	pOut->x = ( pPosWhenPhaseIsZero->x * (1.0f-fSlerpPhase) ) + ( pPosWhenPhaseIsOne->x * fSlerpPhase );
	pOut->y = ( pPosWhenPhaseIsZero->y * (1.0f-fSlerpPhase) ) + ( pPosWhenPhaseIsOne->y * fSlerpPhase );
	pOut->z = ( pPosWhenPhaseIsZero->z * (1.0f-fSlerpPhase) ) + ( pPosWhenPhaseIsOne->z * fSlerpPhase );
	
}


float	VectGetLength( const VECT* pVect )
{
	return( sqrtf( pVect->x*pVect->x + pVect->y*pVect->y + pVect->z*pVect->z ) );
}


float	VectDot( const VECT* pVect1, const VECT* pVect2 )
{
	return( (pVect1->x * pVect2->x) + (pVect1->y * pVect2->y) + (pVect1->z * pVect2->z));
}

void		VectCross( VECT* pOut, const VECT* pIn1, const VECT* pIn2 )
{
	pOut->x = (pIn1->y * pIn2->z) - (pIn1->z * pIn2->y);
	pOut->y = (pIn1->z * pIn2->x) - (pIn1->x * pIn2->z);
	pOut->z = (pIn1->x * pIn2->y) - (pIn1->y * pIn2->x);
}

void		VectSub( VECT* pOut, const VECT* pSrc, const VECT* pSrc2 )
{
	pOut->x = pSrc->x - pSrc2->x;
	pOut->y = pSrc->y - pSrc2->y;
	pOut->z = pSrc->z - pSrc2->z;
}

void		VectLerp( VECT* pOut, const VECT* pSrc, const VECT* pSrc2, float fTime )
{
	// todo
}

void		VectAdd( VECT* pOut, const VECT* pSrc, const VECT* pSrc2 )
{
	pOut->x = pSrc->x + pSrc2->x;
	pOut->y = pSrc->y + pSrc2->y;
	pOut->z = pSrc->z + pSrc2->z;
}

void	VectScale( VECT* pOut, const VECT* pSrc, float fScale )
{
	pOut->x = pSrc->x * fScale;
	pOut->y = pSrc->y * fScale;
	pOut->z = pSrc->z * fScale;
}

float	VectNormalize( VECT* pVect )
{
float	fLen = VectGetLength( pVect );
	
	if ( fLen > 0.0f )
	{
		VectScale( pVect, pVect, 1.0f/fLen );
	}
	return( fLen );
}

float	VectDist( const VECT* pxVect1, const VECT* pxVect2 )
{
VECT	xVec;
	xVec.x = pxVect2->x - pxVect1->x;
	xVec.y = pxVect2->y - pxVect1->y;
	xVec.z = pxVect2->z - pxVect1->z;
	return( VectGetLength( &xVec ) );
}


float	VectDistNoZ( const VECT* pxVect1, const VECT* pxVect2 )
{
VECT	xVec;
	xVec.x = pxVect2->x - pxVect1->x;
	xVec.y = pxVect2->y - pxVect1->y;
	xVec.z = 0.0f;
	return( VectGetLength( &xVec ) );
}


void		VectTransform( VECT* pOut, const VECT* pSrc, const ENGINEMATRIX* pxMatrix )
{
	pOut->x = (pSrc->x * pxMatrix->_11) + (pSrc->y * pxMatrix->_21) + (pSrc->z * pxMatrix->_31) + pxMatrix->_41;
	pOut->y = (pSrc->x * pxMatrix->_12) + (pSrc->y * pxMatrix->_22) + (pSrc->z * pxMatrix->_32) + pxMatrix->_42;
	pOut->z = (pSrc->x * pxMatrix->_13) + (pSrc->y * pxMatrix->_23) + (pSrc->z * pxMatrix->_33) + pxMatrix->_43;
}

void	VectProject( VECT* pxOut, const VECT* pxIn, ENGINEMATRIX* pxProjMat, ENGINEMATRIX* pxViewMat, ENGINEMATRIX* pxWorldMat )
{
	VectTransform( pxOut, pxIn, pxWorldMat );
	VectTransform( pxOut, pxOut, pxViewMat );
	VectTransform( pxOut, pxOut, pxProjMat );
}

void		VectRotateAboutAxis( VECT* pOut, VECT* pSrc, VECT* pxAxis, float fAngle )
{
	// todo

}
void		VectRotateAboutY( VECT* pVec, float fAngle )
{
	// todo

}


void EngineSetMatrixXYZ( ENGINEMATRIX* pxMatrix, float fX, float fY, float fZ )
{
	// todo
}

void EngineSetMatrixFromRotations( const VECT* pxRot, ENGINEMATRIX* pxMatrix )
{
	// todo

}

void		EnginePlaneFromPointNormal( ENGINEPLANE* pxOut, const VECT* pxPoint, const VECT* pxNormal )
{
	pxOut->a = pxNormal->x;
	pxOut->b = pxNormal->y;
	pxOut->c = pxNormal->z;
	pxOut->d = 0.0f - VectDot( pxPoint, pxNormal );
}


BOOL	EnginePlaneIntersectLine( VECT* pxInteresectPointOut, const ENGINEPLANE* pxPlane, const VECT* pxLineStart, const VECT* pxLineEnd )
{

	return( FALSE );
}

void	EngineQuaternionNormalize( ENGINEQUATERNION* pOut )
{
	// todo
}

void	EngineQuaternionFromMatrix( ENGINEQUATERNION* pOut, const ENGINEMATRIX* pIn )
{
	// todo
}

void	EngineMatrixFromQuaternion( ENGINEMATRIX* pOut, const ENGINEQUATERNION* pIn )
{
	EngineMatrixIdentity(pOut);
	pOut->m[0][0] = 1.0f - 2.0f * (pIn->y * pIn->y + pIn->z * pIn->z);
	pOut->m[0][1] = 2.0f * (pIn->x * pIn->y + pIn->z * pIn->w);
	pOut->m[0][2] = 2.0f * (pIn->x * pIn->z - pIn->y * pIn->w);
	pOut->m[1][0] = 2.0f * (pIn->x * pIn->y - pIn->z * pIn->w);
	pOut->m[1][1] = 1.0f - 2.0f * (pIn->x * pIn->x + pIn->z * pIn->z);
	pOut->m[1][2] = 2.0f * (pIn->y *pIn->z + pIn->x *pIn->w);
	pOut->m[2][0] = 2.0f * (pIn->x * pIn->z + pIn->y * pIn->w);
	pOut->m[2][1] = 2.0f * (pIn->y *pIn->z - pIn->x *pIn->w);
	pOut->m[2][2] = 1.0f - 2.0f * (pIn->x * pIn->x + pIn->y * pIn->y);
}

void	EngineMatrixIdentity( ENGINEMATRIX* pxMatrix )
{
	memset( pxMatrix, 0, sizeof( ENGINEMATRIX ) );
	pxMatrix->_11 = 1.0f;
	pxMatrix->_22 = 1.0f;
	pxMatrix->_33 = 1.0f;
	pxMatrix->_44 = 1.0f;

}

void	EngineMatrixRotationX( ENGINEMATRIX* pxMatrix, float x )
{
	EngineMatrixIdentity( pxMatrix );
	// TODO
}


void	EngineMatrixRotationY( ENGINEMATRIX* pxMatrix, float y )
{
	EngineMatrixIdentity( pxMatrix );
	// TODO
}

void		EnginePlaneFromPoints( ENGINEPLANE* pxPlaneOut, const VECT* pxVect1, const VECT* pxVect2, const VECT* pxVect3 )
{
	// TODO
}


void	EngineMatrixRotationZ( ENGINEMATRIX* pxMatrix, float z )
{
	EngineMatrixIdentity( pxMatrix );

	pxMatrix->_11 = cosf( z );
	pxMatrix->_22 = pxMatrix->_11;
	pxMatrix->_21 = sinf( z );
	pxMatrix->_12 = 0.0f - pxMatrix->_21;
	pxMatrix->_33 = 1.0f;
}


void		VectRotateAboutZ( VECT* pVec, float fAngle )
{
ENGINEMATRIX	xMatRotZ;

	EngineMatrixRotationZ( &xMatRotZ, fAngle );
    VectTransform( pVec, pVec, &xMatRotZ );
}

BOOL		EngineCollisionRayIntersectTri( const VECT* pTri1, const VECT* pTri2, const VECT* pTri3, const VECT* pRayStart, const VECT* pRayDir, float* pfU, float* pfV, float* pfDist )
{
	// todo
	return( FALSE );
}