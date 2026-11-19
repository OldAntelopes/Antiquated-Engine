//------------------------------------------------------------------------------------------------
// UIXTests_Input.cpp
//
// Widgets that take values: dropdowns (Elemental's third most used UIX call), text boxes, sliders in
// every mode, list boxes, dials and horizontal scrollbars.
//------------------------------------------------------------------------------------------------

#include "UIXTestCore.h"

//------------------------------------------------------------------------------------------------
// Dropdowns
//------------------------------------------------------------------------------------------------
static void	TestDropdownChanged( UIXObject* pxSourceObject, uint32 ulElementParam )
{
	UIXTestLog( "dropdown #%d selection changed param=%u", UIXTestIndexOf( pxSourceObject ), ulElementParam );
}

static void	LogDropdown( const char* szWhen, UIXDropdown* pDropdown )
{
	UIXTestLog( "%s: expanded %d selected %d", szWhen, UIXTestDropdownIsExpanded( pDropdown ), UIXTestDropdownSelectedIndex( pDropdown ) );
}

// Entry N of an expanded dropdown that opens downwards: 14 pixel lines starting 3 below the header
static void	ClickDropdownEntry( UIXDropdown* pDropdown, int nEntry )
{
UIXRECT		xRect = UIXTestRectOf( pDropdown );

	UIXTestClick( xRect.x + 20, xRect.y + xRect.h + 1 + 2 + ( nEntry * 14 ) + 7 );
}

UIX_TEST( Dropdown_OpenSelectClose )
{
UIXObject*		pPage = UIX::AddPage( UIXRECT( 10, 10, 400, 300 ), "Dropdown" );
UIXDropdown*	pDropdown = UIX::AddDropdown( pPage, UIXRECT( 20, 20, 160, 16 ) );
UIXRECT			xRect;

	pDropdown->AddElement( "Apples", 10 );
	pDropdown->AddElement( "Bananas", 20 );
	pDropdown->AddElement( "Cherries (greyed)", 30, TRUE );
	pDropdown->AddElement( "Damsons", 40 );
	pDropdown->AddElement( "Elderberries with a long name that runs on", 50 );
	pDropdown->SetSelectionChangedCallback( TestDropdownChanged );

	UIXTestFrame( 2 );
	LogDropdown( "start", pDropdown );
	UIXTestSnapshot( "empty", 0, 0, 220, 140 );
	xRect = UIXTestRectOf( pDropdown );

	UIXTestLog( "click the header:" );
	UIXTestClick( xRect.x + 40, xRect.y + 8 );
	LogDropdown( "after header click", pDropdown );
	UIX_CHECK( UIXTestDropdownIsExpanded( pDropdown ), "Dropdown: clicking the header opens it" );
	UIXTestMouseMove( xRect.x + 20, xRect.y + xRect.h + 1 + 2 + 14 + 7 );
	UIXTestSnapshot( "open_hover", 0, 0, 220, 140 );

	UIXTestLog( "click entry 1:" );
	ClickDropdownEntry( pDropdown, 1 );
	LogDropdown( "after entry click", pDropdown );
	UIX_CHECK( UIXTestDropdownSelectedIndex( pDropdown ) == 1, "Dropdown: clicking an entry selects it" );
	UIX_CHECK( !UIXTestDropdownIsExpanded( pDropdown ), "Dropdown: choosing an entry closes it" );
	UIXTestSnapshot( "selected", 0, 0, 220, 140 );

	UIXTestLog( "open, click the greyed entry:" );
	UIXTestClick( xRect.x + 40, xRect.y + 8 );
	ClickDropdownEntry( pDropdown, 2 );
	LogDropdown( "after greyed click", pDropdown );

	UIXTestLog( "click the header again:" );
	UIXTestClick( xRect.x + 40, xRect.y + 8 );
	LogDropdown( "after second header click", pDropdown );

	UIXTestLog( "open, then escape:" );
	if ( !UIXTestDropdownIsExpanded( pDropdown ) )
	{
		UIXTestClick( xRect.x + 40, xRect.y + 8 );
	}
	UIXTestKey( KEY_ESCAPE );
	LogDropdown( "after escape", pDropdown );

	UIXTestLog( "open, then click elsewhere on the page:" );
	UIXTestClick( xRect.x + 40, xRect.y + 8 );
	UIXTestClick( 350, 250 );
	LogDropdown( "after clicking elsewhere", pDropdown );
}

static int		msnForcedIndex = NOTFOUND;
static int	TestDropdownUpdate( UIXObject* pxSourceObject, uint32 ulSelectedElementIndex )
{
	if ( msnForcedIndex != NOTFOUND )
	{
		return( msnForcedIndex );
	}
	return( (int)ulSelectedElementIndex );
}

