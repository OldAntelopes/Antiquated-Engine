//------------------------------------------------------------------------------------------------
// UIXTestAccess.cpp
//
// Read access to UIX's private members for the tests. An explicit template instantiation may name a
// private member (C++ [temp.explicit]), so UIXTestRob stores a pointer to the member in a friend function
// that the accessors below call. This leaves UIX's headers alone; '#define private public' can't be used
// because MSVC puts access into mangled names, so the symbols wouldn't link.
//------------------------------------------------------------------------------------------------

#include "UIXTestAccess.h"

template< typename Tag, typename Tag::type M >
struct UIXTestRob
{
	friend typename Tag::type	UIXTestGet( Tag ) { return( M ); }
};

#define		UIXTEST_ACCESS( NAME, TYPE, MEMBER )	struct NAME { typedef TYPE type; friend type UIXTestGet( NAME ); }; \
													template struct UIXTestRob< NAME, MEMBER >;

typedef		int		UIXTestIconArray[MAX_NUM_UIX_ICONS];
typedef		UIXRECT	(UIXObject::*UIXTestGetRectMethod)( UIXRECT );

// UIX statics
UIXTEST_ACCESS( Rob_UIX_PagesList,			std::vector<UIXPage*>*,	&UIX::msPagesList )
UIXTEST_ACCESS( Rob_UIX_WheelHover,			UIXObject**,			&UIX::mspMousewheelHoverObject )
UIXTEST_ACCESS( Rob_UIX_DragSource,			UIXObject**,			&UIX::mspDragSource )
UIXTEST_ACCESS( Rob_UIX_DragItemType,		int*,					&UIX::msDragItemType )
UIXTEST_ACCESS( Rob_UIX_DragSourceParam,	uint32*,				&UIX::msDragSourceParam )
UIXTEST_ACCESS( Rob_UIX_TextEditFocus,		UIXObject**,			&UIX::mspTextEditFocusObject )
UIXTEST_ACCESS( Rob_UIX_FocusedObject,		UIXObject**,			&UIX::mspFocusedSelectionObject )
UIXTEST_ACCESS( Rob_UIX_ModalObject,		UIXObject**,			&UIX::mspModalObject )
UIXTEST_ACCESS( Rob_UIX_TooltipText,		std::string*,			&UIX::msTooltipText )
UIXTEST_ACCESS( Rob_UIX_TooltipPriority,	int*,					&UIX::msnTooltipPriority )
UIXTEST_ACCESS( Rob_UIX_IconsList,			UIXTestIconArray*,		&UIX::mshUIXIconsList )

// UIXObject
UIXTEST_ACCESS( Rob_Obj_Children,			std::vector<UIXObject*> UIXObject::*,	&UIXObject::mContainsList )
UIXTEST_ACCESS( Rob_Obj_LastDisplayRect,	UIXRECT UIXObject::*,					&UIXObject::mLastRenderDisplayRect )
UIXTEST_ACCESS( Rob_Obj_BeingDragged,		BOOL UIXObject::*,						&UIXObject::mbIsBeingDragged )
UIXTEST_ACCESS( Rob_Obj_ContentsHeight,	int UIXObject::*,						&UIXObject::mChildContentsHeight )
UIXTEST_ACCESS( Rob_Obj_GetActualRect,		UIXTestGetRectMethod,					&UIXObject::GetActualRenderRect )

