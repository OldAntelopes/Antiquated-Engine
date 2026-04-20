
#ifndef TEXTURE_MANAGER_H
#define	TEXTURE_MANAGER_H

typedef	void(*TextureManagerPrintCallback)( const char* );

typedef struct
{
	int		nHandleCounter;
	int		nNumTexturesLoaded;
	int		nNumTextureLoadsFailed;

} TEXTURE_MANAGER_STATS;


#ifdef __cplusplus

#include "../EngineModule.h"

class EngineTextureManagerDX  : public EngineModule
{
public:
	EngineTextureManagerDX();
	~EngineTextureManagerDX();

	void	Init(void);
	void	Update(void);
	void	Free(BOOL bFreeAll);

	void	GetStats(TEXTURE_MANAGER_STATS* pxOut);
	void	Dump(TextureManagerPrintCallback fnPrintCallback);

};


#endif

#ifdef __cplusplus
extern "C"				// All interfaces use a C-linkage
{
#endif
	

							
extern	void	EngineTextureManagerInitDX( void );
extern	void	EngineTextureManagerUpdateDX( void );

extern	void	EngineTextureManagerFreeDX( BOOL );

extern void		EngineTextureManagerGetStats( TEXTURE_MANAGER_STATS* pxOut );


extern void		EngineTextureManagerDump( TextureManagerPrintCallback fnPrintCallback );

extern BOOL	msbEngineNoTexturesOverride;

#ifdef __cplusplus
}
#endif

#endif
