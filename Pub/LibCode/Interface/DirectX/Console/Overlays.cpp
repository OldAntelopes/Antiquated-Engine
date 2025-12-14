
#include "../InterfaceInternalsDX.h"

#include <StandardDef.h>
#include <Interface.h>
#include <Engine.h>

#include "../../Common/Overlays/Overlays.h"
#include "../../Common/InterfaceInstance.h"



void	Overlays::RenderLinesBuffer( void )
{
int		nDrawHowMany;

	if ( mpLineVertices != NULL )
	{
		mpxOverlaysLineVertexBuffer->Unlock();
		mpLineVertices = NULL;
	}

	nDrawHowMany = (mnNextLineVertex / 2);

	if ( nDrawHowMany > 0 )
	{
		EngineSetVertexFormat( VERTEX_FORMAT_FLATVERTEX );
		EngineEnableLighting( FALSE );
		EngineSetTexture( 0, NOTFOUND );
		EngineSetColourMode( 0, COLOUR_MODE_DIFFUSE_ONLY );
		EngineEnableFog( FALSE );
		EngineEnableZTest( FALSE );
		EngineEnableZWrite( FALSE );
		EngineResetColourMode();
		EngineEnableAlphaTest( 0 );
		if ( mboRenderLineAlpha == TRUE )
		{
			EngineEnableBlend( TRUE );
			EngineSetBlendMode( BLEND_MODE_ALPHABLEND );
		}
		else
		{
			EngineEnableBlend( FALSE );
		}
		InterfaceInternalDXSetStreamSource( 0, mpxOverlaysLineVertexBuffer, 0, sizeof(FLATVERTEX) );
		mpInterfaceD3DDevice->DrawPrimitive( D3DPT_LINELIST, 0, nDrawHowMany );
		
		mnNextLineVertex = 0;
	}
}

INTERFACE_API void InterfaceLine( int nLayer, int nX1, int nY1, int nX2, int nY2, uint32 ulCol1, uint32 ulCol2 )
{
	InterfaceInstanceMain()->mpOverlays->Line( nLayer, nX1, nY1, nX2, nY2, ulCol1, ulCol2 );
}


void Overlays::Line( int nLayer, int nX1, int nY1, int nX2, int nY2, uint32 ulCol1, uint32 ulCol2 )
{
FLATVERTEX*		pxLineVertex;
int		nWidth = mpInterfaceInstance->GetWidth();
int		nHeight = mpInterfaceInstance->GetHeight();

	if ( !mpxOverlaysLineVertexBuffer ) return;

	if ( (( nX1 < 0 ) && ( nX2 < 0 )) ||
		 (( nY1 < 0 ) && ( nY2 < 0 )) ||
		 (( nX1 > nWidth ) && ( nX2 > nWidth )) ||
		 (( nY1 > nHeight ) && ( nX2 > nHeight )) )
	{
		return;
	}

	if ( mnNextLineVertex >= (LINE_VERTEX_BUFFER_SIZE-2) )
	{
		RenderLinesBuffer();
	}

	if ( mpLineVertices == NULL )
	{
		if( ( mpxOverlaysLineVertexBuffer ) && 
			( FAILED( mpxOverlaysLineVertexBuffer->Lock(0, 0, (VERTEX_LOCKTYPE)&mpLineVertices, D3DLOCK_DISCARD)) ) )
		{
			return;
		}
		mnNextLineVertex = 0;
	}

	pxLineVertex = mpLineVertices + mnNextLineVertex;

	pxLineVertex->color = ulCol1;
//	mpLineVertices[mnNextLineVertex].tu = 0.0f;
//	mpLineVertices[mnNextLineVertex].tv = 0.0f;
	pxLineVertex->x = (float)nX1 + mpInterfaceInstance->GetDrawDimensions().x;
	pxLineVertex->y = (float)nY1 + mpInterfaceInstance->GetDrawDimensions().y;
	pxLineVertex->z = 0.0f;
	pxLineVertex++;

	pxLineVertex->color = ulCol2;
	pxLineVertex->x = (float)nX2 + mpInterfaceInstance->GetDrawDimensions().x;
	pxLineVertex->y = (float)nY2 + mpInterfaceInstance->GetDrawDimensions().y;
	pxLineVertex->z = 0.0f;
	
	mnNextLineVertex += 2;

}

