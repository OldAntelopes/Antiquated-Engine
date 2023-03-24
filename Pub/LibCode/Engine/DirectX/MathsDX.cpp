
#include "EngineDX.h"

#include <stdio.h>
#include <Engine.h>

#include <StandardDef.h>
#include <Interface.h>

#define		SIN_TABLE_RESOLUTION	65536
#define		SIN_TABLE_SCALE			(65536.0f/A360)

float*	mpfEngineMathsSinTable = NULL;

inline float	sintable( float fAngle )
{
int		nTable;

	nTable = (int)( fAngle * SIN_TABLE_SCALE );
	nTable &= (SIN_TABLE_RESOLUTION-1);
	return( mpfEngineMathsSinTable[ nTable ] );
}



void	EngineMathsFreeSinTable( void )
{
	SystemFree( mpfEngineMathsSinTable );
	mpfEngineMathsSinTable = NULL;
}

void	EngineMathsInitSinTable( void )
{
float	fLoop;
float	fLoopStep;
float*	pfTable;
int		nLoopCount;
	
	// [MEM] 256k.. (but is this even used anymore?)
	mpfEngineMathsSinTable = (float*)SystemMalloc( SIN_TABLE_RESOLUTION * sizeof( float ) );
	pfTable = mpfEngineMathsSinTable;

	fLoop = 0.0f;
	fLoopStep = (A360 / SIN_TABLE_RESOLUTION );
	for ( nLoopCount = 0; nLoopCount < SIN_TABLE_RESOLUTION; nLoopCount++ )
	{
		fLoop = ( (float)nLoopCount * A360 ) / SIN_TABLE_RESOLUTION;
		*(pfTable++) = (float)( sin(fLoop) );
//		fLoop += fLoopStep;
	}

}

//-------------------------------------------------------------------------------------------------
#ifdef TUD11
void	EngineDXMatrixSet( const ENGINEMATRIX* pxMatrix, DirectX::XMMATRIX* pxDXMatrix )
{
	*pxDXMatrix = DirectX::XMMatrixSet( pxMatrix->_11, pxMatrix->_12, pxMatrix->_13, pxMatrix->_14, pxMatrix->_21, pxMatrix->_22, pxMatrix->_23, pxMatrix->_24, pxMatrix->_31, pxMatrix->_32, pxMatrix->_33, pxMatrix->_34, pxMatrix->_41, pxMatrix->_42, pxMatrix->_43, pxMatrix->_44 );
}
void	EngineDXMatrixExtract( const DirectX::XMMATRIX* pxDXMatrix, ENGINEMATRIX* pxMatrix )
{
	DirectX::XMStoreFloat4x4( (DirectX::XMFLOAT4X4*)pxMatrix, *pxDXMatrix );
}
#endif

//----------------------------------------------------------- Matrices

void EngineSetMatrixXYZ( ENGINEMATRIX* pxMatrix, float fX, float fY, float fZ )
{
#ifdef TUD11
	DirectX::XMMATRIX	xMatrix = DirectX::XMMatrixRotationRollPitchYaw( fX, fY, fZ );
	DirectX::XMStoreFloat4x4( (DirectX::XMFLOAT4X4*)pxMatrix, xMatrix );
#else
	D3DXMatrixRotationYawPitchRoll( (D3DXMATRIX*)(pxMatrix), fY, fX, fZ );
#endif
}

void	VectProject( VECT* pxOut, const VECT* pxIn, ENGINEMATRIX* pxProjMat, ENGINEMATRIX* pxViewMat, ENGINEMATRIX* pxWorldMat )
{
	D3DXVec3Project( (D3DXVECTOR3*)pxOut, (D3DXVECTOR3*)pxIn, NULL, (D3DXMATRIX*)pxProjMat, (D3DXMATRIX*)pxViewMat, (D3DXMATRIX*)pxWorldMat );
}


