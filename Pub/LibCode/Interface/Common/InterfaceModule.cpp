#include "../DirectX/InterfaceInternalsDX.h"
#include "StandardDef.h"

#include "Interface.h"

#include "Overlays/TexturedOverlays.h"
#include "InterfaceInstance.h"

void	InterfaceInstance::InitialiseInstance()
{
	mpTexturedOverlays = new TexturedOverlays;
}

InterfaceInstance*		InterfaceInstanceMain()
{
static InterfaceInstance		ms_MainSingletonInstance;
static BOOL			ms_bHasInitialisedMainInstance = FALSE;

	if ( !ms_bHasInitialisedMainInstance )
	{
		ms_MainSingletonInstance.InitialiseInstance();
		ms_bHasInitialisedMainInstance = TRUE;
	}
	return( &ms_MainSingletonInstance );
}
