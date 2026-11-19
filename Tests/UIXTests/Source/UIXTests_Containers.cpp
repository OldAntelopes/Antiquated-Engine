//------------------------------------------------------------------------------------------------
// UIXTests_Containers.cpp
//
// Objects that hold or lay out others: sub-pages, collapsable sections (Elemental's main way of building
// its control panel), scrollable sections, tab bars, menu bars, popup menus, modal popups and custom
// render objects.
//------------------------------------------------------------------------------------------------

#include "UIXTestCore.h"

static void	TestSelected( UIXObject* pxSourceObject, uint32 ulSelectParam )
{
	UIXTestLog( "selected #%d param=%u", UIXTestIndexOf( pxSourceObject ), ulSelectParam );
}

//------------------------------------------------------------------------------------------------
// Sub-pages
//------------------------------------------------------------------------------------------------
UIX_TEST( SubPage_NestedLayout )
{
UIXObject*		pPage = UIX::AddPage( UIXRECT( 10, 10, 400, 300 ), "Outer" );
UIXObject*		pSub;
UIXObject*		pClipped;

	UIX::AddShape( pPage, UIXRECT( 0, 0, 400, 300 ), UIXSHAPE_OUTLINEBOX, FALSE, 0xFF808080 );
	UIX::AddText( pPage, UIXRECT( 10, 0, 200, 20 ), 0xFFE0E0E0, 3, NONE, "Before the sub-page" );
	pSub = UIX::AddSubPage( pPage, UIXRECT( 20, 30, 200, 100 ), "Inner" );
	UIX::AddShape( pSub, UIXRECT( 0, 0, 200, 100 ), UIXSHAPE_SHADEDRECT, FALSE, 0xFF304050, 0xFF203040 );
	UIX::AddButton( pSub, UIXRECT( 10, 10, 120, 20 ), UIXBUTTON_NORMAL, "In sub-page", UIXTestRegisterIDHandler(), 1 );
	UIX::AddText( pPage, UIXRECT( 10, 0, 200, 20 ), 0xFFE0E0E0, 3, NONE, "After the sub-page" );
	pClipped = UIX::AddSubPage( pPage, UIXRECT( 240, 30, 100, 60 ), "Clipped inner", TRUE );
	UIX::AddShape( pClipped, UIXRECT( -10, -10, 200, 200 ), UIXSHAPE_SHADEDRECT, FALSE, 0xFF805020, 0xFF402010 );

	UIXTestFrame( 2 );
	UIXTestLogLayout( NULL, "nested" );
	UIXTestSnapshot( "nested", 0, 0, 420, 320 );
}

