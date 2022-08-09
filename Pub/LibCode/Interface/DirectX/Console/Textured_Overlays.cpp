

#include "../InterfaceInternalsDX.h"

#include <StandardDef.h>
#include <Interface.h>
#include <Engine.h>

#include "Overlays.h"
#include "TexturedOverlays.h"
#include "../../Common/InterfaceUtil.h"

//------------------------------------------------------------------------------------------------------------


#define		MAX_TEX_OVERLAY_VERTICES			2048

#define		MAX_DIFFERENT_TEXTURED_OVERLAYS		128
#define		MAX_RECTS							8192

/*** This structure is used to specify a shape in the Textured Overlays module ***/
typedef struct
{
	BYTE	nType;
	BYTE	bPad;

	short	 nX;
	short	 nY;
	
	short	nWidth;
	short	nHeight;
	
	short	nX2;
	short	nY2;

	ulong	ulCol;
	float	fU1;
	float	fU2;

	union {	float	fU3;	float fRot; };

	float	fV1;
	float	fV2;
	float	fV3;

	void*	pNext;

} TEXTURED_RECT_DEF;
 

#define		MAX_INTERNAL_TEXTURES_LOADED		256


typedef struct
{
	LPGRAPHICSTEXTURE		pD3DTexture;
	char					acFilename[128];
	ulong					ulLastTouched;
	int						nRefCount;

} INTERNAL_TEXTURES;

typedef struct
{
	int						nLayerNum;
	INTF_RENDER_TYPES		nRenderType;
	LPGRAPHICSTEXTURE		pxOverlayTexture;
	TEXTURED_RECT_DEF*		pxRectsInOverlay;

} OVERLAY_DATA;

BOOL		mboInterfaceTexturesFirstInitialise = TRUE;

INTERNAL_TEXTURES	maxInternalTextures[MAX_INTERNAL_TEXTURES_LOADED];
OVERLAY_DATA		maxOverlayData[MAX_DIFFERENT_TEXTURED_OVERLAYS];


/*** These enum lists define settings for the Textured Overlays module **/
enum
{
	TEX_OVLY_NONE = 0,
	TEX_OVLY_RECT,
	TEX_OVLY_TRI,
	TEX_OVLY_SPRITE,
};



TEXTURED_RECT_DEF*		mpxRectBuffer = NULL;
TEXTURED_RECT_DEF*		mpxNextFreeRect = NULL;

int				mnTexRectBufferSize = 0;
int				mnTexRectMaxBufferSize = MAX_RECTS;

IGRAPHICSVERTEXBUFFER*		mpCurrentTexOverlayVertexBuffer; // Buffer to hold vertices

IGRAPHICSVERTEXBUFFER*		mpTexOverlayVertexBuffer1; // Buffer to hold vertices
IGRAPHICSVERTEXBUFFER*		mpTexOverlayVertexBuffer2; // Buffer to hold vertices

int			mnNextTexOverlayVertex = 0;

int			mnNumActiveTexOverlays = 0;
int			mnCurrentTexOverlayRenderTexture = 0;

int		mnCurrentRenderType = 0;

//------------------------------------------------------------------------------------------------------------


TEXTURED_RECT_DEF*	TexOverlayGetNextRect( int nOverlayNum )
{
TEXTURED_RECT_DEF*	pxRet = NULL;
	if ( nOverlayNum >= 0 )
	{
		if ( mpxNextFreeRect != NULL )
		{
			pxRet = mpxNextFreeRect;
			mpxNextFreeRect->pNext = maxOverlayData[ nOverlayNum ].pxRectsInOverlay;
			maxOverlayData[ nOverlayNum ].pxRectsInOverlay = mpxNextFreeRect;

			mpxNextFreeRect++;
			if ( mpxNextFreeRect >= (mpxRectBuffer + (mnTexRectBufferSize-1)) )
			{
				PANIC_IF( TRUE, "Too many textured overlay shapes added.");
				mpxNextFreeRect = NULL;
			}
		}
	}
	return( pxRet );
}

