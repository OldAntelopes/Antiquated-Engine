#ifndef GAMECOMMON_UI_H
#define GAMECOMMON_UI_H

#include <map>
class InterfaceInstance;
class UIButtonImpl;
class UITextBoxImpl;

// ---------- Reserved buttonIDs
enum
{
	UIRESERVEDBUTTONID_USERLOGINCLIENT_NEWUSER = -2,
	UIRESERVEDBUTTONID_STANDARDLOGININTERFACE = -3,
};

enum eUIBUTTON_MODE_FLAGS
{
	UIBUTTON_FLAG_NONE = 0,
	UIBUTTON_FLAG_DISABLED = 0x01,
	UIBUTTON_FLAG_HOVERED = 0x02,
	UIBUTTON_FLAG_LABEL_EDIT = 0x4,
	UIBUTTON_FLAG_NO_LABEL = 0x8,
	UIBUTTON_FLAG_SMALL_FONT = 0x10,
	UIBUTTON_FLAG_FLAT_STYLE = 0x20,
	UIBUTTON_FLAG_CHECKBOX = 0x40,
	UIBUTTON_FLAG_ISCHECKED = 0x80,
};
//-------------------------------------- UIButton

typedef	void(*UIButtonHandler)( int nButtonID, uint32 ulParam, uint32 ulIDParam );
// UIHoldHandler should return TRUE if it has handled the MouseUp action and no further ui should react
typedef	BOOL(*UIHoldHandler)( int nButtonID, uint32 ulParam, uint32 ulIndex, BOOL bIsHeld, BOOL bFirstPress );

typedef	void(*fnValueChangeCallback)( int hDropdownHandle, int nNewSelectedParam, void* pUserParam );
//--------------------------------------------------

class UIInstance
{
public:

	void		RegisterButtonPressHandler( int nButtonID, UIButtonHandler fnButtonHandler );
	void		RegisterHoldHandler( int nButtonID, UIHoldHandler fnHoldHandler );

	//-----------------------------------------
	void		ButtonDraw( int nButtonID, int nX, int nY, int nWidth, int nHeight, const char* szText, eUIBUTTON_MODE_FLAGS modeFlags, uint32 ulParam, uint32 ulIDParam = 0 );
	void		ButtonDrawAlpha( int nButtonID, int nX, int nY, int nWidth, int nHeight, const char* szText, eUIBUTTON_MODE_FLAGS modeFlags, uint32 ulParam, uint32 ulIDParam = 0,  float fAlpha = 0.5f );
	void		CheckboxDraw(int nButtonID, int nX, int nY, int nWidth, int nHeight, const char* szText, BOOL bChecked, uint32 ulParam, uint32 ulIDParam = 0);

	// ---------------------- TextBox -----------------------------
	int			TextBoxCreate( int nMode, const char* szInitialText, int nMaxTextLen );
	void		TextBoxRender( int nHandle, int nScreenX, int nScreenY, int nScreenW, int nScreenH );
	const char*	TextBoxGetText( int nHandle );
	void		TextBoxEndEdit( int nHandle );
	void		TextBoxDestroy( int nHandle );

	// ---------------------- Dropdown -----------------------------
	int			DropdownCreate( void );
	int			DropdownAddElement( int nHandle, const char* szElementName, uint32 ulElementParam );
	void		DropdownRender( int nHandle, int ScreenX, int ScreenY, int ScreenW, int ScreenH, int nFullH, float fAlpha );
	int			DropdownGetSelection( int nHandle, char* szElementNameOut, uint32* pulElementParamOut );
	uint32		DropdownGetSelectedParam( int nHandle );
	int			DropdownGetNumElements( int nHandle );
	void		DropdownSetValueChangeCallback( int nHandle, fnValueChangeCallback callbackFunc, void* pUserParam );
	void		DropdownDestroy( int nHandle );


	//---------------------- UI Operational Functions -----------------------------
	void		Initialise( InterfaceInstance* pInterfaceInstance = NULL, BOOL bMinimalInit = FALSE );
	void		Update( float fDelta );
	void		Shutdown( void );

	void		ReleaseGraphicsForDeviceReset( void );
	void		InitGraphicsPostDeviceReset( void );

	// Press/control handlers
	BOOL		OnPress( int X, int Y );
	BOOL		OnRightButtonPress( int X, int Y );
	BOOL		OnRelease( int X, int Y );
	BOOL		OnReleaseRightButton( int X, int Y );
	BOOL		OnZoom( float fZoomAmount );		// Mousewheel

	void		OnInterfaceDraw( void );
	
	//-----------------------------------------------------------------
	// UI Internal

	void		HoverIDSet( int nButtonID, uint32 ulParam, uint32 ulIndex = 0 );
	void		PressIDSet( int nButtonID, uint32 ulParam, uint32 ulIndex = 0 );
	void		RightPressIDSet( int nButtonID, uint32 ulParam, uint32 ulIndex = 0 );
	BOOL		IsPressed( int X, int Y, int W, int H );
	BOOL		HoverItem( int X, int Y, int W, int H );
	BOOL		IsMouseHover( int X, int Y, int W, int H );
	BOOL		IsRightPressed( int X, int Y, int W, int H );

	void		GetCurrentCursorPosition( int* pnX, int* pnY );
	void		SetCurrentCursorPosition( int nX, int nY );
	
	uint32		GetCurrentHoverIDIndexParam();
	uint32		GetCurrentPressIDIndexParam();

