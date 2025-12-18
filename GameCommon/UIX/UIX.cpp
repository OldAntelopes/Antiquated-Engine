
#include "StandardDef.h"

#include "../UI/UI.h"
#include "UIX.h"
#include "UIXButton.h"
#include "UIXDropdown.h"
#include "UIXListBox.h"
#include "UIXPage.h"
#include "UIXSlider.h"
#include "UIXTextBox.h"
#include "UIXText.h"
#include "UIXCollapsableSection.h"

uint32						UIX::msulNextObjectID = 2001;
std::vector<UIXObject*>		UIX::msPagesList;
std::map<uint32, UIXObject*>	UIX::msComponentIDMap;

//--------------------------------------------------------------------------------------
UIXObject::UIXObject( uint32 uID, UIXRECT rect )
{
	mulID = uID;
	mDisplayRect = rect;
	UIX::msComponentIDMap.emplace( uID, this );

}


void	UIXObject::Update( float delta )
{
	OnUpdate( delta );

	for ( UIXObject* pContainedObject : mContainsList )
	{
		pContainedObject->Update( delta );
	}
}

UIXRECT	UIXObject::Render( InterfaceInstance* pInterface, UIXRECT displayRect )
{
UIXRECT		xUsedRect = displayRect;
UIXRECT		xRect;

	xUsedRect.h = 0;

	xRect = OnRender( pInterface, displayRect );
	xUsedRect.h += xRect.h;
	
	if ( !mContainsList.empty() && ShouldDisplayChildren() )
	{
	UIXRECT		xChildDisplayRect = displayRect;
		
		xChildDisplayRect.y += xUsedRect.h;
		xChildDisplayRect.h -= xUsedRect.h;

		for ( UIXObject* pContainedObject : mContainsList )
		{
			xRect = pContainedObject->Render( pInterface, xChildDisplayRect );
			xUsedRect.h += xRect.h;
			xChildDisplayRect.y += xRect.h;
			xChildDisplayRect.h -= xRect.h;
		}
		if ( GetDisplayRect().h > xUsedRect.h )
		{
			xUsedRect.h = GetDisplayRect().h;
		}
	}
	return( xUsedRect );
}

void	UIXObject::Shutdown()
{
	OnShutdown();

	for ( UIXObject* pContainedObject : mContainsList )
	{
		pContainedObject->Shutdown();
	}
}

//--------------------------------------------------------------------------------------------------


void		UIX::ButtonPressHandler( int nButtonID, uint32 ulParam )
{
	switch( nButtonID )
	{
	case UIX_COLLAPSABLE_SECTION_HEADER:
		{
		UIXCollapsableSection*		pCollapsableSection = (UIXCollapsableSection*)msComponentIDMap[ulParam];
			if ( pCollapsableSection )
			{
				pCollapsableSection->ToggleCollapsed();
			}
		}
		
		break;
	
	}
}

void		UIX::SliderHoldHandler( int nButtonID, uint32 ulParam, BOOL bIsHeld, BOOL bFirstPress  )
{
	switch( nButtonID )
	{
	case UIX_SLIDER_BAR:
		{
			UIXSlider*		pSlider = (UIXSlider*)msComponentIDMap[ulParam];
			if ( pSlider )
			{
				pSlider->OnHeldUpdate( bIsHeld, bFirstPress );
			}
		}
		break;
	default:
		break;
	}
}

void		UIX::Initialise( int mode )
{
	UIRegisterButtonPressHandler( UIX_COLLAPSABLE_SECTION_HEADER, ButtonPressHandler );
	UIRegisterHoldHandler( UIX_SLIDER_BAR, SliderHoldHandler );

}

void		UIX::Update( float delta )
{
	for( UIXObject* pxObjects : msPagesList )
	{
		pxObjects->Update( delta );
	}
}

void		UIX::Reset()
{
	for( UIXObject* pxObjects : msPagesList )
	{
		delete pxObjects;
	}
	msPagesList.clear();
}


void		UIX::Render( InterfaceInstance* pxInterface )
{
UIXRECT		pageDisplayRect;

	for( UIXObject* pxObjects : msPagesList )
	{
		pageDisplayRect = pxObjects->GetDisplayRect();
		pxObjects->Render( pxInterface, pageDisplayRect );
	}
}

void		UIX::Shutdown()
{

}

UIXObject*		UIX::AddPage( UIXRECT rect, const char* szTitle  )
{
UIXPage*		pNewPage = new UIXPage( msulNextObjectID++, rect );

	pNewPage->Initialise( szTitle );
	msPagesList.push_back( pNewPage );
	return( pNewPage );
}

UIXText*		UIX::AddText( UIXObject* pxContainer, UIXRECT rect, const char* szTitle, uint32 ulCol, int font, UIX_TEXT_FLAGS fontFlags )
{
UIXText*		pNewText = new UIXText( msulNextObjectID++, rect );
	
	pNewText->Initialise( szTitle, ulCol, font, fontFlags );
	pxContainer->mContainsList.push_back( pNewText );
	return( pNewText );
}

UIXCollapsableSection*		UIX::AddCollapsableSection( UIXObject* pxContainer, UIXRECT rect, int mode, const char* szTitle, BOOL bStartCollapsed )
{
UIXCollapsableSection*		pNewCollapsableSection = new UIXCollapsableSection( msulNextObjectID++, rect );

	pNewCollapsableSection->Initialise( mode, szTitle, bStartCollapsed );
	pxContainer->mContainsList.push_back( pNewCollapsableSection );
	return( pNewCollapsableSection );
}

UIXButton*			UIX::AddButton( UIXObject* pxContainer, UIXRECT rect, int mode, const char* szTitle, uint32 ulButtonID, uint32 ulButtonParam  )
{
UIXButton*		pNewButton = new UIXButton( msulNextObjectID++, rect );

	pNewButton->Initialise( mode, szTitle, ulButtonID, ulButtonParam );
	pxContainer->mContainsList.push_back( pNewButton );
	return( pNewButton );
}

UIXTextBox*			UIX::AddTextBox( UIXObject* pxContainer, UIXRECT rect )
{

	return( NULL );
}

UIXListBox*			UIX::AddListBox( UIXObject* pxContainer, UIXRECT rect )
{

	return( NULL );
}

UIXSlider*			UIX::AddSlider( UIXObject* pxContainer, UIXRECT rect, UIX_SLIDER_MODE mode, uint32 ulUserParam, float fMin, float fMax, float fInitial, float fMinStep )
{
UIXSlider*		pNewSlider = new UIXSlider( msulNextObjectID++, rect );

	pNewSlider->Initialise( mode, ulUserParam, fMin, fMax, fInitial, fMinStep );
	pxContainer->mContainsList.push_back( pNewSlider );
	return( pNewSlider );
}

UIXDropdown*		UIX::AddDropdown( UIXObject* pxContainer, UIXRECT rect )
{

	return( NULL );
}


