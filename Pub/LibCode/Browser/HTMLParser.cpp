
#include <stdio.h>
#include <string.h>

#include <StandardDef.h>
#include <Interface.h>
#include <CodeUtil.h>

#include "HTMLImage.h"
#include "HTMLElement.h"
#include "HTMLTable.h"
#include "HTMLParser.h"

#include "NoddyBrowser.h"

HTMLParser		m_HTMLParserSingleton;

HTMLParser::HTMLParser()
{
	m_boHasParsedDocument = FALSE;
	m_ElementList.Reset();
}

HTMLParser&		HTMLParser::Get( void )
{
	return( m_HTMLParserSingleton );
}

char*	HTMLParser::GetNextPropAndVal( char* pcRunner, char** ppcProp, char** ppcVal )
{
char*	pcNextProp;
char*	pcValue;
char*	pcClear;
char*	pParam;

	pcNextProp = pcRunner;
	while ( *pcNextProp == ' ' )
	{
		pcNextProp++;
	}
	pParam = pcNextProp;
	pcValue = SplitStringSep( pcNextProp, '=' );

	if ( *pcValue == '"' )
	{
		pcValue++;
		pcNextProp = SplitStringSep( pcValue, '"' );
	}
	else if (*pcValue == '\'' )
	{
		pcValue++;
		pcNextProp = SplitStringSep( pcValue, '\'' );
	}
	else
	{
		pcNextProp = SplitStringSep( pcValue, ' ' );
	}
	if ( *pcNextProp == 0 )
	{
		pcNextProp = SplitStringSep( pcValue, '>' );
	}

	*ppcProp = pParam;
	
	// If we've got a real value
	if ( *pcValue != 0 )
	{
		if ( *pcValue == '\"' )
		{
			pcValue++;
			pcClear = SplitStringSep( pcValue, '\"' );
		}

		*ppcVal = pcValue;
	}
	else
	{
		*ppcVal = NULL;
	}
	return( pcNextProp );
}

BOOL	HTMLParser::ParseTag( char* pcRunner )
{
char*	pcEndElement =	GetEndElement();
char*	pcEndElement2 =	GetEndElement2();
char*	pcEndElement3 =	GetEndElement3();
char*	pcTagParams;

HTMLElement*	pNewNodeElement = NULL;
HTMLElement*	pNewElement = NULL;
HTMLElement*	pParentElement = NULL;

	pcTagParams = SplitStringSep(pcRunner, ' ' );
	// if there was no space in the tag
	if ( *pcTagParams == 0 )
	{
		pcTagParams = SplitStringSep(pcRunner, '>' );
	}

	// end tag
	if ( *pcRunner == '/' )
	{
		pcRunner++;

		if ( !m_pBuildCurrentElement )
		{
			return( FALSE );
		}

		// Check this is the end of the current section
		if ( m_pBuildCurrentElement->ParseIsEndChildTag( pcRunner ) == TRUE )
		{
			// If so, move back up the tree
			m_pBuildCurrentElement = m_pBuildCurrentElement->GetParent();
			return( FALSE );
		}
		else
		{
			pParentElement = m_pBuildCurrentElement->GetParent();
			while ( pParentElement )
			{
				if ( pParentElement->ParseIsEndChildTag( pcRunner ) == TRUE )
				{
					m_pBuildCurrentElement = pParentElement->GetParent();
					return( FALSE );
				}
				else
				{
					pParentElement = pParentElement->GetParent();
				}
			}
		}
	}
	else	// normal tag
	{
		// New line
		if ( stricmp( pcRunner, "p" ) == 0 )
		{
 			pNewElement = new HTMLPara;
		}
		if ( stricmp( pcRunner, "BR" ) == 0 )
		{
			pNewNodeElement = new HTMLNewLine;
		}
		else if ( stricmp( pcRunner, "li" ) == 0 )
		{
			pNewNodeElement = new HTMLNewLine;
		}
		else if ( stricmp( pcRunner, "dir" ) == 0 )
		{
			pNewNodeElement = new HTMLNewLine;
		}
		else if ( stricmp( pcRunner, "HR" ) == 0 )
		{
			pNewNodeElement = new HTMLNewLine;
		}
		else if ( stricmp( pcRunner, "center" ) == 0 )
		{
 			pNewElement = new HTMLCenter;
		}
		else if ( stricmp( pcRunner, "script" ) == 0 )
		{
//			pSection->SetScript( TRUE );
		}
		else if ( stricmp( pcRunner, "tr" ) == 0 )
		{
 			pNewElement = new HTMLTableRow;
		}
		else if ( stricmp( pcRunner, "td" ) == 0 )
		{
 			pNewElement = new HTMLTableCell;
		}
		else if ( stricmp( pcRunner, "table" ) == 0 )
		{
 			pNewElement = new HTMLTable;
		}
		else if ( stricmp( pcRunner, "img" ) == 0 )
		{
 			pNewNodeElement = new HTMLImage;
		}
		else if ( stricmp( pcRunner, "A" ) == 0 )
		{
 			pNewElement = new HTMLLink;
		}
		else if ( stricmp( pcRunner, "I" ) == 0 )
		{
 			pNewElement = new HTMLItalic;
		}
		else if ( stricmp( pcRunner, "B" ) == 0 )
		{
 			pNewElement = new HTMLBold;
		}
		else if ( stricmp( pcRunner, "BODY" ) == 0 )
		{
 			pNewElement = new HTMLBody;
		}
		else if ( stricmp( pcRunner, "FONT" ) == 0 )
		{
 			pNewElement = new HTMLFont;
		}

		if ( pNewNodeElement )
		{
			m_ElementList.AddNewChild( m_pBuildCurrentElement, pNewNodeElement );
			pNewNodeElement->SetText( pcRunner );
			pNewNodeElement->ParseTag( pcTagParams );
		}
		else if ( pNewElement )
		{
			m_ElementList.AddNewChild( m_pBuildCurrentElement, pNewElement );
			m_pBuildCurrentElement = pNewElement;
			pNewElement->ParseTag( pcTagParams );
		}

	}
	return( TRUE );
}

