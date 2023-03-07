#ifndef ANTIQUATED_MODEL_ICONS_H
#define ANTIQUATED_MODEL_ICONS_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef int			ModelIconHandle;

enum eModelIconFlags
{
	MODELICONFLAG_NONE = 0,
	MODELICONFLAG_RENDER_ALPHA = 0x1,
};




extern void		ModelIconsInit( void );

extern ModelIconHandle		ModelIconCreate( const char* szModel, const char* szTexture, eModelIconFlags flags, int nLoadFromArchive );
extern ModelIconHandle		ModelIconCreateFromHandles( int hModel, int hTexture, eModelIconFlags flags, int nLoadFromArchive );

extern void		ModelIconsUpdate( void );

extern int		ModelIconGetIconTexture( ModelIconHandle );		// Note that what is returned is an EngineTexture not an interface one
extern void		ModelIconSetViewDistModifier( ModelIconHandle handle, float fDist );	

extern void		ModelIconsRelease( ModelIconHandle );

extern void		ModelIconsFreeAll( void );

extern void		ModelIconsShutdown( void );


#ifdef __cplusplus
}
#endif


#endif