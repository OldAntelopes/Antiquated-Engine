
#include <math.h>

#include "StandardDef.h"
#include "Engine.h"
#include "Rendering.h"

#include "../../../LibCode/Engine/ModelRendering.h"

#include "../ModelConverter.h"
#include "UVGenerate.h"

enum
{
	BOXSIDE_TOP,
	BOXSIDE_BOTTOM,
	BOXSIDE_LEFT,
	BOXSIDE_RIGHT,
	BOXSIDE_BACK,
	BOXSIDE_FRONT,
};

int			UVGenerateGetBoxSide( VECT* pxNormal )
{
float	fAbsX = fabs( pxNormal->x );
float	fAbsY = fabs( pxNormal->y );
float	fAbsZ = fabs( pxNormal->z );

	if ( fAbsZ > fAbsX )
	{
		if ( fAbsZ > fAbsY )
		{
			// z is most prominent so its top or bottom
			if ( pxNormal->z > 0.0f )
			{
				return( BOXSIDE_TOP );
			}
			return( BOXSIDE_BOTTOM );
		}
		else
		{
			// y is most prominent so its left or right
			if ( pxNormal->y > 0.0f )
			{
				return( BOXSIDE_RIGHT );
			}
			return( BOXSIDE_LEFT );
		}
	}
	// X is larger than Z
	else if ( fAbsY > fAbsX )
	{
		// y is larger than X and Z
		if ( pxNormal->y > 0.0f )
		{
			return( BOXSIDE_RIGHT );
		}
		return( BOXSIDE_LEFT );
	}
	// X is larger than Y and Z
	else 
	{
		if ( pxNormal->x > 0.0f )
		{
			return( BOXSIDE_FRONT );
		}
		return( BOXSIDE_BACK );
	}

}


void		UVGenerateBox( int nModelHandle )
{
MODEL_RENDER_DATA*	pxModelData = maxModelRenderData;
CUSTOMVERTEX*	pxVertices;
CUSTOMVERTEX*	pxVertBase;
int		nVertLoop;
int		nVertsInMesh;
int		nBoxSide;
float	fSideDistX;
float	fSideDistY;
VECT	xBoundSize;
float	fU, fV;

	if ( nModelHandle != NOTFOUND )
	{
		pxModelData += nModelHandle;

		xBoundSize.x = pxModelData->xStats.xBoundMax.x - pxModelData->xStats.xBoundMin.x;
		xBoundSize.y = pxModelData->xStats.xBoundMax.y - pxModelData->xStats.xBoundMin.y;
		xBoundSize.z = pxModelData->xStats.xBoundMax.z - pxModelData->xStats.xBoundMin.z;

		if ( xBoundSize.x == 0.0f ) xBoundSize.x = 1.0f;
		if ( xBoundSize.y == 0.0f ) xBoundSize.y = 1.0f;
		if ( xBoundSize.z == 0.0f ) xBoundSize.z = 1.0f;

		if ( pxModelData->pxBaseMesh != NULL )
		{
			nVertsInMesh = pxModelData->pxBaseMesh->GetNumVertices();
			pxModelData->pxBaseMesh->LockVertexBuffer( NULL, (byte**)( &pxVertices ) );
			pxVertBase = pxVertices;
			for ( nVertLoop = 0; nVertLoop < nVertsInMesh; nVertLoop++ )
			{
				nBoxSide = UVGenerateGetBoxSide( &pxVertices->normal );
				switch( nBoxSide )
				{
				case BOXSIDE_TOP:
					fSideDistX = pxVertices->position.x - pxModelData->xStats.xBoundMin.x;
					fSideDistY = pxVertices->position.y - pxModelData->xStats.xBoundMin.y;
					fU = fSideDistX / xBoundSize.x;
					fV = fSideDistY / xBoundSize.y;
					break;
				case BOXSIDE_BOTTOM:
					fSideDistX = pxVertices->position.x - pxModelData->xStats.xBoundMin.x;
					fSideDistY = pxVertices->position.y - pxModelData->xStats.xBoundMin.y;
					fU = 1.0f - (fSideDistX / xBoundSize.x);
					fV = (fSideDistY / xBoundSize.y);
					break;
				case BOXSIDE_LEFT:
					fSideDistX = pxVertices->position.x - pxModelData->xStats.xBoundMin.x;
					fSideDistY = pxVertices->position.z - pxModelData->xStats.xBoundMin.z;
					fU = 1.0f - (fSideDistX / xBoundSize.x);
					fV = 1.0f - (fSideDistY / xBoundSize.z);
//					fU = 0.0f;
	//				fV = 0.0f;
					break;
				case BOXSIDE_RIGHT:
					fSideDistX = pxVertices->position.x - pxModelData->xStats.xBoundMin.x;
					fSideDistY = pxVertices->position.z - pxModelData->xStats.xBoundMin.z;
					fU = (fSideDistX / xBoundSize.x);
					fV = 1.0f - (fSideDistY / xBoundSize.z);
					break;
				case BOXSIDE_BACK:
					fSideDistX = pxVertices->position.y - pxModelData->xStats.xBoundMin.y;
					fSideDistY = pxVertices->position.z - pxModelData->xStats.xBoundMin.z;
					fU = (fSideDistX / xBoundSize.y);
					fV = 1.0f - (fSideDistY / xBoundSize.z);
					break;
				case BOXSIDE_FRONT:
					fSideDistX = pxVertices->position.y - pxModelData->xStats.xBoundMin.y;
					fSideDistY = pxVertices->position.z - pxModelData->xStats.xBoundMin.z;
					fU = 1.0f - (fSideDistX / xBoundSize.y);
					fV = 1.0f - (fSideDistY / xBoundSize.z);
					break;
				}

				pxVertices->tu = fU;
				pxVertices->tv = fV;
				pxVertices++;
			}
			pxModelData->pxBaseMesh->UnlockVertexBuffer();
		}
	}

}




void		UVGenerate( int nModelHandle, int nMode )
{

	UVGenerateBox( nModelHandle );

}