int AddOverlayTriVertices( FLATVERTEX* pVertices, int nX1, int nY1, int nX2, int nY2, int nX3, int nY3, uint32 ulCol1, uint32 ulCol2, uint32 ulCol3, int nVertIndex, int nIconTextureNum )
{
	pVertices[ nVertIndex ].x = (float)(nX1);
	pVertices[ nVertIndex ].y = (float)(nY1);
	pVertices[ nVertIndex ].z = 0.0f;
	pVertices[ nVertIndex ].color    = ulCol1;
	pVertices[ nVertIndex ].tu       = 0.0f;
	pVertices[ nVertIndex ].tv       = 0.0f;
	nVertIndex++;

	pVertices[ nVertIndex ].x = (float)(nX2);
	pVertices[ nVertIndex ].y = (float)(nY2);
	pVertices[ nVertIndex ].z = 0.0f;
	pVertices[ nVertIndex ].color    = ulCol2;
	pVertices[ nVertIndex ].tu       = 0.0f;
	pVertices[ nVertIndex ].tv       = 1.0f;
	nVertIndex++;

	pVertices[ nVertIndex ].x = (float)(nX3);
	pVertices[ nVertIndex ].y = (float)(nY3);
	pVertices[ nVertIndex ].z = 0.0f;
	pVertices[ nVertIndex ].color    = ulCol3;
	pVertices[ nVertIndex ].tu       = 0.0f;
	pVertices[ nVertIndex ].tv       = 1.0f;
	nVertIndex++;

	return( nVertIndex );
}


/***************************************************************************
 * Function    : AddOverlayVertices
 * Params      : 
 * Returns     :
 * Description : 
 ***************************************************************************/
int AddOverlayVertices( FLATVERTEX* pVertices, int nX, int nY, int nWidth, int nHeight, uint32 ulCol, int nVertIndex, int nIconTextureNum )
{
float	fX;
float	fY;
float	fWidth;
float	fHeight;
float	fV;
float	fV1;

	fV = (float)( nIconTextureNum ) / 8.0f;
	fV1 = fV + ( 1.0f / 8.0f );

	fX = (FLOAT)( nX );
	fY = (FLOAT)( nY );
	fWidth = (FLOAT)( nWidth );
	fHeight = (FLOAT)( nHeight );


	pVertices[ nVertIndex ].x = fX;
	pVertices[ nVertIndex ].y = fY;
	pVertices[ nVertIndex ].z = 0.0f;
//	pVertices[ nVertIndex ].rhw = 1.0f;
	pVertices[ nVertIndex ].color    = ulCol;
	pVertices[ nVertIndex ].tu       = 0.0f;
	pVertices[ nVertIndex ].tv       = fV;
	nVertIndex++;

	pVertices[ nVertIndex ].x = fX + fWidth;
	pVertices[ nVertIndex ].y = fY;
	pVertices[ nVertIndex ].z = 0.0f;
//	pVertices[ nVertIndex ].rhw = 1.0f;
	pVertices[ nVertIndex ].color    = ulCol;
	pVertices[ nVertIndex ].tu       = 1.0f;
	pVertices[ nVertIndex ].tv       = fV;
	nVertIndex++;

	pVertices[ nVertIndex ].x = fX + fWidth;
	pVertices[ nVertIndex ].y = fY + fHeight;
	pVertices[ nVertIndex ].z = 0.0f;
//	pVertices[ nVertIndex ].rhw = 1.0f;
	pVertices[ nVertIndex ].color    = ulCol;
	pVertices[ nVertIndex ].tu       = 1.0f;
	pVertices[ nVertIndex ].tv       = fV1;
	nVertIndex++;

	pVertices[ nVertIndex ].x = fX;
	pVertices[ nVertIndex ].y = fY + fHeight;
	pVertices[ nVertIndex ].z = 0.0f;
//	pVertices[ nVertIndex ].rhw = 1.0f;
	pVertices[ nVertIndex ].color    = ulCol;
	pVertices[ nVertIndex ].tu       = 0.0f;
	pVertices[ nVertIndex ].tv       = fV1;
	nVertIndex++;

	pVertices[ nVertIndex ].x = fX;
	pVertices[ nVertIndex ].y = fY;
	pVertices[ nVertIndex ].z = 0.0f;
//	pVertices[ nVertIndex ].rhw = 1.0f;
	pVertices[ nVertIndex ].color    = ulCol;
	pVertices[ nVertIndex ].tu       = 0.0f;
	pVertices[ nVertIndex ].tv       = fV;
	nVertIndex++;

	pVertices[ nVertIndex ].x = fX + fWidth;
	pVertices[ nVertIndex ].y = fY + fHeight;
	pVertices[ nVertIndex ].z = 0.0f;
//	pVertices[ nVertIndex ].rhw = 1.0f;
	pVertices[ nVertIndex ].color    = ulCol;
	pVertices[ nVertIndex ].tu       = 1.0f;
	pVertices[ nVertIndex ].tv       = fV1;
	nVertIndex++;

	return( nVertIndex );

}


