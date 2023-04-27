
#include "../Marmalade/InterfaceInternalsMarmalade.h"

#include <StandardDef.h>
#include <Interface.h>

#include "InterfaceTexturedPolyLists.h"


#define		MAX_RECTS				(MAX_VERTS_IN_SINGLE_OVERLAY/6)


 
typedef struct
{
	int						nLayerNum;
	INTF_RENDER_TYPES		nRenderType;
	int						hOverlayTexture;
	TEXTURED_RECT_DEF*		pxRectsInOverlay;
	void*					pPlatformMaterial;

} OVERLAY_DATA;

OVERLAY_DATA		maxOverlayData[MAX_DIFFERENT_TEXTURED_OVERLAYS];

TEXTURED_RECT_DEF*		mpxRectBuffer = NULL;
TEXTURED_RECT_DEF*		mpxNextFreeRect = NULL;

int				mnTexRectBufferSize = 0;
int				mnTexRectMaxBufferSize = MAX_RECTS;

int		mnNumActiveTexOverlays = 0;

TEXTURED_RECT_DEF*	TexOverlayGetNextRect( int nOverlayNum )
{
TEXTURED_RECT_DEF*	pxRet = NULL;

	if ( mpxNextFreeRect != NULL )
	{
		pxRet = mpxNextFreeRect;
		mpxNextFreeRect->pNext = maxOverlayData[ nOverlayNum ].pxRectsInOverlay;
		maxOverlayData[ nOverlayNum ].pxRectsInOverlay = mpxNextFreeRect;

		mpxNextFreeRect++;
		if ( mpxNextFreeRect >= (mpxRectBuffer + (mnTexRectBufferSize-1)) )
		{
//			PANIC_IF( TRUE, "Too many textured overlay shapes added.");
			mpxNextFreeRect = NULL;
		}
	}
	return( pxRet );
}


INTERFACE_API void InterfaceTexturedRect( int nOverlayNum, int nX, int nY, int nWidth, int nHeight, uint32 ulCol, float fU, float fV, float fU2, float fV2 )
{
TEXTURED_RECT_DEF* pxRectDef;

	if ( nOverlayNum < 0 ) return;

	pxRectDef = TexOverlayGetNextRect( nOverlayNum );
	if ( pxRectDef != NULL )
	{
		pxRectDef->nType = TEX_OVLY_RECT;
		pxRectDef->nX = nX;
		pxRectDef->nY = nY;
		pxRectDef->nWidth = nWidth;
		pxRectDef->nHeight = nHeight;
		pxRectDef->ulCol = ulCol;
		pxRectDef->fU1 = fU;
		pxRectDef->fV1 = fV;
		pxRectDef->fU2 = fU2;
		pxRectDef->fV2 = fV2;

#ifdef TUD9
		pxRectDef->nX += mnInterfaceDrawX;
		pxRectDef->nY += mnInterfaceDrawY;
		pxRectDef->nX2 += mnInterfaceDrawX;
		pxRectDef->nY2 += mnInterfaceDrawY;
#endif
	}
}

