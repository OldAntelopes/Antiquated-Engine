
#include "EngineDX.h"

#include <StandardDef.h>
#include <Rendering.h>
#include <Interface.h>
#include <Engine.h>

#include "../EngineCamera.h"
#include "../ModelMaterialData.h"
#include "EngineCameraDX.h"
#include "ShadowMapDX.h"
#include "ShaderLoaderDX.h"
#include "../CollisionMaths/collmathsdefines.h"
#include "../CollisionMaths/Plane.h"

float	mfEngineCameraLastSetFOV = A45;
float	mfEngineCameraLastSetNearClip = 0.1f;
float	mfEngineCameraLastSetFarClip = 50.0f;
float		mfAspectOverride = 0.0f;

//-------------------------------------------------------------------------------------------------
// what we really need is a nice generic 'IsBoundingBoxInView' function that operates correctly whateva
// camera mode setup we're in...
// This is (no doubt) a crappy one that won't work all the time..
class Frustum
{
public:
	static Frustum&	Get();

	void			RecalcPlanes( void );	
	CULL_RESULT		ContainsSphere(const VECT* pxSpherePos, float fRadius ) const;
private:
	Plane	m_aFrustumPlanes[6];
};

Frustum&	Frustum::Get()
{
static	Frustum		m_singleton;
	return( m_singleton );
}

CULL_RESULT Frustum::ContainsSphere(const VECT* pxSpherePos, float fRadius ) const
{
float fDistance;	// calculate our distances to each of the planes
int		i;
	for( i = 0; i < 6; ++i) 
	{		// find the distance to this plane
		fDistance = (m_aFrustumPlanes[i].a * pxSpherePos->x) +
					(m_aFrustumPlanes[i].b * pxSpherePos->y) +
					(m_aFrustumPlanes[i].c * pxSpherePos->z) +
					(m_aFrustumPlanes[i].d);

		if(fDistance < -fRadius)
		{
			return(OUTSIDE);
		}
		// else if the distance is between +- radius, then we intersect
		else if((float)fabs(fDistance) < fRadius)
		{
			return(INTERSECTING);
		}
	}	// otherwise we are fully in view
	return(INSIDE);
}
 
void Frustum::RecalcPlanes( void )
{
#ifdef TUD11
	PANIC_IF( TRUE, "DX11 Frustum::RecalcPlanes TBI" );
#else
D3DXMATRIX		matProj;
D3DXMATRIX		matView;
D3DXMATRIXA16 mat;
D3DXVECTOR3		vecFrustum[8];
D3DXPLANE planeFrustum[6];
int		i;
//float	fMag;

	mpEngineDevice->GetTransform( D3DTS_PROJECTION, &matProj );
	mpEngineDevice->GetTransform( D3DTS_VIEW, &matView );
    D3DXMatrixMultiply( &mat, &matView, &matProj );
    D3DXMatrixInverse( &mat, NULL, &mat );

    vecFrustum[0] = D3DXVECTOR3(-1.0f, -1.0f,  0.0f); // xyz
    vecFrustum[1] = D3DXVECTOR3( 1.0f, -1.0f,  0.0f); // Xyz
    vecFrustum[2] = D3DXVECTOR3(-1.0f,  1.0f,  0.0f); // xYz
    vecFrustum[3] = D3DXVECTOR3( 1.0f,  1.0f,  0.0f); // XYz
    vecFrustum[4] = D3DXVECTOR3(-1.0f, -1.0f,  1.0f); // xyZ
    vecFrustum[5] = D3DXVECTOR3( 1.0f, -1.0f,  1.0f); // XyZ
    vecFrustum[6] = D3DXVECTOR3(-1.0f,  1.0f,  1.0f); // xYZ
    vecFrustum[7] = D3DXVECTOR3( 1.0f,  1.0f,  1.0f); // XYZ

    for( i = 0; i < 8; i++ )
        D3DXVec3TransformCoord( &vecFrustum[i], &vecFrustum[i], &mat );

    D3DXPlaneFromPoints( &planeFrustum[0], &vecFrustum[0], 
        &vecFrustum[1], &vecFrustum[2] ); // Near
    D3DXPlaneFromPoints( &planeFrustum[1], &vecFrustum[6], 
        &vecFrustum[7], &vecFrustum[5] ); // Far
    D3DXPlaneFromPoints( &planeFrustum[2], &vecFrustum[2], 
        &vecFrustum[6], &vecFrustum[4] ); // Left
    D3DXPlaneFromPoints( &planeFrustum[3], &vecFrustum[7], 
        &vecFrustum[3], &vecFrustum[5] ); // Right
    D3DXPlaneFromPoints( &planeFrustum[4], &vecFrustum[2], 
        &vecFrustum[3], &vecFrustum[6] ); // Top
    D3DXPlaneFromPoints( &planeFrustum[5], &vecFrustum[1], 
        &vecFrustum[0], &vecFrustum[4] ); // Bottom

    for( i = 0; i < 6; i++ )
	{
		m_aFrustumPlanes[i].a = planeFrustum[i].a;
		m_aFrustumPlanes[i].b = planeFrustum[i].b;
		m_aFrustumPlanes[i].c = planeFrustum[i].c;
		m_aFrustumPlanes[i].d = planeFrustum[i].d;
/*		fMag = (float)( sqrt( (m_aFrustumPlanes[i].a*m_aFrustumPlanes[i].a)+
								(m_aFrustumPlanes[i].b*m_aFrustumPlanes[i].b) +
								(m_aFrustumPlanes[i].c*m_aFrustumPlanes[i].c) +
								(m_aFrustumPlanes[i].d*m_aFrustumPlanes[i].d) ) );
		fMag = 1.0f / fMag;
		m_aFrustumPlanes[i].a *= fMag;
		m_aFrustumPlanes[i].b *= fMag;
		m_aFrustumPlanes[i].c *= fMag;
		m_aFrustumPlanes[i].d *= fMag;
*/	}
#endif
}

