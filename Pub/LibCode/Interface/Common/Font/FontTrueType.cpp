//-------------------------------------------------------------
// FontTrueType
//   TrueType fonts in the font slots (InterfaceFontLoadTrueType).
//
//   A TrueType font answers the same questions as a bitmap font (advance, cell height, where to
//   draw), but its glyphs are rasterised with stb_truetype for the real pixel size of the 2D space,
//   so they map one texel to one pixel at any UI scale. Metrics are worked out once, in whole
//   2D units, so text measures the same at every scale and hand-placed layouts don't move.
//
//   Only fonts loaded with InterfaceFontLoadTrueType come through here; bitmap fonts take
//   the old paths in FontCommon.cpp unchanged.
//-------------------------------------------------------------
#ifdef TUD9
#include "../../DirectX/InterfaceInternalsDX.h"
#endif

#include <stdio.h>
#include <string.h>
#include <math.h>

#include <StandardDef.h>
#include <Interface.h>
#include <CodeUtil.h>

#include "../InterfaceInstance.h"
#include "../Overlays/Overlays.h"
#include "FontCommon.h"

#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#define	TRUETYPE_ATLAS_PADDING		2
#define	TRUETYPE_MIN_SCALE			0.25f
#define	TRUETYPE_MAX_SCALE			8.0f

extern float	mfGiantFontScale;		// FontCommon.cpp

//-------------------------------------------------------------
// Which characters a TrueType font covers: the same as the bitmap fonts (32-126 and the pound sign)
//-------------------------------------------------------------
static int		FontTrueTypeCodepointForByte( int nByte )
{
	if ( ( nByte >= 32 ) && ( nByte <= 126 ) )
	{
		return( nByte );
	}
	if ( nByte == 163 )
	{
		return( 0xA3 );
	}
	return( 0 );
}

//-------------------------------------------------------------
// CFontDef::FreeTrueType
//-------------------------------------------------------------
void	CFontDef::FreeTrueType( void )
{
	if ( mpTrueTypeInfo )
	{
		free( mpTrueTypeInfo );
		mpTrueTypeInfo = NULL;
	}
	if ( mpbTrueTypeData )
	{
		free( mpbTrueTypeData );
		mpbTrueTypeData = NULL;
	}
	if ( mpbFallbackCoverage )
	{
		free( mpbFallbackCoverage );
		mpbFallbackCoverage = NULL;
	}
	mbIsTrueType = FALSE;
}

//-------------------------------------------------------------
// CFontDef::InitialiseTrueTypeFallback
//   Chars the TrueType font hasn't got (such as the icons some bitmap fonts carry above 127) come
//   from a bitmap font instead: its image is kept as coverage and scaled into each bake
//-------------------------------------------------------------
void	CFontDef::InitialiseTrueTypeFallback( InterfaceInstance* pInterfaceInstance, const char* szImage, const char* szLayout )
{
CFontDef*	pLayoutDef;
int			nLoop;
int			nLen = (int)strlen( szLayout );

	if ( ( nLen > 4 ) && ( stricmp( szLayout + nLen - 4, "json" ) == 0 ) )
	{
		// Only the fontrast (cell) layout fits a TrueType font's cells
		return;
	}
	mpbFallbackCoverage = LoadCoverageImage( pInterfaceInstance, szImage, &mnFallbackWidth, &mnFallbackHeight );
	if ( mpbFallbackCoverage == NULL )
	{
		return;
	}

	pLayoutDef = new CFontDef;
	pLayoutDef->m_TextureSizeX = mnFallbackWidth;
	pLayoutDef->m_TextureSizeY = mnFallbackHeight;
	pLayoutDef->ReadFontrastFile( szLayout );
	for ( nLoop = 32; nLoop < 256; nLoop++ )
	{
	FONT_UVCHAR*	pxUV = &pLayoutDef->maFontLookup[nLoop];

		if ( ( maTrueTypeGlyphs[nLoop].nGlyphIndex == 0 ) &&
			 ( pxUV->w > 0.0f ) &&
			 ( pxUV->h > 0.0f ) )
		{
			// ReadFontrastFile always takes the image as 256 wide
			maTrueTypeGlyphs[nLoop].bFromFallback = TRUE;
			maTrueTypeGlyphs[nLoop].nFallbackX = (int)( ( pxUV->u * 256.0f ) + 0.5f );
			maTrueTypeGlyphs[nLoop].nFallbackY = (int)( ( pxUV->v * mnFallbackHeight ) + 0.5f );
			maTrueTypeGlyphs[nLoop].nFallbackW = (int)( ( pxUV->w * 256.0f ) + 0.5f );
			maTrueTypeGlyphs[nLoop].nFallbackH = (int)( ( pxUV->h * mnFallbackHeight ) + 0.5f );
			maTrueTypeGlyphs[nLoop].nAdvance = maTrueTypeGlyphs[nLoop].nFallbackW + 1;	// As the bitmap font
		}
	}
	delete pLayoutDef;
}