void EngineMatrixLookAt( ENGINEMATRIX* pxMatrix, const VECT* pPos, const VECT* pLookAt, const VECT* pUp )
{
#ifdef TUD11
	DirectX::XMVECTOR	xEyePos = DirectX::XMVectorSet( pPos->x, pPos->y, pPos->z, 0.0f );
	DirectX::XMVECTOR	xLookAt = DirectX::XMVectorSet( pLookAt->x, pLookAt->y, pLookAt->z, 0.0f );
	DirectX::XMVECTOR	xUp = DirectX::XMVectorSet( pUp->x, pUp->y, pUp->z, 0.0f );
	DirectX::XMMATRIX	xMatrix = DirectX::XMMatrixLookAtLH( xEyePos, xLookAt, xUp );
	DirectX::XMStoreFloat4x4( (DirectX::XMFLOAT4X4*)pxMatrix, xMatrix );		
#else
	D3DXMatrixLookAtLH( (D3DXMATRIX*)pxMatrix, (D3DXVECTOR3*)pPos, (D3DXVECTOR3*)pLookAt, (D3DXVECTOR3*)pUp );
#endif
}

void	EngineMatrixIdentity( ENGINEMATRIX* pxMatrix )
{
#ifdef TUD11
DirectX::XMMATRIX xMatrix = DirectX::XMMatrixIdentity();
	EngineDXMatrixExtract( &xMatrix, pxMatrix );
#else
	D3DXMatrixIdentity( (D3DXMATRIX*)(pxMatrix) );
#endif
}


void	EngineMatrixScaling( ENGINEMATRIX* pxMatrix, float x, float y, float z )
{
#ifdef TUD11
DirectX::XMMATRIX		xmMatrix = DirectX::XMMatrixScaling( x, y, z );
	DirectX::XMStoreFloat4x4( (DirectX::XMFLOAT4X4*)pxMatrix, xmMatrix );
#else
	D3DXMatrixScaling( (D3DXMATRIX*)(pxMatrix), x, y, z );
#endif
}

void	EngineMatrixInverse( ENGINEMATRIX* pxMatrix )
{
#ifdef TUD11
DirectX::XMMATRIX		xmMatrix;
	
	EngineDXMatrixSet( pxMatrix, &xmMatrix );
	xmMatrix = DirectX::XMMatrixInverse( NULL, xmMatrix );
	DirectX::XMStoreFloat4x4( (DirectX::XMFLOAT4X4*)pxMatrix, xmMatrix );
#else
	D3DXMatrixInverse( (D3DXMATRIX*)pxMatrix, 0, (D3DXMATRIX*)pxMatrix );
#endif
}

void	EngineMatrixRotationX( ENGINEMATRIX* pxMatrix, float x )
{
#ifdef TUD11
DirectX::XMMATRIX		xmMatrix = DirectX::XMMatrixRotationX( x );
	DirectX::XMStoreFloat4x4( (DirectX::XMFLOAT4X4*)pxMatrix, xmMatrix );
#else
	D3DXMatrixRotationX( (D3DXMATRIX*)(pxMatrix), x );
#endif
}

void	EngineMatrixRotationY( ENGINEMATRIX* pxMatrix, float y )
{
#ifdef TUD11
DirectX::XMMATRIX	xmMatrix = DirectX::XMMatrixRotationY( y );
	EngineDXMatrixExtract( &xmMatrix, pxMatrix );
#else
	D3DXMatrixRotationY( (D3DXMATRIX*)(pxMatrix), y );
#endif
}

void	EngineMatrixRotationZ( ENGINEMATRIX* pxMatrix, float z )
{
#ifdef TUD11
DirectX::XMMATRIX	xmMatrix = DirectX::XMMatrixRotationZ( z );
	EngineDXMatrixExtract( &xmMatrix, pxMatrix );
#else
	D3DXMatrixRotationZ( (D3DXMATRIX*)(pxMatrix), z );
#endif
}

void	EngineMatrixRotationAxis( ENGINEMATRIX* pxMatrix, VECT* pxAxis, float  fAngle )
{
#ifdef TUD11
DirectX::XMVECTOR		xmAxis = DirectX::XMVectorSet( pxAxis->x, pxAxis->y, pxAxis->z, 0.0f );
DirectX::XMMATRIX	xmMatrix;

	xmMatrix = DirectX::XMMatrixRotationAxis( xmAxis, fAngle );
	EngineDXMatrixExtract( &xmMatrix, pxMatrix );
#else
	D3DXMatrixRotationAxis( (D3DXMATRIX*)(pxMatrix), (D3DXVECTOR3*)( pxAxis ), fAngle );
#endif
}

