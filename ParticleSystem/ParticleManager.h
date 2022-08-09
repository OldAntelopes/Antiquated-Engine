#ifndef PARTICLE_MANAGER_H
#define PARTICLE_MANAGER_H

#include "Particle.h"

extern void		ParticleManagerInit( void );

extern void		ParticleManagerUpdate( float delta );
extern void		ParticleManagerRender( void );

extern void		ParticleManagerShutdown( void );

extern Particle*	ParticleManagerAddParticle( const char* szParticleTypeName, const VECT* pxPos, const VECT* pxVel, ulong ulCol, float fLongevity, int nInitParam = 0 );


#endif