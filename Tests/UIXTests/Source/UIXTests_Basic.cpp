//------------------------------------------------------------------------------------------------
// UIXTests_Basic.cpp
//
// Pages, text, shapes, buttons and checkboxes: the widgets Elemental uses most (AddText, AddButton,
// AddShape and AddCheckbox are over half its UIX calls).
//------------------------------------------------------------------------------------------------

#include "UIXTestCore.h"

static int		msnSelectedCount = 0;
static uint32	msulLastSelectParam = 0;

static void	TestSelectedCallback( UIXObject* pxSourceObject, uint32 ulSelectParam )
{
	msnSelectedCount++;
	msulLastSelectParam = ulSelectParam;
	UIXTestLog( "selected object=#%d param=%u", UIXTestIndexOf( pxSourceObject ), ulSelectParam );
}

//------------------------------------------------------------------------------------------------
// Pages
//------------------------------------------------------------------------------------------------
UIX_TEST( Page_LayoutStacksChildren )
{
UIXObject*		pPage = UIX::AddPage( UIXRECT( 20, 30, 300, 200 ), "Test page" );

	// Children of a page stack downwards: each blocking child moves the next one down by the height it occupies
	UIX::AddShape( pPage, UIXRECT( 0, 0, 300, 200 ), UIXSHAPE_SHADEDRECT, FALSE, 0xFF303040, 0xFF202028 );
	UIX::AddButton( pPage, UIXRECT( 10, 10, 120, 20 ), UIXBUTTON_NORMAL, "First", UIXTestRegisterIDHandler(), 1 );
	UIX::AddButton( pPage, UIXRECT( 10, 0, 120, 20 ), UIXBUTTON_NORMAL, "Second", UIXTestRegisterIDHandler(), 2 );
	UIX::AddButton( pPage, UIXRECT( 10, 0, 120, 20 ), UIXBUTTON_NORMAL, "Non-blocking", UIXTestRegisterIDHandler(), 3, FALSE );
	UIX::AddButton( pPage, UIXRECT( 140, 0, 120, 20 ), UIXBUTTON_NORMAL, "Beside it", UIXTestRegisterIDHandler(), 4 );
	// Negative x and w: right-aligned, and 'all but' widths
	UIX::AddButton( pPage, UIXRECT( -130, 0, 120, 20 ), UIXBUTTON_NORMAL, "Right aligned", UIXTestRegisterIDHandler(), 5 );
	UIX::AddButton( pPage, UIXRECT( 10, 0, -10, 20 ), UIXBUTTON_NORMAL, "Full width less 10", UIXTestRegisterIDHandler(), 6 );

	UIXTestFrame( 2 );
	UIXTestLogLayout( NULL, "page" );
	UIXTestSnapshot( "page", 0, 0, 340, 250 );

	UIX_CHECK( UIXTestRectOf( pPage ).x == 20, "Page: drawn at its own x" );
}

UIX_TEST( Page_TwoPagesAndDelete )
{
UIXObject*		pPageA = UIX::AddPage( UIXRECT( 10, 10, 200, 100 ), "A" );
UIXObject*		pPageB = UIX::AddPage( UIXRECT( 100, 60, 200, 100 ), "B" );
UIXButton*		pButtonA;
UIXButton*		pButtonB;
int				nButtonID = UIXTestRegisterIDHandler();

	UIX::AddShape( pPageA, UIXRECT( 0, 0, 200, 100 ), UIXSHAPE_SHADEDRECT, FALSE, 0xFF404060, 0xFF404060 );
	pButtonA = UIX::AddButton( pPageA, UIXRECT( 10, 60, 150, 20 ), UIXBUTTON_NORMAL, "Button on A", nButtonID, 1 );
	UIX::AddShape( pPageB, UIXRECT( 0, 0, 200, 100 ), UIXSHAPE_SHADEDRECT, FALSE, 0xFF604040, 0xFF604040 );
	pButtonB = UIX::AddButton( pPageB, UIXRECT( 10, 10, 150, 20 ), UIXBUTTON_NORMAL, "Button on B", nButtonID, 2 );

	UIXTestFrame( 2 );
	UIXTestSnapshot( "overlap", 0, 0, 320, 180 );

	// Where the two buttons overlap, see which page gets the click (pages render in order; the later one is on top)
	UIXTestLog( "click where both buttons are:" );
	UIXTestClick( 120, 75 );

	UIX_CHECK( UIX::FindUIXObjectByID( pButtonB->GetID() ) == pButtonB, "FindUIXObjectByID: finds a live object" );
	UIX::DeleteObject( pPageB );
	UIX_CHECK( UIX::FindUIXObjectByID( pButtonB->GetID() ) == NULL, "DeleteObject: page's children leave the ID map" );
	UIX_CHECK( UIXTestPages().size() == 1, "DeleteObject: page leaves the page list" );

	UIXTestLog( "after deleting page B:" );
	UIXTestClick( 120, 75 );
	UIXTestSnapshot( "afterdelete", 0, 0, 320, 180 );
	(void)pButtonA;
}

