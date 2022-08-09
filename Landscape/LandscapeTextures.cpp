
#include <stdio.h>
#include "StandardDef.h"
#include "Engine.h"

#include "Landscape.h"
#include "LandscapeHeightmap.h"
#include "LandscapeTextures.h"

#include "LandscapeTexturing/LandscapeTileGenTextureSystem.h"
#include "LandscapeTexturing/LandscapeMapTextureSystem.h"

LandscapeTextureSystem*		mspLandscapeTextureSystem = NULL;


void		LandscapeTexturesInit( int nTextureSystem )
{
	switch( (eLandscapeTextureSystem)nTextureSystem )
	{
	case LANDSCAPETEXTURES_TILEGEN:
		mspLandscapeTextureSystem = new LandscapeTileGenTextureSystem;
		break;
	case LANDSCAPETEXTURES_MAP:
		mspLandscapeTextureSystem = new LandscapeMapTextureSystem;	
		break;
	}

	mspLandscapeTextureSystem->Initialise();
}


void		LandscapeTexturesNewFrame( void )
{
	if ( mspLandscapeTextureSystem )
	{
		mspLandscapeTextureSystem->NewFrame();
	}
}


void		LandscapeTexturesShutdown( void )
{
	if ( mspLandscapeTextureSystem )
	{
		mspLandscapeTextureSystem->Shutdown();
		delete mspLandscapeTextureSystem;
		mspLandscapeTextureSystem = NULL;
	}
}


void		LandscapeTextureApply( int nLandscapeTextureNum, int nMapX, int nMapY )
{
	mspLandscapeTextureSystem->ApplyTexture( nLandscapeTextureNum, nMapX, nMapY );
}

void		LandscapeTextureGetUVs( int nLandscapeTextureNum, int nMapX, int nMapY, float* pfUBase, float* pfVBase, float* pfUWidth, float* pfVHeight )
{
	mspLandscapeTextureSystem->GetUVs( nLandscapeTextureNum, nMapX, nMapY, pfUBase, pfVBase, pfUWidth, pfVHeight );
}

int		LandscapeTextureGetTextureNum( int nMapX, int nMapY )
{
	return( mspLandscapeTextureSystem->GetTextureNum( nMapX, nMapY ) );
}

// ------------ Set texture 
void		LandscapeSetTextureResource( int nTextureType, int nTextureTypeIndex, int hTexture )
{
	if ( mspLandscapeTextureSystem )
	{
		mspLandscapeTextureSystem->SetTextureResource( nTextureType, nTextureTypeIndex, hTexture );
	}

}
