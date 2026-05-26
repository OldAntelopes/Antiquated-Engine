
#include "StandardDef.h"
#include <algorithm>

#include "Particle.h"
#include "ParticleManager.h"
#include "ParticleLayer.h"

ParticleLayer::~ParticleLayer()
{
	for (Particle* pParticle : mpParticleList)
	{
		delete pParticle;
	}
	mpParticleList.clear();
}


void	ParticleLayer::Update( float delta )
{
	for (Particle* pParticle : mpParticleList)
	{
		if ( pParticle->GetTypeID() != IN_MORGUE )
		{
			pParticle->Update( delta );
		}
	}

	// Single-pass removal: partition dead particles to the end then bulk-erase
	auto newEnd = std::remove_if( mpParticleList.begin(), mpParticleList.end(),
		[]( Particle* pParticle )
		{
			if ( pParticle->GetTypeID() == IN_MORGUE )
			{
				delete pParticle;
				return true;
			}
			return false;
		});
	mpParticleList.erase( newEnd, mpParticleList.end() );
}


int		ParticleLayer::OnPreRender()
{
int		count = 0;

	for (Particle* pParticle : mpParticleList)
	{
		pParticle->Render();
		count++;
	}
	ParticleManagerAddRenderedParticleCount(count);
	
	return( count );
}

void	ParticleLayer::RenderAndFlush( int layerNum )
{
	for (Particle* pParticle : mpParticleList)
	{
		pParticle->Render();
	}
	Sprites3DFlushLayer(layerNum, FALSE );

}



int		ParticleLayer::OnRender()
{
	// ParticleLayers are 'rendered' in PreRender as what they usually/currently
	// is populate a bunch of Sprite3D RenderObjects, which is whats
	// rendered in the main pass
	return( 0 );
}


Particle*		ParticleLayer::AddParticle( const char* szParticleTypeName, const VECT* pxPos, const VECT* pxVel, uint32 ulCol, float fLongevity, int nInitParam, uint32 ulInitParamChannel, void* pUserObject )
{
Particle*		pParticle = ParticleManagerCreateNewParticle( szParticleTypeName, pxPos, pxVel, ulCol, fLongevity, nInitParam, ulInitParamChannel, pUserObject );

	if ( pParticle )
	{
		mpParticleList.push_back(pParticle);
	}
	return( pParticle );
}

