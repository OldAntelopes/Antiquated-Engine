
#ifndef TEXTURED_OVERLAYS_H
#define TEXTURED_OVERLAYS_H

#ifdef __cplusplus
extern "C"				// All interfaces use a C-linkage
{
#endif

	
extern void InitTexturedOverlays( void );
extern void FreeTexturedOverlays( void );

extern void ResetTexturedOverlays( void );


extern void DrawTexturedOverlays( int nLayer );
extern void	InterfaceTexturedOverlaysSetLimit( int nState );

extern int	TexturedOverlayCreate( int nLayer, TEXTURE_HANDLE hTexture );

extern int	TexturedOverlayCreateDirect( int nLayer, void* pTexture, int hEngineTexture );


#ifdef __cplusplus
}
#endif

#endif


