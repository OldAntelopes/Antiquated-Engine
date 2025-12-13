
#include <stdio.h>
#include <string.h>

#include <StandardDef.h>
#include <Interface.h>
#include <CodeUtil.h>

#include "HTMLImage.h"
#include "HTMLElement.h"
#include "HTMLParser.h"
#include "NoddyBrowser.h"

//--------------------------------------------------
// HTMLElement

void	HTMLElement::ChildOccupied( int width, int height, int nPartialWidth, int nPartialHeight )
{
	if ( m_nCursorX + width > m_xUsedSize.width )
	{
		m_xUsedSize.width = m_nCursorX + width;
	}
	if ( m_nCursorY + height > m_xUsedSize.height )
	{
		m_xUsedSize.height = m_nCursorY + height;
	}

	if ( nPartialWidth )
	{
		m_nCursorX += nPartialWidth;
		m_nCursorY += height;
		m_nCurrentLineExtentY = nPartialHeight;
	}
	else
	{
		m_nCursorX += width;
		m_nCursorY += height;
	}
}

void	HTMLElement::PreDraw( void )
{
	m_xUnrestrictedSize.width = 0;
	m_xUnrestrictedSize.height = 0;
	if ( m_pChildElement != NULL )
	{
		m_pChildElement->PreDraw();
	}
	CalcSize();
	if ( m_pParentElement )
	{
		m_pParentElement->m_xUnrestrictedSize.width += m_xUnrestrictedSize.width;
		m_pParentElement->m_xUnrestrictedSize.height += m_xUnrestrictedSize.height;
	}
	if ( m_pBrotherElement != NULL )
	{
		m_pBrotherElement->PreDraw();
	}

}

void	HTMLElement::Draw( void )
{
	m_xUsedSize.width = 0;
	m_xUsedSize.height = 0;
	PreChildDisplay();
	if ( m_pChildElement != NULL )
	{
		m_pChildElement->SizeChildFromParent(this);
		m_pChildElement->Draw();
	}
	Display();
	if ( m_pBrotherElement != NULL )
	{
		m_pBrotherElement->SizeChildFromParent(m_pParentElement);
		m_pBrotherElement->Draw();
	}
}



uint32	HTMLElement::GetColourValue( char* pcVal )
{
uint32 ulCol = 0;

	if ( *pcVal == '#' )
	{
	uint32	A;
		ulCol = strtoul( pcVal + 1, NULL, 16 );
		A = ulCol >> 24;
		if ( A == 0 )
		{
			A = 0xD0;
		}
		ulCol &= 0xffffff;
		ulCol |= (A << 24);
	}
	else if ( stricmp( pcVal, "white" ) == 0 )
	{
		ulCol = 0xD0FFFFFF;
	}
	else if ( stricmp( pcVal, "BLACK" ) == 0 )
	{
		ulCol = 0xD0000000;
	}
	else if ( ( stricmp( pcVal, "GREY" ) == 0 ) ||
			  ( stricmp( pcVal, "GRAY" ) == 0 ) )
	{
		ulCol = 0xD0808080;
	}
	else if ( stricmp( pcVal, "RED" ) == 0 )
	{
		ulCol = 0xD0F02000;
	}
	else if ( stricmp( pcVal, "YELLOW" ) == 0 )
	{
		ulCol = 0xD0F0E000;
	}
	return( ulCol );

}

int		HTMLElement::GetWidthValue( char* pcVal )
{
int	nStrlen = strlen( pcVal );
int		Loop = 0;
int		nVal;
	while ( Loop < nStrlen )
	{
		if ( pcVal[Loop] == '%' )
		{
			pcVal[Loop] = 0;
			nVal = strtol( pcVal,NULL,10 );
			nVal = ((m_pParentElement->GetWidth() - m_pParentElement->GetCursorX()) * nVal ) / 100;
			return( nVal );
		}
		Loop++;
	}
	nVal = strtol( pcVal,NULL,10 );
	return( nVal );
}
int		HTMLElement::GetHeightValue( char* pcVal )
{
int	nStrlen = strlen( pcVal );
int		Loop = 0;
int		nVal;
	while ( Loop < nStrlen )
	{
		if ( pcVal[Loop] == '%' )
		{
			pcVal[Loop] = 0;
			nVal = strtol( pcVal,NULL,10 );
			nVal = ((m_pParentElement->GetHeight() - m_pParentElement->GetCursorY()) * nVal ) / 100;
			return( nVal );
		}
		Loop++;
	}
	nVal = strtol( pcVal,NULL,10 );
	if ( nVal == 0 )
	{
		nVal = m_pParentElement->GetHeight() - m_pParentElement->GetCursorY();
	}
	return( nVal );
}


