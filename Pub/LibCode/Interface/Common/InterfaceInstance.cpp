#include "../DirectX/InterfaceInternalsDX.h"
#include "StandardDef.h"

#include "Interface.h"

#include "../DirectX/Image.h"
#include "Overlays/TexturedOverlays.h"
#include "../Common/Font/FontCommon.h"
#include "InterfaceInstance.h"

void	InterfaceInstance::Shutdown()
{
	mpOverlays->Shutdown();
	mpTexturedOverlays->Shutdown();
	mpFontSystem->Shutdown();
	mpInterfaceInternals->Shutdown();

	delete mpOverlays;
	delete mpTexturedOverlays;
	delete mpInterfaceInternals;
	delete mpFontSystem;
}

InterfaceInstance::InterfaceInstance()
{
	mpOverlays = new Overlays;
	mpTexturedOverlays = new TexturedOverlays;
	mpInterfaceInternals = new InterfaceInternalsDX;
	mpFontSystem = new FontSystem;

	mpOverlays->InitialiseModule( this );
	mpTexturedOverlays->InitialiseModule( this );
	mpInterfaceInternals->InitialiseModule( this );
	mpFontSystem->InitialiseModule( this );
}

void	InterfaceInstance::SetDevice( void* pDevice )
{
LPGRAPHICSDEVICE		pGraphicsDevice = (LPGRAPHICSDEVICE)pDevice;

	mpOverlays->SetGraphicsDevice( pGraphicsDevice );
	mpTexturedOverlays->SetGraphicsDevice( pGraphicsDevice );
	mpInterfaceInternals->SetGraphicsDevice( pGraphicsDevice );
	mpFontSystem->SetGraphicsDevice( pGraphicsDevice );
}

void InterfaceInstance::SetInitialSize(BOOL boFullScreen, int width, int height, BOOL bSmallMode)
{
	if ( !boFullScreen )
	{
#ifdef WIN32
		SetInitialWindowSize( width, height );
#endif
	}

	mboFullScreen = boFullScreen;
	 
//	mboSmall = boSmallFlag;	// todo

}

BOOL		InterfaceInstance::LoadFont( int nFontNum, const char* pcImageFileName, const char* pcLayoutFile, uint32 ulFlags )
{
	return( mpFontSystem->LoadFont( nFontNum, pcImageFileName, pcLayoutFile, ulFlags ) );
}

void		InterfaceInstance::InitialiseInstance( BOOL bUseDefaultFonts )
{
	if ( mboInterfaceInitialised == FALSE )
	{
		InterfaceSetIsUsingDefaultFonts( bUseDefaultFonts );
		mpFontSystem->InitialiseFonts( bUseDefaultFonts );
		mpOverlays->Initialise();

		mpTexturedOverlays->Initialise();

		InterfaceImagesInit();
	}

	mboInterfaceInitialised = TRUE;
}


InterfaceInstance*		InterfaceInstanceMain()
{
static InterfaceInstance		ms_MainSingletonInstance;
static BOOL			ms_bHasInitialisedMainInstance = FALSE;

	if ( !ms_bHasInitialisedMainInstance )
	{
		ms_bHasInitialisedMainInstance = TRUE;
	}
	return( &ms_MainSingletonInstance );
}

const char*	InterfaceInstance::TextLimitWidth( int nLayer, int nX, int nY, int nMaxWidth, uint32 ulCol, int nFont, const char* text, ... )
{
char		acString[2048];
va_list		marker;
uint32*		pArgs;

	pArgs = (uint32*)( &text ) + 1;

    va_start( marker, text );     
	vsprintf( acString, text, marker );
	if ( ulCol == 0 ) ulCol = 0xd0d0d0d0;			// Default col is an offwhite 

	const char*		pcTextReached = mpFontSystem->TextLimitWidth( nLayer, nX, nY, acString, ulCol, nFont, nMaxWidth );

	if ( pcTextReached )
	{
		return( text + (pcTextReached-acString) );
	}
	return( NULL );
}

void	InterfaceInstance::Text( int nLayer, int nX, int nY, uint32 ulCol, int nFont, const char* text, ... )
{
char		acString[2048];
va_list		marker;
uint32*		pArgs;

	pArgs = (uint32*)( &text ) + 1;

    va_start( marker, text );     
	vsprintf( acString, text, marker );
	if ( ulCol == 0 ) ulCol = 0xd0d0d0d0;			// Default col is an offwhite 

	mpFontSystem->Text( nLayer, nX, nY, acString, ulCol, nFont );
}

void	InterfaceInstance::SetFontFlags( int flags )
{
	mpFontSystem->SetFontFlags( flags );
}

void	InterfaceInstance::TextCentre( int nLayer, int nX1, int nY, uint32 ulCol, int nFont, const char* text, ... )
{
char		acString[4096];
va_list		marker;
uint32*		pArgs;

	pArgs = (uint32*)( &text ) + 1;

    va_start( marker, text );     
	vsprintf( acString, text, marker );
	if ( ulCol == 0 ) ulCol = 0xd0d0d0d0;			// Default col is an offwhite 

	mpFontSystem->TextCentre( nLayer, nX1 - 100, nX1 + 100, nY, acString, ulCol, nFont );
}

