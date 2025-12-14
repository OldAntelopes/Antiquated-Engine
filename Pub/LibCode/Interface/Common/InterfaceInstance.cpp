#include "../DirectX/InterfaceInternalsDX.h"
#include "StandardDef.h"

#include "Interface.h"

#include "../DirectX/Image.h"
#include "Overlays/TexturedOverlays.h"
#include "../Common/Font/FontCommon.h"
#include "InterfaceInstance.h"

InterfaceInstance::InterfaceInstance()
{
	mpTexturedOverlays = new TexturedOverlays;
	mpInterfaceInternals = new InterfaceInternalsDX;
	mpFontSystem = new FontSystem;
}

void		InterfaceInstance::InitialiseInstance( BOOL bUseDefaultFonts )
{
	if ( mboInterfaceInitialised == FALSE )
	{
		InterfaceSetIsUsingDefaultFonts( bUseDefaultFonts );
		mpFontSystem->InitialiseFonts( bUseDefaultFonts );
		InitialiseOverlays();

		mpTexturedOverlays->Initialise();

		InterfaceImagesInit();
	}

	mboInterfaceInitialised = TRUE;
}

void	InterfaceInstance::SetDevice( LPGRAPHICSDEVICE pDevice )
{
	mpInterfaceD3DDevice = pDevice;
	mpTexturedOverlays->InitialiseModule( pDevice, this );
	mpInterfaceInternals->InitialiseModule( pDevice, this );
	mpFontSystem->InitialiseModule( pDevice, this );
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