void HTMLFont::CalcSize( void )
{

}

HTMLFont*	mpDebugFont;
char*	HTMLFont::ParseTag( char* pcRunner )
{
char*	pcProp;
char*	pcVal;

	pcRunner = HTMLParser::Get().GetNextPropAndVal( pcRunner, &pcProp, &pcVal );
	while ( pcProp && pcVal )
	{
		if ( stricmp( pcProp, "color" ) == 0 )
		{
			mulNewColour = GetColourValue( pcVal );
			m_CurrentTextCol = mulNewColour;
		}
		else if ( stricmp( pcProp, "dropshadow" ) == 0 )
		{
			m_Flags |= ELEMENT_FLAG_DROP_SHADOW;
		}
		else if ( stricmp( pcProp, "size" ) == 0 )
		{
			mnFontSize = strtol( pcVal, NULL, 10 );
		}
		else if ( stricmp( pcProp, "face" ) == 0 )
		{
			mbFontChangeFlag = 1;
			mbCurrentFont = 6;
			SplitStringSep(pcVal,',');
			if ( stricmp( pcVal, "verdana" ) == 0 )
			{
				mbCurrentFont = 6;
			}
			else if ( stricmp( pcVal, "space1" ) == 0 )
			{
				mbCurrentFont = 2;
			}
			else if ( stricmp( pcVal, "space2" ) == 0 )
			{
				mbCurrentFont = 3;
			}
			else if ( stricmp( pcVal, "arialbold" ) == 0 )
			{
				mbCurrentFont = 0;
			}
			else if ( stricmp( pcVal, "arialboldlarge" ) == 0 )
			{
				mbCurrentFont = 1;
			}
			else if ( stricmp( pcVal, "font3d" ) == 0 )
			{
				mbCurrentFont = 4;
			}
			else if ( stricmp( pcVal, "arial" ) == 0 )
			{
				mbCurrentFont = 5;
			}
		}
		pcRunner = HTMLParser::Get().GetNextPropAndVal( pcRunner, &pcProp, &pcVal );
	}
	return( pcRunner );
}

void	HTMLFont::SizeChildFromParent( HTMLElement* pParent )
{
	HTMLElement::SizeChildFromParent( pParent );
	m_CurrentTextCol = mulNewColour;
}

void	HTMLFont::Display( void )
{
	// For smaller res, we use smaller text more often
	if ( InterfaceGetWidth() < 900 )
	{
		if ( mnFontSize <= 3 )
		{
			m_Flags |= ELEMENT_FLAG_SMALL;
		}
		else if ( mnFontSize >= 5 )
		{
			m_Flags |= ELEMENT_FLAG_LARGE;
		}
		else if ( mnFontSize >= 9 )
		{
			m_Flags |= ELEMENT_FLAG_GIANT;
		}
	}
	else
	{
		if ( mnFontSize <= 2 )
		{
			m_Flags |= ELEMENT_FLAG_SMALL;
		}
		else if ( mnFontSize >= 4 )
		{
			m_Flags |= ELEMENT_FLAG_LARGE;
		}
		else if ( mnFontSize >= 8 )
		{
			m_Flags |= ELEMENT_FLAG_GIANT;
		}
	}
	if ( mulNewColour != 0 )
	{
		m_CurrentTextCol = 	mulNewColour;
	}

	if ( m_pParentElement )
	{
		m_pParentElement->ChildOccupied( m_xUsedSize.width, m_xUsedSize.height, m_nCursorX, m_nCurrentLineExtentY );
	}
}

void	HTMLBody::Display( void )
{
ELEMENTRECT	xRect;

	GetParent()->GetCurrentBounds( &xRect );
	if ( !m_bTransparentBackground )
	{
		m_ulBackgroundColour |= 0xFF000000;
	}
	InterfaceRect( 0, xRect.x, xRect.y, xRect.width, xRect.height, m_ulBackgroundColour );
	NoddyBrowser::Get().SetPageFullHeight( m_xUsedSize.height );
	// For small res, set the font to be small by default
	if ( InterfaceGetWidth() < 950 )
	{
		m_Flags |= ELEMENT_FLAG_SMALL;
	}
}

