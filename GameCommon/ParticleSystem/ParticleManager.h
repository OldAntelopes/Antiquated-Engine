#ifndef PARTICLE_MANAGER_H
#define PARTICLE_MANAGER_H

#include <vector>

#include "Particle.h"

class ManagedParticleGroup
{
public:
	ManagedParticleGroup(int groupLayerNum) { mGroupLayerNum = groupLayerNum; }

	void	Update( float delta );
	void	Render();
	int		GetGroupLayerID() { return mGroupLayerNum; }

	void	AddParticle(Particle* pParticle)
	{
		mpParticleList.push_back(pParticle);
	}

private:
	std::vector<Particle*>		mpParticleList;

	int		mGroupLayerNum;
};

//-------------------------------------------------

extern void		ParticleManagerInit( void );
extern void		ParticleManagerNewFrame();

extern void		ParticleManagerUpdate( float delta );
extern void		ParticleManagerRender( void );

extern void		ParticleManagerShutdown( void );

extern ManagedParticleGroup*	ParticleManagerCreateParticleGroup( void );
extern void				ParticleManagerDeleteParticleGroup( ManagedParticleGroup* );

extern Particle*	ParticleManagerAddParticle( const char* szParticleTypeName, const VECT* pxPos, const VECT* pxVel, uint32 ulCol, float fLongevity, int nInitParam = 0, uint32 ulInitParamChannel = 0, void* pUserObject = NULL );

extern Particle*	ParticleManagerAddParticleToGroup( ManagedParticleGroup* pParticleGroup, const char* szParticleTypeName, const VECT* pxPos, const VECT* pxVel, uint32 ulCol, float fLongevity, int nInitParam = 0, uint32 ulInitParamChannel = 0, void* pUserObject = NULL );


extern Particle*	ParticleManagerCreateNewParticle( const char* szParticleTypeName, const VECT* pxPos, const VECT* pxVel, uint32 ulCol, float fLongevity, int nInitParam, uint32 ulInitParamChannel, void* pUserObject );

extern int		ParticleManagerGetRenderedParticleCount();
extern void		ParticleManagerAddRenderedParticleCount( int count );

extern void		ParticleManagerInitialiseGraphicsDeviceResources( void );
extern void		ParticleManagerReleaseGraphicsDeviceResources( void );

#endif