//------------------------------------------------------------------------------------------------
// Collapsable sections
//------------------------------------------------------------------------------------------------
UIX_TEST( CollapsableSection_ToggleAndLayout )
{
UIXObject*		pPage = UIX::AddPage( UIXRECT( 10, 10, 300, 400 ), "Sections" );
UIXCollapsableSection*	apSections[3];
UIStateData		xState;
int				nLoop;

	apSections[0] = UIX::AddCollapsableSection( pPage, UIXRECT( 0, 0, 280, 0 ), UIXRECT( 0, 0, 0, 20 ), 0, "Mode 0, open", FALSE );
	UIX::AddButton( apSections[0], UIXRECT( 10, 2, 150, 20 ), UIXBUTTON_NORMAL, "Inside 0", UIXTestRegisterIDHandler(), 1 );
	UIX::AddText( apSections[0], UIXRECT( 10, 2, 200, 18 ), 0xFFE0E0E0, 3, NONE, "Text inside 0" );
	apSections[1] = UIX::AddCollapsableSection( pPage, UIXRECT( 0, 0, 280, 0 ), UIXRECT( 0, 0, 0, 20 ), 1, "Mode 1, collapsed", TRUE );
	UIX::AddButton( apSections[1], UIXRECT( 10, 2, 150, 20 ), UIXBUTTON_NORMAL, "Inside 1", UIXTestRegisterIDHandler(), 2 );
	apSections[2] = UIX::AddCollapsableSection( pPage, UIXRECT( 0, 0, 280, 0 ), UIXRECT( 0, 0, 0, 20 ), 2, "Mode 2, not collapsable", FALSE );
	apSections[2]->SetNotCollapsable();
	UIX::AddButton( apSections[2], UIXRECT( 10, 2, 150, 20 ), UIXBUTTON_NORMAL, "Inside 2", UIXTestRegisterIDHandler(), 3 );

	UIXTestFrame( 2 );
	UIXTestLogLayout( NULL, "start" );
	UIXTestSnapshot( "start", 0, 0, 320, 200 );

	for ( nLoop = 0; nLoop < 3; nLoop++ )
	{
	UIXRECT		xRect = UIXTestRectOf( apSections[nLoop] );

		UIXTestLog( "click section %d's header:", nLoop );
		UIXTestClick( xRect.x + 100, xRect.y + 10 );
		xState.mpAssociatedUIXObj = apSections[nLoop];
		xState.OnUpdate( 0.0f );
		UIXTestLog( "section %d collapsed %d, state data says opened %d", nLoop, UIXTestIsCollapsed( apSections[nLoop] ), xState.mbTabOpened );
	}
	UIX_CHECK( UIXTestIsCollapsed( apSections[0] ), "CollapsableSection: clicking an open header collapses it" );
	UIX_CHECK( !UIXTestIsCollapsed( apSections[1] ), "CollapsableSection: clicking a collapsed header opens it" );
	UIXTestLogLayout( NULL, "after toggling" );
	UIXTestMouseMove( 300, 380 );
	UIXTestSnapshot( "toggled", 0, 0, 320, 200 );

	UIXTestLog( "click the button inside section 1:" );
	UIXTestClick( UIXTestRectOf( UIXTestChildren( apSections[1] )[0] ).x + 20, UIXTestRectOf( UIXTestChildren( apSections[1] )[0] ).y + 10 );
}

static void	TestSectionRightClick( UIXObject* pxSourceObject, uint32 ulSelectParam )
{
	UIXTestLog( "section right click #%d param=%u", UIXTestIndexOf( pxSourceObject ), ulSelectParam );
}

UIX_TEST( CollapsableSection_NestedAndRightClick )
{
UIXObject*		pPage = UIX::AddPage( UIXRECT( 10, 10, 300, 400 ), "Nested" );
UIXCollapsableSection*	pOuter = UIX::AddCollapsableSection( pPage, UIXRECT( 0, 0, 280, 0 ), UIXRECT( 0, 0, 0, 20 ), 0, "Outer", FALSE );
UIXCollapsableSection*	pInner = UIX::AddCollapsableSection( pOuter, UIXRECT( 10, 0, -10, 0 ), UIXRECT( 0, 0, 0, 20 ), 1, "Inner", FALSE );
UIXCollapsableSection*	pAfter = UIX::AddCollapsableSection( pPage, UIXRECT( 0, 0, 280, 0 ), UIXRECT( 0, 0, 0, 20 ), 2, "After", FALSE );

	UIX::AddText( pInner, UIXRECT( 10, 2, 200, 18 ), 0xFFE0E0E0, 3, NONE, "Deep text" );
	UIX::AddText( pAfter, UIXRECT( 10, 2, 200, 18 ), 0xFFE0E0E0, 3, NONE, "After text" );
	pOuter->SetRightClickSelectedCallback( TestSectionRightClick, 3 );
	UIXTestFrame( 2 );
	UIXTestLogLayout( NULL, "nested open" );
	UIXTestSnapshot( "open", 0, 0, 320, 160 );

	UIXTestClick( UIXTestRectOf( pInner ).x + 100, UIXTestRectOf( pInner ).y + 10 );
	UIXTestLogLayout( NULL, "inner collapsed" );
	UIXTestClick( UIXTestRectOf( pOuter ).x + 100, UIXTestRectOf( pOuter ).y + 10 );
	UIXTestLogLayout( NULL, "outer collapsed" );
	UIXTestSnapshot( "collapsed", 0, 0, 320, 160 );

	UIXTestLog( "right click the outer header:" );
	UIXTestRightClick( UIXTestRectOf( pOuter ).x + 100, UIXTestRectOf( pOuter ).y + 10 );
	UIXTestLog( "outer collapsed %d", UIXTestIsCollapsed( pOuter ) );
}

