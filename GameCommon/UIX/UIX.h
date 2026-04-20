#ifndef UIX_H
#define UIX_H

#include "StandardDef.h"
#include <vector>
#include <string>
#include <map>

class InterfaceInstance;
class UIXObject;
class UIXPage;
class UIXButton;
class UIXListBox;
class UIXSlider;
class UIXTextBox;
class UIXText;
class UIXCollapsableSection;
class UIXScrollableSection;
class UIXDropdown;
class UIXShape;
class UIXCustomRender;
class UIXCheckbox;
class UIXModalPopup;
class UIXMenu;
class UIXPopupMenu;
class UIXTabBar;

#define		MAX_NUM_UIX_ICONS		40

enum 
{
	UIX_RESERVED_BUTTONIDS = 0x550000,
	UIX_COLLAPSABLE_SECTION_HEADER,
	UIX_SLIDER_BAR,
	UIX_LISTBOX,
	UIX_SLIDER_BAR_MINRANGE,
	UIX_SLIDER_BAR_MAXRANGE,
	UIX_SCROLLABLE_SECTION_SCROLLBAR,
	UIX_DROPDOWN_HEADER,
	UIX_DROPDOWN_ENTRY,
	UIX_CHECKBOX,
	UIX_LISTBOX_SELECT,
	UIX_SCROLLBAR,
	UIX_TEXTBOX,
	UIX_MENU_ITEM,
	UIX_TAB_SELECT,
	UIX_BUTTON,
	UIX_POPUP_MENU_ITEM,
	UIX_CUSTOM_RENDER,
	UIX_RIGHT_CLICK_SELECT,
	UIX_OBJECT_SELECT,
};

enum eUIXBUTTON_MODE
{
	UIXBUTTON_NORMAL,
	UIXBUTTON_PLAIN_RECT,
	UIXBUTTON_TEXT_WITH_SETTINGS,
	UIXBUTTON_RECT_ICON,
	UIXBUTTON_IMAGE,
	UIXBUTTON_ICON,
	UIXBUTTON_IMAGE_PATH,
	UIXBUTTON_TEXT_WITH_COLLAPSABLE,
};

enum eUIXSHAPE_MODE
{
	UIXSHAPE_SHADEDRECT = 0,
	UIXSHAPE_OUTLINEBOX,
	UIXSHAPE_TRANSOUTLINEBOX,
};

enum UIX_TEXT_FLAGS
{
	NONE = 0,
	ALIGN_RIGHT = 0x1,
	BOLD = 0x2,
	ALIGN_CENTRE = 0x4,
};

enum UIX_SLIDER_MODE
{
	SLIDERMODE_VALUE,
	SLIDERMODE_ANGLE,
	SLIDERMODE_VALUERANGE,
	SLIDERMODE_VERTICAL_VALUE,
	SLIDERMODE_PLUSMINUS_VALUE,
	SLIDERMODE_VALUE_WITH_CONSTRAINTS,
};

enum UIX_CHECKBOX_MODE
{
	STANDARD_CHECKBOX,
	STANDARD_CHECKBOX_WITH_LABEL,
	POPUP_MENU_LIST,
	ICON_CHECKBOX,
	ICON_CHECKBOX_OUTLINED,
	HILIGHT_CHECKBOX,
	HILIGHT_CHECKBOX2,
};

enum UIX_VALUE_CALLBACK_FLAGS
{
	EMPTY,
	IS_BEING_MODIFIED,
};


// Value Update callbacks are used by sliders etc to both inform the user of the current value(s) of the slider and accept changes to the value from the outside
typedef	float(*fnValueUpdateCallback)( uint32 ulUIXObjectID, float fUIXValue, float fUIXMinRangeVal, float fUIXMaxRangeValue, uint32 ulUserParam, BOOL bIsUIHeld );
typedef	void(*fnDragReceiveCallback)( UIXObject* pxSourceObject, uint32 ulDragParam, UIXObject* pxDestObject, uint32 ulDragDestParam, const char* szDragDropFilename );
typedef	void(*fnSelectedCallback)( UIXObject* pxSourceObject, uint32 ulSelectParam );
typedef	void(*fnObjectSelectionCallback)( UIXObject* pxSelectedObject );