UIX_TEST( Dropdown_SettersAndUpdateHandler )
{
UIXObject*		pPage = UIX::AddPage( UIXRECT( 10, 10, 400, 300 ), "Dropdown" );
UIXDropdown*	pDropdown = UIX::AddDropdown( pPage, UIXRECT( 20, 20, 160, 16 ) );
const UIXDropdownEntry*	pEntry;

	pDropdown->AddElement( "Zero", 100 );
	pDropdown->AddElement( "One", 101 );
	pDropdown->AddElement( "Two", 102 );
	pDropdown->SetSelectionChangedCallback( TestDropdownChanged );

	pDropdown->SetSelectedElementIndex( 2 );
	LogDropdown( "SetSelectedElementIndex 2", pDropdown );
	pDropdown->SetSelectedElementName( "one" );
	LogDropdown( "SetSelectedElementName 'one' (case-insensitive)", pDropdown );
	pDropdown->SetSelectedElementName( "Nothing" );
	LogDropdown( "SetSelectedElementName of a missing name", pDropdown );
	pDropdown->SetSelectedElementIndex( 7 );
	LogDropdown( "SetSelectedElementIndex 7 (out of range clears it, no callback)", pDropdown );

	pEntry = pDropdown->GetElementByListIndex( 1 );
	UIXTestLog( "element 1 '%s'", pEntry ? pEntry->GetText().c_str() : "(null)" );
	pEntry = pDropdown->GetElementByListIndex( 3 );
	UIXTestLog( "element 3 %s", pEntry ? "found" : "(null)" );

	pDropdown->RegisterValueUpdateHandler( TestDropdownUpdate );
	msnForcedIndex = 0;
	UIXTestFrame( 1 );
	LogDropdown( "update handler returns 0", pDropdown );
	msnForcedIndex = 9;
	UIXTestFrame( 1 );
	LogDropdown( "update handler returns 9 (ignored)", pDropdown );
	msnForcedIndex = NOTFOUND;
	UIXTestSnapshot( "after", 0, 0, 220, 60 );
}

UIX_TEST( Dropdown_OnlyOneOpenAndKeys )
{
UIXObject*		pPage = UIX::AddPage( UIXRECT( 10, 10, 400, 300 ), "Dropdowns" );
UIXDropdown*	pFirst = UIX::AddDropdown( pPage, UIXRECT( 20, 20, 150, 16 ) );
UIXDropdown*	pSecond = UIX::AddDropdown( pPage, UIXRECT( 200, 20, 150, 16 ) );
UIXRECT			xFirst;
UIXRECT			xSecond;
int				nLoop;

	for ( nLoop = 0; nLoop < 3; nLoop++ )
	{
	char	acName[32];

		sprintf( acName, "First %d", nLoop );
		pFirst->AddElement( acName, nLoop );
		sprintf( acName, "Second %d", nLoop );
		pSecond->AddElement( acName, 100 + nLoop );
	}
	pFirst->SetSelectionChangedCallback( TestDropdownChanged );
	pSecond->SetDropListWidth( 220 );
	UIXTestFrame( 2 );
	xFirst = UIXTestRectOf( pFirst );
	xSecond = UIXTestRectOf( pSecond );

	UIXTestClick( xFirst.x + 20, xFirst.y + 8 );
	UIXTestClick( xSecond.x + 20, xSecond.y + 8 );
	UIXTestLog( "opened first then second: first expanded %d, second expanded %d", UIXTestDropdownIsExpanded( pFirst ), UIXTestDropdownIsExpanded( pSecond ) );
	UIX_CHECK( !UIXTestDropdownIsExpanded( pFirst ), "Dropdown: opening one closes the others" );
	UIXTestSnapshot( "second_open_wide", 0, 0, 440, 120 );
	UIXTestClick( xSecond.x + 20, xSecond.y + 8 );

	// Arrow keys go to the focused object, which a right click on a dropdown sets
	UIXTestLog( "down arrow before focusing:" );
	UIXTestKey( KEY_DOWN_ARROW );
	LogDropdown( "first", pFirst );
	UIXTestLog( "right click to focus, then arrows:" );
	UIXTestRightClick( xFirst.x + 20, xFirst.y + 8 );
	UIXTestLog( "focused #%d", UIXTestIndexOf( UIX::GetFocusedObject() ) );
	UIXTestKey( KEY_DOWN_ARROW );
	LogDropdown( "after down", pFirst );
	UIXTestKey( KEY_UP_ARROW );
	LogDropdown( "after up", pFirst );
	UIXTestKey( KEY_UP_ARROW );
	LogDropdown( "after up (wraps)", pFirst );
	UIXTestKey( KEY_DOWN_ARROW );
	LogDropdown( "after down (wraps)", pFirst );
	UIXTestSnapshot( "focused", 0, 0, 440, 60 );
	UIXTestLog( "right click again to unfocus:" );
	UIXTestRightClick( xFirst.x + 20, xFirst.y + 8 );
	UIXTestLog( "focused #%d", UIXTestIndexOf( UIX::GetFocusedObject() ) );
}

