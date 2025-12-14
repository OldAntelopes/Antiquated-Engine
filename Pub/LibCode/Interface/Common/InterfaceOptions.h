#ifndef INTERFACE_OPTIONS_H
#define INTERFACE_OPTIONS_H

#ifdef __cplusplus
extern "C"
{
#endif

// 
// These are values for the game vid-options dialog. Also can be accessed through InterfaceGetOptions / InterfaceSetOptions
enum
{
	TEXTURE_FILTERING = 0,
	BACK_BUFFER,
	VSYNC,
	OLD_STARTUP,
	MINIMUM_SURFACE_RES,
	FOG_MODE,
	FSAA,
	MAX_OPTIONS,
};

/********************************************************
 ************	   Options Functions     ****************
 **													   **
 ** These routines are used to set & get video options **
 ********************************************************/
extern INTERFACE_API int InterfaceGetOption( int );
extern INTERFACE_API void InterfaceSetOption( int, int );

extern INTERFACE_API void InterfaceSetMipMapBias( float );
extern INTERFACE_API void InterfaceSetFilteringModes( int );


#ifdef WINUSERAPI	// Only include this if winuser.h has been previously included
extern INTERFACE_API void InterfaceInitVidOptions( HINSTANCE hInst, HWND hDialogWind ); 
extern INTERFACE_API INT_PTR CALLBACK InterfaceVidOptionsDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );
#endif

#ifdef __cplusplus
}
#endif

#endif