//-------------------------------------------------------------
// Scales a fallback char's rect of the coverage image into the atlas (bilinear)
//-------------------------------------------------------------
static void		FontTrueTypeScaleFallback( const byte* pbSrc, int nSrcStride, int nSrcHeight, const FONT_TTGLYPH* pxGlyph,
										   byte* pbDest, int nDestStride, int nDestW, int nDestH )
{
int		nX;
int		nY;
int		nSX;
int		nSY;
float	fSX;
float	fSY;
float	fFX;
float	fFY;
float	fTop;
float	fBottom;

	for ( nY = 0; nY < nDestH; nY++ )
	{
		fSY = ( ( ( nY + 0.5f ) * pxGlyph->nFallbackH ) / nDestH ) - 0.5f;
		if ( fSY < 0.0f ) fSY = 0.0f;
		nSY = (int)fSY;
		fFY = fSY - nSY;
		for ( nX = 0; nX < nDestW; nX++ )
		{
		int		nSX1;
		int		nSY1;

			fSX = ( ( ( nX + 0.5f ) * pxGlyph->nFallbackW ) / nDestW ) - 0.5f;
			if ( fSX < 0.0f ) fSX = 0.0f;
			nSX = (int)fSX;
			fFX = fSX - nSX;
			nSX1 = ( nSX + 1 < pxGlyph->nFallbackW ) ? nSX + 1 : nSX;
			nSY1 = ( nSY + 1 < pxGlyph->nFallbackH ) ? nSY + 1 : nSY;
			nSX += pxGlyph->nFallbackX;
			nSX1 += pxGlyph->nFallbackX;
			if ( pxGlyph->nFallbackY + nSY1 >= nSrcHeight ) continue;

			fTop = ( pbSrc[ ( ( pxGlyph->nFallbackY + nSY ) * nSrcStride ) + nSX ] * ( 1.0f - fFX ) ) + ( pbSrc[ ( ( pxGlyph->nFallbackY + nSY ) * nSrcStride ) + nSX1 ] * fFX );
			fBottom = ( pbSrc[ ( ( pxGlyph->nFallbackY + nSY1 ) * nSrcStride ) + nSX ] * ( 1.0f - fFX ) ) + ( pbSrc[ ( ( pxGlyph->nFallbackY + nSY1 ) * nSrcStride ) + nSX1 ] * fFX );
			pbDest[ ( nY * nDestStride ) + nX ] = (byte)( ( fTop * ( 1.0f - fFY ) ) + ( fBottom * fFY ) + 0.5f );
		}
	}
}