INTERFACE_API void InterfaceSprite( int nOverlayNum, int nX, int nY, float fTexGrid, int nTexGridNum, uint32 ulCol, float fRotAngle, float fScale )
{
int		nTexMod;

	if ( maxOverlayData[ nOverlayNum ].hOverlayTexture != NOTFOUND )
	{
	int		nTexWidth, nTexHeight;
	TEXTURED_RECT_DEF* pxRectDef;

		pxRectDef = TexOverlayGetNextRect( nOverlayNum );
		if ( pxRectDef != NULL )
		{
			InterfaceGetTextureSize( maxOverlayData[ nOverlayNum ].hOverlayTexture, &nTexWidth, &nTexHeight );
	
			pxRectDef->nType = TEX_OVLY_SPRITE;
			pxRectDef->nX = nX;
			pxRectDef->nY = nY;

			if ( fTexGrid <= 0.0f )
			{
				fTexGrid = 1.0f;
			}

			pxRectDef->nWidth = (int)( nTexWidth * fTexGrid  );
			pxRectDef->nHeight = (int)( nTexHeight * fTexGrid );

			if ( ulCol == 0 ) ulCol = 0xFFFFFFFF;
			pxRectDef->ulCol = ulCol;

			nTexMod = (int)( 1.0f / fTexGrid );
			if ( nTexMod == 0 ) nTexMod = 1;

			pxRectDef->fU1 = (float)( nTexGridNum % nTexMod ) * fTexGrid;
			pxRectDef->fV1 = (float)( nTexGridNum / nTexMod ) * fTexGrid;

			pxRectDef->fU2 = pxRectDef->fU1 + fTexGrid - 0.0001f;
			pxRectDef->fV2 = pxRectDef->fV1 + fTexGrid - 0.0001f;
		
			pxRectDef->fRot = fRotAngle;

			if ( fScale != 0.0f ) 
			{
				pxRectDef->nWidth = (int)( pxRectDef->nWidth * fScale);
				pxRectDef->nHeight = (int)( pxRectDef->nHeight * fScale);
			}
		}
	}
}


/***************************************************************************
 * Function    : InterfaceTexturedTri
 * Params      : 
 * Returns     :
 * Description : 
 ***************************************************************************/
INTERFACE_API void InterfaceTexturedTri( int nOverlayNum, int* pnVerts, float* pfUVs, uint32 ulCol )
{
TEXTURED_RECT_DEF* pxRectDef;

	pxRectDef = TexOverlayGetNextRect( nOverlayNum );
	if ( pxRectDef != NULL )
	{
		pxRectDef->nType = TEX_OVLY_TRI;
		pxRectDef->nX = pnVerts[0];
		pxRectDef->nY = pnVerts[1];
		pxRectDef->nWidth = pnVerts[2];
		pxRectDef->nHeight = pnVerts[3];
		pxRectDef->nX2 = pnVerts[4];
		pxRectDef->nY2 = pnVerts[5];
		pxRectDef->ulCol = ulCol;
		pxRectDef->fU1 = pfUVs[0];
		pxRectDef->fV1 = pfUVs[1];
		pxRectDef->fU2 = pfUVs[2];
		pxRectDef->fV2 = pfUVs[3];
		pxRectDef->fU3 = pfUVs[4];
		pxRectDef->fV3 = pfUVs[5];
	}
}

int	TexturedOverlayCreate( int nLayer, TEXTURE_HANDLE hTexture )
{
int	nRet;

	nRet = mnNumActiveTexOverlays++;

#ifdef TUD9
//	PANIC_IF( mnNumActiveTexOverlays == MAX_DIFFERENT_TEXTURED_OVERLAYS,"Too many different textured overlays added" );
#endif

	if ( mnNumActiveTexOverlays == MAX_DIFFERENT_TEXTURED_OVERLAYS )
	{
		mnNumActiveTexOverlays--;
		return( nRet );
	}

	maxOverlayData[ nRet ].hOverlayTexture = hTexture;
	maxOverlayData[ nRet ].nRenderType = RENDER_TYPE_NORMAL;
	maxOverlayData[ nRet ].nLayerNum = nLayer;
	maxOverlayData[ nRet ].pPlatformMaterial = NULL;
	return( nRet );
}

void*	InterfaceTexturedOverlaysGetPlatformMaterial( int nOverlayNum )
{
	if ( maxOverlayData[ nOverlayNum ].pPlatformMaterial == NULL )
	{
		maxOverlayData[ nOverlayNum ].pPlatformMaterial = InterfacePlatformGetTexturedOverlayMaterial( maxOverlayData[ nOverlayNum ].hOverlayTexture, maxOverlayData[ nOverlayNum ].nRenderType );
	}

	return( maxOverlayData[ nOverlayNum ].pPlatformMaterial );

}