class UIStateData
{
public:
	void			OnUpdate( float fDelta );

	BOOL			mbTabOpened = TRUE;
	// TODO - Need to do something to handle this pointer reference better
	UIXObject*		mpAssociatedUIXObj = NULL;
};

struct UIXRECT
{
	UIXRECT() { x = 0; y = 0; w = 0; h = 0; }
	UIXRECT( int vx, int vy, int vw, int vh) { x = vx; y = vy; w = vw; h = vh; }

	void	ConsumeVertical( int amount ) { y += amount; h -= amount; }
	void	ConsumeHorizontal( int amount ) { x += amount; w -= amount; }

	BOOL	IsWithinRect( int vx, int vy ) 
	{ 
		if ( ( vx >= x ) && 
			 ( vx <= x + w ) && 
			 ( vy >= y ) && 
			 ( vy <= y + h ) )
		{
			return( TRUE );
		}
		return( FALSE );
	}

	int x;
	int y;
	int w;
	int h;
};

typedef	UIXRECT(*fnCustomRenderCallback)( UIXObject* pObj, InterfaceInstance* pInterface, UIXRECT& rectInOut, uint32 ulUserParam );
typedef	BOOL(*fnCustomDragHoldHandlerCallback)(UIXObject* pObj, uint32 ulParam, BOOL bIsHeld, BOOL bFirstPress);
typedef const char*(*fnCustomTooltipCallback)(UIXObject* pObj, uint32 ulParam);


class UIXObject
{
friend class UIX;
friend class MilkPresetBrowser;
public:
	uint32			GetID() const { return( mulID ); }

	//------------------------------------------ User params
	void*				GetUserObject() { return mpUserObject; }
	void				SetUserObject( void* pObject, bool bDeleteOnShutdown = false ) { mpUserObject = pObject; mbDeleteUserObjectOnDestroy = bDeleteOnShutdown; }

	BOOL				DoesHaveUserParamEx( const char* szKey ) { return( (mUserParamExList.find(szKey) != mUserParamExList.end()) ); }
	int					GetUserParamEx( const char* szKey ) { return( mUserParamExList[szKey] ); }
	void				SetUserParamEx( const char* szKey, int nValue ) { mUserParamExList[szKey] = nValue; }
	//------------------------------------------

	void				SetDragReceiveCallback(int dragType, fnDragReceiveCallback func, uint32 ulDestParam);
	void				SetDraggable(int nDragItemType, uint32 ulDragParam);
	//------------------------------------------
	void				SetTooltipCallback(fnCustomTooltipCallback func, uint32 ulTooltipParam) { mfnCustomTooltipCallback = func; mulCustomTooltipParam = ulTooltipParam; }

	virtual void		UpdateUIStateData( UIStateData* pData ) {}
	virtual float		OnValueChange( UIXObject* pxSourceObj, float fNewValue, BOOL bByUserEditFlag ) { return( fNewValue ); }
	
	bool				HasSelectionCallback() { return( mfnSelectedCallback != NULL ); }
	bool				HasRightClickSelectionCallback() { return( mfnRightClickSelectedCallback != NULL ); }
	bool				HasChildren() const { return( mContainsList.size() > 0 ); }
	bool				DoesContainObjectID( uint32 ulUIXID ) const;
	UIXObject*			GetParent() const { return(mpParent); }
	virtual int			GetScrollPosition() { return( 0 ); }

