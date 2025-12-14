#ifndef INTERFACE_DX9DEVICE_H
#define INTERFACE_DX9DEVICE_H


#ifdef __cplusplus
extern "C"				// All interfaces use a C-linkage
{
#endif
	

// Some dodgy looking externed variables :]

extern int		mnDesiredWidth;
extern int		mnDesiredHeight;

extern BOOL InterfaceShowFullscreenPanic( char* szErrorString );

extern LPGRAPHICSDEVICE InterfaceCreateNewGraphicsDevice( HWND hWindow, BOOL boMinPageSize );


#ifdef __cplusplus
}
#endif


#endif