//-------------------------------------------------------------
// CFontDef::InitialiseTrueType
//   Reads the font file and works out the scale-independent metrics. Doesn't make a texture
//-------------------------------------------------------------
BOOL	CFontDef::InitialiseTrueType( InterfaceInstance* pInterfaceInstance, const char* szTTFFilename, const INTERFACE_TRUETYPE_PARAMS* pxParams )
{
FILE*			pFile;
int				nFileSize;
stbtt_fontinfo*	pxInfo;
int				nAscent;
int				nDescent;
int				nLineGap;
float			fFontHeight;
int				nLoop;
int				nCodepoint;
int				nAdvance;
int				nLeftBearing;
float			fAdvance;

	if ( ( szTTFFilename == NULL ) ||
		 ( pxParams == NULL ) ||
		 ( pxParams->fPixelHeight <= 0.0f ) )
	{
		return( FALSE );
	}

	FreeTrueType();

	pFile = fopen( szTTFFilename, "rb" );
	if ( pFile == NULL )
	{
		return( FALSE );
	}
	nFileSize = SysGetFileSize( pFile );
	if ( nFileSize <= 0 )
	{
		fclose( pFile );
		return( FALSE );
	}
	mpbTrueTypeData = (byte*)malloc( nFileSize );
	if ( fread( mpbTrueTypeData, nFileSize, 1, pFile ) != 1 )
	{
		fclose( pFile );
		FreeTrueType();
		return( FALSE );
	}
	fclose( pFile );

	pxInfo = (stbtt_fontinfo*)malloc( sizeof(stbtt_fontinfo) );
	mpTrueTypeInfo = pxInfo;
	if ( stbtt_InitFont( pxInfo, mpbTrueTypeData, stbtt_GetFontOffsetForIndex( mpbTrueTypeData, 0 ) ) == 0 )
	{
		FreeTrueType();
		return( FALSE );
	}

	mfTrueTypeScale = stbtt_ScaleForPixelHeight( pxInfo, pxParams->fPixelHeight );
	stbtt_GetFontVMetrics( pxInfo, &nAscent, &nDescent, &nLineGap );
	fFontHeight = ( nAscent - nDescent ) * mfTrueTypeScale;

	mnTrueTypeCellHeight = pxParams->nCellHeight;
	if ( mnTrueTypeCellHeight <= 0 )
	{
		mnTrueTypeCellHeight = (int)ceilf( fFontHeight );
	}
	mfTrueTypeBaseline = pxParams->fBaselineY;
	if ( mfTrueTypeBaseline <= 0.0f )
	{
		mfTrueTypeBaseline = ( ( mnTrueTypeCellHeight - fFontHeight ) * 0.5f ) + ( nAscent * mfTrueTypeScale );
	}

	ZeroMemory( maTrueTypeGlyphs, sizeof( maTrueTypeGlyphs ) );
	for ( nLoop = 0; nLoop < 256; nLoop++ )
	{
		nCodepoint = FontTrueTypeCodepointForByte( nLoop );
		if ( nCodepoint != 0 )
		{
			maTrueTypeGlyphs[nLoop].nGlyphIndex = stbtt_FindGlyphIndex( pxInfo, nCodepoint );
		}

		if ( maTrueTypeGlyphs[nLoop].nGlyphIndex != 0 )
		{
			stbtt_GetGlyphHMetrics( pxInfo, maTrueTypeGlyphs[nLoop].nGlyphIndex, &nAdvance, &nLeftBearing );
			fAdvance = ( nAdvance * mfTrueTypeScale ) + pxParams->fLetterSpacing;
			maTrueTypeGlyphs[nLoop].nAdvance = (int)floorf( fAdvance + 0.5f );
			if ( maTrueTypeGlyphs[nLoop].nAdvance < 1 )
			{
				maTrueTypeGlyphs[nLoop].nAdvance = 1;
			}
		}
		else
		{
			// Same as a bitmap font with no entry for the char
			maTrueTypeGlyphs[nLoop].nAdvance = 3;
		}
	}
	if ( pxParams->nSpaceAdvance > 0 )
	{
		maTrueTypeGlyphs[' '].nAdvance = pxParams->nSpaceAdvance;
	}
	if ( ( pxParams->szFallbackImage ) &&
		 ( pxParams->szFallbackLayout ) )
	{
		InitialiseTrueTypeFallback( pInterfaceInstance, pxParams->szFallbackImage, pxParams->szFallbackLayout );
	}

	mbIsTrueType = TRUE;
	mfTrueTypeBakedScaleX = 0.0f;
	mfTrueTypeBakedScaleY = 0.0f;
	return( TRUE );
}

