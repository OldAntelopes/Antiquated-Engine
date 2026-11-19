#ifndef UIX_TEST_ACCESS_H
#define UIX_TEST_ACCESS_H

//------------------------------------------------------------------------------------------------
// UIXTestAccess.h
//
// Every UIX header, plus read access to the private state the tests check (expanded, selected index,
// scroll position, the rect each object last drew at...) without UIX growing accessors just for tests.
// UIXTestAccess.cpp reaches the members with the explicit-instantiation idiom, which C++ exempts from
// access checks, so UIX's headers and binaries are untouched.
//
// Tests only read state through these. The writes (UIXTestResetUIXStatics, UIXTestSetLastDisplayRect)
// are for the harness between tests and for the layout dump, never part of what a test checks.
//------------------------------------------------------------------------------------------------

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <typeinfo>

#include <StdWinInclude.h>
#include "DirectX/d3dx9.h"			// before Interface.h, which declares InterfaceGetD3DDevice only when D3D is included
#include "StandardDef.h"
#include "Interface.h"
#include "InterfaceEx.h"

#include "../../../GameCommon/UI/UI.h"
#include "../../../GameCommon/UIX/UIX.h"
#include "../../../GameCommon/UIX/UIXButton.h"
#include "../../../GameCommon/UIX/UIXCheckbox.h"
#include "../../../GameCommon/UIX/UIXCollapsableSection.h"
#include "../../../GameCommon/UIX/UIXCustomRender.h"
#include "../../../GameCommon/UIX/UIXDial.h"
#include "../../../GameCommon/UIX/UIXScrollableSection.h"
#include "../../../GameCommon/UIX/UIXDropdown.h"
#include "../../../GameCommon/UIX/UIXHorizScrollbar.h"
#include "../../../GameCommon/UIX/UIXListBox.h"
#include "../../../GameCommon/UIX/UIXMenu.h"
#include "../../../GameCommon/UIX/UIXModalPopup.h"
#include "../../../GameCommon/UIX/UIXPage.h"
#include "../../../GameCommon/UIX/UIXPopupMenu.h"
#include "../../../GameCommon/UIX/UIXShape.h"
#include "../../../GameCommon/UIX/UIXSlider.h"
#include "../../../GameCommon/UIX/UIXTabBar.h"
#include "../../../GameCommon/UIX/UIXText.h"
#include "../../../GameCommon/UIX/UIXTextBox.h"
#include "../../../GameCommon/Platform/Platform.h"

//---------------------------------------------- UIX statics
extern std::vector<UIXPage*>&	UIXTestPages( void );
extern UIXObject*	UIXTestMousewheelHoverObject( void );
extern UIXObject*	UIXTestDragDestinationHover( void );
extern int			UIXTestDragItemType( void );
extern const char*	UIXTestTooltipText( void );
extern int			UIXTestIconSheetHandle( int nSheet );
extern void			UIXTestResetUIXStatics( void );

//---------------------------------------------- UIXObject
extern std::vector<UIXObject*>&	UIXTestChildren( UIXObject* pObject );
extern UIXRECT		UIXTestLastDisplayRect( UIXObject* pObject );
extern void			UIXTestSetLastDisplayRect( UIXObject* pObject, UIXRECT xRect );
extern UIXRECT		UIXTestActualRect( UIXObject* pObject );		// GetActualRenderRect( last display rect )
extern BOOL			UIXTestIsBeingDragged( UIXObject* pObject );
extern int			UIXTestChildContentsHeight( UIXObject* pObject );

//---------------------------------------------- Widgets
extern const char*	UIXTestButtonTitle( UIXButton* pButton );
extern const char*	UIXTestTextString( UIXText* pText );
extern void			UIXTestShapeAnimation( UIXShape* pShape, float* pfTime, float* pfPhase );
extern BOOL			UIXTestDropdownIsExpanded( UIXDropdown* pDropdown );
extern int			UIXTestDropdownSelectedIndex( UIXDropdown* pDropdown );
extern int			UIXTestDropdownScrollPosition( UIXDropdown* pDropdown );
extern BOOL			UIXTestIsCollapsed( UIXCollapsableSection* pSection );
extern void			UIXTestSliderValues( UIXSlider* pSlider, float* pfCurrent, float* pfMin, float* pfMax );
extern UIXRECT		UIXTestSliderRenderRect( UIXSlider* pSlider );
extern UIXTextBox*	UIXTestSliderTextBox( UIXSlider* pSlider, int nWhich );		// 0 value, 1 range min, 2 range max
extern float		UIXTestDialValue( UIXDial* pDial );
extern int			UIXTestHorizScrollPosition( UIXHorizScrollbar* pScrollbar );
extern BOOL			UIXTestTabIsSelected( UIXTabBar* pTabBar, int nIndex );
extern UIXRECT		UIXTestListBoxEntryRect( UIXListBox* pListBox, int nFlatIndex );
extern UIXRECT		UIXTestScrollableSectionBarRect( UIXScrollableSection* pSection );

#endif
