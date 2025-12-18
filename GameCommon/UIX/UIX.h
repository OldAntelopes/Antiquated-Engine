#ifndef UIX_H
#define UIX_H

#include "StandardDef.h"
#include <vector>
#include <map>

class InterfaceInstance;
class UIXPage;
class UIXButton;
class UIXListBox;
class UIXSlider;
class UIXTextBox;
class UIXText;
class UIXCollapsableSection;
class UIXDropdown;


enum 
{
	UIX_RESERVED_BUTTONIDS = 0x550000,
	UIX_COLLAPSABLE_SECTION_HEADER,
	UIX_SLIDER_BAR,

};

enum UIX_TEXT_FLAGS
{
	NONE,
	ALIGN_RIGHT,
};

enum UIX_SLIDER_MODE
{
	VALUE,
	ANGLE,
};

enum UIX_VALUE_CALLBACK_FLAGS
{
	EMPTY,
	IS_BEING_MODIFIED,
};

typedef	float(*fnValueUpdateCallback)( uint32 ulUIXObjectID, float fUIXValue, uint32 ulUserParam, BOOL bIsUIHeld );

struct UIXRECT
{
	UIXRECT() { x = 0; y = 0; w = 0; h = 0; }
	UIXRECT( int vx, int vy, int vw, int vh) { x = vx; y = vy; w = vw; h = vh; }
	int x;
	int y;
	int w;
	int h;
};

class UIXObject
{
friend class UIX;
public:
	uint32			GetID() { return( mulID ); }

protected:
	UIXObject( uint32 uID, UIXRECT rect );

	virtual void		OnUpdate( float delta ) {}
	virtual UIXRECT		OnRender( InterfaceInstance* pInterface, UIXRECT rect ) { rect.h = 0; return rect; }
	virtual void		OnShutdown() {}

	void		Update( float delta );
	UIXRECT		Render( InterfaceInstance* pInterface, UIXRECT rect );
	void		Shutdown();

	UIXRECT		GetDisplayRect() { return( mDisplayRect ); }
private:
	virtual bool		ShouldDisplayChildren() { return true; }

	std::vector<UIXObject*>			mContainsList;
	uint32			mulID;
	UIXRECT			mDisplayRect;

};


//-------------------------------------------------

class UIX
{
friend class UIXObject;
public:
	static void		Initialise( int mode );
	static void		Update( float delta );
	static void		Render( InterfaceInstance* pInterface );
	static void		Shutdown();
	static void		Reset();
	static void		ButtonPressHandler( int nButtonID, uint32 ulParam );
	static void		SliderHoldHandler( int nButtonID, uint32 ulParam, BOOL bIsHeld, BOOL bFirstPress );

	static UIXObject*					AddPage( UIXRECT xRect, const char* szTitle );
	static UIXCollapsableSection*		AddCollapsableSection( UIXObject* pxContainer, UIXRECT xRect, int mode, const char* szTitle, BOOL bStartCollapsed );
	static UIXButton*					AddButton( UIXObject* pxContainer, UIXRECT xRect, int mode, const char* szTitle, uint32 ulButtonID, uint32 ulButtonParam );
	static UIXTextBox*					AddTextBox( UIXObject* pxContainer, UIXRECT xRect );
	static UIXListBox*					AddListBox( UIXObject* pxContainer, UIXRECT xRect );
	static UIXSlider*					AddSlider( UIXObject* pxContainer, UIXRECT xRect, UIX_SLIDER_MODE mode = VALUE, uint32 ulUserParam = 0, float fMin = 0.0f, float fMax = 1.0f, float fInitial = 0.0f, float fMinStep = 0.1f  );
	static UIXDropdown*					AddDropdown( UIXObject* pxContainer, UIXRECT xRect );
	static UIXText*						AddText( UIXObject* pxContainer, UIXRECT xRect, const char* szTitle, uint32 ulCol = 0xc0c0c0c0, int font = 0, UIX_TEXT_FLAGS fontFlags = NONE );

protected:
	static std::map<uint32, UIXObject*>		msComponentIDMap;
private:
	static uint32						msulNextObjectID;
	static std::vector<UIXObject*>		msPagesList;
};





#endif