
#include "StandardDef.h"
#include "Interface.h"

#include "../Platform/Platform.h"
#include "UI.h"

class UISlider
{
public:
	UISlider();
	~UISlider();
	 
	void		Draw( int nX, int nY, int nWidth, int nHeight, int nFlags, float fGlobalAlpha );
	int			GetValue( void ) { return( mnValue ); }
	void		SetValue( int nValue ) { mnValue = nValue; }

	void		SetNext( UISlider* pNext ) { mpNext = pNext; }
	UISlider*	GetNext( void ) { return( mpNext ); }

	BOOL		OnPress( int X, int Y );
	BOOL		OnRelease( int X, int Y );

	void		InitSlider( int nHandle, int nValue, int nMin, int nMax );
	void		SetLimits( int nMin, int nMax );

	int			GetHandle( void ) { return( mnSliderHandle ); }
private:
	void		UpdateHold( void );

	int			mnValue;
	int			mnMax;
	int			mnMin;
	int			mnUnitDisplayWidth;
	int			mnPressValue;
	int			mnBarX;
	int			mnBarY;
	int			mnBarW;
	int			mnBarH;

	int			mnTabX;
	int			mnTabY;
	int			mnTabH;
	int			mnTabW;
	int			mnSliderHandle;
	int			mnReverseDir;
	UISlider*		mpNext;

};

int				msnSliderNextHandle = 101;
UISlider*		mspSliderList = NULL;

int				msnSliderPressHandle = NOTFOUND;
int				msnSliderPressX = NOTFOUND;
int				msnSliderPressY = NOTFOUND;
int				msnSliderPressType = 0;

UISlider::UISlider()
{
	mnBarX = 0;
	mnBarY = 0;
	mnBarW = 0;
	mnBarH = 0;

	mnTabX = 0;
	mnTabY = 0;
	mnTabH = 0;
	mnTabW = 0;
}

UISlider::~UISlider()
{


}

void	UISlider::InitSlider( int nHandle, int nValue, int nMin, int nMax )
{
	mnSliderHandle = nHandle;
	mnValue = nValue;
	mnMax = nMax;
	mnMin = nMin;
}

void	UISlider::SetLimits( int nMin, int nMax )
{
	mnMax = nMax;
	mnMin = nMin;

	if ( mnValue < mnMin ) mnValue = mnMin;
	if ( mnValue > mnMax ) mnValue = mnMax;
}

BOOL	UISlider::OnRelease( int X, int Y )
{
int		nMoveX;
int		nMoveUnits;
int		nScreenEpsilon = InterfaceGetWidth()/100;

	if ( mnUnitDisplayWidth < 1 ) mnUnitDisplayWidth = 1;

	nMoveX = X - msnSliderPressX;
	nMoveUnits = nMoveX / mnUnitDisplayWidth;
	
	// If not pressed tab
	if ( msnSliderPressType == 0 )
	{
		if ( ( nMoveX > -nScreenEpsilon ) &&
			 ( nMoveX < nScreenEpsilon ) )
		{
		int		nNewValue;
		
			nNewValue = X - mnBarX;
			nNewValue /= mnUnitDisplayWidth;
			if ( nNewValue < mnMin )
			{
				nNewValue = mnMin;
			}
			else if ( nNewValue > mnMax )
			{
				nNewValue = mnMax;
			}
			mnValue = nNewValue;
		}
	}
	return( TRUE );
}

BOOL	UISlider::OnPress( int X, int Y )
{
	if ( ( X >= mnTabX ) &&
		 ( X <= mnTabX + mnTabW ) &&
		 ( Y >= mnTabY ) &&
		 ( Y <= mnTabY + mnTabH ) )
	{
		msnSliderPressType = 1;
		mnPressValue = mnValue;
		msnSliderPressX = X;
		msnSliderPressY = Y;
		msnSliderPressHandle = GetHandle();
		return( TRUE );
	}
	else if ( ( X >= mnBarX ) &&
			  ( X <= mnBarX + mnBarW ) &&
			  ( Y >= mnBarY ) &&
			  ( Y <= mnBarY + mnBarH ) )
	{
		msnSliderPressType = 0;
		msnSliderPressX = X;
		msnSliderPressY = Y;
		msnSliderPressHandle = GetHandle();
		return( TRUE );
	}
	return( FALSE );
}