/***************************************************************************
 * Function    : InterfaceOverlayRenderType
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
INTERFACE_API void InterfaceOverlayRenderType( int nOverlayNum, INTF_RENDER_TYPES nRenderType )
{
	if ( nOverlayNum >= 0 )
	{
		maxOverlayData[ nOverlayNum ].nRenderType = nRenderType;
	}

}

/***************************************************************************
 * Function    : RenderTexOverlays
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
void RenderTexOverlays( void )
{
int		nDrawHowMany;

	nDrawHowMany = (mnNextTexOverlayVertex / 3);

	if ( nDrawHowMany > 0 )
	{
		EngineSetVertexFormat( VERTEX_FORMAT_FLATVERTEX );
		EngineEnableZTest( FALSE );
		EngineEnableZWrite( FALSE );
		EngineEnableLighting( FALSE );
		EngineEnableBlend( TRUE );
		EngineEnableFog( FALSE );

#ifdef TUD11

#else
		switch ( mnCurrentRenderType )
		{
		case RENDER_TYPE_NORMAL_NOCLAMP:
			mpInterfaceD3DDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP );
			mpInterfaceD3DDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP );
			// No break deliberate
		case RENDER_TYPE_NORMAL:
			mpInterfaceD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			mpInterfaceD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			mpInterfaceD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
			mpInterfaceD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
			mpInterfaceD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
			break;
		case RENDER_TYPE_SUBTRACTIVE:
			mpInterfaceD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
			mpInterfaceD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);
			mpInterfaceD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
			mpInterfaceD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
			mpInterfaceD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
			break;
		case RENDER_TYPE_ALPHA_SUBTRACTIVE:
			mpInterfaceD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
//			mpInterfaceD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);
			mpInterfaceD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			mpInterfaceD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
			mpInterfaceD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
			mpInterfaceD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
			break;
		case RENDER_TYPE_ADDITIVE_NOCLAMP:
			mpInterfaceD3DDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP );
			mpInterfaceD3DDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP );
			// No break deliberate
		case RENDER_TYPE_ADDITIVE:
			mpInterfaceD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			mpInterfaceD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
			mpInterfaceD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
			mpInterfaceD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
			break;
		case RENDER_TYPE_COLORBLEND:
			mpInterfaceD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCCOLOR);
			mpInterfaceD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);
			mpInterfaceD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
			mpInterfaceD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
			mpInterfaceD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
			break;
		}
#endif
		EngineResetColourMode();

		// Enable alpha testing (skips pixels with less than a certain alpha.)
		EngineEnableAlphaTest( 1 );

		if ( maxOverlayData[ mnCurrentTexOverlayRenderTexture ].pxOverlayTexture != NULL )
		{
			mpInterfaceD3DDevice->SetTexture( 0, maxOverlayData[ mnCurrentTexOverlayRenderTexture ].pxOverlayTexture );
			EngineSetColourMode( 0, COLOUR_MODE_TEXTURE_MODULATE );
		}
		else
		{
			mpInterfaceD3DDevice->SetTexture( 0, NULL );
			EngineSetColourMode( 0, COLOUR_MODE_DIFFUSE_ONLY );
		}
		InterfaceInternalDXSetStreamSource( 0, mpCurrentTexOverlayVertexBuffer, 0, sizeof(FLATVERTEX) );

		mpInterfaceD3DDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, nDrawHowMany );

		switch ( mnCurrentRenderType )
		{
		case RENDER_TYPE_NORMAL_NOCLAMP:
		case RENDER_TYPE_ADDITIVE_NOCLAMP:
			// Turn clamping back on once we're done
			mpInterfaceD3DDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
			mpInterfaceD3DDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );
			break;
		}
	}

	mnNextTexOverlayVertex = 0;
	if ( mpCurrentTexOverlayVertexBuffer == mpTexOverlayVertexBuffer1 )
	{
		mpCurrentTexOverlayVertexBuffer = mpTexOverlayVertexBuffer2;
	}
	else
	{
		mpCurrentTexOverlayVertexBuffer = mpTexOverlayVertexBuffer1;
	} 


} /** End of function RenderTexOverlays **/




/***************************************************************************
 * Function    : AddTexturedTriVertices
 * Params      : 
 * Returns     :
 * Description : 
 ***************************************************************************/
void AddTexturedTriVertices( FLATVERTEX** ppVertices, TEXTURED_RECT_DEF* pxRectDef )
{
FLATVERTEX*		pVertices;

	if ( mnNextTexOverlayVertex >= ( MAX_TEX_OVERLAY_VERTICES - 6 ) )
	{
		mpCurrentTexOverlayVertexBuffer->Unlock();
		RenderTexOverlays();
		if( FAILED( mpCurrentTexOverlayVertexBuffer->Lock( 0, 0, (VERTEX_LOCKTYPE)ppVertices, D3DLOCK_DISCARD ) ) )
		{
			PANIC_IF(TRUE, "Vertex lock failed in addtexoverlay" );
		}
	}

	pVertices = *( ppVertices );

	/** First tri **/
	pVertices[ mnNextTexOverlayVertex ].x = (float)( pxRectDef->nX );
	pVertices[ mnNextTexOverlayVertex ].y = (float)( pxRectDef->nY );
	pVertices[ mnNextTexOverlayVertex ].color = pxRectDef->ulCol;
	pVertices[ mnNextTexOverlayVertex ].tu = pxRectDef->fU1;
	pVertices[ mnNextTexOverlayVertex ].tv = pxRectDef->fV1;
//	pVertices[ mnNextTexOverlayVertex ].rhw = 1.0f;
	mnNextTexOverlayVertex++;

	pVertices[ mnNextTexOverlayVertex ].x = (float)( pxRectDef->nWidth );
	pVertices[ mnNextTexOverlayVertex ].y = (float)( pxRectDef->nHeight );
	pVertices[ mnNextTexOverlayVertex ].color = pxRectDef->ulCol;
	pVertices[ mnNextTexOverlayVertex ].tu = pxRectDef->fU2;
	pVertices[ mnNextTexOverlayVertex ].tv = pxRectDef->fV2;
//	pVertices[ mnNextTexOverlayVertex ].rhw = 1.0f;
	mnNextTexOverlayVertex++;

	pVertices[ mnNextTexOverlayVertex ].x = (float)( pxRectDef->nX2 );
	pVertices[ mnNextTexOverlayVertex ].y = (float)( pxRectDef->nY2 );
	pVertices[ mnNextTexOverlayVertex ].color = pxRectDef->ulCol;
	pVertices[ mnNextTexOverlayVertex ].tu = pxRectDef->fU3;
	pVertices[ mnNextTexOverlayVertex ].tv = pxRectDef->fV3;
//	pVertices[ mnNextTexOverlayVertex ].rhw = 1.0f;
	mnNextTexOverlayVertex++;


}




