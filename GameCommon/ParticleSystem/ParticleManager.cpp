
#include "StandardDef.h"
#include <vector>
#include "Engine.h"

//#include "../GameCamera.h"

#include "Particle.h"
#include "ParticleGraphics.h"
#include "ParticleLayer.h"
#include "ParticleManager.h"

int			msnNextParticleTypeID = 5001;

RegisteredParticleList*	mspRegisteredParticleList = NULL;

std::map<int, bool>		msActiveParticleLayers;

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
	msActiveParticleLayers.clear();
}

BOOL	RegisteredParticleList::Register( const char* szParticleName, ParticleNewFunction fnNewParticle )
{
RegisteredParticleList*		pNewParticleRegistration = new RegisteredParticleList;

	pNewParticleRegistration->mszParticleName = (char*)( malloc( strlen( szParticleName ) + 1 ) );
	strcpy( pNewParticleRegistration->mszParticleName, szParticleName );
	pNewParticleRegistration->mfnParticleNew = fnNewParticle;

	pNewParticleRegistration->mpNext = mspRegisteredParticleList;
	mspRegisteredParticleList = pNewParticleRegistration;

	pNewParticleRegistration->mParticleLayerMap.clear();
	pNewParticleRegistration->mnParticleTypeID = msnNextParticleTypeID;

	msnNextParticleTypeID++;
	
	return( TRUE );
}

void	ParticleManagerAddParticleToLayer( RegisteredParticleList*	pRegisteredParticleList, Particle* pParticle, int layer )
{
	if ( msActiveParticleLayers[layer] == false )
	{
		msActiveParticleLayers[layer] = true;
	}

	pParticle->SetNext( pRegisteredParticleList->mParticleLayerMap[layer] );
	pRegisteredParticleList->mParticleLayerMap[layer] = pParticle;
}

Particle*		ParticleManagerAddParticleToGroup( ManagedParticleGroup* pParticleGroup, const char* szParticleName, const VECT* pxPos, const VECT* pxVel, uint32 ulCol, float fLongevity, int nInitParam, uint32 ulInitParamChannel, void* pUserObject )
{
Particle*		pNewParticle = NULL;
RegisteredParticleList*	pRegisteredParticleList = mspRegisteredParticleList;
int layer = pParticleGroup->GetGroupLayerID();

	while( pRegisteredParticleList )
	{
		// TODO - Replace with hash lookup
		if ( stricmp( pRegisteredParticleList->mszParticleName, szParticleName ) == 0 )
		{
			pNewParticle = pRegisteredParticleList->mfnParticleNew();
			pNewParticle->Init( pRegisteredParticleList->mnParticleTypeID, pxPos, pxVel, ulCol, fLongevity, nInitParam, ulInitParamChannel, pUserObject, layer );
			
			if ( msActiveParticleLayers[layer] == false )
			{
				msActiveParticleLayers[layer] = true;
			}
			pParticleGroup->AddParticle(pNewParticle);
			return( pNewParticle );
		}
		pRegisteredParticleList = pRegisteredParticleList->mpNext;
	}
	return( NULL );
}

Particle*	ParticleManagerCreateNewParticle( const char* szParticleName, const VECT* pxPos, const VECT* pxVel, uint32 ulCol, float fLongevity, int nInitParam, uint32 ulInitParamChannel, void* pUserObject )
{
Particle*		pNewParticle = NULL;
RegisteredParticleList*	pRegisteredParticleList = mspRegisteredParticleList;
int		nSpriteRenderLayer = ulInitParamChannel;

	while( pRegisteredParticleList )
	{
		// TODO - Replace with hash lookup
		if ( stricmp( pRegisteredParticleList->mszParticleName, szParticleName ) == 0 )
		{
			pNewParticle = pRegisteredParticleList->mfnParticleNew();
			pNewParticle->Init( pRegisteredParticleList->mnParticleTypeID, pxPos, pxVel, ulCol, fLongevity, nInitParam, ulInitParamChannel, pUserObject, nSpriteRenderLayer);
			return( pNewParticle );
		}
		pRegisteredParticleList = pRegisteredParticleList->mpNext;
	}
	return( NULL );
}

Particle*		ParticleManagerAddParticle( const char* szParticleName, const VECT* pxPos, const VECT* pxVel, uint32 ulCol, float fLongevity, int nInitParam, uint32 ulInitParamChannel, void* pUserObject )
{
Particle*		pNewParticle = NULL;
RegisteredParticleList*	pRegisteredParticleList = mspRegisteredParticleList;
int		nSpriteRenderLayer = ulInitParamChannel;

	while( pRegisteredParticleList )
	{
		// TODO - Replace with hash lookup
		if ( stricmp( pRegisteredParticleList->mszParticleName, szParticleName ) == 0 )
		{
			pNewParticle = pRegisteredParticleList->mfnParticleNew();
			pNewParticle->Init( pRegisteredParticleList->mnParticleTypeID, pxPos, pxVel, ulCol, fLongevity, nInitParam, ulInitParamChannel, pUserObject, nSpriteRenderLayer);

			ParticleManagerAddParticleToLayer(pRegisteredParticleList, pNewParticle, ulInitParamChannel);
			return( pNewParticle );
		}
		pRegisteredParticleList = pRegisteredParticleList->mpNext;
	}
	return( NULL );
}

