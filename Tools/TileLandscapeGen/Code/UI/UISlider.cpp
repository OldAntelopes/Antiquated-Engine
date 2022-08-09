
#include "StandardDef.h"
#include "Interface.h"

#include "../Platform/Platform.h"
#include "UI.h"

class UISlider
{
public:
	UISlider();
	~UISlider();
	 
	void		Draw( int nX, int nY, int nWidth, int nHeight, int nFlags );
	int			GetValue( void ) { return( mnValue ); }
	void		SetValue( int nValue ) { mnValue = nValue; }

	void		SetNext( UISlider* pNext ) { mpNext = pNext; }
	UISlider*	GetNext( void ) { return( mpNext ); }

	BOOL		OnPress( int X, int Y );
	BOOL		OnRelease( int X, int Y );

	void		InitSlider( int nHandle, int nValue, int nMin, int nMax );

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

BOOL	UISlider::OnRelease( int X, int Y )
{
int		nMoveX;
int		nMoveUnits;
int		nScreenEpsilon = InterfaceGetWidth()/100;

	if ( mnUnitDisplayWidth < 1 ) mnUnitDisplayWidth = 1;

	nMoveX = X - msnSliderPressX;
	nMoveUnits = nMoveX / mnUnitDisplayWidth;
	
	// If pressed tab
	if ( msnSliderPressType == 0 )
	{
		if ( ( nMoveX > -nScreenEpsilon ) &&
			 ( nMoveX < nScreenEpsilon ) )
		{
		int		nNewValue;
		
			nNewValue = (mnBarX + mnBarW) - X;
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

		PlatformGetCurrentHoldPosition( &HoldX, &HoldY );

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

void	UISlider::Draw( int nX, int nY, int nWidth, int nHeight, int nFlags )
{
int			nSliderPos;
int			nSliderLineX = nX + 5;
int			nSliderLineW = nWidth - 10;
int			nSliderRange = mnMax - mnMin;
int			nSliderMidY = nY + (nHeight/2);
int			nSliderTabWidth = nWidth / (nSliderRange+1);

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

	InterfaceRect( 0, nSliderLineX, nSliderMidY, nSliderLineW, 1, 0xD0A0A0A0 );
	InterfaceRect( 0, nSliderLineX, nSliderMidY + 1, nSliderLineW, 1, 0xD0404040 );

	if ( nSliderRange > 0 )
	{
		mnTabX = nSliderLineX + nSliderPos;
		mnTabY = nY;
		mnTabW = nSliderTabWidth;
		mnTabH = nHeight;

		InterfaceRect( 1, mnTabX, mnTabY, mnTabW, mnTabH, 0xFF808080 );
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


void		UISliderDraw( int nHandle, int nX, int nY, int nWidth, int nHeight, int nFlags )
{
UISlider*	pSlider = UISliderFindHandle( nHandle );

	if ( pSlider )
	{
		pSlider->Draw( nX, nY, nWidth, nHeight, nFlags );
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

