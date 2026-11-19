//------------------------------------------------------------------------------------------------
// UIXTests_System.cpp
//
// What UIX does across objects: drag and drop, tooltips, the object selection mode (Elemental's MIDI
// mapping), keys, the IDs and user params, deleting and resetting.
//------------------------------------------------------------------------------------------------

#include "UIXTestCore.h"

#define		DRAGTYPE_TEST		7

static void	TestDragReceive( UIXObject* pxSourceObject, uint32 ulDragParam, UIXObject* pxDestObject, uint32 ulDragDestParam, const char* szDragDropFilename )
{
	UIXTestLog( "drag received: source #%d param %u -> dest #%d dest param %u", UIXTestIndexOf( pxSourceObject ), ulDragParam, UIXTestIndexOf( pxDestObject ), ulDragDestParam );
}

//------------------------------------------------------------------------------------------------
// Drag and drop
//------------------------------------------------------------------------------------------------
UIX_TEST( DragDrop_ButtonsSectionsAndListToTargets )
{
UIXObject*		pPage = UIX::AddPage( UIXRECT( 0, 0, 640, 480 ), "Drag" );
int				nButtonID = UIXTestRegisterIDHandler();
UIXButton*		pPlainSource = UIX::AddButton( pPage, UIXRECT( 20, 20, 120, 24 ), UIXBUTTON_PLAIN_RECT, "Plain source", nButtonID, 1, FALSE, 0xFF404060 );
UIXButton*		pNormalSource = UIX::AddButton( pPage, UIXRECT( 20, 60, 120, 24 ), UIXBUTTON_NORMAL, "Normal source", nButtonID, 2, FALSE );
UIXCollapsableSection*	pSection = UIX::AddCollapsableSection( pPage, UIXRECT( 20, 100, 200, 0 ), UIXRECT( 0, 0, 0, 20 ), 0, "Draggable section", FALSE, DRAGTYPE_TEST );
UIXListBox*		pList = UIX::AddListBox( pPage, UIXRECT( 20, 160, 150, 100 ), 0, TRUE, DRAGTYPE_TEST );
UIXShape*		pTarget = UIX::AddShape( pPage, UIXRECT( 350, 20, 150, 100 ), UIXSHAPE_SHADEDRECT, FALSE, 0xFF203020, 0xFF102010 );
UIXRECT			xTarget;

	pPlainSource->SetDraggable( DRAGTYPE_TEST, 11 );
	pNormalSource->SetDraggable( DRAGTYPE_TEST, 12 );
	pList->AddElement( "Drag entry 0", 30 );
	pList->AddElement( "Drag entry 1", 31 );
	pTarget->SetDragReceiveCallback( DRAGTYPE_TEST, TestDragReceive, 99 );

	UIXTestFrame( 2 );
	UIXTestLogLayout( NULL, "drag" );
	xTarget = UIXTestRectOf( pTarget );

	UIXTestLog( "drag the plain-rect button onto the target:" );
	UIXTestMouseMove( 60, 32 );
	UIXTestFrame( 1 );
	UIXTestMouseDown();
	UIXTestFrame( 1 );
	UIXTestLog( "after press: drag type %d, source dragging %d", UIXTestDragItemType(), UIXTestIsBeingDragged( pPlainSource ) );
	UIXTestMouseMove( 200, 50 );
	UIXTestFrame( 1 );
	UIXTestMouseMove( xTarget.x + 50, xTarget.y + 50 );
	UIXTestFrame( 1 );
	UIXTestLog( "over the target: destination hover #%d", UIXTestIndexOf( UIXTestDragDestinationHover() ) );
	UIXTestSnapshot( "dragging", 0, 0, 520, 280 );
	UIXTestMouseUp();
	UIXTestFrame( 1 );
	UIXTestLog( "after release: drag type %d, source dragging %d", UIXTestDragItemType(), UIXTestIsBeingDragged( pPlainSource ) );

	UIXTestLog( "drag the plain-rect button and drop it on empty space:" );
	UIXTestDrag( 60, 32, 250, 400 );

	UIXTestLog( "drag the normal button onto the target:" );
	UIXTestDrag( 60, 72, xTarget.x + 50, xTarget.y + 50 );

	UIXTestLog( "drag the section header onto the target:" );
	UIXTestDrag( UIXTestRectOf( pSection ).x + 80, UIXTestRectOf( pSection ).y + 10, xTarget.x + 50, xTarget.y + 50 );
	UIXTestLog( "section collapsed %d", UIXTestIsCollapsed( pSection ) );

	UIXTestLog( "drag list entry 1 onto the target:" );
	UIXTestDrag( UIXTestListBoxEntryRect( pList, 1 ).x + 30, UIXTestListBoxEntryRect( pList, 1 ).y + 9, xTarget.x + 50, xTarget.y + 50 );

	UIXTestLog( "drag with a 2 pixel move (not counted as a drag move) onto the source itself:" );
	UIXTestDrag( 60, 32, 62, 32 );
}

