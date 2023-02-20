

#ifndef BROWSER_HTML_TABLE_H
#define BROWSER_HTML_TABLE_H

#include "HTMLElement.h"

class HTMLTable : public HTMLElement
{
friend class HTMLTableRow;
friend class HTMLTableCell;

public:
	HTMLTable();
	char* ParseTag( char* pcRunner );
	void	Display( void );
	void	PreChildDisplay( void );
	virtual void	CalcSize( void );
	virtual char*	GetName( void ) { return "table"; }

protected:
	BOOL	m_bWidthFixed;
	BOOL	m_bHeightFixed;

private:
	virtual void ParseExtras(char*,char*) {}
};


class HTMLTableCell : public HTMLTable
{
public:
	void	Display( void );
	void	PreChildDisplay( void );
	virtual void	CalcSize( void );
	void	SizeChildFromParent( HTMLElement* pParent );

	virtual char*	GetName( void ) { return "td"; }

};

class HTMLTableRow : public HTMLTable
{
public:
	void	Display( void );
	void	SizeChildFromParent( HTMLElement* pParent );
	virtual void	CalcSize( void );

	virtual char*	GetName( void ) { return "tr"; }
};



#endif