void HTMLBody::CalcSize( void )
{

}

char*	HTMLBody::ParseTag( char* pcRunner )
{
char*	pcProp;
char*	pcVal;

	pcRunner = HTMLParser::Get().GetNextPropAndVal( pcRunner, &pcProp, &pcVal );
	while ( pcProp && pcVal )
	{
		if ( stricmp( pcProp, "bgcolor" ) == 0 )
		{
			m_ulBackgroundColour = GetColourValue( pcVal );
			m_ulBackgroundColour &= 0xFFFFFF;
			m_ulBackgroundColour |= 0x80000000;
		}
		else if ( stricmp( pcProp, "link" ) == 0 )
		{
			m_CurrentLinkCol = GetColourValue( pcVal );
		}
		else if ( stricmp( pcProp, "alink" ) == 0 )
		{
			m_CurrentHoverLinkCol = GetColourValue( pcVal );
		}
		else if ( stricmp( pcProp, "notrans" ) == 0 )
		{
			m_bTransparentBackground = false;
		}
		pcRunner = HTMLParser::Get().GetNextPropAndVal( pcRunner, &pcProp, &pcVal );
	}
	return( pcRunner );
}


void	HTMLPara::Display( void )
{
	NewLine();
	if ( m_pParentElement )
	{
		m_pParentElement->ChildOccupied( m_xUsedSize.width, m_xUsedSize.height, m_nCursorX, m_nCurrentLineExtentY );
		m_pParentElement->SetCursorX(0);
	}
}

char*	HTMLPara::ParseTag( char* pcRunner )
{
char*	pcProp;
char*	pcVal;

	pcRunner = HTMLParser::Get().GetNextPropAndVal( pcRunner, &pcProp, &pcVal );
	while ( pcProp && pcVal )
	{
		if ( stricmp( pcProp, "align" ) == 0 )
		{
			if (stricmp( pcVal, "center" ) == 0 )
			{
				m_Flags |= ELEMENT_FLAG_CENTERED;			
			}
		}
		pcRunner = HTMLParser::Get().GetNextPropAndVal( pcRunner, &pcProp, &pcVal );
	}
	NewLine();
	return( pcRunner );
}

void HTMLRoot::CalcSize( void )
{
}

void HTMLSimpleElement::CalcSize( void )
{
	m_xUnrestrictedSize.width = 0;
	m_xUnrestrictedSize.height = 0;
}

char*	HTMLSimpleElement::ParseTag( char* pcRunner )
{
char*	pcProp;
char*	pcVal;

	pcRunner = HTMLParser::Get().GetNextPropAndVal( pcRunner, &pcProp, &pcVal );
	while ( pcProp && pcVal )
	{
		pcRunner = HTMLParser::Get().GetNextPropAndVal( pcRunner, &pcProp, &pcVal );
	}
	return( pcRunner );
}

void	HTMLItalic::Display( void )
{
	m_Flags |= ELEMENT_FLAG_ITALIC;
	if ( m_pParentElement )
	{
		m_pParentElement->ChildOccupied( m_xUsedSize.width, m_xUsedSize.height, m_nCursorX, m_nCurrentLineExtentY );
	}
}

void	HTMLBold::Display( void )
{
	m_Flags |= ELEMENT_FLAG_BOLD;
	if ( m_pParentElement )
	{
		m_pParentElement->ChildOccupied( m_xUsedSize.width, m_xUsedSize.height, m_nCursorX, m_nCurrentLineExtentY );
	}
}

void	HTMLUnderline::Display( void )
{
	m_Flags |= ELEMENT_FLAG_BOLD;
	if ( m_pParentElement )
	{
		m_pParentElement->ChildOccupied( m_xUsedSize.width, m_xUsedSize.height, m_nCursorX, m_nCurrentLineExtentY );
	}
}

void HTMLNewLine::CalcSize( void )
{
	m_xUnrestrictedSize.width = 0;
	m_xUnrestrictedSize.height = 13;
}

void	HTMLNewLine::Display( void )
{
	if ( m_pParentElement )
	{
		m_pParentElement->NewLine();
	}
}

char*	HTMLNewLine::ParseTag( char* pcRunner )
{
char*	pcProp;
char*	pcVal;

	pcRunner = HTMLParser::Get().GetNextPropAndVal( pcRunner, &pcProp, &pcVal );
	while ( pcProp && pcVal )
	{
		pcRunner = HTMLParser::Get().GetNextPropAndVal( pcRunner, &pcProp, &pcVal );
	}
	return( pcRunner );
}


