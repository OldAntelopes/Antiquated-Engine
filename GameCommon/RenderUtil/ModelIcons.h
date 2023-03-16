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


// Create a modelIcon either from filenames or handles
extern ModelIconHandle		ModelIconCreate( const char* szModel, const char* szTexture, eModelIconFlags flags, int nLoadFromArchive, const char* szDebugName );
extern ModelIconHandle		ModelIconCreateFromHandles( int hModel, int hTexture, eModelIconFlags flags, int nLoadFromArchive, const char* szDebugName );

// Change the camera distance of the icon view
extern void		ModelIconSetViewDistModifier( ModelIconHandle handle, float fDist );	

// You can either use the model icon by getting the texture and doing things with it
extern int		ModelIconGetIconTexture( ModelIconHandle handle );		// Note that what is returned is an EngineTexture not an interface one

// Or just draw it.. (Creates an interface overlay, renders a textured rect )
extern void		ModelIconDraw( ModelIconHandle handle, int layer, int X, int Y, int W, int H, float fAlpha );

// Stop it spinning
extern void		ModelIconPauseUpdates( ModelIconHandle handle, BOOL bFlag );

// Clean up
extern void		ModelIconRelease( ModelIconHandle handle );


//----------------------------------------------------------
// System stuff

extern void		ModelIconsInit( void );
extern void		ModelIconsUpdate( void );

extern void		ModelIconsFreeAll( void );

extern void		ModelIconsShutdown( void );


#ifdef __cplusplus
}
#endif


#endif