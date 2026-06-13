#ifndef PARTICLE_GRAPHICS_H
#define	PARTICLE_GRAPHICS_H

#include "../RenderUtil/Sprites3D.h"

extern void		ParticleGraphicsInit( );

extern int		ParticleGraphicsCreate( const char* szTextureNum, float fGridScale, BOOL bUseRotation = FALSE, eRenderFlags renderFlags = kRenderFlag_Default, int layer = 0 );

extern int		ParticleGraphicsCreateHandle( int hTex, float fGridScale, BOOL bUseRotation = FALSE, eRenderFlags renderFlags = kRenderFlag_Default, int layer = 0 );

extern SpriteGroup*		ParticleGraphicsGetSpriteGroup( int nParticleGraphicID );

extern int		ParticleGraphicsGetTextureHandle(int nParticleGraphicID);
extern uint32	ParticleGraphicsGetRenderFlags(int nParticleGraphicID);

extern void		ParticleGraphicsShutdown( );

#endif