void	HTMLCenter::Display( void )
{
	if ( m_pParentElement )
	{
		m_pParentElement->ChildOccupied( m_xUsedSize.width, m_xUsedSize.height, m_nCursorX, m_nCurrentLineExtentY );
	}
}

void	HTMLLink::Display( void )
{
	if ( m_pParentElement )
	{
		m_pParentElement->ChildOccupied( m_xUsedSize.width, m_xUsedSize.height, m_nCursorX, m_nCurrentLineExtentY );
	}
}

char*	HTMLCenter::ParseTag( char* pcRunner )
{
	m_Flags |= ELEMENT_FLAG_CENTERED;
	return( pcRunner );
}

void HTMLPara::CalcSize( void )
{
}

void HTMLCenter::CalcSize( void )
{
}


void HTMLLink::CalcSize( void )
{
}

char*	HTMLLink::ParseTag( char* pcRunner )
	{
	char*	pcProp;
	char*	pcVal;

		pcRunner = HTMLParser::Get().GetNextPropAndVal( pcRunner, &pcProp, &pcVal );
		while ( pcProp && pcVal )
		{
			if ( stricmp( pcProp, "href" ) == 0 )
			{
				m_szLink = pcVal;
				SetText( pcVal );			
			}
			pcRunner = HTMLParser::Get().GetNextPropAndVal( pcRunner, &pcProp, &pcVal );
		}
		return( pcRunner );
	}


//----------------------------------------------------------
// Base class and htmlelementlist for parsing

HTMLElement::~HTMLElement()
{
	if ( m_pChildElement )
	{
		delete m_pChildElement;
		m_pChildElement = NULL;
	}
	if ( m_pBrotherElement )
	{
		delete m_pBrotherElement;
		m_pBrotherElement = NULL;
	}
}

HTMLElement::HTMLElement()
{
	m_pParentElement = NULL;
	m_pChildElement = NULL;
	m_pBrotherElement = NULL;

	ZeroMemory( &m_xActualBounds, sizeof( BOUNDS ) );

	m_xUnrestrictedSize.height = 0;
	m_xUnrestrictedSize.width = 0;
	m_xUsedSize.height = 0;
	m_xUsedSize.width = 0;

	mbCurrentFont = 0;
	mbFontChangeFlag = 0;
	m_ulBackgroundColour = 0;
	m_Flags = 0;
	m_pszName = NULL;
	m_pszText = NULL;
	m_szLink = NULL;

	mnCurrentLineHeight = 13;
	m_CurrentTextCol = 0xD0D0D0D0;
	m_CurrentLinkCol = 0xD0E0D040;
	m_CurrentHoverLinkCol = 0xD0E0E0D0;
	m_nCursorX = 0;
	m_nCursorY = 0;
	m_nCurrentLineExtentY = 0;
	mbPad3 = 0;
	mbPad2 = 0;
	m_xSpecifiedSize = m_xUnrestrictedSize;
}

HTMLElementList::HTMLElementList()
{

}