	void				SetRightClickSelectedCallback(fnSelectedCallback  callbackFunc, uint32 ulSelectParam) { mfnRightClickSelectedCallback = callbackFunc; mulRightClickSelectParam = ulSelectParam; }
	void				SetBasePriority( int priorityVal ) { mBasePriority = priorityVal; }

	void				SetObjectSelectionText( const char* szText ) { mSelectObjectText = szText; }		// Displayed when we're in ObjectSelect mode
	
	void				SetUIDParam(u64 param) { mullUIDParam = param; }
	u64					GetUIDParam() { return mullUIDParam; }	

	// For custom drag activation
	void				ActivateDragHold(UIXRECT rect, uint32 ulDragParam);
	virtual void		OnReceiveDragItem( int dragType, UIXObject* pxSourceObject, uint32 ulDragParam, const char* szDragdropFilename = NULL );
protected:
	UIXObject( UIXObject* pParent, uint32 uID, UIXRECT rect );
	virtual ~UIXObject();

	virtual void		OnUpdate( float delta ) {}
	virtual UIXRECT		OnRender( InterfaceInstance* pInterface, UIXRECT rect ) { rect.h = 0; return rect; }
	virtual void		OnPostRender( InterfaceInstance* pInterface, UIXRECT rect ) {}
	virtual void		OnShutdown() {}
	virtual void		OnPostChildrenRender( InterfaceInstance* pInterface ) { }
	virtual void		OnMouseWheel( float fAmount ) {}
	virtual bool		OnSelected( int nButtonID, uint32 ulParam ) { return( true ); }		// Returns true to indicate we should continue to call any other selectionCallbacks (i.e. we can guarantee the current object is still valid (HACK))
	virtual void		OnCloseAllMenus() {}
	virtual void		EndEdit() {}
	virtual void		OnEscape() {}
	virtual void		OnFocusedKeyUp( int keyCode ) {}
	virtual void		OnCloseAllDropdowns( uint32 ulExceptID ) {}
	virtual const char*		GetTooltipText();

	const std::string&			GetObjectSelectionText() { return( mSelectObjectText ); }

	void		Update( float delta );
	UIXRECT		Render( InterfaceInstance* pInterface, UIXRECT rect );
	void		PostRender(InterfaceInstance* pInterface);	
	void		Shutdown();
	void		KeyUp(int keyCode);
	void		SelectObject( int nButtonID, uint32 ulParam );
	void		CloseAllMenus();
	void		CloseAllDropdowns( uint32 ulExceptID );
	

	std::vector<UIXObject*>& GetChildObjectList() { return mContainsList; }
	UIXRECT			GetLocalPositionRect() { return(mDisplayRect); }
	UIXRECT			GetActualRenderRect(UIXRECT parentRect);
	int				GetChildContentsHeight() { return mChildContentsHeight; }
	int				GetChildContentsWidth() { return mChildContentsWidth; }		// childcontentswidth needs implementing for use by Page 
	virtual int		GetSelectionPriorityLayer() { return(mBasePriority); }

	void		SetSelectedCallback(fnSelectedCallback callbackFunc, uint32 ulSelectParam) { mfnSelectedCallback = callbackFunc; mulSelectParam = ulSelectParam; }
	//---------DRAG N DROP-------- Things for the uix object type to implement to support drag n drop
	// ******************************
	// DRAG an item:
	// - Make sure <UIX_ID> is listed in    UIX::Initialise   drag n drop section
	// - Use  UIHoverIDSet( <UIX_ID>, mIndex, mpListBox->GetID() );

