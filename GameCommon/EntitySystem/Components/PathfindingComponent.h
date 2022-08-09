#ifndef PATHFINDING_COMPONENT_H
#define	PATHFINDING_COMPONENT_H


#include "Component.h"

class PathfindingComponent : public Component
{
public:
	PathfindingComponent();

	virtual const char*			GetComponentType( void ) { return( "Pathfinding" ); }
	virtual eCompronentBehaviourTypes	GetBehaviourType( void ) { return( MOVEMENT_BEHAVIOUR_COMPONENT ); }

	virtual void		OnUpdateComponent( float fDelta );

	virtual BOOL		OnEntitySetDestination( const VECT* pxDestination );
	virtual BOOL		OnEntityArrivedAtImmediateDestination( void );

private:
	BOOL		mbHasPathToDestination;
	BYTE*		mpbPathToDestination;
	int			mnPathSize;
	int			mnNextPathWaypoint;
	int			mnCurrentPathMapX;
	int			mnCurrentPathMapY;
	int			mnNextPathMapX;
	int			mnNextPathMapY;
	VECT		mxFinalDestination;

};




#endif