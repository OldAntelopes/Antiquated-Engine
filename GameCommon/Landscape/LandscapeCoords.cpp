
#include "StandardDef.h"
#include "Engine.h"

#include "LandscapeHeightmap.h"
#include "LandscapeCoords.h"
 
float		mfMapToWorldScale = 10.0f;

LANDRENDERVERTEX*		mpxLandRenderVertices = NULL;
int			mnLandscapeCoordsMapSizeX = 0;
int			mnLandscapeCoordsMapSizeY = 0;
int			mnLandscapeCoordsMapSizeTotal = 1;


int		LandscapeGetMapSizeX( void )
{
	return( mnLandscapeCoordsMapSizeX );
}

int		LandscapeGetMapSizeY( void )
{
	return( mnLandscapeCoordsMapSizeY );
}


void LandscapeCoordsUpdateVertexData( int nMapX, int nMapY )
{
LANDRENDERVERTEX*	pxVertex;
int				nIndex = ((nMapY*mnLandscapeCoordsMapSizeX) + nMapX) % mnLandscapeCoordsMapSizeTotal;

	pxVertex = mpxLandRenderVertices + nIndex;
	pxVertex->position.x = nMapX * mfMapToWorldScale;
	pxVertex->position.y = nMapY * mfMapToWorldScale;
	pxVertex->position.z = LandscapeHeightmapGetHeight( nMapX, nMapY );
	pxVertex->normal = LandscapeCalculateNormal( nMapX, nMapY ); 
	pxVertex->color = 0xFFFFFFFF;
	pxVertex->tu       = 0.0f;
	pxVertex->tv       = 0.0f;

} 

const inline LANDRENDERVERTEX*		LandscapeGetRenderVertex( int nMapX, int nMapY )
{
int				nIndex = ((nMapY*mnLandscapeCoordsMapSizeX) + nMapX) % mnLandscapeCoordsMapSizeTotal;

	return( mpxLandRenderVertices + nIndex );
}

void	LandscapeCoordsShutdown( void )
{
	SAFE_FREE( mpxLandRenderVertices );
}


void	LandscapeCoordsInit( int nMapSizeX, int nMapSizeY )
{
int		nLoopX;
int		nLoopY;
LANDRENDERVERTEX*	pxVertex;

	if ( mpxLandRenderVertices)
	{
		free( mpxLandRenderVertices );
	}

	mnLandscapeCoordsMapSizeX = nMapSizeX;
	mnLandscapeCoordsMapSizeY = nMapSizeY;
	mnLandscapeCoordsMapSizeTotal = nMapSizeX * nMapSizeY;
	mpxLandRenderVertices = (LANDRENDERVERTEX*)( malloc( nMapSizeX * nMapSizeY * sizeof( LANDRENDERVERTEX ) ) );

	// Set the positions first so the normals can be calculated on the second pass
	for( nLoopY = 0; nLoopY < nMapSizeY; nLoopY++ )
	{
		for( nLoopX = 0; nLoopX < nMapSizeX; nLoopX++ )
		{
		int				nIndex = ((nLoopY*mnLandscapeCoordsMapSizeX) + nLoopX) % mnLandscapeCoordsMapSizeTotal;

			pxVertex = mpxLandRenderVertices + nIndex;
			pxVertex->position.x = nLoopX * mfMapToWorldScale;
			pxVertex->position.y = nLoopY * mfMapToWorldScale;
			pxVertex->position.z = LandscapeHeightmapGetHeight( nLoopX, nLoopY );
		}
	}

	for( nLoopY = 0; nLoopY < nMapSizeY; nLoopY++ )
	{
		for( nLoopX = 0; nLoopX < nMapSizeX; nLoopX++ )
		{
			LandscapeCoordsUpdateVertexData( nLoopX, nLoopY );
		}
	}

}

void	LandscapeSetTileSize( float fTileSize )
{
	mfMapToWorldScale = fTileSize;
}

float	LandscapeMapToWorldScale( void )
{
	return( mfMapToWorldScale );
}

float	LandscapeWorldToMapScale( void )
{
	return( 1.0f / mfMapToWorldScale );
}

