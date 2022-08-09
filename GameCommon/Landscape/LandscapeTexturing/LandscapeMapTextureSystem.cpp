
#include <stdio.h>
#include "StandardDef.h"
#include "Engine.h"

#include "../Landscape.h"
#include "../LandscapeHeightmap.h"
#include "../LandscapeTextures.h"

#include "../MapData/MapTileData.h"

#include "LandscapeMapTextureSystem.h"


int		mshLandscapeMapMainSurfaceTextureHandle = NOTFOUND;
int		mshLandscapeMapTempSecondarySurfaceTextureHandle = NOTFOUND;


class MapTexturesTextureResourceList
{
public:
	int			mnTextureNum;

	int			mnTextureType;
	int			mnTextureTypeIndex;
	int			mhTextureHandle;

	MapTexturesTextureResourceList*		mpNext;
};

//---------------------------------------------------------------------------------------------------------------

void		LandscapeMapTextureSystem::SetTextureResource( int nTextureType, int nTextureTypeIndex, int hTexture )
{
MapTexturesTextureResourceList*		pNewTextureResource = new MapTexturesTextureResourceList;

	pNewTextureResource->mpNext = mpMapTexturesTextureResourceList;
	mpMapTexturesTextureResourceList = pNewTextureResource;

	pNewTextureResource->mnTextureType = nTextureType;
	pNewTextureResource->mnTextureTypeIndex = nTextureTypeIndex;
	pNewTextureResource->mhTextureHandle = hTexture;

	pNewTextureResource->mnTextureNum = mnNextTextureNum++;
}


void		LandscapeMapTextureSystem::Initialise( void )
{
	mpMapTexturesTextureResourceList = NULL;
	mnNextTextureNum = 1;

	mshLandscapeMapMainSurfaceTextureHandle = EngineLoadTexture( "Data\\Textures\\Land\\At2StdGrass1.jpg", 0, 0 );
	mshLandscapeMapTempSecondarySurfaceTextureHandle = EngineLoadTexture( "Data\\Textures\\Land\\At2Sand1.jpg", 0, 0 );
}

void		LandscapeMapTextureSystem::Shutdown( void )
{
MapTexturesTextureResourceList*		pTextureResourceList = mpMapTexturesTextureResourceList;
MapTexturesTextureResourceList*		pNext;

	while( pTextureResourceList )
	{
		pNext = pTextureResourceList->mpNext;
		delete pTextureResourceList;
		pTextureResourceList = pNext;
	}
	mpMapTexturesTextureResourceList = NULL;

	EngineReleaseTexture( &mshLandscapeMapMainSurfaceTextureHandle );
	EngineReleaseTexture( &mshLandscapeMapTempSecondarySurfaceTextureHandle );

}


void		LandscapeMapTextureSystem::NewFrame( void )
{
}

void		LandscapeMapTextureSystem::ApplyTexture( int nLandscapeTextureNum, int nMapX, int nMapY )
{
	if ( nLandscapeTextureNum != 0 )
	{
	MapTexturesTextureResourceList*		pTextureResourceList = mpMapTexturesTextureResourceList;

		while( pTextureResourceList )
		{
			if ( pTextureResourceList->mnTextureNum == nLandscapeTextureNum )
			{
				EngineSetTexture( 0, pTextureResourceList->mhTextureHandle );
				return;
			}
			pTextureResourceList = pTextureResourceList->mpNext;
		}
	}

	EngineSetTexture( 0, mshLandscapeMapMainSurfaceTextureHandle );
}

void		LandscapeMapTextureSystem::GetUVs( int nLandscapeTextureNum, int nMapX, int nMapY, float* pfUBase, float* pfVBase, float* pfUWidth, float* pfVHeight )
{
	if ( nLandscapeTextureNum == 0 )
	{
	int		nTileNum = ( (nMapX*3) + nMapY) % 4;
	float	fTileBase = (nTileNum/2) * 0.5f;
	float	fTileBaseY = (nTileNum%2) * 0.5f;

		*pfUBase = fTileBase;
		*pfVBase = fTileBaseY;
		*pfUWidth = fTileBase + 0.5f;
		*pfVHeight = fTileBaseY + 0.5f;
	}
	else
	{
		*pfUBase = 0.0f;
		*pfVBase = 0.0f;
		*pfUWidth = 1.0f;
		*pfVHeight = 1.0f;
	}
}

//#define		ALWAYS_LO_RES

int		LandscapeMapTextureSystem::GetTextureNum( int nMapX, int nMapY )
{
TILE_TEXTURE		xTileTexture;
MapTexturesTextureResourceList*		pTextureResourceList = mpMapTexturesTextureResourceList;

	MapTileDataGetTexture( nMapX, nMapY, &xTileTexture );

	if ( xTileTexture.nTextureType == LANDSCAPE_TEXTURETYPE_MAINSURF )
	{
		return( 0 );
	}
	else
	{
		while( pTextureResourceList )
		{
			if ( ( pTextureResourceList->mnTextureType == xTileTexture.nTextureType ) &&
				 ( pTextureResourceList->mnTextureTypeIndex == xTileTexture.nTextureTypeIndex ) )
			{
				return( pTextureResourceList->mnTextureNum );
			}
			pTextureResourceList = pTextureResourceList->mpNext;
		}
	}
	return( 0 );
 }



