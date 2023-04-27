

#ifndef BROWSER_HTML_ELEMENT_H
#define BROWSER_HTML_ELEMENT_H

#include "NoddyBrowser.h"

typedef struct
{
	int		width;
	int		height;
} ELEMENTSIZE;

typedef struct
{
	int		x;
	int		y;
	int		width;
	int		height;

} ELEMENTRECT;

enum
{
	ELEMENT_FLAG_NONE = 0,
	ELEMENT_FLAG_CENTERED = 0x1,
	ELEMENT_FLAG_ITALIC = 0x2,
	ELEMENT_FLAG_BOLD = 0x4,
	ELEMENT_FLAG_RIGHT = 0x8,
	ELEMENT_FLAG_FORCE_WIDTH = 0x10,
	ELEMENT_FLAG_FORCE_HEIGHT = 0x20,
	ELEMENT_FLAG_UNDERLINE = 0x40,
	ELEMENT_FLAG_SMALL = 0x80,
	ELEMENT_FLAG_LARGE = 0x100,
	ELEMENT_FLAG_DROP_SHADOW = 0x200,
	ELEMENT_FLAG_GIANT = 0x400,
};

//--------------------------------------------------
// HTMLElement
//  An abstract base class for all elements
//
class	HTMLElement
{
friend class HTMLElementList;
friend class HTMLTableRow;
friend class HTMLTable;
friend class HTMLTableCell;
public:
	HTMLElement();
	~HTMLElement();

	HTMLElement*	GetParent(void) { return( m_pParentElement ); }
	HTMLElement*	GetChild(void) { return( m_pChildElement ); }
	HTMLElement*	GetBrother(void) { return( m_pBrotherElement ); }

	virtual void	PreChildDisplay( void ) {}
	virtual void	Display( void ) = 0;
	virtual char*	ParseTag( char* ) = 0;
	virtual void	CalcSize( void ) = 0;
	virtual char*	GetName( void ) { return "NULL"; }

	int		GetHeight( void ) { return( m_xActualBounds.height - m_nCursorY ); }
	int		GetWidth( void ) { return( m_xActualBounds.width - m_nCursorX ); }

	int		GetCursorY( void ) { return( m_nCursorY ); }
	int		GetCursorX( void ) { return( m_nCursorX ); }
	int		GetY( void ) { return( m_xActualBounds.y ); }
	int		GetX( void ) { return( m_xActualBounds.x ); }

	void	SetText( char* pszText ) { m_pszText = pszText; }

	void	AddUsedSize( int x, int y ) { m_xUsedSize.width += x; m_xUsedSize.height += y; }

	void	GetCurrentBounds( ELEMENTRECT* );

	void	ChildOccupied( int width, int height, int nPartialWidth, int nPartialHeight );
	void	NewLine( void );
	
	BOOL	ParseIsEndChildTag( char* szTag ) 
	{ 
		if ( stricmp(szTag,GetName()) == 0 )
		{
			return( TRUE );
		}	
		return( FALSE );
	}

	void	SetCursorX( int X );
	void	SetCursorY( int Y ) { m_nCursorY = Y; }
	
	void	SetFlag( int Flag ) { m_Flags |= Flag; }
	int		GetCurrentLineExtentY( void ) { return( m_nCurrentLineExtentY );}
	void	SetCurrentLineExtentY( int Val ) { m_nCurrentLineExtentY = Val;}
protected:
	void	PreDraw( void );
	void	Draw( void );

	uint32	GetColourValue( char* pcVal );
	int		GetWidthValue( char* pcVal );
	int		GetHeightValue( char* pcVal );

	virtual void	SizeChildFromParent( HTMLElement* );

	HTMLElement*	m_pParentElement;
	HTMLElement*	m_pChildElement;
	HTMLElement*	m_pBrotherElement;

	ELEMENTSIZE		m_xSpecifiedSize;
	ELEMENTSIZE		m_xUnrestrictedSize;
	ELEMENTSIZE		m_xUsedSize;

	ELEMENTRECT		m_xActualBounds;