static float	msfPageWheel = 0.0f;
static void	TestPageWheel( float fAmount )
{
	msfPageWheel += fAmount;
	UIXTestLog( "page wheel %.2f", fAmount );
}

static void	TestDefaultWheel( float fAmount )
{
	UIXTestLog( "default wheel %.2f", fAmount );
}

UIX_TEST( Page_MousewheelHandler )
{
UIXPage*		pPage = (UIXPage*)UIX::AddPage( UIXRECT( 50, 50, 200, 150 ), "Wheel" );

	UIX::RegisterDefaultMousewheelHandler( TestDefaultWheel );
	pPage->RegisterMousewheelHandler( TestPageWheel );

	UIXTestLog( "wheel with the mouse outside the page:" );
	UIXTestMouseMove( 400, 400 );
	UIXTestFrame( 1 );
	UIXTestWheel( 1 );

	UIXTestLog( "wheel over the page:" );
	UIXTestMouseMove( 100, 100 );
	UIXTestFrame( 1 );
	UIXTestWheel( 1 );
	UIXTestWheel( -2 );
	UIX_CHECK( msfPageWheel != 0.0f, "Page: mouse wheel handler is called while the mouse is over the page" );

	UIXTestLog( "wheel after moving away again:" );
	UIXTestMouseMove( 400, 400 );
	UIXTestFrame( 1 );
	UIXTestWheel( 1 );
}

UIX_TEST( Page_IsWithinPageAndClipping )
{
UIXPage*		pPage = (UIXPage*)UIX::AddPage( UIXRECT( 40, 40, 160, 80 ), "Clipped", TRUE );

	// A clipping page draws its children to a canvas and copies only its own rect back, so this shape is cut off
	UIX::AddShape( pPage, UIXRECT( -20, -20, 260, 160 ), UIXSHAPE_SHADEDRECT, FALSE, 0xFF20A040, 0xFF104020 );
	UIX::AddText( pPage, UIXRECT( 10, 10, 300, 20 ), 0xFFFFFFFF, 3, NONE, "Text that runs past the edge of the page" );
	UIXTestFrame( 2 );
	UIXTestSnapshot( "clipped", 0, 0, 260, 160 );

	UIX_CHECK( pPage->IsWithinPage( 40, 40 ) == TRUE, "Page: IsWithinPage includes the top left corner" );
	UIX_CHECK( pPage->IsWithinPage( 200, 120 ) == FALSE, "Page: IsWithinPage excludes x+w, y+h" );
	UIX_CHECK( pPage->IsWithinPage( 199, 119 ) == TRUE, "Page: IsWithinPage includes x+w-1, y+h-1" );
	UIXTestLog( "page title '%s'", pPage->GetTitle().c_str() );
}

//------------------------------------------------------------------------------------------------
// Text
//------------------------------------------------------------------------------------------------
static void	TestDynamicText( UIXObject* pxSourceObject, uint32 ulSelectParam, char* pcTextBuffer )
{
	strcpy( pcTextBuffer, "Dynamic text" );
}

