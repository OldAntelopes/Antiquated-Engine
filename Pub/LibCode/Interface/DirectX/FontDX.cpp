
#include "InterfaceInternalsDX.h"

#include <stdio.h>

#include <StandardDef.h>
#include <Interface.h>
#include <Engine.h>

#include "../Common/Overlays/Overlays.h"
#include "../Common/Font/FontCommon.h"
#include "FontDX.h"

int							mnCurrentRenderFont = 0;



IGRAPHICSVERTEXBUFFER*		mpxFontVertexBuffer1 = NULL;
IGRAPHICSVERTEXBUFFER*		mpxFontVertexBuffer2 = NULL;
IGRAPHICSVERTEXBUFFER*		mpxCurrentFontVertexBuffer = NULL;


void	CFontDef::SetTextureAsCurrent( void )
{
	InterfaceSetTextureAsCurrentDirect( mpTexture );
}


///--------------------------------------------------------------
//CFontDef::LoadTexture
// Loads the texture file specified using SetTextureFileName
// The image is expected to be a greyscale. This load function uses the brightness
// of the image to generate an alpha map used for blending the font
//---------------------------------------------------------------
void	CFontDef::LoadTexture( void )
{
LPGRAPHICSTEXTURE pxTempTexture;
D3DLOCKED_RECT	xLockedRectDest;
D3DLOCKED_RECT	xLockedRectSrc;
D3DSURFACE_DESC		xSurface;
byte*		pbDestination;
byte*		pbSrc;
ushort*		puwSrc;
uint32*		pulDest;
ushort*		puwDest;
byte*		pbDest;
int			nX;
int			nY;
int			nWidth;
int			nHeight;
byte		bR, bG, bB;
int			nAlpha;
int			nFormat;

	if ( 0 )	// dont use alpha textures
	{
		mpTexture = InterfaceLoadTextureDX( m_szTextureFilename, 0, 0 );
		mpTexture->GetLevelDesc( 0, &xSurface );

		m_TextureSizeY = xSurface.Height;
		m_TextureSizeX = xSurface.Width;
	}
	else
	{
		pxTempTexture = InterfaceLoadTextureDX( m_szTextureFilename, 0, 0xFF );
		if ( pxTempTexture == NULL ) return;

		pxTempTexture->GetLevelDesc( 0, &xSurface );
		if ( xSurface.Format != D3DFMT_A1R5G5B5 )
		{
			PANIC_IF(TRUE,"Format error in font loadtexture");
		}

		nWidth = xSurface.Width;
		nHeight = xSurface.Height;
		m_TextureSizeY = nHeight;
		m_TextureSizeX = nWidth;
		nFormat = D3DFMT_A8R8G8B8;
		InterfaceInternalDXCreateTexture( nWidth, nHeight, 1, 0, FORMAT_A8R8G8B8, &mpTexture );
		if ( mpTexture == NULL )
		{
			nFormat = D3DFMT_A8R3G3B2;
			InterfaceInternalDXCreateTexture( nWidth, nHeight, 1, 0, FORMAT_A8R3G3B2, &mpTexture );
		
			if ( mpTexture == NULL )
			{
				nFormat = D3DFMT_A4R4G4B4;
				InterfaceInternalDXCreateTexture( nWidth, nHeight, 1, 0, FORMAT_A4R4G4B4, &mpTexture );
			
				if ( mpTexture == NULL )
				{
					nFormat = D3DFMT_A8;
					InterfaceInternalDXCreateTexture( nWidth, nHeight, 1, 0, FORMAT_A8, &mpTexture );
			
					if ( mpTexture == NULL )
					{
						PANIC_IF( TRUE, "Error Code #4315 - Alphablended font texture cannot be created. Please report this to the bug report forums." );
						mpTexture = pxTempTexture;
					}
				}
			}
		}

		mpTexture->GetLevelDesc( 0, &xSurface );
		m_TextureSizeY = xSurface.Height;
		m_TextureSizeX = xSurface.Width;
//		PANIC_IF( m_TextureSizeX != 256, "Generated Font width not 256" );
//		PANIC_IF( m_TextureSizeX != nWidth, "Texture width mismatch in font load" );

		pxTempTexture->LockRect( 0, &xLockedRectSrc, NULL, 0 );//&xDestRect, 0 );
		mpTexture->LockRect( 0, &xLockedRectDest, NULL, 0 );//&xDestRect, 0 );

		if ( ( xLockedRectDest.pBits != NULL ) &&
			 ( xLockedRectSrc.pBits != NULL ) )
		{	
			pbSrc = (byte*)( xLockedRectSrc.pBits );
			pbDestination = (byte*)( xLockedRectDest.pBits );

			for ( nY = 0; nY < nHeight; nY++ )
			{
				puwSrc = (ushort*)( pbSrc );
				pulDest = (uint32*)( pbDestination );
				puwDest = (ushort*)( pbDestination );
				pbDest = pbDestination;
				for ( nX = 0; nX < nWidth; nX++ )
				{
					bR = ( ( *(puwSrc) ) >> 10 ) & 0x1F;
					bG = ( ( *(puwSrc) ) >> 5 ) & 0x1F;
					bB = ( ( *(puwSrc) ) ) & 0x1F;

					bR <<= 3;
					bG <<= 3;
					bB <<= 3;

					nAlpha = (bR + bG + bB) / 3;
					bR = 0xFF;
					bG = 0xFF;
					bB = 0xFF;
					switch( nFormat )
					{
					case D3DFMT_A8R8G8B8:
						*(pulDest) = (nAlpha << 24) | (bR << 16 ) | (bG << 8) | (bB);
						break;
					case D3DFMT_A8:
						*(pbDest) = (byte)(nAlpha);
						break;
					case D3DFMT_A4R4G4B4:
						*(puwDest) = ((nAlpha>>4) << 12) | ((bR>>4) << 8 ) | ((bG>>4) << 4) | (bB>>4);
						break;
					case D3DFMT_A8R3G3B2:
						*(puwDest) = (nAlpha << 8) | ((bR>>5) << 5 ) | ((bG>>5) << 2) | (bB>>6);
						break;
					}

					puwSrc++;
					pulDest++;
					puwDest++;
					pbDest++;

				} // FOR X < width

				pbDestination += xLockedRectDest.Pitch;
				pbSrc += xLockedRectSrc.Pitch;		
			}

			pxTempTexture->UnlockRect( 0 );
			mpTexture->UnlockRect( 0);
		}
		pxTempTexture->Release();
	}
}



