#ifndef ATRACTOR2_UI_H
#define ATRACTOR2_UI_H



#ifdef __cplusplus
extern "C"
{
#endif

enum
{
	UIBUTTONID_NULL,
	UIBUTTONID_MENUINTERFACE,

	UIBUTTONID_MAX,
};

typedef	void(*UIButtonHandler)( int nButtonID, uint32 ulParam );

//-------------------------------------- UIButton

extern void		UIRegisterButtonPressHandler( int nButtonID, UIButtonHandler fnButtonHandler );

extern void		UIButtonDraw( int nButtonID, int nX, int nY, int nWidth, int nHeight, const char* szText, int nMode, uint32 ulParam );

extern BOOL		UIButtonRegion( int nButtonID, int nX, int nY, int nWidth, int nHeight, uint32 ulParam );


//---------------------------------------- UISlider

extern int		UISliderCreate( int nValue, int nMin, int nMax );
extern void		UISliderDraw( int nHandle, int nX, int nY, int nWidth, int nHeight, int nFlags );
extern int		UISliderGetValue( int nHandle );
extern void		UISliderSetValue( int nHandle, int nValue );
extern void		UISliderDestroy( int nHandle );

//---------------------------------------- UIScrollablePage
extern int		UIScrollablePageCreate( void );
extern void		UIScrollablePageRender( int nHandle, int ScreenX, int ScreenY, int ScreenW, int ScreenH, int nFullH );
extern void		UIScrollablePageRenderHorizontal( int nHandle, int ScreenX, int ScreenY, int ScreenW, int ScreenH, int nFullW );
extern int		UIScrollablePageGetPosition( int nHandle );
extern void		UIScrollablePageDestroy( int nHandle );

//---------------------------------------- UICheckbox
extern int		UICheckboxCreate( const char* szText, BOOL bIsChecked );
extern void		UICheckboxRender( int nHandle, int ScreenX, int ScreenY, int ScreenW, int ScreenH );
extern BOOL		UICheckboxIsChecked( int nHandle );
extern void		UICheckboxDestroy( int nHandle );


//---------------------- UI Functions
extern void		UIUpdate( float fDelta );

extern BOOL		UIOnPress( int X, int Y );
extern BOOL		UIOnRelease( int X, int Y );

extern BOOL		UIIsMouseHover( int X, int Y, int W, int H );
extern BOOL		UIIsPressed( int X, int Y, int W, int H );

#ifdef __cplusplus
}
#endif




#endif