//------------------------------------------------------------------------------------------------
// Scrollable sections
//------------------------------------------------------------------------------------------------
UIX_TEST( ScrollableSection_OverflowWheelAndDrag )
{
UIXObject*		pPage = UIX::AddPage( UIXRECT( 20, 20, 300, 200 ), "Scrolling" );
UIXScrollableSection*	pScroll;
UIXScrollbarRestoreState	xSaved;
int				nLoop;
UIXRECT			xBar;

	UIX::AddShape( pPage, UIXRECT( 0, 0, 300, 200 ), UIXSHAPE_OUTLINEBOX, FALSE, 0xFF808080 );
	pScroll = UIX::AddScrollableSection( pPage, UIXRECT( 0, 0, 300, 200 ) );
	for ( nLoop = 0; nLoop < 20; nLoop++ )
	{
		UIX::AddButton( pScroll, UIXRECT( 10, 2, 200, 20 ), UIXBUTTON_NORMAL, "Row", UIXTestRegisterIDHandler(), nLoop );
	}

	// Contents height is only known after a frame, so the scrollbar appears on the second
	UIXTestFrame( 1 );
	UIXTestLog( "after one frame: contents h %d, scroll %d", UIXTestChildContentsHeight( pScroll ), pScroll->GetScrollPosition() );
	UIXTestFrame( 1 );
	UIXTestLogLayout( pScroll, "scroll section" );
	xBar = UIXTestScrollableSectionBarRect( pScroll );
	UIXTestLog( "scrollbar at %d,%d size %d,%d", xBar.x, xBar.y, xBar.w, xBar.h );
	UIXTestSnapshot( "top", 0, 0, 340, 240 );

	UIXTestLog( "wheel down 2 notches over the section:" );
	UIXTestMouseMove( 100, 100 );
	UIXTestFrame( 1 );
	UIXTestWheel( -2 );
	UIXTestLog( "scroll %d", pScroll->GetScrollPosition() );
	UIXTestSnapshot( "wheeled", 0, 0, 340, 240 );
	UIXTestLog( "click what is now under 100,60:" );
	UIXTestClick( 100, 60 );

	UIXTestLog( "wheel up 10 notches (clamps at the top):" );
	UIXTestWheel( 10 );
	UIXTestLog( "scroll %d", pScroll->GetScrollPosition() );
	UIX_CHECK( pScroll->GetScrollPosition() == 0, "ScrollableSection: scrolling up stops at 0" );

	UIXTestLog( "drag the scrollbar down 60 pixels:" );
	xBar = UIXTestScrollableSectionBarRect( pScroll );
	UIXTestDrag( xBar.x + 6, xBar.y + 10, xBar.x + 6, xBar.y + 70 );
	UIXTestLog( "scroll %d", pScroll->GetScrollPosition() );
	UIXTestLog( "drag far past the bottom:" );
	UIXTestDrag( xBar.x + 6, xBar.y + 70, xBar.x + 6, xBar.y + 400 );
	UIXTestLog( "scroll %d", pScroll->GetScrollPosition() );
	UIXTestSnapshot( "bottom", 0, 0, 340, 240 );

	pScroll->StoreScrollState( &xSaved );
	UIXTestWheel( 10 );
	pScroll->RestoreScrollState( &xSaved );
	UIXTestFrame( 1 );
	UIXTestLog( "restored scroll %d", pScroll->GetScrollPosition() );
}

