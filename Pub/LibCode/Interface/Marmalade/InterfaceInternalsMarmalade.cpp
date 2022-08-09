
//#include "Iw2d.h"
#include "s3e.h"
#include "IwGx.h"
#include "IwGxFont.h"
#include "IwMaterial.h"

#include "StandardDef.h"
#include "Interface.h"
#include "Engine.h"
#include "../LibCode/Interface/Common/Font/StringList.h"
#include "../LibCode/Interface/Common/InterfaceUtil.h"
#include "../LibCode/Interface/Common/InterfaceTexturedPolyLists.h"
#include "../LibCode/Interface/Common/InterfaceTextureList.h"
#include "InterfaceInternalsMarmalade.h"
#include "InterfaceTexturesMarmalade.h"

//#define USE_EGL_INIT

#define		TRI_VERT_BUFFER_SIZE			32768
#define		LINE_VERT_BUFFER_SIZE			8192
#define		TEXTURED_TRI_VERT_BUFFER_SIZE	8192

static EGLSurface g_EGLSurface = NULL;
static EGLDisplay g_EGLDisplay = NULL;
static EGLDisplay g_EGLContext = NULL;
bool g_glIsInitialized = false;

CIwGxFontPreparedData		msxPreparedFontData;


CIwMaterial*	mpUntexturedOverlaysMat = NULL;

class UntexturedOverlayBuffer
{
public:
	UntexturedOverlayBuffer()
	{
		mpVertBuffer = NULL;
		mpColourBuffer = NULL;
		mnNumVertsInBuffer = 0;
	}
	
	void		Release( void )
	{
		if ( mpVertBuffer )
		{
			delete [] mpVertBuffer;
			delete [] mpColourBuffer;
		}
	}

	void		Init( int nBufferSize )
	{
		mpVertBuffer = new CIwFVec2[ nBufferSize ];
		mpColourBuffer = new CIwColour[ nBufferSize ];
		mnNumVertsInBuffer = 0;
	}

	CIwFVec2*		mpVertBuffer;
	CIwColour*		mpColourBuffer;
	int				mnNumVertsInBuffer;
};

UntexturedOverlayBuffer		masTriOverlayBuffers[3];
UntexturedOverlayBuffer		masLineOverlayBuffers[3];




class TexturedPolyBuffer		// This'd be handy for models perhaps..
{
public:
	TexturedPolyBuffer()
	{
		mpTexturedTriVertBuffer = NULL;
		mpTexturedTriUVBuffer = NULL;
		mpTexturedTriColourBuffer = NULL;
		mnNumVertsInTexturedTriBuffer = 0;
	}

	~TexturedPolyBuffer()
	{
		Release();
	}

	void	Release( void )
	{
		if ( mpTexturedTriVertBuffer )
		{
			// release vert buffers etc
			delete [] mpTexturedTriVertBuffer;
			delete [] mpTexturedTriUVBuffer;
			delete [] mpTexturedTriColourBuffer;

			mpTexturedTriVertBuffer = NULL;
			mpTexturedTriUVBuffer = NULL;
			mpTexturedTriColourBuffer = NULL;
		}
	}

	void	Init( int nBufferSize )
	{
		mpTexturedTriVertBuffer = new CIwFVec2[nBufferSize];
		mpTexturedTriUVBuffer = new CIwFVec2[nBufferSize];
		mpTexturedTriColourBuffer = new CIwColour[nBufferSize];
	}

	CIwFVec2*		mpTexturedTriVertBuffer;
	CIwFVec2*		mpTexturedTriUVBuffer;
	CIwColour*		mpTexturedTriColourBuffer;
	int				mnNumVertsInTexturedTriBuffer;
};

TexturedPolyBuffer		masTexturedPolyBuffers[MAX_DIFFERENT_TEXTURED_OVERLAYS];

CIwFVec2*		mspTexturedTriVertBuffer = NULL;
CIwFVec2*		mspTexturedTriUVBuffer = NULL;
CIwColour*		mspTexturedTriColourBuffer = NULL;

int				msnNumVertsInTexturedTriBuffer = 0;

CIwGxFont*		mspFontArialBoldLarge = NULL;
CIwGxFont*		mspFontArialBoldSmall = NULL;
CIwGxFont*		mspFontArial10 = NULL;
BOOL			mbOverlaysAdditive = FALSE;

CIwColour		InterfaceGetIWColARGB( ulong ulCol )
{
CIwColour	col;

	col.r = (ulCol >> 16) & 0xFF;
	col.g = (ulCol >> 8) & 0xFF;
	col.b = (ulCol) & 0xFF;
	col.a = (ulCol >> 24) & 0xFF;
	return( col );
}

void	InterfaceInitBuffers( void )
{
	masTriOverlayBuffers[0].Init( TRI_VERT_BUFFER_SIZE );
	masTriOverlayBuffers[1].Init( TRI_VERT_BUFFER_SIZE );
	masTriOverlayBuffers[2].Init( TRI_VERT_BUFFER_SIZE );

	masLineOverlayBuffers[0].Init( LINE_VERT_BUFFER_SIZE );
	masLineOverlayBuffers[1].Init( LINE_VERT_BUFFER_SIZE );
	masLineOverlayBuffers[2].Init( LINE_VERT_BUFFER_SIZE );

	mspTexturedTriVertBuffer = new CIwFVec2[TEXTURED_TRI_VERT_BUFFER_SIZE];
	mspTexturedTriUVBuffer = new CIwFVec2[TEXTURED_TRI_VERT_BUFFER_SIZE];
	mspTexturedTriColourBuffer = new CIwColour[TEXTURED_TRI_VERT_BUFFER_SIZE];
	
	msnNumVertsInTexturedTriBuffer = 0;

	msxPreparedFontData.Reserve( 4096, 50, false );
}

void	InterfaceFreeBuffers( void )
{
	msxPreparedFontData.Clear();

	masTriOverlayBuffers[0].Release();
	masTriOverlayBuffers[1].Release();
	masTriOverlayBuffers[2].Release();

	masLineOverlayBuffers[0].Release();
	masLineOverlayBuffers[1].Release();
	masLineOverlayBuffers[2].Release();

	delete [] mspTexturedTriVertBuffer;
	delete [] mspTexturedTriUVBuffer;
	delete [] mspTexturedTriColourBuffer;

}

