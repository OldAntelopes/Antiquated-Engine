
#include "StandardDef.h"
#include "Engine.h"

//#include "../GameCamera.h"

#include "Particle.h"
#include "ParticleManager.h"

int			msnNextParticleTypeID = 5001;

RegisteredParticleList*	mspRegisteredParticleList = NULL;

void	RegisteredParticleList::Shutdown( void )
{
RegisteredParticleList*		pRegisteredParticles = mspRegisteredParticleList;
RegisteredParticleList*		pNext;

	while( pRegisteredParticles )
	{
		pNext = pRegisteredParticles->mpNext;
		SAFE_FREE( pRegisteredParticles->mszParticleName );
		delete pRegisteredParticles;

		pRegisteredParticles = pNext;
	}
}

BOOL	RegisteredParticleList::Register( const char* szParticleName, ParticleNewFunction fnNewParticle )
{
RegisteredParticleList*		pNewParticleRegistration = new RegisteredParticleList;

	pNewParticleRegistration->mszParticleName = (char*)( malloc( strlen( szParticleName ) + 1 ) );
	strcpy( pNewParticleRegistration->mszParticleName, szParticleName );
	pNewParticleRegistration->mfnParticleNew = fnNewParticle;

	pNewParticleRegistration->mpNext = mspRegisteredParticleList;
	mspRegisteredParticleList = pNewParticleRegistration;

	pNewParticleRegistration->mspActiveParticleList = NULL;
	pNewParticleRegistration->mnParticleTypeID = msnNextParticleTypeID;

	msnNextParticleTypeID++;
	
	return( TRUE );
}



Particle*		ParticleManagerAddParticle( const char* szParticleName, const VECT* pxPos, const VECT* pxVel, uint32 ulCol, float fLongevity, int nInitParam )
{
Particle*		pNewParticle = NULL;
RegisteredParticleList*	pRegisteredParticleList = mspRegisteredParticleList;

	while( pRegisteredParticleList )
	{
		if ( stricmp( pRegisteredParticleList->mszParticleName, szParticleName ) == 0 )
		{
			pNewParticle = pRegisteredParticleList->mfnParticleNew();

			pNewParticle->Init( pRegisteredParticleList->mnParticleTypeID, pxPos, pxVel, ulCol, fLongevity, nInitParam );

			pNewParticle->SetNext( pRegisteredParticleList->mspActiveParticleList );
			pRegisteredParticleList->mspActiveParticleList = pNewParticle;

			return( pNewParticle );
		}
		pRegisteredParticleList = pRegisteredParticleList->mpNext;
	}
	return( NULL );
}


Particle*		ParticleManagerGetFirstParticleOfType( const char* szParticleTypeName )
{
RegisteredParticleList*	pRegisteredParticleList = mspRegisteredParticleList;

	while( pRegisteredParticleList )
	{
		if ( stricmp( pRegisteredParticleList->mszParticleName, szParticleTypeName ) == 0 )
		{
			return( pRegisteredParticleList->mspActiveParticleList );
		}
		pRegisteredParticleList = pRegisteredParticleList->mpNext;
	}
	return( NULL );
}

void	ParticleManagerDeleteParticle( Particle* pParticle )
{
	pParticle->SetTypeID( IN_MORGUE );
}



void		ParticleManagerInit( void )
{

}

void	ParticleManagerDeleteAlllActiveParticles( void )
{
RegisteredParticleList*	pRegisteredParticleList = mspRegisteredParticleList;

	while( pRegisteredParticleList )
	{
	Particle*		pParticle = pRegisteredParticleList->mspActiveParticleList;
	Particle*		pNext;

		while( pParticle )
		{
			pNext = pParticle->GetNext();
			delete pParticle;
			pParticle = pNext;
		}
		pRegisteredParticleList->mspActiveParticleList = NULL;
		pRegisteredParticleList = pRegisteredParticleList->mpNext;
	}
}

void		ParticleManagerUpdate( float delta )
{
RegisteredParticleList*	pRegisteredParticleList = mspRegisteredParticleList;

	while( pRegisteredParticleList )
	{
	Particle*		pParticle = pRegisteredParticleList->mspActiveParticleList;
	Particle*		pLast = NULL;	
	Particle*		pNext;

		while( pParticle )
		{
			pNext = pParticle->GetNext();

			if ( pParticle->GetTypeID() == IN_MORGUE )
			{
				if ( pLast )
				{
					pLast->SetNext( pNext );
				}
				else
				{
					pRegisteredParticleList->mspActiveParticleList = pNext;
				}
				delete pParticle;
			}
			else
			{
				pParticle->Update( delta );
				pLast = pParticle;
			}

			pParticle = pNext;
		}

		pRegisteredParticleList = pRegisteredParticleList->mpNext;
	}
}

void		ParticleManagerRender( void )
{
RegisteredParticleList*	pRegisteredParticleList = mspRegisteredParticleList;

	while( pRegisteredParticleList )
	{
	Particle*		pParticle = pRegisteredParticleList->mspActiveParticleList;

		while( pParticle )
		{
			pParticle->Render();

			pParticle = pParticle->GetNext();
		}

		pRegisteredParticleList = pRegisteredParticleList->mpNext;
	}
}

void		ParticleManagerShutdown( void )
{
	ParticleManagerDeleteAlllActiveParticles();

	RegisteredParticleList::Shutdown();

}