UIX_TEST( ScrollableSection_FitsWithoutScrollbar )
{
UIXObject*		pPage = UIX::AddPage( UIXRECT( 20, 20, 300, 200 ), "Fits" );
UIXScrollableSection*	pScroll = UIX::AddScrollableSection( pPage, UIXRECT( 0, 0, 300, 200 ) );

	UIX::AddButton( pScroll, UIXRECT( 10, 2, 200, 20 ), UIXBUTTON_NORMAL, "Only row", UIXTestRegisterIDHandler(), 1 );
	UIXTestFrame( 2 );
	UIXTestLogLayout( NULL, "fits" );
	UIXTestMouseMove( 100, 100 );
	UIXTestFrame( 1 );
	UIXTestWheel( -3 );
	UIXTestLog( "scroll after wheel %d", pScroll->GetScrollPosition() );
	UIXTestSnapshot( "fits", 0, 0, 340, 240 );
}

//------------------------------------------------------------------------------------------------
// Tab bars
//------------------------------------------------------------------------------------------------
UIX_TEST( TabBar_SelectTabs )
{
UIXObject*		pPage = UIX::AddPage( UIXRECT( 10, 10, 400, 100 ), "Tabs" );
UIXTabBar*		pTabs = UIX::AddTabBar( pPage, UIXRECT( 10, 10, 380, 20 ) );
UIXTabBar*		pPlain = UIX::AddTabBar( pPage, UIXRECT( 10, 10, 380, 20 ) );
UIXTabBarEntry*	apEntries[3];
int				nLoop;
int				nX;

	apEntries[0] = pTabs->AddElement( "General", 100 );
	apEntries[1] = pTabs->AddElement( "Audio", 200 );
	apEntries[2] = pTabs->AddElement( "A longer tab name", 300 );
	apEntries[1]->SetSelected( true );
	pTabs->SetSelectionChangedCallback( TestSelected );
	pPlain->AddElement( "No callback", 1 );

	UIXTestFrame( 2 );
	UIXTestLogLayout( NULL, "tabs" );
	UIXTestSnapshot( "tabs", 0, 0, 420, 80 );

	// Tabs are as wide as their text plus 12, with a pixel between
	nX = UIXTestRectOf( pTabs ).x;
	for ( nLoop = 0; nLoop < 3; nLoop++ )
	{
	int		nWidth = UIXTestInterface()->GetStringWidth( pTabs->GetElementByListIndex( nLoop )->GetText().c_str(), 0 ) + 12;

		UIXTestLog( "click tab %d '%s' (x %d, width %d):", nLoop, pTabs->GetElementByListIndex( nLoop )->GetText().c_str(), nX, nWidth );
		UIXTestClick( nX + ( nWidth / 2 ), UIXTestRectOf( pTabs ).y + 10 );
		UIXTestLog( "tab %d selected flag %d", nLoop, UIXTestTabIsSelected( pTabs, nLoop ) );
		nX += nWidth + 1;
	}
	UIXTestLog( "click the tab bar without a callback:" );
	UIXTestClick( UIXTestRectOf( pPlain ).x + 20, UIXTestRectOf( pPlain ).y + 10 );
}