VECT		LandscapeGetWorldPos( int nMapX, int nMapY )
{ 
//LANDRENDERVERTEX*	pxVertex;
int				nIndex = ( abs((nMapY*mnLandscapeCoordsMapSizeX) + nMapX) ) % mnLandscapeCoordsMapSizeTotal;

	return( mpxLandRenderVertices[nIndex].position );
}

void		LandscapeGetMapCoord( const VECT* pxPos, int* pnMapX, int* pnMapY )
{
float	 fWorldToMapScale = 1.0f / mfMapToWorldScale;

	*pnMapX = (int)( pxPos->x * fWorldToMapScale );
	*pnMapY = (int)( pxPos->y * fWorldToMapScale );
}

VECT		LandscapeGetNormal( int nMapX, int nMapY )
{
//LANDRENDERVERTEX*	pxVertex;
int				nIndex = ( abs((nMapY*mnLandscapeCoordsMapSizeX) + nMapX) ) % mnLandscapeCoordsMapSizeTotal;

	return( mpxLandRenderVertices[nIndex].normal );
}

VECT LandscapeCalculateNormal( int nMapX, int nMapY )
{
VECT		xNormal;
VECT	xCalc;
VECT	xDiff;
VECT	xDiff2;
VECT	xBase;
VECT	xConnectedVert;
short		awOffsets[] = { -1,-1, 0,-1, 1,-1, 1,0, 1,1, 0,1,-1,1, -1,0, -1, -1 };
int			nLoop;
int		nNextX;
int		nNextY;
int		nHeightmapWidth;
	
	nHeightmapWidth = LandscapeHeightmapGetSizeX();

	xNormal.x = 0;
	xNormal.y = 0;
	xNormal.z = 0;

	xBase = LandscapeGetWorldPos( nMapX, nMapY );

	for ( nLoop = 0; nLoop < 16; nLoop += 2 )
	{
		nNextX = nMapX + awOffsets[ nLoop ];
		nNextY = nMapY + awOffsets[ nLoop + 1 ];

		if ( nNextX < 0 )
		{
			nNextX = 0;
		}
		else if ( nNextX >= nHeightmapWidth )
		{
			nNextX = nHeightmapWidth;
		}
		if ( nNextY < 0 )
		{
			nNextY = 0;
		}
		else if ( nNextY >= nHeightmapWidth )
		{
			nNextY = nHeightmapWidth;
		}
		xConnectedVert = LandscapeGetWorldPos( nNextX, nNextY );

		xDiff.x = xBase.x - xConnectedVert.x;
		xDiff.y = xBase.y - xConnectedVert.y;
		xDiff.z = xBase.z - xConnectedVert.z;

		nNextX = nMapX + awOffsets[ nLoop + 2 ];
		nNextY = nMapY + awOffsets[ nLoop + 3 ];

		if ( nNextX < 0 )
		{
			nNextX += nHeightmapWidth;
		}
		else if ( nNextX >= nHeightmapWidth )
		{
			nNextX -= nHeightmapWidth;
		}
		if ( nNextY < 0 )
		{
			nNextY += nHeightmapWidth;
		}
		else if ( nNextY >= nHeightmapWidth )
		{
			nNextY -= nHeightmapWidth;
		}

		xConnectedVert = LandscapeGetWorldPos( nNextX, nNextY);

		xDiff2.x = xBase.x - xConnectedVert.x;
		xDiff2.y = xBase.y - xConnectedVert.y;
		xDiff2.z = xBase.z - xConnectedVert.z;

        VectCross( &xCalc, &xDiff, &xDiff2 );
	    VectNormalize( &xCalc );

		if ( xCalc.z < 0.0f )
		{
			xNormal.x -= xCalc.x;
			xNormal.y -= xCalc.y;
			xNormal.z -= xCalc.z;
		}
		else
		{
			xNormal.x += xCalc.x;
			xNormal.y += xCalc.y;
			xNormal.z += xCalc.z;
		}
	}

	xNormal.x = xNormal.x * 0.95f;	// Increase the effect of x & y slope
	xNormal.y = xNormal.y * 0.95f;
	xNormal.z = xNormal.z * 0.90f;
	
	VectNormalize( &xNormal );
	return( xNormal );
}