#ifndef INTERFACE_INTERNALS_H
#define INTERFACE_INTERNALS_H

#include "InterfaceTypesDX.h"
#include "../Common/InterfaceModule.h"

extern LPGRAPHICSDEVICE        mpInterfaceD3DDevice; // Our rendering device

extern int		mnInterfaceDrawWidth;
extern int		mnInterfaceDrawHeight;
extern int		mnInterfaceDrawX;
extern int		mnInterfaceDrawY;
extern float	mfMipMapBias;
extern int		mnMinFilter;
extern int		mnMagFilter;
extern int		mnMipFilter;

extern BOOL		mboRenderLineAlpha;

extern void InterfaceTurnOffTextureFiltering( void );
extern void InterfaceTurnOnTextureFiltering( int );
extern void		InterfaceSetIsUsingDefaultFonts( BOOL bUsingDefaultFonts );
extern void InterfaceReleaseForDeviceReset( void );
extern void InterfaceRestorePostDeviceReset( void );

enum eInterfaceTextureFormat
{ 
	FORMAT_A8R8G8B8,
	FORMAT_A8R3G3B2,
	FORMAT_A4R4G4B4,
	FORMAT_A8,
	FORMAT_R8G8B8,
	FORMAT_DISPLAY,
};

extern HRESULT				InterfaceInternalDXCreateVertexBuffer( unsigned int Length, unsigned int Usage, unsigned int FVF, IGRAPHICSVERTEXBUFFER** );
extern void					InterfaceInternalDXCreateTexture( int width, int height, int levels, int mode, eInterfaceTextureFormat format, LPGRAPHICSTEXTURE* );
extern void					InterfaceInternalDXSetStreamSource( unsigned int StreamNumber, IGRAPHICSVERTEXBUFFER *pStreamData, unsigned int OffsetInBytes, unsigned int Stride );
extern HRESULT				InterfaceInternalDXCreateImageSurface( unsigned int width, unsigned int height, eInterfaceTextureFormat format, IGRAPHICSSURFACE** );
extern LPGRAPHICSTEXTURE	InterfaceLoadTextureFromArchiveDX( const char* szFilename, int boReduceFilter, int boMipFilter, int nArchiveHandle );
extern IGRAPHICSSURFACE*	LoadJpegDirectArchive( const char* szFilename, int nWidth, int nHeight, int nFlags, int nArchiveHandle );

//extern int					TexturedOverlayCreate( int nLayer, LPGRAPHICSTEXTURE pxTexture );

extern void					InterfaceDXSetInitialSize( BOOL boFullScreen, int nFullScreenSizeX, int nFullScreenSizeY , BOOL boSmallFlag );

//extern BOOL					InterfaceGetDXDeviceCreateParams( BOOL boMinPageSize, D3DPRESENT_PARAMETERS* pD3Dpp );

extern LPGRAPHICSTEXTURE	InterfaceLoadTextureDXFromFileInMem( const char* szFilename, byte* pbMem, int nMemSize, int boReduceFilter, int boMipFilter );

extern IGRAPHICSSURFACE*	LoadJpegDirect( const char* szFilename, int nWidth, int nHeight, int );
extern void					DrawJpegDirect( int nLayer, IGRAPHICSSURFACE* pxSurface, int nX, int nY, int nWidth, int nHeight, int nFlags );

extern void					InterfaceSetSmall( BOOL boSmallFlag );

extern void					InterfaceSetD3DDevice( LPGRAPHICSDEVICE pDevice );

#ifdef __cplusplus
}
#endif

class InterfaceInternalsDX : public InterfaceModule
{
public:
	HRESULT				CreateVertexBuffer( unsigned int Length, unsigned int Usage, unsigned int FVF, IGRAPHICSVERTEXBUFFER** );
	void				CreateTexture( int width, int height, int levels, int mode, eInterfaceTextureFormat format, LPGRAPHICSTEXTURE* );
	void				SetStreamSource( unsigned int StreamNumber, IGRAPHICSVERTEXBUFFER *pStreamData, unsigned int OffsetInBytes, unsigned int Stride );
	HRESULT				CreateImageSurface( unsigned int width, unsigned int height, eInterfaceTextureFormat format, IGRAPHICSSURFACE** );

	LPGRAPHICSTEXTURE	LoadTextureDX( const char* szFilename, int boReduceFilter, int boMipFilter );
	LPGRAPHICSTEXTURE	LoadTextureFromArchive( const char* szFilename, int boReduceFilter, int boMipFilter, int nArchiveHandle );
};

#endif