	// - type implementation should update 'SetDraggableRenderRect' (sets the viusal rect that the user can drag)
	//			(OR : Trigger ActivateDragHold directly if its being fancy)
	// - in the type's OnRender, render the transparent drag item if ( IsDragHoldActive() && DragHasMoved )
	// It can also use SetDragText (when hovered) to pass text to the receiver
	//
	// DROP an item:  (what a type needs to do to an accept a dragged item)
	// 	
	// - in type's OnRender :
	//       if ( CheckDragHoverRegion( UIXRECT(dropRegion) ) )
	//				---- Draw highlight to indicate you can accept the item
	//				----- UIX::HoverAcceptDragItem(this);
	//
	// ( App uses SetDragReceiveCallback  to link the drag item type to a callback func )
	//---------------------------------------------------------------

	
	virtual void		OnHoverDragItem( int dragType ) {}
	virtual BOOL		OnDragHoldUpdate(uint32 ulParam, BOOL bIsHeld, BOOL bFirstPress) { return(FALSE); }		// Optional
	
	void				SetDraggableRenderRect(UIXRECT rect) { mDraggableRenderRect = rect; }

	// Core implementations for drag n drop--------------
	BOOL				IsDraggable() { return(mDragItemType != 0); }
	BOOL				IsDragHoldActive() { return(mbIsBeingDragged); }
	BOOL			    IsFocusedObject();
	BOOL				DragHasMoved();
	UIXRECT				GetDragOffset();
	UIXRECT				GetInitialDragRect() const { return(mDragRectOriginal); }
	BOOL				CheckDragHoverRegion(UIXRECT dragReceiveRegion);
	bool				CanReceiveDragItem( int dragType ) { return( mDragMap[dragType] ); }
	BOOL				HoldHandler( uint32 ulParam, BOOL bIsHeld, BOOL bFirstPress);
	static BOOL			HoldHandlerStatic(int nButtonID, uint32 ulParam, uint32 ulIndex, BOOL bIsHeld, BOOL bFirstPress);
	static void			RegisterDragControlHandler( int nButtonID );
	//-------------------------------------

	int					GetBasePriority() { return( mBasePriority ); }
	int					mChildContentsHeight = 0;			// Cheeky

private:
	virtual bool		ShouldDisplayChildren() { return true; }
	virtual bool		IncludeChildrenInOccupyCalc() { return true; }

	std::vector<UIXObject*>			mContainsList;
	std::map<std::string, int>		mUserParamExList;
	std::map<int, fnDragReceiveCallback>		mDragMap;
	std::map<int, uint32>						mDragMapParams;
//	std::string		mTooltipText;

	uint32			mulID;
	UIXRECT			mDisplayRect;
	void*			mpUserObject = NULL;
	bool			mbDeleteUserObjectOnDestroy = false;
	int				mChildContentsWidth = 0;
	UIXRECT			mLastRenderDisplayRect;
	UIXObject*		mpParent;
	fnSelectedCallback	mfnSelectedCallback = NULL;
	uint32				mulSelectParam = 0;
	u64					mullUIDParam = 0;	
	fnSelectedCallback	mfnRightClickSelectedCallback = NULL;
	uint32				mulRightClickSelectParam = 0;
	float				mfHoverTime = 0.0f;
	int					mBasePriority = 0;
	fnCustomTooltipCallback	mfnCustomTooltipCallback = NULL;
	uint32					mulCustomTooltipParam = 0;
	std::string				mSelectObjectText;

	//--------------------  Drag n drop stuff
	// todo - this stuff could probably be standardised into some form of standard draggable object

	UIXRECT				mDraggableRenderRect;
	int					mDragItemType = 0;
	uint32				mDragItemParam = 0;
	UIXRECT				mDragRectOriginal;
	UIXRECT				mDragRectMouseOriginal;
	BOOL				mbIsBeingDragged = FALSE;
	//---------------

};


//-------------------------------------------------

class UIX
{
friend class UIXObject;
friend class UIXPage;
public:

	static void		Initialise( int mode );
	static void		Update( float delta );
	static void		Render( InterfaceInstance* pInterface );
	static void		Shutdown();
	static void		Reset();
	static void		OnMouseWheel( float fOffset );
	static void		OnKeyUp( int keyCode );
	static void		CloseAllMenus();
	static void		CloseAllDropdowns( uint32 ulExceptID );
	static void		OnInterfaceDraw();

