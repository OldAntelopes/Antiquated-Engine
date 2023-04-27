#include <stdio.h>
#include <string.h>

#include <windows.h>			// For OpenGL
#include <gl/gl.h>

#include <StandardDef.h>
#include <Interface.h>

#include "TextureLoader/InterfaceTextureManager.h"
#include "../Common/InterfaceUtil.h"
#include "../Common/Overlays/Overlays.h"
#include "../Common/Font/FontCommon.h"
#include "FontGL.h"

int							mnCurrentRenderFont = 0;

void InitialiseFontBuffersGL( void )
{
	mpFontVertices = (FLATVERTEX*)( malloc( sizeof( FLATVERTEX) * SIZE_OF_FONT_VERTEX_BUFFER) );
}


///--------------------------------------------------------------
//CFontDef::LoadTexture
// Loads the texture file specified using SetTextureFileName
// The image is expected to be a greyscale. This load function uses the brightness
// of the image to generate an alpha map used for blending the font
//---------------------------------------------------------------
void	CFontDef::LoadTexture( void )
{
	mhTexture = InterfaceGetTexture( m_szTextureFilename, 0 );
}

void	CFontDef::SetTextureAsCurrent( void )
{
	InterfaceSetTextureAsCurrent( mhTexture );
}


/***************************************************************************
 * Function    : DrawFontBufferGL
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
void DrawFontBufferGL( void )
{
int				nDrawHowMany;
int				nTriLoop;

	nDrawHowMany = (mnFontVertexIndex / 3);

	if ( nDrawHowMany > 0 )
	{
	float	fItalicOffset = 0.0f;
	float	fR, fG, fB, fA;
	FLATVERTEX*		pVertices = mpFontVertices;
	FLATVERTEX*		pVert1;
	FLATVERTEX*		pVert2;
	FLATVERTEX*		pVert3;

//		if ( nFlag & FONT_FLAG_ITALIC )
//		{
//			fItalicOffset = 3.0f;
//		}

		glBegin (GL_TRIANGLES);

		for( nTriLoop = 0; nTriLoop < nDrawHowMany; nTriLoop++ )
		{	
			pVert1 = pVertices;
			pVert2 = pVertices + 1;
			pVert3 = pVertices + 2;

			InterfaceUnpackCol( pVertices->color, &fR, &fG, &fB, &fA );
			glColor4f( fR, fG, fB, fA );

			glTexCoord2f (pVert1->tu, 1.0f-pVert1->tv);
			glVertex3f (pVert1->x + fItalicOffset, pVert1->y, 0.0f);
	
			glTexCoord2f (pVert2->tu, 1.0f-pVert2->tv);
			glVertex3f (pVert2->x + fItalicOffset, pVert2->y, 0.0f);

			glTexCoord2f (pVert3->tu, 1.0f-pVert3->tv);
			glVertex3f (pVert3->x + fItalicOffset, pVert3->y, 0.0f);

			pVertices += 3;
		}

		glEnd ();

	}

	mnFontVertexIndex = 0;
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
uint32		nCol;
int		nFont;
int		nFlag;
int		nCurrentBindTexture = -1;
float	fTextScale;

	// TODO - SHould move this function to common and insert only the bits we need to 
	
	glEnable (GL_TEXTURE_2D); 
	glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR );

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
					nCol = InterfaceGetARGBForColour( maxTextBuffer[ nLoop ].nCol );

					if ( maxTextBuffer[ nLoop ].nAlign == ALIGN_LEFT )
					{
						xRect.left = maxTextBuffer[ nLoop ].nX;
						xRect.top = maxTextBuffer[ nLoop ].nY;	
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

					if ( nCurrentBindTexture != mnCurrentRenderFont )
					{
						InterfaceFontSetAsCurrentTexture( mnCurrentRenderFont );
						nCurrentBindTexture = mnCurrentRenderFont;
					}

					FontDrawText( maxTextBuffer[ nLoop ].acString, &xRect, maxTextBuffer[ nLoop ].nAlign, nCol, nFont, nFlag, fTextScale  );
//TODO					if ( nFlag & FONT_FLAG_UNDERLINED )
//TODO					{
//TODO					int		nWidth = GetStringWidth( maxTextBuffer[ nLoop ].acString, nFont );
//TODO					int		nHeight = GetStringHeight( maxTextBuffer[ nLoop ].acString, nFont );
//TODO						InterfaceLine( nLayer, xRect.left, xRect.top + nHeight, xRect.left + nWidth, xRect.top + nHeight, nCol, nCol );
//TODO					}
					mnCurrentFontFlags = 0;
				}
			}
		}
		DrawFontBufferGL();
	}

    glDisable (GL_TEXTURE_2D); /* disable texture mapping */

}