INTERFACE_API int	InterfaceNewFrame( ulong ulClearCol )
{
int		R = (ulClearCol >> 16) & 0xFF;
int		G = (ulClearCol >> 8) & 0xFF;
int		B = (ulClearCol & 0xFF);

    IwGxSetColClear( R, G, B, 0x00);
	// Clear the screen
    IwGxClear(IW_GX_COLOUR_BUFFER_F | IW_GX_DEPTH_BUFFER_F);
	return( 0 );
}


INTERFACE_API void	InterfaceRect( int nLayer, int nX, int nY, int nWidth, int nHeight, ulong ulCol )
{
CIwFVec2*		pVerts = masTriOverlayBuffers[nLayer].mpVertBuffer + masTriOverlayBuffers[nLayer].mnNumVertsInBuffer;
CIwColour*		pCols = masTriOverlayBuffers[nLayer].mpColourBuffer + masTriOverlayBuffers[nLayer].mnNumVertsInBuffer;
CIwColour		col;
CIwFVec2*		pVerts2 = pVerts + 1;
CIwFVec2*		pVerts3 = pVerts + 2;

	if ( masTriOverlayBuffers[nLayer].mnNumVertsInBuffer > TRI_VERT_BUFFER_SIZE - 6 )
	{
		return;
	}
	col = InterfaceGetIWColARGB( ulCol );

	// TODO - Check for end of buffer [and flip if necc?]

	pVerts->x = (float)( nX );
	pVerts->y = (float)( nY );
	pVerts++;
	pCols->Set( col );
	pCols++;

	pVerts->x = (float)( nX );
	pVerts->y = (float)( nY + nHeight );
	pVerts++;
	pCols->Set( col );
	pCols++;

	pVerts->x = (float)( nX + nWidth );
	pVerts->y = (float)( nY );
	pVerts++;
	pCols->Set( col );
	pCols++;

	*pVerts = *pVerts3;
	pVerts++;
	pCols->Set( col );
	pCols++;

	*pVerts = *pVerts2;
	pVerts++;
	pCols->Set( col );
	pCols++;

	pVerts->x = (float)( nX + nWidth );
	pVerts->y = (float)( nY + nHeight );
	pVerts++;
	pCols->Set( col );
	pCols++;
	masTriOverlayBuffers[nLayer].mnNumVertsInBuffer += 6;

}

INTERFACE_API void	InterfaceOutlineBox( int nLayer, int nX, int nY, int nWidth, int nHeight, ulong ulCol )
{
	InterfaceRect( nLayer, nX, nY, nWidth, 1, ulCol );
	InterfaceRect( nLayer, nX, nY, 1, nHeight, ulCol );
	InterfaceRect( nLayer, nX + nWidth, nY, 1, nHeight, ulCol );
	InterfaceRect( nLayer, nX, nY + nHeight, nWidth, 1, ulCol );

}

INTERFACE_API void	InterfaceShadedRect( int nLayer, int nX, int nY, int nWidth, int nHeight, ulong ulCol1, ulong ulCol2,ulong ulCol3, ulong ulCol4 )
{
CIwFVec2*		pVerts = masTriOverlayBuffers[nLayer].mpVertBuffer + masTriOverlayBuffers[nLayer].mnNumVertsInBuffer;
CIwColour*		pCols = masTriOverlayBuffers[nLayer].mpColourBuffer + masTriOverlayBuffers[nLayer].mnNumVertsInBuffer;
CIwColour		col1;
CIwColour		col2;
CIwColour		col3;
CIwColour		col4;
CIwFVec2*		pVerts2 = pVerts + 1;
CIwFVec2*		pVerts3 = pVerts + 2;

	if ( masTriOverlayBuffers[nLayer].mnNumVertsInBuffer > TRI_VERT_BUFFER_SIZE - 6 )
	{
		return;
	}

	col1 = InterfaceGetIWColARGB( ulCol1 );
	col2 = InterfaceGetIWColARGB( ulCol2 );
	col3 = InterfaceGetIWColARGB( ulCol3 );
	col4 = InterfaceGetIWColARGB( ulCol4 );

	// TODO - Check for end of buffer [and flip if necc?]

	pVerts->x = (float)( nX );
	pVerts->y = (float)( nY );
	pVerts++;
	pCols->Set( col1 );
	pCols++;

	pVerts->x = (float)( nX );
	pVerts->y = (float)( nY + nHeight );
	pVerts++;
	pCols->Set( col3 );
	pCols++;

	pVerts->x = (float)( nX + nWidth );
	pVerts->y = (float)( nY );
	pVerts++;
	pCols->Set( col2 );
	pCols++;

	*pVerts = *pVerts3;
	pVerts++;
	pCols->Set( col2 );
	pCols++;

	*pVerts = *pVerts2;
	pVerts++;
	pCols->Set( col3 );
	pCols++;

	pVerts->x = (float)( nX + nWidth );
	pVerts->y = (float)( nY + nHeight );
	pVerts++;
	pCols->Set( col4 );
	pCols++;

	masTriOverlayBuffers[nLayer].mnNumVertsInBuffer += 6;

}

INTERFACE_API void	InterfaceLine( int nLayer, int nX1, int nY1, int nX2, int nY2, ulong ulCol1, ulong ulCol2 )
{
CIwFVec2*		pVerts = masLineOverlayBuffers[nLayer].mpVertBuffer + masLineOverlayBuffers[nLayer].mnNumVertsInBuffer;
CIwColour*		pCols = masLineOverlayBuffers[nLayer].mpColourBuffer + masLineOverlayBuffers[nLayer].mnNumVertsInBuffer;
CIwColour		col1;
CIwColour		col2;

	if ( masLineOverlayBuffers[nLayer].mnNumVertsInBuffer >= LINE_VERT_BUFFER_SIZE - 2 )
	{
		return;
	}

	col1 = InterfaceGetIWColARGB( ulCol1 );
	col2 = InterfaceGetIWColARGB( ulCol2 );

	// TODO - Check for end of buffer [and flip if necc?]

	pVerts->x = (float)( nX1 );
	pVerts->y = (float)( nY1 );
	pVerts++;
	pCols->Set( col1 );
	pCols++;

	pVerts->x = (float)( nX2 );
	pVerts->y = (float)( nY2 );
	pVerts++;
	pCols->Set( col2 );
	pCols++;

	masLineOverlayBuffers[nLayer].mnNumVertsInBuffer += 2;

}