void HTMLElement::SizeChildFromParent( HTMLElement* pParent )
{
	m_xActualBounds.x = pParent->m_xActualBounds.x;
	m_xActualBounds.y = pParent->m_xActualBounds.y;
	m_xActualBounds.x += pParent->m_nCursorX;
	m_xActualBounds.y += pParent->m_nCursorY;
	m_xActualBounds.width = pParent->m_xActualBounds.width;
	m_xActualBounds.width -= pParent->m_nCursorX;
	m_xActualBounds.height = pParent->m_xActualBounds.height;
	m_xActualBounds.height -= pParent->m_nCursorY;
	m_CurrentLinkCol = pParent->m_CurrentLinkCol;
	m_CurrentHoverLinkCol = pParent->m_CurrentHoverLinkCol;
	m_CurrentTextCol = pParent->m_CurrentTextCol;
	m_nCurrentLineExtentY = pParent->m_nCurrentLineExtentY;
	if ( mbFontChangeFlag == 0 )
	{
		mbCurrentFont = pParent->mbCurrentFont;
	}

	if ( m_Flags & ELEMENT_FLAG_FORCE_WIDTH )
	{
		m_xUnrestrictedSize.width = m_xSpecifiedSize.width;
		if ( m_xUnrestrictedSize.width < m_xActualBounds.width )
		{
			m_xActualBounds.width = m_xUnrestrictedSize.width;
		}
	}

	if ( m_Flags & ELEMENT_FLAG_FORCE_HEIGHT )
	{
		m_xUnrestrictedSize.height = m_xSpecifiedSize.height;
		if ( m_xUnrestrictedSize.height < m_xActualBounds.height )
		{
			m_xActualBounds.height = m_xUnrestrictedSize.height;
		}
	}

	m_nCursorX = 0;
	m_nCursorY = 0;
	// The following flags get inherited by a child from its parent element. (which um, is what inheritance means. so that was pointless)
	if ( (!(m_Flags & ELEMENT_FLAG_CENTERED)) &&
		 ((pParent->m_Flags & ELEMENT_FLAG_CENTERED)) )
	{
		m_Flags |= ELEMENT_FLAG_CENTERED;
	}
	if ( (!(m_Flags & ELEMENT_FLAG_DROP_SHADOW)) &&
		 ((pParent->m_Flags & ELEMENT_FLAG_DROP_SHADOW)) )
	{
		m_Flags |= ELEMENT_FLAG_DROP_SHADOW;
	}
	if ( (!(m_Flags & ELEMENT_FLAG_ITALIC)) &&
		 ((pParent->m_Flags & ELEMENT_FLAG_ITALIC)) )
	{
		m_Flags |= ELEMENT_FLAG_ITALIC;
	}
	if ( (!(m_Flags & ELEMENT_FLAG_BOLD)) &&
		 ((pParent->m_Flags & ELEMENT_FLAG_BOLD)) )
	{
		m_Flags |= ELEMENT_FLAG_BOLD;
	}
	if ( (!(m_Flags & ELEMENT_FLAG_SMALL)) &&
		 ((pParent->m_Flags & ELEMENT_FLAG_SMALL)) )
	{
		m_Flags |= ELEMENT_FLAG_SMALL;
	}
	if ( (!(m_Flags & ELEMENT_FLAG_LARGE)) &&
		 ((pParent->m_Flags & ELEMENT_FLAG_LARGE)) )
	{
		m_Flags |= ELEMENT_FLAG_LARGE;
	}
	if ( (!(m_Flags & ELEMENT_FLAG_GIANT)) &&
		 ((pParent->m_Flags & ELEMENT_FLAG_GIANT)) )
	{
		m_Flags |= ELEMENT_FLAG_GIANT;
	}
	
	if ( m_szLink == 0 )
	{
		m_szLink = pParent->m_szLink;
	}
}


void	HTMLElementList::Display( int x, int y, int width, int height )
{
	m_Root.m_xActualBounds.x = x;
	m_Root.m_xActualBounds.y = y;
	m_Root.m_xActualBounds.width = width;
	m_Root.m_xActualBounds.height = height;

	m_Root.m_xActualBounds = m_Root.m_xActualBounds;
	m_Root.m_nCursorX = 0;
	m_Root.m_nCursorY = 0;
	m_Root.PreDraw();
	m_Root.Draw();
}


void HTMLElementList::AddNewChild( HTMLElement* pParent, HTMLElement* pChild )
{
HTMLElement*	pNewChild;
HTMLElement*	pBrother;

	if ( pParent->m_pChildElement == NULL )
	{
		pParent->m_pChildElement = pChild;
		pChild->m_pParentElement = pParent;
	}
	else
	{
		pNewChild = pParent->m_pChildElement;
		do 
		{	
			pBrother = pNewChild;
			pNewChild = pNewChild->m_pBrotherElement;
		} while ( pNewChild != NULL );

		pBrother->m_pBrotherElement = pChild;
		pChild->m_pParentElement = pParent;
	}
}

void	HTMLElementList::Reset( void )
{
	if ( m_Root.m_pChildElement )
	{
		delete m_Root.m_pChildElement;
		m_Root.m_pChildElement = NULL;
	}
}

void	HTMLElement::GetCurrentBounds( ELEMENTRECT* pRect )
{
	pRect->x = m_xActualBounds.x + m_nCursorX;
	pRect->y = m_xActualBounds.y + m_nCursorY;
	pRect->width = m_xActualBounds.width - m_nCursorX;
	pRect->height = m_xActualBounds.height - m_nCursorY;

}

