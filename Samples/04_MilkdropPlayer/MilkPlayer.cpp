
#include "StandardDef.h"
#include "InterfaceEx.h"
#include "Engine.h"

#include "MilkComponent.h"
#include "AudioInModule.h"
#include "MilkPlayer.h"


void	MilkPlayerSample::Initialise()
{
	mpMilkComponent = new MilkEmitterComponent();
	mpAudioInModule = new AudioInModule();
	mpAudioInModule->OnInitialise();
}


void	MilkPlayerSample::Update( float delta )
{
	mpMilkComponent->OnUpdate(delta, NULL);
	mpAudioInModule->OnUpdate(delta);

}

void	MilkPlayerSample::Render()
{
	// Renders the milkdrop output to a render target texture
	mpMilkComponent->OnRender(NULL, mpAudioInModule );	

	int			hTexture = mpMilkComponent->GetMilkTexture();

	int		nInterfaceOverlay = EngineTextureCreateInterfaceOverlay( 0, hTexture );
	InterfaceTexturedRect(nInterfaceOverlay, 0, 0, InterfaceGetWidth(), InterfaceGetHeight(), 0xFFFFFFFF);
}


void	MilkPlayerSample::Shutdown()
{
	
}