UIX_TEST( Text_FlagsAndFonts )
{
UIXObject*		pPage = UIX::AddPage( UIXRECT( 10, 10, 400, 300 ), "Text" );
UIXText*		pDynamic;
int				nFont;

	UIX::AddShape( pPage, UIXRECT( 0, 0, 400, 300 ), UIXSHAPE_OUTLINEBOX, FALSE, 0xFF808080 );
	UIX::AddText( pPage, UIXRECT( 10, 10, 380, 18 ), 0xFFE0E0E0, 3, NONE, "Left aligned, font 3" );
	UIX::AddText( pPage, UIXRECT( 10, 30, 380, 18 ), 0xFFE0E0E0, 3, ALIGN_RIGHT, "Right aligned" );
	UIX::AddText( pPage, UIXRECT( 10, 50, 380, 18 ), 0xFFE0E0E0, 3, ALIGN_CENTRE, "Centred" );
	UIX::AddText( pPage, UIXRECT( 10, 70, 380, 18 ), 0xFFE0E0E0, 3, SMALL, "Small" );
	// Flags are matched exactly, so a combination falls back to left aligned
	UIX::AddText( pPage, UIXRECT( 10, 90, 380, 18 ), 0xFFE0E0E0, 3, (UIX_TEXT_FLAGS)( ALIGN_RIGHT | BOLD ), "Right and bold" );
	UIX::AddText( pPage, UIXRECT( 10, 110, 380, 18 ), 0, 3, NONE, "Colour 0 means the default off-white" );
	UIX::AddText( pPage, UIXRECT( 10, 130, 380, 18 ), 0xFFE0E0E0, 3, NONE, "Formatted %d %s", 42, "args" );
	UIX::AddText( pPage, UIXRECT( 10, 150, 100, 18 ), 0xFFE0E0E0, 3, NONE, "Limited to 100 pixels wide, so this is cut" );
	for ( nFont = 0; nFont < 5; nFont++ )
	{
		UIX::AddText( pPage, UIXRECT( 10 + ( nFont * 70 ), 175, 70, 30 ), 0xFFE0E0E0, nFont, NONE, "Font %d", nFont );
	}
	pDynamic = UIX::AddText( pPage, UIXRECT( 10, 220, 380, 18 ), 0xFFC0F0C0, 3, NONE, "Static" );
	pDynamic->SetDynamicTextFunction( TestDynamicText );

	UIXTestFrame( 2 );
	UIXTestLogLayout( NULL, "text" );
	UIXTestSnapshot( "text", 0, 0, 420, 320 );
	UIX_CHECK( strcmp( UIXTestTextString( pDynamic ), "Dynamic text" ) == 0, "Text: dynamic text function replaces the text when drawn" );
}

//------------------------------------------------------------------------------------------------
// Shapes
//------------------------------------------------------------------------------------------------
UIX_TEST( Shape_ModesBlockingAndPress )
{
UIXObject*		pPage = UIX::AddPage( UIXRECT( 10, 10, 400, 300 ), "Shapes" );
UIXShape*		pAnimOut;
UIXShape*		pAnimIn;
int				nButtonID = UIXTestRegisterIDHandler();

	UIX::AddShape( pPage, UIXRECT( 10, 10, 100, 40 ), UIXSHAPE_SHADEDRECT, TRUE, 0xFFC04040, 0xFF401010 );
	UIX::AddShape( pPage, UIXRECT( 10, 10, 100, 40 ), UIXSHAPE_OUTLINEBOX, TRUE, 0xFF40C040 );
	UIX::AddShape( pPage, UIXRECT( 10, 10, 100, 40 ), UIXSHAPE_TRANSOUTLINEBOX, TRUE, 0xFF4040C0 );
	UIX::AddShape( pPage, UIXRECT( 150, 0, 100, 40 ), UIXSHAPE_SHADEDRECT, FALSE, 0xFF808020, 0xFF202008, nButtonID, 77 );
	pAnimOut = UIX::AddShape( pPage, UIXRECT( 150, 60, 100, 40 ), UIXSHAPE_TRANSOUTLINEBOX, FALSE, 0xFFE0E0E0 );
	pAnimIn = UIX::AddShape( pPage, UIXRECT( 270, 60, 100, 40 ), UIXSHAPE_TRANSOUTLINEBOX, FALSE, 0xFFE0E0E0 );
	pAnimOut->SetAnimation( 0, 1.0f );
	pAnimIn->SetAnimation( 1, 1.0f );

	UIXTestFrame( 2 );
	UIXTestLogLayout( NULL, "shapes" );
	UIXTestSnapshot( "start", 0, 0, 420, 200 );

	UIXTestLog( "click the pressable shape:" );
	UIXTestClick( 200, 30 );
	UIXTestFrame( 25 );
	UIXTestSnapshot( "halfway", 0, 0, 420, 200 );
	UIXTestFrame( 40 );
	UIXTestSnapshot( "finished", 0, 0, 420, 200 );
	{
	float	fOutTime, fOutPhase, fInTime, fInPhase;

		UIXTestShapeAnimation( pAnimOut, &fOutTime, &fOutPhase );
		UIXTestShapeAnimation( pAnimIn, &fInTime, &fInPhase );
		UIXTestLog( "after the animations: out time %.2f phase %.2f, in time %.2f phase %.2f", fOutTime, fOutPhase, fInTime, fInPhase );
	}
}

