
#include "string.h"
#include <StandardDef.h>
#include <Interface.h>

#include "../Platform/Platform.h"
#include "UI.h"
#include "UIDropdown.h"

class UIDropdownListItem
{
public:
	UIDropdownListItem()
	{
		m_pNext = NULL;
		m_szTitle = NULL;
		m_param = 0;
	}
	
	~UIDropdownListItem()
	{
		SAFE_FREE( m_szTitle );
	}
		
	void		SetItem( const char* szTitle, int nParam );

	const char*				GetTitle( void ) { return( m_szTitle ); }
	int						GetParam( void ) { return( m_param ); }

	UIDropdownListItem*		GetNext( void ) { return( m_pNext ); }
	void					SetNext( UIDropdownListItem* pNext ) { m_pNext = pNext; }
private:
	UIDropdownListItem*		m_pNext;
	char*					m_szTitle;
	int						m_param;
};

void	UIDropdownListItem::SetItem( const char* szTitle, int nParam )
{
	if ( m_szTitle != NULL )
	{
		free( m_szTitle );
	}
	m_szTitle = (char*)malloc( strlen( szTitle ) + 1 );
	strcpy( m_szTitle, szTitle );
	m_param = nParam;
}

//-------------------------------------------------------------------

class UIDropdown
{
public:
	UIDropdown()
	{
		m_pNext = NULL;
		m_Handle = NOTFOUND;
		m_pSelectedItem = NULL;
		m_pItems = NULL;
		m_MouseHoverItem = NOTFOUND;
		m_bIsShowingList = false;
		m_numItems = 0;
		m_FirstInView = 0;
		m_ItemsInView = 0;
	}
	~UIDropdown()
	{
	UIDropdownListItem*	pItems = m_pItems;
	UIDropdownListItem*	pNext;

		while( pItems )
		{
			pNext = pItems->GetNext();
			delete pItems;
			pItems = pNext;
		}
		m_pItems = NULL;
	}
	
	UIDropdownListItem*			AddItem( const char* szTitle, int nParam );
	void			Reset( void );

	void			Render( int x, int y, int w, int h, int nMaxH, float fAlpha );

	UIDropdownListItem*			GetSelectedItem( void ) { return( m_pSelectedItem ); }
	void						SetSelectedItemIndex( int index );
	void						SetSelectedItemParam( int param );
	int							GetNumElements( void );

	BOOL			OnMouseWheelMove( float fOffset );
	BOOL			OnRelease( int X, int Y );
	BOOL			HasMouseFocus( void );

	void			SetHandle( int nHandle ) { m_Handle = nHandle; }
	int				GetHandle( void ) { return( m_Handle ); }
	UIDropdown*		GetNext( void ) { return( m_pNext ); }
	void			SetNext( UIDropdown* pNext ) { m_pNext = pNext; }
private:
	UIDropdownListItem*		m_pItems;

	UIDropdownListItem*		m_pSelectedItem;
	UIDropdownListItem*		m_pHoverItem;

	int				m_numItems;
	UIDropdown*		m_pNext;
	int				m_Handle;
	int				m_MouseHoverItem;
	bool			m_bIsShowingList;
	int				m_FirstInView;
	int				m_ItemsInView;
};

void		UIDropdown::Reset( void )
{
UIDropdownListItem*	pNext;
UIDropdownListItem*	pItem = m_pItems;

	while ( pItem )
	{
		pNext = pItem->GetNext();
		delete pItem;
		pItem = pNext;
	}
	m_numItems = 0;
	m_pItems = NULL;
	m_pSelectedItem = NULL;
	m_pHoverItem = NULL;
	m_bIsShowingList = false;
}

int			UIDropdown::GetNumElements( void )
{
	return( m_numItems );
}

void				UIDropdown::SetSelectedItemParam( int param )
{
UIDropdownListItem*	pItem = m_pItems;

	while ( pItem )
	{
		if ( pItem->GetParam() == param )
		{
			m_pSelectedItem = pItem;
			return;
		}
		pItem = pItem->GetNext();
	}
}

void				UIDropdown::SetSelectedItemIndex( int index )
{
UIDropdownListItem*	pItem = m_pItems;
int		nCount = 0;

	while ( pItem )
	{
		if ( nCount == index )
		{
			m_pSelectedItem = pItem;
			return;
		}
		nCount++;
		pItem = pItem->GetNext();
	}

}


