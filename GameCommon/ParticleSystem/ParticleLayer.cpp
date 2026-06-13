
#include "StandardDef.h"
#include "Engine.h"
#include <algorithm>

#include "../RenderUtil/MultiVertexBuffers.h"
#include "../RenderUtil/Sprites3D.h"
#include "Particle.h"
#include "ParticleGraphics.h"
#include "ParticleManager.h"
#include "ParticleLayer.h"

#define NUM_PARTICLELAYER_VERTEX_BUFFERS		2
#define PARTICLE_LAYER_VERTEX_BUFFER_SIZE		16384

//#define USE_SPRITES3D_FOR_PARTICLE_RENDERING

#ifndef USE_SPRITES3D_FOR_PARTICLE_RENDERING
MultiVertexBuffers		mxParticleLayerSpriteBuffers;
#endif

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

#ifdef USE_SPRITES3D_FOR_PARTICLE_RENDERING
	for (Particle* pParticle : mpParticleList)
	{
		pParticle->Render();
		count++;
	}
	ParticleManagerAddRenderedParticleCount(count);
#else
	for (Particle* pParticle : mpParticleList)
	{
		pParticle->PreRender();
		count++;
	}
#endif

	return( count );
}

void	ParticleLayer::RenderAndFlush( int layerNum )
{
	for (Particle* pParticle : mpParticleList)
	{
		pParticle->RenderParticle( NULL, 0 );
	}
	Sprites3DFlushLayer(layerNum, FALSE );

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


int		ParticleLayer::UpdateTextureHandle()
{
	if ( mpParticleList.size() > 0 )
	{
	int		nParticleGraphic = mpParticleList[0]->GetParticleGraphicNum();

		SetTextureHandle( ParticleGraphicsGetTextureHandle( nParticleGraphic ) );
		SetRenderFlags( ParticleGraphicsGetRenderFlags( nParticleGraphic ) );
	}
	return( NOTFOUND );
}

int	ParticleLayer::OnRender( void )
{
#ifdef USE_SPRITES3D_FOR_PARTICLE_RENDERING
	// ParticleLayers are 'rendered' in PreRender as what they usually/currently
	// is populate a bunch of Sprite3D RenderObjects, which is whats
	// rendered in the main pass
	return( 0 );
#else
Sprite		xSprite;
int		count = 0;

	Sprites3DCommonRenderSetup(FALSE);
	UpdateTextureHandle();

	// DONT render if the texture hasn't loaded yet. We'd prefer nothing than big white squares
	if ( EngineTextureIsFullyLoaded( GetTextureHandle() ) == TRUE )
	{
		mxParticleLayerSpriteBuffers.Lock();

		u64 ullEventID = SysProfileStartEvent( "ParticleLayer::Render", mGroupLayerNum );		
		float		fAspectRatio = 1.0f;
		uint32		ulRenderFlags = GetRenderFlags();

		if ( ulRenderFlags & kRenderFlag_CustomAspect )
		{
			fAspectRatio = 1920.0f/1080.f; // TODO - proper
		}
		else
		{
			fAspectRatio = 1.0f;
		}

		Sprites3DCreateCamFacingOffsets( fAspectRatio );

		for (Particle* pParticle : mpParticleList)
		{	
			pParticle->RenderParticle(&mxParticleLayerSpriteBuffers, ulRenderFlags);
			count++;
		}
		ParticleManagerAddRenderedParticleCount(count);
			
		count++;

		mxParticleLayerSpriteBuffers.FlushWhenFull( 0, FALSE );
		SysProfileEndEvent( ullEventID );		
	}
	return( count );
#endif
}

void	ParticleLayer::InitialiseGraphicsDeviceResources()
{
	mxParticleLayerSpriteBuffers.Init( NUM_PARTICLELAYER_VERTEX_BUFFERS, PARTICLE_LAYER_VERTEX_BUFFER_SIZE, "ParticlesVB" );

}

void	ParticleLayer::ReleaseGraphicsDeviceResources()
{
	mxParticleLayerSpriteBuffers.Shutdown();
}