INTERFACE_API void	InterfaceOverlaysAdditive( BOOL bFlag )
{
	InterfaceInstanceMain()->mpOverlays->EnableAdditive( bFlag );
}

void	Overlays::EnableAdditive( BOOL bFlag )
{
	mbAdditiveOverlays = bFlag;
}

INTERFACE_API void InterfaceShadedRect( int nLayer, int nX, int nY, int nWidth, int nHeight, uint32 ulCol1, uint32 ulCol2,uint32 ulCol3, uint32 ulCol4 )
{
	InterfaceInstanceMain()->mpOverlays->ShadedRect( nLayer, nX, nY, nWidth, nHeight, ulCol1, ulCol2, ulCol3, ulCol4 );
}
	
void Overlays::ShadedRect( int nLayer, int nX, int nY, int nWidth, int nHeight, uint32 ulCol1, uint32 ulCol2,uint32 ulCol3, uint32 ulCol4 )
{
FLATVERTEX* pVertices;
int nVertIndex;
float	fX;
float	fY;
float	fWidth;
float	fHeight;
float	fV = 0.0f;
float	fV1 = 1.0f;
OVERLAY_VERTEX_BUFFER_CONTAINER*		pxContainer;
	
	pxContainer = maOverlayVertexBuffer + (nLayer % MAX_NUM_OVERLAY_LAYERS );
	
	if ( !pxContainer->mpxVertexBuffer ) return;

	if ( pxContainer->mnNextOverlayVertex >= (NUM_OVERLAY_VERTICES-6) )
	{
		PANIC_IF( TRUE, "Too many overlay boxes added" );
		return;
	}

	if ( !pxContainer->mboBufferIsLocked )
	{
		if( FAILED( pxContainer->mpxVertexBuffer->Lock( 0, 0, (VERTEX_LOCKTYPE)&pxContainer->mpOverlayVertices, D3DLOCK_DISCARD ) ) )
		{
			return;
		}
		pxContainer->mboBufferIsLocked = TRUE;
	}

	nVertIndex = pxContainer->mnNextOverlayVertex;
	pVertices = pxContainer->mpOverlayVertices;
	
	fX = (FLOAT)( nX );
	fY = (FLOAT)( nY );
	fWidth = (FLOAT)( nWidth );
	fHeight = (FLOAT)( nHeight );

	pVertices[ nVertIndex ].x = fX;
	pVertices[ nVertIndex ].y = fY;
	pVertices[ nVertIndex ].z = 0.0f;
//	pVertices[ nVertIndex ].rhw = 1.0f;
	pVertices[ nVertIndex ].color    = ulCol1;
	pVertices[ nVertIndex ].tu       = 0.0f;
	pVertices[ nVertIndex ].tv       = fV;
	nVertIndex++;

	pVertices[ nVertIndex ].x = fX + fWidth;
	pVertices[ nVertIndex ].y = fY;
	pVertices[ nVertIndex ].z = 0.0f;
//	pVertices[ nVertIndex ].rhw = 1.0f;
	pVertices[ nVertIndex ].color    = ulCol2;
	pVertices[ nVertIndex ].tu       = 1.0f;
	pVertices[ nVertIndex ].tv       = fV;
	nVertIndex++;

	pVertices[ nVertIndex ].x = fX + fWidth;
	pVertices[ nVertIndex ].y = fY + fHeight;
	pVertices[ nVertIndex ].z = 0.0f;
//	pVertices[ nVertIndex ].rhw = 1.0f;
	pVertices[ nVertIndex ].color    = ulCol4;
	pVertices[ nVertIndex ].tu       = 1.0f;
	pVertices[ nVertIndex ].tv       = fV1;
	nVertIndex++;

	pVertices[ nVertIndex ].x = fX;
	pVertices[ nVertIndex ].y = fY + fHeight;
	pVertices[ nVertIndex ].z = 0.0f;
//	pVertices[ nVertIndex ].rhw = 1.0f;
	pVertices[ nVertIndex ].color    = ulCol3;
	pVertices[ nVertIndex ].tu       = 0.0f;
	pVertices[ nVertIndex ].tv       = fV1;
	nVertIndex++;

	pVertices[ nVertIndex ].x = fX;
	pVertices[ nVertIndex ].y = fY;
	pVertices[ nVertIndex ].z = 0.0f;
//	pVertices[ nVertIndex ].rhw = 1.0f;
	pVertices[ nVertIndex ].color    = ulCol1;
	pVertices[ nVertIndex ].tu       = 0.0f;
	pVertices[ nVertIndex ].tv       = fV;
	nVertIndex++;

	pVertices[ nVertIndex ].x = fX + fWidth;
	pVertices[ nVertIndex ].y = fY + fHeight;
	pVertices[ nVertIndex ].z = 0.0f;
//	pVertices[ nVertIndex ].rhw = 1.0f;
	pVertices[ nVertIndex ].color    = ulCol4;
	pVertices[ nVertIndex ].tu       = 1.0f;
	pVertices[ nVertIndex ].tv       = fV1;
	nVertIndex++;

	pxContainer->mnNextOverlayVertex = nVertIndex;

}

