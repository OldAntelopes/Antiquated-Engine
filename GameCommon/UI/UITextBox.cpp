
#include <stdio.h>

#include "StandardDef.h"
#include "Interface.h"

#include "../Platform/Platform.h"
#include "UI.h"
#include "UITextBox.h"

uint32			msulTextBoxRenderFrame = 0;


class UITextBox
{
public:
	UITextBox()
	{
		mpNext = NULL;
		mszText = NULL;
		mnMode = 0;
		mnTextBufferSize = 0;
		mnTextMaxLen = 0;
		mbIsFocused = FALSE;
		mulLastRenderFrame = 0;
	}

	~UITextBox()
	{
		SAFE_FREE( mszText );
	}

	void			Init( int nMode, const char* szInitialText, int nMaxTextLen );

	void			Render( int nScreenX, int nScreenY, int nScreenW, int nScreenH );
	const char*		GetText() { return( mszText ); }

	BOOL			IsOnScreen( void );
	void			SetText( const char* szNewText );
	void			OnSelect( int nX, int nY );

	UITextBox*		GetNext( void ) { return( mpNext ); }
	void			SetNext( UITextBox* pNext ) { mpNext = pNext; }
	
	int				GetHandle( void ) { return( mnHandleID ); }
	void			SetHandle( int nHandle ) { mnHandleID = nHandle; }

	void			SetFocused( BOOL bFlag ) { mbIsFocused = bFlag; }

private:
	int				mnHandleID;
	char*			mszText;
	int				mnTextBufferSize;
	int				mnTextMaxLen;
	int				mnMode;
	uint32			mulLastRenderFrame;
	BOOL			mbIsFocused;
	UITextBox*		mpNext;
};

int				msnTextBoxNextHandle = 401;
UITextBox*		mspTextBoxList = NULL;

UITextBox*		mspTextBoxHover = NULL;
UITextBox*		mspFocusedTextBox = NULL;


BOOL		UITextBox::IsOnScreen( void )
{ 
	if ( msulTextBoxRenderFrame - 1 <= mulLastRenderFrame )
	{
		return( TRUE );
	}
	return( FALSE ); 
}

void		UITextBox::Init( int nMode, const char* szInitialText, int nMaxTextLen )
{
	mnMode = nMode;
	mnTextMaxLen = nMaxTextLen;
	mnTextBufferSize = mnTextMaxLen + 1;
	mszText = (char*)( malloc( nMaxTextLen + 1 ) );
	if ( szInitialText )
	{
		strcpy( mszText, szInitialText );
	}
	else
	{
		mszText[0] = 0;
	}
}

void	UITextBox::SetText( const char* szNewText )
{
	strcpy( mszText, szNewText );
}

void	UITextBox::Render( int nScreenX, int nScreenY, int nScreenW, int nScreenH )
{
int		nTextHeight;
int		nFontToUse = 0;
int		nTextOffsetY = 0;

	mulLastRenderFrame = msulTextBoxRenderFrame;

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

	nTextHeight = GetStringHeight( mszText, nFontToUse );
	nTextOffsetY = ((nScreenH - 2) - nTextHeight ) / 2;

	// Is a password dialog
	if ( mnMode == 1 )
	{
	char	acInputBuffer[512];
	int		nTextLen;
	int		nLoop;

		if ( mbIsFocused == TRUE )
		{
			strcpy( acInputBuffer, PlatformKeyboardGetInputString( TRUE ) );
		}
		else
		{
			strcpy( acInputBuffer, mszText );
		}

		nTextLen = strlen( acInputBuffer );
		if ( mbIsFocused == TRUE )
		{ 
			if ( nTextLen > 0 )
			{
				if ( acInputBuffer[nTextLen-1] == '_' )
				{
					nTextLen -= 2;
				}
				else
				{
					nTextLen--;
				}
			}
		}

		for( nLoop = 0; nLoop < nTextLen; nLoop++ )
		{
			acInputBuffer[nLoop] = '*';
		}
		InterfaceTextLimitWidth( 2, nScreenX + 3, nScreenY + nTextOffsetY, acInputBuffer, 0xE0303030, nFontToUse, nScreenW - 6 );
	}
	else
	{
		if ( mbIsFocused == TRUE )
		{
		char	acInputBuffer[512];
			strcpy( acInputBuffer, PlatformKeyboardGetInputString( TRUE ) );
			InterfaceTextLimitWidth( 2, nScreenX + 3, nScreenY + nTextOffsetY, acInputBuffer, 0xE0303030, nFontToUse, nScreenW - 6 );
		}
		else if ( mszText )
		{
			InterfaceTextLimitWidth( 2, nScreenX + 3, nScreenY + nTextOffsetY, mszText, 0xE0303030, nFontToUse, nScreenW - 6 );
		}
	}


}


void			UITextBoxNewFrame( void )
{
	mspTextBoxHover = NULL;
	msulTextBoxRenderFrame++;

}

int				UITextBoxCreate( int nMode, const char* szInitialText, int nMaxTextLen )
{
UITextBox*		pNewTextBox = new UITextBox;
int				nNewHandle =  msnTextBoxNextHandle++;

	pNewTextBox->SetHandle( nNewHandle );

	if ( mspTextBoxList == NULL )
	{
		mspTextBoxList = pNewTextBox;
	}
	else
	{
	UITextBox*		pLastInList = mspTextBoxList;

		while( pLastInList->GetNext() != NULL )
		{
			pLastInList = pLastInList->GetNext();
		}
		pLastInList->SetNext( pNewTextBox );
	}

	pNewTextBox->SetNext( NULL );

	pNewTextBox->Init( nMode, szInitialText, nMaxTextLen );

	return( nNewHandle );
}

