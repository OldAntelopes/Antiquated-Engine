#ifndef ENGINE_POST_PROCESS_DX_H
#define ENGINE_POST_PROCESS_DX_H


#ifdef __cplusplus
extern "C"
{
#endif

extern void		EnginePostProcessSetEnabled( BOOL bFlag );

extern BOOL		EnginePostProcessStartScene( ulong ulBackgroundCol );
extern void		EnginePostProcessEndScene( void );


extern BOOL		EnginePostProcessInitGraphics( void );
extern void		EnginePostProcessFreeGraphics( void );

extern void		EnginePostProcessSetValue( const char* szName, float fValue );

extern void		EnginePostProcessGrabScreen( int hDestTexture );


#ifdef __cplusplus
}
#endif

#endif