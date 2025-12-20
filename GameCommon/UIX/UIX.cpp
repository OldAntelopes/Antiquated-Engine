
#include <stdarg.h>
#include "StandardDef.h"

#include "../UI/UI.h"
#include "UIX.h"
#include "UIXButton.h"
#include "UIXDropdown.h"
#include "UIXListBox.h"
#include "UIXPage.h"
#include "UIXSlider.h"
#include "UIXShape.h"
#include "UIXTextBox.h"
#include "UIXText.h"
#include "UIXCustomRender.h"
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
UIXRECT		xMaxRect;

	xUsedRect.y = 0;
	xUsedRect.h = 0;

	xRect = OnRender( pInterface, displayRect );
	xUsedRect.h = xRect.h;
	xUsedRect.y = xRect.y;
	
	if ( !mContainsList.empty() && ShouldDisplayChildren() )
	{
	UIXRECT		xChildDisplayRect = displayRect;
	int			nCursRelY = 0;
		
		xChildDisplayRect.y += xUsedRect.h;
		xChildDisplayRect.h -= xUsedRect.h;

		for ( UIXObject* pContainedObject : mContainsList )
		{
			xRect = pContainedObject->Render( pInterface, xChildDisplayRect );

			if ( xRect.y + nCursRelY > xMaxRect.y )
			{
				xMaxRect.y = xRect.y + nCursRelY;		
			}
			xUsedRect.h += xRect.h;
			nCursRelY += xRect.h;
			xChildDisplayRect.y += xRect.h;
			xChildDisplayRect.h -= xRect.h;
		}

		int presetChildBlockSize = GetDisplayRect().h;

		xUsedRect.y += xMaxRect.y;
		if ( xUsedRect.y > xUsedRect.h )
		{
			xUsedRect.h = xUsedRect.y;
		}
		// MaxRect contains the lowest point that was drawn to by children including those bits that dont change the cursor
		// and will tell us if we need to expand the box
		if ( xMaxRect.y > presetChildBlockSize )
		{
			if ( xMaxRect.y > xUsedRect.h )
			{
				xUsedRect.h = xMaxRect.y;
			}
		}
		// The usedRect effectively tracks the cursor line position, not always meaningful as the child UI may never adjust it
		else if ( xUsedRect.h < presetChildBlockSize )
		{
//			xUsedRect.h = fPresetChildBlockSize;
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

void		UIX::DeleteObject( UIXObject* pObject )
{
	msPagesList.erase( std::remove(msPagesList.begin(), msPagesList.end(), pObject), msPagesList.end() );
	msComponentIDMap.erase( pObject->GetID() );
	pObject->Shutdown();
	delete pObject;
}

UIXObject*		UIX::AddPage( UIXRECT rect, const char* szTitle  )
{
UIXPage*		pNewPage = new UIXPage( msulNextObjectID++, rect );

	pNewPage->Initialise( szTitle );
	msPagesList.push_back( pNewPage );
	return( pNewPage );
}

UIXText*		UIX::AddText( UIXObject* pxContainer, UIXRECT rect, uint32 ulCol, int font, UIX_TEXT_FLAGS fontFlags, const char* szTitle, ... )
{
UIXText*		pNewText = new UIXText( msulNextObjectID++, rect );
char		acString[1024];
va_list		marker;
uint32*		pArgs;

	pArgs = (uint32*)( &szTitle ) + 1;

    va_start( marker, szTitle );     
	vsprintf( acString, szTitle, marker );
	if ( ulCol == 0 ) ulCol = 0xd0d0d0d0;			// Default col is an offwhite 

	pNewText->Initialise( acString, ulCol, font, fontFlags );
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

UIXCustomRender*	UIX::AddCustomRender( UIXObject* pxContainer, UIXRECT rect, fnCustomRenderCallback renderFunc, uint32 ulUserParam1, uint32 ulUserParam2 )
{
UIXCustomRender*		pNewCustomRender = new UIXCustomRender( msulNextObjectID++, rect );

	pNewCustomRender->Initialise( renderFunc, ulUserParam1, ulUserParam2 );
	pxContainer->mContainsList.push_back( pNewCustomRender );
	return( pNewCustomRender );
}

UIXShape*			UIX::AddShape( UIXObject* pxContainer, UIXRECT rect, int mode, BOOL bBlocks, uint32 ulCol1, uint32 ulCol2 )
{
UIXShape*		pNewShape = new UIXShape( msulNextObjectID++, rect );

	pNewShape->Initialise( mode, bBlocks, ulCol1, ulCol2 );
	pxContainer->mContainsList.push_back( pNewShape );
	return( pNewShape );
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