UIX_TEST( Dropdown_LongListScrollsAndDropsUp )
{
UIXObject*		pPage = UIX::AddPage( UIXRECT( 0, 0, 640, 480 ), "Long dropdowns" );
UIXDropdown*	pTop = UIX::AddDropdown( pPage, UIXRECT( 20, 20, 150, 16 ) );
UIXDropdown*	pBottom = UIX::AddDropdown( pPage, UIXRECT( 300, 440, 150, 16 ) );
UIXRECT			xTop;
UIXRECT			xBottom;
int				nLoop;

	for ( nLoop = 0; nLoop < 40; nLoop++ )
	{
	char	acName[32];

		sprintf( acName, "Entry %d", nLoop );
		pTop->AddElement( acName, nLoop );
		pBottom->AddElement( acName, nLoop );
	}
	pTop->SetSelectionChangedCallback( TestDropdownChanged );
	UIXTestFrame( 2 );
	xTop = UIXTestRectOf( pTop );
	xBottom = UIXTestRectOf( pBottom );

	UIXTestClick( xTop.x + 20, xTop.y + 8 );
	UIXTestSnapshot( "top_open", 0, 0, 200, 480 );
	UIXTestLog( "wheel down 3 notches over the list:" );
	UIXTestMouseMove( xTop.x + 40, 200 );
	UIXTestFrame( 1 );
	UIXTestWheel( -3 );
	UIXTestLog( "scroll position %d", UIXTestDropdownScrollPosition( pTop ) );
	UIXTestSnapshot( "top_scrolled", 0, 0, 200, 480 );
	UIXTestLog( "click the first visible line:" );
	ClickDropdownEntry( pTop, 0 );
	LogDropdown( "top", pTop );
	UIXTestClick( xTop.x + 20, xTop.y + 8 );
	UIXTestLog( "reopened: scroll position %d", UIXTestDropdownScrollPosition( pTop ) );
	UIXTestClick( xTop.x + 20, xTop.y + 8 );

	UIXTestLog( "open the one near the bottom of the screen:" );
	UIXTestClick( xBottom.x + 20, xBottom.y + 8 );
	LogDropdown( "bottom", pBottom );
	UIXTestSnapshot( "bottom_open", 280, 0, 200, 480 );
}

//------------------------------------------------------------------------------------------------
// Text boxes
//------------------------------------------------------------------------------------------------
static void	TestEndEdit( UIXObject* pxSourceObject, const char* szNewText )
{
	UIXTestLog( "end edit #%d text='%s'", UIXTestIndexOf( pxSourceObject ), szNewText );
}

UIX_TEST( TextBox_EditEnterTabAndSwitch )
{
UIXObject*		pPage = UIX::AddPage( UIXRECT( 10, 10, 400, 300 ), "Text boxes" );
UIXTextBox*		pFirst = UIX::AddTextBox( pPage, UIXRECT( 20, 20, 200, 20 ), 0, "Hello" );
UIXTextBox*		pSecond = UIX::AddTextBox( pPage, UIXRECT( 20, 50, 200, 20 ), 0, "Second" );
UIXTextBox*		pLong = UIX::AddTextBox( pPage, UIXRECT( 20, 80, 100, 20 ), 0, "" );
UIXRECT			xFirst;
UIXRECT			xSecond;

	pFirst->SetEndEditCallback( TestEndEdit );
	pSecond->SetEndEditCallback( TestEndEdit );
	pLong->SetText( "A long line, %d characters or so, that won't fit", 50 );
	UIXTestFrame( 2 );
	UIXTestLogLayout( NULL, "text boxes" );
	UIXTestSnapshot( "idle", 0, 0, 260, 120 );
	xFirst = UIXTestRectOf( pFirst );
	xSecond = UIXTestRectOf( pSecond );

	UIXTestLog( "click the first box:" );
	UIXTestClick( xFirst.x + 50, xFirst.y + 10 );
	UIX_CHECK( UIX::GetTextEditFocus() == pFirst, "TextBox: a click gives it the edit focus" );
	UIXTestLog( "keyboard string '%s'", PlatformKeyboardGetInputString( FALSE ) );
	UIXTestSnapshot( "focused_selected", 0, 0, 260, 120 );
	UIXTestType( "World" );
	UIXTestLog( "typed 'World': text '%s'", pFirst->GetText().c_str() );
	UIXTestKey( KEY_BACKSPACE );
	UIXTestType( "!" );
	UIXTestSnapshot( "typed", 0, 0, 260, 120 );
	UIXTestKey( KEY_RETURN );
	UIXTestLog( "after enter: text '%s' focus #%d", pFirst->GetText().c_str(), UIXTestIndexOf( UIX::GetTextEditFocus() ) );
	UIX_CHECK( pFirst->GetText() == "Worl!", "TextBox: typing replaces the selected text; backspace deletes" );

	UIXTestLog( "click first, type, then tab:" );
	UIXTestClick( xFirst.x + 50, xFirst.y + 10 );
	UIXTestType( "Tabbed" );
	UIXTestKey( KEY_TAB );
	UIXTestLog( "after tab: text '%s' focus #%d", pFirst->GetText().c_str(), UIXTestIndexOf( UIX::GetTextEditFocus() ) );

	UIXTestLog( "click first, type, then click the second box:" );
	UIXTestClick( xFirst.x + 50, xFirst.y + 10 );
	UIXTestType( "Switched" );
	UIXTestClick( xSecond.x + 50, xSecond.y + 10 );
	UIXTestLog( "first '%s', second '%s', focus #%d", pFirst->GetText().c_str(), pSecond->GetText().c_str(), UIXTestIndexOf( UIX::GetTextEditFocus() ) );
	UIXTestKey( KEY_RETURN );

	UIXTestLog( "click first, type, then click empty page:" );
	UIXTestClick( xFirst.x + 50, xFirst.y + 10 );
	UIXTestType( "Abandoned" );
	UIXTestClick( 300, 250 );
	UIXTestLog( "first '%s', focus #%d", pFirst->GetText().c_str(), UIXTestIndexOf( UIX::GetTextEditFocus() ) );
	UIXTestSnapshot( "end", 0, 0, 260, 120 );
}

