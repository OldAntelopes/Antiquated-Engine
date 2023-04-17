
#include "EngineDX.h"

#include <Engine.h>
#include <stdio.h>

#ifndef STANDALONE
#ifdef DEBUG
#define DEBUG_COLLISION_TRIS
#endif
#endif

#include <StandardDef.h>
#include <Engine.h>
#include <Rendering.h>
#include <Interface.h>
#include "System.h"

#include "../ModelFiles.h"
#include "../CollisionMaths/collmathsdefines.h"
#include "../CollisionMaths.h"
#include "CollisionDX.h"
#include "../ModelRendering.h"

#ifdef DEBUG_COLLISION_TRIS
#include "../../Universal/GameCode/Util/Line Renderer.h"
#endif



BOOL		EngineCollisionRayPlane( VECT* pxPlanePos1, VECT* pxPlanePos2, VECT* pxPlanePos3, VECT* pxRayStart, VECT* pxRayDir  )
{
VECT	xRayEnd;

	VectScale( &xRayEnd, pxRayDir, 10000.0f );
	VectAdd( &xRayEnd, &xRayEnd, pxRayStart );

#ifdef TUD11
	DirectX::XMVECTOR	xmVect1 = DirectX::XMVectorSet( pxPlanePos1->x, pxPlanePos1->y,pxPlanePos1->z, 0.0f );
	DirectX::XMVECTOR	xmVect2 = DirectX::XMVectorSet( pxPlanePos2->x, pxPlanePos2->y,pxPlanePos2->z, 0.0f );
	DirectX::XMVECTOR	xmVect3 = DirectX::XMVectorSet( pxPlanePos3->x, pxPlanePos3->y,pxPlanePos3->z, 0.0f );
	DirectX::XMVECTOR	xmPlane = DirectX::XMPlaneFromPoints( xmVect1, xmVect2, xmVect3 );
	DirectX::XMVECTOR	xmRayStart = DirectX::XMVectorSet( pxRayStart->x, pxRayStart->y, pxRayStart->z, 0.0f );
	DirectX::XMVECTOR	xmRayEnd = DirectX::XMVectorSet( xRayEnd.x, xRayEnd.y, xRayEnd.z, 0.0f );
	DirectX::XMVECTOR	xmResult;

	xmResult = DirectX::XMPlaneIntersectLine( xmPlane, xmRayStart, xmRayEnd );
	if ( DirectX::XMVectorGetX( xmResult ) != _FPCLASS_QNAN )
	{
		return( TRUE );
	}
	return( FALSE );
#else
	D3DXPLANE	xPlane;

	D3DXPlaneFromPoints( &xPlane, (D3DXVECTOR3*)pxPlanePos1, (D3DXVECTOR3*)pxPlanePos2,(D3DXVECTOR3*)pxPlanePos3 );
	if ( D3DXPlaneIntersectLine( (D3DXVECTOR3*)&mxCollResultPos, &xPlane, (D3DXVECTOR3*)pxRayStart, (D3DXVECTOR3*)&xRayEnd ) != NULL )
	{
		// TODO - use the hitpoint
		return( TRUE );
	}
#endif
	return( FALSE );
}


BOOL	EngineCollisionBoxSphere( VECT* pxSpherePos, float fSphereRadius, VECT* pxBoundMin, VECT* pxBoundMax )
{
#ifdef TUD11
DirectX::BoundingSphere		xBoundingSphere;
DirectX::BoundingBox		xBoundBox;

	xBoundingSphere.Radius = fSphereRadius;
	xBoundingSphere.Center.x = pxSpherePos->x; 
	xBoundingSphere.Center.y = pxSpherePos->y; 
	xBoundingSphere.Center.z = pxSpherePos->z; 

	DirectX::XMVECTOR		xmBoundMin = DirectX::XMVectorSet( pxBoundMin->x, pxBoundMin->y, pxBoundMin->z, 0.0f );
	DirectX::XMVECTOR		xmBoundMax = DirectX::XMVectorSet( pxBoundMax->x, pxBoundMax->y, pxBoundMax->z, 0.0f );
	DirectX::BoundingBox::CreateFromPoints( xBoundBox, xmBoundMin, xmBoundMax );

	if ( xBoundingSphere.Intersects( xBoundBox ) )
	{
		return( TRUE );
	}
	return( FALSE );
#else
	return( D3DXSphereBoundProbe( (D3DXVECTOR3*)pxSpherePos, fSphereRadius, (D3DXVECTOR3*)pxBoundMin, (D3DXVECTOR3*)pxBoundMax ) );
#endif
}


