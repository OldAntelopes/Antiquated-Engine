
#include "../../Milkdrop/vis_milk2/plugin.h"
#include "StandardDef.h"
#include "InterfaceEx.h"
#include "Engine.h"
#include "../../Pub/LibCode/Engine/DirectX/EngineDX.h"
#include "../../Pub/LibCode/Engine/DirectX/ShaderLoaderDX.h"

#include "AudioInModule.h"
#include "MilkComponent.h"

CPlugin*	g_pCurrentPlugin = NULL;
_locale_t g_use_C_locale;


CPlugin		mMilkdropPlugin;		


void	MilkEmitterComponent::SetDefaults()
{
}



void	MilkEmitterComponent::InitialiseMilkPlugin() 
{
	mbHasInitialisedMilkPlugin = true;

	// (UNTIL WE RID THIS GLOBAL)
	// WE MUST ALWAYS SET g_pCurrentPlugin before we do anything with milkdrop otherwise we'll get weird thread issues
	g_pCurrentPlugin = &mMilkdropPlugin;
	mMilkdropPlugin.PluginPreInitialize(0, 0, "Data\\Samples\\" );

	int	resW = 1920;
	int resH = 1080;

	D3DPRESENT_PARAMETERS		d3dPp;

	InterfaceGetLastUsedD3DPP( &d3dPp );
	d3dPp.BackBufferWidth = InterfaceInstanceMain()->GetWidth();
	d3dPp.BackBufferHeight = InterfaceInstanceMain()->GetHeight();
	resW = d3dPp.BackBufferWidth;
	resH = d3dPp.BackBufferHeight;
    mMilkdropPlugin.PluginInitialize(
        EngineGetDXDevice(),
        &d3dPp,
        InterfaceGetWindow(),
		resW,
		resH);

	resW = d3dPp.BackBufferWidth;
	resH = d3dPp.BackBufferHeight;

	mhMilkTexture = EngineCreateRenderTargetTexture(resW, resH, 3, "Milk Texture");

}

//#define METHOD_ONE
void	MilkEmitterComponent::OnGraphicsDeviceChanged()
{
#ifdef METHOD_ONE
	if ( mbHasInitialisedMilkPlugin )
	{
		g_pCurrentPlugin = &mMilkdropPlugin;
		mMilkdropPlugin.PluginQuit();
	
		mbHasInitialisedMilkPlugin = false;
		mhMilkTexture = NOTFOUND;
	}
#else
	if ( mbHasInitialisedMilkPlugin )
	{
		g_pCurrentPlugin = &mMilkdropPlugin;

		D3DPRESENT_PARAMETERS		d3dPp;
		InterfaceGetLastUsedD3DPP( &d3dPp );
		d3dPp.BackBufferWidth = InterfaceInstanceMain()->GetWidth();
		d3dPp.BackBufferHeight = InterfaceInstanceMain()->GetHeight();

		mMilkdropPlugin.ResetForGraphicsChange( mpEngineDevice, &d3dPp, InterfaceGetWindow() );

		int	resW = d3dPp.BackBufferWidth;
		int resH = d3dPp.BackBufferHeight;

		mhMilkTexture = EngineCreateRenderTargetTexture(resW, resH, 3, "Milk Texture");
	}
#endif
}


void	MilkEmitterComponent::OnRender( const InstanceData* pCurrentInstanceData, AudioInModule* pAudioInModule )
{

	// (UNTIL WE RID THIS GLOBAL)
	// WE MUST ALWAYS SET g_pCurrentPlugin before we do anything with milkdrop otherwise we'll get weird thread issues
	g_pCurrentPlugin = &mMilkdropPlugin;

	EngineSetRenderTargetTexture(mhMilkTexture, 0, FALSE);

	BYTE*	pcmLeftOut = pAudioInModule->GetAudioBufOutLeft();
	BYTE*	pcmRightOut = pAudioInModule->GetAudioBufOutRight();

	mMilkdropPlugin.SetSoundData( pAudioInModule->GetSoundInfo() );

	// Display the loaded milk file
    mMilkdropPlugin.PluginRender(
        (unsigned char*) pcmLeftOut,
        (unsigned char*) pcmRightOut);

	EngineRestoreRenderTarget();

	// Move this to common util
	EngineCameraUpdate();
	EngineCameraSetProjection( PI/4,0.01f,1000.0f );
	EngineDefaultState();
	EngineSetBlendMode( BLEND_MODE_ALPHABLEND );

	EngineSetVertexShader(NULL, NULL);
	EngineSetPixelShader(NULL, NULL);
	EngineShadersStandardVertexDeclaration(0);

} 


void	MilkEmitterComponent::ActivatePresetIndex( int index )
{
	// Here the app goes off into the land of playlists etc.. for the sample we hardcode a single milk preset to play
wchar_t		wideBuff[512];
const char* szFilename = "Data\\Samples\\martin - cascade.milk";

	mbstowcs(wideBuff, szFilename, 511);
	// Tell the milk plugin to play this preset
	g_pCurrentPlugin = &mMilkdropPlugin;
	mMilkdropPlugin.LoadPreset(wideBuff, 0.0f);
}


void	MilkEmitterComponent::StaticShutdown()
{
}


MilkEmitterComponent::~MilkEmitterComponent()
{
	if ( mbHasInitialisedMilkPlugin )
	{
		g_pCurrentPlugin = &mMilkdropPlugin;
		mMilkdropPlugin.PluginQuit();
		mbHasInitialisedMilkPlugin = false;
	}
}


void	MilkEmitterComponent::OnUpdate( float delta, InstanceData* pCurrentInstanceData )
{
	if ( mbHasInitialisedMilkPlugin == false )
	{
		InitialiseMilkPlugin();
		ActivatePresetIndex(0);
	}

}