UIDropdownListItem*		UIDropdown::AddItem( const char* szTitle, int nParam )
{
UIDropdownListItem*	pNewItem = new UIDropdownListItem;
UIDropdownListItem*	pList;

	pNewItem->SetItem( szTitle, nParam );

	if ( m_pItems == NULL )
	{
		m_pItems = pNewItem;
	}
	else
	{
		pList = m_pItems;
		while ( pList->GetNext() != NULL )
		{
			pList = pList->GetNext();
		}
		pList->SetNext( pNewItem );
	}

	if ( m_pSelectedItem == NULL )
	{
		m_pSelectedItem = pNewItem;
	}
	m_numItems++;

	return( pNewItem );
}


UIDropdown*		m_spUIActiveDropdowns = NULL;
int				m_snUINextDropdownHandle = 1;
bool			m_sbUIDropdownIsAnyListActive = false;
	

void	UIDropdownDestroy( int handle )
{
UIDropdown*		pDropdowns = m_spUIActiveDropdowns;
UIDropdown*		pLast = NULL;

	while( pDropdowns )
	{
		if ( pDropdowns->GetHandle() == handle )
		{
			if ( pLast == NULL )
			{
				m_spUIActiveDropdowns = pDropdowns->GetNext();
			}
			else
			{
				pLast->SetNext( pDropdowns->GetNext() );
			}
			delete pDropdowns;
			return;
		}
		else
		{
			pLast = pDropdowns;
		}
		pDropdowns = pDropdowns->GetNext();
	}

}

int		UIDropdownCreate( void )
{
UIDropdown*		pNewDropdown = new UIDropdown;
int				nHandle = m_snUINextDropdownHandle++;

	pNewDropdown->SetHandle( nHandle );
	pNewDropdown->SetNext( m_spUIActiveDropdowns );
	m_spUIActiveDropdowns = pNewDropdown;
	return( nHandle );
}

UIDropdown*		DropdownFind( int nHandle )
{
UIDropdown*		pDropdown = m_spUIActiveDropdowns;
	while ( pDropdown )
	{
		if ( pDropdown->GetHandle() == nHandle )
		{
			return( pDropdown );
		}
		pDropdown = pDropdown->GetNext();
	}
	return( NULL );
}

void		DropdownSetSelectedParam( int nHandle, int nParam )
{
UIDropdown*		pDropdown = DropdownFind( nHandle );

	if ( pDropdown )
	{
		pDropdown->SetSelectedItemParam( nParam );
	}
}
		
void		DropdownSetSelected( int nHandle, int nIndex )
{
UIDropdown*		pDropdown = DropdownFind( nHandle );

	if ( pDropdown )
	{
		pDropdown->SetSelectedItemIndex( nIndex );
	}
}


int		UIDropdownAddElement( int nHandle, const char* szElementName, ulong ulElementParam )
{
UIDropdown*		pDropdown = DropdownFind( nHandle );

	if ( pDropdown )
	{
	UIDropdownListItem*		pListItem;

		pListItem = pDropdown->AddItem( szElementName, ulElementParam );
		return( 1 );
	}
	return( 0 );
}

int		UIDropdownGetNumElements( int nHandle )
{
UIDropdown*		pDropdown = DropdownFind( nHandle );

	if ( pDropdown )
	{
		return( pDropdown->GetNumElements() );
	}
	return( 0 );

}

ulong	UIDropdownGetSelectedParam( int nHandle )
{
UIDropdown*		pDropdown = DropdownFind( nHandle );

	if ( pDropdown )
	{
	UIDropdownListItem*		pListItem = pDropdown->GetSelectedItem();
		
		if ( pListItem )
		{
			return( pListItem->GetParam() );
		}
	}
	return( NOTFOUND );
}


BOOL		UIDropdown::HasMouseFocus( void )
{
	if ( ( m_MouseHoverItem != NOTFOUND ) ||
		 ( m_bIsShowingList ) )
	{
		return( TRUE );
	}
	return( FALSE );
}


BOOL		UIDropdown::OnMouseWheelMove( float fOffset )
{
	if ( m_bIsShowingList )
	{
		m_FirstInView -= (int)( fOffset );
	
		if ( m_FirstInView < 0 )
		{
			m_FirstInView = 0;
		}
		else if ( m_FirstInView + m_ItemsInView >= m_numItems )
		{
			m_FirstInView = m_numItems - m_ItemsInView;
		}
		return( TRUE );
	}
	return( FALSE );
}