// Widgets
UIXTEST_ACCESS( Rob_Button_Title,			std::string UIXButton::*,			&UIXButton::mTitle )
UIXTEST_ACCESS( Rob_Text_Text,				std::string UIXText::*,				&UIXText::mText )
UIXTEST_ACCESS( Rob_Shape_AnimTime,			float UIXShape::*,					&UIXShape::mfAnimationTime )
UIXTEST_ACCESS( Rob_Shape_AnimPhase,		float UIXShape::*,					&UIXShape::mfAnimationPhase )
UIXTEST_ACCESS( Rob_Dropdown_Expanded,		BOOL UIXDropdown::*,				&UIXDropdown::mbIsExpanded )
UIXTEST_ACCESS( Rob_Dropdown_Selected,		int UIXDropdown::*,					&UIXDropdown::mnSelectedIndex )
UIXTEST_ACCESS( Rob_Dropdown_Scrollbar,		UIXScrollbar* UIXDropdown::*,		&UIXDropdown::mpScrollbar )
UIXTEST_ACCESS( Rob_Section_Collapsed,		BOOL UIXCollapsableSection::*,		&UIXCollapsableSection::mbIsCollapsed )
UIXTEST_ACCESS( Rob_Slider_Current,			float UIXSlider::*,					&UIXSlider::mfCurrentVal )
UIXTEST_ACCESS( Rob_Slider_Min,				float UIXSlider::*,					&UIXSlider::mfMinVal )
UIXTEST_ACCESS( Rob_Slider_Max,				float UIXSlider::*,					&UIXSlider::mfMaxVal )
UIXTEST_ACCESS( Rob_Slider_RenderRect,		UIXRECT UIXSlider::*,				&UIXSlider::mRenderRect )
UIXTEST_ACCESS( Rob_Slider_ValueBox,		UIXTextBox* UIXSlider::*,			&UIXSlider::mpValueTextBox )
UIXTEST_ACCESS( Rob_Slider_MinBox,			UIXTextBox* UIXSlider::*,			&UIXSlider::mpRangeMinTextBox )
UIXTEST_ACCESS( Rob_Slider_MaxBox,			UIXTextBox* UIXSlider::*,			&UIXSlider::mpRangeMaxTextBox )
UIXTEST_ACCESS( Rob_Dial_Current,			float UIXDial::*,					&UIXDial::mfCurrentVal )
UIXTEST_ACCESS( Rob_HScroll_Position,		int UIXHorizScrollbar::*,			&UIXHorizScrollbar::mnScrollPosition )
UIXTEST_ACCESS( Rob_TabBar_List,			std::vector<UIXTabBarEntry*> UIXTabBar::*,	&UIXTabBar::mTabList )
UIXTEST_ACCESS( Rob_TabEntry_Selected,		bool UIXTabBarEntry::*,				&UIXTabBarEntry::mbSelected )
UIXTEST_ACCESS( Rob_ListBox_Flat,			std::vector<UIXListBoxEntry*> UIXListBox::*,	&UIXListBox::mListBoxEntriesFlatList )
UIXTEST_ACCESS( Rob_ListEntry_LastRender,	UIXRECT UIXListBoxEntry::*,			&UIXListBoxEntry::mLastRender )
UIXTEST_ACCESS( Rob_Scrollable_BarRect,		UIXRECT UIXScrollableSection::*,	&UIXScrollableSection::mScrollbarLastRender )

//------------------------------------------------------------------------------------------------
std::vector<UIXPage*>&	UIXTestPages( void )
{
	return( *UIXTestGet( Rob_UIX_PagesList() ) );
}

UIXObject*	UIXTestMousewheelHoverObject( void )
{
	return( *UIXTestGet( Rob_UIX_WheelHover() ) );
}

UIXObject*	UIXTestDragDestinationHover( void )
{
	return( UIX::GetDragDestinationHover() );
}

int		UIXTestDragItemType( void )
{
	return( UIX::GetDragItemType() );
}

const char*	UIXTestTooltipText( void )
{
	return( UIXTestGet( Rob_UIX_TooltipText() )->c_str() );
}

int		UIXTestIconSheetHandle( int nSheet )
{
	return( ( *UIXTestGet( Rob_UIX_IconsList() ) )[nSheet] );
}

void	UIXTestResetUIXStatics( void )
{
	UIX::HoverAcceptDragItem( NULL );
	UIX::SetDragText( "" );
	UIX::SetUISelectionMode( FALSE );
	UIX::RegisterObjectSelectionHandler( NULL );
	UIX::RegisterDefaultMousewheelHandler( NULL );
	*UIXTestGet( Rob_UIX_WheelHover() ) = NULL;
	*UIXTestGet( Rob_UIX_DragSource() ) = NULL;
	*UIXTestGet( Rob_UIX_DragItemType() ) = 0;
	*UIXTestGet( Rob_UIX_DragSourceParam() ) = 0;
	*UIXTestGet( Rob_UIX_TextEditFocus() ) = NULL;
	*UIXTestGet( Rob_UIX_FocusedObject() ) = NULL;
	*UIXTestGet( Rob_UIX_ModalObject() ) = NULL;
	*UIXTestGet( Rob_UIX_TooltipText() ) = "";
	*UIXTestGet( Rob_UIX_TooltipPriority() ) = NOTFOUND;
}

//------------------------------------------------------------------------------------------------
std::vector<UIXObject*>&	UIXTestChildren( UIXObject* pObject )
{
	return( pObject->*UIXTestGet( Rob_Obj_Children() ) );
}

UIXRECT		UIXTestLastDisplayRect( UIXObject* pObject )
{
	return( pObject->*UIXTestGet( Rob_Obj_LastDisplayRect() ) );
}

void		UIXTestSetLastDisplayRect( UIXObject* pObject, UIXRECT xRect )
{
	pObject->*UIXTestGet( Rob_Obj_LastDisplayRect() ) = xRect;
}