BOOL		EngineCollisionBoxBoundProbe( const VECT* pxBoxMin, const VECT* pxBoxMax, const VECT* pxRayStart, const VECT* pxRayDir )
{
#ifdef TUD11
DirectX::BoundingBox		xBoundBox;
float	fHitDist;

	DirectX::XMVECTOR		xmBoundMin = DirectX::XMVectorSet( pxBoxMin->x, pxBoxMin->y, pxBoxMin->z, 0.0f );
	DirectX::XMVECTOR		xmBoundMax = DirectX::XMVectorSet( pxBoxMax->x, pxBoxMax->y, pxBoxMax->z, 0.0f );
	DirectX::BoundingBox::CreateFromPoints( xBoundBox, xmBoundMin, xmBoundMax );

	DirectX::XMVECTOR		xmRayStart = DirectX::XMVectorSet( pxRayStart->x, pxRayStart->y, pxRayStart->z, 0.0f );
	DirectX::XMVECTOR		xmRayDir = DirectX::XMVectorSet( pxRayDir->x, pxRayDir->y, pxRayDir->z, 0.0f );
	if ( xBoundBox.Intersects( xmRayStart, xmRayDir, fHitDist ) )
	{
		return(	TRUE );
	}
	return( FALSE );
#else
	return( D3DXBoxBoundProbe( (const D3DXVECTOR3*)pxBoxMin, (const D3DXVECTOR3*)pxBoxMax, (const D3DXVECTOR3*)pxRayStart, (const D3DXVECTOR3*)pxRayDir ) );
#endif
}

BOOL		EngineCollisionRayBoundBox( const VECT* pxBoxMin, const VECT* pxBoxMax, const VECT* pxRayStart, const VECT* pxRayDir )
{
	if ( EngineCollisionBoxBoundProbe( pxBoxMin, pxBoxMax, pxRayStart, pxRayDir ) == TRUE )
	{
	VECT	xRayEnd;
	VECT		xReverseRayDir;

		VectAdd( &xRayEnd, pxRayStart, pxRayDir );
		VectScale( &xReverseRayDir, pxRayDir, -1.0f );
		// Check from the other direction too (coz the box bound probe effectively uses an infinite length ray)
		if ( EngineCollisionBoxBoundProbe( pxBoxMin, pxBoxMax, &xRayEnd, &xReverseRayDir ) == TRUE )
		{
			return( TRUE );
		}
	}
	return( FALSE );
}


BOOL		EngineCollisionRayIntersectTri( const VECT* pTri1, const VECT* pTri2, const VECT* pTri3, const VECT* pRayStart, const VECT* pRayDir, float* pfU, float* pfV, float* pfDist )
{
#ifdef TUD11
float	fHitDist;

	DirectX::XMVECTOR		xmRayStart = DirectX::XMVectorSet( pRayStart->x, pRayStart->y, pRayStart->z, 0.0f );
	DirectX::XMVECTOR		xmRayDir = DirectX::XMVectorSet( pRayDir->x, pRayDir->y, pRayDir->z, 0.0f );
	DirectX::XMVECTOR		xmTri1 = DirectX::XMVectorSet( pTri1->x, pTri1->y, pTri1->z, 0.0f );
	DirectX::XMVECTOR		xmTri2 = DirectX::XMVectorSet( pTri2->x, pTri2->y, pTri2->z, 0.0f );
	DirectX::XMVECTOR		xmTri3 = DirectX::XMVectorSet( pTri3->x, pTri3->y, pTri3->z, 0.0f );
	if ( DirectX::TriangleTests::Intersects( xmRayStart, xmRayDir, xmTri1, xmTri2, xmTri3, fHitDist ) )
	{
		return( TRUE );
	}
	return( FALSE );
#else
	return( D3DXIntersectTri( (const D3DXVECTOR3*)pTri1, (const D3DXVECTOR3*)pTri2, (const D3DXVECTOR3*)pTri3, (const D3DXVECTOR3*)pRayStart, (const D3DXVECTOR3*)pRayDir, pfU, pfV, pfDist ) );
#endif
}