//------------------------------------------------------------------------------------------------
// Buttons
//------------------------------------------------------------------------------------------------
UIX_TEST( Button_NormalClickAndHover )
{
UIXObject*		pPage = UIX::AddPage( UIXRECT( 10, 10, 300, 200 ), "Buttons" );
int				nButtonID = UIXTestRegisterIDHandler();
UIXButton*		pButton = UIX::AddButton( pPage, UIXRECT( 20, 20, 140, 24 ), UIXBUTTON_NORMAL, "Click me", nButtonID, 5 );

	UIXTestFrame( 2 );
	UIXTestSnapshot( "idle", 0, 0, 200, 70 );
	UIXTestMouseMove( 60, 40 );
	UIXTestFrame( 2 );
	UIXTestSnapshot( "hover", 0, 0, 200, 70 );

	UIXTestLog( "click inside:" );
	UIXTestClick( 60, 40 );
	UIXTestLog( "press inside, release outside:" );
	UIXTestMouseMove( 60, 40 );
	UIXTestFrame( 1 );
	UIXTestMouseDown();
	UIXTestFrame( 1 );
	UIXTestMouseMove( 250, 150 );
	UIXTestFrame( 1 );
	UIXTestMouseUp();
	UIXTestFrame( 1 );
	UIXTestLog( "press outside, release inside:" );
	UIXTestMouseMove( 250, 150 );
	UIXTestFrame( 1 );
	UIXTestMouseDown();
	UIXTestFrame( 1 );
	UIXTestMouseMove( 60, 40 );
	UIXTestFrame( 1 );
	UIXTestMouseUp();
	UIXTestFrame( 1 );
	UIXTestLog( "click on the exact top left corner:" );
	UIXTestClick( 30, 30 );
	UIXTestLog( "click one pixel inside the corner:" );
	UIXTestClick( 31, 31 );
	(void)pButton;
}

static int		msnImageHandlerCalls = 0;
static int	TestDynamicImage( UIXObject* pObject, void* pImageHandlerParam )
{
	msnImageHandlerCalls++;
	return( UIXTestIconSheet() );
}

