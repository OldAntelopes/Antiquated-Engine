#ifndef PARTICLE_GRAPHICS_H
#define	PARTICLE_GRAPHICS_H

#include "../RenderUtil/Sprites3D.h"


extern int		ParticleGraphicsCreate( const char* szTextureNum, float fGridScale, BOOL bUseRotation = FALSE, eSpriteGroupRenderFlags renderFlags = kSpriteRender_Default );

extern SPRITE_GROUP		ParticleGraphicsGetSpriteGroup( int nParticleGraphicID );



#endif