void		EngineMatrixShadow( ENGINEMATRIX* pxOutMatrix, const VECT* pxLight, const ENGINEPLANE* pxPlane )
{
#ifdef TUD11
DirectX::XMVECTOR		xmPlane = DirectX::XMVectorSet( pxPlane->a, pxPlane->b, pxPlane->c, pxPlane->d );
DirectX::XMVECTOR		xmLight = DirectX::XMVectorSet( pxLight->x, pxLight->y, pxLight->z, 0.0f );
DirectX::XMMATRIX	xmMatrix;
	
	xmMatrix = DirectX::XMMatrixShadow( xmPlane, xmLight );
	EngineDXMatrixExtract( &xmMatrix, pxOutMatrix );
#else
D3DXVECTOR4		vec4Light( pxLight->x, pxLight->y, pxLight->z, 0.0f );

	D3DXMatrixShadow( (D3DXMATRIX*)pxOutMatrix, &vec4Light, (D3DXPLANE*)( pxPlane ) );
#endif
}

void		EngineMatrixOrtho( ENGINEMATRIX* pxOutMatrix, float fWidth, float fHeight )
{
#ifdef TUD11
DirectX::XMMATRIX	xmMatrix = DirectX::XMMatrixOrthographicLH( fWidth, fHeight, 0.0f, 1.0f );
	
	EngineDXMatrixExtract( &xmMatrix, pxOutMatrix );
#else
	D3DXMatrixOrthoLH(  (D3DXMATRIX*)pxOutMatrix, fWidth, fHeight, 0.0f, 1.0f);
#endif
}

//----------------------------------------------------------- Vectors

float	VectDist( const VECT* pxVect1, const VECT* pxVect2 )
{
#ifdef TUD11
DirectX::XMVECTOR		xmVectSep = DirectX::XMVectorSet( pxVect2->x - pxVect1->x, pxVect2->y - pxVect1->y, pxVect2->z - pxVect1->z, 0.0f );
DirectX::XMVECTOR		xmResult;

	xmResult = DirectX::XMVector3Length( xmVectSep );
	return( DirectX::XMVectorGetX( xmResult ) );
#else
D3DXVECTOR3	xVec;
	xVec.x = pxVect2->x - pxVect1->x;
	xVec.y = pxVect2->y - pxVect1->y;
	xVec.z = pxVect2->z - pxVect1->z;
	return( D3DXVec3Length( &xVec ) );
#endif
}

float	VectDistNoZ( const VECT* pxVect1, const VECT* pxVect2 )
{
#ifdef TUD11
DirectX::XMVECTOR		xmVectSep = DirectX::XMVectorSet( pxVect2->x - pxVect1->x, pxVect2->y - pxVect1->y, 0.0f, 0.0f );
DirectX::XMVECTOR		xmResult;

	xmResult = DirectX::XMVector3Length( xmVectSep );
	return( DirectX::XMVectorGetX( xmResult ) );
#else
D3DXVECTOR3	xVec;
	xVec.x = pxVect2->x - pxVect1->x;
	xVec.y = pxVect2->y - pxVect1->y;
	xVec.z = 0.0f;
	return( D3DXVec3Length( &xVec ) );
#endif
}

void		VectTransform( VECT* pOut, const VECT* pSrc, const ENGINEMATRIX* pxMatrix )
{
#ifdef TUD11
DirectX::XMMATRIX		xmMatrix;
DirectX::XMVECTOR		xSrc = DirectX::XMVectorSet( pSrc->x, pSrc->y, pSrc->z, 0.0f );
DirectX::XMVECTOR		xResult;

	EngineDXMatrixSet( pxMatrix, &xmMatrix );
	xResult = DirectX::XMVector3TransformCoord( xSrc, xmMatrix );
	DirectX::XMStoreFloat3( (DirectX::XMFLOAT3*)pOut, xResult );
#else
	D3DXVec3TransformCoord( (D3DXVECTOR3*)(pOut), (D3DXVECTOR3*)(pSrc), (D3DXMATRIX*)(pxMatrix) );
#endif
}

