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
	void		RenderDebugInfo();
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

	// Presses a button as a click on it would (controller focus; see UISetControllerSelectableHook)
	BOOL		ControllerPressButton( int nButtonID, uint32 ulParam, uint32 ulIDParam );

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

extern uint32	UIGetGlowColour();

//----------------------------------------------------------------------------
//---------------------- Controller (pad) focus hook --------------------------
// Optional, and off unless the app installs a hook. An app that gives a pad focus over its UI installs one with
// UISetControllerSelectableHook; then each control that takes input reports itself to the hook as it's drawn (what it
// is, its rect, its IDs), and the app calls UIControllerSelectableActivate (A) or UIControllerSelectableStep
// (left / right) on whichever one has focus. With no hook installed nothing is reported and the controls behave
// exactly as before. UIX buttons, checkboxes and dropdown entries draw through UIButtonDraw, so they report too.
enum eUICONTROLLER_SELECTABLE_KIND
{
	UICONTROLLERSELECTABLE_BUTTON = 0,		// UIButtonDraw / UIButtonDrawAlpha / UIButtonRegion (not disabled): activate presses it
	UICONTROLLERSELECTABLE_CHECKBOX,		// CheckboxDraw, or a UIBUTTON_FLAG_CHECKBOX button: activate presses it
	UICONTROLLERSELECTABLE_SLIDER,			// UISliderDraw: step moves it by about a twentieth of its range
	UICONTROLLERSELECTABLE_DROPDOWN,		// UIDropdownRender: activate opens or closes the list, step chooses the previous / next entry
	UICONTROLLERSELECTABLE_TEXTBOX,			// UITextBoxRender: activate starts editing it, as a click would
};

struct UIControllerSelectable
{
	int				nKind;		// eUICONTROLLER_SELECTABLE_KIND
	int				nX;
	int				nY;
	int				nW;
	int				nH;
	int				nID;		// The button ID, or the slider, dropdown or text box handle
	uint32			ulParam;
	uint32			ulIDParam;
	UIInstance*		pUIInstance;	// The instance a button or text box was drawn with (NULL for sliders and dropdowns)
};

// Called once for each control as it's drawn; copy what you need, the pointer is only valid during the call
typedef	void(*fnUIControllerSelectableHook)( const UIControllerSelectable* pSelectable );

extern void		UISetControllerSelectableHook( fnUIControllerSelectableHook fnHook );		// NULL (the default) turns it off
extern fnUIControllerSelectableHook		UIGetControllerSelectableHook( void );
// Return TRUE if the control took the press or the step. nDirection is -1 (left) or +1 (right)
extern BOOL		UIControllerSelectableActivate( const UIControllerSelectable* pSelectable );
extern BOOL		UIControllerSelectableStep( const UIControllerSelectable* pSelectable, int nDirection );

// UI internal: the controls call this as they draw
extern void		UIControllerSelectableReport( int nKind, int nX, int nY, int nW, int nH, int nID, uint32 ulParam, uint32 ulIDParam, UIInstance* pUIInstance );

#endif