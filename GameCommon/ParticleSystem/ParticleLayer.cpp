
#include "StandardDef.h"

#include "Particle.h"
#include "ParticleManager.h"
#include "ParticleLayer.h"

void	ParticleLayer::Update( float delta )
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


int		ParticleLayer::OnPreRender()
{
	for (Particle* pParticle : mpParticleList)
	{
		pParticle->Render();
	}
	return( 0 );
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

