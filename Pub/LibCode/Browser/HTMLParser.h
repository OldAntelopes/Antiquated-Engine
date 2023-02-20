

#ifndef BROWSER_HTML_PARSER_H
#define BROWSER_HTML_PARSER_H

#include "HTMLElement.h"

class	HTMLParser
{
public:
	HTMLParser();
	static HTMLParser&		Get();

	void	Init( void );

	void	DisplayDocument( int x, int y, int width, int height );
	void	ParseDocument( char* pcMem );
	BOOL	ParseTag( char* );
	char*	GetNextPropAndVal( char* pcRunner, char** ppcProp, char** ppcVal );
	
	void	Cleanup( void );
private:
	HTMLElementList		m_ElementList;
	HTMLElement*		m_pBuildCurrentElement;

	void	SetEndElement( char* szEndElement );
	void	SetEndElement2( char* szEndElement );
	void	SetEndElement3( char* szEndElement );
	char*	GetEndElement( void ) { return( m_szEndElement ); }
	char*	GetEndElement2( void ) { return( m_szEndElement2 ); }
	char*	GetEndElement3( void ) { return( m_szEndElement3 ); }

	char	m_szEndElement[64];
	char	m_szEndElement2[64];
	char	m_szEndElement3[64];
	char*	mpcRunner;

	BOOL	m_boHasParsedDocument;

};


#endif