/***************************************************************************
 * Function    : AddTexturedRectVertices
 * Params      : 
 * Returns     :
 * Description : 
 ***************************************************************************/
void AddTexturedRectVertices( FLATVERTEX** ppVertices, TEXTURED_RECT_DEF* pxRectDef )
{
FLATVERTEX*		pVertices;

	if ( mnNextTexOverlayVertex >= ( MAX_TEX_OVERLAY_VERTICES - 6 ) )
	{
		mpCurrentTexOverlayVertexBuffer->Unlock();
		RenderTexOverlays();
		if( FAILED( mpCurrentTexOverlayVertexBuffer->Lock( 0, 0, (VERTEX_LOCKTYPE)ppVertices, D3DLOCK_DISCARD ) ) )
		{
			PANIC_IF(TRUE, "Vertex lock failed in addtexoverlay" );
		}
	}

	pVertices = *( ppVertices );
	pVertices += mnNextTexOverlayVertex;

	/** First tri **/
	pVertices->x = (float)( pxRectDef->nX ) + 0.25f;
	pVertices->y = (float)( pxRectDef->nY ) + 0.25f;
	pVertices->z = 0.0f;
	pVertices->color = pxRectDef->ulCol;
	pVertices->tu = pxRectDef->fU1;
	pVertices->tv = pxRectDef->fV1;
	pVertices++; 

	pVertices->x = (float)( pxRectDef->nX + pxRectDef->nWidth  ) + 0.25f;
	pVertices->y = (float)( pxRectDef->nY ) + 0.25f;
	pVertices->z = 0.0f;
	pVertices->color = pxRectDef->ulCol;
	pVertices->tu = pxRectDef->fU2;
	pVertices->tv = pxRectDef->fV1;
//	pVertices->rhw = 1.0f;
	pVertices++;

	pVertices->x = (float)( pxRectDef->nX ) + 0.25f;
	pVertices->y = (float)( pxRectDef->nY + pxRectDef->nHeight) + 0.25f;
	pVertices->z = 0.0f;
	pVertices->color = pxRectDef->ulCol;
	pVertices->tu = pxRectDef->fU1;
	pVertices->tv = pxRectDef->fV2;
//	pVertices->rhw = 1.0f;
	pVertices++;
	
	//** Second tri **
	pVertices->x = (float)( pxRectDef->nX + pxRectDef->nWidth ) + 0.25f;
	pVertices->y = (float)( pxRectDef->nY ) + 0.25f;
	pVertices->z = 0.0f;
	pVertices->color = pxRectDef->ulCol;
	pVertices->tu = pxRectDef->fU2;
	pVertices->tv = pxRectDef->fV1;
//	pVertices->rhw = 1.0f;
	pVertices++;
	
	pVertices->x = (float)( pxRectDef->nX + pxRectDef->nWidth  ) + 0.25f;
	pVertices->y = (float)( pxRectDef->nY + pxRectDef->nHeight ) + 0.25f;
	pVertices->z = 0.0f;
	pVertices->color = pxRectDef->ulCol;
	pVertices->tu = pxRectDef->fU2;
	pVertices->tv = pxRectDef->fV2;
//	pVertices->rhw = 1.0f;
	pVertices++;
	
	pVertices->x = (float)( pxRectDef->nX ) + 0.25f;
	pVertices->y = (float)( pxRectDef->nY + pxRectDef->nHeight ) + 0.25f;
	pVertices->z = 1.0f;
	pVertices->color = pxRectDef->ulCol;
	pVertices->tu = pxRectDef->fU1;
	pVertices->tv = pxRectDef->fV2;
//	pVertices->rhw = 1.0f;
	pVertices++;

	mnNextTexOverlayVertex += 6;
}