//------------------------------------------------------------------------------------------------
// Sliders
//------------------------------------------------------------------------------------------------
struct SliderLogState
{
	float	fValue;
	float	fMin;
	float	fMax;
	BOOL	bHeld;
	int		nCalls;
};

static SliderLogState	maxSliderLog[16];

static float	TestSliderValueUpdate( uint32 ulUIXObjectID, float fUIXValue, float fUIXMinRangeVal, float fUIXMaxRangeValue, uint32 ulUserParam, BOOL bIsUIHeld )
{
SliderLogState*		pState = &maxSliderLog[ulUserParam & 15];

	// Called every frame, so log only changes
	if ( ( pState->nCalls == 0 ) ||
		 ( pState->fValue != fUIXValue ) ||
		 ( pState->fMin != fUIXMinRangeVal ) ||
		 ( pState->fMax != fUIXMaxRangeValue ) ||
		 ( pState->bHeld != bIsUIHeld ) )
	{
		UIXTestLog( "slider param %u value %.3f range %.3f..%.3f held %d", ulUserParam, fUIXValue, fUIXMinRangeVal, fUIXMaxRangeValue, bIsUIHeld );
	}
	pState->fValue = fUIXValue;
	pState->fMin = fUIXMinRangeVal;
	pState->fMax = fUIXMaxRangeValue;
	pState->bHeld = bIsUIHeld;
	pState->nCalls++;
	return( fUIXValue );
}

static void	LogSlider( const char* szWhen, UIXSlider* pSlider )
{
float	fCurrent, fMin, fMax;

	UIXTestSliderValues( pSlider, &fCurrent, &fMin, &fMax );
	UIXTestLog( "%s: value %.3f range %.3f..%.3f", szWhen, fCurrent, fMin, fMax );
}

UIX_TEST( Slider_AllModesRender )
{
UIXObject*		pPage = UIX::AddPage( UIXRECT( 10, 10, 620, 460 ), "Sliders" );

	UIX::AddSlider( pPage, UIXRECT( 10, 10, 400, 18 ), SLIDERMODE_VALUE, 1, 0.0f, 1.0f, 0.25f, 0.1f, "Value" );
	UIX::AddSlider( pPage, UIXRECT( 10, 40, 400, 18 ), SLIDERMODE_VALUE, 2, 0.0f, 100.0f, 40.0f, 1.0f, "Value 0-100" );
	UIX::AddSlider( pPage, UIXRECT( 10, 70, 400, 18 ), SLIDERMODE_VALUE, 3, 0.0f, 1.0f, 0.75f, 0.1f, "No boxes", FALSE );
	UIX::AddSlider( pPage, UIXRECT( 10, 100, 400, 18 ), SLIDERMODE_ANGLE, 4, 0.0f, A360, 1.0f, 0.1f, "Angle" );
	UIX::AddSlider( pPage, UIXRECT( 10, 130, 400, 18 ), SLIDERMODE_VALUERANGE, 5, 0.0f, 10.0f, 5.0f, 0.1f, "Range" );
	UIX::AddSlider( pPage, UIXRECT( 10, 160, 400, 18 ), SLIDERMODE_INTVALUERANGE, 6, 0.0f, 20.0f, 5.0f, 1.0f, "Int range" );
	UIX::AddSlider( pPage, UIXRECT( 10, 190, 400, 18 ), SLIDERMODE_PLUSMINUS_VALUE, 7, -1.0f, 1.0f, 0.5f, 0.1f, "Plus minus" );
	UIX::AddSlider( pPage, UIXRECT( 10, 220, 400, 18 ), SLIDERMODE_VALUE_WITH_CONSTRAINTS, 8, 0.0f, 1.0f, 0.6f, 0.1f, "Constraints" );
	UIX::AddSlider( pPage, UIXRECT( 10, 250, 400, 18 ), SLIDERMODE_INTVALUE_WITH_CONSTRAINTS, 9, 0.0f, 50.0f, 20.0f, 1.0f, "Int constraints" );
	UIX::AddSlider( pPage, UIXRECT( 450, 10, 30, 200 ), SLIDERMODE_VERTICAL_VALUE, 10, 0.0f, 1.0f, 0.3f, 0.1f, "V" );

	UIXTestFrame( 2 );
	UIXTestLogLayout( NULL, "sliders" );
	UIXTestSnapshot( "modes", 0, 0, 520, 290 );
}