void		VectRotateAboutAxis( VECT* pOut, const VECT* pSrc, const VECT* pxAxis, float fAngle )
{
#ifdef TUD11
DirectX::XMMATRIX	xmMatrix;
DirectX::XMVECTOR		xAxis = DirectX::XMVectorSet( pxAxis->x, pxAxis->y, pxAxis->z, 0.0f );
DirectX::XMVECTOR		xSrc = DirectX::XMVectorSet( pSrc->x, pSrc->y, pSrc->z, 0.0f );
DirectX::XMVECTOR		xResult;

	xmMatrix = DirectX::XMMatrixRotationAxis( xAxis, fAngle );
	xResult = DirectX::XMVector3TransformCoord( xSrc, xmMatrix );
	DirectX::XMStoreFloat3( (DirectX::XMFLOAT3*)pOut, xResult );
#else
D3DXMATRIX	matTransform;

	D3DXMatrixRotationAxis( &matTransform, (D3DXVECTOR3*)pxAxis, fAngle );
	D3DXVec3TransformCoord( (D3DXVECTOR3*)(pOut), (D3DXVECTOR3*)(pSrc), &matTransform );
#endif
}


void EngineMatrixMultiply( ENGINEMATRIX* pxMatrix1, const ENGINEMATRIX* pMatrix2 )
{
#ifdef TUD11
DirectX::XMMATRIX	xmMatrix1;
DirectX::XMMATRIX	xmMatrix2;
DirectX::XMMATRIX	xmResult;
	
	EngineDXMatrixSet( pxMatrix1, &xmMatrix1 );
	EngineDXMatrixSet( pMatrix2, &xmMatrix2 );

	xmResult = DirectX::XMMatrixMultiply( xmMatrix1, xmMatrix2 );
	EngineDXMatrixExtract( &xmResult, pxMatrix1 );
#else
	D3DXMatrixMultiply( (D3DXMATRIX*)pxMatrix1, (D3DXMATRIX*)pxMatrix1, (D3DXMATRIX*)pMatrix2 );
#endif
}



float	VectGetLength( const VECT* pVect )
{
#ifdef TUD11
DirectX::XMVECTOR		xVect = DirectX::XMVectorSet( pVect->x, pVect->y, pVect->z, 0.0f ); 
	
	xVect = DirectX::XMVector3Length( xVect );
	return( DirectX::XMVectorGetX( xVect ) );
#else
	return( D3DXVec3Length( (D3DXVECTOR3*)(pVect) ) );
#endif
}

float	VectDot( const VECT* pVect1, const VECT* pVect2 )
{
#ifdef TUD11
DirectX::XMVECTOR		xVect1 = DirectX::XMVectorSet( pVect1->x, pVect1->y, pVect1->z, 0.0f ); 
DirectX::XMVECTOR		xVect2 = DirectX::XMVectorSet( pVect2->x, pVect2->y, pVect2->z, 0.0f ); 
	
	xVect1 = DirectX::XMVector3Dot( xVect1, xVect2 );
	return( DirectX::XMVectorGetX( xVect1 ) );
#else
	return( D3DXVec3Dot( (D3DXVECTOR3*)pVect1,(D3DXVECTOR3*)pVect2 ) );
#endif
}

void		VectSub( VECT* pOut, const VECT* pSrc, const VECT* pSrc2 )
{
#ifdef TUD11
	pOut->x = pSrc->x - pSrc2->x;
	pOut->y = pSrc->y - pSrc2->y;
	pOut->z = pSrc->z - pSrc2->z;
#else
	D3DXVec3Subtract( (D3DXVECTOR3*)(pOut),(D3DXVECTOR3*)(pSrc), (D3DXVECTOR3*)(pSrc2) );
#endif
}

void		VectAdd( VECT* pOut, const VECT* pSrc, const VECT* pSrc2 )
{
#ifdef TUD11
	pOut->x = pSrc->x + pSrc2->x;
	pOut->y = pSrc->y + pSrc2->y;
	pOut->z = pSrc->z + pSrc2->z;
#else
	D3DXVec3Add( (D3DXVECTOR3*)(pOut),(D3DXVECTOR3*)(pSrc), (D3DXVECTOR3*)(pSrc2) );
#endif
}