void AddSpriteVertices( FLATVERTEX** ppVertices, TEXTURED_RECT_DEF* pxRectDef )
{
FLATVERTEX*		pVertices;
VECT			xVect;

	if ( mnNextTexOverlayVertex >= ( MAX_TEX_OVERLAY_VERTICES - 6 ) )
	{
		mpCurrentTexOverlayVertexBuffer->Unlock();
		RenderTexOverlays();
		if( FAILED( mpCurrentTexOverlayVertexBuffer->Lock( 0, 0, (VERTEX_LOCKTYPE)ppVertices, D3DLOCK_DISCARD ) ) )
		{
			PANIC_IF(TRUE, "Vertex lock failed in addtexoverlay" );
		}
	}  

	pVertices = *( ppVertices );
	pVertices += mnNextTexOverlayVertex;

	xVect.x = pxRectDef->nWidth * -0.5f;
	xVect.y = pxRectDef->nHeight * -0.5f;
	xVect.z = 0.0f;
	VectRotateAboutZ( &xVect, pxRectDef->fRot );
	/** First tri **/
	pVertices->x = (float)( pxRectDef->nX ) + xVect.x + 0.25f;
	pVertices->y = (float)( pxRectDef->nY ) + xVect.y + 0.25f;
	pVertices->z = 0.0f;
	pVertices->color = pxRectDef->ulCol;
	pVertices->tu = pxRectDef->fU1;
	pVertices->tv = pxRectDef->fV1;
	pVertices++; 

	xVect.x = pxRectDef->nWidth * 0.5f;
	xVect.y = pxRectDef->nHeight * -0.5f;
	xVect.z = 0.0f;
	VectRotateAboutZ( &xVect, pxRectDef->fRot );

	pVertices->x = (float)( pxRectDef->nX + xVect.x ) + 0.25f;
	pVertices->y = (float)( pxRectDef->nY + xVect.y ) + 0.25f;
	pVertices->z = 0.0f;
	pVertices->color = pxRectDef->ulCol;
	pVertices->tu = pxRectDef->fU2;
	pVertices->tv = pxRectDef->fV1;
//	pVertices->rhw = 1.0f;
	pVertices++;

	xVect.x = pxRectDef->nWidth * -0.5f;
	xVect.y = pxRectDef->nHeight * 0.5f;
	xVect.z = 0.0f;
	VectRotateAboutZ( &xVect, pxRectDef->fRot );

	pVertices->x = (float)( pxRectDef->nX + xVect.x ) + 0.25f;
	pVertices->y = (float)( pxRectDef->nY + xVect.y ) + 0.25f;
	pVertices->z = 0.0f;
	pVertices->color = pxRectDef->ulCol;
	pVertices->tu = pxRectDef->fU1;
	pVertices->tv = pxRectDef->fV2;
//	pVertices->rhw = 1.0f;
	pVertices++; 
	
	//** Second tri **

	xVect.x = pxRectDef->nWidth * 0.5f;
	xVect.y = pxRectDef->nHeight * -0.5f;
	xVect.z = 0.0f;
	VectRotateAboutZ( &xVect, pxRectDef->fRot );

	pVertices->x = (float)( pxRectDef->nX + xVect.x ) + 0.25f;
	pVertices->y = (float)( pxRectDef->nY + xVect.y ) + 0.25f;
	pVertices->z = 0.0f;
	pVertices->color = pxRectDef->ulCol;
	pVertices->tu = pxRectDef->fU2;
	pVertices->tv = pxRectDef->fV1;
//	pVertices->rhw = 1.0f;
	pVertices++;
	
	xVect.x = pxRectDef->nWidth * 0.5f;
	xVect.y = pxRectDef->nHeight * 0.5f;
	xVect.z = 0.0f;
	VectRotateAboutZ( &xVect, pxRectDef->fRot );

	pVertices->x = (float)( pxRectDef->nX + xVect.x  ) + 0.25f;
	pVertices->y = (float)( pxRectDef->nY + xVect.y ) + 0.25f;
	pVertices->z = 0.0f;
	pVertices->color = pxRectDef->ulCol;
	pVertices->tu = pxRectDef->fU2;
	pVertices->tv = pxRectDef->fV2;
//	pVertices->rhw = 1.0f;
	pVertices++;
	
	xVect.x = pxRectDef->nWidth * -0.5f;
	xVect.y = pxRectDef->nHeight * 0.5f;
	xVect.z = 0.0f;
	VectRotateAboutZ( &xVect, pxRectDef->fRot );

	pVertices->x = (float)( pxRectDef->nX + xVect.x ) + 0.25f;
	pVertices->y = (float)( pxRectDef->nY + xVect.y ) + 0.25f;
	pVertices->z = 1.0f;
	pVertices->color = pxRectDef->ulCol;
	pVertices->tu = pxRectDef->fU1;
	pVertices->tv = pxRectDef->fV2;
//	pVertices->rhw = 1.0f;
	pVertices++;

	mnNextTexOverlayVertex += 6;
}


/***************************************************************************
 * Function    : DrawTexturedOverlays
 * Params      : 
 * Returns     :
 * Description : 
 ***************************************************************************/
void DrawTexturedOverlays( int nLayer )
{
int		nLoop;
TEXTURED_RECT_DEF*	pxRectDefinition;
FLATVERTEX*	pVertices;

	for ( nLoop = 0; nLoop < MAX_DIFFERENT_TEXTURED_OVERLAYS; nLoop++ )
	{
		pxRectDefinition = maxOverlayData[ nLoop ].pxRectsInOverlay;
		// If there are any rects in this overlay
		if ( pxRectDefinition != NULL )
		{
			if ( maxOverlayData[ nLoop ].nLayerNum == nLayer )
			{
				mnCurrentTexOverlayRenderTexture = nLoop;
				mnCurrentRenderType = maxOverlayData[ nLoop ].nRenderType;
				if( FAILED( mpCurrentTexOverlayVertexBuffer->Lock( 0, 0, (VERTEX_LOCKTYPE)&pVertices, D3DLOCK_DISCARD ) ) )
				{
					return;
				}
				// Add all the vertices required to draw all the shapes in this overlay
				while ( pxRectDefinition != NULL )
				{
					switch ( pxRectDefinition->nType )
					{
					case TEX_OVLY_RECT:
						AddTexturedRectVertices( &pVertices, pxRectDefinition );
						break;
					case TEX_OVLY_TRI:
						AddTexturedTriVertices( &pVertices, pxRectDefinition );
						break;
					case TEX_OVLY_SPRITE:
						AddSpriteVertices( &pVertices, pxRectDefinition );
						break;

					}
				
					pxRectDefinition = (TEXTURED_RECT_DEF*)( pxRectDefinition->pNext );
				}

				mpCurrentTexOverlayVertexBuffer->Unlock();
				RenderTexOverlays();
		
				maxOverlayData[ nLoop ].pxRectsInOverlay = NULL;
			}
		}
	}
	
	mpxNextFreeRect = mpxRectBuffer;
	mnNumActiveTexOverlays = 0;
}