//--------------------------------------------------------------------------------------------------



/***************************************************************************
 * Function    : DrawFontBufferDX
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
void DrawFontBufferDX( void )
{
int				nDrawHowMany;

	nDrawHowMany = (mnFontVertexIndex / 3);

	if ( nDrawHowMany > 0 )
	{
		mpxCurrentFontVertexBuffer->Unlock();
	
		if ( InterfaceFontSetAsCurrentTexture( mnCurrentRenderFont ) == TRUE )
		{
			EngineSetColourMode( 0, COLOUR_MODE_TEXTURE_MODULATE );

			EngineEnableAlphaTest( 1 );
			EngineEnableBlend( TRUE );
			mpInterfaceD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			mpInterfaceD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		}
		else
		{
			InterfaceFontSetAsCurrentTexture( mnCurrentRenderFont );
			mpInterfaceD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
			mpInterfaceD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			mpInterfaceD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
			mpInterfaceD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
			mpInterfaceD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
			EngineResetColourMode();
			EngineEnableAlphaTest( 0 );
			EngineEnableBlend( TRUE );
			mpInterfaceD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			mpInterfaceD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);
		}

		if ( InterfaceFontIsFilteringOn( mnCurrentRenderFont ) == TRUE )
		{
			InterfaceSetGlobalParam( INTF_TEXTURE_FILTERING, 2 );
		}
		else
		{
			InterfaceSetGlobalParam( INTF_TEXTURE_FILTERING, 0 );
		}

		InterfaceInternalDXSetStreamSource( 0, mpxCurrentFontVertexBuffer, 0, sizeof(FLATVERTEX) );
		mpInterfaceD3DDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, nDrawHowMany );

/*		if ( mpxCurrentFontVertexBuffer == mpxFontVertexBuffer1 )
		{
			mpxCurrentFontVertexBuffer = mpxFontVertexBuffer2;
		}
		else
		{
			mpxCurrentFontVertexBuffer = mpxFontVertexBuffer1;
		}
*/
		if( FAILED( mpxCurrentFontVertexBuffer->Lock( 0, 0, (VERTEX_LOCKTYPE)&mpFontVertices, D3DLOCK_DISCARD ) ) )
		{
			PANIC_IF(TRUE,"Font vertex buffer lock failed" );
		}
	}

	mnFontVertexIndex = 0;
}


void InitialiseFontBuffersDX( void )
{
	if ( mpxFontVertexBuffer1 == NULL )
	{
		if( FAILED( InterfaceInternalDXCreateVertexBuffer( SIZE_OF_FONT_VERTEX_BUFFER * sizeof(FLATVERTEX),
													  D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC, D3DFVF_FLATVERTEX,
													  &mpxFontVertexBuffer1 ) ) )
		{
//			PANIC_IF( TRUE, "Couldnt create font Vertex buffer");
			return;
		}
	}
	mpxCurrentFontVertexBuffer = mpxFontVertexBuffer1;
}



