#ifndef INTERFACE_GL_TEXTUREMANAGER_H
#define INTERFACE_GL_TEXTUREMANAGER_H


extern void		InterfaceTextureManagerInit( void );
extern void		InterfaceTextureManagerUpdate( void );
extern void		InterfaceTextureManagerFree( void );

extern int	InterfaceLoadTextureFromFileInMem( const char* szFilename, unsigned char* pbMem, int nMemSize, int boReduceFilter, int boMipFilter );


// External APIs (Defined in Interface.h)

// extern INTERFACE_API int	InterfaceGetTexture( const char* szFilename, int nFlags );

#endif