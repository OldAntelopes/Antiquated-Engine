#ifndef LANDSCAPE_RENDER_H
#define LANDSCAPE_RENDER_H


extern void		LandscapeRenderInit( int nLandscapeTextureSystem );

extern void		LandscapeRenderNewFrame( void );
extern BOOL		LandscapeRenderTopdown( int nFocusLoc );
extern void		LandscapeRenderFlushTiles( void );

extern void		LandscapeRenderShutdown( void );

extern BOOL		LandscapeRenderHasInitialised( void );



#endif