/***************************************************************************
 * Function    : InterfaceRect
 ***************************************************************************/
INTERFACE_API void InterfaceRect( int nLayer, int nX, int nY, int nWidth, int nHeight, uint32 ulCol)
{
	InterfaceInstanceMain()->mpOverlays->Rect( nLayer, nX, nY, nWidth, nHeight, ulCol );
}

void Overlays::Rect( int nLayer, int nX, int nY, int nWidth, int nHeight, uint32 ulCol)
{
OVERLAY_VERTEX_BUFFER_CONTAINER*		pxContainer;
	
	pxContainer = maOverlayVertexBuffer + (nLayer % MAX_NUM_OVERLAY_LAYERS );
	
	if ( !pxContainer->mpxVertexBuffer ) return;

	if ( pxContainer->mnNextOverlayVertex >= (NUM_OVERLAY_VERTICES-6) )
	{
		PANIC_IF( TRUE, "Too many overlay vertices added" );
		return;
	}

	if ( !pxContainer->mboBufferIsLocked )
	{
		if( FAILED( pxContainer->mpxVertexBuffer->Lock( 0, 0, (VERTEX_LOCKTYPE)&pxContainer->mpOverlayVertices, D3DLOCK_DISCARD ) ) )
		{
			return;
		}
		pxContainer->mboBufferIsLocked = TRUE;
	}
	
	pxContainer->mnNextOverlayVertex = AddOverlayVertices( pxContainer->mpOverlayVertices, nX + mpInterfaceInstance->GetDrawDimensions().x, nY + mpInterfaceInstance->GetDrawDimensions().y, nWidth, nHeight, ulCol, pxContainer->mnNextOverlayVertex, 0 );
}

