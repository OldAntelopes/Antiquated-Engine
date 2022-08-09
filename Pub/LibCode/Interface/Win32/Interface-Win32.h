#ifndef INTERFACE_WIN32_H
#define INTERFACE_WIN32_H


#ifdef __cplusplus
extern "C"				// All interfaces use a C-linkage
{
#endif
	

extern void	InterfaceWin32SetInitialWindowSize( int sizeX, int sizeY );

extern void	InterfaceSetWindowStyle( bool bFullscreen );

extern void	InterfaceSetWindowHasChanged( BOOL );

extern HWND						mhwndInterfaceMain;

extern int		mnWindowLeft;
extern int		mnWindowTop;
extern int		mnWindowWidth;
extern int		mnWindowHeight;
extern BOOL		mboFullScreen;

#ifdef __cplusplus
}
#endif


#endif		// #ifndef INTERFACE_WIN32_H