INTERFACE_API void	InterfaceTri( int nLayer, int nX1, int nY1, int nX2, int nY2, int nX3, int nY3, ulong ulCol1, ulong ulCol2, ulong ulCol3 )
{
CIwFVec2*		pVerts = masTriOverlayBuffers[nLayer].mpVertBuffer + masTriOverlayBuffers[nLayer].mnNumVertsInBuffer;
CIwColour*		pCols = masTriOverlayBuffers[nLayer].mpColourBuffer + masTriOverlayBuffers[nLayer].mnNumVertsInBuffer;
CIwColour		col1;
CIwColour		col2;
CIwColour		col3;

	if ( masTriOverlayBuffers[nLayer].mnNumVertsInBuffer > TRI_VERT_BUFFER_SIZE - 3 )
	{
		return;
	}
	col1 = InterfaceGetIWColARGB( ulCol1 );
	col2 = InterfaceGetIWColARGB( ulCol2 );
	col3 = InterfaceGetIWColARGB( ulCol3 );

	// TODO - Check for end of buffer [and flip if necc?]

	pVerts->x = (float)( nX1 );
	pVerts->y = (float)( nY1 );
	pVerts++;
	pCols->Set( col1 );
	pCols++;

	pVerts->x = (float)( nX2 );
	pVerts->y = (float)( nY2 );
	pVerts++;
	pCols->Set( col2 );
	pCols++;

	pVerts->x = (float)( nX3 );
	pVerts->y = (float)( nY3 );
	pVerts++;
	pCols->Set( col3 );
	pCols++;

	masTriOverlayBuffers[nLayer].mnNumVertsInBuffer += 3;
}

INTERFACE_API int InterfaceGetWidth( void )
{
	return( IwGxGetScreenWidth() );
}

INTERFACE_API int InterfaceGetHeight( void )
{
	return( IwGxGetScreenHeight() );
}

float		PrepareFont( int nFont, int nCurrentFontFlags )
{
float	fScale = 1.0f;

	switch( nFont )
	{
	case 0:
//		fScale = 0.7f;
		IwGxFontSetFont(mspFontArialBoldSmall);
		break;
	case 1:
//		fScale = 0.9f;
		IwGxFontSetFont(mspFontArialBoldLarge);
		break;
	case 2:
		fScale = 1.0f;
		IwGxFontSetFont(mspFontArial10);
		break;
	case 3:
		fScale = 0.7f;
		IwGxFontSetFont(mspFontArial10);
		break;
	case 4:
		fScale = 0.6f;
		IwGxFontSetFont(mspFontArial10);
		break;	
	}

	if ( nCurrentFontFlags & FONT_FLAG_SMALL )
	{
		fScale *= 0.75f;
	}
	else if ( nCurrentFontFlags & FONT_FLAG_LARGE )
	{
		fScale *= 1.25f;
	}
	else if ( nCurrentFontFlags & FONT_FLAG_GIANT )
	{
		fScale *= 2.0f;
	}
	else if ( nCurrentFontFlags & FONT_FLAG_MASSIVE )
	{
		fScale *= 3.1f;
	}
	else if ( nCurrentFontFlags & FONT_FLAG_ENORMOUS )
	{
		fScale *= 5.0f;
	}

	IwGxFontSetScale( IW_FIXED(fScale) );
	extern int		mnCurrentFontFlags;

	return( fScale );
}

int		GetStringHeight( const char* pcString, int nFont )
{
float	fScale;
CIwRect		drawRect;

	drawRect.Make( 0, 0, InterfaceGetWidth(), InterfaceGetHeight() );
	IwGxFontSetRect( drawRect );
//	memset( &xPreparedData, 0, sizeof( xPreparedData ) );
//	IwGxFontSetAlignmentHor( IW_GX_FONT_ALIGN_LEFT );
//	IwGxFontSetAlignmentVer( IW_GX_FONT_ALIGN_BOTTOM );
//	IwGxFontSetRect(CIwRect(0,0, width, InterfaceGetHeight()));  
	fScale = PrepareFont( nFont, mnCurrentFontFlags );
	IwGxFontPrepareText( msxPreparedFontData, (const IwChar*)pcString );

	return( (int)( msxPreparedFontData.GetHeight() * fScale ) );
}

int		InterfaceTextGetHeight( const char* pcString, int nFont )
{
	return( GetStringHeight( pcString, nFont ) );
}

int		GetStringWidth( const char* pcString, int nFont )
{
float	fScale;

//	IwGxFontSetAlignmentHor( IW_GX_FONT_ALIGN_LEFT );
//	IwGxFontSetAlignmentVer( IW_GX_FONT_ALIGN_BOTTOM );
//	IwGxFontSetRect(CIwRect(0,0, width, InterfaceGetHeight()));  
	fScale = PrepareFont( nFont, mnCurrentFontFlags );
	IwGxFontPrepareText( msxPreparedFontData, (const IwChar*)pcString );

	return( (int)( msxPreparedFontData.GetWidth() * fScale ) );
}


int	InterfaceTextRectGetUsedWidth( const char* szString, int font, int nMaxWidth )
{
float		fScale;

	IwGxFontSetAlignmentHor( IW_GX_FONT_ALIGN_LEFT );
	IwGxFontSetAlignmentVer( IW_GX_FONT_ALIGN_BOTTOM );
	IwGxFontSetRect(CIwRect(0,0, nMaxWidth, InterfaceGetHeight()));  
	fScale = PrepareFont( font, mnCurrentFontFlags );
	IwGxFontPrepareText( msxPreparedFontData, (const IwChar*)szString );

	return( (int)( msxPreparedFontData.GetWidth() * fScale ) );
}


int		InterfaceTextGetHeightUsed( const char* szString, int nFont, int width )
{
float		fScale;

	IwGxFontSetAlignmentHor( IW_GX_FONT_ALIGN_LEFT );
	IwGxFontSetAlignmentVer( IW_GX_FONT_ALIGN_BOTTOM );
	IwGxFontSetRect(CIwRect(0,0, width, InterfaceGetHeight()));  
	fScale = PrepareFont( nFont, mnCurrentFontFlags );
	IwGxFontPrepareText( msxPreparedFontData, (const IwChar*)szString );

	return( (int)( msxPreparedFontData.GetHeight() * fScale ) );
}

void InterfaceEnableTextureFiltering( BOOL bFlag )
{
	// nothing to do.. its always on in marmalade land..
}