//-------------------------------------------------------------
// CFontDef::BakeTrueType
//   Rasterises every glyph for fScaleX/Y render pixels per 2D unit into a new texture
//-------------------------------------------------------------
BOOL	CFontDef::BakeTrueType( InterfaceInstance* pInterfaceInstance, float fScaleX, float fScaleY )
{
stbtt_fontinfo*	pxInfo = (stbtt_fontinfo*)mpTrueTypeInfo;
float			fPixelScaleX;
float			fPixelScaleY;
int				anX0[256];
int				anY0[256];
int				anX1[256];
int				anY1[256];
int				anPosX[256];
int				anPosY[256];
int				nAtlasWidth;
int				nAtlasHeight;
int				nPenX;
int				nPenY;
int				nRowHeight;
int				nUsedHeight;
int				nLoop;
int				nW;
int				nH;
byte*			pbAlpha;
BOOL			bFits = FALSE;
BOOL			bResult;

	if ( ( mbIsTrueType == FALSE ) ||
		 ( pxInfo == NULL ) )
	{
		return( FALSE );
	}
	if ( fScaleX < TRUETYPE_MIN_SCALE ) fScaleX = TRUETYPE_MIN_SCALE;
	if ( fScaleX > TRUETYPE_MAX_SCALE ) fScaleX = TRUETYPE_MAX_SCALE;
	if ( fScaleY < TRUETYPE_MIN_SCALE ) fScaleY = TRUETYPE_MIN_SCALE;
	if ( fScaleY > TRUETYPE_MAX_SCALE ) fScaleY = TRUETYPE_MAX_SCALE;

	fPixelScaleX = mfTrueTypeScale * fScaleX;
	fPixelScaleY = mfTrueTypeScale * fScaleY;

	for ( nLoop = 0; nLoop < 256; nLoop++ )
	{
		anX0[nLoop] = anY0[nLoop] = anX1[nLoop] = anY1[nLoop] = 0;
		if ( maTrueTypeGlyphs[nLoop].nGlyphIndex != 0 )
		{
			stbtt_GetGlyphBitmapBox( pxInfo, maTrueTypeGlyphs[nLoop].nGlyphIndex, fPixelScaleX, fPixelScaleY,
									&anX0[nLoop], &anY0[nLoop], &anX1[nLoop], &anY1[nLoop] );
		}
		else if ( ( maTrueTypeGlyphs[nLoop].bFromFallback ) &&
				  ( mpbFallbackCoverage ) )
		{
			// A bitmap font's char fills its cell from the top, so it goes up from the baseline by the baseline's depth
			anX0[nLoop] = 0;
			anY0[nLoop] = -(int)floorf( ( mfTrueTypeBaseline * fScaleY ) + 0.5f );
			anX1[nLoop] = (int)floorf( ( maTrueTypeGlyphs[nLoop].nFallbackW * fScaleX ) + 0.5f );
			anY1[nLoop] = anY0[nLoop] + (int)floorf( ( maTrueTypeGlyphs[nLoop].nFallbackH * fScaleY ) + 0.5f );
		}
	}

	// Shelf-pack into the narrowest power-of-two square-ish texture they fit
	nAtlasHeight = 0;
	for ( nAtlasWidth = 256; ( nAtlasWidth <= 4096 ) && ( bFits == FALSE ); nAtlasWidth *= 2 )
	{
		nPenX = TRUETYPE_ATLAS_PADDING;
		nPenY = TRUETYPE_ATLAS_PADDING;
		nRowHeight = 0;
		for ( nLoop = 0; nLoop < 256; nLoop++ )
		{
			nW = anX1[nLoop] - anX0[nLoop];
			nH = anY1[nLoop] - anY0[nLoop];
			anPosX[nLoop] = 0;
			anPosY[nLoop] = 0;
			if ( ( nW <= 0 ) || ( nH <= 0 ) ) continue;

			if ( nPenX + nW + TRUETYPE_ATLAS_PADDING > nAtlasWidth )
			{
				nPenX = TRUETYPE_ATLAS_PADDING;
				nPenY += nRowHeight + TRUETYPE_ATLAS_PADDING;
				nRowHeight = 0;
			}
			anPosX[nLoop] = nPenX;
			anPosY[nLoop] = nPenY;
			nPenX += nW + TRUETYPE_ATLAS_PADDING;
			if ( nH > nRowHeight ) nRowHeight = nH;
		}
		nUsedHeight = nPenY + nRowHeight + TRUETYPE_ATLAS_PADDING;
		nAtlasHeight = 16;
		while ( nAtlasHeight < nUsedHeight )
		{
			nAtlasHeight *= 2;
		}
		if ( nAtlasHeight <= nAtlasWidth )
		{
			bFits = TRUE;
			break;
		}
	}
	if ( bFits == FALSE )
	{
		return( FALSE );
	}

	pbAlpha = (byte*)malloc( nAtlasWidth * nAtlasHeight );
	ZeroMemory( pbAlpha, nAtlasWidth * nAtlasHeight );
	ZeroMemory( maFontLookup, 256 * sizeof(FONT_UVCHAR) );

	for ( nLoop = 0; nLoop < 256; nLoop++ )
	{
		nW = anX1[nLoop] - anX0[nLoop];
		nH = anY1[nLoop] - anY0[nLoop];
		maTrueTypeGlyphs[nLoop].nX0 = anX0[nLoop];
		maTrueTypeGlyphs[nLoop].nY0 = anY0[nLoop];
		if ( ( nW > 0 ) && ( nH > 0 ) )
		{
			if ( maTrueTypeGlyphs[nLoop].nGlyphIndex != 0 )
			{
				stbtt_MakeGlyphBitmap( pxInfo, pbAlpha + ( anPosY[nLoop] * nAtlasWidth ) + anPosX[nLoop], nW, nH, nAtlasWidth,
										fPixelScaleX, fPixelScaleY, maTrueTypeGlyphs[nLoop].nGlyphIndex );
			}
			else
			{
				FontTrueTypeScaleFallback( mpbFallbackCoverage, mnFallbackWidth, mnFallbackHeight, &maTrueTypeGlyphs[nLoop],
										   pbAlpha + ( anPosY[nLoop] * nAtlasWidth ) + anPosX[nLoop], nAtlasWidth, nW, nH );
			}
			maTrueTypeGlyphs[nLoop].nWidth = nW;
			maTrueTypeGlyphs[nLoop].nHeight = nH;

			// Also fill in the UV lookup, for code that reads it (InterfaceFontLookupChar)
			maFontLookup[nLoop].u = (float)anPosX[nLoop] / nAtlasWidth;
			maFontLookup[nLoop].v = (float)anPosY[nLoop] / nAtlasHeight;
			maFontLookup[nLoop].w = (float)nW / nAtlasWidth;
			maFontLookup[nLoop].h = (float)nH / nAtlasHeight;
		}
		else
		{
			maTrueTypeGlyphs[nLoop].nWidth = 0;
			maTrueTypeGlyphs[nLoop].nHeight = 0;
		}
		maFontLookup[nLoop].advance = (float)maTrueTypeGlyphs[nLoop].nAdvance;
	}

	FreeTexture( pInterfaceInstance );
	bResult = CreateTextureFromAlpha( pInterfaceInstance, pbAlpha, nAtlasWidth, nAtlasHeight );
	free( pbAlpha );

	mfTrueTypeBakedScaleX = fScaleX;
	mfTrueTypeBakedScaleY = fScaleY;
	return( bResult );
}