void	HTMLElement::NewLine( void )
{
	if ( m_nCurrentLineExtentY == 0 )
	{
		m_nCursorY += mnCurrentLineHeight;
	}
	else
	{
		m_nCursorY += m_nCurrentLineExtentY;
	}

	if ( m_nCursorY > m_xUsedSize.height )
	{
		m_xUsedSize.height = m_nCursorY;
	}
	if ( m_nCursorX > m_xUsedSize.width )
	{
		m_xUsedSize.width = m_nCursorX;
	}
	SetCursorX( 0 );
	m_nCurrentLineExtentY = 0;
}


void HTMLText::CalcSize( void )
{
	if ( m_pszText )
	{
	int		nFontNum = mbCurrentFont;
		m_xUnrestrictedSize.width = GetStringWidth( m_pszText, nFontNum );
		m_xUnrestrictedSize.height = 13;
	}
}


void HTMLText::Display( void )
{
int		nWidthAvailable;
char*	pcTextToDisplay = m_pszText;
char*	pcEndOfFirstLine = NULL;
int		nFontFlags = 0;
uint32	ulCol = 0xD0E0E0E0;
uint32	ulRealCol = 0xD0E0E0E0;
int		nWidthDisplayed = 0;
int		nRealY;
int		nFontNum = mbCurrentFont;
BOOL	bShouldDisplay = TRUE;
//	if ( m_bIsItalic ) nFontFlags |= FONT_FLAG_ITALIC;
	if ( !pcTextToDisplay ) return;

	do 
	{
		nWidthAvailable = m_xActualBounds.width - m_nCursorX;
		if ( nWidthAvailable <= 0 )
		{
			nWidthAvailable = GetStringWidth( pcTextToDisplay, nFontNum ) + 1;
		}

		if ( m_Flags & ELEMENT_FLAG_ITALIC )
		{
			nFontFlags |= FONT_FLAG_ITALIC;
		}
		if ( m_Flags & ELEMENT_FLAG_BOLD )
		{
			nFontFlags |= FONT_FLAG_BOLD;
		}
		if ( m_Flags & ELEMENT_FLAG_SMALL )
		{
			nFontFlags |= FONT_FLAG_SMALL;
		}
		if ( m_Flags & ELEMENT_FLAG_LARGE )
		{
			nFontFlags |= FONT_FLAG_LARGE;
		}
		if ( m_Flags & ELEMENT_FLAG_GIANT )
		{
			nFontFlags |= FONT_FLAG_GIANT;
		}
		if ( m_Flags & ELEMENT_FLAG_DROP_SHADOW )
		{
			nFontFlags |= FONT_FLAG_DROP_SHADOW;
		}
		
		if ( m_szLink == NULL )
		{
			InterfaceSetFontFlags(nFontFlags);
			ulCol = m_CurrentTextCol;
		}
		else	// Text is a link
		{
		BOUNDS	xBounds;
		int		nWidthDrawn = GetStringWidth( pcTextToDisplay, nFontNum );
			if ( nWidthDrawn > nWidthAvailable )
			{
				nWidthDrawn = nWidthAvailable;
			}
			nFontFlags |= FONT_FLAG_UNDERLINED|FONT_FLAG_DROP_SHADOW;
			InterfaceSetFontFlags(nFontFlags);
			xBounds.nMinX = m_xActualBounds.x + m_nCursorX;
			xBounds.nMaxX = m_xActualBounds.x + m_nCursorX + nWidthDrawn;	
			xBounds.nMinY = m_xActualBounds.y + m_nCursorY;
			xBounds.nMaxY = m_xActualBounds.y + m_nCursorY + mnCurrentLineHeight;
			if (  m_Flags & ELEMENT_FLAG_CENTERED )
			{
				xBounds.nMinX = m_xActualBounds.x + m_nCursorX + ((m_xActualBounds.width/2) - (nWidthDrawn / 2));
				xBounds.nMaxX = m_xActualBounds.x + m_nCursorX + ((m_xActualBounds.width/2) + (nWidthDrawn / 2));
				xBounds.nMinY = m_xActualBounds.y + m_nCursorY;
				xBounds.nMaxY = m_xActualBounds.y + m_nCursorY + mnCurrentLineHeight;
			}

			if ( NoddyBrowser::Get().IsMouseOver( &xBounds ) == TRUE )
			{
				DoSetMouseOverCursor( TRUE );
				NoddyBrowser::Get().SetActiveLink( m_szLink, &xBounds );
				ulCol =  m_CurrentHoverLinkCol;
			}
			else
			{
				ulCol = m_CurrentLinkCol;
			}
		}

		ulRealCol = ulCol;
		nRealY = m_xActualBounds.y + m_nCursorY;
		nRealY -= NoddyBrowser::Get().GetScrollTop();
		if ( ( nRealY < NoddyBrowser::Get().GetDisplayTop() ) ||
			 ( nRealY > NoddyBrowser::Get().GetDisplayBottom() - mnCurrentLineHeight ) )
		{
			bShouldDisplay = FALSE;
		}

		mnCurrentLineHeight = GetStringHeight( pcTextToDisplay, nFontNum ) - 1;

		if ( m_Flags & ELEMENT_FLAG_CENTERED )
		{
		int		nStringWidth = GetStringWidth(pcTextToDisplay, nFontNum );

			if ( nStringWidth < nWidthAvailable )
			{
				if ( m_nCursorX == 0 )
				{
					if ( bShouldDisplay )
					{
						InterfaceTextCenter( 2, m_xActualBounds.x + m_nCursorX, m_xActualBounds.x + m_xActualBounds.width, nRealY, pcTextToDisplay, ulRealCol, nFontNum );
					}
					m_nCursorX += ( ((m_xActualBounds.width - m_nCursorX) / 2) - (nStringWidth/2) );
				}
				else
				{
					if ( bShouldDisplay )
					{
						InterfaceTextLimitWidth( 2, m_xActualBounds.x + m_nCursorX, nRealY, pcTextToDisplay, ulRealCol, nFontNum, nWidthAvailable );			
					}
				}
				pcEndOfFirstLine = NULL;
			}
			else
			{
				if ( bShouldDisplay )
				{
					pcEndOfFirstLine = InterfaceTextLimitWidth( 2, m_xActualBounds.x + m_nCursorX, nRealY, pcTextToDisplay, ulRealCol, nFontNum, nWidthAvailable );
				}
				else
				{
					pcEndOfFirstLine = InterfaceTextLimitWidth( 2, m_xActualBounds.x + m_nCursorX, nRealY, pcTextToDisplay, 0, nFontNum, nWidthAvailable );
				}
			}
		}
		else
		{
			if ( bShouldDisplay )
			{
				pcEndOfFirstLine = InterfaceTextLimitWidth( 2, m_xActualBounds.x + m_nCursorX, nRealY, pcTextToDisplay, ulRealCol, nFontNum, nWidthAvailable );
			}
			else
			{
				pcEndOfFirstLine = InterfaceTextLimitWidth( 2, m_xActualBounds.x + m_nCursorX, nRealY, pcTextToDisplay, 0, nFontNum, nWidthAvailable );
			}
		}
	
		if ( pcEndOfFirstLine == pcTextToDisplay )
		{
			pcEndOfFirstLine = NULL;
		}
		nWidthDisplayed = GetStringWidth( pcTextToDisplay, nFontNum);
		if ( nWidthDisplayed > nWidthAvailable )
		{
			nWidthDisplayed = nWidthAvailable;
		}
		InterfaceSetFontFlags(0);
		m_nCursorX += nWidthDisplayed;
		if ( m_nCursorX > m_xUsedSize.width )
		{
			m_xUsedSize.width = m_nCursorX;
		}

		if ( ( pcEndOfFirstLine ) &&
			 ( pcEndOfFirstLine[0] != 0 ))
		{
			NewLine();
			if ( m_pParentElement )
			{
				m_pParentElement->SetCursorX(0);
				m_xActualBounds.x = m_pParentElement->GetX();
				m_xActualBounds.width = m_pParentElement->GetWidth();
			}
			if ( pcEndOfFirstLine[0] == ' ' )
			{
				pcEndOfFirstLine++;
			}
			nWidthDisplayed = 0;
		}
		pcTextToDisplay = pcEndOfFirstLine;

	} while ( pcEndOfFirstLine != NULL );
	

	if ( m_pParentElement )
	{
		m_pParentElement->ChildOccupied( m_xUsedSize.width, m_xUsedSize.height, nWidthDisplayed, mnCurrentLineHeight );
	}
}


void	HTMLElement::SetCursorX( int X )
{
	m_nCursorX = X; 
	if ( m_pParentElement ) 
	{
		m_pParentElement->SetCursorX(X);
	}
}