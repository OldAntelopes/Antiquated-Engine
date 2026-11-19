
#include <StandardDef.h>
#include <Engine.h>
#include <Interface.h>

#include "ModelDisplayOptions.h"

// Lights from model effects use engine light slots from here up (the editor's own lighting uses 0 and 1)
#define		MODELDISPLAY_FIRST_LIGHT_SLOT		2
#define		MODELDISPLAY_MAX_LIGHTS				4

typedef struct
{
	VECT	xPos;
	float	fRange;
	float	fAtten0;
	float	fAtten1;
	float	fR;
	float	fG;
	float	fB;

} MODELDISPLAY_LIGHT;

static BOOL		msbModelDisplayDaytime = FALSE;			// Night, as the converter has always shown models (IsDaytime used to default to false)
static uint32	msulModelDisplayChannels = 0;
static BOOL		msbModelDisplayRelief = FALSE;
static float	msfModelDisplayPlaybackSpeed = 1.0f;
static float	msfModelDisplayTickRemainder = 0.0f;

static MODELDISPLAY_LIGHT	msaxModelDisplayLights[MODELDISPLAY_MAX_LIGHTS];
static int					msnModelDisplayNumLights = 0;
static MODELDISPLAY_LIGHT	msaxModelDisplayLastLights[MODELDISPLAY_MAX_LIGHTS];
static int					msnModelDisplayNumLastLights = 0;
static int					msnModelDisplayNumLightsActive = 0;

//-------------------------------------------------------------------------

void	ModelDisplayOptionsSetDaytime( BOOL bDaytime )
{
	msbModelDisplayDaytime = bDaytime;
}

BOOL	ModelDisplayOptionsIsDaytime( void )
{
	return( msbModelDisplayDaytime );
}

void	ModelDisplayOptionsSetChannels( uint32 ulChannelFlags )
{
	msulModelDisplayChannels = ulChannelFlags;
}

uint32	ModelDisplayOptionsGetChannels( void )
{
	return( msulModelDisplayChannels );
}

void	ModelDisplayOptionsSetRelief( BOOL bOn )
{
	msbModelDisplayRelief = bOn;
	// The Universal's defaults (render.modelrelief* tweakables in its DrawWorld.cpp)
	EngineSetModelRelief( bOn, 1.0f, 10.0f, 1.0f, 40.0f, 90.0f );
}

BOOL	ModelDisplayOptionsGetRelief( void )
{
	return( msbModelDisplayRelief );
}

void	ModelDisplayOptionsSetPlaybackSpeed( float fSpeed )
{
	msfModelDisplayPlaybackSpeed = fSpeed;
	msfModelDisplayTickRemainder = 0.0f;
}

float	ModelDisplayOptionsGetPlaybackSpeed( void )
{
	return( msfModelDisplayPlaybackSpeed );
}

uint32	ModelDisplayOptionsScaleTick( uint32 ulRealTickGap )
{
float	fTicks = ( (float)ulRealTickGap * msfModelDisplayPlaybackSpeed ) + msfModelDisplayTickRemainder;
uint32	ulTicks = (uint32)fTicks;

	msfModelDisplayTickRemainder = fTicks - (float)ulTicks;
	return( ulTicks );
}

//-------------------------------------------------------------------------
// Function    : ModelDisplayOptionsAddLight
// Description : Called (via EngineGameInterface::AddLight) while a model's effects are processed in its draw. The lights
//				 are switched on for the next frame's draw
//-------------------------------------------------------------------------
void	ModelDisplayOptionsAddLight( const VECT* pxPos, float fRange, float fAtten0, float fAtten1, float fR, float fG, float fB )
{
MODELDISPLAY_LIGHT*		pxLight;

	if ( ( pxPos == NULL ) || ( msnModelDisplayNumLights >= MODELDISPLAY_MAX_LIGHTS ) ) return;

	pxLight = &msaxModelDisplayLights[msnModelDisplayNumLights++];
	pxLight->xPos = *pxPos;
	pxLight->fRange = ( fRange > 0.0f ) ? fRange : 1.0f;
	pxLight->fAtten0 = fAtten0;
	pxLight->fAtten1 = fAtten1;
	pxLight->fR = fR;
	pxLight->fG = fG;
	pxLight->fB = fB;
}