#ifndef TUD9
BOOL	CFontDef::CreateTextureFromAlpha( InterfaceInstance* pInterfaceInstance, const byte* pbAlpha, int nWidth, int nHeight )
{
	return( FALSE );
}

byte*	CFontDef::LoadCoverageImage( InterfaceInstance* pInterfaceInstance, const char* szFilename, int* pnWidth, int* pnHeight )
{
	return( NULL );
}
#endif

//-------------------------------------------------------------
// FontSystem: loading
//-------------------------------------------------------------
BOOL	FontSystem::LoadTrueTypeFont( int nFontNum, const char* szTTFFilename, const INTERFACE_TRUETYPE_PARAMS* pxParams )
{
#ifdef TUD9
CFontDef*	pNewDef;
float		fScaleX;
float		fScaleY;

	if ( ( nFontNum < 0 ) || ( nFontNum >= MAX_FONTS_IN_GAME ) )
	{
		return( FALSE );
	}

	pNewDef = new CFontDef;
	GetTrueTypeRenderScale( &fScaleX, &fScaleY );
	if ( ( pNewDef->InitialiseTrueType( mpInterfaceInstance, szTTFFilename, pxParams ) == FALSE ) ||
		 ( pNewDef->BakeTrueType( mpInterfaceInstance, fScaleX, fScaleY ) == FALSE ) )
	{
		delete pNewDef;
		return( FALSE );
	}

	if ( mpFontDefs[nFontNum] )
	{
		delete mpFontDefs[nFontNum];
	}
	mpFontDefs[nFontNum] = pNewDef;
	return( TRUE );
#else
	return( FALSE );
#endif
}

