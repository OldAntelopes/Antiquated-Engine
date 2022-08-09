
#include "StandardDef.h"

#include "LandscapeMap.h"

LANDSCAPE_MAP_DATA*		mspLandscapeMapData = NULL;

LANDSCAPE_MAP_DATA		mxBlankTile;

void						LandscapeMapInit( void )
{

	mxBlankTile.nLandscapeTextureNum = 0;
	mxBlankTile.fLandHeight = 0.0f;
}

LANDSCAPE_MAP_DATA*		LandscapeMapGetData( int nMapX, int nMapY )
{


	return( &mxBlankTile );
}

void						LandscapeMapShutdown( void )
{


}