INTERFACE_API void	InterfaceTri( int nLayer, int nX1, int nY1, int nX2, int nY2, int nX3, int nY3, uint32 ulCol1, uint32 ulCol2, uint32 ulCol3 )
{
	InterfaceInstanceMain()->mpOverlays->Triangle( nLayer, nX1, nY1, nX2, nY2, nX3, nY3, ulCol1, ulCol2, ulCol3 );
}


void	Overlays::Triangle( int nLayer, int nX1, int nY1, int nX2, int nY2, int nX3, int nY3, uint32 ulCol1, uint32 ulCol2, uint32 ulCol3 )
{
OVERLAY_VERTEX_BUFFER_CONTAINER*		pxContainer;
	
	pxContainer = maOverlayVertexBuffer + (nLayer % MAX_NUM_OVERLAY_LAYERS );
	
	if ( !pxContainer->mpxVertexBuffer ) return;

	if ( pxContainer->mnNextOverlayVertex >= (NUM_OVERLAY_VERTICES-3) )
	{
		PANIC_IF( TRUE, "Too many overlay boxes added" );
		return;
	}

	if ( !pxContainer->mboBufferIsLocked )
	{
		if( FAILED( pxContainer->mpxVertexBuffer->Lock( 0, 0, (VERTEX_LOCKTYPE)&pxContainer->mpOverlayVertices, D3DLOCK_DISCARD ) ) )
		{
			return;
		}
		pxContainer->mboBufferIsLocked = TRUE;
	}

	nX1 += mpInterfaceInstance->GetDrawDimensions().x;
	nX2 += mpInterfaceInstance->GetDrawDimensions().x;
	nX3 += mpInterfaceInstance->GetDrawDimensions().x;
	nY1 += mpInterfaceInstance->GetDrawDimensions().y;
	nY2 += mpInterfaceInstance->GetDrawDimensions().y;
	nY3 += mpInterfaceInstance->GetDrawDimensions().y;

	pxContainer->mnNextOverlayVertex = AddOverlayTriVertices( pxContainer->mpOverlayVertices, nX1, nY1, nX2, nY2, nX3, nY3, ulCol1, ulCol2, ulCol3, pxContainer->mnNextOverlayVertex, 0 );
}


/***************************************************************************
 * Function    : InterfaceOutlineBox
 ***************************************************************************/