UIX_TEST( Button_AllModes )
{
UIXObject*		pPage = UIX::AddPage( UIXRECT( 10, 10, 600, 400 ), "Button modes" );
int				nButtonID = UIXTestRegisterIDHandler();
UIXButton*		pGlowing;
UIXButton*		pDynamic;
UIXButton*		pCollapsable;
UIXObject*		pImagePage;

	UIX::AddButton( pPage, UIXRECT( 10, 10, 160, 22 ), UIXBUTTON_NORMAL, "Normal", nButtonID, 1 );
	UIX::AddButton( pPage, UIXRECT( 10, 5, 160, 22 ), UIXBUTTON_DISABLED, "Disabled", nButtonID, 2 );
	UIX::AddButton( pPage, UIXRECT( 10, 5, 160, 22 ), UIXBUTTON_PLAIN_RECT, "Plain rect", nButtonID, 3, TRUE, 0xFF404060 );
	UIX::AddButton( pPage, UIXRECT( 10, 5, 160, 22 ), UIXBUTTON_PLAIN_RECT_BOLD_TEXT, "Plain rect bold", nButtonID, 4, TRUE, 0xFF406040 );
	UIX::AddButton( pPage, UIXRECT( 10, 5, 160, 22 ), UIXBUTTON_TEXT_WITH_SETTINGS, "Text with settings", nButtonID, 5 );
	pCollapsable = UIX::AddButton( pPage, UIXRECT( 10, 5, 160, 22 ), UIXBUTTON_TEXT_WITH_COLLAPSABLE, "Text with collapsable", nButtonID, 6 );
	UIX::AddButton( pPage, UIXRECT( 10, 5, 30, 30 ), UIXBUTTON_ICON, "", nButtonID, 7, TRUE, 0, 3 );
	UIX::AddButton( pPage, UIXRECT( 10, 5, 30, 30 ), UIXBUTTON_RECT_ICON, "", nButtonID, 8, TRUE, 0xFF604020, 4 );
	pGlowing = UIX::AddButton( pPage, UIXRECT( 10, 5, 30, 30 ), UIXBUTTON_RECT_ICON, "", nButtonID, 9, TRUE, 0xFF604020, 5 );
	pGlowing->SetGlowing( TRUE );
	// Image buttons on a second page, so the first page's stacking doesn't move them
	pImagePage = UIX::AddPage( UIXRECT( 260, 20, 340, 200 ), "Image buttons" );
	// A UIXBUTTON_IMAGE button releases the texture it was given (its icon number) when it is deleted, so give it
	// a reference of its own. Left at the default icon number 0, it would release texture 0, so pass NOTFOUND.
	UIX::AddButton( pImagePage, UIXRECT( 0, 0, 64, 64 ), UIXBUTTON_IMAGE, "", nButtonID, 10, FALSE, 0xFFFFFFFF, UIXTestInterface()->GetTexture( "..\\Tests\\UIXTests\\Out\\UIXTestIcons.png", 0 ) );
	pDynamic = UIX::AddButton( pImagePage, UIXRECT( 80, 0, 64, 64 ), UIXBUTTON_IMAGE, "", nButtonID, 11, FALSE, 0, NOTFOUND );
	pDynamic->SetDynamicButtonImageHandler( TestDynamicImage, NULL );
	UIX::AddButton( pImagePage, UIXRECT( 160, 0, 64, 64 ), UIXBUTTON_IMAGE, "", nButtonID, 12, FALSE, 0xFF806040, NOTFOUND );
	UIX::AddButton( pImagePage, UIXRECT( 0, 80, 64, 64 ), UIXBUTTON_IMAGE_PATH, "..\\Tests\\UIXTests\\Out\\UIXTestIcons.png", nButtonID, 13, FALSE, 0xFFFFFFFF );
	UIX::AddButton( pImagePage, UIXRECT( 80, 80, 64, 64 ), UIXBUTTON_IMAGE_PATH, "Data\\NoSuchImage.png", nButtonID, 14, FALSE, 0xFF608080 );

	UIXTestFrame( 2 );
	UIXTestLogLayout( NULL, "button modes" );
	UIXTestSnapshot( "modes", 0, 0, 620, 420 );
	UIX_CHECK( msnImageHandlerCalls > 0, "Button: dynamic image handler is asked for the texture when drawn" );

	pCollapsable->SetCollapsedState( TRUE );
	UIXTestMouseMove( 100, 160 );
	UIXTestFrame( 1 );
	UIXTestMouseDown();
	UIXTestSnapshot( "collapsable_pressed", 0, 130, 200, 60 );
	UIXTestMouseUp();
	UIXTestFrame( 1 );

	UIXTestLog( "click each button in turn:" );
	{
	std::vector<UIXObject*>		buttons = UIXTestChildren( pPage );
	UIXObject*		pChild;
	size_t			nLoop;

		buttons.insert( buttons.end(), UIXTestChildren( pImagePage ).begin(), UIXTestChildren( pImagePage ).end() );
		for ( nLoop = 0; nLoop < buttons.size(); nLoop++ )
		{
		UIXRECT		xRect;

			pChild = buttons[nLoop];
			xRect = UIXTestRectOf( pChild );
			UIXTestLog( "click #%d (%d,%d):", UIXTestIndexOf( pChild ), xRect.x + ( xRect.w / 2 ), xRect.y + ( xRect.h / 2 ) );
			UIXTestClick( xRect.x + ( xRect.w / 2 ), xRect.y + ( xRect.h / 2 ) );
		}
	}
}

