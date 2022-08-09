
#include <math.h>
#include "StandardDef.h"
#include "Engine.h"
#include "Rendering.h"

#include "../Entity.h"
#include "SimpleTravelComponent.h"

//------------------------------------------------------------------------------------------

REGISTER_COMPONENT(SimpleTravelComponent, "SimpleTravel" );
 
//------------------------------------------------------------------------------------------


SimpleTravelComponent::SimpleTravelComponent()
{
			//------- Farm off to component?
	mbIsTravelling = FALSE;
	mulTargetEntityUID = 0;
	mfTargetDist = 0.0f;
	memset( &mxDestination, 0, sizeof( mxDestination ) );

}

void	SimpleTravelComponent::OnUpdateComponent( float fDelta )
{
	if ( mbIsTravelling )
	{
		MoveToDestination( fDelta, &mxDestination );
	}
}
	
void	SimpleTravelComponent::OnRenderComponent( void )
{

}

BOOL	SimpleTravelComponent::OnEntitySetDestination( const VECT* pxDestination )
{
	if ( pxDestination )
	{
		mbIsTravelling = TRUE;
		mxDestination = *pxDestination;
	}
	else
	{
		mbIsTravelling = FALSE;
	}
	return( TRUE );
}


BOOL	SimpleTravelComponent::MoveToDestination( float fDelta, const VECT* pxDestination )
{
VECT	xVectToDest;
VECT	xNewVectToDest;
float	fDistToDest;
//	float	fMoveSpeed = 10.5f;
float	fMoveSpeed = 1.5f;
float	fNewDistToDest;
float	fTravelAngle;
VECT	xPos;
VECT	xRot;
Entity*		pEntity = GetEntity();

	VectSub( &xVectToDest, pxDestination, pEntity->GetPos() );
	xVectToDest.z = 0.0f;
	fDistToDest = VectGetLength( &xVectToDest );
	VectNormalize( &xVectToDest );
	VectScale( &xVectToDest, &xVectToDest, fMoveSpeed * fDelta );

	VectAdd( &xPos, pEntity->GetPos(), &xVectToDest );

	VectSub( &xNewVectToDest, &xPos, pxDestination );
	xNewVectToDest.z = 0.0f;
	fNewDistToDest = VectGetLength( &xNewVectToDest );
	if ( fNewDistToDest >= fDistToDest )
	{
		pEntity->SetPos( pxDestination );
		mbIsTravelling = FALSE;
	}
	else
	{
		pEntity->SetPos( &xPos );

		fTravelAngle = atan2f( xNewVectToDest.x, xNewVectToDest.y );
		xRot = *pEntity->GetRot();
		xRot.z = (A360 - fTravelAngle) - A90;
		pEntity->SetRot( &xRot );
	}
	
	return( TRUE );
}

BOOL	SimpleTravelComponent::HasDestination( void )
{
	return( mbIsTravelling );
}

BOOL	SimpleTravelComponent::EntityAnimationUpdate( int hModel )
{
	if ( mbIsTravelling )
	{
		if ( ModelGetCurrentAnimation( hModel ) == ANIM_STANDING )
		{
			ModelSetAnimationImmediate( hModel, ANIM_WALK, 0, 0, 0 );
		}	
		ModelSetAnimationNext( hModel, ANIM_WALK, 0, 0, 0 );
	}
	else
	{
		// todo - random idles...

		ModelSetAnimationNext( hModel, ANIM_STANDING, 0, 0, 0 );
	}
	return( TRUE );
}

