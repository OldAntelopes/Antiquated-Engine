#ifndef INTERFACE_INTERNALS_MARMALADE_H
#define INTERFACE_INTERNALS_MARMALADE_H

#include "IwTexture.h"

#ifdef __cplusplus
extern "C"				// All interfaces use a C-linkage
{
#endif

struct TEXTURED_RECT_DEF;

typedef CIwTexture*		LPGRAPHICSTEXTURE;


extern LPGRAPHICSTEXTURE	InterfaceGetPlatformTexture( const char* szFilename, int nFlags, int nArchiveHandle, int* pnPitch );
extern LPGRAPHICSTEXTURE	InterfaceGetBlankPlatformTexture( int nWidth, int nHeight, int nMode, int* pnPitch );
extern void					InterfaceReleasePlatformTexture( int, LPGRAPHICSTEXTURE );

extern void					InterfacePlatformAddTexturedRectVertices( int nOverlayNum, TEXTURED_RECT_DEF* pxRectDef );
extern void					InterfacePlatformAddTexturedTriVertices( int nOverlayNum, TEXTURED_RECT_DEF* pxRectDef );
extern void					InterfacePlatformAddSpriteVertices( int nOverlayNum, TEXTURED_RECT_DEF* pxRectDef );

extern void					InterfaceDrawTexturedPolys( int nOverlayNum, int hTexture, int nLayer );

extern void*				InterfacePlatformGetTexturedOverlayMaterial( int hTexture, int nRenderType );
extern void					InterfacePlatformReleaseTexturedOverlayMaterial( void* pPlatformMaterial );

#ifdef __cplusplus
}
#endif



#endif
