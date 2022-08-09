
#ifndef TEXTURE_MANAGER_H
#define	TEXTURE_MANAGER_H

#ifdef __cplusplus
extern "C"				// All interfaces use a C-linkage
{
#endif
	
typedef struct
{
	int		nHandleCounter;
	int		nNumTexturesLoaded;
	int		nNumTextureLoadsFailed;

} TEXTURE_MANAGER_STATS;

											
extern	void	EngineTextureManagerInitDX( void );
extern	void	EngineTextureManagerUpdateDX( void );

extern	void	EngineTextureManagerFreeDX( BOOL );

extern void		EngineTextureManagerGetStats( TEXTURE_MANAGER_STATS* pxOut );

typedef	void(*TextureManagerPrintCallback)( const char* );

extern void		EngineTextureManagerDump( TextureManagerPrintCallback fnPrintCallback );

extern BOOL	msbEngineNoTexturesOverride;

#ifdef __cplusplus
}
#endif

#endif