INTERFACE_API void InterfaceOutlineBox ( int nLayer, int nX, int nY, int nWidth, int nHeight, uint32 ulCol )
{
	InterfaceInstanceMain()->mpOverlays->OutlineBox( nLayer, nX, nY, nWidth, nHeight, ulCol );
	
}

	
void Overlays::OutlineBox( int nLayer, int nX, int nY, int nWidth, int nHeight, uint32 ulCol )
{
uint32	ulColHi = ulCol;
uint32	ulColLo;
uint32	ulColLo2;
uint32	ulColLo3;
float	R, G, B, A;

	A = (float)( ulCol >> 24 );
	R = (float)(( ulCol >> 16 ) & 0xFF);
	G = (float)(( ulCol >> 8 ) & 0xFF);
	B = (float)( ulCol & 0xFF );
 
	if ( ( R > 0.6f ) && 
		 ( G > 0.6f ) &&
		 ( B > 0.6f ) )
	{
		ulColLo = (uint32)( A ) << 24;
		ulColLo |= (uint32)( R * 0.8f ) << 16;
		ulColLo |= (uint32)( G * 0.8f ) << 8;
		ulColLo |= (uint32)( B * 0.8f );

		ulColLo2 = (uint32)( A ) << 24;
		ulColLo2 |= (uint32)( R * 0.3f ) << 16;
		ulColLo2 |= (uint32)( G * 0.3f ) << 8;
		ulColLo2 |= (uint32)( B * 0.3f );

		ulColLo3 = (uint32)( A ) << 24;
		ulColLo3 |= (uint32)( R * 0.5f ) << 16;
		ulColLo3 |= (uint32)( G * 0.5f ) << 8;
		ulColLo3 |= (uint32)( B * 0.5f );

	/*
		InterfaceLine( nLayer, nX - 1, nY, nX + nWidth + 1, nY, ulColHi, GetColWithModifiedAlpha(ulColHi,0.9f) );
		InterfaceLine( nLayer, nX - 1, nY, nX - 1, nY + nHeight + 1, ulColHi, GetColWithModifiedAlpha(ulColHi,0.9f) );

		InterfaceLine( nLayer, nX + nWidth, nY, nX + nWidth, nY + nHeight + 1, ulColLo, ulColLo );
		InterfaceLine( nLayer, nX - 1, nY + nHeight, nX + nWidth + 1, nY + nHeight, ulColLo, ulColLo );
	*/
		InterfaceRect( nLayer, nX + nWidth - 2, nY - 1, 3, nHeight + 2, ulColLo2 );
		InterfaceRect( nLayer, nX - 2, nY + nHeight - 1, nWidth + 3, 3, ulColLo2 );

		InterfaceRect( nLayer, nX - 2, nY - 1, nWidth + 2, 1, ulColLo2 );
		InterfaceRect( nLayer, nX - 1, nY, nWidth + 1, 1, ulColHi );
		InterfaceRect( nLayer, nX, nY + 1, nWidth, 1, ulColLo3 );
		InterfaceRect( nLayer, nX - 2, nY - 1, 1, nHeight + 2, ulColLo2 );
		InterfaceRect( nLayer, nX - 1, nY, 1, nHeight + 1, ulColHi );
		InterfaceRect( nLayer, nX, nY + 1, 1, nHeight - 1, ulColLo3 );

		InterfaceRect( nLayer, nX + nWidth - 1, nY, 1, nHeight + 1, ulColLo );
		InterfaceRect( nLayer, nX - 1, nY + nHeight, nWidth + 1, 1, ulColLo );
	}
	else
	{
		InterfaceRect( nLayer, nX - 1, nY, nWidth + 1, 1, ulColHi );
		InterfaceRect( nLayer, nX - 1, nY, 1, nHeight + 1, ulColHi );
		InterfaceRect( nLayer, nX + nWidth - 1, nY, 1, nHeight + 1, ulColHi );
		InterfaceRect( nLayer, nX - 1, nY + nHeight, nWidth + 1, 1, ulColHi );
	}
}

/***************************************************************************
 * Function    : InterfaceShadedBox
 ***************************************************************************/
INTERFACE_API void InterfaceShadedBox( int nLayer, int nX, int nY, int nWidth, int nHeight, int nStyle )
{
	InterfaceInstanceMain()->mpOverlays->ShadedBox( nLayer, nX, nY, nWidth, nHeight, nStyle );
}