	InterfaceInstance*		GetInterfaceInstance();

	// Just for ease of transferring from the old static model
	// (Should be able to move them back to protected once the msTempSingleton is removed)
	UIButtonImpl*		mpUIButtonImpl = NULL;
	UITextBoxImpl*		mpUITextBoxImpl = NULL;

protected:

	InterfaceInstance*		mpInterfaceInstance = NULL;
	

	short		mwUIPressX = 0;
	short		mwUIPressY = 0;
	short		mwUIRightPressX = 0;
	short		mwUIRightPressY = 0;
	int			mnUISetCursorX = NOTFOUND;
	int			mnUISetCursorY = NOTFOUND;

	int			mnUIButtonIDPressed = NOTFOUND;
	uint32		mulUIButtonIDPressedParam = 0;
	uint32		mulUIButtonIDPressedIDParam = 0;
	int			mnUIRightButtonIDPressed = NOTFOUND;
	uint32		mulUIRightButtonIDPressedParam = 0;
	uint32		mulUIRightButtonIDPressedIDParam = 0;

	int			mnUIButtonIDHovered = NOTFOUND;
	uint32		mulUIButtonIDHoveredParam = 0;
	uint32		mulUIButtonIDHoveredID = 0;

	int			mnUIButtonIDHeld = NOTFOUND;
	uint32		mulUIButtonIDHeldParam = 0;
	uint32		mulUIButtonIDHeldID = 0;

	std::map<int, UIButtonHandler>	msButtonHandlerList;
	std::map<int, UIHoldHandler>	msHoldHandlerList;
};

extern void		UIRegisterButtonPressHandler( int nButtonID, UIButtonHandler fnButtonHandler );
extern void		UIRegisterHoldHandler( int nButtonID, UIHoldHandler fnHoldHandler );

extern void		UIButtonDraw( int nButtonID, int nX, int nY, int nWidth, int nHeight, const char* szText, eUIBUTTON_MODE_FLAGS modeFlags, uint32 ulParam, uint32 ulIDParam = 0 );
extern BOOL		UIButtonRegion( int nButtonID, int nX, int nY, int nWidth, int nHeight, uint32 ulParam, uint32 ulIDParam = 0  );
extern void		UIButtonDrawAlpha( int nButtonID, int nX, int nY, int nWidth, int nHeight, const char* szText, eUIBUTTON_MODE_FLAGS modeFlags, uint32 ulParam, uint32 ulIDParam = 0,  float fAlpha = 0.5f );


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

//---------------------------------------- UIListBox
extern int		UIListBoxCreate( BOOL bContentsDraggable = FALSE );
extern int		UIListBoxAddElement( int nHandle, const char* szElementName, uint32 ulElementParam );
extern void		UIListBoxRender( int nHandle, int ScreenX, int ScreenY, int ScreenW, int ScreenH, int nFullH, float fAlpha );
extern int		UIListBoxGetSelection( int nHandle, char* szElementNameOut, uint32* pulElementParamOut );
extern int		UIListBoxGetNumElements( int nHandle );

//---------------------------------------- UITextBox
extern int			UITextBoxCreate( int nMode, const char* szInitialText, int nMaxTextLen );
extern void			UITextBoxRender( int nHandle, int nScreenX, int nScreenY, int nScreenW, int nScreenH );
extern const char*	UITextBoxGetText( int nHandle );
extern void			UITextBoxEndEdit( int nHandle );
extern void			UITextBoxDestroy( int nHandle );


//----------------------------------------------------------------------------
//---------------------- UI Operational Functions -----------------------------
extern void		UIInitialise( InterfaceInstance* pInterfaceInstance = NULL );
extern void		UISetActiveInterface( InterfaceInstance* pInterfaceInstance );
extern void		UIUpdate( float fDelta );
extern void		UIShutdown( void );

extern void		UIReleaseGraphicsForDeviceReset( void );
extern void		UIInitGraphicsPostDeviceReset( void );

extern BOOL		UIOnPress( int X, int Y );
extern BOOL		UIOnRightButtonPress( int X, int Y );
extern BOOL		UIOnRelease( int X, int Y );
extern BOOL		UIOnReleaseRightButton( int X, int Y );
extern BOOL		UIOnZoom( float fZoomAmount );		// Mousewheel

extern void		UIOnInterfaceDraw( void );

//-----------------------------------------------------------------
// UI Internal

extern void		UIHoverIDSet( int nButtonID, uint32 ulParam, uint32 ulIndex = 0 );
extern void		UIPressIDSet( int nButtonID, uint32 ulParam, uint32 ulIndex = 0 );
extern void		UIRightPressIDSet( int nButtonID, uint32 ulParam, uint32 ulIndex = 0 );
extern BOOL		UIIsPressed( int X, int Y, int W, int H );
extern BOOL		UIHoverItem( int X, int Y, int W, int H );
extern BOOL		UIIsRightPressed( int X, int Y, int W, int H );
extern BOOL		UIIsMouseHover( int X, int Y, int W, int H );

extern void		UIGetCurrentCursorPosition( int* pnX, int* pnY );
extern void		UISetCurrentCursorPosition( int nX, int nY );

InterfaceInstance*		UIInterfaceInstance();

extern uint32	UIGetCurrentHoverIDIndexParam();
extern uint32	UIGetCurrentPressIDIndexParam();


#endif