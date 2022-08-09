
#include <stdio.h>
#include "StandardDef.h"
#include "Interface.h"
#include "Engine.h"

#include "../LandscapeCoords.h"
#include "../Landscape.h"

#include "Trees.h"
#include "TreeLists.h"
#include "TreeGenerator.h"

#include "../MapData/MapGen.h"
#include "../MapData/MapTileData.h"

void		TreesGenerate( void )
{
LandscapeInitParams*		pxLandscapeInitParams = LandscapeGetInitParams();
int		nLoop;
//int		nNumTrees = 20000;
int		nNumTrees = (int)( 30000 * pxLandscapeInitParams->fTreeGenDensity );
float	fWorldX;
float	fWorldY;
float	fWorldMaxX = LandscapeGetMapSizeX() * LandscapeMapToWorldScale();
float	fWorldMaxY = LandscapeGetMapSizeY() * LandscapeMapToWorldScale();
Tree*	pTree;
int		nRandTries;
int		nMaxRandTries = 10;
eMapGenType		tileType;
BOOL	bSpotOk;

	MapGenSRand( 3482194 );

	MapGenLoadBitmap();

	MapTileDataParseMapGen();

	for ( nLoop = 0; nLoop < nNumTrees; nLoop++ )
	{
		for ( nRandTries = 0; nRandTries < nMaxRandTries; nRandTries++ )
		{
			fWorldX = MapGenFRand( 0.0f, fWorldMaxX );
			fWorldY = MapGenFRand( 0.0f, fWorldMaxY );

			bSpotOk = FALSE;
			tileType = MapGenGetTileType( fWorldX, fWorldY );
			switch( tileType )
			{
			case MAPGEN_CLIFF:
			case MAPGEN_SEA:
			case MAPGEN_SAND:
			case MAPGEN_EMPTY:
				break;
			case MAPGEN_FULLGRASS:
				if ( ( MapGenRand( 10 ) ) == 0 )
				{
					bSpotOk = TRUE;
				}
				break;
			case MAPGEN_PARTIALGRASS:
			case MAPGEN_ROCK:
				bSpotOk = TRUE;
				break;
			}

			if ( bSpotOk )
			{
				break;
			}
		}
	
		if ( nRandTries < nMaxRandTries )
		{
			pTree = new Tree;

			pTree->mxPos.x = fWorldX;
			pTree->mxPos.y = fWorldY;
			pTree->mxPos.z = 0.0f;
			pTree->mfRot = MapGenFRand( 0.0f, A360 );
			pTree->mfTreeSpriteSize = MapGenFRand( pxLandscapeInitParams->fTreeScaleMin, pxLandscapeInitParams->fTreeScaleMax );

			TreeListsAddTree( pTree );
		}

	}

	MapGenReleaseBitmap();
}