void	HTMLParser::ParseDocument( char* pcMem )
{
char*	pcElementBuffer = NoddyBrowser::Get().GetElementBuffer();
BOOL	bMoreElementsToParse = TRUE;
BOOL	bMoreBodyToParse = TRUE;
BOOL	bIsTag = FALSE;
char*	pcLastElement;
int		nLen;

	if ( m_boHasParsedDocument == FALSE )
	{
		m_boHasParsedDocument = TRUE;
		mpcRunner = pcMem;
		m_pBuildCurrentElement = m_ElementList.GetRoot();

		while ( bMoreBodyToParse )
		{
			bMoreElementsToParse = TRUE;
			while ( bMoreElementsToParse )
			{
				*pcElementBuffer = 0;
				pcLastElement = mpcRunner;
				mpcRunner = NoddyBrowser::GetNextElement( mpcRunner, pcElementBuffer, &bIsTag );
				nLen = mpcRunner - pcLastElement;
				if ( *pcElementBuffer != 0 )
				{
					if ( stricmp( pcElementBuffer, "/body" ) == 0 )
					{
						bMoreBodyToParse = FALSE;
						bMoreElementsToParse = FALSE;
					}
					else if ( bIsTag )
					{
						bMoreElementsToParse = HTMLParser::Get().ParseTag( pcElementBuffer );
					}
					else
					{
						if ( strlen(pcElementBuffer) > 0 )
						{
							// add a text element...
							HTMLElement* pNewElement = new HTMLText;
							m_ElementList.AddNewChild( m_pBuildCurrentElement, pNewElement );
							pNewElement->SetText(pcElementBuffer);
							pcElementBuffer[nLen] = 0;
							nLen++;
						}
					}
					if ( m_pBuildCurrentElement == NULL )
					{
						bMoreElementsToParse = false;
					}
					pcElementBuffer += nLen;
				}

				if ( *mpcRunner == 0 )
				{
					bMoreBodyToParse = FALSE;
					bMoreElementsToParse = FALSE;
				}
			}
		}
	}
}

void	HTMLParser::DisplayDocument( int x, int y, int width, int height )
{
	m_ElementList.Display( x,y, width, height );

}

void	HTMLParser::Cleanup( void )
{
	m_ElementList.Reset();
	m_boHasParsedDocument = FALSE;
}