static int		msnRightClicks = 0;
static void	TestRightClickCallback( UIXObject* pxSourceObject, uint32 ulSelectParam )
{
	msnRightClicks++;
	UIXTestLog( "right click callback object=#%d param=%u", UIXTestIndexOf( pxSourceObject ), ulSelectParam );
}

UIX_TEST( Button_RightPressAndRightClickCallback )
{
UIXObject*		pPage = UIX::AddPage( UIXRECT( 10, 10, 300, 200 ), "Right" );
int				nLeftID = UIXTestRegisterIDHandler();
int				nRightID = UIXTestRegisterIDHandler();
UIXButton*		pRightPress = UIX::AddButton( pPage, UIXRECT( 10, 10, 140, 22 ), UIXBUTTON_NORMAL, "Right press ID", nLeftID, 1 );
UIXButton*		pCallback = UIX::AddButton( pPage, UIXRECT( 10, 10, 140, 22 ), UIXBUTTON_NORMAL, "Right click callback", nLeftID, 2 );
UIXButton*		pPlain = UIX::AddButton( pPage, UIXRECT( 10, 10, 140, 22 ), UIXBUTTON_NORMAL, "No right click", nLeftID, 3 );

	pRightPress->AddRightButtonPress( nRightID, 99 );
	pCallback->SetRightClickSelectedCallback( TestRightClickCallback, 55 );
	UIXTestFrame( 2 );

	UIXTestLog( "right click the one with a right press ID:" );
	UIXTestRightClick( UIXTestRectOf( pRightPress ).x + 20, UIXTestRectOf( pRightPress ).y + 10 );
	UIXTestLog( "right click the one with a right click callback:" );
	UIXTestRightClick( UIXTestRectOf( pCallback ).x + 20, UIXTestRectOf( pCallback ).y + 10 );
	UIX_CHECK( msnRightClicks == 1, "Button: right click callback is called once per right click" );
	UIXTestLog( "right click a button with neither (nothing happens):" );
	UIXTestRightClick( UIXTestRectOf( pPlain ).x + 20, UIXTestRectOf( pPlain ).y + 10 );
	UIXTestLog( "focused object #%d", UIXTestIndexOf( UIX::GetFocusedObject() ) );
	UIXTestLog( "left click the right press one:" );
	UIXTestClick( UIXTestRectOf( pRightPress ).x + 20, UIXTestRectOf( pRightPress ).y + 10 );
}

static bool		msbAcceptLabel = true;
static bool	TestLabelEdit( UIXObject* pObject, const char* szInputText )
{
	UIXTestLog( "label edit callback object=#%d text='%s' -> %s", UIXTestIndexOf( pObject ), szInputText, msbAcceptLabel ? "accept" : "reject" );
	return( msbAcceptLabel );
}

UIX_TEST( Button_LabelEdit )
{
UIXObject*		pPage = UIX::AddPage( UIXRECT( 10, 10, 300, 200 ), "Label" );
int				nButtonID = UIXTestRegisterIDHandler();
UIXButton*		pButton = UIX::AddButton( pPage, UIXRECT( 10, 10, 200, 22 ), UIXBUTTON_PLAIN_RECT, "Old name", nButtonID, 1, TRUE, 0xFF303050 );

	UIXTestFrame( 1 );
	msbAcceptLabel = true;
	pButton->EnableLabelEdit( TestLabelEdit );
	UIX_CHECK( UIX::GetTextEditFocus() == pButton, "Button: EnableLabelEdit takes the text edit focus" );
	UIX_CHECK( UIX::IsTextInputActive() == TRUE, "Button: EnableLabelEdit registers a keyboard handler" );
	UIXTestType( "New" );
	UIXTestSnapshot( "editing", 0, 0, 240, 50 );
	UIXTestKey( KEY_RETURN );
	UIXTestLog( "title after accepted edit '%s'", UIXTestButtonTitle( pButton ) );
	UIX_CHECK( strcmp( UIXTestButtonTitle( pButton ), "New" ) == 0, "Button: an accepted label edit keeps the typed text (typing replaces the selected text)" );
	UIX_CHECK( UIX::GetTextEditFocus() == NULL, "Button: Enter ends the label edit" );

	msbAcceptLabel = false;
	pButton->EnableLabelEdit( TestLabelEdit );
	UIXTestType( "Rejected" );
	UIXTestKey( KEY_RETURN );
	UIXTestLog( "title after rejected edit '%s'", UIXTestButtonTitle( pButton ) );
	UIXTestSnapshot( "after", 0, 0, 240, 50 );
}

