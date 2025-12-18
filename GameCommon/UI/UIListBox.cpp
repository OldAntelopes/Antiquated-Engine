
#include <stdio.h>
#include <vector>

#include "StandardDef.h"
#include "Interface.h"

#include "../Platform/Platform.h"
#include "UI.h"
#include "UITextBox.h"

uint32			msulListBoxRenderFrame = 0;

class UIListBoxEntry
{
public:
	UIListBoxEntry()
	{
		mszText = NULL;
		mulParam = 0;
	}
	
	void	Init( const char* szText, uint32 param );

private:
	char*	mszText;
	uint32	mulParam;
};


class UIListBox
{
public:
	UIListBox()
	{
		mnMode = 0;
		mbIsFocused = FALSE;
		mulLastRenderFrame = 0;
	}

	~UIListBox()
	{
	}

	void			Init( int nMode );

	void			Render( int nScreenX, int nScreenY, int nScreenW, int nScreenH );

	BOOL			IsOnScreen( void );
	void			OnSelect( int nX, int nY );

	int				GetHandle( void ) { return( mnHandleID ); }
	void			SetHandle( int nHandle ) { mnHandleID = nHandle; }

	void			SetFocused( BOOL bFlag ) { mbIsFocused = bFlag; }

	void			AddEntry( const char* szElementName, uint32 ulElementParam );
private:
	int				mnHandleID;
	int				mnMode;
	uint32			mulLastRenderFrame;
	BOOL			mbIsFocused;

	std::vector<UIListBoxEntry*>	mListBoxEntries;
};

int				msnListBoxNextHandle = 401;
UIListBoxEntry*		mspListBoxEntryHover = NULL;

std::vector<UIListBox*>		msListBoxList;
//-------------------------------------------------------------------------


void	UIListBoxEntry::Init( const char* szText, uint32 param )
{
	mszText = (char*)( SystemMalloc( strlen(szText) + 1 ));
	strcpy( mszText, szText );
	mulParam = param;

}


//-------------------------------------------------------------------------

void		UIListBox::Init( int nMode )
{
	mnMode = nMode;
}


void	UIListBox::AddEntry( const char* szElementName, uint32 ulElementParam )
{


}

void	UIListBox::Render( int nScreenX, int nScreenY, int nScreenW, int nScreenH )
{
int		nFontToUse = 0;
int		nTextOffsetY = 0;

	mulLastRenderFrame = msulListBoxRenderFrame;

	if ( nScreenH > 30 ) nFontToUse = 1;

	if ( mbIsFocused )
	{
		InterfaceRect( 1, nScreenX, nScreenY, nScreenW, nScreenH, 0xFFE0D0B0 );
		InterfaceRect( 1, nScreenX, nScreenY, nScreenW, 2, 0xD0B09070 );
		InterfaceRect( 1, nScreenX, nScreenY+2, 2, nScreenH-2, 0xD0B09070 );
	}
	else
	{
		InterfaceRect( 1, nScreenX, nScreenY, nScreenW, nScreenH, 0xFFD0C0A0 );
		InterfaceRect( 1, nScreenX, nScreenY, nScreenW, 3, 0xD0907050 );
		InterfaceRect( 1, nScreenX, nScreenY+3, 3, nScreenH-3, 0xD0907050 );
	}
	InterfaceLine( 2, nScreenX - 1, nScreenY - 1, nScreenX + nScreenW, nScreenY - 1, 0xE0202020, 0xE0202020 );
	InterfaceLine( 2, nScreenX - 1, nScreenY - 1, nScreenX - 1, nScreenY + nScreenH, 0xE0202020, 0xE0202020 );
	InterfaceLine( 2, nScreenX, nScreenY + nScreenH, nScreenX + nScreenW, nScreenY + nScreenH, 0xE0c0c0c0, 0xE0c0c0c0 );
	InterfaceLine( 2, nScreenX + nScreenW, nScreenY, nScreenX + nScreenW, nScreenY + nScreenH, 0xE0c0c0c0, 0xE0c0c0c0 );



}

UIListBox*		UIListBoxFind( int nHandle )
{
	for ( UIListBox* pListBox : msListBoxList )
	{
		if ( pListBox->GetHandle() == nHandle )
		{
			return( pListBox );
		}
	}
	return( NULL );
}

int		UIListBoxAddElement( int nHandle, const char* szElementName, uint32 ulElementParam )
{
UIListBox* pListBox = UIListBoxFind( nHandle );

	if ( pListBox )
	{
		pListBox->AddEntry( szElementName, ulElementParam );
	}
	return 0;
}


void			UIListBoxNewFrame( void )
{
	mspListBoxEntryHover = NULL;
	msulListBoxRenderFrame++;

}

int				UIListBoxCreate( int nMode )
{
UIListBox*		pNewListBox = new UIListBox;
int				nNewHandle =  msnListBoxNextHandle++;

	pNewListBox->SetHandle( nNewHandle );

	msListBoxList.push_back( pNewListBox );

	pNewListBox->Init( nMode );

	return( nNewHandle );
}

void			UIListBoxRender( int nHandle, int nScreenX, int nScreenY, int nScreenW, int nScreenH )
{
int				nHoldX, nHoldY;

	PlatformGetCurrentCursorPosition( &nHoldX, &nHoldY );

	UIListBox* pListBox = UIListBoxFind( nHandle );
	if ( pListBox )
	{
		if ( ( nHoldX >= nScreenX ) &&
			 ( nHoldX <= nScreenX + nScreenW ) &&
			 ( nHoldY >= nScreenY ) &&
			 ( nHoldY <= nScreenY + nScreenH ) )
		{
//			mspTextBoxHover = pTextBox;
			PlatformSetMouseOverCursor( TRUE );
		}
		pListBox->Render( nScreenX, nScreenY, nScreenW, nScreenH );
	}

}


void			UIListBoxDestroy( int nHandle )
{
	// TODO
}

BOOL		UIListBoxOnPress( int X, int Y )
{
	if ( mspListBoxEntryHover )
	{

		return( TRUE );
	}
	return( FALSE );
}

BOOL		UIListBoxOnRelease( int X, int Y )
{
	if ( mspListBoxEntryHover )
	{
		// TODO

		mspListBoxEntryHover = NULL;

		return( TRUE );
	}
	return( FALSE );

}


void	UIListBoxShutdown( void )
{
	// TODO
}