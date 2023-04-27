
#include "StandardDef.h"
#include "Engine.h"
#include "Rendering.h"

#include "../../../Pub/LibCode/Engine/ModelRendering.h"

#include "../ModelConverter.h"
#include "VertexColours.h"


void	ModelConvPreLight( int nHandle )
{

	// todo - MG Dec 2011--------------------------------------------------------------
#ifdef NEEDS_UPDATING_WITH_NEW_ENGINE_CODE
	// todo - MG Dec 2011--------------------------------------------------------------


MODEL_RENDER_DATA*	pxModelData = maxModelRenderData;
CUSTOMVERTEX*	pxVertices;
int		nVertLoop;
int		nVertsInMesh;
VECT	xSunPos = { 50.0f, 50.0f, 50.0f };
VECT	xSunDir = { -1.0f, -1.0f, -1.0f };
VECT	xRay = { -1.0f, -1.0f, -1.0f };
BOOL	boCollision = FALSE;
float	fCollisionDist = 10000.0f;
uint32 	ulCollIndex;
float	fDist;
uint32	ulAmbientR = 0x40;
uint32	ulAmbientG = 0x40;
uint32	ulAmbientB = 0x40;
uint32	ambient	= 0xFF404040;
VECT	xNormal;

	if ( nHandle != NOTFOUND )
	{
		pxModelData += nHandle;
		
		if ( pxModelData->pxBaseMesh != NULL )
		{
			VectNormalize( &xSunDir );
			nVertsInMesh = pxModelData->pxBaseMesh->GetNumVertices();
			pxModelData->pxBaseMesh->LockVertexBuffer( NULL, (byte**)( &pxVertices ) );
			pxModelData->xGlobalProperties.bNoLighting = 1;
			for ( nVertLoop = 0; nVertLoop < nVertsInMesh; nVertLoop++ )
			{
				fCollisionDist = 10000.0f;
				boCollision = FALSE;
				xRay.x = pxVertices->position.x - xSunPos.x;
				xRay.y = pxVertices->position.y - xSunPos.y;
				xRay.z = pxVertices->position.z - xSunPos.z;
				VectNormalize( &xRay );
				// Cast ray from sun to vertex and check if obscured (shadow if so)
				fDist = VectDist( (VECT*)&pxVertices->position, &xSunPos );
				D3DXIntersect( pxModelData->pxBaseMesh, (D3DXVECTOR3*)&xSunPos, (D3DXVECTOR3*)&xRay, &boCollision, &ulCollIndex, NULL, NULL, &fCollisionDist, NULL, NULL );
				if ( boCollision )
				{
					// If vertex is in shadow
					if ( fCollisionDist < fDist - 0.0001f )
					{
						pxVertices->color = ambient;
					}
					else
					{
					float	fDot;
					uint32	R,G,B;
						xNormal = *( (VECT*)&pxVertices->normal );
						fDot = VectDot( &xNormal, &xSunDir ) * -1.0f;
						if ( fDot > 0.0f )
						{
							R = (uint32)((0xFF-ulAmbientR) * fDot) + ulAmbientR;
							G = (uint32)((0xFF-ulAmbientG) * fDot) + ulAmbientG;
							B = (uint32)((0xFF-ulAmbientB) * fDot) + ulAmbientB;
							pxVertices->color = 0xFF000000 | (R<<16) | (G<<8) | (B);
						}
						else
						{
							pxVertices->color = ambient;
						}
					}
				}
				else
				{
					pxVertices->color = ambient;
				}
				// TODO Calc dist from sun to vertex to calc light brightness
				pxVertices++;
			}
			pxModelData->pxBaseMesh->UnlockVertexBuffer();
		}
	}
	ModelConverterDisplayFrame( TRUE );

	// todo - MG Dec 2011--------------------------------------------------------------
#endif		// #ifdef NEEDS_UPDATING_WITH_NEW_ENGINE_CODE
	// todo - MG Dec 2011--------------------------------------------------------------

}

void	ModelConvResetVertexColours( int nHandle )
{
MODEL_RENDER_DATA*	pxModelData = maxModelRenderData;
CUSTOMVERTEX*	pxVertices;
int		nVertLoop;
int		nVertsInMesh;

	if ( nHandle != NOTFOUND )
	{
		pxModelData += nHandle;
		
		if ( pxModelData->pxBaseMesh != NULL )
		{
			nVertsInMesh = pxModelData->pxBaseMesh->GetNumVertices();
			pxModelData->pxBaseMesh->LockVertexBuffer( NULL, (byte**)( &pxVertices ) );

			for ( nVertLoop = 0; nVertLoop < nVertsInMesh; nVertLoop++ )
			{
				pxVertices->color = 0xFFFFFFFF;
				pxVertices++;
			}
			pxModelData->pxBaseMesh->UnlockVertexBuffer();
		}
	}
	ModelConverterDisplayFrame( TRUE );
}