//-------------------------------------------------------------------------
// Function    : ModelDisplayOptionsBeginModelDraw
// Description : Switches on last frame's effect lights and the chosen material channels
//-------------------------------------------------------------------------
void	ModelDisplayOptionsBeginModelDraw( void )
{
ENGINE_LIGHT	xLight;
int				nLoop;

	memcpy( msaxModelDisplayLastLights, msaxModelDisplayLights, sizeof( msaxModelDisplayLights ) );
	msnModelDisplayNumLastLights = msnModelDisplayNumLights;
	msnModelDisplayNumLights = 0;

	for ( nLoop = 0; nLoop < msnModelDisplayNumLastLights; nLoop++ )
	{
	MODELDISPLAY_LIGHT*		pxLight = &msaxModelDisplayLastLights[nLoop];

		memset( &xLight, 0, sizeof( xLight ) );
		xLight.Type = POINT_LIGHT;
		xLight.Diffuse.r = pxLight->fR;
		xLight.Diffuse.g = pxLight->fG;
		xLight.Diffuse.b = pxLight->fB;
		xLight.Diffuse.a = 1.0f;
		xLight.Specular = xLight.Diffuse;
		xLight.Position = pxLight->xPos;
		xLight.Range = pxLight->fRange;
		// Falls to about a fifth at the edge of its range for a sign lamp (atten1 = 4)
		xLight.Attenuation0 = 1.0f;
		xLight.Attenuation1 = pxLight->fAtten0 / pxLight->fRange;
		xLight.Attenuation2 = pxLight->fAtten1 / ( pxLight->fRange * pxLight->fRange );
		EngineActivateLight( MODELDISPLAY_FIRST_LIGHT_SLOT + nLoop, &xLight );
	}
	for ( nLoop = msnModelDisplayNumLastLights; nLoop < msnModelDisplayNumLightsActive; nLoop++ )
	{
		EngineDeactivateLight( MODELDISPLAY_FIRST_LIGHT_SLOT + nLoop );
	}
	msnModelDisplayNumLightsActive = msnModelDisplayNumLastLights;

	EngineClearModelMaterialChannels();
	if ( msulModelDisplayChannels & MODELDISPLAY_CHANNEL_HEADLIGHTS ) EngineSetModelMaterialChannel( 1, 1.0f );
	if ( msulModelDisplayChannels & MODELDISPLAY_CHANNEL_BRAKE )
	{
		EngineSetModelMaterialChannel( 2, 1.0f );
	}
	else if ( msulModelDisplayChannels & MODELDISPLAY_CHANNEL_REVERSE )
	{
		// The Universal's reversing lamps (VehicleLamps.cpp)
		EngineSetModelMaterialChannelColour( 2, 1.0f, 1.4f, 1.4f, 1.3f );
	}
	if ( msulModelDisplayChannels & MODELDISPLAY_CHANNEL_LEFT ) EngineSetModelMaterialChannel( 3, 1.0f );
	if ( msulModelDisplayChannels & MODELDISPLAY_CHANNEL_RIGHT ) EngineSetModelMaterialChannel( 4, 1.0f );
}

void	ModelDisplayOptionsEndModelDraw( void )
{
	EngineClearModelMaterialChannels();
}

//-------------------------------------------------------------------------
// Function    : ModelDisplayOptionsDrawOverlay
// Description : A small marker in the light's colour where each effect light is
//-------------------------------------------------------------------------
void	ModelDisplayOptionsDrawOverlay( void (*fnProject)( VECT*, int*, int* ) )
{
int		nLoop;
int		nX;
int		nY;
uint32	ulCol;

	for ( nLoop = 0; nLoop < msnModelDisplayNumLastLights; nLoop++ )
	{
	MODELDISPLAY_LIGHT*		pxLight = &msaxModelDisplayLastLights[nLoop];
	int		nR = (int)( FClamp( pxLight->fR, 0.0f, 1.0f ) * 255.0f );
	int		nG = (int)( FClamp( pxLight->fG, 0.0f, 1.0f ) * 255.0f );
	int		nB = (int)( FClamp( pxLight->fB, 0.0f, 1.0f ) * 255.0f );

		fnProject( &pxLight->xPos, &nX, &nY );
		ulCol = 0xFF000000 | ( nR << 16 ) | ( nG << 8 ) | nB;
		InterfaceLine( 1, nX - 5, nY, nX + 6, nY, ulCol, ulCol );
		InterfaceLine( 1, nX, nY - 5, nX, nY + 6, ulCol, ulCol );
		InterfaceLine( 1, nX - 3, nY - 3, nX + 4, nY + 4, 0xC0FFFFFF, 0xC0FFFFFF );
		InterfaceLine( 1, nX - 3, nY + 3, nX + 4, nY - 4, 0xC0FFFFFF, 0xC0FFFFFF );
	}
}