//------------------------------------------------------------------------------------------------
// Tooltips: shown after the mouse rests on an object for a second
//------------------------------------------------------------------------------------------------
static const char*	TestTooltip( UIXObject* pObj, uint32 ulParam )
{
	return( ( ulParam == 1 ) ? "A helpful tooltip" : NULL );
}

UIX_TEST( Tooltip_AfterHoverDelay )
{
UIXObject*		pPage = UIX::AddPage( UIXRECT( 10, 10, 400, 300 ), "Tooltips" );
UIXButton*		pButton = UIX::AddButton( pPage, UIXRECT( 50, 50, 140, 24 ), UIXBUTTON_NORMAL, "Has a tooltip", UIXTestRegisterIDHandler(), 1 );
UIXButton*		pNone = UIX::AddButton( pPage, UIXRECT( 50, 20, 140, 24 ), UIXBUTTON_NORMAL, "Tooltip is NULL", UIXTestRegisterIDHandler(), 2 );

	pButton->SetTooltipCallback( TestTooltip, 1 );
	pNone->SetTooltipCallback( TestTooltip, 2 );
	UIXTestFrame( 2 );

	UIXTestMouseMove( UIXTestRectOf( pButton ).x + 30, UIXTestRectOf( pButton ).y + 12 );
	UIXTestFrame( 30 );
	UIXTestLog( "after half a second: tooltip text '%s'", UIXTestTooltipText() );
	UIXTestSnapshot( "half_second", 0, 0, 300, 160 );
	UIXTestFrame( 40 );
	UIXTestLog( "after over a second: tooltip text '%s'", UIXTestTooltipText() );
	UIXTestSnapshot( "shown", 0, 0, 300, 160 );

	UIXTestMouseMove( UIXTestRectOf( pNone ).x + 30, UIXTestRectOf( pNone ).y + 12 );
	UIXTestFrame( 80 );
	UIXTestSnapshot( "null_tooltip", 0, 0, 300, 160 );
}

//------------------------------------------------------------------------------------------------
// Object selection mode: with it on, objects that have a UID param draw an overlay and a click selects
// them instead of using them
//------------------------------------------------------------------------------------------------
static void	TestObjectSelection( UIXObject* pxSelectedObject )
{
	UIXTestLog( "object selection handler #%d uid %u", UIXTestIndexOf( pxSelectedObject ), (uint32)pxSelectedObject->GetUIDParam() );
}

