#ifndef INTERFACE_TEXTURE_LIST_H
#define INTERFACE_TEXTURE_LIST_H


#ifdef __cplusplus
extern "C"				// All interfaces use a C-linkage
{
#endif



extern  int	InterfaceGetTextureInternal( const char* szFilename, int nFlags, int hArchive );

extern void		InterfaceTextureListInit( void );
extern void		InterfaceTextureListNewFrame( void );
extern void		InterfaceTextureListFree( void );

extern void*		InterfaceTextureListGetPlatformTexture( int nTextureHandle );
extern int			InterfaceTextureListGetPitch( int nTextureHandle );

#ifdef __cplusplus
}
#endif



#endif
