#include "../DirectX/InterfaceInternalsDX.h"
#include "StandardDef.h"

#include "Interface.h"

#include "../DirectX/Image.h"
#include "Overlays/TexturedOverlays.h"
#include "../Common/Font/FontCommon.h"
#include "InterfaceInstance.h"

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


void	InterfaceInstance::Text( int nLayer, int nX, int nY, uint32 ulCol, int nFont, const char* text, ... )
{
char		acString[4096];
va_list		marker;
uint32*		pArgs;

	pArgs = (uint32*)( &text ) + 1;

    va_start( marker, text );     
	vsprintf( acString, text, marker );
	if ( ulCol == 0 ) ulCol = 0xd0d0d0d0;			// Default col is an offwhite 

	mpFontSystem->Text( nLayer, nX, nY, acString, ulCol, nFont );
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

int		InterfaceInstance::GetStringWidth( const char* pcString, int nFont )
{
	return( mpFontSystem->GetStringWidth( pcString, nFont ) );
}

int		InterfaceInstance::GetStringHeight( const char* pcString, int nFont )
{
	return( mpFontSystem->GetStringHeight( pcString, nFont ) );
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

int			InterfaceInstance::GetTexture( const char* szFilename, int nFlags )
{
	return( mpTexturedOverlays->GetTextureInternal( szFilename, nFlags, NOTFOUND ) );
}

int		InterfaceInstance::CreateNewTexturedOverlay( int nLayer, int nTextureHandle )
{
	return( mpTexturedOverlays->CreateOverlay( nLayer, nTextureHandle ) );
}

void InterfaceInstance::TexturedRect( int nOverlayNum, int nX, int nY, int nWidth, int nHeight, uint32 ulCol, float fU, float fV, float fUWidth, float fUHeight )
{
	mpTexturedOverlays->AddRect( nOverlayNum, nX, nY, nWidth, nHeight, ulCol, fU, fV, fUWidth, fUHeight );
}