UIX_TEST( SelectionMode_SelectsObjectsWithUIDs )
{
UIXObject*		pPage = UIX::AddPage( UIXRECT( 10, 10, 500, 300 ), "Selection" );
int				nButtonID = UIXTestRegisterIDHandler();
UIXButton*		pMapped = UIX::AddButton( pPage, UIXRECT( 20, 20, 140, 24 ), UIXBUTTON_NORMAL, "Mapped", nButtonID, 1 );
UIXButton*		pTexted = UIX::AddButton( pPage, UIXRECT( 20, 10, 140, 24 ), UIXBUTTON_NORMAL, "Mapped, with text", nButtonID, 2 );
UIXButton*		pUnmapped = UIX::AddButton( pPage, UIXRECT( 20, 10, 140, 24 ), UIXBUTTON_NORMAL, "No UID", nButtonID, 3 );
UIXSlider*		pSlider = UIX::AddSlider( pPage, UIXRECT( 20, 10, 300, 18 ), SLIDERMODE_VALUE, 1, 0.0f, 1.0f, 0.5f, 0.1f, "Slider", FALSE );
UIXDial*		pDial = UIX::AddDial( pPage, UIXRECT( 340, 20, 50, 50 ), 0, 0.5f );

	pMapped->SetUIDParam( 1001 );
	pTexted->SetUIDParam( 1002 );
	pTexted->SetObjectSelectionText( "CC 12" );
	pSlider->SetUIDParam( 1003 );
	pDial->SetUIDParam( 1004 );
	UIX::RegisterObjectSelectionHandler( TestObjectSelection );

	UIXTestFrame( 2 );
	UIXTestSnapshot( "off", 0, 0, 420, 160 );
	UIX::SetUISelectionMode( TRUE );
	UIXTestMouseMove( 450, 280 );
	UIXTestFrame( 2 );
	UIXTestSnapshot( "on", 0, 0, 420, 160 );

	UIXTestLog( "click each in selection mode:" );
	UIXTestClick( UIXTestRectOf( pMapped ).x + 20, UIXTestRectOf( pMapped ).y + 12 );
	UIXTestClick( UIXTestRectOf( pTexted ).x + 20, UIXTestRectOf( pTexted ).y + 12 );
	UIXTestClick( UIXTestRectOf( pUnmapped ).x + 20, UIXTestRectOf( pUnmapped ).y + 12 );
	UIXTestClick( UIXTestSliderRenderRect( pSlider ).x + 20, UIXTestSliderRenderRect( pSlider ).y + 9 );
	UIXTestClick( UIXTestRectOf( pDial ).x + 25, UIXTestRectOf( pDial ).y + 25 );
	UIXTestLog( "focused #%d", UIXTestIndexOf( UIX::GetFocusedObject() ) );
	UIXTestMouseMove( 450, 280 );
	UIXTestSnapshot( "dial_focused", 0, 0, 420, 160 );

	UIXTestLog( "SetSelectedUIXObjectID of the first button:" );
	UIX::SetSelectedUIXObjectID( pMapped->GetID() );
	UIXTestLog( "focused #%d", UIXTestIndexOf( UIX::GetFocusedObject() ) );
	UIX::SetUISelectionMode( FALSE );
	UIXTestLog( "click the first button with selection mode off:" );
	UIXTestClick( UIXTestRectOf( pMapped ).x + 20, UIXTestRectOf( pMapped ).y + 12 );
}

//------------------------------------------------------------------------------------------------
// IDs, user params, the press ID during a callback
//------------------------------------------------------------------------------------------------
static void	TestPressIDHandler( int nButtonID, uint32 ulParam, uint32 ulIDParam )
{
	UIXTestLog( "handler: param %u, GetCurrentPressObjectID is object #%d", ulParam, UIXTestIndexOf( UIX::FindUIXObjectByID( UIX::GetCurrentPressObjectID() ) ) );
}

class TestUserObject
{
public:
	~TestUserObject() { msnDeleted++; }
	static int	msnDeleted;
};
int		TestUserObject::msnDeleted = 0;