BOOL		UIDropdown::OnRelease( int X, int Y )
{
	if ( m_MouseHoverItem != NOTFOUND )
	{
		if ( m_MouseHoverItem == 0 )
		{
			m_bIsShowingList = !m_bIsShowingList;
		}
		else if ( m_MouseHoverItem == 1 )
		{
			m_pSelectedItem = m_pHoverItem;
			m_bIsShowingList = false;
		}
		m_MouseHoverItem = NOTFOUND;
		return( TRUE );
	}
	else 
	{
		if ( m_bIsShowingList )
		{
			m_bIsShowingList = false;
			return( TRUE );
		}
	}
	return( FALSE );
}

void		UIDropdown::Render( int X, int Y, int W, int H, int nMaxH, float fAlpha )
{
int		nTextY = Y + ((H-16)/2);
int		nItemSize = 40;
int	nTexOffY = (nItemSize - 24) / 2;
ulong	ulCol;

	if ( nTexOffY < 0 ) nTexOffY = 0;

	if ( nTextY < Y + 1 ) nTextY = Y + 1;

	m_MouseHoverItem = NOTFOUND;

	ulCol = GetColWithModifiedAlpha( 0xc0808080, fAlpha );
	InterfaceOutlineBox( 2, X, Y, W, H, ulCol );
	ulCol = GetColWithModifiedAlpha( 0x80000000, fAlpha );
	InterfaceRect( 0, X, Y, W, H, ulCol  );

	if ( m_pSelectedItem )
	{
	const char*		pszText = m_pSelectedItem->GetTitle();
		ulCol = GetColWithModifiedAlpha( 0xC0D0D0D0, fAlpha );
		InterfaceText( 0, X + 5, nTextY, pszText, ulCol, 1 );
	} 

	if ( m_bIsShowingList )
	{
	int		nListHeight = m_numItems * (nItemSize+2);
	int		liney = Y + H + 4;
	UIDropdownListItem*	pItem = m_pItems;
	int		nMaxListHeight;
	BOOL	bDisplayingExtraItems = FALSE;
	int		nListCount = 0;

		nMaxListHeight = nMaxH;
		if ( nListHeight > nMaxListHeight )
		{
			nListHeight = nMaxListHeight;
			bDisplayingExtraItems = TRUE;
		}

		m_sbUIDropdownIsAnyListActive = true;
		InterfaceOutlineBox( 1, X, Y+H+2, W, nListHeight + 2, 0xC0808080 );
		InterfaceRect( 2, X, Y+H+2, W, nListHeight + 2, 0xE0000000 );

		// TODO - SCROLLBAR
		if ( bDisplayingExtraItems )
		{
		int		nTriX1 = X + (W/2) - 40;
		int		nTriX2 = X + (W/2);
		int		nTriX3 = X + (W/2) + 40;
		int		nTriY1 = Y + H + 2;
		int		nTriY2 = Y + H + 20;
		ulong	ulTriCol = 0xD0C0C0C0;

			nMaxListHeight = Y + H + nListHeight - 36;

			m_ItemsInView = (nMaxListHeight - liney) / 17;
			liney += nItemSize + 1;

			if ( m_FirstInView == 0 )
			{
				ulTriCol = 0xA0808080;
			}

			InterfaceTri( 2, nTriX1, nTriY2, nTriX2, nTriY1, nTriX3, nTriY2, ulTriCol, ulTriCol, ulTriCol );

			if ( m_FirstInView + m_ItemsInView < m_numItems )
			{
				ulTriCol = 0xD0C0C0C0;
			}
			else
			{
				ulTriCol = 0xA0808080;
			}

			nTriY1 = Y + H + nListHeight - 23;
			nTriY2 = Y + H + nListHeight - 3;
			InterfaceTri( 2, nTriX1, nTriY1, nTriX2, nTriY2, nTriX3, nTriY1, ulTriCol, ulTriCol, ulTriCol );
		}
		else
		{
			nMaxListHeight = InterfaceGetHeight() - 100;
			m_ItemsInView = (nMaxListHeight - liney) / 17;
		}


		while ( pItem )
		{
			if ( nListCount >= m_FirstInView )
			{
				if ( liney < nMaxListHeight )
				{
				int	pressX, pressY;

					PlatformGetCurrentCursorPosition( &pressX, &pressY );

					if ( ( pressX >= X + 5 ) &&
						 ( pressX <= X + W - 10 ) &&
						 ( pressY >= liney ) &&
						 ( pressY < liney + nItemSize ) )
					{
						InterfaceRect( 2, X + 3, liney, W - 6, nItemSize, 0xC0A08010 );
						PlatformSetMouseOverCursor( TRUE );
						m_MouseHoverItem = 1;
						m_pHoverItem = pItem;
					}
					InterfaceText( 2, X + 5, liney + nTexOffY, pItem->GetTitle(), 0xC0c0c0c0, 1 );
					liney += nItemSize;
				}
			}
			nListCount++;
			pItem = pItem->GetNext();
		}
	}

	int		nDropArrowX = X + W - 32;
	int		nDropArrowY = nTextY - 1;
	int		nDropArrowW = 31;
	int		nDropArrowH = H - 2;

	if ( m_sbUIDropdownIsAnyListActive )
	{
	int	pressX, pressY;

		PlatformGetCurrentCursorPosition( &pressX, &pressY );

		if ( ( pressX >= nDropArrowX ) &&
			 ( pressX <= nDropArrowX + nDropArrowW ) &&
			 ( pressY >= nDropArrowY ) &&
			 ( pressY <= nDropArrowY + nDropArrowH ) )
		{
			PlatformSetMouseOverCursor( TRUE );
			m_MouseHoverItem = 0;
		}
	}
	else
	{
	int	pressX, pressY;

		PlatformGetCurrentCursorPosition( &pressX, &pressY );

		if ( ( pressX >= X ) &&
			 ( pressX <= X + W ) &&
			 ( pressY >= nDropArrowY ) &&
			 ( pressY <= nDropArrowY + nDropArrowH ) )
		{
			PlatformSetMouseOverCursor( TRUE );
			m_MouseHoverItem = 0;
		}
	}

	// Draw dropdown box on right
	InterfaceOutlineBox( 1, X + W - 32, Y + 1, 32, H - 2, 0xC0808080 );

	InterfaceTri( 1, X + W - 25, nDropArrowY + 5, X + W - 7, nDropArrowY + 5, X + W - 16, nDropArrowY + 15, ulCol, ulCol, ulCol );
}