/***************************************************************************
 * Function    : TexturedOverlayCreate
 * Params      : The texture that will be used for all rectangles on this overlay
 * Returns     : The number of the overlay for which all rectangles will use this texture
 * Description : See what feckin happens when i fill in this bit?
 ***************************************************************************/
int	TexturedOverlayCreate( int nLayer, LPGRAPHICSTEXTURE pxTexture )
{
int	nRet;

	nRet = mnNumActiveTexOverlays++;

	if (  mnNumActiveTexOverlays == MAX_DIFFERENT_TEXTURED_OVERLAYS )
	{
		PANIC_IF( TRUE,"Too many different textured overlays added" );
		mnNumActiveTexOverlays--;
		return( nRet );
	}

	maxOverlayData[ nRet ].pxOverlayTexture = pxTexture;
	maxOverlayData[ nRet ].nRenderType = RENDER_TYPE_NORMAL;
	maxOverlayData[ nRet ].nLayerNum = nLayer;
	return( nRet );
}


/***************************************************************************
 * Function    : InterfaceTexturedTri
 * Params      : 
 * Returns     :
 * Description : 
 ***************************************************************************/
INTERFACE_API void InterfaceTexturedTri( int nOverlayNum, int* pnVerts, float* pfUVs, ulong ulCol )
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


/***************************************************************************
 * Function    : InterfaceTexturedRect
 * Params      : 
 * Returns     :
 * Description : 
 ***************************************************************************/
INTERFACE_API void InterfaceTexturedRect( int nOverlayNum, int nX, int nY, int nWidth, int nHeight, ulong ulCol, float fU, float fV, float fU2, float fV2 )
{
TEXTURED_RECT_DEF* pxRectDef;

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

		pxRectDef->nX += mnInterfaceDrawX;
		pxRectDef->nY += mnInterfaceDrawY;
		pxRectDef->nX2 += mnInterfaceDrawX;
		pxRectDef->nY2 += mnInterfaceDrawY;
	}
}


/***************************************************************************
 * Function    : InterfaceDrawSprite
 * Params      : 
 * Returns     :
 * Description : 
 ***************************************************************************/
INTERFACE_API void InterfaceSprite( int nOverlayNum, int nX, int nY, float fTexGrid, int nTexGridNum, ulong ulCol, float fRotAngle, float fScale )
{
TEXTURED_RECT_DEF*	pxRectDef;
D3DSURFACE_DESC		xSurface;
int		nTexMod;

	if ( nOverlayNum >= 0 )
	{
		if ( maxOverlayData[ nOverlayNum ].pxOverlayTexture != NULL )
		{
			pxRectDef = TexOverlayGetNextRect( nOverlayNum );
			if ( pxRectDef != NULL )
			{
				maxOverlayData[ nOverlayNum ].pxOverlayTexture->GetLevelDesc( 0, &xSurface );

				pxRectDef->nType = TEX_OVLY_SPRITE;
				pxRectDef->nX = nX;
				pxRectDef->nY = nY;

				if ( fTexGrid <= 0.0f )
				{
					fTexGrid = 1.0f;
				}

				pxRectDef->nWidth = (int)( xSurface.Width * fTexGrid  );
				pxRectDef->nHeight = (int)( xSurface.Height * fTexGrid );

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

				pxRectDef->nX += mnInterfaceDrawX;
				pxRectDef->nY += mnInterfaceDrawY;
				pxRectDef->nX2 += mnInterfaceDrawX;
				pxRectDef->nY2 += mnInterfaceDrawY;
			}
		}
	}
}

/***************************************************************************
 * Function    : FreeTextRectLists
 * Params      : 
 * Returns     :
 * Description : 
 ***************************************************************************/
void FreeTextRectLists( void )
{
int		nLoop;

	if ( mpxRectBuffer != NULL )
	{
		free( mpxRectBuffer );
		mpxRectBuffer = NULL;
	}
	for ( nLoop = 0; nLoop <MAX_DIFFERENT_TEXTURED_OVERLAYS; nLoop++ )
	{
		maxOverlayData[ nLoop ].pxRectsInOverlay = NULL;
	}
	mpxNextFreeRect = NULL;

}


/***************************************************************************
 * Function    : InitTextRectLists
 * Params      : 
 * Returns     :
 * Description : 
 ***************************************************************************/
void InitTextRectLists( void )
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
		PANIC_IF( TRUE,"Couldnt allocate memory for textured overlay buffer" );
	}
}

/***************************************************************************
 * Function    : InitTexturedOverlays
 * Params      : 
 * Returns     :
 * Description : 
 ***************************************************************************/