	static UIXObject*					AddPage( UIXRECT rect, const char* szTitle, BOOL bUseClipping = FALSE );
	static UIXObject*					AddSubPage( UIXObject* pxContainer, UIXRECT rect, const char* szTitle, BOOL bUseClipping = FALSE );
	static UIXCollapsableSection*		AddCollapsableSection( UIXObject* pxContainer, UIXRECT rect, UIXRECT headerRect, int mode, const char* szTitle, BOOL bStartCollapsed, int draggableType = 0 );
	static UIXScrollableSection*		AddScrollableSection( UIXObject* pxContainer, UIXRECT rect );
	static UIXButton*					AddButton( UIXObject* pxContainer, UIXRECT rect, eUIXBUTTON_MODE mode, const char* szTitle, uint32 ulButtonID, uint32 ulButtonParam, BOOL IsBlocking = TRUE, uint32 ulCol = 0xD0404040, int iconNum = 0 );
	static UIXTextBox*					AddTextBox( UIXObject* pxContainer, UIXRECT rect, int mode, const char* szDefaultText );
	static UIXListBox*					AddListBox( UIXObject* pxContainer, UIXRECT rect, int mode = 0, BOOL bContentsDraggable = FALSE, int dragItemType = 0 );
	static UIXSlider*					AddSlider( UIXObject* pxContainer, UIXRECT rect, UIX_SLIDER_MODE mode = SLIDERMODE_VALUE, uint32 ulUserParam = 0, float fMin = 0.0f, float fMax = 1.0f, float fInitial = 0.0f, float fMinStep = 0.1f, const char* szText = NULL, BOOL bShowTextBoxes = TRUE );
	static UIXDropdown*					AddDropdown( UIXObject* pxContainer, UIXRECT rect );
	static UIXText*						AddText( UIXObject* pxContainer, UIXRECT rect, uint32 ulCol = 0xc0c0c0c0, int font = 0, UIX_TEXT_FLAGS fontFlags = NONE,  const char* szTitle = NULL, ... );
	static UIXShape*					AddShape( UIXObject* pxContainer, UIXRECT rect, eUIXSHAPE_MODE mode = UIXSHAPE_SHADEDRECT, BOOL bBlocks = FALSE, uint32 ulCol1 = 0xC0C0C0C0, uint32 ulCol2 = 0xC0C0C0C0, uint32 ulButtonID = 0, uint32 ulButtonParam = 0 );
	static UIXCustomRender*				AddCustomRender( UIXObject* pxContainer, UIXRECT rect, fnCustomRenderCallback renderFunc, uint32 ulUserParam = 0, fnCustomDragHoldHandlerCallback dragFunc = NULL);
	static UIXCheckbox*					AddCheckbox( UIXObject* pxContainer, UIXRECT rect, UIX_CHECKBOX_MODE mode, BOOL bIsChecked, const char* szText, fnSelectedCallback selectedFunc, uint32 ulSelectParam = 0 );
	static UIXModalPopup*				AddModalPopup( UIXObject* pxContainer, UIXRECT rect );
	static UIXMenu*						AddMenuBar( UIXObject* pxContainer, UIXRECT rect );
	static UIXTabBar*					AddTabBar( UIXObject* pxContainer, UIXRECT rect );
	static UIXPopupMenu*				AddPopupMenu( UIXObject* pxContainer, UIXRECT rect );
	
	static void							DeleteObject( UIXObject* pObject );
	static UIXObject*					FindUIXObjectByID( uint32 ulObjectID );