UIX_TEST( Object_IDsAndUserParams )
{
UIXObject*		pPage = UIX::AddPage( UIXRECT( 10, 10, 300, 200 ), "IDs" );
UIXButton*		pButton;
UIXButton*		pOwner;
int				nButtonID = 0x7100;

	UIRegisterButtonPressHandler( nButtonID, TestPressIDHandler );
	pButton = UIX::AddButton( pPage, UIXRECT( 10, 10, 120, 22 ), UIXBUTTON_PLAIN_RECT, "IDs", nButtonID, 42, TRUE, 0xFF404040 );
	pOwner = UIX::AddButton( pPage, UIXRECT( 10, 10, 120, 22 ), UIXBUTTON_PLAIN_RECT, "Owner", nButtonID, 43, TRUE, 0xFF404040 );

	UIX_CHECK( pButton->GetID() > pPage->GetID(), "Object: IDs increase as objects are added" );
	UIX_CHECK( pPage->DoesContainObjectID( pButton->GetID() ), "Object: DoesContainObjectID finds a child" );
	UIX_CHECK( !pButton->DoesContainObjectID( pPage->GetID() ), "Object: DoesContainObjectID doesn't look upwards" );
	UIX_CHECK( pButton->GetParent() == pPage, "Object: GetParent is the container it was added to" );
	UIX_CHECK( pPage->HasChildren() && !pButton->HasChildren(), "Object: HasChildren" );

	UIX_CHECK( !pButton->DoesHaveUserParamEx( "colour" ), "Object: no user param before it is set" );
	pButton->SetUserParamEx( "colour", 5 );
	UIX_CHECK( pButton->DoesHaveUserParamEx( "colour" ) && ( pButton->GetUserParamEx( "colour" ) == 5 ), "Object: user params keep their value" );
	pButton->SetUIDParam( 0x123456789ULL );
	UIX_CHECK( pButton->GetUIDParam() == 0x123456789ULL, "Object: UID param keeps all 64 bits" );

	TestUserObject::msnDeleted = 0;
	pOwner->SetUserObject( new TestUserObject, true );
	pButton->SetUserObject( new TestUserObject, false );
	UIXTestFrame( 2 );
	UIXTestClick( UIXTestRectOf( pButton ).x + 20, UIXTestRectOf( pButton ).y + 10 );
	delete (TestUserObject*)pButton->GetUserObject();
	TestUserObject::msnDeleted = 0;
	UIX::DeleteObject( pPage );
	// ~UIXObject frees a delete-on-destroy user object through a void*, so its destructor doesn't run
	UIXTestLog( "user object destructors run when its owner was deleted: %d", TestUserObject::msnDeleted );
}

//------------------------------------------------------------------------------------------------
// Escape, CloseAllDropdowns, deleting focused objects, Reset
//------------------------------------------------------------------------------------------------
UIX_TEST( System_EscapeAndCloseAll )
{
UIXObject*		pPage = UIX::AddPage( UIXRECT( 10, 10, 400, 300 ), "Escape" );
UIXDropdown*	pFirst = UIX::AddDropdown( pPage, UIXRECT( 20, 20, 150, 16 ) );
UIXDropdown*	pSecond = UIX::AddDropdown( pPage, UIXRECT( 200, 20, 150, 16 ) );
UIXObject*		pOtherPage = UIX::AddPage( UIXRECT( 10, 300, 400, 100 ), "Other page" );
UIXDropdown*	pThird = UIX::AddDropdown( pOtherPage, UIXRECT( 20, 20, 150, 16 ) );

	pFirst->AddElement( "One", 1 );
	pSecond->AddElement( "Two", 2 );
	pThird->AddElement( "Three", 3 );
	UIXTestFrame( 2 );

	UIXTestClick( UIXTestRectOf( pThird ).x + 20, UIXTestRectOf( pThird ).y + 8 );
	UIXTestLog( "third open: %d", UIXTestDropdownIsExpanded( pThird ) );
	UIXTestKey( KEY_ESCAPE );
	UIXTestLog( "after escape, third open: %d", UIXTestDropdownIsExpanded( pThird ) );

	UIXTestClick( UIXTestRectOf( pFirst ).x + 20, UIXTestRectOf( pFirst ).y + 8 );
	UIX::CloseAllDropdowns( pFirst->GetID() );
	UIXTestLog( "CloseAllDropdowns except the first: first open %d", UIXTestDropdownIsExpanded( pFirst ) );
	UIX::CloseAllDropdowns( 0 );
	UIXTestLog( "CloseAllDropdowns: first open %d", UIXTestDropdownIsExpanded( pFirst ) );
}

