//#if 0

#ifdef USING_OPENGL
#include <windows.h>					// For OpenGL

#include "gl/glew.h"
#include "gl/gl.h"
#include "../../OpenGL/InterfaceInternalsGL.h"
#include "../../OpenGL/VertexBufferGL.h"
#include "../../OpenGL/TextureLoader/InterfaceTextureManager.h"
#else
#include "../../DirectX/InterfaceInternalsDX.h"
#endif

#include <StandardDef.h>
#include <Interface.h>
#include <Engine.h>
#include <EngineMaths.h>

#include "../InterfaceUtil.h"
#include "../InterfaceInstance.h"

#include "Overlays.h"
#include "TexturedOverlays.h"


LPGRAPHICSTEXTURE	InterfaceGetBlankPlatformTexture( int nWidth, int nHeight, int Mode, int* pnPitch )
{
LPGRAPHICSTEXTURE	pxTexture;

	InterfaceInternalDXCreateTexture( nWidth, nHeight, 1, 0, FORMAT_A8R8G8B8, &pxTexture );
	return( pxTexture );
}
void		InterfaceRotateVectAboutZ( VECT* pVec, float fAngle )
{
ENGINEMATRIX	xMatRotZ;
ENGINEMATRIX*	pxMatrix = &xMatRotZ;
 float	fNorm;
VECT	xTemp;
VECT*	pSrc = pVec;
VECT*	pOut = pVec;

	memset( pxMatrix, 0, sizeof( ENGINEMATRIX ) );
	pxMatrix->_11 = 1.0f;
	pxMatrix->_22 = 1.0f;
	pxMatrix->_33 = 1.0f;
	pxMatrix->_44 = 1.0f;
	pxMatrix->_11 = cosf( fAngle );
	pxMatrix->_22 = pxMatrix->_11;
	pxMatrix->_12 = sinf( fAngle );
	pxMatrix->_21 = 0.0f - pxMatrix->_12;
	pxMatrix->_33 = 1.0f;
	
    fNorm = pxMatrix->m[0][3] * pSrc->x + pxMatrix->m[1][3] * pSrc->y + pxMatrix->m[2][3] *pSrc->z + pxMatrix->m[3][3];

	if ( fNorm != 0.0f )
	{
		xTemp.x = (pxMatrix->m[0][0] * pSrc->x + pxMatrix->m[1][0] * pSrc->y + pxMatrix->m[2][0] * pSrc->z + pxMatrix->m[3][0]) / fNorm;
		xTemp.y = (pxMatrix->m[0][1] * pSrc->x + pxMatrix->m[1][1] * pSrc->y + pxMatrix->m[2][1] * pSrc->z + pxMatrix->m[3][1]) / fNorm;
		xTemp.z = (pxMatrix->m[0][2] * pSrc->x + pxMatrix->m[1][2] * pSrc->y + pxMatrix->m[2][2] * pSrc->z + pxMatrix->m[3][2]) / fNorm;
		*pOut = xTemp;
    }
    else
    {
		pOut->x = 0.0f;
		pOut->y = 0.0f;
		pOut->z = 0.0f;
    }
}

//------------------------------------------------------------------------------------------------------------

void TexturedOverlays::SetRenderType( int nOverlayNum, INTF_RENDER_TYPES nRenderType )
{
	if ( nOverlayNum >= 0 )
	{
		maxOverlayData[ nOverlayNum ].nRenderType = nRenderType;
	}
}


void	TexturedOverlays::SetTextureAsCurrent( int nTextureHandle )
{
	nTextureHandle %= MAX_INTERNAL_TEXTURES_LOADED;
	if ( maxInternalTextures[ nTextureHandle ].pTexture != NULL )
	{
		mpInterfaceD3DDevice->SetTexture( 0, maxInternalTextures[ nTextureHandle ].pTexture );
	}
}

TEXTURED_RECT_DEF*	TexturedOverlays::GetNextRect( int nOverlayNum )
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
//				PANIC_IF( TRUE, "Too many textured overlay shapes added.");
				mpxNextFreeRect = NULL;
			}
		}
	}
	return( pxRet );
}

void TexturedOverlays::FlushTexOverlayBuffer( void )
{
int		nDrawHowMany;

	nDrawHowMany = (mnNextTexOverlayVertex / 3);

#ifdef USING_OPENGL
	InterfaceSetTextureAsCurrent( maxOverlayData[ mnCurrentTexOverlayRenderTexture ].hOverlayTexture );
    mpCurrentTexOverlayVertexBuffer->Render( nDrawHowMany );

#else
	if ( nDrawHowMany > 0 )
	{
		EngineSetVertexFormat( VERTEX_FORMAT_FLATVERTEX );
		EngineEnableZTest( FALSE );
		EngineEnableZWrite( FALSE );
		EngineEnableLighting( FALSE );
		EngineEnableBlend( TRUE );
		EngineEnableFog( FALSE );

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
			mpInterfaceD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
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

		// Enable alpha testing (skips pixels with less than a certain alpha.)
		EngineEnableAlphaTest( TRUE );

		if ( maxOverlayData[ mnCurrentTexOverlayRenderTexture ].pTexture != NULL )
		{
			InterfaceSetTextureAsCurrentDirect( maxOverlayData[ mnCurrentTexOverlayRenderTexture ].pTexture );
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
		case RENDER_TYPE_ADDITIVE_NOCLAMP:
		case RENDER_TYPE_NORMAL_NOCLAMP:
			// Turn clamping back on once we're done
			mpInterfaceD3DDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
			mpInterfaceD3DDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );
			break;
		}
	}
