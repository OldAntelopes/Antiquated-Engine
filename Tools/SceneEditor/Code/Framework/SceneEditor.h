#ifndef TRACTOR_ENGINE_DEMO_H
#define	TRACTOR_ENGINE_DEMO_H


extern BOOL	DemoFrameworkQuit( void );

extern HWND		ghwndMain;
extern void	DemoFrameworkDebugPrint( const char* szText );

extern void	SceneEditorNewFrame( void );

extern void	SceneEditorGetFocusPoint( VECT*	pvecPosition );

extern void	SceneEditorSetGlobalMoveSpeed( int nCamspeed );


#endif //#ifndef TRACTOR_ENGINE_DEMO_H