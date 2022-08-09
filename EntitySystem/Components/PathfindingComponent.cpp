
#include "StandardDef.h"

#include "../../Landscape/MapData/PathFinding.h"

#include "../Entity.h"
#include "PathfindingComponent.h"

REGISTER_COMPONENT(PathfindingComponent, "Pathfinding" );

PathfindingComponent::PathfindingComponent()
{
	mpbPathToDestination = NULL;
	mbHasPathToDestination = FALSE;
	memset( &mxFinalDestination, 0, sizeof( mxFinalDestination ) );

}


void	PathfindingComponent::OnUpdateComponent( float fDelta )
{


}
	
BOOL	PathfindingComponent::OnEntitySetDestination( const VECT* pxDestination )
{
	if ( pxDestination )
	{
	int		nPathSize;
	BYTE*	pbPathToDestination;
	int		nSourcePathMapX;
	int		nSourcePathMapY;
	VECT	xPos = *GetEntity()->GetPos();

		// TODO - This will need to be made async..
		pbPathToDestination = PathDataFindPath( xPos.x, xPos.y, pxDestination->x, pxDestination->y, &nPathSize, &nSourcePathMapX, &nSourcePathMapY );
		if ( pbPathToDestination )
		{
		VECT	xNextDestination;

			if ( mpbPathToDestination )
			{
				free( mpbPathToDestination);
				mpbPathToDestination = NULL;
				mbHasPathToDestination = FALSE;
			}

			mpbPathToDestination = (BYTE*)( malloc( nPathSize ) );
			memcpy( mpbPathToDestination, pbPathToDestination, nPathSize );
			mnCurrentPathMapX = nSourcePathMapX;
			mnCurrentPathMapY = nSourcePathMapY;
			mbHasPathToDestination = TRUE;
			mnNextPathWaypoint = 0;
			mnPathSize = nPathSize;
			PathDataGetNextWaypoint( mnCurrentPathMapX, mnCurrentPathMapY, mpbPathToDestination[mnNextPathWaypoint], &mnNextPathMapX, &mnNextPathMapY );
			xNextDestination.x = PathDataGetGameWorldX( mnNextPathMapX );
			xNextDestination.y = PathDataGetGameWorldY( mnNextPathMapY );

			mxFinalDestination = *pxDestination;
			GetEntity()->SetImmediateDestination( &xNextDestination );
			mnNextPathWaypoint++;
		}
	}
	else
	{
		if ( mpbPathToDestination )
		{
			free( mpbPathToDestination);
			mpbPathToDestination = NULL;
			mbHasPathToDestination = FALSE;
		}
		GetEntity()->SetImmediateDestination( NULL );
	}
	return( TRUE );
}

BOOL		PathfindingComponent::OnEntityArrivedAtImmediateDestination( void )
{
	if ( mbHasPathToDestination )
	{
		mnCurrentPathMapX = mnNextPathMapX;
		mnCurrentPathMapY = mnNextPathMapY;

		if ( mnNextPathWaypoint < mnPathSize )
		{
		VECT	xNextDestination;

			PathDataGetNextWaypoint( mnCurrentPathMapX, mnCurrentPathMapY, mpbPathToDestination[mnNextPathWaypoint], &mnNextPathMapX, &mnNextPathMapY );
			xNextDestination.x = PathDataGetGameWorldX( mnNextPathMapX );
			xNextDestination.y = PathDataGetGameWorldY( mnNextPathMapY );
			GetEntity()->SetImmediateDestination( &xNextDestination );
			mnNextPathWaypoint++;
		}
		else
		{
			GetEntity()->SetDestination( NULL );
		}
		return( TRUE );
	}

	return( FALSE );

}