#endif		// ifndef OPENGL

	mnNextTexOverlayVertex = 0;
	if ( mpCurrentTexOverlayVertexBuffer == mpTexOverlayVertexBuffer1 )
	{
		mpCurrentTexOverlayVertexBuffer = mpTexOverlayVertexBuffer2;
	}
	else
	{
		mpCurrentTexOverlayVertexBuffer = mpTexOverlayVertexBuffer1;
	} 
} 



/***************************************************************************
 * Function    : AddTexturedRectVertices
 * Params      : 
 * Returns     :
 * Description : 
 ***************************************************************************/
void TexturedOverlays::AddTexturedTriVertices( FLATVERTEX** ppVertices, TEXTURED_RECT_DEF* pxRectDef )
{
FLATVERTEX*		pVertices;

	if ( mnNextTexOverlayVertex >= ( MAX_TEX_OVERLAY_VERTICES - 6 ) )
	{
		mpCurrentTexOverlayVertexBuffer->Unlock();
		FlushTexOverlayBuffer();
#ifdef USING_OPENGL
		*ppVertices = mpCurrentTexOverlayVertexBuffer->Lock();
#else
		if( FAILED( mpCurrentTexOverlayVertexBuffer->Lock( 0, 0, (VERTEX_LOCKTYPE)ppVertices, D3DLOCK_DISCARD ) ) )
		{
			PANIC_IF(TRUE, "Vertex lock failed in addtexoverlay" );
		}
#endif
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
void TexturedOverlays::AddTexturedRectVertices( FLATVERTEX** ppVertices, TEXTURED_RECT_DEF* pxRectDef )
{
FLATVERTEX*		pVertices;

	if ( mnNextTexOverlayVertex >= ( MAX_TEX_OVERLAY_VERTICES - 6 ) )
	{
		mpCurrentTexOverlayVertexBuffer->Unlock();
		FlushTexOverlayBuffer();
#ifdef USING_OPENGL
		*ppVertices = mpCurrentTexOverlayVertexBuffer->Lock();
#else
		if( FAILED( mpCurrentTexOverlayVertexBuffer->Lock( 0, 0, (VERTEX_LOCKTYPE)ppVertices, D3DLOCK_DISCARD ) ) )
		{
			PANIC_IF(TRUE, "Vertex lock failed in addtexoverlay" );
		}
#endif
	}

	pVertices = *( ppVertices );
	pVertices += mnNextTexOverlayVertex;

	/** First tri **/
	pVertices->x = (float)( pxRectDef->nX ) + 0.25f;
	pVertices->y = (float)( pxRectDef->nY ) + 0.25f;
	pVertices->z = 1.0f;
 	pVertices->color = pxRectDef->ulCol;
	pVertices->tu = pxRectDef->fU1;
#ifdef USING_OPENGL
	pVertices->tv = 1.0f - pxRectDef->fV1;
#else
	pVertices->tv = pxRectDef->fV1;
#endif
	pVertices++;

	pVertices->x = (float)( pxRectDef->nX + pxRectDef->nWidth ) + 0.25f;
	pVertices->y = (float)( pxRectDef->nY ) + 0.25f;
	pVertices->z = 1.0f;
	pVertices->color = pxRectDef->ulCol;
	pVertices->tu = pxRectDef->fU2;
#ifdef USING_OPENGL
	pVertices->tv = 1.0f - pxRectDef->fV1;
#else
	pVertices->tv = pxRectDef->fV1;
#endif
//	pVertices->rhw = 1.0f;
	pVertices++;

	pVertices->x = (float)( pxRectDef->nX ) + 0.25f;
	pVertices->y = (float)( pxRectDef->nY + pxRectDef->nHeight ) + 0.25f;
	pVertices->z = 1.0f;
	pVertices->color = pxRectDef->ulCol2;
	pVertices->tu = pxRectDef->fU1;
#ifdef USING_OPENGL
	pVertices->tv = 1.0f - pxRectDef->fV2;
#else
	pVertices->tv = pxRectDef->fV2;
#endif
//	pVertices->rhw = 1.0f;
	pVertices++;
	
	//** Second tri **
	pVertices->x = (float)( pxRectDef->nX + pxRectDef->nWidth ) + 0.25f;
	pVertices->y = (float)( pxRectDef->nY ) + 0.25f;
	pVertices->z = 1.0f;
	pVertices->color = pxRectDef->ulCol;
	pVertices->tu = pxRectDef->fU2;
#ifdef USING_OPENGL
	pVertices->tv = 1.0f - pxRectDef->fV1;
#else
	pVertices->tv = pxRectDef->fV1;
#endif
//	pVertices->rhw = 1.0f;
	pVertices++;
	
	pVertices->x = (float)( pxRectDef->nX + pxRectDef->nWidth ) + 0.25f;
	pVertices->y = (float)( pxRectDef->nY + pxRectDef->nHeight ) + 0.25f;
	pVertices->z = 1.0f;
	pVertices->color = pxRectDef->ulCol2;
	pVertices->tu = pxRectDef->fU2;
#ifdef USING_OPENGL
	pVertices->tv = 1.0f - pxRectDef->fV2;
#else
	pVertices->tv = pxRectDef->fV2;
#endif
//	pVertices->rhw = 1.0f;
	pVertices++;
	
	pVertices->x = (float)( pxRectDef->nX ) + 0.25f;
	pVertices->y = (float)( pxRectDef->nY + pxRectDef->nHeight ) + 0.25f;
	pVertices->z = 1.0f;
	pVertices->color = pxRectDef->ulCol2;
	pVertices->tu = pxRectDef->fU1;
#ifdef USING_OPENGL
	pVertices->tv = 1.0f - pxRectDef->fV2;
#else
	pVertices->tv = pxRectDef->fV2;
#endif
//	pVertices->rhw = 1.0f;
	pVertices++;

	mnNextTexOverlayVertex += 6;
}


void TexturedOverlays::AddSpriteVertices( FLATVERTEX** ppVertices, TEXTURED_RECT_DEF* pxRectDef )
{
FLATVERTEX*		pVertices;
VECT			xVect;

	if ( mnNextTexOverlayVertex >= ( MAX_TEX_OVERLAY_VERTICES - 6 ) )
	{
		mpCurrentTexOverlayVertexBuffer->Unlock();
		FlushTexOverlayBuffer();
#ifdef USING_OPENGL
		*ppVertices = mpCurrentTexOverlayVertexBuffer->Lock();
		if( *ppVertices == NULL )
//		if( FAILED( mpCurrentTexOverlayVertexBuffer->Lock( 0, 0, (VERTEX_LOCKTYPE)ppVertices, D3DLOCK_DISCARD ) ) )
		{
			PANIC_IF(TRUE, "Vertex lock failed in addtexoverlay" );
		}
#else
		if( FAILED( mpCurrentTexOverlayVertexBuffer->Lock( 0, 0, (VERTEX_LOCKTYPE)ppVertices, D3DLOCK_DISCARD ) ) )
		{
			PANIC_IF(TRUE, "Vertex lock failed in addtexoverlay" );
		}
#endif
	}

	pVertices = *( ppVertices );
	pVertices += mnNextTexOverlayVertex;

	xVect.x = pxRectDef->nWidth * -0.5f;
	xVect.y = pxRectDef->nHeight * -0.5f;
	xVect.z = 0.0f;
	InterfaceRotateVectAboutZ( &xVect, pxRectDef->fRot );			// Should optimise this - the vectAboutZ generates a matrix and transforms the whole vec each time.. a much simpler calc could be used
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
	InterfaceRotateVectAboutZ( &xVect, pxRectDef->fRot );		// Should optimise this - the vectAboutZ generates a matrix and transforms the whole vec each time.. a much simpler calc could be used

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
	InterfaceRotateVectAboutZ( &xVect, pxRectDef->fRot );		// Should optimise this - the vectAboutZ generates a matrix and transforms the whole vec each time.. a much simpler calc could be used

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
	InterfaceRotateVectAboutZ( &xVect, pxRectDef->fRot );		// Should optimise this - the vectAboutZ generates a matrix and transforms the whole vec each time.. a much simpler calc could be used

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
	InterfaceRotateVectAboutZ( &xVect, pxRectDef->fRot );

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
	InterfaceRotateVectAboutZ( &xVect, pxRectDef->fRot );

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

void TexturedOverlays::Render( int nLayer )
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

#ifdef USING_OPENGL
				pVertices = mpCurrentTexOverlayVertexBuffer->Lock();
#else
				if( FAILED( mpCurrentTexOverlayVertexBuffer->Lock( 0, 0, (VERTEX_LOCKTYPE)&pVertices, D3DLOCK_DISCARD ) ) )
				{
					return;
				}
#endif
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

#ifdef USING_OPENGL
				// TODO
#else
				mpCurrentTexOverlayVertexBuffer->Unlock();
#endif
				FlushTexOverlayBuffer();
		
				maxOverlayData[ nLoop ].pxRectsInOverlay = NULL;
				if ( maxOverlayData[ nLoop ].hEngineTexture > 0 )
				{
					// Reduce the ref count on the engine texture if we were using it
					EngineReleaseTexture( &maxOverlayData[ nLoop ].hEngineTexture );
					maxOverlayData[ nLoop ].hEngineTexture = NOTFOUND;
				}

			}
		}
	}
	
	mpxNextFreeRect = mpxRectBuffer;
	mnNumActiveTexOverlays = 0;
}