//------------------------------------------------------------------------------------------------
// Checkboxes
//------------------------------------------------------------------------------------------------
UIX_TEST( Checkbox_AllModesToggle )
{
UIXObject*		pPage = UIX::AddPage( UIXRECT( 10, 10, 400, 300 ), "Checkboxes" );
UIXCheckbox*	apBoxes[8];
int				nLoop;
int				nNumBoxes = 0;

	apBoxes[nNumBoxes++] = UIX::AddCheckbox( pPage, UIXRECT( 10, 10, 16, 16 ), STANDARD_CHECKBOX, FALSE, "", TestSelectedCallback, 1 );
	apBoxes[nNumBoxes++] = UIX::AddCheckbox( pPage, UIXRECT( 10, 35, 160, 18 ), STANDARD_CHECKBOX_WITH_LABEL, TRUE, "With label", TestSelectedCallback, 2 );
	apBoxes[nNumBoxes++] = UIX::AddCheckbox( pPage, UIXRECT( 10, 60, 160, 20 ), POPUP_MENU_LIST, TRUE, "Popup menu list", TestSelectedCallback, 3 );
	apBoxes[nNumBoxes++] = UIX::AddCheckbox( pPage, UIXRECT( 10, 85, 24, 24 ), ICON_CHECKBOX, FALSE, "", TestSelectedCallback, 4 );
	apBoxes[nNumBoxes++] = UIX::AddCheckbox( pPage, UIXRECT( 40, 85, 24, 24 ), ICON_CHECKBOX_OUTLINED, TRUE, "", TestSelectedCallback, 5 );
	apBoxes[nNumBoxes++] = UIX::AddCheckbox( pPage, UIXRECT( 10, 115, 160, 20 ), HILIGHT_CHECKBOX, FALSE, "Hilight", TestSelectedCallback, 6 );
	apBoxes[nNumBoxes++] = UIX::AddCheckbox( pPage, UIXRECT( 10, 140, 160, 20 ), HILIGHT_CHECKBOX2, TRUE, "Hilight 2", TestSelectedCallback, 7 );
	apBoxes[nNumBoxes++] = UIX::AddCheckbox( pPage, UIXRECT( 10, 165, 160, 20 ), POPUP_MENU_LIST, FALSE, "No callback, greyed", NULL, 8 );
	apBoxes[3]->SetToggleIcons( 6, 7 );
	apBoxes[4]->SetToggleIcons( 8, 9 );

	UIXTestFrame( 2 );
	UIXTestLogLayout( NULL, "checkboxes" );
	UIXTestSnapshot( "before", 0, 0, 200, 200 );

	msnSelectedCount = 0;
	for ( nLoop = 0; nLoop < nNumBoxes; nLoop++ )
	{
	UIXRECT		xRect = UIXTestRectOf( apBoxes[nLoop] );
	BOOL		bWasChecked = apBoxes[nLoop]->IsChecked();

		UIXTestClick( xRect.x + ( xRect.w / 2 ), xRect.y + ( xRect.h / 2 ) );
		UIXTestLog( "checkbox #%d checked %d -> %d", UIXTestIndexOf( apBoxes[nLoop] ), bWasChecked, apBoxes[nLoop]->IsChecked() );
		UIX_CHECK( apBoxes[nLoop]->IsChecked() != bWasChecked, "Checkbox: a click toggles it" );
	}
	UIX_CHECK( msnSelectedCount == nNumBoxes - 1, "Checkbox: the selected callback runs once per click, when there is one" );
	UIXTestMouseMove( 300, 280 );
	UIXTestSnapshot( "after", 0, 0, 200, 200 );

	apBoxes[1]->SetText( "Renamed" );
	apBoxes[5]->SetBackgroundCol( 0xFF800000 );
	UIXTestSnapshot( "renamed", 0, 0, 200, 200 );
}