void		UIDropdownRender( int nHandle, int X, int Y, int W, int H, int nFullH, float fAlpha )
{
UIDropdown*		pDropdown = DropdownFind( nHandle );

	if ( pDropdown )
	{
		pDropdown->Render( X, Y, W, H, nFullH, fAlpha );
	}
}


void		UIDropdownReset( int nHandle )
{
UIDropdown*		pDropdown = DropdownFind( nHandle );

	if ( pDropdown )
	{
		pDropdown->Reset();
	}
}

BOOL		DropdownHasMouseFocus( void )
{
UIDropdown*	pDropdown = m_spUIActiveDropdowns;
	
	while( pDropdown )
	{
		if ( pDropdown->HasMouseFocus() )
		{
			return( TRUE );
		}
		pDropdown = pDropdown->GetNext();
	}

	return( FALSE );
}

BOOL		DropdownOnMouseWheelMove( float fOffset )
{
UIDropdown*	pDropdown = m_spUIActiveDropdowns;
	
	while( pDropdown )
	{
		if ( pDropdown->OnMouseWheelMove( fOffset ) )
		{
			return( TRUE );
		}
		pDropdown = pDropdown->GetNext();
	}

	return( FALSE );
}

int		UIDropdownOnPress( int X, int Y )
{

	return( FALSE );
}

int		UIDropdownOnRelease( int X, int Y )
{
UIDropdown*	pDropdown = m_spUIActiveDropdowns;
	
	while( pDropdown )
	{
		if ( pDropdown->OnRelease(X,Y) )
		{
			return( TRUE );
		}
		pDropdown = pDropdown->GetNext();
	}

	return( FALSE );
}

void		DropdownDelete( int nHandle )
{
UIDropdown*		pPrevious = NULL;
UIDropdown*		pDropdown = m_spUIActiveDropdowns;

	while ( pDropdown )
	{
		if ( pDropdown->GetHandle() == nHandle )
		{
			if ( pPrevious )
			{
				pPrevious->SetNext( pDropdown->GetNext() );
			}
			else
			{
				m_spUIActiveDropdowns = pDropdown->GetNext();
			}
			delete( pDropdown );
			return;
		}
		pPrevious = pDropdown;
		pDropdown = pDropdown->GetNext();
	}
}

void	UIDropdownNewFrame( void )
{
	m_sbUIDropdownIsAnyListActive = false;
}