int	TexturedOverlays::CreateOverlay( int nLayer, TEXTURE_HANDLE hTexture )
{
int	nRet;

	nRet = mnNumActiveTexOverlays++;

	if ( mnNumActiveTexOverlays == MAX_DIFFERENT_TEXTURED_OVERLAYS )
	{
		PANIC_IF( TRUE,"Too many different textured overlays added" );
		mnNumActiveTexOverlays--;
		return( nRet );
	}

	if ( hTexture != NOTFOUND )
	{
		maxOverlayData[ nRet ].pTexture = maxInternalTextures[ hTexture ].pTexture;
	}
	else
	{
		maxOverlayData[ nRet ].pTexture = NULL;
	}
	maxOverlayData[ nRet ].nRenderType = RENDER_TYPE_NORMAL;
	maxOverlayData[ nRet ].nLayerNum = nLayer;
	maxOverlayData[ nRet ].hEngineTexture = NOTFOUND;
	return( nRet );
}

int	TexturedOverlays::CreateBlankTexture( int nWidth, int nHeight, int Mode)
{
int		nHandle;
int		nPitch;

	nHandle = GetNewInternalTextureHandle();

	if ( nHandle != NOTFOUND )
	{
		maxInternalTextures[ nHandle ].pTexture = InterfaceGetBlankPlatformTexture( nWidth, nHeight, Mode, &nPitch );
		maxInternalTextures[ nHandle ].nRefCount = 1;
		strcpy( maxInternalTextures[ nHandle ].acFilename, "gentex" );
	}
	return( nHandle );
}