BOOL	FontSystem::IsFontTrueType( int nFontNum )
{
	if ( ( nFontNum >= 0 ) &&
		 ( nFontNum < MAX_FONTS_IN_GAME ) &&
		 ( mpFontDefs[nFontNum] ) )
	{
		return( mpFontDefs[nFontNum]->IsTrueType() );
	}
	return( FALSE );
}

BOOL	InterfaceFontLoadTrueType( int nFontNum, const char* szTTFFilename, const INTERFACE_TRUETYPE_PARAMS* pxParams )
{
	return( InterfaceInstanceMain()->mpFontSystem->LoadTrueTypeFont( nFontNum, szTTFFilename, pxParams ) );
}

//-------------------------------------------------------------
// CFontDef::GetTrueTypeInkExtent
//   The outlines' reach above and below the baseline, from the font units (not the baked bitmaps)
//-------------------------------------------------------------
BOOL	CFontDef::GetTrueTypeInkExtent( const char* szString, float* pfAbove, float* pfBelow )
{
stbtt_fontinfo*	pxInfo = (stbtt_fontinfo*)mpTrueTypeInfo;
int					nX0;
int					nY0;
int					nX1;
int					nY1;
int					nMaxY = 0;
int					nMinY = 0;
BOOL				bFound = FALSE;
const FONT_TTGLYPH*	pxGlyph;

	if ( ( mbIsTrueType == FALSE ) || ( pxInfo == NULL ) || ( szString == NULL ) )
	{
		return( FALSE );
	}
	while ( *szString )
	{
		pxGlyph = &maTrueTypeGlyphs[ (BYTE)*szString ];
		if ( ( pxGlyph->nGlyphIndex != 0 ) &&
			 ( stbtt_GetGlyphBox( pxInfo, pxGlyph->nGlyphIndex, &nX0, &nY0, &nX1, &nY1 ) ) )
		{
			if ( ( bFound == FALSE ) || ( nY1 > nMaxY ) ) nMaxY = nY1;
			if ( ( bFound == FALSE ) || ( nY0 < nMinY ) ) nMinY = nY0;
			bFound = TRUE;
		}
		szString++;
	}
	if ( bFound )
	{
		*pfAbove = nMaxY * mfTrueTypeScale;
		*pfBelow = -nMinY * mfTrueTypeScale;
	}
	return( bFound );
}

BOOL	InterfaceFontGetTrueTypeInkExtent( int nFontNum, const char* szString, float* pfAboveBaseline, float* pfBelowBaseline )
{
	return( InterfaceInstanceMain()->mpFontSystem->GetTrueTypeInkExtent( nFontNum, szString, pfAboveBaseline, pfBelowBaseline ) );
}

BOOL	FontSystem::GetTrueTypeInkExtent( int nFontNum, const char* szString, float* pfAbove, float* pfBelow )
{
	if ( IsFontTrueType( nFontNum ) == FALSE )
	{
		return( FALSE );
	}
	return( mpFontDefs[nFontNum]->GetTrueTypeInkExtent( szString, pfAbove, pfBelow ) );
}

BOOL	InterfaceFontIsTrueType( int nFontNum )
{
	return( InterfaceInstanceMain()->mpFontSystem->IsFontTrueType( nFontNum ) );
}

//-------------------------------------------------------------
// GetTrueTypeRenderScale
//   Render pixels per 2D unit, as the 2D projection actually maps them (InterfaceInstance::Draw)
//-------------------------------------------------------------
void	FontSystem::GetTrueTypeRenderScale( float* pfScaleX, float* pfScaleY )
{
int		nWidth = mpInterfaceInstance->GetWidth();
int		nHeight = mpInterfaceInstance->GetHeight();

	*pfScaleX = 1.0f;
	*pfScaleY = 1.0f;
	if ( ( nWidth > 0 ) && ( nHeight > 0 ) )
	{
		*pfScaleX = (float)mpInterfaceInstance->GetRenderWidth() / nWidth;
		*pfScaleY = (float)mpInterfaceInstance->GetRenderHeight() / nHeight;
	}
}