void Overlays::ShadedBox( int nLayer, int nX, int nY, int nWidth, int nHeight, int nStyle )
{
	if ( ( nStyle < 0xFFFFFF ) &&
		 ( nStyle > 0 ) )
	{
		switch ( nStyle )
		{
		case 0:
		default:
			InterfaceRect( 1, nX - 2, nY - 2, 2, nHeight + 4, 0x404080CF );
			InterfaceRect( 1, nX + nWidth, nY - 2, 2, nHeight + 4, 0x404080CF );

			InterfaceRect( 1, nX, nY - 2, nWidth, 2, 0x404080CF );
			InterfaceRect( 1, nX, nY + nHeight, nWidth, 2, 0x404080CF );
			break;
		case 1:
			InterfaceRect( 1, nX, nY, nWidth, nHeight, 0x70000000 );
			InterfaceRect( 1, nX - 2, nY - 2, 2, nHeight + 4, 0x404080CF );
			InterfaceRect( 1, nX + nWidth, nY - 2, 2, nHeight + 4, 0x404080CF );

			InterfaceRect( 1, nX, nY - 2, nWidth, 2, 0x404080CF );
			InterfaceRect( 1, nX, nY + nHeight, nWidth, 2, 0x404080CF );
			break;
		case 2:
			InterfaceRect( 1, nX, nY, nWidth, nHeight, 0x70404040 );
			InterfaceRect( 1, nX - 2, nY - 2, 2, nHeight + 4, 0x60B0B0B0 );
			InterfaceRect( 1, nX + nWidth, nY - 2, 2, nHeight + 4, 0x60B0B0B0 );

			InterfaceRect( 1, nX, nY - 2, nWidth, 2, 0x60101010 );
			InterfaceRect( 1, nX, nY + nHeight, nWidth, 2, 0x60101010 );
			break;
		}
	}
	else 
	{
		InterfaceRect( 1, nX, nY, nWidth, nHeight, (uint32)(nStyle) );

		InterfaceRect( 1, nX - 2, nY - 2, 2, nHeight + 4, 0x404080CF );
		InterfaceRect( 1, nX + nWidth, nY - 2, 2, nHeight + 4, 0x404080CF );

		InterfaceRect( 1, nX, nY - 2, nWidth, 2, 0x404080CF );
		InterfaceRect( 1, nX, nY + nHeight, nWidth, 2, 0x404080CF );
	}

}

/***************************************************************************
 * Function    : RenderOverlays
 ***************************************************************************/
void RenderOverlays( int nLayer )
{
	InterfaceInstanceMain()->mpOverlays->Render( nLayer );
}


/***************************************************************************
 * Function    : RenderOverlays
 ***************************************************************************/
void Overlays::Render( int nLayer )
{
OVERLAY_VERTEX_BUFFER_CONTAINER*		pxContainer;
int				nDrawHowMany;

	// Draw InterfaceLines alongside layer 1 overlays
	if ( nLayer == 1 )
	{
		if ( mnNextLineVertex >= 0 )
		{
			RenderLinesBuffer();
		}
	}

	pxContainer = maOverlayVertexBuffer + (nLayer % MAX_NUM_OVERLAY_LAYERS );
	if ( !pxContainer->mpxVertexBuffer ) return;

	nDrawHowMany = (pxContainer->mnNextOverlayVertex / 3);
	
	if ( nDrawHowMany > 0 )
	{
		EngineSetVertexFormat( VERTEX_FORMAT_FLATVERTEX );
		EngineEnableLighting( FALSE );
		mpInterfaceD3DDevice->SetTexture( 0, NULL );
		EngineSetColourMode( 0, COLOUR_MODE_DIFFUSE_ONLY );
		// Enable alpha testing (skips pixels with less than a certain alpha.)
		EngineEnableFog( FALSE );
		EngineEnableZTest( FALSE );
		EngineEnableZWrite( FALSE );
		EngineEnableAlphaTest( 0 );
		EngineEnableBlend( TRUE );
		if ( mbAdditiveOverlays )
		{
			mpInterfaceD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			mpInterfaceD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE);
		}
		else
		{
			mpInterfaceD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			mpInterfaceD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		}

		if ( pxContainer->mboBufferIsLocked )
		{
			pxContainer->mpxVertexBuffer->Unlock();
			pxContainer->mboBufferIsLocked = FALSE;
		}
		InterfaceInternalDXSetStreamSource( 0, pxContainer->mpxVertexBuffer, 0, sizeof(FLATVERTEX) );
		mpInterfaceD3DDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, nDrawHowMany );
		pxContainer->mnNextOverlayVertex = 0;
	}

}

/***************************************************************************
 * Function    : InitialiseOverlays
 ***************************************************************************/
HRESULT InitialiseOverlays( void )
{
	InterfaceInstanceMain()->mpOverlays->Initialise();
	return S_OK;
}