//------------------------------------------------------------------------------------------------
// Menu bars
//------------------------------------------------------------------------------------------------
UIX_TEST( Menu_OpenChooseAndClose )
{
UIXObject*		pPage = UIX::AddPage( UIXRECT( 0, 0, 640, 480 ), "Menus" );
UIXMenu*		pMenu = UIX::AddMenuBar( pPage, UIXRECT( 0, 0, 640, 24 ) );
UIXMenuItem*	pFile = pMenu->AddMenuItem( "File" );
UIXMenuItem*	pView = pMenu->AddMenuItem( "View", 160 );
UIXMenuItem*	pChecked;
UIXRECT			xRect;

	pFile->AddMenuItem( "New", TestSelected, 1 );
	pFile->AddMenuItem( "Open", TestSelected, 2 );
	pFile->AddMenuItem( "Greyed (no callback)" );
	pChecked = pView->AddMenuItem( "Checked item", TestSelected, 3 );
	pChecked->SetChecked( TRUE );
	pView->AddMenuItem( "Another", TestSelected, 4 );
	pMenu->AddMenuItem( "Empty" );

	UIXTestFrame( 2 );
	UIXTestSnapshot( "closed", 0, 0, 300, 120 );
	xRect = pFile->GetLastRenderRect();
	UIXTestLog( "File at %d,%d size %d,%d; View at %d,%d size %d,%d", xRect.x, xRect.y, xRect.w, xRect.h, pView->GetLastRenderRect().x, pView->GetLastRenderRect().y, pView->GetLastRenderRect().w, pView->GetLastRenderRect().h );

	UIXTestLog( "click File:" );
	UIXTestClick( xRect.x + 5, xRect.y + 5 );
	UIXTestLog( "File expanded %d", pFile->IsExpanded() );
	UIX_CHECK( pFile->IsExpanded(), "Menu: clicking a top item opens it" );
	UIXTestMouseMove( xRect.x + 30, xRect.y + xRect.h + 4 + 18 + 8 );
	UIXTestSnapshot( "file_open", 0, 0, 300, 120 );

	// Items: 16 high, every 18, starting 4 below the bar item
	UIXTestLog( "click Open:" );
	UIXTestClick( xRect.x + 30, xRect.y + xRect.h + 4 + 18 + 8 );
	UIXTestLog( "File expanded %d after choosing an item", pFile->IsExpanded() );
	UIXTestLog( "click the greyed item:" );
	UIXTestClick( xRect.x + 30, xRect.y + xRect.h + 4 + 36 + 8 );

	UIXTestLog( "click View while File is open:" );
	UIXTestClick( pView->GetLastRenderRect().x + 5, pView->GetLastRenderRect().y + 5 );
	UIXTestLog( "File expanded %d, View expanded %d", pFile->IsExpanded(), pView->IsExpanded() );
	UIXTestSnapshot( "view_open", 0, 0, 300, 120 );

	UIXTestLog( "escape:" );
	UIXTestKey( KEY_ESCAPE );
	UIXTestLog( "View expanded %d", pView->IsExpanded() );

	UIXTestLog( "open View, then UIX::CloseAllMenus:" );
	UIXTestClick( pView->GetLastRenderRect().x + 5, pView->GetLastRenderRect().y + 5 );
	UIX::CloseAllMenus();
	UIXTestFrame( 1 );
	UIXTestLog( "View expanded %d", pView->IsExpanded() );
	UIXTestLog( "click the Empty top item:" );
	UIXTestClick( UIXTestRectOf( pMenu ).x + 120, UIXTestRectOf( pMenu ).y + 8 );
}

//------------------------------------------------------------------------------------------------
// Popup menus
//------------------------------------------------------------------------------------------------
static void	TestPopupRightClick( UIXObject* pxSourceObject, uint32 ulSelectParam )
{
	UIXTestLog( "popup item right click #%d param=%u", UIXTestIndexOf( pxSourceObject ), ulSelectParam );
}