//-------------------------------------------------------------
// UpdateTrueTypeFonts
//   Called before strings are drawn: re-bakes any TrueType font made for a different scale
//-------------------------------------------------------------
void	FontSystem::UpdateTrueTypeFonts( void )
{
float	fScaleX;
float	fScaleY;
int		nLoop;

	GetTrueTypeRenderScale( &fScaleX, &fScaleY );
	for ( nLoop = 0; nLoop < MAX_FONTS_IN_GAME; nLoop++ )
	{
		if ( ( mpFontDefs[nLoop] ) &&
			 ( mpFontDefs[nLoop]->IsTrueType() ) &&
			 ( mpFontDefs[nLoop]->TrueTypeNeedsBake( fScaleX, fScaleY ) ) )
		{
			mpFontDefs[nLoop]->BakeTrueType( mpInterfaceInstance, fScaleX, fScaleY );
		}
	}
}

//-------------------------------------------------------------
// Metrics
//-------------------------------------------------------------

// The size multiplier the FONT_FLAG_ size flags give (matches FontGetCharUWidthAndScreenWidth)
float	FontSystem::GetFontFlagScaling( int nFlags )
{
	if ( nFlags & FONT_FLAG_SMALL )
	{
		return( 0.75f );
	}
	else if ( nFlags & FONT_FLAG_TINY )
	{
		return( 0.25f );
	}
	else if ( nFlags & FONT_FLAG_GIANT )
	{
		return( mfGiantFontScale );
	}
	else if ( nFlags & FONT_FLAG_ENORMOUS )
	{
		return( 5.0f );
	}
	else if ( nFlags & FONT_FLAG_MASSIVE )
	{
		return( 3.1f );
	}
	else if ( nFlags & FONT_FLAG_LARGE )
	{
		return( 1.25f );
	}
	return( 1.0f );
}

// The height of a char's cell at normal size: a bitmap font's texture rows, a TrueType font's cell
float	FontSystem::GetFontCellHeight( BYTE cChar, int nFont )
{
	if ( ( nFont >= 0 ) &&
		 ( nFont < MAX_FONTS_IN_GAME ) &&
		 ( mpFontDefs[nFont] ) &&
		 ( mpFontDefs[nFont]->IsTrueType() ) )
	{
		return( (float)mpFontDefs[nFont]->GetTrueTypeCellHeight() );
	}
	return( GetFontVHeight( cChar, nFont ) * GetFontTextureHeight( nFont ) );
}

// Answers as FontGetCharUWidthAndScreenWidth does for a bitmap font: callers add 1 to the
// screen width and the advance, so a TrueType font gives its advance less 1
float	FontSystem::FontGetTrueTypeCharWidth( BYTE cChar, int nFont, int nFlags, float* pfUWidth, int* pnScrWidth, float *pfAdvance )
{
const FONT_TTGLYPH*	pxGlyph = mpFontDefs[nFont]->GetTrueTypeGlyph( cChar );
float		fScreenScaling = GetFontFlagScaling( nFlags );
float		fWidth = pxGlyph->nAdvance * fScreenScaling;
FONT_UVCHAR	xUVChar;

	if ( pfAdvance )
	{
		*pfAdvance = fWidth - 1.0f;
	}
	if ( pnScrWidth )
	{
		*pnScrWidth = (int)( fWidth + 0.5f ) - 1;
	}
	if ( pfUWidth )
	{
		mpFontDefs[nFont]->LookupChar( cChar, &xUVChar );
		*pfUWidth = xUVChar.w;
	}
	return( fScreenScaling );
}