UIX_TEST( Slider_DragValueAndCallback )
{
UIXObject*		pPage = UIX::AddPage( UIXRECT( 10, 10, 620, 300 ), "Slider drag" );
UIXSlider*		pSlider = UIX::AddSlider( pPage, UIXRECT( 10, 10, 400, 18 ), SLIDERMODE_VALUE, 1, 0.0f, 1.0f, 0.25f, 0.1f, "Drag me", FALSE );
UIXSlider*		pVertical = UIX::AddSlider( pPage, UIXRECT( 450, 10, 30, 200 ), SLIDERMODE_VERTICAL_VALUE, 2, 0.0f, 1.0f, 0.5f, 0.1f, "V" );
UIXSlider*		pAngle = UIX::AddSlider( pPage, UIXRECT( 10, 60, 400, 18 ), SLIDERMODE_ANGLE, 3, 0.0f, A360, 0.0f, 0.1f, "Angle" );
UIXRECT			xBar;

	memset( maxSliderLog, 0, sizeof( maxSliderLog ) );
	pSlider->RegisterValueUpdateHandler( TestSliderValueUpdate );
	pVertical->RegisterValueUpdateHandler( TestSliderValueUpdate );
	UIXTestFrame( 2 );

	xBar = UIXTestSliderRenderRect( pSlider );
	UIXTestLog( "value bar at %d,%d size %d,%d", xBar.x, xBar.y, xBar.w, xBar.h );
	UIXTestLog( "drag from a quarter to three quarters:" );
	UIXTestDrag( xBar.x + ( xBar.w / 4 ), xBar.y + 8, xBar.x + ( ( xBar.w * 3 ) / 4 ), xBar.y + 8 );
	LogSlider( "after drag", pSlider );
	UIXTestLog( "drag past the right end:" );
	UIXTestDrag( xBar.x + ( xBar.w / 2 ), xBar.y + 8, xBar.x + xBar.w + 100, xBar.y + 8 );
	LogSlider( "after drag past the end", pSlider );
	UIX_CHECK( fabsf( maxSliderLog[1].fValue - 1.0f ) < 0.001f, "Slider: dragging past the end clamps to the maximum" );
	UIXTestSnapshot( "after_drags", 0, 0, 520, 230 );

	xBar = UIXTestSliderRenderRect( pVertical );
	UIXTestLog( "vertical bar at %d,%d size %d,%d; drag from the middle to the top:", xBar.x, xBar.y, xBar.w, xBar.h );
	UIXTestDrag( xBar.x + 10, xBar.y + ( xBar.h / 2 ), xBar.x + 10, xBar.y + 5 );
	LogSlider( "vertical", pVertical );

	xBar = UIXTestSliderRenderRect( pAngle );
	UIXTestLog( "angle bar at %d,%d size %d,%d; click three quarters along:", xBar.x, xBar.y, xBar.w, xBar.h );
	UIXTestDrag( xBar.x + ( ( xBar.w * 3 ) / 4 ), xBar.y + 8, xBar.x + ( ( xBar.w * 3 ) / 4 ), xBar.y + 8, 1 );
	LogSlider( "angle", pAngle );
	UIXTestSnapshot( "after_vertical_and_angle", 0, 0, 520, 230 );
}