	int			m_nCursorX;
	int			m_nCursorY;
	int			m_nCurrentLineExtentY;
	int		mnCurrentLineHeight;
	uint32	m_CurrentTextCol;
	uint32	m_CurrentLinkCol;
	uint32	m_CurrentHoverLinkCol;
	char*	m_szLink;
	int		m_Flags;

	BYTE	mbCurrentFont;
	BYTE	mbFontChangeFlag;
	BYTE	mbPad2;
	BYTE	mbPad3;

	uint32		m_ulBackgroundColour;

	char*		m_pszName;
	char*		m_pszText;
};

class	HTMLRoot : public HTMLElement
{
public:
	virtual void	CalcSize( void );
	virtual void	Display( void ) {}
	virtual char*	ParseTag( char* ) { return( NULL ); }
	virtual char*	GetName( void ) { return "body"; }
};

//--------------------------------------------------
// HTMLElementList 
//  Used to attach a bunch of HTMLElements together in treelike formation
//
class	HTMLElementList
{
public:
	HTMLElementList();
	void	Reset( void );

	void	AddNewChild( HTMLElement* pParent, HTMLElement* pChild );
	void	Display( int x, int y, int width, int height );

	HTMLElement*	GetRoot(void) { return( &m_Root ); }
private:
	HTMLRoot		m_Root;
};


//---------------------------------------------------------
// List of elements...
//

class	HTMLText : public HTMLElement
{
public:
	virtual void	CalcSize( void );
	void	Display( void );
	virtual char*	ParseTag( char* ptag ) { return ptag; }
};


class HTMLPara : public HTMLText
{
public:
	void	Display(void);
	virtual void	CalcSize( void );
	virtual char*	ParseTag( char* pcRunner );
	virtual char*	GetName( void ) { return "p"; }
};

// HTMLSimpleElement
//  Pure virtual that just implements the standard ParseTag and CalcSize for an element
//  that has no physical appearance and ignores any parameters  
class	HTMLSimpleElement : public HTMLElement
{
public:
	virtual char*	ParseTag( char* pcRunner );
	virtual void	CalcSize( void );
};

class HTMLCenter : public HTMLElement
{
	virtual char*	ParseTag( char* pcRunner );
	virtual void	CalcSize( void );
	void	Display( void );
	virtual char*	GetName( void ) { return "center"; }
};


class HTMLLink : public HTMLElement
{
public:
	virtual char*	ParseTag( char* pcRunner );
	void	Display( void );
	virtual void	CalcSize( void );
	virtual char*	GetName( void ) { return "a"; }
};

class HTMLNewLine : public HTMLElement
{
public:
	virtual char*	ParseTag( char* pcRunner );
	void	Display( void );
	virtual void	CalcSize( void );
	virtual char*	GetName( void ) { return "br"; }
};

class HTMLFont : public HTMLElement
{
public:
	HTMLFont()
	{
		mnFontSize = 3;
		mulNewColour = 0xE0F0F0F0;
	}
	virtual char*	ParseTag( char* pcRunner );
	void	Display( void );
	virtual void	CalcSize( void );
	virtual char*	GetName( void ) { return "font"; }
protected:
	virtual void	SizeChildFromParent( HTMLElement* );
private:
	int		mnFontSize;
	uint32		mulNewColour;

};

class HTMLBody : public HTMLElement
{
public:
	HTMLBody() { m_bTransparentBackground = true; }
	virtual char*	ParseTag( char* pcRunner );
	void	Display( void );
	virtual void	CalcSize( void );
private:
	bool	m_bTransparentBackground;
};

class HTMLItalic : public HTMLSimpleElement
{
public:
	void	Display( void );
	virtual char*	GetName( void ) { return "i"; }
};

class HTMLBold : public HTMLSimpleElement
{
public:
	void	Display( void );
	virtual char*	GetName( void ) { return "b"; }
};
class HTMLUnderline : public HTMLSimpleElement
{
public:
	void	Display( void );
	virtual char*	GetName( void ) { return "ul"; }
};



#endif