UIX_TEST( System_DeleteFocusedObjects )
{
UIXObject*		pPage = UIX::AddPage( UIXRECT( 10, 10, 400, 300 ), "Delete" );
UIXTextBox*		pText = UIX::AddTextBox( pPage, UIXRECT( 20, 20, 200, 20 ), 0, "Editing" );
UIXDropdown*	pDropdown = UIX::AddDropdown( pPage, UIXRECT( 20, 60, 150, 16 ) );
UIXObject*		pWheelPage = UIX::AddPage( UIXRECT( 10, 320, 400, 100 ), "Wheel" );
UIXScrollableSection*	pScroll = UIX::AddScrollableSection( pWheelPage, UIXRECT( 0, 0, 400, 100 ) );
int				nLoop;

	pDropdown->AddElement( "Item", 1 );
	for ( nLoop = 0; nLoop < 10; nLoop++ )
	{
		UIX::AddShape( pScroll, UIXRECT( 0, 0, 200, 20 ), UIXSHAPE_SHADEDRECT, TRUE, 0xFF404040, 0xFF303030 );
	}
	UIXTestFrame( 2 );
	UIXTestMouseMove( 100, 350 );
	UIXTestFrame( 2 );
	UIXTestLog( "wheel hover #%d", UIXTestIndexOf( UIXTestMousewheelHoverObject() ) );

	UIXTestClick( UIXTestRectOf( pText ).x + 20, UIXTestRectOf( pText ).y + 10 );
	UIXTestRightClick( UIXTestRectOf( pDropdown ).x + 20, UIXTestRectOf( pDropdown ).y + 8 );
	UIXTestLog( "before delete: edit focus #%d, focused #%d, text input active %d", UIXTestIndexOf( UIX::GetTextEditFocus() ), UIXTestIndexOf( UIX::GetFocusedObject() ), UIX::IsTextInputActive() );
	UIX::DeleteObject( pPage );
	UIXTestLog( "after deleting the page: edit focus %s, focused %s, text input active %d", UIX::GetTextEditFocus() ? "set" : "NULL", UIX::GetFocusedObject() ? "set" : "NULL", UIX::IsTextInputActive() );
	// The keyboard handler still points at the deleted text box, so typing now would use freed memory. Don't.
	PlatformKeyboardRegisterHandler( NULL, NULL );

	UIX::DeleteObject( pWheelPage );
	UIXTestLog( "after deleting the scroll section's page: wheel hover %s", UIXTestMousewheelHoverObject() ? "set" : "NULL" );
	UIXTestFrame( 1 );
	(void)pScroll;
}

UIX_TEST( System_DeleteChildObject )
{
UIXObject*		pPage = UIX::AddPage( UIXRECT( 10, 10, 300, 200 ), "Parent" );
UIXButton*		pKeep = UIX::AddButton( pPage, UIXRECT( 10, 10, 120, 22 ), UIXBUTTON_NORMAL, "Kept", UIXTestRegisterIDHandler(), 1 );
UIXButton*		pDelete = UIX::AddButton( pPage, UIXRECT( 10, 10, 120, 22 ), UIXBUTTON_NORMAL, "Deleted", UIXTestRegisterIDHandler(), 2 );
uint32			ulDeletedID = pDelete->GetID();
std::vector<UIXObject*>&	children = UIXTestChildren( pPage );
UIXObject*		pDeletedPointer = pDelete;
BOOL			bStillListed;

	UIXTestFrame( 2 );
	UIX::DeleteObject( pDelete );
	bStillListed = ( std::find( children.begin(), children.end(), pDeletedPointer ) != children.end() );
	UIXTestLog( "after DeleteObject on a child: in the ID map %d, still in its parent's child list %d", UIX::FindUIXObjectByID( ulDeletedID ) != NULL, bStillListed );

	// DeleteObject doesn't take a child out of its parent's list, so the next render would use freed memory.
	// Take the stale pointer out here (without touching it) so the harness can carry on.
	children.erase( std::remove( children.begin(), children.end(), pDeletedPointer ), children.end() );
	UIXTestFrame( 1 );
	(void)pKeep;
}