void	InterfaceSetGlobalParam( INTF_DRAW_PARAM nParam, int nState )
{
	switch( nParam )
	{
	case INTF_LINES_ALPHA:
	case INTF_FULLSCREEN:
	case INTF_NO_RENDER:
	case INTF_TEXTURERECT_LIMIT:
	case INTF_ANISOTROPIC:
		break;
	case INTF_TEXTURE_FILTERING:
		break;
	}

}


// TODO - Expose this..
//		IwGxGetScissorScreenSpace
//  for viewport scissoring 


void	InterfaceRenderTextList( int nLayer )
{
TEXT_BUFFER*		pxTextBuffer;
CIwColour		col;

	IwGxLightingOn(); 

	pxTextBuffer = StringListGetNext( nLayer, NULL );
	while( pxTextBuffer )
	{
		col = InterfaceGetIWColARGB( pxTextBuffer->ulCol );
		IwGxFontSetCol( col ); 

		if ( pxTextBuffer->wFlag & FONT_FLAG_ITALIC )
		{
			IwGxFontSetFlags( IW_GX_FONT_ITALIC_F );
		}

		switch( pxTextBuffer->wAlign )
		{
		case ALIGN_LEFT:
		default:
			IwGxFontSetAlignmentHor( IW_GX_FONT_ALIGN_LEFT );
			IwGxFontSetAlignmentVer( IW_GX_FONT_ALIGN_TOP );
			IwGxFontSetRect(CIwRect(pxTextBuffer->nX, pxTextBuffer->nY,(int16)IwGxGetScreenWidth()-pxTextBuffer->nX,(int16)IwGxGetScreenHeight()-pxTextBuffer->nY));  
			break;
		case ALIGN_SCALED:
			IwGxFontSetAlignmentHor( IW_GX_FONT_ALIGN_LEFT );
			IwGxFontSetAlignmentVer( IW_GX_FONT_ALIGN_TOP );
			IwGxFontSetRect(CIwRect(pxTextBuffer->nX, pxTextBuffer->nY,(int16)IwGxGetScreenWidth()-pxTextBuffer->nX,(int16)IwGxGetScreenHeight()-pxTextBuffer->nY));  
			IwGxFontSetScale( (iwfixed)pxTextBuffer->fScale );
			break;
		case ALIGN_RIGHT:
			IwGxFontSetAlignmentHor( IW_GX_FONT_ALIGN_RIGHT );
			IwGxFontSetAlignmentVer( IW_GX_FONT_ALIGN_TOP );
			// Set the formatting rect - this controls where the text appears and what it is formatted against  
			IwGxFontSetRect(CIwRect(0, pxTextBuffer->nY,(int16)pxTextBuffer->nX,(int16)IwGxGetScreenHeight()-pxTextBuffer->nY));  
			break;
		case ALIGN_CENTER:
			IwGxFontSetAlignmentHor( IW_GX_FONT_ALIGN_CENTRE );
			IwGxFontSetAlignmentVer( IW_GX_FONT_ALIGN_TOP );
			// Set the formatting rect - this controls where the text appears and what it is formatted against  
			IwGxFontSetRect(CIwRect(pxTextBuffer->nX, pxTextBuffer->nY,(int16)pxTextBuffer->nWidth,(int16)IwGxGetScreenHeight()-pxTextBuffer->nY));  
			break;
		case ALIGN_RECT:
			IwGxFontSetAlignmentHor( IW_GX_FONT_ALIGN_LEFT );
			IwGxFontSetAlignmentVer( IW_GX_FONT_ALIGN_BOTTOM );
			IwGxFontSetRect(CIwRect(pxTextBuffer->nX,pxTextBuffer->nY,(int16)pxTextBuffer->nWidth,(int16)pxTextBuffer->nHeight));  
			break;
		case ALIGN_BOX_CENTRE:
			IwGxFontSetAlignmentHor( IW_GX_FONT_ALIGN_CENTRE );
			IwGxFontSetAlignmentVer( IW_GX_FONT_ALIGN_TOP );
			// Set the formatting rect - this controls where the text appears and what it is formatted against  
			IwGxFontSetRect(CIwRect(pxTextBuffer->nX,pxTextBuffer->nY,(int16)pxTextBuffer->nWidth,(int16)IwGxGetScreenHeight()-pxTextBuffer->nY));  
			break;
		case ALIGN_BOX:
			IwGxFontSetAlignmentHor( IW_GX_FONT_ALIGN_LEFT );
			IwGxFontSetAlignmentVer( IW_GX_FONT_ALIGN_TOP );
			// Set the formatting rect - this controls where the text appears and what it is formatted against  
			IwGxFontSetRect(CIwRect(pxTextBuffer->nX,pxTextBuffer->nY,(int16)pxTextBuffer->nWidth,(int16)IwGxGetScreenHeight()-pxTextBuffer->nY));  
			break;
		}

		PrepareFont( pxTextBuffer->bFont, pxTextBuffer->wFlag );
		// Draw the text    
		IwGxFontDrawText( pxTextBuffer->pcString ); 
//		free( pxTextBuffer->pcString );
		IwGxFontClearFlags( 0xFFFFFFFF );

		pxTextBuffer->pcString = NULL;
		pxTextBuffer = StringListGetNext( nLayer, pxTextBuffer );
	}
}

void		InterfaceDrawTris( int nLayer )
{
    // Set screenspace vertex coords
    IwGxSetVertStreamScreenSpace(masTriOverlayBuffers[nLayer].mpVertBuffer, masTriOverlayBuffers[nLayer].mnNumVertsInBuffer);
    // Set vertex colours
	IwGxSetColStream(masTriOverlayBuffers[nLayer].mpColourBuffer, masTriOverlayBuffers[nLayer].mnNumVertsInBuffer);
    // Draw primitives
    IwGxDrawPrims(IW_GX_TRI_LIST, NULL, masTriOverlayBuffers[nLayer].mnNumVertsInBuffer);
	masTriOverlayBuffers[nLayer].mnNumVertsInBuffer = 0;
}

void		InterfaceDrawLines( int nLayer )
{
    // Set screenspace vertex coords
	IwGxSetVertStreamScreenSpace(masLineOverlayBuffers[nLayer].mpVertBuffer, masLineOverlayBuffers[nLayer].mnNumVertsInBuffer);
    // Set vertex colours
	IwGxSetColStream(masLineOverlayBuffers[nLayer].mpColourBuffer, masLineOverlayBuffers[nLayer].mnNumVertsInBuffer);
    // Draw primitives
	IwGxDrawPrims(IW_GX_LINE_LIST, NULL, masLineOverlayBuffers[nLayer].mnNumVertsInBuffer);
	masLineOverlayBuffers[nLayer].mnNumVertsInBuffer = 0;
}