// Needed???
Particle*		ParticleManagerGetFirstParticleOfType( const char* szParticleTypeName, int layer )
{
RegisteredParticleList*	pRegisteredParticleList = mspRegisteredParticleList;

	while( pRegisteredParticleList )
	{
		if ( stricmp( pRegisteredParticleList->mszParticleName, szParticleTypeName ) == 0 )
		{
			return( pRegisteredParticleList->mParticleLayerMap[layer] );
		}
		pRegisteredParticleList = pRegisteredParticleList->mpNext;
	}
	return( NULL );
}

void	ParticleManagerDeleteParticle( Particle* pParticle )
{
	pParticle->SetTypeID( IN_MORGUE );
}


//---------------------------------------------------------
void	ManagedParticleGroup::Update( float delta )
{
	for (Particle* pParticle : mpParticleList)
	{
		if ( pParticle->GetTypeID() != IN_MORGUE )
		{
			pParticle->Update( delta );
		}
	}

	auto it = mpParticleList.begin();
	
	while(it != mpParticleList.end())
	{
		Particle* pParticle = *it;

		if( pParticle->GetTypeID() == IN_MORGUE) 
		{
	        it = mpParticleList.erase(it);
			delete pParticle;
	    }
	    else ++it;
	}
}

void	ManagedParticleGroup::Render()
{
	for (Particle* pParticle : mpParticleList)
	{
		pParticle->RenderParticle( NULL, 0 );		
	}

	Sprites3DFlushLayer(mGroupLayerNum, FALSE );

}


//---------------------------------------

void		ParticleManagerInitialiseGraphicsDeviceResources( void )
{
	ParticleLayer::InitialiseGraphicsDeviceResources();
}

void		ParticleManagerReleaseGraphicsDeviceResources( void )
{
	ParticleLayer::ReleaseGraphicsDeviceResources();

}


void		ParticleManagerInit( void )
{
	ParticleManagerInitialiseGraphicsDeviceResources();
}

int		msnParticleGroupLayerNum = 1000;

ManagedParticleGroup*	ParticleManagerCreateParticleGroup( void )
{
	return(new ManagedParticleGroup(msnParticleGroupLayerNum++));
}

void				ParticleManagerDeleteParticleGroup( ManagedParticleGroup* pParticleGroup )
{
	delete pParticleGroup;
}


void	ParticleManagerDeleteAllActiveParticles( void )
{
RegisteredParticleList*	pRegisteredParticleList = mspRegisteredParticleList;

	while( pRegisteredParticleList )
	{
		for (auto& layerPair : pRegisteredParticleList->mParticleLayerMap)
		{
		Particle*		pParticle = layerPair.second;
		Particle*		pNext;

			while( pParticle )
			{	
				pNext = pParticle->GetNext();
				delete pParticle;
				pParticle = pNext;
			}
			layerPair.second = NULL;
		}

		pRegisteredParticleList = pRegisteredParticleList->mpNext;
	}
}

void		ParticleManagerUpdate( float delta )
{
RegisteredParticleList*	pRegisteredParticleList = mspRegisteredParticleList;

	while( pRegisteredParticleList )
	{
		for (auto& layerPair : pRegisteredParticleList->mParticleLayerMap)
		{
		Particle*		pParticle = layerPair.second;
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
						layerPair.second = pNext;
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
		}

		pRegisteredParticleList = pRegisteredParticleList->mpNext;
	}
}

int		msnNumRenderedParticles = 0;
int		msnNumRenderedParticleGroups = 0;

void		ParticleManagerNewFrame()
{
	msnNumRenderedParticles = 0;
	msnNumRenderedParticleGroups = 0;
}

void		ParticleManagerAddRenderedParticleCount( int count )
{
	msnNumRenderedParticleGroups ++;
	msnNumRenderedParticles += count;
}

int		ParticleManagerGetRenderedParticleCount()
{
	return( msnNumRenderedParticles );
}

void		ParticleManagerRender( void )
{

	for ( auto& activeLayerPair : msActiveParticleLayers)
	{
	RegisteredParticleList*	pRegisteredParticleList = mspRegisteredParticleList;

		while( pRegisteredParticleList )
		{
		Particle*		pParticle = pRegisteredParticleList->mParticleLayerMap[activeLayerPair.first];
		Particle*		pLast = NULL;	
//		Particle*		pNext;

			while( pParticle )
			{	
				pParticle->RenderParticle( NULL, 0 );
				msnNumRenderedParticles++;
				pParticle = pParticle->GetNext();
			}
			pRegisteredParticleList = pRegisteredParticleList->mpNext;
			msnNumRenderedParticleGroups++;
		}
	}
}

void		ParticleManagerShutdown( void )
{
	ParticleManagerDeleteAllActiveParticles();
	ParticleGraphicsShutdown();

	RegisteredParticleList::Shutdown();

}