void	InterfaceInstance::TextRight( int nLayer, int nX, int nY, uint32 ulCol, int nFont, const char* text, ... )
{
char		acString[4096];
va_list		marker;
uint32*		pArgs;

	pArgs = (uint32*)( &text ) + 1;

    va_start( marker, text );     
	vsprintf( acString, text, marker );
	if ( ulCol == 0 ) ulCol = 0xd0d0d0d0;			// Default col is an offwhite 

	mpFontSystem->TextRight( nLayer, nX, nY, acString, ulCol, nFont );
}


void	InterfaceInstance::SetRenderCanvas()
{
	mpInterfaceInternals->SetRenderCanvas();
}

void	InterfaceInstance::CopyRenderCanvasToBackBuffer( int X, int Y, int W, int H )
{
	mpInterfaceInternals->CopyRenderCanvasToBackBuffer( X, Y, W, H );
}

void	InterfaceInstance::SetViewport( int X, int Y, int nWidth, int nHeight )
{
	mpInterfaceInternals->SetViewport( X, Y, nWidth, nHeight );

}

int		InterfaceInstance::GetStringWidth( const char* pcString, int nFont )
{
	return( mpFontSystem->GetStringWidth( pcString, nFont ) );
}

int		InterfaceInstance::GetStringHeight( const char* pcString, int nFont )
{
	return( mpFontSystem->GetStringHeight( pcString, nFont ) );
}

void	InterfaceInstance::OutlineBox( int nLayer, int nX, int nY, int nWidth, int nHeight, uint32 ulCol)
{
	mpOverlays->OutlineBox( nLayer, nX, nY, nWidth, nHeight, ulCol );
}

void	InterfaceInstance::Rect( int nLayer, int nX, int nY, int nWidth, int nHeight, uint32 ulCol)
{
	mpOverlays->Rect( nLayer, nX, nY, nWidth, nHeight, ulCol );
}

void	InterfaceInstance::Line( int nLayer, int X1, int Y1, int X2, int Y2, uint32 ulCol)
{
	mpOverlays->Line( nLayer, X1, Y1, X2, Y2, ulCol, ulCol );
}


void	InterfaceInstance::Triangle( int nLayer, int nX1, int nY1, int nX2, int nY2, int nX3, int nY3, uint32 ulCol1, uint32 ulCol2, uint32 ulCol3 )
{
	mpOverlays->Triangle( nLayer, nX1, nY1, nX2, nY2,nX3, nY3, ulCol1, ulCol2, ulCol3 );
}


void	InterfaceInstance::ShadedRect( int nLayer, int nX, int nY, int nWidth, int nHeight, uint32 ulCol1, uint32 ulCol2,uint32 ulCol3, uint32 ulCol4 )
{
	mpOverlays->ShadedRect( nLayer, nX, nY, nWidth, nHeight, ulCol1, ulCol2, ulCol3, ulCol4 );
}

void	InterfaceInstance::ReleaseTexture( int nTextureHandle )
{
	mpTexturedOverlays->ReleaseTexture( nTextureHandle );
}

int			InterfaceInstance::LoadTextureFromFileInMem( const char* szFilename, unsigned char* pbMem, int nMemSize, int nFlags )
{
	return( mpTexturedOverlays->GetTextureFromFileInMem( szFilename, pbMem, nMemSize, nFlags ) );
}

int			InterfaceInstance::LoadTextureAsync(const char* szFilename, int nFlags)
{
	return(mpTexturedOverlays->GetTextureInternal(szFilename, nFlags, NOTFOUND, TRUE));
}

int			InterfaceInstance::GetTexture( const char* szFilename, int nFlags, BOOL bAsync )
{
	return( mpTexturedOverlays->GetTextureInternal( szFilename, nFlags, NOTFOUND, bAsync ) );
}

void*		InterfaceInstance::GetRawTexture( int nHandle )
{
	return( mpTexturedOverlays->GetRawTexture( nHandle ) );
}

int		InterfaceInstance::CreateNewTexturedOverlayForEngineTexture( int nLayer, int nEngineTextureHandle )
{
	return( mpTexturedOverlays->CreateOverlayForEngineTexture( nLayer, nEngineTextureHandle ) );

}

int		InterfaceInstance::CreateNewTexturedOverlay( int nLayer, int nTextureHandle )
{
	return( mpTexturedOverlays->CreateOverlay( nLayer, nTextureHandle ) );
}

void InterfaceInstance::TexturedRect( int nOverlayNum, int nX, int nY, int nWidth, int nHeight, uint32 ulCol, float fU, float fV, float fUWidth, float fUHeight )
{
	mpTexturedOverlays->AddRect( nOverlayNum, nX, nY, nWidth, nHeight, ulCol, fU, fV, fUWidth, fUHeight );
}

BOOL	InterfaceInstance::DidLoadFail( int nHandle )
{
	return mpTexturedOverlays->DidLoadFail( nHandle );
}

BOOL	InterfaceInstance::HasFullyLoaded( int nHandle )
{
	return mpTexturedOverlays->HasFullyLoaded( nHandle );
}