UIXRECT		UIXTestActualRect( UIXObject* pObject )
{
	return( ( pObject->*UIXTestGet( Rob_Obj_GetActualRect() ) )( UIXTestLastDisplayRect( pObject ) ) );
}

BOOL		UIXTestIsBeingDragged( UIXObject* pObject )
{
	return( pObject->*UIXTestGet( Rob_Obj_BeingDragged() ) );
}

//------------------------------------------------------------------------------------------------
const char*	UIXTestButtonTitle( UIXButton* pButton )
{
	return( ( pButton->*UIXTestGet( Rob_Button_Title() ) ).c_str() );
}

const char*	UIXTestTextString( UIXText* pText )
{
	return( ( pText->*UIXTestGet( Rob_Text_Text() ) ).c_str() );
}

void		UIXTestShapeAnimation( UIXShape* pShape, float* pfTime, float* pfPhase )
{
	*pfTime = pShape->*UIXTestGet( Rob_Shape_AnimTime() );
	*pfPhase = pShape->*UIXTestGet( Rob_Shape_AnimPhase() );
}

BOOL		UIXTestDropdownIsExpanded( UIXDropdown* pDropdown )
{
	return( pDropdown->*UIXTestGet( Rob_Dropdown_Expanded() ) );
}

int			UIXTestDropdownSelectedIndex( UIXDropdown* pDropdown )
{
	return( pDropdown->*UIXTestGet( Rob_Dropdown_Selected() ) );
}

int			UIXTestDropdownScrollPosition( UIXDropdown* pDropdown )
{
	return( ( pDropdown->*UIXTestGet( Rob_Dropdown_Scrollbar() ) )->GetScrollPosition() );
}

BOOL		UIXTestIsCollapsed( UIXCollapsableSection* pSection )
{
	return( pSection->*UIXTestGet( Rob_Section_Collapsed() ) );
}

void		UIXTestSliderValues( UIXSlider* pSlider, float* pfCurrent, float* pfMin, float* pfMax )
{
	*pfCurrent = pSlider->*UIXTestGet( Rob_Slider_Current() );
	*pfMin = pSlider->*UIXTestGet( Rob_Slider_Min() );
	*pfMax = pSlider->*UIXTestGet( Rob_Slider_Max() );
}

UIXRECT		UIXTestSliderRenderRect( UIXSlider* pSlider )
{
	return( pSlider->*UIXTestGet( Rob_Slider_RenderRect() ) );
}

UIXTextBox*	UIXTestSliderTextBox( UIXSlider* pSlider, int nWhich )
{
	switch( nWhich )
	{
	case 0:
		return( pSlider->*UIXTestGet( Rob_Slider_ValueBox() ) );
	case 1:
		return( pSlider->*UIXTestGet( Rob_Slider_MinBox() ) );
	default:
		return( pSlider->*UIXTestGet( Rob_Slider_MaxBox() ) );
	}
}

float		UIXTestDialValue( UIXDial* pDial )
{
	return( pDial->*UIXTestGet( Rob_Dial_Current() ) );
}

int			UIXTestHorizScrollPosition( UIXHorizScrollbar* pScrollbar )
{
	return( pScrollbar->*UIXTestGet( Rob_HScroll_Position() ) );
}

BOOL		UIXTestTabIsSelected( UIXTabBar* pTabBar, int nIndex )
{
std::vector<UIXTabBarEntry*>&	tabs = pTabBar->*UIXTestGet( Rob_TabBar_List() );

	if ( ( nIndex < 0 ) || ( nIndex >= (int)tabs.size() ) )
	{
		return( FALSE );
	}
	return( ( tabs[nIndex]->*UIXTestGet( Rob_TabEntry_Selected() ) ) ? TRUE : FALSE );
}

UIXRECT		UIXTestListBoxEntryRect( UIXListBox* pListBox, int nFlatIndex )
{
std::vector<UIXListBoxEntry*>&	entries = pListBox->*UIXTestGet( Rob_ListBox_Flat() );

	if ( ( nFlatIndex < 0 ) || ( nFlatIndex >= (int)entries.size() ) )
	{
		return( UIXRECT() );
	}
	return( entries[nFlatIndex]->*UIXTestGet( Rob_ListEntry_LastRender() ) );
}

UIXRECT		UIXTestScrollableSectionBarRect( UIXScrollableSection* pSection )
{
	return( pSection->*UIXTestGet( Rob_Scrollable_BarRect() ) );
}

int			UIXTestChildContentsHeight( UIXObject* pObject )
{
	return( pObject->*UIXTestGet( Rob_Obj_ContentsHeight() ) );
}
