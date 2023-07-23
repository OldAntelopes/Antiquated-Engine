#ifndef INTERFACE_DX9DEVICE_H
#define INTERFACE_DX9DEVICE_H


#ifdef __cplusplus
extern "C"				// All interfaces use a C-linkage
{
#endif
	

// Some dodgy looking externed variables :]

extern int		mnRenderSurfaceWidth;
extern int		mnRenderSurfaceHeight;  

extern int		mnDesiredWidth;
extern int		mnDesiredHeight;

extern BOOL InterfaceShowFullscreenPanic( char* szErrorString );


#ifdef __cplusplus
}
#endif


#endif
