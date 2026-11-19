#ifndef SKELETALANIM_SAMPLE_SCENE_H
#define SKELETALANIM_SAMPLE_SCENE_H

extern void		SceneInitialise( const char* szCommandLine );
extern BOOL		SceneUpdate( uint32 ulTickIncrease );		// Returns FALSE when the app should quit
extern void		SceneRender( void );
extern void		SceneDrawOverlay( void );
extern void		SceneOnKey( int nVirtKey );
extern void		SceneFree( void );

// main.cpp
extern void		MainRenderFrame( uint32 ulUpdateTime );

#endif