void		InterfaceDrawTexturedPolys( int nOverlayNum, int hTexture, int nLayer )
{
CIwTexture*		pTexture = (CIwTexture*)( InterfaceTextureListGetPlatformTexture( hTexture ) );
TexturedPolyBuffer*		pPolyBuffer = masTexturedPolyBuffers + nOverlayNum;
CIwMaterial*	pMat = (CIwMaterial*)InterfaceTexturedOverlaysGetPlatformMaterial( nOverlayNum );

	if ( pMat )
	{
	    IwGxSetMaterial(pMat);

		if ( pPolyBuffer->mpTexturedTriVertBuffer )
		{
			// Set screenspace vertex coords
			IwGxSetVertStreamScreenSpace(pPolyBuffer->mpTexturedTriVertBuffer, pPolyBuffer->mnNumVertsInTexturedTriBuffer);
			IwGxSetUVStream( pPolyBuffer->mpTexturedTriUVBuffer );
			// Set vertex colours
			IwGxSetColStream(pPolyBuffer->mpTexturedTriColourBuffer, pPolyBuffer->mnNumVertsInTexturedTriBuffer);
			// Draw primitives
			IwGxDrawPrims(IW_GX_TRI_LIST, NULL, pPolyBuffer->mnNumVertsInTexturedTriBuffer);
			pPolyBuffer->mnNumVertsInTexturedTriBuffer = 0;
		}
	}
}

BOOL InterfaceDoesNeedChanging ( void )
{
	return( FALSE );
}

float	InterfaceGetFPS( void )
{
	return( (float)IwGxGetFrameRate() );
}

INTERFACE_API void	InterfaceDraw( void )
{
    IwGxFlush();
	IwGxSetScreenSpaceSlot(0);  
	// Turn all lighting off
    IwGxLightingOff();
	IwGxSetLightCol(0, 0xFF, 0xFF, 0xFF);
	IwGxSetLightCol(1, 0xFF, 0xFF, 0xFF);

	IwGxSetLightType(0, IW_GX_LIGHT_UNUSED);
    IwGxSetLightType(1, IW_GX_LIGHT_UNUSED);

	IwGxSetSortMode(IW_GX_SORT_NONE);
	IwGxSetSortModeAlpha(IW_GX_SORT_NONE);

	InterfaceTexturedPolyListsDraw( 0 );

    IwGxSetMaterial(mpUntexturedOverlaysMat);
	InterfaceDrawTris( 0 );
	InterfaceDrawLines( 0 );

	InterfaceRenderTextList( 0 );

	InterfaceTexturedPolyListsDraw( 1 );

	IwGxSetMaterial(mpUntexturedOverlaysMat);

	InterfaceDrawTris( 1 );
	InterfaceDrawLines( 1 );

	InterfaceRenderTextList( 1 );

	InterfaceTexturedPolyListsDraw( 2 );
	IwGxSetMaterial(mpUntexturedOverlaysMat);

	InterfaceDrawTris( 2 );
	InterfaceDrawLines( 2 );
	InterfaceRenderTextList( 2 );

//	IwGxPrintFrameRate( IwGxGetScreenWidth()-70, 2 );

    IwGxFlush();

	StringListReset();

	InterfaceTexturedPolyListsReset();

}

#define MAX_CONFIGS 30