void InitTexturedOverlays( void )
{
int		nLoop;

	if ( mboInterfaceTexturesFirstInitialise == TRUE )
	{
		ZeroMemory( maxInternalTextures, sizeof( INTERNAL_TEXTURES ) * MAX_INTERNAL_TEXTURES_LOADED );
		mboInterfaceTexturesFirstInitialise = FALSE;
	}
/*	else
	{
	int		nLoop;
		
		for( nLoop = 0; nLoop < MAX_INTERNAL_TEXTURES_LOADED; nLoop++ )
		{
			if ( maxInternalTextures[ nLoop ].pD3DTexture )
			{
				maxInternalTextures[ nLoop ].pD3DTexture->Release();
				maxInternalTextures[ nLoop ].pD3DTexture = InterfaceLoadTextureDX( maxInternalTextures[ nLoop ].acFilename, 1, 1 );
			}
		}
	}
*/

	mnNumActiveTexOverlays = 0;

	for ( nLoop = 0; nLoop < MAX_DIFFERENT_TEXTURED_OVERLAYS; nLoop++ )
	{
		maxOverlayData[nLoop].pxOverlayTexture = NULL;
	}
	InitTextRectLists();

	if ( mpTexOverlayVertexBuffer1 == NULL )
	{
		// Create the vertex buffer.
		if( FAILED( InterfaceInternalDXCreateVertexBuffer( MAX_TEX_OVERLAY_VERTICES * sizeof(FLATVERTEX),
													  D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC, D3DFVF_FLATVERTEX,
													  &mpTexOverlayVertexBuffer1 ) ) )
		{
			PANIC_IF( TRUE, "Couldnt create Textured Overlay Vertex buffer");
			return;
		}

		// Create the vertex buffer.
		if( FAILED( InterfaceInternalDXCreateVertexBuffer( MAX_TEX_OVERLAY_VERTICES * sizeof(FLATVERTEX),
													  D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC, D3DFVF_FLATVERTEX,
													  &mpTexOverlayVertexBuffer2 ) ) )
		{
			PANIC_IF( TRUE, "Couldnt create Textured Overlay Vertex buffer");
			return;
		}
	}

	mpCurrentTexOverlayVertexBuffer = mpTexOverlayVertexBuffer1;

}


/***************************************************************************
 * Function    : FreeTexturedOverlays
 * Params      : 
 * Returns     :
 * Description : 
 ***************************************************************************/
void FreeTexturedOverlays( void )
{
int	nLoop;

    if( mpTexOverlayVertexBuffer1 != NULL )
	{
        mpTexOverlayVertexBuffer1->Release();
		mpTexOverlayVertexBuffer1 = NULL;
	}
    if( mpTexOverlayVertexBuffer2 != NULL )
	{
        mpTexOverlayVertexBuffer2->Release();
		mpTexOverlayVertexBuffer2 = NULL;
	}

	mpCurrentTexOverlayVertexBuffer = NULL;

	for ( nLoop = 0; nLoop < MAX_DIFFERENT_TEXTURED_OVERLAYS; nLoop++ )
	{
		maxOverlayData[ nLoop ].pxOverlayTexture = NULL;
	}

	FreeTextRectLists();

}

int		InterfaceGetNewInternalTextureHandle( void )
{
int		nLoop = 1;
	
	do
	{
		if ( maxInternalTextures[ nLoop ].pD3DTexture == NULL )
		{
			return( nLoop );
		}
		nLoop++;
	} while ( nLoop < MAX_INTERNAL_TEXTURES_LOADED );

	return( NOTFOUND );
}

void	InterfaceTexturedOverlaysSetLimit( int nState )
{
	if ( ( nState > 0 ) &&
		 ( nState != mnTexRectMaxBufferSize ) )
	{
		mnTexRectMaxBufferSize = nState;
		InitTextRectLists();
	}
}


INTERFACE_API int	InterfaceGetTextureSize( int nTextureHandle, int* pnW, int* pnH )
{
	*pnW = 0;
	*pnH = 0;
	if ( nTextureHandle != NOTFOUND )
	{
	D3DSURFACE_DESC		xSurface;

		if ( maxInternalTextures[ nTextureHandle ].pD3DTexture != NULL )
		{
			maxInternalTextures[ nTextureHandle ].pD3DTexture->GetLevelDesc( 0, &xSurface );
			*pnW = xSurface.Width;
			*pnH = xSurface.Height;
			return( 1 );
		}
	}
	return( 0 );
}


INTERFACE_API int  InterfaceCreateUntexturedOverlay( int nLayer )
{
	return( TexturedOverlayCreate( nLayer, NOTFOUND ) );
}


INTERFACE_API int  InterfaceCreateNewTexturedOverlay( int nLayer, int nTextureHandle )
{
	if ( nTextureHandle != NOTFOUND )
	{
		nTextureHandle %= MAX_INTERNAL_TEXTURES_LOADED;
		if ( maxInternalTextures[ nTextureHandle ].pD3DTexture != NULL )
		{
			return( TexturedOverlayCreate( nLayer, maxInternalTextures[ nTextureHandle ].pD3DTexture ) );
		}
	}
	return( NOTFOUND );
}