/***************************************************************************
 * Function    : RenderStrings
 * Params      : Layer
 * Description : Whacks out primitives for all the text added during the rendering frame
 ***************************************************************************/
void RenderStrings( int nLayer )
{
int		nLoop;
RECT	xRect;
uint32	ulCol;
int		nFont;
int		nFlag;
float	fTextScale;

	if ( !mpxCurrentFontVertexBuffer ) return;

	InterfaceSetGlobalParam( INTF_TEXTURE_FILTERING, 2 );

	EngineSetVertexFormat( VERTEX_FORMAT_FLATVERTEX );
	EngineEnableLighting( FALSE );
	EngineEnableFog( FALSE );
	EngineEnableCulling( 0 );
	EngineEnableZTest( FALSE );
	EngineEnableZWrite( FALSE );

    if( FAILED( mpxCurrentFontVertexBuffer->Lock( 0, 0, (VERTEX_LOCKTYPE)&mpFontVertices, D3DLOCK_DISCARD ) ) )
	{
		PANIC_IF(TRUE,"Font vertex buffer lock failed" );
        return;
	}

	xRect.right = 0;
	xRect.bottom = 0;

	for ( mnCurrentRenderFont = 0; 	mnCurrentRenderFont < 8; mnCurrentRenderFont++ )
	{
		for ( nLoop = 0; nLoop < mnPosInTextBuffer; nLoop++ )
		{
			if ( maxTextBuffer[ nLoop ].nLayer == nLayer )
			{
				fTextScale = 0.0f;
				nFont = maxTextBuffer[ nLoop ].bFont;
				if ( nFont == mnCurrentRenderFont )
				{
					ulCol = InterfaceGetARGBForColour( maxTextBuffer[ nLoop ].nCol );

					if ( ( maxTextBuffer[ nLoop ].nAlign == ALIGN_LEFT ) ||
						 ( maxTextBuffer[ nLoop ].nAlign == ALIGN_SCALED ) )
					{
						xRect.left = maxTextBuffer[ nLoop ].nX;
						xRect.top = maxTextBuffer[ nLoop ].nY;	

						if ( maxTextBuffer[nLoop].nAlign == ALIGN_SCALED )
						{
							fTextScale = maxTextBuffer[nLoop].fScale;
						}
					}
					else
					{	
						xRect.left		= maxTextBuffer[nLoop].xAlignRect.left;
						xRect.right		= maxTextBuffer[nLoop].xAlignRect.right;
						xRect.top		= maxTextBuffer[nLoop].xAlignRect.top;	
						xRect.bottom	= maxTextBuffer[nLoop].xAlignRect.top + 10;	
					}
					nFlag = maxTextBuffer[nLoop].wFlag;
					switch ( maxTextBuffer[ nLoop ].nCol )
					{
					case COL_PUBLIC_DROP_SHADOW:
					case COL_CHATGROUP_DROP_SHADOW:
					case COL_SYSMSG_DROP_SHADOW:
					case COL_WARNING:
						nFlag |= FONT_FLAG_DROP_SHADOW;
						break;
					default:
						break;
					}
					mnCurrentFontFlags = nFlag;
					FontDrawText( maxTextBuffer[ nLoop ].acString, &xRect, maxTextBuffer[ nLoop ].nAlign, ulCol, nFont, nFlag, fTextScale  );
					if ( nFlag & FONT_FLAG_UNDERLINED )
					{
					int		nWidth = GetStringWidth( maxTextBuffer[ nLoop ].acString, nFont );
					int		nHeight = GetStringHeight( maxTextBuffer[ nLoop ].acString, nFont );
						InterfaceLine( nLayer, xRect.left, xRect.top + nHeight, xRect.left + nWidth, xRect.top + nHeight, ulCol, ulCol );
					}
					mnCurrentFontFlags = 0;
				}
			}
		}
		DrawFontBufferDX();
	}

	mpxCurrentFontVertexBuffer->Unlock();
	InterfaceSetGlobalParam( INTF_TEXTURE_FILTERING, 0 );

}



/***************************************************************************
 * Function    : FreeFont
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
void FreeFont( BOOL bFreeEverything )
{
    if( mpxFontVertexBuffer1 != NULL )
	{
        mpxFontVertexBuffer1->Release();
		mpxFontVertexBuffer1 = NULL;
	}

    if( mpxFontVertexBuffer2 != NULL )
	{
        mpxFontVertexBuffer2->Release();
		mpxFontVertexBuffer2 = NULL;
	}
	mpxCurrentFontVertexBuffer = NULL;

	if ( bFreeEverything )
	{
		FontDefFreeAll();
	}

}