void			UITextBoxRender( int nHandle, int nScreenX, int nScreenY, int nScreenW, int nScreenH )
{
UITextBox*		pTextBox = mspTextBoxList;
int				nHoldX, nHoldY;

	PlatformGetCurrentCursorPosition( &nHoldX, &nHoldY );

	while( pTextBox )
	{
		if ( pTextBox->GetHandle() == nHandle )
		{
			if ( ( nHoldX >= nScreenX ) &&
				 ( nHoldX <= nScreenX + nScreenW ) &&
				 ( nHoldY >= nScreenY ) &&
				 ( nHoldY <= nScreenY + nScreenH ) )
			{
				mspTextBoxHover = pTextBox;
				PlatformSetMouseOverCursor( TRUE );
			}
			pTextBox->Render( nScreenX, nScreenY, nScreenW, nScreenH );

			if ( pTextBox == mspFocusedTextBox )
			{
				pTextBox->SetText( PlatformKeyboardGetInputString( FALSE ) );
			}

			return;
		}
		pTextBox = pTextBox->GetNext();
	}

}
void			UITextBoxEndEdit( int nHandle )
{
	if ( ( mspFocusedTextBox ) &&
		 ( mspFocusedTextBox->GetHandle() == nHandle ) )
	{
		mspFocusedTextBox->SetText( PlatformKeyboardGetInputString(FALSE) );
		mspFocusedTextBox->SetFocused( FALSE );

		PlatformKeyboardSetInputString( "" );
	}

}

const char*		UITextBoxGetText( int nHandle )
{
UITextBox*		pTextBox = mspTextBoxList;

	while( pTextBox )
	{
		if ( pTextBox->GetHandle() == nHandle )
		{
			return( pTextBox->GetText() );
		}
		pTextBox = pTextBox->GetNext();
	}
	return( "" );
}

void			UITextBoxDestroy( int nHandle )
{
UITextBox*		pTextBox = mspTextBoxList;
UITextBox*		pLast = NULL;
UITextBox*		pNext;

	while( pTextBox )
	{
		pNext = pTextBox->GetNext();
		if ( pTextBox->GetHandle() == nHandle )
		{
			if ( pLast == NULL )
			{
				mspTextBoxList = pNext;
			}
			else
			{
				pLast->SetNext( pNext );
			}
			delete pTextBox;
			return;
		}
		pTextBox = pNext;
	}
}

int		UITextBoxKeyboardMessageHandler( int nResponseCode, const char* szInputText )
{
	switch( nResponseCode )
	{
	case 1:	// press enter
		mspFocusedTextBox->SetText( szInputText );
		mspFocusedTextBox->SetFocused( FALSE );
		mspFocusedTextBox = NULL;
		PlatformKeyboardSetInputString( "" );
		break;
	case 2:		// press TAB
		{
		UITextBox*		pTextBox = mspTextBoxList;
	
			while( pTextBox )
			{
				if ( pTextBox == mspFocusedTextBox )
				{
				UITextBox*		pNextFocus;
					mspFocusedTextBox->SetText( szInputText );
					mspFocusedTextBox->SetFocused( FALSE );	
					PlatformKeyboardSetInputString( "" );

					pNextFocus = pTextBox;
					do
					{
						if ( pNextFocus->GetNext() == NULL )
						{
							pNextFocus = mspTextBoxList;
						}
						else
						{
							pNextFocus = pNextFocus->GetNext();
						}
						
						if ( pNextFocus->IsOnScreen() == TRUE )
						{
							mspFocusedTextBox = pNextFocus;
							break;
						}

					} while ( pNextFocus != mspFocusedTextBox );

					PlatformKeyboardSetInputString( mspFocusedTextBox->GetText() );
					mspFocusedTextBox->SetFocused( TRUE );
					break;
				}
				pTextBox = pTextBox->GetNext();
			}
		}
		break;
	}
	return( 0 );
}

void		UITextBoxEndCurrentEdit( void )
{
	if ( mspFocusedTextBox )
	{
		UITextBoxEndEdit( mspFocusedTextBox->GetHandle() );
	}
}

BOOL		UITextBoxOnRelease( int X, int Y )
{
	if ( mspTextBoxHover )
	{
		if ( mspFocusedTextBox )
		{
		const char*		pcInput = PlatformKeyboardGetInputString(FALSE);

			mspFocusedTextBox->SetText( pcInput );
			mspFocusedTextBox->SetFocused( FALSE );
		}
		mspTextBoxHover->SetFocused( TRUE );
		mspFocusedTextBox = mspTextBoxHover;
		mspTextBoxHover = NULL;

		PlatformKeyboardRegisterHandler( UITextBoxKeyboardMessageHandler );
		PlatformKeyboardActivate( 0,  mspFocusedTextBox->GetText(), "" );
		return( TRUE );
	}
	return( FALSE );

}


void	UITextboxShutdown( void )
{
UITextBox*		pTextBox = mspTextBoxList;
UITextBox*		pNext;

	while( pTextBox )
	{
		pNext = pTextBox->GetNext();
		delete pTextBox;
		pTextBox = pNext;
	}
	mspTextBoxList = NULL;
}