void	InterfaceOverlayRenderType( int nOverlayNum, INTF_RENDER_TYPES nRenderType )
{
	maxOverlayData[nOverlayNum].nRenderType = nRenderType;
}

INTERFACE_API int	InterfaceCreateNewTexturedOverlay( int nLayer, int nTextureHandle )
{
	if ( nTextureHandle != NOTFOUND )
	{
		return( TexturedOverlayCreate( nLayer, nTextureHandle ) );
	}
	return( NOTFOUND );
}

void InterfaceFreeTexturedPolyLists( void )
{
	if ( mpxRectBuffer != NULL )
	{
		free( mpxRectBuffer );
	}
	mpxRectBuffer =	NULL;
	mpxNextFreeRect = NULL;
}

void InterfaceInitTexturedPolyLists( void )
{
int		nLoop;

	if ( mpxRectBuffer != NULL )
	{
		free( mpxRectBuffer );
	}

	mpxRectBuffer = (TEXTURED_RECT_DEF*)( malloc( mnTexRectMaxBufferSize * sizeof(TEXTURED_RECT_DEF) ) );
	if ( mpxRectBuffer != NULL )
	{
		mnTexRectBufferSize = mnTexRectMaxBufferSize;
		ZeroMemory( mpxRectBuffer, mnTexRectMaxBufferSize * sizeof(TEXTURED_RECT_DEF) );
		for ( nLoop = 0; nLoop < MAX_DIFFERENT_TEXTURED_OVERLAYS; nLoop++ )
		{
			maxOverlayData[ nLoop ].pxRectsInOverlay = NULL;
		}
		mpxNextFreeRect = mpxRectBuffer;
	}
	else
	{
#ifdef TUD9
//		PANIC_IF( TRUE,"Couldnt allocate memory for textured overlay buffer" );
#endif
	}
}


void	InterfaceTexturedPolyListsReset( void )
{
	mpxNextFreeRect = mpxRectBuffer;
	mnNumActiveTexOverlays = 0;

	// TODO - Need to free up all the materials.. 

}


void InterfaceTexturedPolyListsDraw( int nLayer )
{
int		nLoop;
TEXTURED_RECT_DEF*	pxRectDefinition;

	for ( nLoop = 0; nLoop < MAX_DIFFERENT_TEXTURED_OVERLAYS; nLoop++ )
	{
		pxRectDefinition = maxOverlayData[ nLoop ].pxRectsInOverlay;
		// If there are any rects in this overlay
		if ( pxRectDefinition != NULL )
		{
			if ( maxOverlayData[ nLoop ].nLayerNum == nLayer )
			{
				// Add all the vertices required to draw all the shapes in this overlay
				while ( pxRectDefinition != NULL )
				{
					switch ( pxRectDefinition->nType )
					{
					case TEX_OVLY_SPRITE:
						InterfacePlatformAddSpriteVertices( nLoop, pxRectDefinition );
						break;
					case TEX_OVLY_RECT:
						InterfacePlatformAddTexturedRectVertices( nLoop, pxRectDefinition );
						break;
					case TEX_OVLY_TRI:
						InterfacePlatformAddTexturedTriVertices( nLoop, pxRectDefinition );
						break;
					}
				
					pxRectDefinition = (TEXTURED_RECT_DEF*)( pxRectDefinition->pNext );
				}

				InterfaceDrawTexturedPolys( nLoop, maxOverlayData[ nLoop ].hOverlayTexture, nLayer );
				maxOverlayData[ nLoop ].pxRectsInOverlay = NULL;
				maxOverlayData[ nLoop ].hOverlayTexture = NOTFOUND;
				InterfacePlatformReleaseTexturedOverlayMaterial( maxOverlayData[ nLoop ].pPlatformMaterial );
				maxOverlayData[ nLoop ].pPlatformMaterial = NULL;
			}
		}
	}
	
}