//-------------------------------------------------------------
// FontDrawTrueTypeChar
//   nX, nY is the top left of the char's cell, as for a bitmap font. At normal size the glyph is
//   snapped to the render pixel grid so each texel lands on one pixel
//-------------------------------------------------------------
float	FontSystem::FontDrawTrueTypeChar( BYTE cChar, int nX, int nY, uint32 ulCol, int nFont, int nFlag )
{
CFontDef*			pFontDef = mpFontDefs[nFont];
const FONT_TTGLYPH*	pxGlyph = pFontDef->GetTrueTypeGlyph( cChar );
float				fScreenScaling = GetFontFlagScaling( nFlag );
float				fScaleX = pFontDef->GetTrueTypeBakedScaleX();
float				fScaleY = pFontDef->GetTrueTypeBakedScaleY();
float				fCellTop = (float)( nY + pFontDef->GetGlobalVerticalOffset() );
float				fPenPixelX;
float				fBaselinePixelY;
float				fShadowOffset;
float				fMainAlpha;
FLOATRECT			xScrRect;
FLOATRECT			xScrRect2;
FLOATRECT			xTexRect;
FONT_UVCHAR			xUVChar;

	if ( ( pxGlyph->nWidth > 0 ) &&
		 ( pxGlyph->nHeight > 0 ) &&
		 ( fScaleX > 0.0f ) &&
		 ( fScaleY > 0.0f ) )
	{
		pFontDef->LookupChar( cChar, &xUVChar );
		xTexRect.x1 = xUVChar.u;
		xTexRect.x2 = xUVChar.u + xUVChar.w;
		xTexRect.y1 = xUVChar.v;
		xTexRect.y2 = xUVChar.v + xUVChar.h;

		if ( fScreenScaling == 1.0f )
		{
			// D3D9 samples at pixel centres, so the quad's edges go half a pixel up and left
			fPenPixelX = floorf( ( nX * fScaleX ) + 0.5f );
			fBaselinePixelY = floorf( ( ( fCellTop + pFontDef->GetTrueTypeBaseline() ) * fScaleY ) + 0.5f );
			xScrRect.x1 = ( fPenPixelX + pxGlyph->nX0 - 0.5f ) / fScaleX;
			xScrRect.y1 = ( fBaselinePixelY + pxGlyph->nY0 - 0.5f ) / fScaleY;
		}
		else
		{
			xScrRect.x1 = nX + ( ( pxGlyph->nX0 / fScaleX ) * fScreenScaling );
			xScrRect.y1 = fCellTop + ( ( pFontDef->GetTrueTypeBaseline() + ( pxGlyph->nY0 / fScaleY ) ) * fScreenScaling );
		}
		xScrRect.x2 = xScrRect.x1 + ( ( pxGlyph->nWidth / fScaleX ) * fScreenScaling );
		xScrRect.y2 = xScrRect.y1 + ( ( pxGlyph->nHeight / fScaleY ) * fScreenScaling );

		// As the bitmap fonts: bold draws the char twice
		if ( nFlag & FONT_FLAG_BOLD )
		{
			AddCharVertices( &xScrRect, &xTexRect, ulCol, nFlag );
		}
		if ( nFlag & FONT_FLAG_DROP_SHADOW )
		{
			fShadowOffset = 2.0f;
			if ( nFlag & FONT_FLAG_ENORMOUS )
			{
				fShadowOffset = 4.0f;
			}
			else if ( nFlag & FONT_FLAG_MASSIVE )
			{
				fShadowOffset = 3.0f;
			}
			else if ( ( nFlag & FONT_FLAG_SMALL ) ||
					  ( nFlag & FONT_FLAG_TINY ) )
			{
				fShadowOffset = 1.0f;
			}
			xScrRect2 = xScrRect;
			xScrRect2.x1 += fShadowOffset;
			xScrRect2.y1 += fShadowOffset;
			xScrRect2.x2 += fShadowOffset;
			xScrRect2.y2 += fShadowOffset;
			fMainAlpha = (float)( ulCol >> 24 ) / 255.0f;
			AddCharVertices( &xScrRect2, &xTexRect, GetColWithModifiedAlpha( 0xA0000000, fMainAlpha ), nFlag );
		}
		AddCharVertices( &xScrRect, &xTexRect, ulCol, nFlag );
	}

	if ( IsFontFixedWidth( nFont ) == TRUE )
	{
		return( 10 );
	}
	return( ( pxGlyph->nAdvance * fScreenScaling ) - pFontDef->mnFontOccupyWidthReduction );
}
