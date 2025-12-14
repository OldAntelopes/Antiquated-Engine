
#include "InterfaceInternalsDX.h"

#include <math.h>
#include <stdio.h>

#include <StandardDef.h>
#include <Interface.h>

#include "../Common/InterfaceUtil.h"
#include "Maths.h"

#ifdef TUD9
#define Z_BUFFER_READS
#endif

/***************************************************************************
 * Function    : GetDifferenceBetweenAngles
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
FLOAT GetDifferenceBetweenAngles( FLOAT fAngle1, FLOAT fAngle2 )
{
FLOAT	fDiff;

	fDiff = fAngle1 - fAngle2;

	if ( fDiff < 0 )
	{
		fDiff += (FLOAT)(2*PI);
	}

	if ( fDiff > PI )
	{
		fDiff = (FLOAT)( (2*PI) - fDiff );
	}

	return( fDiff );
} 


#ifndef SERVER

#ifdef TUD11

#else
IDirect3DSurface9*	mpZCopySurface = NULL;
#endif
float mfTargetViewOffset = 0.0f;


float	GetViewRotationFromVector( const MVECT* pxVect )
{
	return( atan2f( pxVect->x, pxVect->y ) );
}



#ifndef Z_BUFFER_READS

ushort	ReadZAtScreenCoord( int nX, int nY )
{
	return( 0 );
}

ushort	GetZDepthFromWorldCoord( MVECT* pxWorldCoord )
{
	return( 0 );
}

#else
/***************************************************************************
 * Function    : ReadZAtScreenCoord
 * Params      :
 * Returns     :
 * Description : Only included if Z_BUFFER_READS defined
 ***************************************************************************/
ushort	ReadZAtScreenCoord( int nX, int nY )
{
IGRAPHICSSURFACE*	pZBuffer;
RECT				xLockRect;
D3DLOCKED_RECT		xSurfaceRect;
ushort				uwDepth = 0xFFFF;

	if ( InterfaceIsZBufferLockable() )
	{
		if ( nX < 0 ) nX = 0;
		if ( nY < 0 ) nY = 0;
		if ( nX > InterfaceGetWidth()-1) nX = InterfaceGetWidth()-1;
		if ( nY > InterfaceGetHeight()-1) nY = InterfaceGetHeight()-1;
		xLockRect.left = nX;
		xLockRect.right = nX + 1;
		xLockRect.top = nY;
		xLockRect.bottom = nY + 1;
	
		mpLegacyInterfaceD3DDeviceSingleton->GetDepthStencilSurface( &pZBuffer );
	
		if ( pZBuffer->LockRect( &xSurfaceRect, &xLockRect, D3DLOCK_READONLY ) == D3D_OK )
		{
			uwDepth = *( (ushort*)( xSurfaceRect.pBits ) );
	
			pZBuffer->UnlockRect();
		}
		else
		{
			//PANIC_IF( TRUE, "Z Buffer lock failed" );
			InterfaceSetZBufferLockable( FALSE );
		}
	
		pZBuffer->Release();
	}

	return( uwDepth );
}

/***************************************************************************
 * Function    : GetZDepthFromWorldCoord
 * Params      :
 * Returns     :
 * Description : Only included if Z_BUFFER_READS defined
 ***************************************************************************/
ushort	GetZDepthFromWorldCoord( MVECT* pxWorldCoord )
{
D3DXMATRIX matProj;
D3DXMATRIX matView;
D3DXMATRIX matWorld;
D3DXVECTOR3	xVect;

	xVect.x = 0.0f;
	xVect.y = 0.0f;
	xVect.z = 0.0f;

	mpLegacyInterfaceD3DDeviceSingleton->GetTransform( D3DTS_PROJECTION, &matProj );
	mpLegacyInterfaceD3DDeviceSingleton->GetTransform( D3DTS_VIEW, &matView );
	mpLegacyInterfaceD3DDeviceSingleton->GetTransform( D3DTS_WORLD, &matWorld );
	
	D3DXVec3Project( &xVect, (D3DXVECTOR3*)( pxWorldCoord ), NULL, &matProj, &matView, &matWorld );

//	xVect.z -= gfNearClipPlane;

	if ( xVect.z < 0 )
	{
		return( 0xFFFF );
	}

	xVect.z *= 65535;
//	xVect.z /= (gfFarClipPlane-gfNearClipPlane);

	return( (ushort)( xVect.z ) );
}
#endif



#endif