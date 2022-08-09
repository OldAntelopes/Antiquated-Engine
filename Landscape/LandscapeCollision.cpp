
#include <float.h>
#include <math.h>
#include "StandardDef.h"
#include "Engine.h"

#include "LandscapeCoords.h"
#include "LandscapeCollision.h"


void	LandscapeTileGetTri1( int nMapX, int nMapY, VECT* pxVert1, VECT* pxVert2, VECT* pxVert3 )
{
	*pxVert1 = LandscapeGetWorldPos( nMapX, nMapY );
	*pxVert2 = LandscapeGetWorldPos( nMapX + 1, nMapY );
	*pxVert3 = LandscapeGetWorldPos( nMapX, nMapY + 1 );
}

void	LandscapeTileGetTri2( int nMapX, int nMapY, VECT* pxVert1, VECT* pxVert2, VECT* pxVert3 )
{
	*pxVert1 = LandscapeGetWorldPos( nMapX + 1, nMapY );
	*pxVert2 = LandscapeGetWorldPos( nMapX + 1, nMapY + 1 );
	*pxVert3 = LandscapeGetWorldPos( nMapX, nMapY + 1 );
}


void	LandscapeTileGetBoundingSphere( int nMapX, int nMapY, VECT* pxBoundSpherePos, float* pfBoundSphereRadius )
{
VECT	xPos1 = LandscapeGetWorldPos( nMapX, nMapY );
VECT	xPos2 = LandscapeGetWorldPos( nMapX + 1, nMapY );
VECT	xPos3 = LandscapeGetWorldPos( nMapX, nMapY + 1 );
VECT	xPos4 = LandscapeGetWorldPos( nMapX + 1, nMapY + 1 );
VECT	xMidPoint;
float	fRadius;
float	fTemp;

	xMidPoint.x = (xPos1.x + xPos2.x + xPos3.x + xPos4.x) * 0.25f;
	xMidPoint.y = (xPos1.y + xPos2.y + xPos3.y + xPos4.y) * 0.25f;
	xMidPoint.z = (xPos1.z + xPos2.z + xPos3.z + xPos4.z) * 0.25f;

	fRadius = VectDist( &xPos1, &xMidPoint );
	fTemp = VectDist( &xPos2, &xMidPoint );
	if ( fTemp > fRadius ) fRadius = fTemp;
	fTemp = VectDist( &xPos3, &xMidPoint );
	if ( fTemp > fRadius ) fRadius = fTemp;
	fTemp = VectDist( &xPos4, &xMidPoint );
	if ( fTemp > fRadius ) fRadius = fTemp;
	
	*pxBoundSpherePos = xMidPoint;
	*pfBoundSphereRadius = fRadius;	
}

BOOL	LandscapeRayTest( const VECT* pxRayStart, const VECT* pxRayDir, VECT* pxIntersectPoint, BOOL bClosest )
{
ENGINEPLANE		xPlane;
VECT	xFloorPos1 = { -1000.0f, -1000.0f, 0.0f };
VECT	xFloorPos2 = { 1000.0f, -1000.0f, 0.0f };
VECT	xFloorPos3 = { -1000.0f, 1000.0f, 0.0f };
VECT	xRayEnd;
VECT	xRayDir = *pxRayDir;

	VectScale( &xRayDir, &xRayDir, 10000.0f );
	VectAdd( &xRayEnd, pxRayStart, &xRayDir );
	
	EnginePlaneFromPoints( &xPlane, &xFloorPos1, &xFloorPos2, &xFloorPos3 );

	// find where on the zero-floor plane the ray hits
	if ( EnginePlaneIntersectLine( pxIntersectPoint, &xPlane, pxRayStart, &xRayEnd ) == TRUE )
	{
	int		nMapX1, nMapX2;
	int		nMapY1, nMapY2;
	int		nLoopX, nLoopY;
	VECT	xBoundSpherePos;
	float	fBoundSphereRadius;
	const VECT*	pxCollLineStart;
	const VECT*	pxCollLineEnd;
	VECT	xTileCollisionPoint;
	float	fCollisionDist;
	float	fClosestCollision = 9999999.0f;
	BOOL	bColl = FALSE;

		LandscapeGetMapCoord( pxRayStart, &nMapX1, &nMapY1 );
		LandscapeGetMapCoord( pxIntersectPoint, &nMapX2, &nMapY2 );
		if ( nMapX2 < nMapX1 )
		{
		int		nSwap = nMapX1;
			nMapX1 = nMapX2;
			nMapX2 = nSwap;
		}
		if ( nMapY2 < nMapY1 )
		{
		int		nSwap = nMapY1;
			nMapY1 = nMapY2;
			nMapY2 = nSwap;
		}
		nMapX1 -= 1;
		nMapY1 -= 1;
		nMapX2 += 1;
		nMapY2 += 1;

		// We now have a map region to check against...
		for ( nLoopY = nMapY1; nLoopY < nMapY2; nLoopY++ )
		{
			for ( nLoopX = nMapX1; nLoopX < nMapX2; nLoopX++ )
			{
				LandscapeTileGetBoundingSphere( nLoopX, nLoopY, &xBoundSpherePos, &fBoundSphereRadius );

				pxCollLineStart = pxRayStart;
				pxCollLineEnd = pxIntersectPoint;

				// use  EngineCollisionLineSphere   with the ray to floor against the bounding sphere of each tile
				if ( EngineCollisionLineSphere( pxCollLineStart, pxCollLineEnd, &xBoundSpherePos, fBoundSphereRadius, 0 ) == TRUE )
				{
				VECT	xVert1;
				VECT	xVert2;
				VECT	xVert3;
				float	fCollU, fCollV;

#ifdef NO_TILE_TRI_CHECK
					xTileCollisionPoint = LandscapeGetWorldPos( nLoopX, nLoopY );
					fCollisionDist = VectDist( &xTileCollisionPoint, pxCollLineStart );
					bColl = TRUE;
#else
					VectNormalize( &xRayDir );
					// if that hits, do an individual check against the triangles in the tile to get the accurate collision point
					LandscapeTileGetTri1( nLoopX, nLoopY, &xVert1, &xVert2, &xVert3 );
					bColl = EngineCollisionRayIntersectTri( &xVert1, &xVert2, &xVert3, pxCollLineStart, &xRayDir, &fCollU, &fCollV, &fCollisionDist );
					if ( !bColl )
					{
						LandscapeTileGetTri2( nLoopX, nLoopY, &xVert1, &xVert2, &xVert3 );
						bColl = EngineCollisionRayIntersectTri( &xVert1, &xVert2, &xVert3, pxCollLineStart, &xRayDir, &fCollU, &fCollV, &fCollisionDist );
					}
#endif
					if ( ( bColl ) &&
						 ( fCollisionDist < fClosestCollision ) )
					{
//						VectNormalize( &xRayDir );
						VectScale( &xRayDir, &xRayDir, fCollisionDist );
						VectAdd( &xTileCollisionPoint, pxCollLineStart, &xRayDir );

						fClosestCollision = fCollisionDist;
						*pxIntersectPoint = xTileCollisionPoint;
						if ( !bClosest )
						{
							return( TRUE );
						}
					}
				}
			}
		}
		return( bColl );
	}
	return( FALSE );
}