BOOL		ModelRayTest( int nModelHandle, const VECT* pxPos, const VECT* pxRot, const VECT* pxRayStart, const VECT* pxRayEnd, VECT* pxHit, VECT* pxHitNormal, int* pnFaceNum, int flags )
{
ENGINEMATRIX		matRotation;
ENGINEMATRIX		matInvRotation;
VECT		xRay;
VECT		xRayVect;
VECT		xBoundsMin;
VECT		xBoundsMax;
VECT		xPoint;
float	fMinCollisionDist;

	if ( nModelHandle == NOTFOUND ) return( FALSE );

	if ( maxModelRenderData[ nModelHandle ].xCollisionAttachData.nModelHandle != NOTFOUND )
	{
		nModelHandle = maxModelRenderData[ nModelHandle ].xCollisionAttachData.nModelHandle;
	}

	// TODO TODOOO!
	xRayVect.x = pxRayEnd->x - pxRayStart->x;
	xRayVect.y = pxRayEnd->y - pxRayStart->y;
	xRayVect.z = pxRayEnd->z - pxRayStart->z;
	fMinCollisionDist = VectGetLength( &xRayVect );
	EngineSetMatrixFromRotations( pxRot, &matRotation );
	matInvRotation = matRotation;
	EngineMatrixInverse( &matInvRotation );
	xRay = xRayVect;
	VectNormalize( &xRay );

	VectTransform( &xRay, &xRay, &matInvRotation );
	
	xBoundsMin = ModelGetStats(nModelHandle)->xBoundMin;
	xBoundsMax = ModelGetStats(nModelHandle)->xBoundMax;

	xPoint.x = pxRayStart->x - pxPos->x;
	xPoint.y = pxRayStart->y - pxPos->y;
	xPoint.z = pxRayStart->z - pxPos->z;
	VectTransform( &xPoint, &xPoint, &matInvRotation );

	// If the ray intersects with the bounding box of the model
	if ( EngineCollisionBoxBoundProbe( &xBoundsMin, &xBoundsMax, &xPoint, &xRay ) == TRUE )
	{
		if ( flags == 1 )
		{
			return( TRUE );
		}
		else
		{
		BOOL	boCollision = FALSE;
		float	fCollisionDist = 10000.0f;
		EngineMesh*		pxBaseMesh;
		ulong 	ulCollIndex;

			pxBaseMesh = maxModelRenderData[ nModelHandle ].pxBaseMesh;
			pxBaseMesh->RayTest( (VECT*)&xPoint, (VECT*)&xRay, &boCollision, &ulCollIndex, NULL, NULL, &fCollisionDist, NULL, NULL );
			if ( boCollision )
			{
	//			AddFontString( 300,220,"Is ray coll long", COL_WHITE );
				// If the collision occured
				if ( fCollisionDist < fMinCollisionDist )
				{
					if ( pnFaceNum )
					{
						*pnFaceNum = (int)(ulCollIndex);
					}
					fMinCollisionDist = fCollisionDist;
					VectTransform( &xRay, &xRay, &matRotation );
					// Return the building num and details
					VectScale( &xRay,&xRay, fMinCollisionDist );
					if ( pxHit )
					{
						pxHit->x = pxRayStart->x + xRay.x;
						pxHit->y = pxRayStart->y + xRay.y;
						pxHit->z = pxRayStart->z + xRay.z;
					}

					if ( pxHitNormal )
					{
					CUSTOMVERTEX*		pxVertices;
					ushort*		puwIndices;
					VECT		xFaceNormal;
//					VECT		xNormal1, xNormal2, xNormal3;
					CUSTOMVERTEX*		pxVert1;
					CUSTOMVERTEX*	pxVert2;
					CUSTOMVERTEX*	pxVert3;
					VECT			xFaceEdge1;
					VECT			xFaceEdge2;

						pxBaseMesh->LockVertexBuffer(kLock_ReadOnly, (byte**)( &pxVertices ) );
						pxBaseMesh->LockIndexBuffer(kLock_ReadOnly, (byte**)( &puwIndices ) );
					
						pxVert1 = pxVertices + puwIndices[ (ulCollIndex*3) ];
						pxVert2 = pxVertices + puwIndices[ (ulCollIndex*3) + 1 ];
						pxVert3 = pxVertices + puwIndices[ (ulCollIndex*3) + 2 ];

						VectSub( &xFaceEdge1, &pxVert1->position, &pxVert2->position );
						VectSub( &xFaceEdge2, &pxVert2->position, &pxVert3->position );
						VectCross( &xFaceNormal, &xFaceEdge1, &xFaceEdge2 );
						VectNormalize( &xFaceNormal );
//						ulCollIndex = (puwIndices[ ulCollIndex*3 ]);
						*pxHitNormal = xFaceNormal;
						pxBaseMesh->UnlockVertexBuffer();
						pxBaseMesh->UnlockIndexBuffer();
						VectTransform( pxHitNormal, pxHitNormal, &matRotation );
					}
					return( TRUE );
				}
			}
		}
	}
	return( FALSE );
}


