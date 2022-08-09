#ifndef SUBSPACE_CONTROLLER_COMPONENT_H
#define	SUBSPACE_CONTROLLER_COMPONENT_H


#include "ControllerComponent.h"


class SubspaceControllerComponent : public ControllerComponent
{
public:
	SubspaceControllerComponent();

	virtual const char*			GetComponentType( void ) { return( "SubspaceController" ); }

	virtual void		OnUpdateComponent( float fDelta );
	virtual void		OnRenderComponent( void );

	virtual void		OnEntityInitialised( void );

private:
	void	ApplySteering( float fDelta );

	float	mfRotation;

	float	mfRotationSpeed;

//	float	mfSpeed;
	float	mfAccThisFrame;

};




#endif