void	VectHermite( VECT* pOut, const VECT* pPos1, const VECT* pTangent1, const VECT* pPos2, const VECT* pTangent2, float fVal )
{ 
#ifdef TUD11
DirectX::XMVECTOR		xmPos1 = DirectX::XMVectorSet( pPos1->x, pPos1->y, pPos1->z, 0.0f );
DirectX::XMVECTOR		xmTangent1 = DirectX::XMVectorSet( pTangent1->x, pTangent1->y, pTangent1->z, 0.0f );
DirectX::XMVECTOR		xmPos2 = DirectX::XMVectorSet( pPos2->x, pPos2->y, pPos2->z, 0.0f );
DirectX::XMVECTOR		xmTangent2 = DirectX::XMVectorSet( pTangent2->x, pTangent2->y, pTangent2->z, 0.0f );
DirectX::XMVECTOR		xmResult;

	xmResult = DirectX::XMVectorHermite( xmPos1, xmTangent1, xmPos2, xmTangent2, fVal );
	DirectX::XMStoreFloat3( (DirectX::XMFLOAT3*)pOut, xmResult );
#else
	// Hermite interpolation between position V1, tangent T1 (when s == 0) and position V2, tangent T2 (when s == 1).
	D3DXVec3Hermite( (D3DXVECTOR3*)pOut, (D3DXVECTOR3*)pPos1,(D3DXVECTOR3*)pTangent1, (D3DXVECTOR3*)pPos2, (D3DXVECTOR3*)pTangent2, fVal );
#endif
}

