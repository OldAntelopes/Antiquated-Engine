
#include <math.h>
#include "StandardDef.h"
#include "Engine.h"
#include "Rendering.h"

#include "Components/Component.h"
//#include "../GameCamera.h"
#include "EntityGraphics.h"
#include "EntityManager.h"
#include "Entity.h"

Entity::Entity()
{
	mnEntityGraphicsNum = NOTFOUND;
	memset( &mxPos, 0, sizeof( mxPos ) );
	memset( &mxRot, 0, sizeof( mxRot ) );
	memset( &mxVel, 0, sizeof( mxVel ) );
	mpComponentList = NULL;
	mbUsingQuaternion = FALSE;
	mxOrientation.x = 0.0f;
	mxOrientation.y = 0.0f;
	mxOrientation.z = 0.0f;
	mxOrientation.w = 1.0f;
	mfAutoDeleteTimer = 0.0f;
}

Entity::~Entity()
{
Component*		pComponents = mpComponentList;
Component*		pNextComponent;

	while( pComponents )
	{
		pNextComponent = pComponents->GetNext();
		delete pComponents;
		pComponents = pNextComponent;
	}

}


void	Entity::SetGraphic( const char* szModelName, const char* szTextureName, int nInstanceNum )
{
	mnEntityGraphicsNum = EntityGraphicsCreate( szModelName, szTextureName, nInstanceNum );
}
	
void	Entity::Init( uint32 ulEntityUID, int type, const VECT* pxPos, int nInitParam )
{
	mulEntityUID = ulEntityUID;
	mType = type;
	if ( pxPos )
	{
		mxPos = *pxPos;
	}

	OnInit( nInitParam );
}

Component*	Entity::AddComponent( const char* szComponentType )
{
Component*	pComponent = Component::Create( szComponentType );

	SysPanicIf( pComponent == NULL, "Referenced invalid Entity Component" );

	pComponent->SetNext( mpComponentList );
	mpComponentList = pComponent;
	
	pComponent->AddComponentToEntity( this );

	return( pComponent );
}

void	Entity::AddToWorld( float fAutoDeleteTime )
{
Component*		pComponents = mpComponentList;

	if ( fAutoDeleteTime != -1.0f )
	{
		SetAutoDeleteTime( fAutoDeleteTime );
	}
	OnAddToWorld();

	while( pComponents )
	{
		pComponents->OnEntityAddToWorld();
		pComponents = pComponents->GetNext();
	}
}

void	Entity::Respawn( const VECT* pxPos )
{
Component*		pComponents = mpComponentList;
	
	mxPos = *pxPos;
	while( pComponents )
	{
		pComponents->OnEntityRespawn( pxPos );
		pComponents = pComponents->GetNext();
	}
	
}


void	Entity::SetVel( const VECT* pxVel )
{ 
Component*		pComponents = mpComponentList;

	mxVel = *pxVel; 
	while( pComponents )
	{
		pComponents->OnEntitySetVel( pxVel );
		pComponents = pComponents->GetNext();
	}
}

void	Entity::Update( float fDelta )
{
Component*		pComponents = mpComponentList;

	// Update all components
	while( pComponents )
	{
		pComponents->UpdateComponent( fDelta );
		pComponents = pComponents->GetNext();
	}

	OnUpdate( fDelta );

	if ( mfAutoDeleteTimer > 0.0f )
	{
		mfAutoDeleteTimer -= fDelta;
		if ( mfAutoDeleteTimer < 0.0f )
		{
			EntityManagerDeleteEntity( this );
		}
	}
}

Component*	Entity::GetNamedComponent( const char* szComponentType )
{
Component*		pComponents = mpComponentList;

	// Update all components
	while( pComponents )
	{
		if ( stricmp( pComponents->GetComponentType(), szComponentType ) == 0 )
		{
			return( pComponents );
		}
		pComponents = pComponents->GetNext();
	}
	return( NULL );
}


Component*	Entity::GetComponent( eCompronentBehaviourTypes behaviourType )
{
Component*		pComponents = mpComponentList;

	while( pComponents )
	{
		if ( pComponents->GetBehaviourType() == behaviourType )
		{
			return( pComponents );
		}
		pComponents = pComponents->GetNext();
	}
	return( NULL );
}


BOOL	Entity::HasDestination( void )
{
Component*		pComponents = mpComponentList;
MovementComponent*		pMovementComponent;

	while( pComponents )
	{
		if ( pComponents->GetBehaviourType() == MOVEMENT_BEHAVIOUR_COMPONENT )
		{
			pMovementComponent = (MovementComponent*)( pComponents );
			return( pMovementComponent->HasDestination() );
		}
		pComponents = pComponents->GetNext();
	}
	return( FALSE );
}