UIX_TEST( PopupMenu_ItemsSubmenuAndChoose )
{
UIXObject*		pPage = UIX::AddPage( UIXRECT( 0, 0, 640, 480 ), "Popup" );
UIXPopupMenu*	pPopup = UIX::AddPopupMenu( pPage, UIXRECT( 100, 50, 180, 160 ) );
UIXPopupMenuItem*	pFirst;
UIXPopupMenuItem*	pSub;
UIXPopupMenuItem*	pCheck;
UIXRECT			xRect;

	pPopup->AddHeaderMenuItem( "Header %d", 1 );
	pFirst = pPopup->AddMenuItem( "First", TestSelected, 1 );
	pFirst->SetRightClickSelectedCallback( TestPopupRightClick, 11 );
	pPopup->AddMenuItem( "-" );
	pCheck = pPopup->AddMenuItem( "Checkbox item", TestSelected, 2, TRUE, TRUE );
	pPopup->AddMenuItem( "Greyed" );
	pSub = pPopup->AddMenuItem( "Submenu" );
	pSub->AddMenuItem( "Sub one", TestSelected, 3 );
	pSub->AddMenuItem( "Sub two", TestSelected, 4 );

	UIXTestFrame( 2 );
	UIXTestSnapshot( "closed", 0, 0, 480, 260 );
	{
	size_t		nLoop;

		for ( nLoop = 0; nLoop < UIXTestChildren( pPopup ).size(); nLoop++ )
		{
		UIXPopupMenuItem*	pItem = (UIXPopupMenuItem*)UIXTestChildren( pPopup )[nLoop];

			xRect = pItem->GetLastRenderRect();
			UIXTestLog( "item '%s' at %d,%d size %d,%d header %d checkbox %d checked %d", pItem->GetText(), xRect.x, xRect.y, xRect.w, xRect.h, pItem->IsHeader(), pItem->IsCheckbox(), pItem->IsChecked() );
		}
	}

	UIXTestLog( "click First:" );
	xRect = pFirst->GetLastRenderRect();
	UIXTestClick( xRect.x + 20, xRect.y + 8 );
	UIXTestLog( "right click First:" );
	UIXTestRightClick( xRect.x + 20, xRect.y + 8 );
	UIXTestLog( "click the checkbox item:" );
	xRect = pCheck->GetLastRenderRect();
	UIXTestClick( xRect.x + 20, xRect.y + 8 );
	UIXTestLog( "checkbox item checked %d (the app toggles it)", pCheck->IsChecked() );

	UIXTestLog( "hover the submenu for 10 frames:" );
	xRect = pSub->GetLastRenderRect();
	UIXTestMouseMove( xRect.x + 20, xRect.y + 8 );
	UIXTestFrame( 10 );
	UIXTestLog( "submenu expanded %d", pSub->IsExpanded() );
	UIXTestLog( "hover 20 more frames:" );
	UIXTestFrame( 20 );
	UIXTestLog( "submenu expanded %d", pSub->IsExpanded() );
	UIX_CHECK( pSub->IsExpanded(), "PopupMenu: hovering a submenu item opens it after a delay" );
	UIXTestSnapshot( "submenu", 0, 0, 480, 260 );
	{
	UIXPopupMenuItem*	pSubOne = (UIXPopupMenuItem*)UIXTestChildren( pSub )[0];

		xRect = pSubOne->GetLastRenderRect();
		UIXTestLog( "sub one at %d,%d size %d,%d; click it:", xRect.x, xRect.y, xRect.w, xRect.h );
		UIXTestClick( xRect.x + 20, xRect.y + 8 );
	}
	UIXTestLog( "submenu expanded %d after choosing", pSub->IsExpanded() );
}

//------------------------------------------------------------------------------------------------
// Modal popups: they draw over what came before and take the clicks
//------------------------------------------------------------------------------------------------
UIX_TEST( ModalPopup_BlocksWhatIsBehind )
{
UIXObject*		pPage = UIX::AddPage( UIXRECT( 0, 0, 640, 480 ), "Behind" );
UIXModalPopup*	pModal;
int				nButtonID = UIXTestRegisterIDHandler();

	UIX::AddButton( pPage, UIXRECT( 50, 50, 200, 24 ), UIXBUTTON_NORMAL, "Behind the popup", nButtonID, 1, FALSE );
	pModal = UIX::AddModalPopup( pPage, UIXRECT( 100, 30, 300, 150 ) );
	UIX::AddButton( pModal, UIXRECT( 20, 80, 160, 24 ), UIXBUTTON_NORMAL, "On the popup", nButtonID, 2 );

	UIXTestFrame( 2 );
	UIXTestLogLayout( NULL, "modal" );
	UIXTestSnapshot( "modal", 0, 0, 440, 200 );
	UIXTestLog( "click the part of the button behind the popup that the popup covers:" );
	UIXTestClick( 150, 62 );
	UIXTestLog( "click the part outside the popup:" );
	UIXTestClick( 70, 62 );
	UIXTestLog( "click the popup's own button:" );
	UIXTestClick( UIXTestRectOf( UIXTestChildren( pModal )[0] ).x + 20, UIXTestRectOf( UIXTestChildren( pModal )[0] ).y + 12 );
}