CULL_RESULT	EngineIsBoundingSphereInView( const VECT* pxOrigin, float fRadius )
{
	return( Frustum::Get().ContainsSphere( pxOrigin, fRadius ) );
}

CULL_RESULT	EngineIsBoundingBoxInView( VECT* pxMin, VECT* pxMax )
{
//	return( Frustum::Get().ContainsSphere( pxOrigin, fRadius ) );
	// todo
	return( OUTSIDE );
}

VECT	mxEyeOffset;

BOOL	mbStereoRightEye = FALSE;

void	EngineCameraSetEyeOffset( BOOL bRight )
{
	mbStereoRightEye = bRight;
}



/***************************************************************************
 * Function    : EngineCameraUpdate
 * Params      :
 * Description : 
 ***************************************************************************/
void EngineCameraUpdate( void )
{
ENGINEMATRIX matWorld;

	EngineMatrixIdentity( &matWorld );

	if ( EngineIsVRMode() )
	{
	VECT	xRight;
		VectCross( &xRight, &mxEngineCamUpVect, &mxEngineCamVect );
		VectNormalize( &xRight );
		VectScale( &xRight, &xRight, 0.001f );

		if ( mbStereoRightEye )
		{
		VECT	xEyePos;
		VECT	xTarget;

			VectScale( &xRight, &xRight, -1.0f );
			VectAdd( &xEyePos, &mxEngineCamPos, &xRight );
			VectAdd( &xTarget, &xEyePos, &mxEngineCamVect );
			EngineMatrixLookAt( &mEngineViewMatrix, &xEyePos, &xTarget, &mxEngineCamUpVect );
		}
		else
		{
		VECT	xEyePos;
		VECT	xTarget;

			VectAdd( &xEyePos, &mxEngineCamPos, &xRight );
			VectAdd( &xTarget, &xEyePos, &mxEngineCamVect );
			EngineMatrixLookAt( &mEngineViewMatrix, &xEyePos, &xTarget, &mxEngineCamUpVect );
		}
	}
	else
	{
	VECT	xTarget;

		VectAdd( &xTarget, &mxEngineCamPos, &mxEngineCamVect );
		EngineMatrixLookAt( &mEngineViewMatrix, &mxEngineCamPos, &xTarget, &mxEngineCamUpVect );
	}
     
	if ( mpEngineDevice != NULL )
	{
		EngineSetWorldMatrix( NULL );
		EngineSetViewMatrix( &mEngineViewMatrix );
	}
	// Recalc the view frustum planes
	Frustum::Get().RecalcPlanes();
}



void		EngineCameraSetViewAspectOverride( float fAspect )
{
	mfAspectOverride = fAspect;
}

void		EngineCameraSetProjectionOrtho( void )
{
D3DXMATRIX	Ortho2D;	

	D3DXMatrixOrthoLH(&Ortho2D, (float)InterfaceGetWidth(), (float)InterfaceGetHeight(), 0.0f, 1000.0f);
	EngineSetProjectionMatrix( &Ortho2D );
}


