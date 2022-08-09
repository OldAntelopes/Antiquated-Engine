
#include <stdio.h>
#include "StandardDef.h"
#include "Engine.h"

#include "../LandscapeCoords.h"
#include "../Landscape.h"

#include "libastar\astar.h"

typedef struct
{
	ulong	ulPathFileHeaderCode;
	BYTE	bVersion;
	BYTE	bSizeOfHeader;
	BYTE	bPad1;
	BYTE	bPad2;
	
	int		nMapSizeX;
	int		nMapSizeY;

} PATH_FILE_HEADER;


int		mnPathMapSizeX = 0;
int		mnPathMapSizeY = 0;

BYTE*	mspbPathData = NULL;
astar_t*		mspAstar = NULL;
direction_t*	mspLastDirections = NULL;

BYTE	map_get( ulong x, ulong y)
{
BYTE*	pbMapData = mspbPathData + ((y*mnPathMapSizeX) + x);

	if ( *pbMapData == 0 )
	{
		return( 255 );
	}
	return( 1 );
}

void	PathDataLoad( void )
{
char	acFilename[256];
void*	pFile;
PATH_FILE_HEADER		xPathFileHeader;
int		nPathMapSize;

	sprintf( acFilename, "%s\\pathdata.pth", LandscapeGetLevelRootPath() );
	
	pFile = SysFileOpen( acFilename, "rb" );
	if ( pFile )
	{
		SysFileRead( (BYTE*)&xPathFileHeader, sizeof( xPathFileHeader ), 1, pFile );

		mnPathMapSizeX = xPathFileHeader.nMapSizeX;
		mnPathMapSizeY = xPathFileHeader.nMapSizeY;

		nPathMapSize = mnPathMapSizeX * mnPathMapSizeY;
		mspbPathData = (BYTE*)malloc( nPathMapSize );

		SysFileRead( mspbPathData, nPathMapSize, 1, pFile );

		SysFileClose( pFile );
	}

	mspAstar = astar_new (mnPathMapSizeX, mnPathMapSizeY, map_get, NULL);
    astar_set_origin (mspAstar, 0, 0);

}

void	PathDataFree( void )
{
	SAFE_FREE( mspbPathData );
	astar_destroy( mspAstar );
	mspAstar = NULL;
}

void	PathDataFindValidPosition( int* pnPathMapX, int* pnPathMapY )
{
ulong	ulPathMapX = (ulong)( *pnPathMapX );
ulong	ulPathMapY = (ulong)( *pnPathMapY );

	if ( map_get( ulPathMapX, ulPathMapY ) == 255 )
	{
	ulong		ulLoopX;
	ulong		ulLoopY;

		for( ulLoopY = ulPathMapY - 1; ulLoopY <= ulPathMapY + 1; ulLoopY++ )
		{
			for( ulLoopX = ulPathMapX - 1; ulLoopX <= ulPathMapX + 1; ulLoopX++ )
			{
				if ( map_get( ulLoopX, ulLoopY ) != 255 )
				{
					*pnPathMapX = (int)ulLoopX;
					*pnPathMapY = (int)ulLoopY;
					return;
				}
			}
		}
	}
}

// TODO - We'll want to async/thread (and pipe) this entire thing...

BYTE*	PathDataFindPath( float fSourceWorldX, float fSourceWorldY, float fDestWorldX, float fDestWorldY, int* pnNumSteps, int* pnSourcePathMapX, int* pnSourcePathMapY )
{
float	fMapModX = (float)mnPathMapSizeX / LandscapeGetMapSizeX();
float	fMapModY = (float)mnPathMapSizeY / LandscapeGetMapSizeY();
float	fWorldToGameMap = LandscapeWorldToMapScale();
int		nPathMapSourceX, nPathMapSourceY;
int		nPathMapDestX, nPathMapDestY;
int		result;

	if ( mspLastDirections )
	{
		astar_free_directions( mspLastDirections );
		mspLastDirections = NULL;
	}

	if ( mspbPathData )
	{
		// Convert world coords to the map scale used in the path finding data..
		nPathMapSourceX = (int)( (fSourceWorldX * fWorldToGameMap) * fMapModX );
		nPathMapSourceY = (int)( (fSourceWorldY * fWorldToGameMap) * fMapModY );

		PathDataFindValidPosition( &nPathMapSourceX, &nPathMapSourceY );

		nPathMapDestX = (int)( (fDestWorldX * fWorldToGameMap) * fMapModX );
		nPathMapDestY = (int)( (fDestWorldY * fWorldToGameMap) * fMapModY );

		PathDataFindValidPosition( &nPathMapDestX, &nPathMapDestY );

	   result = astar_run (mspAstar, nPathMapSourceX, nPathMapSourceY, nPathMapDestX, nPathMapDestY);
	   if (astar_have_route (mspAstar)) 
	   {
	   ulong num_steps = astar_get_directions (mspAstar, &mspLastDirections);

			*pnSourcePathMapX = nPathMapSourceX;
			*pnSourcePathMapY = nPathMapSourceY;
			*pnNumSteps = (int)( num_steps );
			return( mspLastDirections );
	   }
	   else
	   {
		   *pnNumSteps = -1;
		   return( NULL );
	   }
	}
	else
	{
	   *pnNumSteps = -1;
	   return( NULL );
	}
}


float		PathDataGetGameWorldX( int nPathMapX )
{
float	fGameMapToWorld = LandscapeMapToWorldScale();
float	fMapModX = (float)LandscapeGetMapSizeX() / mnPathMapSizeX;
float	fWorldX;

	fWorldX = ( ( nPathMapX * fMapModX ) * fGameMapToWorld ) + (fGameMapToWorld*0.5f);
	return( fWorldX );
}


float		PathDataGetGameWorldY( int nPathMapY )
{
float	fGameMapToWorld = LandscapeMapToWorldScale();
float	fMapModY = (float)LandscapeGetMapSizeY() / mnPathMapSizeY;
float	fWorldY;

	fWorldY = ( ( nPathMapY * fMapModY ) * fGameMapToWorld ) + (fGameMapToWorld*0.5f);
	return( fWorldY );
}


void		PathDataGetNextWaypoint( int nCurrentPathMapX, int nCurrentPathMapY, int nDirection, int* pnNextPathMapX, int* pnNextPathMapY )
{
	switch( nDirection )
	{
	case 0:
		*pnNextPathMapX = nCurrentPathMapX;
		*pnNextPathMapY = nCurrentPathMapY - 1;
		break;
	case 1:
		*pnNextPathMapX = nCurrentPathMapX + 1;
		*pnNextPathMapY = nCurrentPathMapY - 1;
	case 2:
		*pnNextPathMapX = nCurrentPathMapX + 1;
		*pnNextPathMapY = nCurrentPathMapY;
		break;
	case 3:
		*pnNextPathMapX = nCurrentPathMapX + 1;
		*pnNextPathMapY = nCurrentPathMapY + 1;
		break;
	case 4:
		*pnNextPathMapX = nCurrentPathMapX;
		*pnNextPathMapY = nCurrentPathMapY + 1;
		break;
	case 5:
		*pnNextPathMapX = nCurrentPathMapX - 1;
		*pnNextPathMapY = nCurrentPathMapY + 1;
		break;
	case 6:
		*pnNextPathMapX = nCurrentPathMapX - 1;
		*pnNextPathMapY = nCurrentPathMapY;
		break;
	case 7:
		*pnNextPathMapX = nCurrentPathMapX - 1;
		*pnNextPathMapY = nCurrentPathMapY - 1;
		break;
	}

}