BOOL	Entity::ArrivedAtImmediateDestination( void )
{
Component*		pComponents = mpComponentList;

	while( pComponents )
	{
		if ( pComponents->OnEntityArrivedAtImmediateDestination() == TRUE )
		{
			return( TRUE );
		}
		pComponents = pComponents->GetNext();
	}

	return( FALSE );
}

BOOL	Entity::SetImmediateDestination( VECT* pxDestination )
{
Component*		pComponents = mpComponentList;

	while( pComponents )
	{
		if ( pComponents->OnEntitySetImmediateDestination( pxDestination ) == TRUE )
		{
			return( TRUE );
		}
		pComponents = pComponents->GetNext();
	}

	return( FALSE );
}

void	Entity::Event( int nEventID )
{
Component*		pComponents = mpComponentList;

	while( pComponents )
	{
		pComponents->OnEvent( nEventID );
		pComponents = pComponents->GetNext();
	}
}

BOOL	Entity::SetDestination( VECT* pxDestination )
{
Component*		pComponents = mpComponentList;

	while( pComponents )
	{
		if ( pComponents->OnEntitySetDestination( pxDestination ) == TRUE )
		{
			return( TRUE );
		}
		pComponents = pComponents->GetNext();
	}

	// If no component processes the 'OnEntitySetDestination' then we fall back to 
	// setting the ImmediateDestination..
	SetImmediateDestination( pxDestination );
	return( FALSE );
}

void	Entity::DefaultRender( void )
{
	if ( mnEntityGraphicsNum != NOTFOUND )
	{
	int		hModelNum = EntityGraphicsGetModelHandle( mnEntityGraphicsNum );
	int		hTextureNum = EntityGraphicsGetTextureHandle( mnEntityGraphicsNum );
	float	fRadius;
	Component*		pComponents = mpComponentList;
	BOOL	bHandled = FALSE;

		while( pComponents )
		{
			bHandled = pComponents->EntityAnimationUpdate( hModelNum );
			if ( bHandled )
			{
				break;
			}
			pComponents = pComponents->GetNext();
		}
		
		if ( !bHandled )
		{
			OnEntityAnimationUpdate( hModelNum );
		}

		// TODO - If the entity is more or less on the ground (and not too big), we
		//     should do a tile index check here first for quickness

		fRadius = ModelGetStats( hModelNum )->fBoundSphereRadius;
		if ( EngineIsBoundingSphereInView( &mxPos, fRadius * 2.0f ) != OUTSIDE )
		{
			EngineSetTexture( 0, hTextureNum );

			if ( mbUsingQuaternion == TRUE )
			{
				ModelRenderQuat( hModelNum, &mxPos, &mxOrientation, 0 );
			}
			else
			{
				ModelRender( hModelNum, &mxPos, &mxRot, 0 );
			}
		}
	}
}

void	Entity::Render( void )
{
Component*		pComponents = mpComponentList;

	// Update all components
	while( pComponents )
	{
		pComponents->RenderComponent();
		pComponents = pComponents->GetNext();
	}

	OnRender();

	// only call this if RenderComponent/OnRender doesn't claim all rendering duties..
	if ( UseDefaultRender() )
	{
		DefaultRender();
	}

}

void	Entity::GetForward( VECT* pxOut )
{
ENGINEMATRIX	xMatrix;

	if ( mbUsingQuaternion == TRUE )
	{
		EngineMatrixFromQuaternion( &xMatrix, &mxOrientation );
	}
	else
	{
	const VECT*		pxRot = GetRot();

		EngineSetMatrixXYZ( &xMatrix, pxRot->x, pxRot->y, pxRot->z );
	}
	pxOut->x = xMatrix._21;
	pxOut->y = xMatrix._22;
	pxOut->z = xMatrix._23;
}

void	Entity::GetUp( VECT* pxOut )
{
ENGINEMATRIX	xMatrix;
	
	if ( mbUsingQuaternion == TRUE )
	{
		EngineMatrixFromQuaternion( &xMatrix, &mxOrientation );
	}
	else
	{
	const VECT*		pxRot = GetRot();

		EngineSetMatrixXYZ( &xMatrix, pxRot->x, pxRot->y, pxRot->z );
	}

	pxOut->x = xMatrix._31;
	pxOut->y = xMatrix._32;
	pxOut->z = xMatrix._33;
}

void	Entity::GetRight( VECT* pxOut )
{
ENGINEMATRIX	xMatrix;
	
	if ( mbUsingQuaternion == TRUE )
	{
		EngineMatrixFromQuaternion( &xMatrix, &mxOrientation );
	}
	else
	{
	const VECT*		pxRot = GetRot();

		EngineSetMatrixXYZ( &xMatrix, pxRot->x, pxRot->y, pxRot->z );
	}

	// Reverse coz the matrix gives us left
	pxOut->x = 0.0f - xMatrix._11;
	pxOut->y = 0.0f - xMatrix._12;
	pxOut->z = 0.0f - xMatrix._13;
}