INTERFACE_API void InterfaceReleaseTexture( int nTextureHandle )
{
	if ( nTextureHandle != NOTFOUND )
	{
		nTextureHandle %= MAX_INTERNAL_TEXTURES_LOADED;
		if ( maxInternalTextures[ nTextureHandle ].pD3DTexture != NULL )
		{ 
			if ( maxInternalTextures[ nTextureHandle ].nRefCount > 1 )
			{
				maxInternalTextures[ nTextureHandle ].nRefCount--;
			}
			else
			{
				maxInternalTextures[ nTextureHandle ].pD3DTexture->Release();
				maxInternalTextures[ nTextureHandle ].pD3DTexture = NULL;
				maxInternalTextures[ nTextureHandle ].acFilename[0] = 0;
				maxInternalTextures[ nTextureHandle ].ulLastTouched = 0;
				maxInternalTextures[ nTextureHandle ].nRefCount = 0;
			}
		}
	}
}




INTERFACE_API int  InterfaceGetTextureFromFileInMem( const char* szFilename, unsigned char* pbMem, int nMemSize, int nFlags )
{
int		nHandle;

	nHandle = InterfaceGetNewInternalTextureHandle();

	if ( nHandle != NOTFOUND )
	{
		if ( (nFlags & 0x1) == 0 )
		{
			maxInternalTextures[ nHandle ].pD3DTexture = InterfaceLoadTextureDXFromFileInMem( szFilename, pbMem, nMemSize, 1, 1 );
		}
		else
		{
			maxInternalTextures[ nHandle ].pD3DTexture = InterfaceLoadTextureDXFromFileInMem( szFilename, pbMem, nMemSize, 0, 0 );
		}

		// Load failed
		if ( !maxInternalTextures[ nHandle ].pD3DTexture )
		{
			return( NOTFOUND );
		}
		maxInternalTextures[ nHandle ].ulLastTouched = GetTickCount();
		maxInternalTextures[ nHandle ].nRefCount = 1;

		if ( strlen( szFilename ) < 127 ) 
		{
			strcpy( maxInternalTextures[ nHandle ].acFilename, szFilename );
		}
	}

	return( nHandle );
}


INTERFACE_API BYTE*	InterfaceLockTexture( int nTextureHandle, int* pnPitch, int* pnFormat, int nFlags )
{
BYTE*	pbImageData = NULL;

	*pnPitch = 0;
	*pnFormat = 0;

	if ( nTextureHandle >= 0 )
	{
		if ( maxInternalTextures[ nTextureHandle ].pD3DTexture )
		{
		D3DLOCKED_RECT		xLockedRect;
		D3DSURFACE_DESC		xSurfDesc;
		HRESULT		hRet;

			maxInternalTextures[ nTextureHandle ].pD3DTexture->GetLevelDesc( 0, &xSurfDesc );
			*pnFormat = xSurfDesc.Format;
			hRet = maxInternalTextures[ nTextureHandle ].pD3DTexture->LockRect( 0, &xLockedRect, NULL, 0 );
			if ( hRet ==  D3D_OK )
			{
				*pnPitch = xLockedRect.Pitch;
				pbImageData = (BYTE*)xLockedRect.pBits;
			}
			else
			{
				pnPitch = 0;
				pbImageData = NULL;
			}
		}
	}
	return( pbImageData );
}

INTERFACE_API void	InterfaceTextureGetColourAtPoint( int nTextureHandle, byte* pbLockedTextureData, int nPitch, int nFormat, int x, int y, float* pfRed, float* pfGreen, float* pfBlue, float* pfAlpha )
{
byte*		pbRow = pbLockedTextureData + (y * nPitch);
int		nR = 0;
int		nG = 0;
int		nB = 0;
int		nA = 0;
ulong	ulCol;
ushort	uwColVal;

	switch( nFormat )
	{
	case D3DFMT_R8G8B8:			// 3 bytes per pixel
		nR = *( pbRow + (x*3) );
		nG = *( pbRow + (x*3) + 1 );
		nB = *( pbRow + (x*3) + 2 );
		break;
	case D3DFMT_X1R5G5B5:
	case D3DFMT_A1R5G5B5:
		uwColVal = *( (ushort*)( pbRow + (x*2) ) );
		nR = ( ( uwColVal >> 10 ) & 0x1F) << 3;
		nG = ( ( uwColVal >> 5 ) & 0x1F ) << 3;
		nB = ( uwColVal & 0x1F ) << 3;
		break;
	case D3DFMT_R5G6B5:           // 2 bytes per pixel
		uwColVal = *( (ushort*)( pbRow + (x*2) ) );
		nR = ( uwColVal >> 11 ) << 3;
		nG = ( ( uwColVal >> 5 ) & 0x3F ) << 2;
		nB = ( uwColVal & 0x1F ) << 3;
		break;
	case D3DFMT_A8R8G8B8:		// 4 bytes per pixel
	case D3DFMT_X8R8G8B8:
		ulCol = *( (ulong*)( pbRow + (x * 4) ) );
		nA = ((ulCol >> 24) & 0xFF);
		nR = ((ulCol >> 16) & 0xFF);
		nG = ((ulCol >> 8) & 0xFF);
		nB = (ulCol & 0xFF);
		break;
	case D3DFMT_A8:
	case D3DFMT_P8:
	case D3DFMT_L8:
		nR = *( pbRow + x );
		nG = nR;
		nB = nR;
		break;
	}
/*
    D3DFMT_A4R4G4B4             = 26,
    D3DFMT_R3G3B2               = 27,
    D3DFMT_A8R3G3B2             = 29,
    D3DFMT_X4R4G4B4             = 30,
    D3DFMT_A2B10G10R10          = 31,
    D3DFMT_G16R16               = 34,

    D3DFMT_A8P8                 = 40,
    D3DFMT_A8L8                 = 51,
    D3DFMT_A4L4                 = 52,

    D3DFMT_V8U8                 = 60,
    D3DFMT_L6V5U5               = 61,
    D3DFMT_X8L8V8U8             = 62,
    D3DFMT_Q8W8V8U8             = 63,
    D3DFMT_V16U16               = 64,
    D3DFMT_W11V11U10            = 65,
    D3DFMT_A2W10V10U10          = 67,
*/
	*pfRed = (float)(nR) / 255.0f;
	*pfGreen = (float)(nG) / 255.0f;
	*pfBlue = (float)(nB) / 255.0f;
	*pfAlpha = (float)(nA) / 255.0f;

}