UIX_TEST( Slider_RangeHandlesAndTextEntry )
{
UIXObject*		pPage = UIX::AddPage( UIXRECT( 10, 10, 620, 300 ), "Slider ranges" );
UIXSlider*		pRange = UIX::AddSlider( pPage, UIXRECT( 10, 10, 420, 18 ), SLIDERMODE_VALUE_WITH_CONSTRAINTS, 4, 0.0f, 10.0f, 5.0f, 0.1f, "Range" );
UIXSlider*		pLimited = UIX::AddSlider( pPage, UIXRECT( 10, 50, 420, 18 ), SLIDERMODE_VALUE_WITH_CONSTRAINTS, 5, 0.0f, 10.0f, 5.0f, 0.1f, "Limited" );
UIXRECT			xBar;
UIXTextBox*		pBox;
UIXRECT			xBox;
int				nBarX;

	memset( maxSliderLog, 0, sizeof( maxSliderLog ) );
	pRange->RegisterValueUpdateHandler( TestSliderValueUpdate );
	pLimited->RegisterValueUpdateHandler( TestSliderValueUpdate );
	pLimited->AllowRangeExpand( FALSE );
	UIXTestFrame( 2 );
	UIXTestSnapshot( "start", 0, 0, 460, 90 );

	// The bar sits after the 180 pixel text area; the value grab is a 7 pixel strip at the current value
	xBar = UIXTestSliderRenderRect( pRange );
	UIXTestLog( "bar at %d,%d size %d,%d", xBar.x, xBar.y, xBar.w, xBar.h );
	nBarX = xBar.x + ( xBar.w / 2 );
	UIXTestLog( "drag the value grab from the middle to 80%%:" );
	UIXTestDrag( nBarX, xBar.y + 8, xBar.x + ( ( xBar.w * 8 ) / 10 ), xBar.y + 8 );
	LogSlider( "after value drag", pRange );
	UIXTestLog( "drag the min handle from 0 to 30%%:" );
	UIXTestDrag( xBar.x - 3, xBar.y + 10, xBar.x + ( ( xBar.w * 3 ) / 10 ), xBar.y + 10 );
	LogSlider( "after min drag", pRange );
	UIXTestLog( "drag the max handle from the end to 60%%:" );
	UIXTestDrag( xBar.x + xBar.w + 2, xBar.y + 10, xBar.x + ( ( xBar.w * 6 ) / 10 ), xBar.y + 10 );
	LogSlider( "after max drag", pRange );
	UIXTestFrame( 2 );
	LogSlider( "a frame later (value is clamped into the range in OnUpdate)", pRange );
	UIXTestSnapshot( "after_handles", 0, 0, 460, 90 );

	UIXTestLog( "type 25 into the max box (range expand allowed):" );
	pBox = UIXTestSliderTextBox( pRange, 2 );
	xBox = UIXTestRectOf( pBox );
	UIXTestLog( "max box at %d,%d size %d,%d", xBox.x, xBox.y, xBox.w, xBox.h );
	UIXTestClick( xBox.x + 10, xBox.y + 6 );
	UIXTestLog( "focus is the max box: %d", UIX::GetTextEditFocus() == pBox );
	UIXTestType( "25" );
	UIXTestKey( KEY_RETURN );
	LogSlider( "after typing 25", pRange );

	UIXTestLog( "type 25 into the max box of the limited slider:" );
	pBox = UIXTestSliderTextBox( pLimited, 2 );
	xBox = UIXTestRectOf( pBox );
	UIXTestClick( xBox.x + 10, xBox.y + 6 );
	UIXTestType( "25" );
	UIXTestKey( KEY_RETURN );
	LogSlider( "limited after typing 25", pLimited );
	UIXTestLog( "max box now shows '%s'", pBox->GetText().c_str() );

	UIXTestLog( "type 1 into the limited slider's value box:" );
	pBox = UIXTestSliderTextBox( pLimited, 0 );
	xBox = UIXTestRectOf( pBox );
	UIXTestClick( xBox.x + 10, xBox.y + 6 );
	UIXTestType( "1" );
	UIXTestKey( KEY_RETURN );
	UIXTestFrame( 1 );
	LogSlider( "limited after typing 1", pLimited );

	pRange->ModifySliderRange( 3.0f, -5.0f, 20.0f );
	LogSlider( "ModifySliderRange 3, -5..20 (expand allowed)", pRange );
	pLimited->ModifySliderRange( 3.0f, -5.0f, 20.0f );
	LogSlider( "ModifySliderRange 3, -5..20 (limited)", pLimited );
	UIXTestSnapshot( "end", 0, 0, 460, 90 );
}

//------------------------------------------------------------------------------------------------
// List boxes
//------------------------------------------------------------------------------------------------
static void	TestListBoxChanged( UIXObject* pxSourceObject, uint32 ulElementParam )
{
	UIXTestLog( "listbox #%d selection param=%u", UIXTestIndexOf( pxSourceObject ), ulElementParam );
}

UIX_TEST( ListBox_EntriesSubEntriesAndSelection )
{
UIXObject*		pPage = UIX::AddPage( UIXRECT( 10, 10, 400, 300 ), "List" );
UIXListBox*		pList = UIX::AddListBox( pPage, UIXRECT( 10, 10, 250, 200 ) );
UIXListBox*		pPlain = UIX::AddListBox( pPage, UIXRECT( 280, 0, 100, 60 ) );
const UIXListBoxEntry*	pEntry;
int				nLoop;

	// UIXListBoxEntry::AddSubElement is protected, so apps can't make sub-entries and they aren't tested here
	pList->AddElement( "First", 1 );
	pList->AddElement( "Second", 2 );
	pList->AddElement( "Third", 21 );
	pList->AddElement( "Fourth", 22 );
	pList->AddElement( "Not selectable", 3, FALSE );
	pList->AddElement( "Last", 4 );
	pList->SetSelectionChangedCallback( TestListBoxChanged );
	pPlain->AddElement( "No callback", 9 );

	UIXTestFrame( 2 );
	UIXTestLogLayout( NULL, "list" );
	for ( nLoop = 0; nLoop < 6; nLoop++ )
	{
	UIXRECT		xEntry = UIXTestListBoxEntryRect( pList, nLoop );

		pEntry = pList->GetElementByListIndex( nLoop );
		UIXTestLog( "entry %d '%s' at %d,%d size %d,%d", nLoop, pEntry->GetText().c_str(), xEntry.x, xEntry.y, xEntry.w, xEntry.h );
	}
	UIXTestMouseMove( 60, 72 );
	UIXTestSnapshot( "hover", 0, 0, 420, 220 );

	for ( nLoop = 0; nLoop < 6; nLoop++ )
	{
	UIXRECT		xEntry = UIXTestListBoxEntryRect( pList, nLoop );

		UIXTestLog( "click entry %d:", nLoop );
		UIXTestClick( xEntry.x + 30, xEntry.y + 9 );
	}
	UIXTestLog( "click the list without a callback:" );
	UIXTestClick( UIXTestListBoxEntryRect( pPlain, 0 ).x + 20, UIXTestListBoxEntryRect( pPlain, 0 ).y + 9 );
}

