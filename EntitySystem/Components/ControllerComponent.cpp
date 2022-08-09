
#include "StandardDef.h"

#include "ControllerComponent.h"


void	ControllerComponent::ResetControlStates( void )
{
int		nLoop;

	for ( nLoop = 0; nLoop < NUM_CONTROL_STATES; nLoop++ )
	{
		mafControlStates[ nLoop ]  = 0.0f;
	}

}


ControllerComponent::ControllerComponent()
{
	ResetControlStates();
}



void	ControllerComponent::SetControl( eControl controlType, float fAmount )
{
	mafControlStates[ controlType ] = fAmount;

}

float	ControllerComponent::GetControlState( eControl controlType )
{
	return( mafControlStates[ controlType ] );
}

