
#include "StandardDef.h"
#include "Engine.h"

//#include "../GameCamera.h"

#include "Entity.h"
#include "EntityGraphics.h"
#include "EntityManager.h"

uint32		msulNextEntityUID = 0x100;
int			msnNextEntityTypeID = 5001;

RegisteredEntityList*	mspRegisteredEntityList = NULL;

void	RegisteredEntityList::Shutdown( void )
{
RegisteredEntityList*		pRegisteredEntities = mspRegisteredEntityList;
RegisteredEntityList*		pNext;

	while( pRegisteredEntities )
	{
		pNext = pRegisteredEntities->mpNext;
		SAFE_FREE( pRegisteredEntities->mszEntityName );
		delete pRegisteredEntities;

		pRegisteredEntities = pNext;
	}
	mspRegisteredEntityList = NULL;
}

BOOL	RegisteredEntityList::Register( const char* szEntityName, EntityNewFunction fnNewEntity )
{
RegisteredEntityList*		pNewEntityRegistration = new RegisteredEntityList;

	pNewEntityRegistration->mszEntityName = (char*)( malloc( strlen( szEntityName ) + 1 ) );
	strcpy( pNewEntityRegistration->mszEntityName, szEntityName );
	pNewEntityRegistration->mfnEntityNew = fnNewEntity;

	pNewEntityRegistration->mpNext = mspRegisteredEntityList;
	mspRegisteredEntityList = pNewEntityRegistration;

	pNewEntityRegistration->mspActiveEntityList = NULL;
	pNewEntityRegistration->mnEntityTypeID = msnNextEntityTypeID;

	msnNextEntityTypeID++;
	
	return( TRUE );
}



Entity*		EntityManagerAddEntity( const char* szEntityName, const VECT* pxPos, int nInitParam )
{
Entity*		pNewEntity = NULL;
RegisteredEntityList*	pRegisteredEntityList = mspRegisteredEntityList;

	while( pRegisteredEntityList )
	{ 
		if ( stricmp( pRegisteredEntityList->mszEntityName, szEntityName ) == 0 )
		{
			pNewEntity = pRegisteredEntityList->mfnEntityNew();

			pNewEntity->Init( msulNextEntityUID, pRegisteredEntityList->mnEntityTypeID, pxPos, nInitParam );
			msulNextEntityUID++;

			pNewEntity->SetNext( pRegisteredEntityList->mspActiveEntityList );
			pRegisteredEntityList->mspActiveEntityList = pNewEntity;

			return( pNewEntity );
		}
		pRegisteredEntityList = pRegisteredEntityList->mpNext;
	}
	return( NULL );
}


Entity*		EntityManagerGetFirstEntityOfType( const char* szEntityTypeName )
{
RegisteredEntityList*	pRegisteredEntityList = mspRegisteredEntityList;

	while( pRegisteredEntityList )
	{
		if ( stricmp( pRegisteredEntityList->mszEntityName, szEntityTypeName ) == 0 )
		{
			return( pRegisteredEntityList->mspActiveEntityList );
		}
		pRegisteredEntityList = pRegisteredEntityList->mpNext;
	}
	return( NULL );
}

void	EntityManagerDeleteEntity( Entity* pEntity )
{
	pEntity->SetTypeID( IN_MORGUE );
}

void	EntityManagerDeleteAllEntities( void )
{
RegisteredEntityList*	pRegisteredEntityList = mspRegisteredEntityList;
Entity*		pEntities;
Entity*		pNext;

	while( pRegisteredEntityList )
	{
		pEntities = pRegisteredEntityList->mspActiveEntityList;
		while( pEntities )
		{
			pNext = pEntities->GetNext();
			delete pEntities;
			pEntities = pNext;
		}
		pRegisteredEntityList->mspActiveEntityList = NULL;

		pRegisteredEntityList = pRegisteredEntityList->mpNext;
	}

}



void		EntityManagerInit( void )
{

}

void		EntityManagerGlobalEvent( int nEventID )
{
RegisteredEntityList*	pRegisteredEntityList = mspRegisteredEntityList;

	while( pRegisteredEntityList )
	{
	Entity*		pEntity = pRegisteredEntityList->mspActiveEntityList;

		while( pEntity )
		{
			pEntity->Event( nEventID );
			pEntity = pEntity->GetNext();
		}

		pRegisteredEntityList = pRegisteredEntityList->mpNext;
	}

}

void		EntityManagerUpdate( float delta )
{
RegisteredEntityList*	pRegisteredEntityList = mspRegisteredEntityList;

	while( pRegisteredEntityList )
	{
	Entity*		pEntity = pRegisteredEntityList->mspActiveEntityList;
	Entity*		pLast = NULL;	
	Entity*		pNext;

		while( pEntity )
		{
			pNext = pEntity->GetNext();

			if ( pEntity->GetTypeID() == IN_MORGUE )
			{
				if ( pLast )
				{
					pLast->SetNext( pNext );
				}
				else
				{
					pRegisteredEntityList->mspActiveEntityList = pNext;
				}
				delete pEntity;
			}
			else
			{
				pEntity->Update( delta );
				pLast = pEntity;
			}

			pEntity = pNext;
		}

		pRegisteredEntityList = pRegisteredEntityList->mpNext;
	}
}

void		EntityManagerRender( void )
{
RegisteredEntityList*	pRegisteredEntityList = mspRegisteredEntityList;

	while( pRegisteredEntityList )
	{
	Entity*		pEntity = pRegisteredEntityList->mspActiveEntityList;

		while( pEntity )
		{
			pEntity->Render();

			pEntity = pEntity->GetNext();
		}

		pRegisteredEntityList = pRegisteredEntityList->mpNext;
	}
}

void		EntityManagerShutdown( void )
{
	EntityManagerDeleteAllEntities();
	EntityGraphicsDeleteAll();

	RegisteredEntityList::Shutdown();
	RegisteredComponentList::Shutdown();

}


Entity*		EntityManagerGetEntity( uint32 ulEntityUID )
{
RegisteredEntityList*	pRegisteredEntityList = mspRegisteredEntityList;

	while( pRegisteredEntityList )
	{
	Entity*		pEntity = pRegisteredEntityList->mspActiveEntityList;

		while( pEntity )
		{
			if ( pEntity->GetEntityUID() == ulEntityUID )
			{
				return( pEntity );
			}
			pEntity = pEntity->GetNext();
		}
		pRegisteredEntityList = pRegisteredEntityList->mpNext;
	}
	return( NULL );
}
