
#include <stdio.h>
#include <string.h>

#include <StandardDef.h>
#include <Interface.h>
extern "C"
{
extern void	DoSetMouseOverCursor( BOOL );
}

#include "HTMLImage.h"
#include "HTMLTable.h"
#include "HTMLParser.h"
#include "NoddyBrowser.h"



//-----------------------------------------------------------------------------------

HTMLTable::HTMLTable()
{
	m_bWidthFixed = FALSE;
	m_bHeightFixed = FALSE;
	m_ulBackgroundColour = 0;
}


void	HTMLTableCell::PreChildDisplay( void )
{
	if ( m_pParentElement )
	{
		m_xActualBounds.y = m_pParentElement->m_xActualBounds.y;
		m_xActualBounds.x = m_pParentElement->m_xActualBounds.x + m_pParentElement->GetCursorX();
		m_xActualBounds.width = m_pParentElement->m_xActualBounds.width - m_pParentElement->GetCursorX();
		m_xActualBounds.height = m_pParentElement->m_xActualBounds.height;
	}
}

void	HTMLTable::PreChildDisplay( void )
{
	if ( m_pParentElement )
	{
		m_xActualBounds.y = m_pParentElement->m_xActualBounds.y + m_pParentElement->GetCursorY();
		m_xActualBounds.x = m_pParentElement->m_xActualBounds.x;
		m_xActualBounds.width = m_pParentElement->m_xActualBounds.width;
	}
}

void	HTMLTableCell::Display( void )
{
	if ( m_pParentElement )
	{
		m_pParentElement->ChildOccupied( m_xUsedSize.width, 0, 0, 0 );
		m_pParentElement->AddUsedSize( 0, m_xUsedSize.height );
		m_pParentElement->SetCursorY( 0 );
	}
}



void	HTMLTableRow::Display( void )
{
	if ( m_pParentElement )
	{
	 	if ( m_xUsedSize.height - m_nCursorY == 0 )
		{
			m_xUsedSize.height += 13;
		}
		m_pParentElement->ChildOccupied( 0, m_xUsedSize.height, 0, 0 );
		m_pParentElement->AddUsedSize( m_xUsedSize.width, 0 );
		m_pParentElement->m_nCursorX = 0;
	}
}

void	HTMLTableRow::CalcSize( void )
{


}
void	HTMLTableCell::CalcSize( void )
{


}
void	HTMLTable::CalcSize( void )
{


}

void	HTMLTable::Display( void )
{
	if ( m_pParentElement )
	{
		m_pParentElement->ChildOccupied( m_xUsedSize.width, m_xUsedSize.height, 0, 0 );
		m_pParentElement->SetCursorX(0);
	}
}

char* HTMLTable::ParseTag( char* pcRunner )
{
char*	pcProp;
char*	pcVal;
int		nTableWidth = 0;
int	nX;
int	nY;
int nHeight = 0;
int	nWidthAvail;
int	nHeightAvail;

	pcRunner = HTMLParser::Get().GetNextPropAndVal( pcRunner, &pcProp, &pcVal );
	while ( pcProp && pcVal )
	{
		if ( stricmp( pcProp, "width" ) == 0 )
		{		
			nTableWidth = GetWidthValue(pcVal );
			m_Flags |= ELEMENT_FLAG_FORCE_WIDTH;
			m_xSpecifiedSize.width = nTableWidth;
		}
		else if ( stricmp( pcProp, "height" ) == 0 )
		{
			nHeight = GetHeightValue(pcVal );	
			m_xSpecifiedSize.height = nHeight;
			m_Flags |= ELEMENT_FLAG_FORCE_HEIGHT;
		}
		else if ( stricmp( pcProp, "bgcolor" ) == 0 )
		{
			m_ulBackgroundColour = GetColourValue( pcVal );
		}
		else
		{
			ParseExtras(pcProp,pcVal);
		}
		pcRunner = HTMLParser::Get().GetNextPropAndVal( pcRunner, &pcProp, &pcVal );
	}

	nHeightAvail = m_pParentElement->GetHeight() - m_pParentElement->GetCursorY(); 
	nWidthAvail = m_pParentElement->GetWidth() - m_pParentElement->GetCursorX();
	nX = m_pParentElement->GetX() + m_pParentElement->GetCursorX();
	nY = m_pParentElement->GetY() + m_pParentElement->GetCursorY();
	if ( ( nTableWidth == 0 ) ||
		 ( nTableWidth > nWidthAvail ) )
	{
		nTableWidth = nWidthAvail;
	}
	if ( ( nHeight == 0 ) ||
		 ( nHeight > nHeightAvail ) )
	{
		nHeight = nHeightAvail;
	}

	return( pcRunner );
}



void HTMLTableCell::SizeChildFromParent( HTMLElement* pParent )
{
	HTMLElement::SizeChildFromParent( pParent );
}


void HTMLTableRow::SizeChildFromParent( HTMLElement* pParent )
{
	HTMLElement::SizeChildFromParent( pParent );
	m_Flags &= !ELEMENT_FLAG_CENTERED;
}

