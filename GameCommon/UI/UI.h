#ifndef GAMECOMMON_UI_H
#define GAMECOMMON_UI_H

class InterfaceInstance;

// ---------- Reserved buttonIDs
enum
{
	UIRESERVEDBUTTONID_USERLOGINCLIENT_NEWUSER = -2,
	UIRESERVEDBUTTONID_STANDARDLOGININTERFACE = -3,
};

typedef	void(*UIButtonHandler)( int nButtonID, uint32 ulParam );

typedef	void(*fnValueChangeCallback)( int hDropdownHandle, int nNewSelectedParam, void* pUserParam );


//-------------------------------------- UIButton

extern void		UIRegisterButtonPressHandler( int nButtonID, UIButtonHandler fnButtonHandler );

extern void		UIButtonDraw( int nButtonID, int nX, int nY, int nWidth, int nHeight, const char* szText, int nMode, uint32 ulParam );
extern BOOL		UIButtonRegion( int nButtonID, int nX, int nY, int nWidth, int nHeight, uint32 ulParam );
extern void		UIButtonDrawAlpha( int nButtonID, int nX, int nY, int nWidth, int nHeight, const char* szText, int nMode, uint32 ulParam, float fAlpha );


//---------------------------------------- UISlider

extern int		UISliderCreate( int nValue, int nMin, int nMax );
extern void		UISliderSetLimits( int nHandle, int nMin, int nMax );
extern void		UISliderDraw( int nHandle, int nX, int nY, int nWidth, int nHeight, int nFlags, float fGlobalAlpha );
extern int		UISliderGetValue( int nHandle );
extern void		UISliderSetValue( int nHandle, int nValue );
extern void		UISliderDestroy( int nHandle );

//---------------------------------------- UIScrollablePage
extern int		UIScrollablePageCreate( void );
extern BOOL		UIScrollablePageRender( int nHandle, int ScreenX, int ScreenY, int ScreenW, int ScreenH, int nFullH );
extern BOOL		UIScrollablePageRenderHorizontal( int nHandle, int ScreenX, int ScreenY, int ScreenW, int ScreenH, int nFullW );
extern int		UIScrollablePageGetPosition( int nHandle );
extern void		UIScrollablePageDestroy( int nHandle );

//---------------------------------------- UIDropdown
extern int		UIDropdownCreate( void );
extern int		UIDropdownAddElement( int nHandle, const char* szElementName, uint32 ulElementParam );
extern void		UIDropdownRender( int nHandle, int ScreenX, int ScreenY, int ScreenW, int ScreenH, int nFullH, float fAlpha );
extern int		UIDropdownGetSelection( int nHandle, char* szElementNameOut, uint32* pulElementParamOut );
extern uint32	UIDropdownGetSelectedParam( int nHandle );
extern int		UIDropdownGetNumElements( int nHandle );
extern void		UIDropdownSetValueChangeCallback( int nHandle, fnValueChangeCallback callbackFunc, void* pUserParam );

extern void		UIDropdownReset( int nHandle );
extern void		UIDropdownDestroy( int nHandle );

//---------------------------------------- UITextBox
extern int			UITextBoxCreate( int nMode, const char* szInitialText, int nMaxTextLen );
extern void			UITextBoxRender( int nHandle, int nScreenX, int nScreenY, int nScreenW, int nScreenH );
extern const char*	UITextBoxGetText( int nHandle );
extern void			UITextBoxEndEdit( int nHandle );
extern void			UITextBoxDestroy( int nHandle );


//----------------------------------------------------------------------------
//---------------------- UI Operational Functions -----------------------------
extern void		UIInitialise( InterfaceInstance* pInterfaceInstance = NULL );
extern void		UIUpdate( float fDelta );
extern void		UIShutdown( void );

extern void		UIReleaseGraphicsForDeviceReset( void );
extern void		UIInitGraphicsPostDeviceReset( void );

extern BOOL		UIOnPress( int X, int Y );
extern BOOL		UIOnRelease( int X, int Y );
extern BOOL		UIOnZoom( float fZoomAmount );		// Mousewheel

extern void		UIOnInterfaceDraw( void );

//-----------------------------------------------------------------
// UI Internal

extern void		UIPressIDSet( int nButtonID, uint32 ulParam );
extern BOOL		UIIsPressed( int X, int Y, int W, int H );
extern BOOL		UIHoverItem( int X, int Y, int W, int H );

extern void		UIGetCurrentCursorPosition( int* pnX, int* pnY );
extern void		UISetCurrentCursorPosition( int nX, int nY );

InterfaceInstance*		UIInterfaceInstance();


#endif