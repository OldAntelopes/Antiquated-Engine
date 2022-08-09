
#include "StandardDef.h"
#include "Interface.h"

#include "../Platform/Platform.h"
#include "UI.h"
#include "UICheckbox.h"

class UICheckbox
{
public:
	void			Render( int X, int Y, int W, int H )
	{
	int		nCheckboxW = H - 2;
	
		mnPosX = X;
		mnPosY = Y;
		mnPosW = W;
		mnPosH = H;

		InterfaceOutlineBox( 0, X, Y, nCheckboxW, H, 0xf0f0f0f0 );
		if ( mbIsChecked )
		{
			InterfaceText( 1, X + 5, Y + 1, "x", 0xd0d0d0d0, 1 );
		}
		else
		{

		}
		InterfaceText( 1, X + nCheckboxW + 8, Y + 4, mszText, 0xd0e0e0e0, 0 );

		if ( UIIsMouseHover( X, Y, W, H ) )
		{
			PlatformSetMouseOverCursor( TRUE );
		}

	}

	BOOL	OnRelease( int X, int Y )
	{
		if ( ( X >= mnPosX ) &&
			 ( X <= mnPosX + mnPosW ) &&
			 ( Y >= mnPosY ) &&
			 ( Y <= mnPosY + mnPosH ) )
		{
			mbIsChecked = !mbIsChecked;
			return( TRUE );
		}

		return( FALSE );
	}

	int				mnPosX;
	int				mnPosY;
	int				mnPosW;
	int				mnPosH;

	char*			mszText;
	BOOL			mbIsChecked;
	int				mnHandle;
	UICheckbox*		mpNext;
};

UICheckbox*		mspCheckboxList = NULL;
int				msnNextCheckboxHandle = 0x100;

UICheckbox*		UICheckboxFind( int nHandle )
{
UICheckbox*		pCheckbox = mspCheckboxList;

	while( pCheckbox )
	{
		if ( pCheckbox->mnHandle == nHandle )
		{
			return( pCheckbox );
		}
		pCheckbox = pCheckbox->mpNext;
	}
	return( NULL );	
}


int		UICheckboxCreate( const char* szText, BOOL bIsChecked )
{
UICheckbox*		pCheckbox = new UICheckbox;
int			nNewHandle = msnNextCheckboxHandle++;
	
	pCheckbox->mnHandle = nNewHandle;
	pCheckbox->mpNext = mspCheckboxList;
	mspCheckboxList = pCheckbox;

	pCheckbox->mszText = (char*)( malloc( strlen( szText ) + 1 ) );
	strcpy( pCheckbox->mszText, szText );
	pCheckbox->mbIsChecked = bIsChecked;
	return( nNewHandle );
}


void		UICheckboxRender( int nHandle, int ScreenX, int ScreenY, int ScreenW, int ScreenH )
{
UICheckbox*		pCheckbox = UICheckboxFind( nHandle );

	if ( pCheckbox )
	{
		pCheckbox->Render( ScreenX, ScreenY, ScreenW, ScreenH );
	}

}

BOOL		UICheckboxIsChecked( int nHandle )
{
UICheckbox*		pCheckbox = UICheckboxFind( nHandle );

	if ( pCheckbox )
	{
		return( pCheckbox->mbIsChecked );
	}
	return( FALSE );

}

void		UICheckboxDestroy( int nHandle )
{
	// TODO 

}

int			UICheckboxOnRelease( int X, int Y )
{
UICheckbox*		pCheckbox = mspCheckboxList;

	while( pCheckbox )
	{
		if ( pCheckbox->OnRelease( X, Y ) )
		{
			return( TRUE );
		}
		pCheckbox = pCheckbox->mpNext;
	}
	return( FALSE );
}