static int eglInit()
{
    EGLint major;
    EGLint minor;
    EGLint numFound = 0;
    EGLConfig configList[MAX_CONFIGS];
    int version = s3eGLGetInt(S3E_GL_VERSION);
    if ((version >> 8) != 1)
    {
//        s3eDebugTracePrintf("reported GL version is %d", version);
//        s3eDebugErrorShow(S3E_MESSAGE_CONTINUE, "This example required GLES v1.x");
        return 1;
    }
    g_EGLDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (!g_EGLDisplay)
    {
        s3eDebugErrorShow(S3E_MESSAGE_CONTINUE, "eglGetDisplay failed");
        return 1;
    }
    EGLBoolean res = eglInitialize(g_EGLDisplay, &major, &minor);
    if (!res)
    {
        s3eDebugErrorShow(S3E_MESSAGE_CONTINUE, "eglInitialize failed");
        return 1;
    }
    if (!eglGetConfigs(g_EGLDisplay, configList, MAX_CONFIGS, &numFound))
    {
        s3eDebugErrorShow(S3E_MESSAGE_CONTINUE, "eglGetConfigs failed");
        return 1;
    }
    int requiredRedDepth = 8;
    bool requiredFound = s3eConfigGetInt("GL", "EGL_RED_SIZE", &requiredRedDepth) == S3E_RESULT_SUCCESS;
    int config = -1;
    printf("found %d configs\n", numFound);
    for (int i = 0; i < numFound; i++)
    {
        EGLint surfacetype = 0;
        EGLint actualRedDepth = 0;
        EGLint depth = 0;
        eglGetConfigAttrib(g_EGLDisplay, configList[i], EGL_SURFACE_TYPE, &surfacetype);
        eglGetConfigAttrib(g_EGLDisplay, configList[i], EGL_RED_SIZE, &actualRedDepth);
        eglGetConfigAttrib(g_EGLDisplay, configList[i], EGL_DEPTH_SIZE, &depth);
        printf("config %d: depth=%d\n", i, depth);
        if (depth > 16 && (surfacetype & EGL_WINDOW_BIT) && ((!requiredFound || (actualRedDepth == requiredRedDepth))))
        {
            config = i;
            break;
        }
    }
    if (config == -1)
    {
        s3eDebugErrorShow(S3E_MESSAGE_CONTINUE, "No suitable config found.  Trying random config");
        config = 0;
    }
    printf("using config %d\n", config);
    g_EGLContext = eglCreateContext(g_EGLDisplay, configList[config], NULL, NULL);
    if (!g_EGLContext)
    {
        s3eDebugErrorPrintf("eglCreateContext failed: %#x", eglGetError());
        return 1;
    }
    void* nativeWindow = s3eGLGetNativeWindow();
    g_EGLSurface = eglCreateWindowSurface(g_EGLDisplay, configList[config], nativeWindow, NULL);
    if (!g_EGLSurface)
    {
        s3eDebugErrorPrintf("eglCreateWindowSurface failed: %#x", eglGetError());
        return 1;
    }
    res = eglMakeCurrent(g_EGLDisplay, g_EGLSurface, g_EGLSurface, g_EGLContext);
    if (!res)
    {
        s3eDebugErrorShow(S3E_MESSAGE_CONTINUE, "eglMakeCurrent failed");
        return 1;
    }
    
	int w = s3eSurfaceGetInt(S3E_SURFACE_WIDTH);
    int h = s3eSurfaceGetInt(S3E_SURFACE_HEIGHT);

    //Prepare the GL State
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrthof(-2.0, 2.0, -2.0, 2.0, -20.0, 20.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glShadeModel(GL_SMOOTH);
    g_glIsInitialized = true;
    return 0;
}

static void eglCleanup()
{
	if ( g_glIsInitialized )
	{
		g_glIsInitialized = false;
		eglMakeCurrent(g_EGLDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		if (g_EGLContext && g_EGLDisplay)
		{
			eglDestroyContext(g_EGLDisplay, g_EGLContext);
			g_EGLContext = NULL;
		}
		if (g_EGLSurface && g_EGLDisplay)
		{
			eglDestroySurface(g_EGLDisplay, g_EGLSurface);
			g_EGLSurface = NULL;
		}
	}
}

static void eglTerminate()
{
    g_glIsInitialized = false;
    eglMakeCurrent(g_EGLDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (g_EGLContext && g_EGLDisplay)
    {
        eglDestroyContext(g_EGLDisplay, g_EGLContext);
        g_EGLContext = NULL;
    }
    if (g_EGLSurface && g_EGLDisplay)
    {
        eglDestroySurface(g_EGLDisplay, g_EGLSurface);
        g_EGLSurface = NULL;
    }
    eglTerminate(g_EGLDisplay);
}

int32 pauseHandler(void *systemData, void *userData)
{
    eglTerminate();
    return 0;
}

int32 unpauseHandler(void *systemData, void *userData)
{
    if (eglInit())
    {
        s3eDebugErrorShow(S3E_MESSAGE_CONTINUE, "eglInit failed");
    }
    return 0;
}


INTERFACE_API void	InterfaceInitDisplayDevice( BOOL boMinRenderPageSize )
{
#ifdef USE_EGL_INIT
	s3eGLRegister (S3E_GL_SUSPEND, pauseHandler, NULL);
    s3eGLRegister (S3E_GL_RESUME, unpauseHandler, NULL);

    if ( !eglInit() )
	{
//        return;
	}
#endif

	IwGxInit();

}

BOOL					InterfaceIsOversized( void )
{
	if ( ( InterfaceGetWidth() > 1024 ) &&
		 ( InterfaceGetHeight() > 1024 ) )
	{
		return( TRUE );
	}
	return( FALSE );
}

INTERFACE_API void	InterfaceInit( BOOL bUseDefaultFont )
{
    //Initialise modules     
	IwResManagerInit();     
	IwGxFontInit();   

	s3eDeviceYield(0);

	const int32 width = s3eSurfaceGetInt(S3E_SURFACE_WIDTH);
	const int32 height = s3eSurfaceGetInt(S3E_SURFACE_HEIGHT);
	const int32 direction = s3eSurfaceGetInt(S3E_SURFACE_DEVICE_BLIT_DIRECTION);
	const bool landscape = width >= height;
  
//	s3eSurfaceSetInt( S3E_SURFACE_DEVICE_ORIENTATION_LOCK, landscape ? S3E_SURFACE_LANDSCAPE : S3E_SURFACE_PORTRAIT );
	s3eDeviceYield(0);

	// If we're on a really big (retina-scale) display
	if ( ( width > 1024 ) &&
		 ( height > 1024 ) )
	{
		 // Load bigger fonts
		mspFontArialBoldSmall = IwGxFontCreateTTFont( "Fonts/arialbd.ttf", 14);
		mspFontArialBoldLarge = IwGxFontCreateTTFont( "Fonts/arialbd.ttf", 18);
		mspFontArial10 = IwGxFontCreateTTFont( "Fonts/arial.ttf", 18);
	}
	else
	{
		mspFontArialBoldSmall = IwGxFontCreateTTFont( "Fonts/arialbd.ttf", 7);
		mspFontArialBoldLarge = IwGxFontCreateTTFont( "Fonts/arialbd.ttf", 10);
		mspFontArial10 = IwGxFontCreateTTFont( "Fonts/arial.ttf", 10);
	}

	IwGxFontSetFont(mspFontArialBoldSmall);

    // Set screen clear colour
    IwGxSetColClear(0x40, 0x40, 0x40, 0x00);
 
	InterfaceInitBuffers();
	StringListInit();

	// Allocate a material from the IwGx global cache
    mpUntexturedOverlaysMat = new CIwMaterial;
	mpUntexturedOverlaysMat->SetTexture( NULL );
	mpUntexturedOverlaysMat->SetCullMode(CIwMaterial::CULL_NONE);
	mpUntexturedOverlaysMat->SetAlphaMode(CIwMaterial::ALPHA_BLEND);
	mpUntexturedOverlaysMat->SetColAmbient(0xffffffff);

//	pMat->SetModulateMode(CIwMaterial::MODULATE_NONE);

	InterfaceTextureListInit();
	InterfaceInitTexturedPolyLists();
}

void	InterfaceFreeTexturedPolyBuffers( void )
{
int		nLoop;

	for( nLoop = 0; nLoop < MAX_DIFFERENT_TEXTURED_OVERLAYS; nLoop++ )
	{
		masTexturedPolyBuffers[nLoop].Release();
	}
}


INTERFACE_API void	InterfaceFree( void ) 
{
//int		nLoop;
    IwGxFlush();
    IwGxSwapBuffers();

	InterfaceFreeTexturedPolyBuffers();

	InterfaceFreeTexturedPolyLists();
	InterfaceTextureListFree();

	InterfaceTexturesMarmaladeFreeAll();

    IwGxFlush();
	InterfaceFreeBuffers();
	StringListShutdown();

	delete mpUntexturedOverlaysMat;

	IwGxFontDestroyTTFont( mspFontArial10 );
	IwGxFontDestroyTTFont( mspFontArialBoldSmall );
	IwGxFontDestroyTTFont( mspFontArialBoldLarge );

	IwGxFontTerminate();     
	IwResManagerTerminate();

#ifdef USE_EGL_INIT

	s3eGLUnRegister( S3E_GL_SUSPEND, pauseHandler );
    s3eGLUnRegister( S3E_GL_RESUME, unpauseHandler);

	eglCleanup();

	// apparently we don't need to do this - iwGxTerminate seems to do it for us
//	eglTerminate();
#endif

	IwGxTerminate();

}


INTERFACE_API void	InterfacePresent( void )
{
	// Flush and swap
    IwGxSwapBuffers();

	InterfaceTexturesMarmaladeNewFrame();
	InterfaceTextureListNewFrame();

}

INTERFACE_API void	InterfaceBeginRender( void )
{

}


INTERFACE_API void	InterfaceDrawNoMatrix( void )
{

}

INTERFACE_API void	InterfaceEndRender( void )
{

}


INTERFACE_API void InterfaceSetWindowPosition( int nLeft, int nTop )
{

}

INTERFACE_API void	InterfaceOverlaysAdditive( BOOL bFlag )
{
	mbOverlaysAdditive = bFlag;
	if ( bFlag )
	{
		mpUntexturedOverlaysMat->SetAlphaMode(CIwMaterial::ALPHA_ADD);
	}
	else
	{
		mpUntexturedOverlaysMat->SetAlphaMode(CIwMaterial::ALPHA_BLEND);
	}
}

INTERFACE_API BOOL	InterfaceFontLoad( int nFontNum, const char* pcImageFileName, const char* pcLayoutFile, ulong ulFlags )
{
	// todo 
	return( FALSE );
}




void OnSetInitialSize( BOOL boFullScreen, int nFullScreenSizeX, int nFullScreenSizeY , BOOL boSmallFlag )
{

}


void InterfacePlatformAddTexturedTriVertices( int nOverlayNum, TEXTURED_RECT_DEF* pxRectDef )
{
TexturedPolyBuffer*		pPolyBuffer = masTexturedPolyBuffers + nOverlayNum;

	if ( pPolyBuffer->mpTexturedTriVertBuffer == NULL )
	{
		pPolyBuffer->Init( MAX_VERTS_IN_SINGLE_OVERLAY );
	}
	if ( pPolyBuffer->mpTexturedTriVertBuffer )
	{
	CIwFVec2*		pVerts = pPolyBuffer->mpTexturedTriVertBuffer + pPolyBuffer->mnNumVertsInTexturedTriBuffer;
	CIwFVec2*		pUVs = pPolyBuffer->mpTexturedTriUVBuffer + pPolyBuffer->mnNumVertsInTexturedTriBuffer;
	CIwColour*		pCols = pPolyBuffer->mpTexturedTriColourBuffer + pPolyBuffer->mnNumVertsInTexturedTriBuffer;
	CIwColour		col;

		col = InterfaceGetIWColARGB( pxRectDef->ulCol );

		// TODO - Check for end of buffer [and flip if necc?]

		pVerts->x = (float)( pxRectDef->nX );
		pVerts->y = (float)( pxRectDef->nY );
		pVerts++;
		pCols->Set( col );
		pCols++;
		pUVs->x = pxRectDef->fU1;
		pUVs->y = pxRectDef->fV1;
		pUVs++;

		pVerts->x = (float)( pxRectDef->nWidth );
		pVerts->y = (float)( pxRectDef->nHeight );
		pVerts++;
		pCols->Set( col );
		pCols++;
		pUVs->x = pxRectDef->fU2;
		pUVs->y = pxRectDef->fV2;
		pUVs++;

		pVerts->x = (float)( pxRectDef->nX2 );
		pVerts->y = (float)( pxRectDef->nY2 );
		pVerts++;
		pCols->Set( col );
		pCols++;
		pUVs->x = pxRectDef->fU3;
		pUVs->y = pxRectDef->fV3;
		pUVs++;

		pPolyBuffer->mnNumVertsInTexturedTriBuffer += 3;
	}
}



void InterfacePlatformAddSpriteVertices( int nOverlayNum, TEXTURED_RECT_DEF* pxRectDef )
{
TexturedPolyBuffer*		pPolyBuffer = masTexturedPolyBuffers + nOverlayNum;

	if ( pPolyBuffer->mpTexturedTriVertBuffer == NULL )
	{
		pPolyBuffer->Init( MAX_VERTS_IN_SINGLE_OVERLAY );
	}

	if ( pPolyBuffer->mpTexturedTriVertBuffer )
	{
	CIwFVec2*		pVerts = pPolyBuffer->mpTexturedTriVertBuffer + pPolyBuffer->mnNumVertsInTexturedTriBuffer;
	CIwFVec2*		pUVs = pPolyBuffer->mpTexturedTriUVBuffer + pPolyBuffer->mnNumVertsInTexturedTriBuffer;
	CIwColour*		pCols = pPolyBuffer->mpTexturedTriColourBuffer + pPolyBuffer->mnNumVertsInTexturedTriBuffer;
	CIwColour		col;
	VECT			xVect;

		xVect.x = pxRectDef->nWidth * -0.5f;
		xVect.y = pxRectDef->nHeight * -0.5f;
		xVect.z = 0.0f;
		VectRotateAboutZ( &xVect, pxRectDef->fRot );

		col = InterfaceGetIWColARGB( pxRectDef->ulCol );

		// TODO - Check for end of buffer [and flip if necc?]

		pVerts->x = (float)( pxRectDef->nX ) + xVect.x + 0.25f;
		pVerts->y = (float)( pxRectDef->nY ) + xVect.y + 0.25f;
		pVerts++;
		pCols->Set( col );
		pCols++;
		pUVs->x = pxRectDef->fU1;
		pUVs->y = pxRectDef->fV1;
		pUVs++;

		xVect.x = pxRectDef->nWidth * 0.5f;
		xVect.y = pxRectDef->nHeight * -0.5f;
		xVect.z = 0.0f;
		VectRotateAboutZ( &xVect, pxRectDef->fRot );

		pVerts->x = (float)( pxRectDef->nX + xVect.x ) + 0.25f;
		pVerts->y = (float)( pxRectDef->nY + xVect.y ) + 0.25f;

		pVerts++;
		pCols->Set( col );
		pCols++;
		pUVs->x = pxRectDef->fU2;
		pUVs->y = pxRectDef->fV1;
		pUVs++;

		xVect.x = pxRectDef->nWidth * -0.5f;
		xVect.y = pxRectDef->nHeight * 0.5f;
		xVect.z = 0.0f;
		VectRotateAboutZ( &xVect, pxRectDef->fRot );

		pVerts->x = (float)( pxRectDef->nX + xVect.x ) + 0.25f;
		pVerts->y = (float)( pxRectDef->nY + xVect.y ) + 0.25f;	
		pVerts++;
		pCols->Set( col );
		pCols++;
		pUVs->x = pxRectDef->fU1;
		pUVs->y = pxRectDef->fV2;
		pUVs++;

		xVect.x = pxRectDef->nWidth * 0.5f;
		xVect.y = pxRectDef->nHeight * -0.5f;
		xVect.z = 0.0f;
		VectRotateAboutZ( &xVect, pxRectDef->fRot );

		pVerts->x = (float)( pxRectDef->nX + xVect.x ) + 0.25f;
		pVerts->y = (float)( pxRectDef->nY + xVect.y ) + 0.25f;

		pVerts++;
		pCols->Set( col );
		pCols++;
		pUVs->x = pxRectDef->fU2;
		pUVs->y = pxRectDef->fV1;
		pUVs++;

		xVect.x = pxRectDef->nWidth * 0.5f;
		xVect.y = pxRectDef->nHeight * 0.5f;
		xVect.z = 0.0f;
		VectRotateAboutZ( &xVect, pxRectDef->fRot );

		pVerts->x = (float)( pxRectDef->nX + xVect.x  ) + 0.25f;
		pVerts->y = (float)( pxRectDef->nY + xVect.y ) + 0.25f;
		pVerts++;
		pCols->Set( col );
		pCols++;
		pUVs->x = pxRectDef->fU2;
		pUVs->y = pxRectDef->fV2;
		pUVs++;
			
		xVect.x = pxRectDef->nWidth * -0.5f;
		xVect.y = pxRectDef->nHeight * 0.5f;
		xVect.z = 0.0f;
		VectRotateAboutZ( &xVect, pxRectDef->fRot );

		pVerts->x = (float)( pxRectDef->nX + xVect.x ) + 0.25f;
		pVerts->y = (float)( pxRectDef->nY + xVect.y ) + 0.25f;
		pVerts++;
		pCols->Set( col );
		pCols++;
		pUVs->x = pxRectDef->fU1;
		pUVs->y = pxRectDef->fV2;
		pUVs++;

		pPolyBuffer->mnNumVertsInTexturedTriBuffer += 6;
	}
}


void InterfacePlatformAddTexturedRectVertices( int nOverlayNum, TEXTURED_RECT_DEF* pxRectDef )
{
TexturedPolyBuffer*		pPolyBuffer = masTexturedPolyBuffers + nOverlayNum;

	if ( pPolyBuffer->mpTexturedTriVertBuffer == NULL )
	{
		pPolyBuffer->Init( MAX_VERTS_IN_SINGLE_OVERLAY );
	}

	if ( pPolyBuffer->mpTexturedTriVertBuffer )
	{
	CIwFVec2*		pVerts = pPolyBuffer->mpTexturedTriVertBuffer + pPolyBuffer->mnNumVertsInTexturedTriBuffer;
	CIwFVec2*		pUVs = pPolyBuffer->mpTexturedTriUVBuffer + pPolyBuffer->mnNumVertsInTexturedTriBuffer;
	CIwColour*		pCols = pPolyBuffer->mpTexturedTriColourBuffer + pPolyBuffer->mnNumVertsInTexturedTriBuffer;
	CIwColour		col;

		col = InterfaceGetIWColARGB( pxRectDef->ulCol );

		// TODO - Check for end of buffer [and flip if necc?]

		pVerts->x = (float)( pxRectDef->nX );
		pVerts->y = (float)( pxRectDef->nY );
		pVerts++;
		pCols->Set( col );
		pCols++;
		pUVs->x = pxRectDef->fU1;
		pUVs->y = pxRectDef->fV1;
		pUVs++;

		pVerts->x = (float)( pxRectDef->nX + pxRectDef->nWidth );
		pVerts->y = (float)( pxRectDef->nY );
		pVerts++;
		pCols->Set( col );
		pCols++;
		pUVs->x = pxRectDef->fU2;
		pUVs->y = pxRectDef->fV1;
		pUVs++;

		pVerts->x = (float)( pxRectDef->nX );
		pVerts->y = (float)( pxRectDef->nY + pxRectDef->nHeight );
		pVerts++;
		pCols->Set( col );
		pCols++;
		pUVs->x = pxRectDef->fU1;
		pUVs->y = pxRectDef->fV2;
		pUVs++;

		pVerts->x = (float)( pxRectDef->nX + pxRectDef->nWidth );
		pVerts->y = (float)( pxRectDef->nY );
		pVerts++;
		pCols->Set( col );
		pCols++;
		pUVs->x = pxRectDef->fU2;
		pUVs->y = pxRectDef->fV1;
		pUVs++;

		pVerts->x = (float)( pxRectDef->nX + pxRectDef->nWidth );
		pVerts->y = (float)( pxRectDef->nY + pxRectDef->nHeight );
		pVerts++;
		pCols->Set( col );
		pCols++;
		pUVs->x = pxRectDef->fU2;
		pUVs->y = pxRectDef->fV2;
		pUVs++;
			
		pVerts->x = (float)( pxRectDef->nX );
		pVerts->y = (float)( pxRectDef->nY + pxRectDef->nHeight );
		pVerts++;
		pCols->Set( col );
		pCols++;
		pUVs->x = pxRectDef->fU1;
		pUVs->y = pxRectDef->fV2;
		pUVs++;

		pPolyBuffer->mnNumVertsInTexturedTriBuffer += 6;
	}
}



INTERFACE_API void InterfaceSetWindowSize( BOOL boFullScreen, int nWidth, int nHeight, BOOL boAdjust )
{


}


INTERFACE_API void	InterfaceSetTextureAsCurrent( int nTextureHandle )
{
/*
InterfaceTextureListItem*		pItem = InterfaceTextureManagerGetManagedItem( nTextureHandle );

	if ( pItem )
	{
		mpTextureMat->SetTexture( pTexture );
		IwGxSetMaterial(mpTextureMat);
		glBindTexture (GL_TEXTURE_2D, pItem->mpGLTexture->TextureID);
	}
*/
}

INTERFACE_API void PanicImpl( const char* szErrorString )
{
	s3eDebugTracePrintf(szErrorString);

}

INTERFACE_API int  InterfaceGetTextureFromFileInMem( const char* szFilename, unsigned char* pbMem, int nMemSize, int nFlags )
{
	// TODO

	return( NOTFOUND );
}