//------------------------------------------------------------------------------------------------
// Custom render: Elemental's second favourite for anything bespoke. The callback draws and may use
// UIX::CheckForPress / IsMouseHover itself.
//------------------------------------------------------------------------------------------------
static int		msnCustomRenders = 0;
static int		msnCustomPressID = 0;
static UIXRECT	TestCustomRender( UIXObject* pObj, InterfaceInstance* pInterface, UIXRECT& rectInOut, uint32 ulUserParam )
{
UIXRECT		xOccupied = rectInOut;

	msnCustomRenders++;
	pInterface->Rect( 0, rectInOut.x, rectInOut.y, rectInOut.w, rectInOut.h, ( ulUserParam == 1 ) ? 0xFF206080 : 0xFF806020 );
	if ( UIX::IsMouseHover( rectInOut ) )
	{
		pInterface->OutlineBox( 1, rectInOut.x, rectInOut.y, rectInOut.w, rectInOut.h, 0xFFFFFFFF );
	}
	UIX::CheckForPress( pObj, rectInOut, msnCustomPressID, ulUserParam );
	UIX::CheckForRightButtonPress( pObj, rectInOut, msnCustomPressID, ulUserParam + 100 );
	pInterface->Text( 1, rectInOut.x + 4, rectInOut.y + 4, 0xFFFFFFFF, 3, "Custom %u", ulUserParam );
	xOccupied.y = rectInOut.h;
	return( xOccupied );
}

UIX_TEST( CustomRender_DrawPressAndPostRender )
{
UIXObject*		pPage = UIX::AddPage( UIXRECT( 10, 10, 400, 300 ), "Custom" );
UIXCustomRender*	pNormal;
UIXCustomRender*	pPost;

	msnCustomPressID = UIXTestRegisterIDHandler();
	msnCustomRenders = 0;
	pNormal = UIX::AddCustomRender( pPage, UIXRECT( 10, 10, 150, 40 ), TestCustomRender, 1 );
	UIX::AddText( pNormal, UIXRECT( 0, 45, 150, 18 ), 0xFFE0E0E0, 3, NONE, "Child of custom" );
	UIX::AddText( pPage, UIXRECT( 10, 0, 200, 18 ), 0xFFE0E0E0, 3, NONE, "Sibling after custom" );
	pPost = UIX::AddCustomRender( pPage, UIXRECT( 200, 10, 150, 40 ), TestCustomRender, 2 );
	pPost->SetUsePostRender( TRUE );

	UIXTestFrame( 2 );
	UIXTestLog( "renders after 2 frames: %d", msnCustomRenders );
	UIXTestLogLayout( NULL, "custom" );
	UIXTestMouseMove( 50, 30 );
	UIXTestSnapshot( "custom", 0, 0, 420, 120 );
	UIXTestLog( "click the normal one:" );
	UIXTestClick( 50, 30 );
	UIXTestLog( "right click the normal one:" );
	UIXTestRightClick( 50, 30 );
	UIXTestLog( "click the post-render one:" );
	UIXTestClick( UIXTestRectOf( pPost ).x + 20, UIXTestRectOf( pPost ).y + 20 );
}