void	VectReflect( VECT* pxOut, const VECT* pxInVect, const VECT* pxPlaneNormal )
{
float	fDot = VectDot( pxInVect, pxPlaneNormal );
VECT	xReflect;

	fDot *= 2.0f;
	VectScale( &xReflect, pxPlaneNormal, fDot );
	VectSub( pxOut, pxInVect, &xReflect );
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

void		VectLerp( VECT* pOut, const VECT* pSrc, const VECT* pSrc2, float fTime )
{
#ifdef TUD11
DirectX::XMVECTOR		xSrc1 = DirectX::XMVectorSet( pSrc->x, pSrc->y, pSrc->z, 0.0f ); 
DirectX::XMVECTOR		xSrc2 = DirectX::XMVectorSet( pSrc2->x, pSrc2->y, pSrc2->z, 0.0f ); 
DirectX::XMVECTOR		xResult;

	xResult = DirectX::XMVectorLerp( xSrc1, xSrc2, fTime );
	DirectX::XMStoreFloat3( (DirectX::XMFLOAT3*)pOut, xResult );
#else
	D3DXVec3Lerp( (D3DXVECTOR3*)pOut, (D3DXVECTOR3*)pSrc, (D3DXVECTOR3*)pSrc2, fTime );
#endif
}

float	VectGetRotZAngleBetweenVects( const VECT* pxVect1, const VECT* pxVect2 )
{
VECT	xDiff;
	
	VectSub( &xDiff, pxVect1, pxVect2 );
	return( atan2f( xDiff.x, xDiff.y ) );
}


void	VectScale( VECT* pOut, const VECT* pSrc, float fScale )
{
#ifdef TUD11
DirectX::XMVECTOR		xVect1 = DirectX::XMVectorSet( pSrc->x, pSrc->y, pSrc->z, 0.0f ); 

	xVect1 = DirectX::XMVectorScale( xVect1, fScale );
	DirectX::XMStoreFloat3( (DirectX::XMFLOAT3*)pOut, xVect1 );
#else
	D3DXVec3Scale( (D3DXVECTOR3*)(pOut),(D3DXVECTOR3*)(pSrc), fScale );
#endif
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

void		VectCross( VECT* pOut, const VECT* pIn1, const VECT* pIn2 )
{
#ifdef TUD11
DirectX::XMVECTOR		xVect1 = DirectX::XMVectorSet( pIn1->x, pIn1->y, pIn1->z, 0.0f ); 
DirectX::XMVECTOR		xVect2 = DirectX::XMVectorSet( pIn2->x, pIn2->y, pIn2->z, 0.0f ); 
DirectX::XMVECTOR		xResult;
	
	xResult = DirectX::XMVector3Cross( xVect1, xVect2 );
	DirectX::XMStoreFloat3( (DirectX::XMFLOAT3*)pOut, xResult );
#else
	D3DXVec3Cross( (D3DXVECTOR3*)(pOut),(D3DXVECTOR3*)(pIn1),(D3DXVECTOR3*)(pIn2) );
#endif
}

#define OPTIMISED_AXIS_ROTATIONS

void		VectRotateAboutZ( VECT* pVec, float fAngle )
{
#ifdef TUD11
DirectX::XMMATRIX		xMatrix;
DirectX::XMVECTOR		xVect1 = DirectX::XMVectorSet( pVec->x, pVec->y, pVec->z, 0.0f ); 
DirectX::XMVECTOR		xResult;

	xMatrix = DirectX::XMMatrixRotationZ( fAngle );
	xResult = DirectX::XMVector3TransformCoord( xVect1, xMatrix );
	DirectX::XMStoreFloat3( (DirectX::XMFLOAT3*)pVec, xResult );
#else

#ifdef OPTIMISED_AXIS_ROTATIONS
//	VECT	xTest = *pVec;
	// Should be more optimal to do it this way...
	// x' = x*cos q - y*sin q
	// y' = x*sin q + y*cos q 
	// z' = z
	float	fCosA = sintable( fAngle + A90 );
	float	fSinA = sintable( fAngle );
	float	fXSinA = pVec->x*fSinA;
	pVec->x = (pVec->x*fCosA) - (pVec->y*fSinA);
	pVec->y = fXSinA + (pVec->y*fCosA);

	/*
	D3DXMATRIX	xMatRotZ;
	D3DXMatrixRotationZ( &xMatRotZ, fAngle );
	D3DXVec3TransformCoord( (D3DXVECTOR3*)&xTest, (D3DXVECTOR3*)&xTest, &xMatRotZ );

	if ( ( xTest.x != pVec->x ) || (xTest.y != pVec->y ) )
	{
	int		nBreak = 0;
		nBreak++;
	}
*/
#else
	D3DXMATRIX	xMatRotZ;

	D3DXMatrixRotationZ( &xMatRotZ, fAngle );
	D3DXVec3TransformCoord( (D3DXVECTOR3*)pVec, (D3DXVECTOR3*)pVec, &xMatRotZ );
#endif
#endif
}

void		VectRotateAboutY( VECT* pVec, float fAngle )
{
#ifdef TUD11
DirectX::XMMATRIX		xMatrix;
DirectX::XMVECTOR		xVect1 = DirectX::XMVectorSet( pVec->x, pVec->y, pVec->z, 0.0f ); 
DirectX::XMVECTOR		xResult;

	xMatrix = DirectX::XMMatrixRotationY( fAngle );
	xResult = DirectX::XMVector3TransformCoord( xVect1, xMatrix );
	DirectX::XMStoreFloat3( (DirectX::XMFLOAT3*)pVec, xResult );
#else
D3DXMATRIX	xMatRotY;
	D3DXMatrixRotationY( &xMatRotY, fAngle );
    D3DXVec3TransformCoord( (D3DXVECTOR3*)pVec, (D3DXVECTOR3*)pVec, &xMatRotY );
#endif
}
void		VectRotateAboutX( VECT* pVec, float fAngle )
{
#ifdef TUD11
DirectX::XMMATRIX		xMatrix;
DirectX::XMVECTOR		xVect1 = DirectX::XMVectorSet( pVec->x, pVec->y, pVec->z, 0.0f ); 
DirectX::XMVECTOR		xResult;

	xMatrix = DirectX::XMMatrixRotationX( fAngle );
	xResult = DirectX::XMVector3TransformCoord( xVect1, xMatrix );
	DirectX::XMStoreFloat3( (DirectX::XMFLOAT3*)pVec, xResult );
#else
D3DXMATRIX	xMatRotX;
	D3DXMatrixRotationX( &xMatRotX, fAngle );
    D3DXVec3TransformCoord( (D3DXVECTOR3*)pVec, (D3DXVECTOR3*)pVec, &xMatRotX );
#endif
}


void	EngineQuaternionFromMatrix( ENGINEQUATERNION* pOut, const ENGINEMATRIX* pIn )
{
#ifdef TUD11
DirectX::XMMATRIX		xMatrix;
DirectX::XMVECTOR		xQuat;

	EngineDXMatrixSet( pIn, &xMatrix );
	xQuat = DirectX::XMQuaternionRotationMatrix( xMatrix );
	DirectX::XMStoreFloat4( (DirectX::XMFLOAT4*)pOut, xQuat );
#else
	D3DXQuaternionRotationMatrix( (D3DXQUATERNION*)pOut, (D3DXMATRIX*)pIn );
#endif
}

void	EngineMatrixFromQuaternion( ENGINEMATRIX* pOut, const ENGINEQUATERNION* pIn )
{
#ifdef TUD11
DirectX::XMMATRIX		xMatrix;
DirectX::XMVECTOR		xQuat = DirectX::XMVectorSet( pIn->x, pIn->y, pIn->z, pIn->w );

	xMatrix = DirectX::XMMatrixRotationQuaternion( xQuat );
	EngineDXMatrixExtract( &xMatrix, pOut );
#else
	D3DXMatrixRotationQuaternion( (D3DXMATRIX*)pOut, (D3DXQUATERNION*)pIn );
#endif
}

void	EngineQuaternionSlerp( ENGINEQUATERNION* pOut, ENGINEQUATERNION* pIn1, ENGINEQUATERNION* pIn2, float fBlendAmount)
{
#ifdef TUD11
DirectX::XMVECTOR		xQuat1 = DirectX::XMVectorSet( pIn1->x, pIn1->y, pIn1->z, pIn1->w );
DirectX::XMVECTOR		xQuat2 = DirectX::XMVectorSet( pIn2->x, pIn2->y, pIn2->z, pIn2->w );
DirectX::XMVECTOR		xResult;

	xResult = DirectX::XMQuaternionSlerp( xQuat1, xQuat2, fBlendAmount );
	DirectX::XMStoreFloat4( (DirectX::XMFLOAT4*)pOut, xResult );
#else
	D3DXQuaternionSlerp( (D3DXQUATERNION*)pOut, (D3DXQUATERNION*)pIn1, (D3DXQUATERNION*)pIn2, fBlendAmount );
#endif
}

void	EngineQuaternionNormalize( ENGINEQUATERNION* pOut )
{
#ifdef TUD11
DirectX::XMVECTOR		xQuat = DirectX::XMVectorSet( pOut->x, pOut->y, pOut->z, pOut->w );
DirectX::XMVECTOR		xResult;

	xResult = DirectX::XMQuaternionNormalize( xQuat );
	DirectX::XMStoreFloat4( (DirectX::XMFLOAT4*)pOut, xResult );
#else
	D3DXQuaternionNormalize( (D3DXQUATERNION*)pOut,(D3DXQUATERNION*)pOut );
#endif
}


float	EngineQuaternionDot( ENGINEQUATERNION* pQuat1, ENGINEQUATERNION* pQuat2 )
{
#ifdef TUD11
DirectX::XMVECTOR		xQuat1 = DirectX::XMVectorSet( pQuat1->x, pQuat1->y, pQuat1->z, pQuat1->w );
DirectX::XMVECTOR		xQuat2 = DirectX::XMVectorSet( pQuat2->x, pQuat2->y, pQuat2->z, pQuat2->w );
DirectX::XMVECTOR		xResult;

	xResult = DirectX::XMQuaternionDot( xQuat1, xQuat2 );
	return( DirectX::XMVectorGetX( xResult ) );
#else
	return( D3DXQuaternionDot( (D3DXQUATERNION*)pQuat1,(D3DXQUATERNION*)pQuat2 ) );
#endif
}


void	EnginePlaneFromPointNormal( ENGINEPLANE* pxOut, const VECT* pxPoint, const VECT* pxNormal )
{
#ifdef TUD11
DirectX::XMVECTOR		xPoint = DirectX::XMVectorSet( pxPoint->x, pxPoint->y, pxPoint->z, 0.0f );
DirectX::XMVECTOR		xNormal = DirectX::XMVectorSet( pxNormal->x, pxNormal->y, pxNormal->z, 0.0f );
DirectX::XMVECTOR		xResult;

	xResult = DirectX::XMPlaneFromPointNormal( xPoint, xNormal );
	DirectX::XMStoreFloat4( (DirectX::XMFLOAT4*)pxOut, xResult );
#else
	D3DXPlaneFromPointNormal( (D3DXPLANE*)pxOut, (D3DXVECTOR3*)pxPoint, (D3DXVECTOR3*)pxNormal );
#endif
}
	
/*
BOOL	PlaneIntersectLine( VECT* pPlaneOrigin, VECT* pPlaneNormal, VECT* pRayStart, VECT* pRayEnd, VECT* pCollisionOut )
{
D3DXPLANE		xPlane;
	D3DXPlaneFromPointNormal( &xPlane, (D3DXVECTOR3*)pPlaneOrigin, (D3DXVECTOR3*)pPlaneNormal );
	if ( D3DXPlaneIntersectLine((D3DXVECTOR3*)pCollisionOut, &xPlane, (D3DXVECTOR3*)pRayStart, (D3DXVECTOR3*)pRayEnd ) != NULL )
	{
		return( TRUE );
	}
	return( FALSE );
}
*/

BOOL	EnginePlaneIntersectLine( VECT* pxInteresectPointOut, const ENGINEPLANE* pxPlane, const VECT* pxLineStart, const VECT* pxLineEnd )
{
#ifdef TUD11
DirectX::XMVECTOR		xPlane = DirectX::XMVectorSet( pxPlane->a, pxPlane->b, pxPlane->c, pxPlane->d );
DirectX::XMVECTOR		xLineStart = DirectX::XMVectorSet( pxLineStart->x, pxLineStart->y, pxLineStart->z, 0.0f );
DirectX::XMVECTOR		xLineEnd = DirectX::XMVectorSet( pxLineEnd->x, pxLineEnd->y, pxLineEnd->z, 0.0f );
DirectX::XMVECTOR		xResult;

	xResult = DirectX::XMPlaneIntersectLine( xPlane, xLineStart, xLineEnd );
	if ( DirectX::XMVectorGetX( xResult ) != _FPCLASS_QNAN )
#else
	if ( D3DXPlaneIntersectLine( (D3DXVECTOR3*)pxInteresectPointOut, (D3DXPLANE*)pxPlane, (D3DXVECTOR3*)pxLineStart,  (D3DXVECTOR3*)pxLineEnd ) != NULL )
#endif
	{
	float	fStartDistToIntersection = VectDist( pxLineStart, pxInteresectPointOut );
	float	fEndDistToIntersection = VectDist( pxLineEnd, pxInteresectPointOut );
	float	fLineLength = VectDist( pxLineStart, pxLineEnd );

		if ( ( fStartDistToIntersection > fLineLength ) ||
			 ( fEndDistToIntersection > fLineLength ) )
		{
			return( FALSE );
		}
		return( TRUE );
	}
	return( FALSE );

}

float	EnginePlaneDotCoord( const ENGINEPLANE* pxPlane, const VECT* pxPoint )
{
#ifdef TUD11
DirectX::XMVECTOR		xPoint = DirectX::XMVectorSet( pxPoint->x, pxPoint->y, pxPoint->z, 0.0f );
DirectX::XMVECTOR		xPlane = DirectX::XMVectorSet( pxPlane->a, pxPlane->b, pxPlane->c, pxPlane->d );
DirectX::XMVECTOR		xResult;	

	xResult = DirectX::XMPlaneDotCoord( xPlane, xPoint );
	return( DirectX::XMVectorGetX( xResult ) );
#else
	return( D3DXPlaneDotCoord( (D3DXPLANE*)pxPlane, (D3DXVECTOR3*)pxPoint ) );
#endif
}


void		EnginePlaneFromPoints( ENGINEPLANE* pxPlaneOut, const VECT* pxVect1, const VECT* pxVect2, const VECT* pxVect3 )
{
#ifdef TUD11
DirectX::XMVECTOR		xVect1 = DirectX::XMVectorSet( pxVect1->x, pxVect1->y, pxVect1->z, 0.0f );
DirectX::XMVECTOR		xVect2 = DirectX::XMVectorSet( pxVect2->x, pxVect2->y, pxVect2->z, 0.0f );
DirectX::XMVECTOR		xVect3 = DirectX::XMVectorSet( pxVect3->x, pxVect3->y, pxVect3->z, 0.0f );
DirectX::XMVECTOR		xResult;

	xResult = DirectX::XMPlaneFromPoints( xVect1, xVect2, xVect3 );
	DirectX::XMStoreFloat4( (DirectX::XMFLOAT4*)pxPlaneOut, xResult );
#else
	D3DXPlaneFromPoints( (D3DXPLANE*)pxPlaneOut, (D3DXVECTOR3*)pxVect1,(D3DXVECTOR3*)pxVect2, (D3DXVECTOR3*)pxVect3 );
#endif
}