//------------------------------------------------------------------------------------------------
// Dials
//------------------------------------------------------------------------------------------------
static float	msfDialTarget = -1.0f;
static float	TestDialUpdate( uint32 ulUIXObjectID, float fUIXValue, float fUIXMinRangeVal, float fUIXMaxRangeValue, uint32 ulUserParam, BOOL bIsUIHeld )
{
	if ( msfDialTarget >= 0.0f )
	{
		return( msfDialTarget );
	}
	return( fUIXValue );
}

UIX_TEST( Dial_ValuesAndUpdateHandler )
{
UIXObject*		pPage = UIX::AddPage( UIXRECT( 10, 10, 400, 300 ), "Dials" );
UIXDial*		apDials[5];
int				nLoop;

	for ( nLoop = 0; nLoop < 5; nLoop++ )
	{
		apDials[nLoop] = UIX::AddDial( pPage, UIXRECT( 10 + ( nLoop * 70 ), 10, 60, 60 ), nLoop, nLoop * 0.25f );
	}
	UIXTestFrame( 2 );
	UIXTestLogLayout( NULL, "dials" );
	UIXTestSnapshot( "values", 0, 0, 380, 90 );

	// The dial takes no mouse input of its own; its value comes from the update handler
	UIXTestDrag( 40, 40, 40, 10 );
	UIXTestLog( "dial 0 after a drag: %.3f", UIXTestDialValue( apDials[0] ) );
	apDials[0]->RegisterValueUpdateHandler( TestDialUpdate );
	msfDialTarget = 0.6f;
	UIXTestFrame( 1 );
	UIXTestLog( "dial 0 after handler returns 0.6: %.3f", UIXTestDialValue( apDials[0] ) );
	msfDialTarget = -1.0f;
	UIXTestSnapshot( "handled", 0, 0, 380, 90 );
}

//------------------------------------------------------------------------------------------------
// Horizontal scrollbars
//------------------------------------------------------------------------------------------------
static int		msnHScrollLastLogged = -12345;
static int	TestHScrollControl( UIXObject* pObj, BOOL bIsHeld, int nScrollValue, int* pnMaxRange, int* pnViewRange )
{
	if ( ( bIsHeld ) || ( nScrollValue != msnHScrollLastLogged ) )
	{
		UIXTestLog( "hscroll control held %d value %d max %d view %d", bIsHeld, nScrollValue, *pnMaxRange, *pnViewRange );
		msnHScrollLastLogged = nScrollValue;
	}
	return( nScrollValue );
}

UIX_TEST( HorizScrollbar_DragAndCallback )
{
UIXObject*		pPage = UIX::AddPage( UIXRECT( 10, 10, 400, 300 ), "HScroll" );
UIXHorizScrollbar*	pBar = UIX::AddHorizScrollbar( pPage, UIXRECT( 10, 10, 300, 16 ), TestHScrollControl, 0, 25, 100 );
UIXRECT			xRect;

	UIX::AddHorizScrollbar( pPage, UIXRECT( 10, 40, 300, 16 ), NULL, 0, 100, 100 );
	msnHScrollLastLogged = -12345;
	UIXTestFrame( 2 );
	UIXTestLogLayout( NULL, "hscroll" );
	UIXTestSnapshot( "start", 0, 0, 340, 70 );
	xRect = UIXTestRectOf( pBar );

	UIXTestLog( "drag the bar right by 100 pixels:" );
	UIXTestDrag( xRect.x + 20, xRect.y + 8, xRect.x + 120, xRect.y + 8 );
	UIXTestLog( "position %d", UIXTestHorizScrollPosition( pBar ) );
	UIXTestLog( "drag far past the end:" );
	UIXTestDrag( xRect.x + 120, xRect.y + 8, xRect.x + 600, xRect.y + 8 );
	UIXTestLog( "position %d", UIXTestHorizScrollPosition( pBar ) );
	UIX_CHECK( UIXTestHorizScrollPosition( pBar ) == 75, "HorizScrollbar: dragging past the end stops at max - view" );
	UIXTestSnapshot( "end", 0, 0, 340, 70 );
}

//------------------------------------------------------------------------------------------------
// The controller (pad) focus hook in GameCommon UI (UISetControllerSelectableHook). Off by default: every
// other test runs without it, so their references show that nothing changes until an app installs one.
//------------------------------------------------------------------------------------------------
#define		HOOKTEST_MAX_REPORTS	16
static UIControllerSelectable	msaxHookTestReports[HOOKTEST_MAX_REPORTS];
static int						msnHookTestNumReports = 0;

