
#include "StandardDef.h"

#include "SimpleTravelComponent.h"
#include "GroundMotionComponent.h"
#include "VehicleController.h"


RegisteredComponentList*	mspRegisteredComponentList = NULL;

void	RegisteredComponentList::Shutdown( void )
{
RegisteredComponentList*		pRegisteredComponents = mspRegisteredComponentList;
RegisteredComponentList*		pNext;

	while( pRegisteredComponents )
	{
		pNext = pRegisteredComponents->mpNext;
		SAFE_FREE( pRegisteredComponents->mszComponentName );
		delete pRegisteredComponents;

		pRegisteredComponents = pNext;
	}
	mspRegisteredComponentList = NULL;
}

BOOL	RegisteredComponentList::Register( const char* szComponentName, ComponentNewFunction fnNewComponent )
{
RegisteredComponentList*		pNewComponentRegistration = new RegisteredComponentList;

	pNewComponentRegistration->mszComponentName = (char*)( malloc( strlen( szComponentName ) + 1 ) );
	strcpy( pNewComponentRegistration->mszComponentName, szComponentName );
	pNewComponentRegistration->mfnComponentNew = fnNewComponent;

	pNewComponentRegistration->mpNext = mspRegisteredComponentList;
	mspRegisteredComponentList = pNewComponentRegistration;

	return( TRUE );
}


Component*	 Component::Create( const char* szComponentName )
{
Component*		pNewComponent = NULL;
RegisteredComponentList*	pComponentList = mspRegisteredComponentList;

	while( pComponentList )
	{
		if ( stricmp( pComponentList->mszComponentName, szComponentName ) == 0 )
		{
			pNewComponent = pComponentList->mfnComponentNew();
			return( pNewComponent );
		}
		pComponentList = pComponentList->mpNext;
	}
	return( NULL );
}
 
void	Component::AddComponentToEntity( Entity* pEntity )
{
	mpParentEntity = pEntity;

	OnEntityInitialised();
}

void	Component::UpdateComponent( float fDelta )
{

	OnUpdateComponent( fDelta );
}

void	Component::RenderComponent( void )
{

	OnRenderComponent();

}


void	Component::ReleaseComponent( void )
{


}
		