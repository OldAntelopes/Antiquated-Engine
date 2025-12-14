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


void	InterfaceInstance::Text( int nLayer, int nX, int nY, const char* szString, uint32 ulCol, int nFont )
{
	mpFontSystem->Text( nLayer, nX, nY, szString, ulCol, nFont );
}
void	InterfaceInstance::TextCentre( int nLayer, int nX1, int nX2, int nY, const char* szString, uint32 ulCol, int nFont )
{
	mpFontSystem->TextCentre( nLayer, nX1, nX2, nY, szString, ulCol, nFont );

}
void	InterfaceInstance::TextRight( int nLayer, int nX, int nY, const char* szString, uint32 ulCol, int nFont )
{
	mpFontSystem->TextRight( nLayer, nX, nY, szString, ulCol, nFont );
}

int		InterfaceInstance::GetStringWidth( const char* pcString, int nFont )
{
	return( mpFontSystem->GetStringWidth( pcString, nFont ) );
}

int		InterfaceInstance::GetStringHeight( const char* pcString, int nFont )
{
	return( mpFontSystem->GetStringHeight( pcString, nFont ) );
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