void	UISlider::UpdateHold( void )
{
int		HoldX;
int		HoldY;
	
	if ( msnSliderPressType == 1 )
	{
	int	nMoveX;
	int	nMoveUnits;

		PlatformGetCurrentCursorPosition( &HoldX, &HoldY );

		if ( mnReverseDir == 1 )
		{
			nMoveX = msnSliderPressX - HoldX;
		}
		else
		{
			nMoveX = HoldX - msnSliderPressX;
		}
		if ( mnUnitDisplayWidth != 0 )
		{
			nMoveUnits = nMoveX / mnUnitDisplayWidth;
		}
		else
		{
			nMoveUnits = nMoveX;
		}
	
		mnValue = mnPressValue + nMoveUnits;
		if ( mnValue < mnMin ) 
		{
			mnValue = mnMin;
		}
		else if ( mnValue > mnMax )
		{
			mnValue = mnMax;
		}
	}
}

void	UISlider::Draw( int nX, int nY, int nWidth, int nHeight, int nFlags, float fGlobalAlpha )
{
int			nSliderPos;
int			nSliderLineX = nX + 5;
int			nSliderLineW = nWidth - 10;
int			nSliderRange = mnMax - mnMin;
int			nSliderMidY = nY + (nHeight/2);
int			nSliderTabWidth = nWidth / (nSliderRange+1);
int			nCursorX, nCursorY;
uint32		ulCol;

	PlatformGetCurrentCursorPosition( &nCursorX, &nCursorY );

	if ( nSliderRange < 2 )
	{
		nSliderTabWidth = nWidth / (nSliderRange+1);
	}
	else
	{
		nSliderTabWidth = nWidth / (nSliderRange);
	}


	mnBarX = nX;
	mnBarY = nY;
	mnBarW = nWidth;
	mnBarH = nHeight;

	if ( ( nCursorX >= mnBarX ) &&
		 ( nCursorX <= mnBarX + mnBarW ) &&
		 ( nCursorY >= mnBarY ) &&
		 ( nCursorY <= mnBarY + mnBarH ) )
	{
		PlatformSetMouseOverCursor( TRUE );
	}

	mnUnitDisplayWidth = nSliderTabWidth;

	if ( nSliderTabWidth < 30 )
	{
		nSliderTabWidth = 30;
	}
	nSliderPos = mnValue - mnMin;
	if ( nFlags == 1 )
	{
		nSliderPos = nSliderLineW - ( (nSliderPos * nSliderLineW) / (nSliderRange+1) );
		nSliderPos -= nSliderTabWidth;
		mnReverseDir = 1;
	}
	else
	{
		nSliderPos = (nSliderPos * nSliderLineW) / (nSliderRange+1);
		mnReverseDir = 0;
	}

	ulCol = GetColWithModifiedAlpha( 0xD0808078, fGlobalAlpha );
	InterfaceRect( 0, nSliderLineX, nSliderMidY, nSliderLineW, 1, ulCol );
	ulCol = GetColWithModifiedAlpha( 0xD0404040, fGlobalAlpha );
	InterfaceRect( 0, nSliderLineX, nSliderMidY + 1, nSliderLineW, 1, 0xD0404040 );


	if ( nSliderRange > 0 )
	{
		mnTabX = nSliderLineX + nSliderPos;
		mnTabY = nY;
		mnTabW = nSliderTabWidth;
		mnTabH = nHeight;

		ulCol = GetColWithModifiedAlpha( 0xD0404040, fGlobalAlpha );
		InterfaceRect( 1, mnTabX, mnTabY, mnTabW, 1, ulCol );
		InterfaceRect( 1, mnTabX, mnTabY + 1, 1, mnTabH - 2, ulCol );

		ulCol = GetColWithModifiedAlpha( 0xFF808080, fGlobalAlpha );
		InterfaceRect( 1, mnTabX, mnTabY, mnTabW, mnTabH, ulCol );

		ulCol = GetColWithModifiedAlpha( 0xFFb0b0b0, fGlobalAlpha );
		InterfaceRect( 1, mnTabX + 1, mnTabY + 1, mnTabW - 2, 1, ulCol );
		InterfaceRect( 1, mnTabX + 1, mnTabY + 2, 1, mnTabH - 4, ulCol );

		ulCol = GetColWithModifiedAlpha( 0xFF303030, fGlobalAlpha );
		InterfaceRect( 1, mnTabX, mnTabY + mnTabH - 1, mnTabW, 1, ulCol );
		InterfaceRect( 1, mnTabX + mnTabW, mnTabY + 1, 1, mnTabH - 2, ulCol );

		ulCol = GetColWithModifiedAlpha( 0xFF606060, fGlobalAlpha );
		InterfaceRect( 1, mnTabX + 1, mnTabY + mnTabH - 2, mnTabW - 2, 1, ulCol );
		InterfaceRect( 1, mnTabX + mnTabW - 1, mnTabY + 1, 1, mnTabH - 2, ulCol );
	}

	if ( msnSliderPressHandle == GetHandle() )
	{
		UpdateHold();
	}
}

