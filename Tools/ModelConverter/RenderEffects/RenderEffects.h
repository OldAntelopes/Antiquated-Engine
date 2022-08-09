#ifndef RENDER_EFFECTS_H
#define RENDER_EFFECTS_H



void		RenderEffectsInitStaticGraphics( void );
void		RenderEffectsFreeStaticGraphics( void );


void		RenderEffectsInit( void );

void		RenderEffectsUpdate( float fDelta );
void		RenderEffectsRender( void );
void		RenderEffectsAddParticle( int nParticleType, const VECT* pxPos );

void		RenderEffectsFree( void );



#endif