	static void							SetDragItemType( int type, UIXObject* pxFromObject, uint32 param ) { msDragItemType = type; mspDragSource = pxFromObject; msDragSourceParam = param; }
	static int							GetDragItemType() { return msDragItemType; }
	static void							SetDragText( const char* szText ) { msDragText = szText; }
	static const char*					GetDragText() { return msDragText.c_str(); }
	static uint32						GetDragItemSourceParam() { return msDragSourceParam; }
	static UIXObject*					GetDragDestinationHover() { return mspDragDestinationHover; }
	static void							HoverAcceptDragItem( UIXObject* pxObject ) { mspDragDestinationHover = pxObject; }
	static void							EndDragItemType( int type );

	static void							SetMousewheelHoverObject(UIXObject* pObject);
	static void							SetActiveTooltip( int priority, const char* szText );
	
	static void							SetUISelectionMode(BOOL bActive) { mbUISelectionModeActive = bActive; }
	static BOOL							IsUISelectionModeActive() { return(mbUISelectionModeActive); }		

	static void							SetTextEditFocus( UIXObject* pObject );
	static UIXObject*					GetTextEditFocus() { return( mspTextEditFocusObject ); }

	static UIXObject*					GetFocusedObject() { return( mspFocusedSelectionObject ); }
	static BOOL							IsOffscreen( int x, int y );
	static BOOL							IsInActivePageRegion( int x, int y );
	static BOOL							IsRectInActivePageRegion( UIXRECT rect );
	static BOOL							IsMouseHover( UIXRECT rect, BOOL bSetCursor = TRUE );
	static BOOL							IsMouseHover( int x, int y, int w, int h, BOOL bSetCursor = TRUE );

	static BOOL							CheckForPress( UIXObject* pxObject, UIXRECT rect, uint32 ulButtonID, uint32 ulButtonParam );
	static BOOL							CheckForRightButtonPress( UIXObject* pxObject, UIXRECT rect, uint32 ulButtonID, uint32 ulButtonParam );

	static void							DrawIcon( InterfaceInstance* pInterface, int iconNum, UIXRECT rect, uint32 ulCol );
	static void							LoadIcon( InterfaceInstance* pInterface, int iconNum, const char* szFilename );
	static void							LoadIconSheet( InterfaceInstance* pInterface, int sheetNum, const char* szFilename );

	static uint32						GetNextObjectID();
	static uint32						GetCurrentPressObjectID();
	static void							RegisterObjectSelectionHandler( fnObjectSelectionCallback handler ) { msfnObjectSelectionHandler = handler; }  
protected:
	static void							SetActivePageRegion(UIXRECT rect) { mxActivePageRegion = rect; }

	static std::map<uint32, UIXObject*>		msComponentIDMap;
private:
	static void		ButtonPressHandler( int nButtonID, uint32 ulParam, uint32 ulIDParam );
	static BOOL		SliderHoldHandler( int nButtonID, uint32 ulParam, uint32 ulIndex, BOOL bIsHeld, BOOL bFirstPress );
	static void		RenderTooltip(InterfaceInstance* pxInterface);

	static uint32						msulNextObjectID;
	static std::vector<UIXPage*>		msPagesList;
	static int							msDragItemType;
	static UIXObject*					mspDragDestinationHover;
	static UIXObject*					mspDragSource;
	static UIXObject*					mspMousewheelHoverObject;
	static UIXObject*					mspTextEditFocusObject;
	static UIXObject*					mspFocusedSelectionObject;
	static uint32						msDragSourceParam ;
	static UIXObject*					mspModalObject;
	static UIXRECT						mxActivePageRegion;
	static int							msSelectionPriority;
	static int							msPressedSelectionPriority;
	static int							msMouseWheelHoverPriority;
	static int					mshUIXIconsList[MAX_NUM_UIX_ICONS];
	static int					mshUIXIconOverlays[MAX_NUM_UIX_ICONS];
	static std::string					msDragText;
	static std::string					msTooltipText;
	static int							msnTooltipPriority;
	static BOOL							mbUISelectionModeActive;		
	static fnObjectSelectionCallback	msfnObjectSelectionHandler;

};





#endif