UISlider*	UISliderFindHandle( int nHandle )
{
UISlider*	pSlider = mspSliderList;

	while( pSlider )
	{
		if ( pSlider->GetHandle() == nHandle )
		{
			return( pSlider );
		}
		pSlider = pSlider->GetNext();
	}
	return( NULL );
}



int		UISliderCreate( int nValue, int nMin, int nMax )
{
UISlider*	pSlider = new UISlider;
int			nHandle = msnSliderNextHandle++;

	pSlider->SetNext( mspSliderList );
	mspSliderList = pSlider;

	pSlider->InitSlider( nHandle, nValue, nMin, nMax );

	return( nHandle );
}

void		UISliderSetLimits( int nHandle, int nMin, int nMax )
{
UISlider*	pSlider = UISliderFindHandle( nHandle );

	if ( pSlider )
	{
		pSlider->SetLimits( nMin, nMax );
	}
}

void		UISliderDraw( int nHandle, int nX, int nY, int nWidth, int nHeight, int nFlags, float fGlobalAlpha )
{
UISlider*	pSlider = UISliderFindHandle( nHandle );

	if ( pSlider )
	{
		pSlider->Draw( nX, nY, nWidth, nHeight, nFlags, fGlobalAlpha );
	}
}

void		UISliderSetValue( int nHandle, int nValue )
{
UISlider*	pSlider = UISliderFindHandle( nHandle );

	if ( pSlider )
	{
		pSlider->SetValue( nValue );
	}
}

int		UISliderGetValue( int nHandle )
{
UISlider*	pSlider = UISliderFindHandle( nHandle );

	if ( pSlider )
	{
		return( pSlider->GetValue() );
	}
	return( 0 );
}

void		UISliderDestroy( int nHandle )
{
UISlider*	pSlider = mspSliderList;
UISlider*	pLast = NULL;

	while( pSlider )
	{
		if ( pSlider->GetHandle() == nHandle )
		{
			if ( pLast )
			{
				pLast->SetNext( pSlider->GetNext() );
			}
			else
			{
				mspSliderList = pSlider->GetNext();
			}
			delete pSlider;
			return;
		}
		else
		{
			pLast = pSlider;
			pSlider = pSlider->GetNext();
		}
	}

}

BOOL		UISliderOnPress( int X, int Y )
{
	if ( msnSliderPressHandle != NOTFOUND )
	{
	UISlider*	pSlider = UISliderFindHandle( msnSliderPressHandle );

		InterfaceText( 0, 100, 100, "Slider on move", 0xD0D0D0D0, 0 );
		// On Move... ?
		return( TRUE );
	}
	else
	{
	UISlider*	pSlider = mspSliderList;

		while( pSlider )
		{
			if ( pSlider->OnPress( X, Y ) )
			{
				msnSliderPressHandle = pSlider->GetHandle();
				return( TRUE );
			}
			pSlider = pSlider->GetNext();
		}
	}
	return( FALSE );
}

BOOL		UISliderOnRelease( int X, int Y, BOOL bFocused )
{
	if ( msnSliderPressHandle != NOTFOUND )
	{
		if ( bFocused )
		{
		UISlider*	pSlider = UISliderFindHandle( msnSliderPressHandle );

			if ( pSlider )
			{
				pSlider->OnRelease( X, Y );
			}
			msnSliderPressHandle = NOTFOUND;
			return( TRUE );
		}
		else
		{
			msnSliderPressHandle = NOTFOUND;
		}
	}
	return( FALSE );
}