UIX_TEST( System_ResetWithTwoPages )
{
UIXObject*		pFirst = UIX::AddPage( UIXRECT( 10, 10, 100, 100 ), "First" );
UIXObject*		pSecond = UIX::AddPage( UIXRECT( 200, 10, 100, 100 ), "Second" );
UIXButton*		pFirstButton = UIX::AddButton( pFirst, UIXRECT( 0, 0, 80, 20 ), UIXBUTTON_NORMAL, "A", UIXTestRegisterIDHandler(), 1 );
UIXButton*		pSecondButton = UIX::AddButton( pSecond, UIXRECT( 0, 0, 80, 20 ), UIXBUTTON_NORMAL, "B", UIXTestRegisterIDHandler(), 2 );
uint32			ulFirstID = pFirstButton->GetID();
uint32			ulSecondID = pSecondButton->GetID();
UIXObject*		pLeftOver;

	UIXTestFrame( 1 );
	UIX::Reset();
	UIXTestLog( "after Reset: pages %d, first button found %d, second button found %d", (int)UIXTestPages().size(), UIX::FindUIXObjectByID( ulFirstID ) != NULL, UIX::FindUIXObjectByID( ulSecondID ) != NULL );
	UIX_CHECK( UIXTestPages().empty(), "Reset: no pages left in the list" );

	// Reset loops over the page list while DeleteObject erases from it. With two pages the loop reads the
	// stale slot past the new end, which still holds the second page, so both go. With three or more it
	// reaches a page it has already freed, so this test stays at two. If a page is ever left over, delete it
	// here so the harness doesn't carry it into the next test.
	pLeftOver = UIX::FindUIXObjectByID( ulSecondID );
	if ( pLeftOver )
	{
		UIX::DeleteObject( pLeftOver->GetParent() );
	}
	UIXTestFrame( 1 );
}

//------------------------------------------------------------------------------------------------
// Icon sheets: icons 0-35 come from sheet 0 (6x6 cells of 19 pixels); icon 36 and up draw a whole
// sheet (36 = sheet 1), as Elemental's folder icon does
//------------------------------------------------------------------------------------------------
UIX_TEST( Icons_SheetsAndCells )
{
UIXObject*		pPage = UIX::AddPage( UIXRECT( 10, 10, 400, 100 ), "Icons" );
char			acSheet[MAX_PATH];
int				nLoop;

	sprintf( acSheet, "..\Tests\UIXTests\Out\UIXTestIcons.png" );
	UIX::LoadIconSheet( UIXTestInterface(), 1, acSheet );
	for ( nLoop = 0; nLoop < 8; nLoop++ )
	{
		UIX::AddButton( pPage, UIXRECT( 10 + ( nLoop * 34 ), 10, 30, 30 ), UIXBUTTON_RECT_ICON, "", 0, 0, FALSE, 0xFF303030, nLoop * 5 );
	}
	UIX::AddButton( pPage, UIXRECT( 10, 50, 40, 40 ), UIXBUTTON_RECT_ICON, "", 0, 0, FALSE, 0xFF303030, 36 );
	UIX::AddButton( pPage, UIXRECT( 60, 50, 40, 40 ), UIXBUTTON_RECT_ICON, "", 0, 0, FALSE, 0xFF303030, MAX_NUM_UIX_ICONS );
	UIXTestFrame( 2 );
	UIXTestSnapshot( "icons", 0, 0, 300, 110 );
}