void	Overlays::Initialise( void )
{
int		nLoop;
OVERLAY_VERTEX_BUFFER_CONTAINER*		pxContainer;

	for( nLoop = 0; nLoop < MAX_NUM_OVERLAY_LAYERS; nLoop++ )
	{
		pxContainer = maOverlayVertexBuffer + nLoop;
		if ( pxContainer->mpxVertexBuffer == NULL )
		{
			// Create the vertex buffer.
			if( FAILED( InterfaceInternalDXCreateVertexBuffer( NUM_OVERLAY_VERTICES * sizeof(FLATVERTEX),
													  D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC, D3DFVF_FLATVERTEX,
													  &pxContainer->mpxVertexBuffer ) ) )
			{
				PANIC_IF( TRUE, "Couldnt create Overlay Vertex buffer");
				return;
			}
		}
	}

	if ( mpxOverlaysLineVertexBuffer == NULL )
	{
		// Create the vertex buffer.
		if( FAILED( InterfaceInternalDXCreateVertexBuffer( LINE_VERTEX_BUFFER_SIZE * sizeof(FLATVERTEX),
													  D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC, D3DFVF_FLATVERTEX,
													  &mpxOverlaysLineVertexBuffer ) ) )
		{
			PANIC_IF( TRUE, "Couldnt create Line Vertex buffer 1");
			return;
		}
	}

} 

/***************************************************************************
 * Function    : FreeOverlays
 ***************************************************************************/
void FreeOverlays( void )
{
	InterfaceInstanceMain()->mpOverlays->Shutdown();
}


void Overlays::Shutdown( void )
{
int		nLoop;
OVERLAY_VERTEX_BUFFER_CONTAINER*		pxContainer;

    if( mpxOverlaysLineVertexBuffer != NULL )
	{
        mpxOverlaysLineVertexBuffer->Release();
		mpxOverlaysLineVertexBuffer = NULL;
	}
	mpLineVertices = NULL;

	for( nLoop = 0; nLoop < MAX_NUM_OVERLAY_LAYERS; nLoop++ )
	{
		pxContainer = maOverlayVertexBuffer + nLoop;
	    if( pxContainer->mpxVertexBuffer != NULL )
		{
			if ( pxContainer->mboBufferIsLocked )
			{
				pxContainer->mpxVertexBuffer->Unlock();
				pxContainer->mboBufferIsLocked = FALSE;
			}
			pxContainer->mpxVertexBuffer->Release();
			pxContainer->mpxVertexBuffer = NULL;
		}
	}
}


/***************************************************************************
 * Function    : LockOverlays
 ***************************************************************************/
void LockOverlays( void )
{
	InterfaceInstanceMain()->mpOverlays->LockOverlays();
}

void Overlays::LockOverlays( void )
{
int		nLoop;
OVERLAY_VERTEX_BUFFER_CONTAINER*		pxContainer;

	for( nLoop = 0; nLoop < MAX_NUM_OVERLAY_LAYERS; nLoop++ )
	{
		pxContainer = maOverlayVertexBuffer + nLoop;

		if( FAILED( pxContainer->mpxVertexBuffer->Lock( 0, 0, (VERTEX_LOCKTYPE)&pxContainer->mpOverlayVertices, D3DLOCK_DISCARD ) ) )
		{
		    return;
		}
		pxContainer->mboBufferIsLocked = TRUE;
	}
}


/***************************************************************************
 * Function    : UnlockOverlays
 ***************************************************************************/
void UnlockOverlays( void )
{
	InterfaceInstanceMain()->mpOverlays->UnlockOverlays();
}


/***************************************************************************
 * Function    : UnlockOverlays
 ***************************************************************************/
void Overlays::UnlockOverlays( void )
{
int		nLoop;
OVERLAY_VERTEX_BUFFER_CONTAINER*		pxContainer;

	for( nLoop = 0; nLoop < MAX_NUM_OVERLAY_LAYERS; nLoop++ )
	{
		pxContainer = maOverlayVertexBuffer + nLoop;
		pxContainer->mpxVertexBuffer->Unlock();
		pxContainer->mboBufferIsLocked = FALSE;
	}
}