int	TexturedOverlays::CreateOverlayDirect( int nLayer, void* pTexture, int hEngineTexture )
{
int	nRet;

	nRet = mnNumActiveTexOverlays++;

	if ( mnNumActiveTexOverlays == MAX_DIFFERENT_TEXTURED_OVERLAYS )
	{
		PANIC_IF( TRUE,"Too many different textured overlays added" );
		mnNumActiveTexOverlays--;
		return( nRet );
	}

	maxOverlayData[ nRet ].pTexture = (LPGRAPHICSTEXTURE)pTexture;
	maxOverlayData[ nRet ].nRenderType = RENDER_TYPE_NORMAL;
	maxOverlayData[ nRet ].nLayerNum = nLayer;
	maxOverlayData[ nRet ].hEngineTexture = hEngineTexture;
	return( nRet );

}

void TexturedOverlays::AddTri( int nOverlayNum, int* pnVerts, float* pfUVs, uint32 ulCol )
{
TEXTURED_RECT_DEF* pxRectDef;

	pxRectDef = GetNextRect( nOverlayNum );
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

void TexturedOverlays::AddRect( int nOverlayNum, int nX, int nY, int nWidth, int nHeight, uint32 ulCol, float fU, float fV, float fUWidth, float fUHeight )
{
TEXTURED_RECT_DEF* pxRectDef;

	if ( nOverlayNum < 0 ) return;

	pxRectDef = GetNextRect( nOverlayNum );
	if ( pxRectDef != NULL )
	{
		pxRectDef->nType = TEX_OVLY_RECT;
		pxRectDef->nX = nX;
		pxRectDef->nY = nY;
		pxRectDef->nWidth = nWidth;
		pxRectDef->nHeight = nHeight;
		pxRectDef->ulCol = ulCol;
		pxRectDef->ulCol2 = ulCol;
		pxRectDef->fU1 = fU;
		pxRectDef->fV1 = fV;
		pxRectDef->fU2 = fUWidth;
		pxRectDef->fV2 = fUHeight;

#ifndef USING_OPENGL
		pxRectDef->nX += mnInterfaceDrawX;
		pxRectDef->nY += mnInterfaceDrawY;
		pxRectDef->nX2 += mnInterfaceDrawX;
		pxRectDef->nY2 += mnInterfaceDrawY;
#endif
	}
}


void	TexturedOverlays::AddRectShaded( int nOverlayNum, int nX, int nY, int nWidth, int nHeight, uint32 ulCol, uint32 ulCol2, float fU, float fV, float fUWidth, float fUHeight )
{
TEXTURED_RECT_DEF* pxRectDef;

	if ( nOverlayNum < 0 ) return;

	pxRectDef = GetNextRect( nOverlayNum );
	if ( pxRectDef != NULL )
	{
		pxRectDef->nType = TEX_OVLY_RECT;
		pxRectDef->nX = nX;
		pxRectDef->nY = nY;
		pxRectDef->nWidth = nWidth;
		pxRectDef->nHeight = nHeight;
		pxRectDef->ulCol = ulCol;
		pxRectDef->ulCol2 = ulCol2;
		pxRectDef->fU1 = fU;
		pxRectDef->fV1 = fV;
		pxRectDef->fU2 = fUWidth;
		pxRectDef->fV2 = fUHeight;

#ifndef USING_OPENGL
		pxRectDef->nX += mnInterfaceDrawX;
		pxRectDef->nY += mnInterfaceDrawY;
		pxRectDef->nX2 += mnInterfaceDrawX;
		pxRectDef->nY2 += mnInterfaceDrawY;
#endif
	}
}


int  TexturedOverlays::GetTextureFromFileInMem( const char* szFilename, unsigned char* pbMem, int nMemSize, int nFlags )
{
int		nHandle;

	nHandle = GetNewInternalTextureHandle();

	if ( nHandle != NOTFOUND )
	{
		if ( (nFlags & 0x1) == 0 )
		{
			maxInternalTextures[ nHandle ].pTexture = InterfaceLoadTextureDXFromFileInMem( szFilename, pbMem, nMemSize, 1, 1 );
		}
		else
		{
			maxInternalTextures[ nHandle ].pTexture = InterfaceLoadTextureDXFromFileInMem( szFilename, pbMem, nMemSize, 0, 0 );
		}

		// Load failed
		if ( !maxInternalTextures[ nHandle ].pTexture )
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

void TexturedOverlays::AddSprite( int nOverlayNum, int nX, int nY, float fTexGrid, int nTexGridNum, uint32 ulCol, float fRotAngle, float fScale )
{
#ifdef USING_OPENGL
		// TODO!!
#else
TEXTURED_RECT_DEF*	pxRectDef;
int		nTexMod;

	if ( nOverlayNum >= 0 )
	{
		if ( maxOverlayData[ nOverlayNum ].pTexture != NULL )
		{
		int		nTexW;
		int		nTexH;

			pxRectDef = GetNextRect( nOverlayNum );
			if ( pxRectDef != NULL )
			{
			D3DSURFACE_DESC		xSurface;

				maxOverlayData[ nOverlayNum ].pTexture->GetLevelDesc( 0, &xSurface );
				nTexW = xSurface.Width;
				nTexH = xSurface.Height;

				pxRectDef->nType = TEX_OVLY_SPRITE;
				pxRectDef->nX = nX;
				pxRectDef->nY = nY;

				if ( fTexGrid <= 0.0f )
				{
					fTexGrid = 1.0f;
				}

				pxRectDef->nWidth = (int)( nTexW * fTexGrid  );
				pxRectDef->nHeight = (int)( nTexH * fTexGrid );
			 
				if ( ulCol == 0 ) ulCol = 0xFFFFFFFF;   // mmmm 
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
#endif
}

void TexturedOverlays::ReleaseTexture( int nTextureHandle )
{
	if ( nTextureHandle != NOTFOUND )
	{
		nTextureHandle %= MAX_INTERNAL_TEXTURES_LOADED;
#ifdef USING_OPENGL
			// TODO 

#else
		if ( maxInternalTextures[ nTextureHandle ].pTexture != NULL )
		{ 
			if ( maxInternalTextures[ nTextureHandle ].nRefCount > 1 )
			{
				maxInternalTextures[ nTextureHandle ].nRefCount--;
			}
			else
			{
				maxInternalTextures[ nTextureHandle ].pTexture->Release();
				maxInternalTextures[ nTextureHandle ].pTexture = NULL;
				maxInternalTextures[ nTextureHandle ].acFilename[0] = 0;
				maxInternalTextures[ nTextureHandle ].ulLastTouched = 0;
				maxInternalTextures[ nTextureHandle ].nRefCount = 0;
			}
		}
	}
#endif
}

int	TexturedOverlays::GetTextureSize( int nTextureHandle, int* pnW, int* pnH )
{
	*pnW = 0;
	*pnH = 0;
	if ( nTextureHandle != NOTFOUND )
	{
	D3DSURFACE_DESC		xSurface;

		if ( maxInternalTextures[ nTextureHandle ].pTexture != NULL )
		{
			maxInternalTextures[ nTextureHandle ].pTexture->GetLevelDesc( 0, &xSurface );
			*pnW = xSurface.Width;
			*pnH = xSurface.Height;
			return( 1 );
		}
	}
	return( 0 );
}

void TexturedOverlays::Shutdown( void )
{
int	nLoop;
#ifdef USING_OPENGL
			// TODO 

#else
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
#endif
	for ( nLoop = 0; nLoop < MAX_DIFFERENT_TEXTURED_OVERLAYS; nLoop++ )
	{
		maxOverlayData[ nLoop ].pTexture = NULL;
	}

	FreeTextRectLists();

}


/***************************************************************************
 * Function    : FreeTextRectLists
  ***************************************************************************/
void TexturedOverlays::FreeTextRectLists( void )
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
 ***************************************************************************/
void TexturedOverlays::InitTextRectLists( void )
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
#ifndef USING_OPENGL
		PANIC_IF( TRUE,"Couldnt allocate memory for textured overlay buffer" );
#endif
	}
}

void	TexturedOverlays::ExportTexture( int nTextureHandle, const char* szFilename, int nMode )
{
#ifdef TUD11
	PANIC_IF( TRUE, "DX11 EngineExportTexture TBI" );
#else
LPGRAPHICSTEXTURE	pTexture = maxInternalTextures[ nTextureHandle ].pTexture;
HRESULT		ret;

	switch( nMode )
	{
	case 3:
		ret = D3DXSaveTextureToFile( (LPCSTR)szFilename,D3DXIFF_PNG,pTexture,NULL );
		break;
	case 2:
		ret = D3DXSaveTextureToFile( (LPCSTR)szFilename,D3DXIFF_JPG,pTexture,NULL );
		break;
	case 1:
		ret = D3DXSaveTextureToFile( (LPCSTR)szFilename,D3DXIFF_DDS,pTexture,NULL );
		break;
	case 0:
	default:
		ret = D3DXSaveTextureToFile( (LPCSTR)szFilename, D3DXIFF_BMP, pTexture, NULL );
		break;
	}
#endif

}

void TexturedOverlays::Initialise( void )
{
int		nLoop;

	if ( mboInterfaceTexturesFirstInitialise == TRUE )
	{
		ZeroMemory( maxInternalTextures, sizeof( INTERNAL_TEXTURES ) * MAX_INTERNAL_TEXTURES_LOADED );
		mboInterfaceTexturesFirstInitialise = FALSE;
	}

	mnNumActiveTexOverlays = 0;

	for ( nLoop = 0; nLoop < MAX_DIFFERENT_TEXTURED_OVERLAYS; nLoop++ )
	{
		maxOverlayData[nLoop].pTexture = NULL;
		maxOverlayData[nLoop].hEngineTexture = NOTFOUND;
	}
	InitTextRectLists();

	if ( mpTexOverlayVertexBuffer1 == NULL )
	{
#ifdef USING_OPENGL
		// TODO 
		VertexBufferCreate( MAX_TEX_OVERLAY_VERTICES, &mpTexOverlayVertexBuffer1 );
		VertexBufferCreate( MAX_TEX_OVERLAY_VERTICES, &mpTexOverlayVertexBuffer2 );

#else
		// Create the vertex buffer.
		if( FAILED( mpInterfaceInstance->mpInterfaceInternals->CreateVertexBuffer( MAX_TEX_OVERLAY_VERTICES * sizeof(FLATVERTEX),
													  D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC, D3DFVF_FLATVERTEX,
													  &mpTexOverlayVertexBuffer1 ) ) )
		{
			PANIC_IF( TRUE, "Couldnt create Textured Overlay Vertex buffer");
			return;
		}

		// Create the vertex buffer.
		if( FAILED( mpInterfaceInstance->mpInterfaceInternals->CreateVertexBuffer( MAX_TEX_OVERLAY_VERTICES * sizeof(FLATVERTEX),
													  D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC, D3DFVF_FLATVERTEX,
													  &mpTexOverlayVertexBuffer2 ) ) )
		{
			PANIC_IF( TRUE, "Couldnt create Textured Overlay Vertex buffer");
			return;
		}
#endif
	}

	mpCurrentTexOverlayVertexBuffer = mpTexOverlayVertexBuffer1;
}


void	TexturedOverlays::UnlockTexture( int nTextureHandle )
{
	if ( nTextureHandle >= 0 )
	{
		if ( maxInternalTextures[ nTextureHandle ].pTexture )
		{
			maxInternalTextures[ nTextureHandle ].pTexture->UnlockRect( 0 );
		}
	}
}

int		TexturedOverlays::GetNewInternalTextureHandle( void )
{
int		nLoop = 0;
	
	do
	{
		if ( maxInternalTextures[ nLoop ].pTexture == NULL )
		{
			return( nLoop );
		}
		nLoop++;
	} while ( nLoop < MAX_INTERNAL_TEXTURES_LOADED );

	return( NOTFOUND );
}

int TexturedOverlays::GetTextureInternal( const char* szFilename, int nFlags, int nArchiveHandle )
{
int		nHandle;

	// Look for this filename if its already loaded
	nHandle = FindTexture( szFilename );
	if ( nHandle >= 0 )
	{
		// If found, add a reference to this texture
		maxInternalTextures[ nHandle ].nRefCount++;
		return( nHandle );
	}

	nHandle = GetNewInternalTextureHandle();

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
				maxInternalTextures[ nHandle ].pTexture = InterfaceLoadTextureFromArchiveDX( szFilename, 1, 1, nArchiveHandle );
			}
			else
			{
				maxInternalTextures[ nHandle ].pTexture = InterfaceLoadTextureDX( szFilename, 1, 1 );
			}
			break;
		case 1:			// Mipmaps
			if ( nArchiveHandle > 0 )
			{
				maxInternalTextures[ nHandle ].pTexture = InterfaceLoadTextureFromArchiveDX( szFilename, 0, 0, nArchiveHandle );
			}
			else
			{
				maxInternalTextures[ nHandle ].pTexture = InterfaceLoadTextureDX( szFilename, 0, 0 );
			}
			break;
		case 3:		// feck knows
			if ( nArchiveHandle > 0 )
			{
				maxInternalTextures[ nHandle ].pTexture = InterfaceLoadTextureFromArchiveDX( szFilename, 2, 0xFF, nArchiveHandle );
			}
			else
			{
				maxInternalTextures[ nHandle ].pTexture = InterfaceLoadTextureDX( szFilename, 2, 0xFF );
			}
			break;
		}

		// Load failed
		if ( !maxInternalTextures[ nHandle ].pTexture )
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

void	TexturedOverlays::SetLimit( int nState )
{
	if ( ( nState > 0 ) &&
		 ( nState != mnTexRectMaxBufferSize ) )
	{
		mnTexRectMaxBufferSize = nState;
		InitTextRectLists();
	}
}

int			TexturedOverlays::FindTexture( const char* szFilename )
{
int		nLoop = 0;

	do
	{
		if ( maxInternalTextures[ nLoop ].pTexture != NULL )
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

BYTE*	TexturedOverlays::LockTexture( int nTextureHandle, int* pnPitch, int* pnFormat, int nFlags )
{
BYTE*	pbImageData = NULL;

	*pnPitch = 0;
	*pnFormat = 0;

	if ( nTextureHandle >= 0 )
	{
		if ( maxInternalTextures[ nTextureHandle ].pTexture )
		{
		D3DLOCKED_RECT		xLockedRect;
		D3DSURFACE_DESC		xSurfDesc;
		HRESULT		hRet;

			maxInternalTextures[ nTextureHandle ].pTexture->GetLevelDesc( 0, &xSurfDesc );
			*pnFormat = xSurfDesc.Format;
			hRet = maxInternalTextures[ nTextureHandle ].pTexture->LockRect( 0, &xLockedRect, NULL, 0 );
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


//------------------------------------------------------------------------------------------------------------
// C INTERFACE
//------------------------------------------------------------------------------------------------------------

INTERFACE_API void	InterfaceSetTextureAsCurrentDirect( void* pTexture )
{
	mpInterfaceD3DDevice->SetTexture( 0, (LPGRAPHICSTEXTURE)pTexture );
}

INTERFACE_API void	InterfaceSetTextureAsCurrent( int nTextureHandle )
{
	InterfaceInstanceMain()->mpTexturedOverlays->SetTextureAsCurrent( nTextureHandle );
}




/***************************************************************************
 * Function    : InterfaceOverlayRenderType
  ***************************************************************************/
INTERFACE_API void InterfaceOverlayRenderType( int nOverlayNum, INTF_RENDER_TYPES nRenderType )
{
	InterfaceInstanceMain()->mpTexturedOverlays->SetRenderType( nOverlayNum, nRenderType );
}




/***************************************************************************
 * Function    : DrawTexturedOverlays
  ***************************************************************************/
void DrawTexturedOverlays( int nLayer )
{
	InterfaceInstanceMain()->mpTexturedOverlays->Render( nLayer );
}


/***************************************************************************
 * Function    : TexturedOverlayCreate
 * Params      : The texture that will be used for all rectangles on this overlay
 * Returns     : The number of the overlay for which all rectangles will use this texture
 * Description : See what feckin happens when i fill in this bit?
 ***************************************************************************/
int	TexturedOverlayCreate( int nLayer, TEXTURE_HANDLE hTexture )
{
	return( InterfaceInstanceMain()->mpTexturedOverlays->CreateOverlay( nLayer, hTexture ) );
}


int	TexturedOverlayCreateDirect( int nLayer, void* pTexture, int hEngineTexture )
{
	return( InterfaceInstanceMain()->mpTexturedOverlays->CreateOverlayDirect( nLayer, pTexture, hEngineTexture ) );
}


/***************************************************************************
 * Function    : InterfaceTexturedTri
 ***************************************************************************/
INTERFACE_API void InterfaceTexturedTri( int nOverlayNum, int* pnVerts, float* pfUVs, uint32 ulCol )
{
	InterfaceInstanceMain()->mpTexturedOverlays->AddTri( nOverlayNum, pnVerts, pfUVs, ulCol );
}

/***************************************************************************
 * Function    : InterfaceTexturedRect
 ***************************************************************************/
INTERFACE_API void InterfaceTexturedRect( int nOverlayNum, int nX, int nY, int nWidth, int nHeight, uint32 ulCol, float fU, float fV, float fUWidth, float fUHeight )
{
	InterfaceInstanceMain()->mpTexturedOverlays->AddRect( nOverlayNum, nX, nY, nWidth, nHeight, ulCol, fU, fV, fUWidth, fUHeight );
}

INTERFACE_API void	InterfaceTexturedRectShaded( int nOverlayNum, int nX, int nY, int nWidth, int nHeight, uint32 ulCol, uint32 ulCol2, float fU, float fV, float fUWidth, float fUHeight )
{
	InterfaceInstanceMain()->mpTexturedOverlays->AddRectShaded( nOverlayNum, nX, nY, nWidth, nHeight, ulCol, ulCol2, fU, fV, fUWidth, fUHeight );
}

/***************************************************************************
 * Function    : InterfaceSprite
 ***************************************************************************/
INTERFACE_API void InterfaceSprite( int nOverlayNum, int nX, int nY, float fTexGrid, int nTexGridNum, uint32 ulCol, float fRotAngle, float fScale )
{
	InterfaceInstanceMain()->mpTexturedOverlays->AddSprite( nOverlayNum, nX, nY, fTexGrid, nTexGridNum, ulCol, fRotAngle, fScale );
}



/***************************************************************************
 * Function    : InitTexturedOverlays
 ***************************************************************************/
void InitTexturedOverlays( void )
{
	InterfaceInstanceMain()->mpTexturedOverlays->Initialise();
}



/***************************************************************************
 * Function    : FreeTexturedOverlays
 ***************************************************************************/
void FreeTexturedOverlays( void )
{
	InterfaceInstanceMain()->mpTexturedOverlays->Shutdown();
}

void	InterfaceTexturedOverlaysSetLimit( int nState )
{
	InterfaceInstanceMain()->mpTexturedOverlays->SetLimit( nState );
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
//		if ( maxInternalTextures[ nTextureHandle ].pTexture != NULL )
//		{
			return( TexturedOverlayCreate( nLayer, nTextureHandle ) );
//		}
	}
	return( NOTFOUND );
}

INTERFACE_API void InterfaceReleaseTexture( int nTextureHandle )
{
	InterfaceInstanceMain()->mpTexturedOverlays->ReleaseTexture( nTextureHandle );
}



#ifndef USING_OPENGL
INTERFACE_API int	InterfaceGetTextureSize( int nTextureHandle, int* pnW, int* pnH )
{
	return( InterfaceInstanceMain()->mpTexturedOverlays->GetTextureSize( nTextureHandle, pnW, pnH ));
}


INTERFACE_API int  InterfaceGetTextureFromFileInMem( const char* szFilename, unsigned char* pbMem, int nMemSize, int nFlags )
{
	return( InterfaceInstanceMain()->mpTexturedOverlays->GetTextureFromFileInMem( szFilename, pbMem, nMemSize, nFlags ) );
}

void	InterfaceExportTexture( int nTextureHandle, const char* szFilename, int nMode )
{
	InterfaceInstanceMain()->mpTexturedOverlays->ExportTexture( nTextureHandle, szFilename, nMode );
}


INTERFACE_API BYTE*	InterfaceLockTexture( int nTextureHandle, int* pnPitch, int* pnFormat, int nFlags )
{
	return( InterfaceInstanceMain()->mpTexturedOverlays->LockTexture( nTextureHandle, pnPitch, pnFormat, nFlags ) );	
}



INTERFACE_API void	InterfaceTextureGetColourAtPoint( byte* pbLockedTextureData, int nPitch, int nFormat, int x, int y, float* pfRed, float* pfGreen, float* pfBlue, float* pfAlpha )
{
byte*		pbRow = pbLockedTextureData + (y * nPitch);
int		nR = 0;
int		nG = 0;
int		nB = 0;
int		nA = 0;
uint32	ulCol;
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
		ulCol = *( (uint32*)( pbRow + (x * 4) ) );
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
	InterfaceInstanceMain()->mpTexturedOverlays->UnlockTexture( nTextureHandle);
}


int			InterfaceFindTexture( const char* szFilename )
{
	return( InterfaceInstanceMain()->mpTexturedOverlays->FindTexture( szFilename ) );
}


INTERFACE_API int InterfaceGetTextureInternal( const char* szFilename, int nFlags, int nArchiveHandle )
{
	return( InterfaceInstanceMain()->mpTexturedOverlays->GetTextureInternal( szFilename, nFlags, nArchiveHandle ));
}



INTERFACE_API int InterfaceGetTexture( const char* szFilename, int nFlags )
{
	return( InterfaceGetTextureInternal( szFilename, nFlags, NOTFOUND ) );
}


int	InterfaceCreateBlankTexture( int nWidth, int nHeight, int Mode)
{
	return( InterfaceInstanceMain()->mpTexturedOverlays->CreateBlankTexture( nWidth, nHeight, Mode ) );
}

#endif	// non-GL stuff above  (GL does it in OpenGL/InterfaceTextureManager)



//#endif // if 0