INTERFACE_API void	InterfaceUnlockTexture( int nTextureHandle )
{
	if ( nTextureHandle >= 0 )
	{
		if ( maxInternalTextures[ nTextureHandle ].pD3DTexture )
		{
			maxInternalTextures[ nTextureHandle ].pD3DTexture->UnlockRect( 0 );
		}
	}
}

int			InterfaceFindTexture( const char* szFilename )
{
int		nLoop = 0;

	do
	{
		if ( maxInternalTextures[ nLoop ].pD3DTexture != NULL )
		{
			if ( _strnicmp( maxInternalTextures[ nLoop ].acFilename, szFilename, 127 ) == 0 )
			{
				return( nLoop );
			}
		}
		nLoop++;
	} while ( nLoop < MAX_INTERNAL_TEXTURES_LOADED );
	return( NOTFOUND );
}

INTERFACE_API int InterfaceGetTextureInternal( const char* szFilename, int nFlags, int nArchiveHandle )
{
int		nHandle;

	// Look for this filename if its already loaded
	nHandle = InterfaceFindTexture( szFilename );
	if ( nHandle >= 0 )
	{
		// If found, add a reference to this texture
		maxInternalTextures[ nHandle ].nRefCount++;
		return( nHandle );
	}

	nHandle = InterfaceGetNewInternalTextureHandle();

	if ( nHandle != NOTFOUND )
	{
		switch( nFlags )
		{
		case 0:			// No mipping
		case 2:			// Used when a texture needs to be lockable
		case 4:			// Used when a texture needs to be lockable
		default:
			if ( nArchiveHandle > 0 )
			{
				maxInternalTextures[ nHandle ].pD3DTexture = InterfaceLoadTextureFromArchiveDX( szFilename, 1, 1, nArchiveHandle );
			}
			else
			{
				maxInternalTextures[ nHandle ].pD3DTexture = InterfaceLoadTextureDX( szFilename, 1, 1 );
			}
			break;
		case 1:			// Mipmaps
			if ( nArchiveHandle > 0 )
			{
				maxInternalTextures[ nHandle ].pD3DTexture = InterfaceLoadTextureFromArchiveDX( szFilename, 0, 0, nArchiveHandle );
			}
			else
			{
				maxInternalTextures[ nHandle ].pD3DTexture = InterfaceLoadTextureDX( szFilename, 0, 0 );
			}
			break;
		case 3:		// feck knows
			if ( nArchiveHandle > 0 )
			{
				maxInternalTextures[ nHandle ].pD3DTexture = InterfaceLoadTextureFromArchiveDX( szFilename, 2, 0xFF, nArchiveHandle );
			}
			else
			{
				maxInternalTextures[ nHandle ].pD3DTexture = InterfaceLoadTextureDX( szFilename, 2, 0xFF );
			}
			break;
		}

		// Load failed
		if ( !maxInternalTextures[ nHandle ].pD3DTexture )
		{
			return( NOTFOUND );
		}
		maxInternalTextures[ nHandle ].nRefCount = 1;
		maxInternalTextures[ nHandle ].ulLastTouched = GetTickCount();

		if ( strlen( szFilename ) < 127 ) 
		{
			strcpy( maxInternalTextures[ nHandle ].acFilename, szFilename );
		}
	}

	return( nHandle );
}


INTERFACE_API int InterfaceGetTexture( const char* szFilename, int nFlags )
{
	return( InterfaceGetTextureInternal( szFilename, nFlags, NOTFOUND ) );
}


LPGRAPHICSTEXTURE	InterfaceGetBlankPlatformTexture( int nWidth, int nHeight, int Mode, int* pnPitch )
{
LPGRAPHICSTEXTURE	pxTexture;

	InterfaceInternalDXCreateTexture( nWidth, nHeight, 1, 0, FORMAT_A8R8G8B8, &pxTexture );
	return( pxTexture );
}


int	InterfaceCreateBlankTexture( int nWidth, int nHeight, int Mode)
{
int		nHandle;
int		nPitch;

	nHandle = InterfaceGetNewInternalTextureHandle();

	if ( nHandle != NOTFOUND )
	{
		maxInternalTextures[ nHandle ].pD3DTexture = InterfaceGetBlankPlatformTexture( nWidth, nHeight, Mode, &nPitch );
		maxInternalTextures[ nHandle ].nRefCount = 1;
		strcpy( maxInternalTextures[ nHandle ].acFilename, "gentex" );
	}
	return( nHandle );
}