void	EngineCameraSetOthorgonalView( int width, int height )
{
ENGINEMATRIX Ortho2D;	
ENGINEMATRIX xMatrix; 
	
	// Set View to identity
	EngineMatrixIdentity(&xMatrix);
	EngineSetViewMatrix( &xMatrix);

	// Set world transform to invert Y and center in middle of screen
	xMatrix._22 = -1.0f;
	xMatrix._41 = (float)( -(width/2) );
	xMatrix._42 = (float)( +(height/2) );
	EngineSetWorldMatrix( &xMatrix );

	EngineMatrixOrtho( &Ortho2D, (float)width, (float)height );
	EngineSetProjectionMatrix( &Ortho2D );
}


void		EngineCameraGetCurrentProjectionSettings( float* pfFOV, float* pfNearClip, float* pfFarClip )
{
	*pfFOV = mfEngineCameraLastSetFOV;
	*pfNearClip = mfEngineCameraLastSetNearClip;
	*pfFarClip = mfEngineCameraLastSetFarClip;
}

void		EngineCameraSetProjection( float fFOV, float fNearClip, float fFarClip )
{
D3DXMATRIX matProj;
float	fAspect = (float)(InterfaceGetWidth()) / (float)(InterfaceGetHeight());

	if ( mfAspectOverride != 0.0f )
	{
		fAspect = mfAspectOverride;
	}
	D3DXMatrixPerspectiveFovLH( &matProj, fFOV, fAspect,fNearClip, fFarClip );
	EngineSetProjectionMatrix( &matProj );
	mfEngineCameraLastSetFOV = fFOV;
	mfEngineCameraLastSetNearClip = fNearClip;
	mfEngineCameraLastSetFarClip = fFarClip;
}

void		EngineCameraInitDX( void )
{
	if ( mpEngineDevice )
	{
	float fNearClipPlane = 0.01f;
	float fFarClipPlane = 1000.0f;
	float fFOV = A30;

    D3DXMATRIX matProj;
	float	fAspect = (float)(InterfaceGetWidth()) / (float)(InterfaceGetHeight());
		D3DXMatrixPerspectiveFovLH( &matProj, fFOV, fAspect, fNearClipPlane, fFarClipPlane );
		EngineSetProjectionMatrix( &matProj );

		// Recalc the view frustum planes
		Frustum::Get().RecalcPlanes();
	}
}

void		EngineGetScreenCoordForWorldCoordWithWorldMatrix( const VECT* pPos, int* pX, int* pY )
{
D3DXMATRIX matProj;
D3DXMATRIX matView;
D3DXMATRIX matWorld;
D3DXVECTOR3	xVect;
int		nMidX;
int		nMidY;

	xVect.x = 0.0f;
	xVect.y = 0.0f;
	xVect.z = 0.0f;

	mpEngineDevice->GetTransform( D3DTS_PROJECTION, &matProj );
	mpEngineDevice->GetTransform( D3DTS_VIEW, &matView );
	mpEngineDevice->GetTransform( D3DTS_WORLD, &matWorld );
	
	D3DXVec3Project( &xVect, (D3DXVECTOR3*)( pPos ), NULL, &matProj, &matView, &matWorld );

	/** Doing my own viewport calculations.. dont know why.. **/
	nMidX = InterfaceGetWidth() / 2;
	nMidY = InterfaceGetHeight() / 2;

	*(pX) = (int)( nMidX * xVect.x ) + nMidX;
	*(pY) = (int)( nMidY * -xVect.y ) + nMidY;
}


void		EngineGetScreenCoordForWorldCoord( const VECT* pPos, int* pX, int* pY )
{
D3DXMATRIX matProj;
D3DXMATRIX matView;
D3DXMATRIX matWorld;
D3DXVECTOR3	xVect;
int		nMidX;
int		nMidY;

	xVect.x = 0.0f;
	xVect.y = 0.0f;
	xVect.z = 0.0f;

	mpEngineDevice->GetTransform( D3DTS_PROJECTION, &matProj );
	mpEngineDevice->GetTransform( D3DTS_VIEW, &matView );
	D3DXMatrixIdentity( &matWorld );
	
	D3DXVec3Project( &xVect, (D3DXVECTOR3*)( pPos ), NULL, &matProj, &matView, &matWorld );

	/** Doing my own viewport calculations.. dont know why.. **/
	nMidX = InterfaceGetWidth() / 2;
	nMidY = InterfaceGetHeight() / 2;

	*(pX) = (int)( nMidX * xVect.x ) + nMidX;
	*(pY) = (int)( nMidY * -xVect.y ) + nMidY;
}