static void	HookTestRecord( const UIControllerSelectable* pSelectable )
{
	if ( msnHookTestNumReports < HOOKTEST_MAX_REPORTS )
	{
		msaxHookTestReports[msnHookTestNumReports++] = *pSelectable;
	}
}

static void	HookTestLogReports( const char* szWhen )
{
int		nLoop;

	UIXTestLog( "%s: %d reported", szWhen, msnHookTestNumReports );
	for ( nLoop = 0; nLoop < msnHookTestNumReports; nLoop++ )
	{
		UIXTestLog( "  kind %d rect %d,%d,%d,%d", msaxHookTestReports[nLoop].nKind, msaxHookTestReports[nLoop].nX, msaxHookTestReports[nLoop].nY, msaxHookTestReports[nLoop].nW, msaxHookTestReports[nLoop].nH );
	}
}

UIX_TEST( ControllerHook_ReportsActivatesAndSteps )
{
UIXObject*		pPage = UIX::AddPage( UIXRECT( 10, 10, 300, 200 ), "Pad" );
int				nButtonID = UIXTestRegisterIDHandler();
int				nSlider = UISliderCreate( 50, 0, 100 );
int				nDropdown = UIDropdownCreate();
UIControllerSelectable	xSelectable;
int				nLoop;
BOOL			bFoundButton = FALSE;

	UIX::AddButton( pPage, UIXRECT( 20, 20, 140, 24 ), UIXBUTTON_NORMAL, "Pad me", nButtonID, 9 );
	UIDropdownAddElement( nDropdown, "One", 1 );
	UIDropdownAddElement( nDropdown, "Two", 2 );
	UIDropdownAddElement( nDropdown, "Three", 3 );

	// No hook (the default): nothing is reported
	UIX_CHECK( UIGetControllerSelectableHook() == NULL, "ControllerHook: off by default" );
	msnHookTestNumReports = 0;
	UIXTestFrame( 1 );
	UIX_CHECK( msnHookTestNumReports == 0, "ControllerHook: nothing reported without a hook" );

	// With one: the UIX button reports its rect, and activating the report presses it as a click would
	UISetControllerSelectableHook( HookTestRecord );
	msnHookTestNumReports = 0;
	UIXTestFrame( 1 );
	HookTestLogReports( "with the hook" );
	for ( nLoop = 0; nLoop < msnHookTestNumReports; nLoop++ )
	{
		if ( ( msaxHookTestReports[nLoop].nKind == UICONTROLLERSELECTABLE_BUTTON ) &&
			 ( msaxHookTestReports[nLoop].nID == nButtonID ) )
		{
			bFoundButton = TRUE;
			xSelectable = msaxHookTestReports[nLoop];
		}
	}
	UIX_CHECK( bFoundButton, "ControllerHook: a UIX button reports itself" );
	if ( bFoundButton )
	{
		UIXTestLog( "activate the button's report:" );
		UIX_CHECK( UIControllerSelectableActivate( &xSelectable ) == TRUE, "ControllerHook: activating a button presses it" );
		UIXTestFrame( 1 );
	}

	// Sliders step by a twentieth of their range and stop at the ends
	memset( &xSelectable, 0, sizeof( xSelectable ) );
	xSelectable.nKind = UICONTROLLERSELECTABLE_SLIDER;
	xSelectable.nID = nSlider;
	UIControllerSelectableStep( &xSelectable, 1 );
	UIXTestLog( "slider after a step right: %d", UISliderGetValue( nSlider ) );
	UIX_CHECK( UISliderGetValue( nSlider ) == 55, "ControllerHook: a slider steps by a twentieth of its range" );
	for ( nLoop = 0; nLoop < 30; nLoop++ )
	{
		UIControllerSelectableStep( &xSelectable, -1 );
	}
	UIX_CHECK( UISliderGetValue( nSlider ) == 0, "ControllerHook: a slider stops at its minimum" );

	// Dropdowns step through their entries and stop at the ends
	xSelectable.nKind = UICONTROLLERSELECTABLE_DROPDOWN;
	xSelectable.nID = nDropdown;
	UIX_CHECK( UIControllerSelectableStep( &xSelectable, -1 ) == FALSE, "ControllerHook: a dropdown stops at its first entry (the first added is selected)" );
	UIControllerSelectableStep( &xSelectable, 1 );
	UIX_CHECK( UIDropdownGetSelectedParam( nDropdown ) == 2, "ControllerHook: a dropdown steps to the next entry" );
	UIControllerSelectableStep( &xSelectable, 1 );
	UIX_CHECK( UIControllerSelectableStep( &xSelectable, 1 ) == FALSE, "ControllerHook: a dropdown stops at its last entry" );
	UIXTestLog( "dropdown after stepping right: param %u", UIDropdownGetSelectedParam( nDropdown ) );
	UIX_CHECK( UIDropdownGetSelectedParam( nDropdown ) == 3, "ControllerHook: a dropdown ends on its last entry" );

	UISetControllerSelectableHook( NULL );
	UISliderDestroy( nSlider );
	UIDropdownDestroy( nDropdown );
}
